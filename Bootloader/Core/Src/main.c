/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "sha256.h"
#include "metadata.h"
#include "flash_storage.h"
#include "image_validation.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
/* USER CODE BEGIN PFP */
static void JumpToApplication(uint32_t image_start, uint32_t image_end);

static firmware_slot_t GetOtherSlot(firmware_slot_t slot);

static uint32_t GetSlotImageStart(firmware_slot_t slot);

static uint32_t GetSlotImageRegionEnd(firmware_slot_t slot);

static HAL_StatusTypeDef FindConfirmedSlot(
    const firmware_metadata_record_t *slot_a_record,
    const firmware_metadata_record_t *slot_b_record,
    bool slot_a_record_valid,
    bool slot_b_record_valid,
    firmware_slot_t *confirmed_slot);

static void Bootloader_FailSafe(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int __io_putchar(int ch)
{
    ITM_SendChar((uint32_t)ch);
    return ch;
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	firmware_metadata_record_t slot_a_record;
	firmware_metadata_record_t slot_b_record;

	bool slot_a_record_valid = false;
	bool slot_b_record_valid = false;

	HAL_StatusTypeDef status;

	firmware_slot_t active_slot;
	firmware_slot_t inactive_slot;

	const firmware_metadata_t *active_metadata;

	uint8_t active_digest[SHA256_DIGEST_SIZE];

	uint32_t active_image_start;
	uint32_t active_image_end;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CRC_Init();
  /* USER CODE BEGIN 2 */

  /* Read latest committed Slot A metadata record */
  slot_a_record_valid =
      Metadata_ReadLatestRecord(
          SLOT_A_METADATA_ADDRESS,
          SLOT_A_METADATA_REGION_END,
          &slot_a_record);

  if (!slot_a_record_valid)
  {
      printf("No valid Slot A metadata record\r\n");
  }

  /* Read latest committed Slot B metadata record */
  slot_b_record_valid =
      Metadata_ReadLatestRecord(
          SLOT_B_METADATA_ADDRESS,
          SLOT_B_METADATA_REGION_END,
          &slot_b_record);

  if (!slot_b_record_valid)
  {
      printf("No valid Slot B metadata record\r\n");
  }

  /*
   * Determine the currently confirmed firmware slot.
   */
  status = FindConfirmedSlot(
      &slot_a_record,
      &slot_b_record,
      slot_a_record_valid,
      slot_b_record_valid,
      &active_slot);

  if (status != HAL_OK)
  {
      printf("No valid confirmed firmware slot\r\n");
      Bootloader_FailSafe();
  }

  /*
   * The other slot is currently inactive.
   */
  inactive_slot = GetOtherSlot(active_slot);

  printf(
      "Active Slot: %s\r\n",
      (active_slot == FIRMWARE_SLOT_A) ? "A" : "B");

  printf(
      "Inactive Slot: %s\r\n",
      (inactive_slot == FIRMWARE_SLOT_A) ? "A" : "B");

  /*
   * Select metadata belonging to the confirmed slot.
   */
  active_metadata =
      (active_slot == FIRMWARE_SLOT_A) ?
      &slot_a_record.metadata :
      &slot_b_record.metadata;

  /*
   * Validate only the confirmed slot metadata
   * against its fixed memory region.
   */
  if (!Metadata_Validate(
          active_metadata,
          GetSlotImageStart(active_slot),
          GetSlotImageRegionEnd(active_slot)))
  {
      printf("Invalid confirmed slot metadata\r\n");
      Bootloader_FailSafe();
  }

  /*
   * Get confirmed firmware image address.
   */
  active_image_start =
      GetSlotImageStart(active_slot);

  active_image_end =
      active_image_start +
      active_metadata->image_size;

  printf(
      "Active image start: 0x%08lX\r\n",
      (unsigned long)active_image_start);

  printf(
      "Active image size: 0x%08lX\r\n",
      (unsigned long)active_metadata->image_size);

  /*
   * Independently verify the confirmed firmware image.
   */
  printf(
      "Calculating SHA-256 of active Slot %s\r\n",
      (active_slot == FIRMWARE_SLOT_A) ? "A" : "B");

  ImageValidation_CalculateSHA256(
      active_image_start,
      active_metadata->image_size,
      active_digest);

  if (ImageValidation_VerifySHA256(
          active_digest,
          active_metadata->sha256))
  {
      printf("Active firmware SHA-256 matched\r\n");

      JumpToApplication(
          active_image_start,
          active_image_end);
  }

  printf("Active firmware SHA-256 mismatch\r\n");

  Bootloader_FailSafe();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CS_I2C_SPI_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CS_I2C_SPI_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2S3_WS_Pin */
  GPIO_InitStruct.Pin = I2S3_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(I2S3_WS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BOOT1_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOOT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio_SCL_Pin Audio_SDA_Pin */
  GPIO_InitStruct.Pin = Audio_SCL_Pin|Audio_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static void JumpToApplication(uint32_t image_start,
                              uint32_t image_end)
{
    uint32_t app_sp;
    uint32_t app_reset_handler = 0x00U;

    /* Read initial stack pointer from image vector table */
    app_sp = *(volatile uint32_t *)image_start;

    /* Read Reset_Handler address from image vector table */
    app_reset_handler =
        *(volatile uint32_t *)(image_start + 4U);

    /* Validate Application Stack Pointer */
    if ((app_sp < SRAM_START_ADDRESS) ||
        (app_sp > SRAM_END_ADDRESS))
    {
        printf("Invalid Application Stack Pointer\r\n");
        return;
    }

    /* Validate Reset_Handler address */
    if ((app_reset_handler < image_start) ||
        (app_reset_handler >= image_end))
    {
        printf("Invalid Application Reset Handler\r\n");
        return;
    }

    /* Reset_Handler must be Thumb code */
    if ((app_reset_handler & 1U) == 0U)
    {
        printf("Invalid Reset Handler: Thumb bit not set\r\n");
        return;
    }

    /* Point Cortex-M to the selected application's vector table */
    SCB->VTOR = image_start;

    /* Load application stack pointer */
    __set_MSP(app_sp);

    /* Jump to application Reset_Handler */
    void (*reset_handler)(void) =
        (void (*)(void))app_reset_handler;

    reset_handler();
}

static firmware_slot_t GetOtherSlot(firmware_slot_t slot)
{
    return (slot == FIRMWARE_SLOT_A) ?
           FIRMWARE_SLOT_B :
           FIRMWARE_SLOT_A;
}

static uint32_t GetSlotImageStart(firmware_slot_t slot)
{
    return (slot == FIRMWARE_SLOT_A) ?
           SLOT_A_IMAGE_START :
           SLOT_B_IMAGE_START;
}

static uint32_t GetSlotImageRegionEnd(firmware_slot_t slot)
{
    return (slot == FIRMWARE_SLOT_A) ?
           SLOT_A_IMAGE_REGION_END :
           SLOT_B_IMAGE_REGION_END;
}

static HAL_StatusTypeDef FindConfirmedSlot(
    const firmware_metadata_record_t *slot_a_record,
    const firmware_metadata_record_t *slot_b_record,
    bool slot_a_record_valid,
    bool slot_b_record_valid,
    firmware_slot_t *confirmed_slot)
{
    bool slot_a_confirmed = false;
    bool slot_b_confirmed = false;

    if (confirmed_slot == NULL)
    {
        return HAL_ERROR;
    }

    if (slot_a_record_valid &&
        (slot_a_record->metadata.update_state ==
         FIRMWARE_STATE_CONFIRMED))
    {
        slot_a_confirmed = true;
    }

    if (slot_b_record_valid &&
        (slot_b_record->metadata.update_state ==
         FIRMWARE_STATE_CONFIRMED))
    {
        slot_b_confirmed = true;
    }

    /*
     * Both slots cannot be CONFIRMED simultaneously.
     */
    if (slot_a_confirmed && slot_b_confirmed)
    {
        return HAL_ERROR;
    }

    if (slot_a_confirmed)
    {
        *confirmed_slot = FIRMWARE_SLOT_A;
        return HAL_OK;
    }

    if (slot_b_confirmed)
    {
        *confirmed_slot = FIRMWARE_SLOT_B;
        return HAL_OK;
    }

    /*
     * No confirmed slot is available.
     */
    return HAL_ERROR;
}

/**
 * @brief  Keep the bootloader in a safe state when no valid
 *         firmware image is available.
 * @retval None
 */
static void Bootloader_FailSafe(void)
{
    printf("Bootloader: no valid firmware image\r\n");
    printf("Bootloader Fail Safe code is running\r\n");

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOD, LD4_Pin);
        HAL_Delay(500);
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

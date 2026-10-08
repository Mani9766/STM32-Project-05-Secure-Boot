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

static HAL_StatusTypeDef UpdateSlotState(
    firmware_slot_t slot,
    firmware_metadata_record_t *record,
    firmware_update_state_t new_state);

static HAL_StatusTypeDef ProcessPendingValidation(
    firmware_slot_t pending_slot,
    firmware_metadata_record_t *pending_record,
    const firmware_metadata_record_t *confirmed_record);

static HAL_StatusTypeDef PromoteValidatedFirmware(
    firmware_slot_t slot,
    firmware_metadata_record_t *record);

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

	bool slot_a_pending = false;
	bool slot_b_pending = false;

	bool slot_a_validated = false;
	bool slot_b_validated = false;

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
   * Detect an unfinished trial boot.
   *
   * If a slot is BOOT_PENDING after reset, the previous
   * trial did not confirm, so boot the ROLLBACK slot.
   */
  slot_a_pending =
      slot_a_record_valid &&
      (slot_a_record.metadata.update_state ==
       FIRMWARE_STATE_BOOT_PENDING);

  slot_b_pending =
      slot_b_record_valid &&
      (slot_b_record.metadata.update_state ==
       FIRMWARE_STATE_BOOT_PENDING);

  if (slot_a_pending && slot_b_pending)
  {
      printf("Invalid state: both slots are BOOT_PENDING\r\n");
      Bootloader_FailSafe();
  }

  if (slot_a_pending || slot_b_pending)
  {
      firmware_slot_t pending_slot;
      firmware_slot_t rollback_slot;

      firmware_metadata_record_t *pending_record;
      firmware_metadata_record_t *rollback_record;

      /*
       * Identify the failed trial slot and the rollback slot.
       */
      if (slot_a_pending)
      {
          pending_slot = FIRMWARE_SLOT_A;
          pending_record = &slot_a_record;

          if (slot_b_record_valid &&
              (slot_b_record.metadata.update_state ==
               FIRMWARE_STATE_ROLLBACK))
          {
              rollback_slot = FIRMWARE_SLOT_B;
              rollback_record = &slot_b_record;
          }
          else
          {
              printf("No valid Slot B ROLLBACK image\r\n");
              Bootloader_FailSafe();
          }
      }
      else
      {
          pending_slot = FIRMWARE_SLOT_B;
          pending_record = &slot_b_record;

          if (slot_a_record_valid &&
              (slot_a_record.metadata.update_state ==
               FIRMWARE_STATE_ROLLBACK))
          {
              rollback_slot = FIRMWARE_SLOT_A;
              rollback_record = &slot_a_record;
          }
          else
          {
              printf("No valid Slot A ROLLBACK image\r\n");
              Bootloader_FailSafe();
          }
      }

      printf(
          "BOOT_PENDING detected in Slot %s\r\n",
          (pending_slot == FIRMWARE_SLOT_A) ? "A" : "B");

      printf(
          "Rolling back to Slot %s\r\n",
          (rollback_slot == FIRMWARE_SLOT_A) ? "A" : "B");

      /*
       * The trial firmware did not confirm successfully.
       * Mark it INVALID so it is not retried.
       */
      status = UpdateSlotState(
          pending_slot,
          pending_record,
          FIRMWARE_STATE_INVALID);

      if (status != HAL_OK)
      {
          printf("Failed to mark trial slot as INVALID\r\n");
          Bootloader_FailSafe();
      }

      /*
       * Restore the rollback image as the confirmed firmware.
       */
      status = UpdateSlotState(
          rollback_slot,
          rollback_record,
          FIRMWARE_STATE_CONFIRMED);

      if (status != HAL_OK)
      {
          printf("Failed to restore ROLLBACK slot as CONFIRMED\r\n");
          Bootloader_FailSafe();
      }

      /*
       * The rollback slot is now the firmware to boot.
       */
      active_slot = rollback_slot;

      printf(
          "Rollback complete: Slot %s is now CONFIRMED\r\n",
          (active_slot == FIRMWARE_SLOT_A) ? "A" : "B");
  }
  else
  {
      /*
       * No unfinished trial boot.
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
   * Detect a pending firmware update.
   *
   * At most one slot may be PENDING_VALIDATION.
   */
  slot_a_pending =
      slot_a_record_valid &&
      (slot_a_record.metadata.update_state ==
       FIRMWARE_STATE_PENDING_VALIDATION);

  slot_b_pending =
      slot_b_record_valid &&
      (slot_b_record.metadata.update_state ==
       FIRMWARE_STATE_PENDING_VALIDATION);

  if (slot_a_pending && slot_b_pending)
  {
      printf("Invalid state: both slots are pending validation\r\n");
      Bootloader_FailSafe();
  }

  if (slot_a_pending || slot_b_pending)
  {
      firmware_slot_t pending_slot;

      firmware_metadata_record_t *pending_record;

      const firmware_metadata_record_t *confirmed_record;

      if (slot_a_pending)
      {
          pending_slot = FIRMWARE_SLOT_A;
          pending_record = &slot_a_record;
      }
      else
      {
          pending_slot = FIRMWARE_SLOT_B;
          pending_record = &slot_b_record;
      }

      confirmed_record =
          (active_slot == FIRMWARE_SLOT_A) ?
          &slot_a_record :
          &slot_b_record;

      status = ProcessPendingValidation(
          pending_slot,
          pending_record,
          confirmed_record);

      if (status != HAL_OK)
      {
          printf("Pending firmware validation processing failed\r\n");
          Bootloader_FailSafe();
      }
  }

  /*
   * ---------------------------------------------------------
   * Process VALIDATED firmware here
   * ---------------------------------------------------------
   */

  slot_a_validated =
      slot_a_record_valid &&
      (slot_a_record.metadata.update_state ==
       FIRMWARE_STATE_VALIDATED);

  slot_b_validated =
      slot_b_record_valid &&
      (slot_b_record.metadata.update_state ==
       FIRMWARE_STATE_VALIDATED);

  if (slot_a_validated && slot_b_validated)
  {
      printf("Invalid state: both slots are VALIDATED\r\n");
      Bootloader_FailSafe();
  }

  if (slot_a_validated || slot_b_validated)
  {
      firmware_slot_t validated_slot;
      firmware_metadata_record_t *validated_record;

      if (slot_a_validated)
      {
          validated_slot = FIRMWARE_SLOT_A;
          validated_record = &slot_a_record;
      }
      else
      {
          validated_slot = FIRMWARE_SLOT_B;
          validated_record = &slot_b_record;
      }

      /*
       * A validated firmware must always be the inactive slot.
       */
      if (validated_slot == active_slot)
      {
          printf("Invalid state: active slot is VALIDATED\r\n");
          Bootloader_FailSafe();
      }

      printf(
          "Validated firmware found in Slot %s\r\n",
          (validated_slot == FIRMWARE_SLOT_A) ? "A" : "B");

      status = PromoteValidatedFirmware(
          validated_slot,
          validated_record);

      if (status == HAL_OK)
      {
    	  status = UpdateSlotState(
    	      active_slot,
    	      (active_slot == FIRMWARE_SLOT_A) ?
    	          &slot_a_record :
    	          &slot_b_record,
    	      FIRMWARE_STATE_ROLLBACK);

    	  if (status != HAL_OK)
    	  {
    	      printf("Failed to set rollback state\r\n");
    	      Bootloader_FailSafe();
    	  }

          uint32_t trial_image_start;
          uint32_t trial_image_end;

          trial_image_start =
              GetSlotImageStart(validated_slot);

          trial_image_end =
              trial_image_start +
              validated_record->metadata.image_size;

          printf(
              "Starting trial boot from Slot %s\r\n",
              (validated_slot == FIRMWARE_SLOT_A) ? "A" : "B");

          JumpToApplication(
              trial_image_start,
              trial_image_end);
      }

      printf("Firmware promotion failed\r\n");
  }

  /*
   * ---------------------------------------------------------
   * Normal boot of confirmed firmware
   * ---------------------------------------------------------
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

static HAL_StatusTypeDef UpdateSlotState(
    firmware_slot_t slot,
    firmware_metadata_record_t *record,
    firmware_update_state_t new_state)
{
    firmware_metadata_t new_metadata;
    HAL_StatusTypeDef status;

    if (record == NULL)
    {
        return HAL_ERROR;
    }

    new_metadata = record->metadata;
    new_metadata.update_state = new_state;

    if (slot == FIRMWARE_SLOT_A)
    {
        status = Metadata_WriteRecord(
            SLOT_A_METADATA_ADDRESS,
            SLOT_A_METADATA_REGION_END,
            FLASH_SECTOR_2,
            FLASH_VOLTAGE_RANGE_3,
            &new_metadata);
    }
    else
    {
        status = Metadata_WriteRecord(
            SLOT_B_METADATA_ADDRESS,
            SLOT_B_METADATA_REGION_END,
            FLASH_SECTOR_3,
            FLASH_VOLTAGE_RANGE_3,
            &new_metadata);
    }

    if (status != HAL_OK)
    {
        return status;
    }

    /*
     * Refresh the local record so it represents the newly
     * committed metadata record.
     */
    if (slot == FIRMWARE_SLOT_A)
    {
        if (!Metadata_ReadLatestRecord(
                SLOT_A_METADATA_ADDRESS,
                SLOT_A_METADATA_REGION_END,
                record))
        {
            return HAL_ERROR;
        }
    }
    else
    {
        if (!Metadata_ReadLatestRecord(
                SLOT_B_METADATA_ADDRESS,
                SLOT_B_METADATA_REGION_END,
                record))
        {
            return HAL_ERROR;
        }
    }

    return HAL_OK;
}

static HAL_StatusTypeDef ProcessPendingValidation(
    firmware_slot_t pending_slot,
    firmware_metadata_record_t *pending_record,
    const firmware_metadata_record_t *confirmed_record)
{
    uint8_t staged_digest[SHA256_DIGEST_SIZE];

    if ((pending_record == NULL) ||
        (confirmed_record == NULL))
    {
        return HAL_ERROR;
    }

    printf(
        "Processing pending firmware in Slot %s\r\n",
        (pending_slot == FIRMWARE_SLOT_A) ? "A" : "B");

    /*
     * The staged image must fit inside the download/staging area.
     */
    if (!Metadata_Validate(
            &pending_record->metadata,
            DOWNLOAD_START_ADDRESS,
            DOWNLOAD_REGION_END))
    {
        printf("Invalid staged firmware metadata\r\n");

        return UpdateSlotState(
            pending_slot,
            pending_record,
            FIRMWARE_STATE_INVALID);
    }

    /*
     * Reject firmware that is not newer than the currently
     * confirmed firmware.
     */
    if (pending_record->metadata.version <=
        confirmed_record->metadata.version)
    {
        printf("Staged firmware is not newer\r\n");

        return UpdateSlotState(
            pending_slot,
            pending_record,
            FIRMWARE_STATE_INVALID);
    }

    printf(
        "Staged firmware version: %lu\r\n",
        (unsigned long)pending_record->metadata.version);

    printf("Calculating staged firmware SHA-256\r\n");

    ImageValidation_CalculateSHA256(
        DOWNLOAD_START_ADDRESS,
        pending_record->metadata.image_size,
        staged_digest);

    if (!ImageValidation_VerifySHA256(
            staged_digest,
            pending_record->metadata.sha256))
    {
        printf("Staged firmware SHA-256 mismatch\r\n");

        return UpdateSlotState(
            pending_slot,
            pending_record,
            FIRMWARE_STATE_INVALID);
    }

    printf("Staged firmware SHA-256 matched\r\n");

    /*
     * The firmware is valid and can now be promoted
     * into the inactive slot.
     *
     * Promotion itself is handled by the next task.
     */
    return UpdateSlotState(
        pending_slot,
        pending_record,
        FIRMWARE_STATE_VALIDATED);
}

static HAL_StatusTypeDef PromoteValidatedFirmware(
    firmware_slot_t slot,
    firmware_metadata_record_t *record)
{
    uint32_t destination_start;
    uint32_t destination_region_end;

    uint32_t flash_sector;

    uint8_t destination_digest[SHA256_DIGEST_SIZE];

    HAL_StatusTypeDef status;

    if (record == NULL)
    {
        return HAL_ERROR;
    }

    if (record->metadata.update_state !=
        FIRMWARE_STATE_VALIDATED)
    {
        return HAL_ERROR;
    }

    /*
     * The validated image is still stored in the
     * download/staging region.
     */
    if (!Metadata_Validate(
            &record->metadata,
            DOWNLOAD_START_ADDRESS,
            DOWNLOAD_REGION_END))
    {
        printf("Invalid staged firmware metadata\r\n");
        return HAL_ERROR;
    }

    destination_start =
        GetSlotImageStart(slot);

    destination_region_end =
        GetSlotImageRegionEnd(slot);

    /*
     * The same image must fit into the destination slot.
     */
    if (!Metadata_Validate(
            &record->metadata,
            destination_start,
            destination_region_end))
    {
        printf("Firmware does not fit destination slot\r\n");
        return HAL_ERROR;
    }

    flash_sector =
        (slot == FIRMWARE_SLOT_A) ?
        FLASH_SECTOR_5 :
        FLASH_SECTOR_6;

    printf(
        "Promoting staged firmware to Slot %s\r\n",
        (slot == FIRMWARE_SLOT_A) ? "A" : "B");

    /*
     * Erase the inactive firmware slot.
     */
    printf("Erasing destination slot\r\n");

    status = FlashStorage_EraseSector(
        flash_sector,
        FLASH_VOLTAGE_RANGE_3);

    if (status != HAL_OK)
    {
        printf("Failed to erase destination slot\r\n");
        return status;
    }

    /*
     * Copy firmware from staging area into the
     * selected inactive slot.
     */
    printf("Programming destination slot\r\n");

    status = FlashStorage_ProgramImage(
        destination_start,
        DOWNLOAD_START_ADDRESS,
        record->metadata.image_size);

    if (status != HAL_OK)
    {
        printf("Failed to program destination slot\r\n");

        /*
         * Keep VALIDATED state.
         * The staging image is still available and
         * promotion can be retried after reset.
         */
        return status;
    }

    /*
     * Independently calculate SHA-256 of the image
     * after programming.
     */
    printf("Verifying programmed destination SHA-256\r\n");

    ImageValidation_CalculateSHA256(
        destination_start,
        record->metadata.image_size,
        destination_digest);

    if (!ImageValidation_VerifySHA256(
            destination_digest,
            record->metadata.sha256))
    {
        printf("Destination SHA-256 mismatch\r\n");

        /*
         * Keep VALIDATED state.
         * The failed destination can be erased and
         * programmed again on the next attempt.
         */
        return HAL_ERROR;
    }

    printf("Destination SHA-256 matched\r\n");

    /*
     * Destination image is now programmed and verified.
     * It is ready for trial boot.
     */
    status = UpdateSlotState(
        slot,
        record,
        FIRMWARE_STATE_BOOT_PENDING);

    if (status != HAL_OK)
    {
        printf("Failed to set BOOT_PENDING state\r\n");
        return status;
    }

    printf("Slot %s is now BOOT_PENDING\r\n",
           (slot == FIRMWARE_SLOT_A) ? "A" : "B");

    return HAL_OK;
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

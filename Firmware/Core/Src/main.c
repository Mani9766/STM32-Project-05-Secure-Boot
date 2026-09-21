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
#include <app_image_info.h>
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
#define SRAM_START_ADDRESS                 0x20000000U
#define SRAM_END_ADDRESS                   0x20020000U

#define CANDIDATE_IMAGE_START              0x08010000U
#define CANDIDATE_IMAGE_REGION_END         0x08020000U

#define FLASH_ACTIVE_METADATA_ADDRESS      0x08008000U
#define FLASH_CANDIDATE_METADATA_ADDRESS   0x0800C000U

#define ACTIVE_IMAGE_REGION_END  0x08100000U

//#define UPDATE_SECTOR_2
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
static void JumpToApplication(uint32_t image_start, uint32_t image_end);
static HAL_StatusTypeDef UpdateActiveMetadata(
    const uint8_t *digest) __attribute__((unused));
static void Bootloader_FailSafe(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  firmware_metadata_t active_metadata;
  firmware_metadata_t candidate_metadata;

  HAL_StatusTypeDef status;

  bool candidate_metadata_valid = false;

  uint8_t candidate_digest[SHA256_DIGEST_SIZE];
  uint32_t candidate_image_end;
  uint8_t active_digest[SHA256_DIGEST_SIZE];
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
  /* USER CODE BEGIN 2 */

  #ifdef UPDATE_SECTOR_2

      uint8_t metadata_digest[SHA256_DIGEST_SIZE];

      printf("Updating active metadata in Sector 2\r\n");

      /*
       * Calculate SHA-256 of the current active image.
       */
      ImageValidation_CalculateSHA256(
          APP_IMAGE_START,
          APP_IMAGE_SIZE,
          metadata_digest);

      /*
       * Erase Sector 2 and program active metadata.
       */
      status = UpdateActiveMetadata(metadata_digest);

      if (status != HAL_OK)
      {
          printf("Failed to update active metadata\r\n");
          Bootloader_FailSafe();
      }

      printf("Active metadata updated successfully\r\n");

      /*
       * Stop here.
       * Do not execute the normal bootloader flow.
       */
      while (1)
      {
      }

  #else

      /* Read Active Image Metadata */

      status = FlashStorage_ReadMetadata(
          FLASH_ACTIVE_METADATA_ADDRESS,
          &active_metadata);

      if (status != HAL_OK)
      {
          printf("Failed to read active metadata\r\n");
          Bootloader_FailSafe();
      }

      if (!Metadata_Validate(&active_metadata,
                             APP_IMAGE_START,
                             ACTIVE_IMAGE_REGION_END))
      {
          printf("Invalid Active Image Metadata\r\n");
          Bootloader_FailSafe();
      }


      /* Read Candidate Image Metadata */

      status = FlashStorage_ReadMetadata(
          FLASH_CANDIDATE_METADATA_ADDRESS,
          &candidate_metadata);

      if (status != HAL_OK)
      {
          printf("Failed to read candidate metadata\r\n");
          candidate_metadata_valid = false;
      }
      else
      {
          candidate_metadata_valid =
              Metadata_Validate(&candidate_metadata,
                                CANDIDATE_IMAGE_START,
                                CANDIDATE_IMAGE_REGION_END);

          if (!candidate_metadata_valid)
          {
              printf("Invalid Candidate Image Metadata\r\n");
          }
      }


      /* Handle Candidate Image */

      if (candidate_metadata_valid &&
          Metadata_IsCandidateNewer(&active_metadata,
                                    &candidate_metadata))
      {
          printf("New candidate firmware detected\r\n");

          candidate_image_end =
              CANDIDATE_IMAGE_START +
              candidate_metadata.image_size;

          printf("Calculating candidate SHA-256\r\n");

          ImageValidation_CalculateSHA256(
              CANDIDATE_IMAGE_START,
              candidate_metadata.image_size,
              candidate_digest);

          if (ImageValidation_VerifySHA256(
                  candidate_digest,
                  candidate_metadata.sha256))
          {
              printf("Candidate SHA-256 matched\r\n");
              printf("Booting candidate image\r\n");

              JumpToApplication(
                  CANDIDATE_IMAGE_START,
                  candidate_image_end);
          }
          else
          {
              printf("Candidate SHA-256 mismatch\r\n");
              printf("Candidate image rejected\r\n");
          }
      }


      /* Verify Active Image and use it as fallback */

      {
          uint32_t active_image_end =
              APP_IMAGE_START + active_metadata.image_size;

          printf("Calculating active SHA-256\r\n");

          ImageValidation_CalculateSHA256(
              APP_IMAGE_START,
              active_metadata.image_size,
              active_digest);

          if (ImageValidation_VerifySHA256(
                  active_digest,
                  active_metadata.sha256))
          {
              printf("Active SHA-256 matched\r\n");

              JumpToApplication(
                  APP_IMAGE_START,
                  active_image_end);
          }

          printf("Active SHA-256 mismatch\r\n");
          Bootloader_FailSafe();
      }

  #endif

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

/**
 * @brief  Update Active Image metadata in Sector 2.
 * @param  digest SHA-256 digest of the Active Image.
 * @retval HAL_OK if metadata was erased and programmed successfully.
 */
static HAL_StatusTypeDef UpdateActiveMetadata(
    const uint8_t *digest)
{
    firmware_metadata_t metadata;
    HAL_StatusTypeDef status;

    if (digest == NULL)
    {
        return HAL_ERROR;
    }

    metadata.magic = FIRMWARE_METADATA_MAGIC;
    metadata.image_size = APP_IMAGE_SIZE;
    metadata.version = 0U;

    memcpy(metadata.sha256,
           digest,
           SHA256_DIGEST_SIZE);

    metadata.update_state = 2U;

    status = FlashStorage_EraseSector(
        FLASH_SECTOR_2,
        FLASH_VOLTAGE_RANGE_3);

    if (status != HAL_OK)
    {
        return status;
    }

    return FlashStorage_ProgramMetadata(
        FLASH_ACTIVE_METADATA_ADDRESS,
        &metadata);
}

/**
 * @brief  Keep the bootloader in a safe state when no valid
 *         firmware image is available.
 * @retval None
 */
static void Bootloader_FailSafe(void)
{
    printf("Bootloader: no valid firmware image\r\n");

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

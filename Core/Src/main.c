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
#include "cmsis_os.h"
#include "dac.h"
#include "fdcan.h"
#include "i2c.h"
#include "i2s.h"
#include "iwdg.h"
#include "ramecc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "aic3104.h"
#include "adf7021.h"
#include "ADF4360.h"
#include "source.h"
#include "stm32h7xx_hal_gpio.h"
#include <stdbool.h>
#include <sys/_intsup.h>
#include <stdio.h>

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

/* USER CODE BEGIN PV */
#define AUDIO_SAMPLE_RATE_HZ      48000.0f
/* Choose frame length N so N*f/Fs is integer for seamless loop */
/* eg: Fs=48k, N=240 and f=1k gives 5 integer cycles per frame */
#define AUDIO_FRAME_SAMPLES       240U

static AIC3104_Config_t g_aic3104_cfg;
static float g_iq_audio_buffer[AUDIO_FRAME_SAMPLES * 2U];
static int16_t g_i2s_tx_stereo[AUDIO_FRAME_SAMPLES * 2U];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static void PrepareIqAudioFrame(void);
static void DumpAic3104Regs(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;

  if (ch == '\n') {
    uint8_t cr = '\r';
    (void)HAL_UART_Transmit(&huart1, &cr, 1U, 100U);
  }

  (void)HAL_UART_Transmit(&huart1, &c, 1U, 100U);
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

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_FDCAN1_Init();
  MX_I2S1_Init();
  MX_SPI4_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_DAC1_Init();
  MX_I2C1_Init();
  MX_IWDG1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_RAMECC_Init();
  /* USER CODE BEGIN 2 */
  /* Initialize ADF7021 transceiver */
  // ADF7021_Config_t adf7021_cfg = {
  //     .hspi = &hspi2,                      /* SPI2 for register access */
  //     .en_port = ADF7021_EN_GPIO_Port,     /* EN (CE) pin on GPIOD */
  //     .en_pin = ADF7021_EN_Pin,            /* GPIO pin for enable */
      
  //     .xtal_freq_hz = 16000000,            /* 16 MHz TCXO */
  //     .xtal_type = 1,                      /* 1 = External TCXO (not internal crystal) */
      
  //     .center_freq_hz = 144000000,         /* 144 MHz center frequency (adjust as needed) */
  //     .ref_freq_hz = 16000000,             /* Reference frequency = XTAL */
      
  //     .data_rate_bps = 1200,               /* 1200 bps data rate */
      
  //     .mod_type = ADF7021_MOD_2FSK,        /* 2-FSK modulation */
  //     .freq_deviation = 2400,              /* 2.4 kHz frequency deviation */
      
  //     .tx_power = ADF7021_PA_POWER_0dBm,  /* 0 dBm TX power */
      
  //     .if_filter_bw = 12500                /* 12.5 kHz IF filter bandwidth */
  // };
  
  // if (ADF7021_Init(&adf7021_cfg) != HAL_OK) {
  //     Error_Handler();
  // }

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 9;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
static void PrepareIqAudioFrame(void)
{
  uint32_t i;

  sine_source(g_iq_audio_buffer,
              AUDIO_FRAME_SAMPLES,
              1000.0f,
              AUDIO_SAMPLE_RATE_HZ,
              0.8f);

  for (i = 0U; i < AUDIO_FRAME_SAMPLES; i++) {
    int16_t sample_i = (int16_t)(g_iq_audio_buffer[2U * i] * 32767.0f);
    int16_t sample_q = (int16_t)(g_iq_audio_buffer[(2U * i) + 1U] * 32767.0f);
    g_i2s_tx_stereo[(2U * i)] = sample_i;
    g_i2s_tx_stereo[(2U * i) + 1U] = sample_q;
  }
}

static void DumpAic3104Regs(void)
{
  const uint8_t regs[] = {
    AIC3104_REG_DAC_PWR,
    AIC3104_REG_DACL1_TO_LLOPM,
    AIC3104_REG_DACR1_TO_RLOPM,
    AIC3104_REG_LLOPM_CTRL,
    AIC3104_REG_RLOPM_CTRL
  };
  char msg[64];

  for (uint32_t i = 0U; i < (uint32_t)(sizeof(regs) / sizeof(regs[0])); i++) {
    uint8_t value = 0U;
    int len;

    if (AIC3104_ReadReg(&g_aic3104_cfg, regs[i], &value) == HAL_OK) {
      len = snprintf(msg, sizeof(msg), "AIC3104 R0x%02X = 0x%02X\r\n", regs[i], value);
    } else {
      len = snprintf(msg, sizeof(msg), "AIC3104 R0x%02X read fail\r\n", regs[i]);
    }

    if (len > 0) {
      uint16_t tx_len = (len < (int)sizeof(msg)) ? (uint16_t)len : (uint16_t)(sizeof(msg) - 1U);
      (void)HAL_UART_Transmit(&huart1, (uint8_t *)msg, tx_len, 100U);
    }
  }
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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

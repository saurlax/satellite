/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>

#include "ADF4360.h"
#include "adf7021.h"
#include "aic3104.h"
#include "i2c.h"
#include "i2s.h"
#include "iwdg.h"
#include "ltc5599.h"
#include "spi.h"
#include "source.h"
#include "usart.h"

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
/* USER CODE BEGIN Variables */
#define AUDIO_SAMPLE_RATE_HZ      48000.0f
/* Choose frame length N so N*f/Fs is integer for seamless loop */
/* eg: Fs=48k, N=240 and f=1k gives 5 integer cycles per frame */
#define AUDIO_FRAME_SAMPLES       240U

static AIC3104_Config_t g_aic3104_cfg;
static ADF7021_Config_t g_adf7021_cfg;
static LTC5599_Config_t g_ltc5599_cfg;
static float g_iq_audio_buffer[AUDIO_FRAME_SAMPLES * 2U];
static int16_t g_i2s_tx_stereo[AUDIO_FRAME_SAMPLES * 2U];

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId watchdogTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void PrepareIqAudioFrame(void);
static void DumpAic3104Regs(void);
static void RunRfSmokeTest(void);
static void UartPrintf(const char *fmt, ...);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartWatchdogTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of watchdogTask */
  osThreadDef(watchdogTask, StartWatchdogTask, osPriorityAboveNormal, 0, 128);
  watchdogTaskHandle = osThreadCreate(osThread(watchdogTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  (void)argument;
  printf("printf via UART1 is ready\r\n");
  RunRfSmokeTest();

  /* Initialize ADF7021 transceiver before codec setup */
  ADF7021_DefaultConfig(&g_adf7021_cfg, &hspi2);
  g_adf7021_cfg.ce_port = ADF7021_EN_GPIO_Port;
  g_adf7021_cfg.ce_pin = ADF7021_EN_Pin;
  g_adf7021_cfg.rx_freq_hz = 145000000UL;
  g_adf7021_cfg.tx_freq_hz = 436500000UL;

  if (ADF7021_Init(&g_adf7021_cfg) != HAL_OK) {
    Error_Handler();
  }

  /* Initialize AIC3104 codec */
  AIC3104_DefaultConfig(&g_aic3104_cfg, &hi2c1);
  g_aic3104_cfg.hi2s = &hi2s1;
  g_aic3104_cfg.reset_port = AIC3104_RST_GPIO_Port;
  g_aic3104_cfg.reset_pin = AIC3104_RST_Pin;
  g_aic3104_cfg.sample_rate = AIC3104_FS_48K;
  g_aic3104_cfg.i2s_mode = AIC3104_MODE_SLAVE;
  g_aic3104_cfg.enable_adc = false;
  g_aic3104_cfg.enable_dac = true;
  g_aic3104_cfg.enable_hp = true;
  g_aic3104_cfg.enable_lineout = true;
  g_aic3104_cfg.enable_hpcom = true;
  g_aic3104_cfg.output_common_mode = 0U;

  if (AIC3104_Init(&g_aic3104_cfg) != HAL_OK) {
    Error_Handler();
  }

  DumpAic3104Regs();
  PrepareIqAudioFrame();

  if (HAL_I2S_Transmit_DMA(&hi2s1, (uint16_t *)g_i2s_tx_stereo, AUDIO_FRAME_SAMPLES * 2U) != HAL_OK) {
    Error_Handler();
  }

  /* Infinite loop */
  for(;;)
  {
    osDelay(1000U);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartWatchdogTask */
/**
* @brief Function implementing the watchdogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartWatchdogTask */
void StartWatchdogTask(void const * argument)
{
  /* USER CODE BEGIN StartWatchdogTask */
  (void)argument;

  for(;;)
  {
    HAL_GPIO_TogglePin(WDI_GPIO_Port, WDI_Pin);
    HAL_IWDG_Refresh(&hiwdg1);
    osDelay(100U);
  }
  /* USER CODE END StartWatchdogTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
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
    AIC3104_REG_HPOUT_SC,
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

static void RunRfSmokeTest(void)
{
  uint64_t actual_hz = 0ULL;
  uint8_t reg0 = 0U;

  HAL_GPIO_WritePin(TXPLL__CE_GPIO_Port, TXPLL__CE_Pin, GPIO_PIN_SET);

  if (ADF4360_Init(ADF4360_7)) {
    actual_hz = ADF4360_SetFrequency(436500000ULL);
    UartPrintf("ADF4360-7 set %lu Hz, LD=%u\r\n",
               (unsigned long)actual_hz,
               (unsigned int)ADF4360_DefaultLockDetect());
  } else {
    UartPrintf("ADF4360-7 init fail\r\n");
  }

  LTC5599_DefaultConfig(&g_ltc5599_cfg, &hspi4);
  g_ltc5599_cfg.cs_port = LTC5599_CS_GPIO_Port;
  g_ltc5599_cfg.cs_pin = LTC5599_CS_Pin;

  if (LTC5599_Probe(&g_ltc5599_cfg, &reg0) == HAL_OK) {
    UartPrintf("LTC5599 reg0 = 0x%02X\r\n", reg0);
  } else {
    UartPrintf("LTC5599 probe fail\r\n");
  }
}

static void UartPrintf(const char *fmt, ...)
{
  char msg[96];
  va_list ap;
  int len;

  va_start(ap, fmt);
  len = vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);

  if (len > 0) {
    uint16_t tx_len = (len < (int)sizeof(msg)) ? (uint16_t)len : (uint16_t)(sizeof(msg) - 1U);
    (void)HAL_UART_Transmit(&huart1, (uint8_t *)msg, tx_len, 100U);
  }
}


/* USER CODE END Application */

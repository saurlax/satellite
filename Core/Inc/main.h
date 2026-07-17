/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TXPLL_LE_Pin GPIO_PIN_0
#define TXPLL_LE_GPIO_Port GPIOC
#define TXPLL_LD_Pin GPIO_PIN_3
#define TXPLL_LD_GPIO_Port GPIOC
#define TEMP_Pin GPIO_PIN_0
#define TEMP_GPIO_Port GPIOA
#define TXPLL_CE_Pin GPIO_PIN_1
#define TXPLL_CE_GPIO_Port GPIOA
#define LTC5599_EN_Pin GPIO_PIN_2
#define LTC5599_EN_GPIO_Port GPIOA
#define LTC5599_TTCK_Pin GPIO_PIN_3
#define LTC5599_TTCK_GPIO_Port GPIOA
#define LTC5599_CS_Pin GPIO_PIN_4
#define LTC5599_CS_GPIO_Port GPIOA
#define AIC3104_DPWR_Pin GPIO_PIN_6
#define AIC3104_DPWR_GPIO_Port GPIOA
#define ADF7021_TESTA_Pin GPIO_PIN_0
#define ADF7021_TESTA_GPIO_Port GPIOB
#define FAULT3V3_Pin GPIO_PIN_7
#define FAULT3V3_GPIO_Port GPIOE
#define IO00_Pin GPIO_PIN_9
#define IO00_GPIO_Port GPIOE
#define IO01_Pin GPIO_PIN_10
#define IO01_GPIO_Port GPIOE
#define IO03_Pin GPIO_PIN_11
#define IO03_GPIO_Port GPIOE
#define IO04_Pin GPIO_PIN_12
#define IO04_GPIO_Port GPIOE
#define IO06_Pin GPIO_PIN_13
#define IO06_GPIO_Port GPIOE
#define AIC3104_RST_Pin GPIO_PIN_13
#define AIC3104_RST_GPIO_Port GPIOD
#define TCAN_SHDN2_Pin GPIO_PIN_15
#define TCAN_SHDN2_GPIO_Port GPIOD
#define PA_EN_Pin GPIO_PIN_7
#define PA_EN_GPIO_Port GPIOC
#define TCAN_SHDN1_Pin GPIO_PIN_9
#define TCAN_SHDN1_GPIO_Port GPIOC
#define KHM_POW_ON_Pin GPIO_PIN_0
#define KHM_POW_ON_GPIO_Port GPIOD
#define ADF7021_MUXOUT_Pin GPIO_PIN_1
#define ADF7021_MUXOUT_GPIO_Port GPIOD
#define WDI_Pin GPIO_PIN_2
#define WDI_GPIO_Port GPIOD
#define ADF7021_SWD_Pin GPIO_PIN_3
#define ADF7021_SWD_GPIO_Port GPIOD
#define ADF7021_EN_Pin GPIO_PIN_4
#define ADF7021_EN_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

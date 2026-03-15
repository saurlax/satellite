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
#define TXPLL__EN_Pin GPIO_PIN_0
#define TXPLL__EN_GPIO_Port GPIOC
#define TXPLL_LD_Pin GPIO_PIN_3
#define TXPLL_LD_GPIO_Port GPIOC
#define TEMP_Pin GPIO_PIN_0
#define TEMP_GPIO_Port GPIOA
#define TXPLL__CE_Pin GPIO_PIN_1
#define TXPLL__CE_GPIO_Port GPIOA
#define FAULT3V3_Pin GPIO_PIN_7
#define FAULT3V3_GPIO_Port GPIOE
#define TXPLL__ENE13_Pin GPIO_PIN_13
#define TXPLL__ENE13_GPIO_Port GPIOE
#define AIC3104_RST_Pin GPIO_PIN_13
#define AIC3104_RST_GPIO_Port GPIOD
#define PA_EN_Pin GPIO_PIN_7
#define PA_EN_GPIO_Port GPIOC
#define ADF7021_SWD_Pin GPIO_PIN_0
#define ADF7021_SWD_GPIO_Port GPIOD
#define ADF7021_EN_Pin GPIO_PIN_1
#define ADF7021_EN_GPIO_Port GPIOD
#define GPIO4_Pin GPIO_PIN_3
#define GPIO4_GPIO_Port GPIOD
#define GPIO5_Pin GPIO_PIN_4
#define GPIO5_GPIO_Port GPIOD
#define WDI_Pin GPIO_PIN_5
#define WDI_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

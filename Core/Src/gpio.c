/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PC13   ------> PWR_WKUP4
     PC14-OSC32_IN (OSC32_IN)   ------> RCC_OSC32_IN
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PA13 (JTMS/SWDIO)   ------> DEBUG_JTMS-SWDIO
     PA14 (JTCK/SWCLK)   ------> DEBUG_JTCK-SWCLK
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TXPLL_LE_GPIO_Port, TXPLL_LE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TXPLL_CE_Pin|LTC5599_EN_Pin|LTC5599_TTCK_Pin|LTC5599_CS_Pin
                          |AIC3104_DPWR_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ADF7021_TESTA_GPIO_Port, ADF7021_TESTA_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, IO04_Pin|IO06_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, AIC3104_RST_Pin|TCAN_SHDN2_Pin|KHM_POW_ON_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, PA_EN_Pin|TCAN_SHDN1_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, WDI_Pin|ADF7021_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : TXPLL_LE_Pin PA_EN_Pin TCAN_SHDN1_Pin */
  GPIO_InitStruct.Pin = TXPLL_LE_Pin|PA_EN_Pin|TCAN_SHDN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : TXPLL_LD_Pin */
  GPIO_InitStruct.Pin = TXPLL_LD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TXPLL_LD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TEMP_Pin */
  GPIO_InitStruct.Pin = TEMP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TEMP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TXPLL_CE_Pin LTC5599_EN_Pin LTC5599_TTCK_Pin LTC5599_CS_Pin
                           AIC3104_DPWR_Pin */
  GPIO_InitStruct.Pin = TXPLL_CE_Pin|LTC5599_EN_Pin|LTC5599_TTCK_Pin|LTC5599_CS_Pin
                          |AIC3104_DPWR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ADF7021_TESTA_Pin */
  GPIO_InitStruct.Pin = ADF7021_TESTA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ADF7021_TESTA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : FAULT3V3_Pin IO00_Pin IO01_Pin IO03_Pin */
  GPIO_InitStruct.Pin = FAULT3V3_Pin|IO00_Pin|IO01_Pin|IO03_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : IO04_Pin IO06_Pin */
  GPIO_InitStruct.Pin = IO04_Pin|IO06_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : AIC3104_RST_Pin TCAN_SHDN2_Pin KHM_POW_ON_Pin WDI_Pin
                           ADF7021_EN_Pin */
  GPIO_InitStruct.Pin = AIC3104_RST_Pin|TCAN_SHDN2_Pin|KHM_POW_ON_Pin|WDI_Pin
                          |ADF7021_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : ADF7021_MUXOUT_Pin */
  GPIO_InitStruct.Pin = ADF7021_MUXOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADF7021_MUXOUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ADF7021_SWD_Pin */
  GPIO_InitStruct.Pin = ADF7021_SWD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADF7021_SWD_GPIO_Port, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_CLOSE);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

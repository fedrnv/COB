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
#define COB_LED_ERROR_PORT      GPIOG
#define COB_LED_ERROR_PIN       GPIO_PIN_10
#define COB_LED_EXCHANGE_PORT   GPIOG
#define COB_LED_EXCHANGE_PIN    GPIO_PIN_0
#define COB_LED_MODE6_PORT      GPIOA
#define COB_LED_MODE6_PIN       GPIO_PIN_5
#define COB_LED_MODE7_PORT      GPIOA
#define COB_LED_MODE7_PIN       GPIO_PIN_7
#define COB_LED_ON              GPIO_PIN_RESET
#define COB_LED_OFF             GPIO_PIN_SET

static void COB_StatusLED_Write(GPIO_PinState error, GPIO_PinState exchange,
                                GPIO_PinState mode6, GPIO_PinState mode7);

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPION_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin : PN12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_XSPIM_P2;
  HAL_GPIO_Init(GPION, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void COB_StatusLED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin = COB_LED_ERROR_PIN | COB_LED_EXCHANGE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = COB_LED_MODE6_PIN | COB_LED_MODE7_PIN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  COB_StatusLED_Write(COB_LED_OFF, COB_LED_OFF, COB_LED_OFF, COB_LED_OFF);
}

void COB_StatusLED_EthernetStarting(void)
{
  COB_StatusLED_ErrorSet(false);
  COB_StatusLED_ExchangeSet(false);
}

void COB_StatusLED_EthernetReady(void)
{
  COB_StatusLED_ErrorSet(false);
  COB_StatusLED_ExchangeSet(true);
}

void COB_StatusLED_EthernetError(void)
{
  COB_StatusLED_ErrorSet(true);
  COB_StatusLED_ExchangeSet(false);
}

void COB_StatusLED_ErrorSet(bool active)
{
  HAL_GPIO_WritePin(COB_LED_ERROR_PORT, COB_LED_ERROR_PIN, active ? COB_LED_ON : COB_LED_OFF);
}

void COB_StatusLED_ExchangeSet(bool active)
{
  HAL_GPIO_WritePin(COB_LED_EXCHANGE_PORT, COB_LED_EXCHANGE_PIN, active ? COB_LED_ON : COB_LED_OFF);
}

void COB_StatusLED_BlueToggle(void)
{
  HAL_GPIO_TogglePin(COB_LED_MODE6_PORT, COB_LED_MODE6_PIN);
}

void COB_StatusLED_RedToggle(void)
{
  HAL_GPIO_TogglePin(COB_LED_ERROR_PORT, COB_LED_ERROR_PIN);
}

void COB_StatusLED_TestPassToggle(void)
{
  COB_StatusLED_ErrorSet(false);
  COB_StatusLED_ExchangeSet(false);
  COB_StatusLED_BlueToggle();
}

void COB_StatusLED_TestFailToggle(void)
{
  HAL_GPIO_WritePin(COB_LED_MODE6_PORT, COB_LED_MODE6_PIN, COB_LED_OFF);
  COB_StatusLED_ExchangeSet(false);
  COB_StatusLED_RedToggle();
}

static void COB_StatusLED_Write(GPIO_PinState error, GPIO_PinState exchange,
                                GPIO_PinState mode6, GPIO_PinState mode7)
{
  HAL_GPIO_WritePin(COB_LED_ERROR_PORT, COB_LED_ERROR_PIN, error);
  HAL_GPIO_WritePin(COB_LED_EXCHANGE_PORT, COB_LED_EXCHANGE_PIN, exchange);
  HAL_GPIO_WritePin(COB_LED_MODE6_PORT, COB_LED_MODE6_PIN, mode6);
  HAL_GPIO_WritePin(COB_LED_MODE7_PORT, COB_LED_MODE7_PIN, mode7);
}

/* USER CODE END 2 */

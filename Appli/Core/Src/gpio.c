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
#define COB_LED_BLUE_PORT       GPIOG
#define COB_LED_BLUE_PIN        GPIO_PIN_8
#define COB_LED_RED_PORT        GPIOG
#define COB_LED_RED_PIN         GPIO_PIN_10
#define COB_LED_GREEN_PORT      GPIOG
#define COB_LED_GREEN_PIN       GPIO_PIN_0
#define COB_LED_ON              GPIO_PIN_RESET
#define COB_LED_OFF             GPIO_PIN_SET
#define COB_PWR_EN_PORT         GPIOD
#define COB_PWR_EN_PIN          GPIO_PIN_10
#define COB_PWR_STARTUP_LOOPS   2000000U

static void COB_StatusLED_Write(GPIO_PinState blue, GPIO_PinState red, GPIO_PinState green);
static void COB_BusyWait(uint32_t loops);

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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

}

/* USER CODE BEGIN 2 */
void COB_BoardPower_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitStruct.Pin = COB_PWR_EN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(COB_PWR_EN_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(COB_PWR_EN_PORT, COB_PWR_EN_PIN, GPIO_PIN_SET);
  COB_BusyWait(COB_PWR_STARTUP_LOOPS);
}

void COB_StatusLED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOG_CLK_ENABLE();

  GPIO_InitStruct.Pin = COB_LED_BLUE_PIN | COB_LED_RED_PIN | COB_LED_GREEN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  COB_StatusLED_Write(COB_LED_OFF, COB_LED_OFF, COB_LED_OFF);
}

void COB_StatusLED_EthernetStarting(void)
{
  COB_StatusLED_Write(COB_LED_OFF, COB_LED_ON, COB_LED_ON);
}

void COB_StatusLED_EthernetReady(void)
{
  COB_StatusLED_Write(COB_LED_ON, COB_LED_OFF, COB_LED_OFF);
}

void COB_StatusLED_EthernetError(void)
{
  COB_StatusLED_Write(COB_LED_OFF, COB_LED_ON, COB_LED_OFF);
}

void COB_StatusLED_BlueToggle(void)
{
  HAL_GPIO_TogglePin(COB_LED_BLUE_PORT, COB_LED_BLUE_PIN);
}

static void COB_StatusLED_Write(GPIO_PinState blue, GPIO_PinState red, GPIO_PinState green)
{
  HAL_GPIO_WritePin(COB_LED_BLUE_PORT, COB_LED_BLUE_PIN, blue);
  HAL_GPIO_WritePin(COB_LED_RED_PORT, COB_LED_RED_PIN, red);
  HAL_GPIO_WritePin(COB_LED_GREEN_PORT, COB_LED_GREEN_PIN, green);
}

static void COB_BusyWait(uint32_t loops)
{
  while (loops > 0U)
  {
    __NOP();
    loops--;
  }
}

/* USER CODE END 2 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    pssi.c
  * @brief   This file provides code for the configuration
  *          of the PSSI instances.
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
#include "pssi.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

PSSI_HandleTypeDef hpssi;

/* PSSI init function */
void MX_PSSI_Init(void)
{

  /* USER CODE BEGIN PSSI_Init 0 */

  /* USER CODE END PSSI_Init 0 */

  /* USER CODE BEGIN PSSI_Init 1 */

  /* USER CODE END PSSI_Init 1 */
  hpssi.Instance = PSSI;
  hpssi.Init.DataWidth = HAL_PSSI_16BITS;
  hpssi.Init.BusWidth = HAL_PSSI_16LINES;
  hpssi.Init.ControlSignal = HAL_PSSI_DE_RDY_DISABLE;
  hpssi.Init.ClockPolarity = HAL_PSSI_FALLING_EDGE;
  hpssi.Init.DataEnablePolarity = HAL_PSSI_DEPOL_ACTIVE_LOW;
  hpssi.Init.ReadyPolarity = HAL_PSSI_RDYPOL_ACTIVE_LOW;
  if (HAL_PSSI_Init(&hpssi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN PSSI_Init 2 */

  /* USER CODE END PSSI_Init 2 */

}

void HAL_PSSI_MspInit(PSSI_HandleTypeDef* pssiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(pssiHandle->Instance==PSSI)
  {
  /* USER CODE BEGIN PSSI_MspInit 0 */

  /* USER CODE END PSSI_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_PSSI;
    PeriphClkInitStruct.PssiClockSelection = RCC_PSSICLKSOURCE_HCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* PSSI clock enable */
    __HAL_RCC_DCMI_PSSI_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**PSSI GPIO Configuration
    PC10     ------> PSSI_D14
    PE8     ------> PSSI_D4
    PC6     ------> PSSI_D1
    PC5     ------> PSSI_D2
    PD4     ------> PSSI_D9
    PE10     ------> PSSI_D3
    PB14     ------> PSSI_D10
    PC0     ------> PSSI_DE
    PD12     ------> PSSI_D12
    PD13     ------> PSSI_D13
    PE4     ------> PSSI_D5
    PD5     ------> PSSI_PDCK
    PB8     ------> PSSI_RDY
    PH2     ------> PSSI_D11
    PD7     ------> PSSI_D0
    PE1     ------> PSSI_D8
    PD11     ------> PSSI_D15
    PF1     ------> PSSI_D7
    PG2     ------> PSSI_D6
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_4|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_5
                          |GPIO_PIN_7|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_PSSI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* USER CODE BEGIN PSSI_MspInit 1 */

  /* USER CODE END PSSI_MspInit 1 */
  }
}

void HAL_PSSI_MspDeInit(PSSI_HandleTypeDef* pssiHandle)
{

  if(pssiHandle->Instance==PSSI)
  {
  /* USER CODE BEGIN PSSI_MspDeInit 0 */

  /* USER CODE END PSSI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DCMI_PSSI_CLK_DISABLE();

    /**PSSI GPIO Configuration
    PC10     ------> PSSI_D14
    PE8     ------> PSSI_D4
    PC6     ------> PSSI_D1
    PC5     ------> PSSI_D2
    PD4     ------> PSSI_D9
    PE10     ------> PSSI_D3
    PB14     ------> PSSI_D10
    PC0     ------> PSSI_DE
    PD12     ------> PSSI_D12
    PD13     ------> PSSI_D13
    PE4     ------> PSSI_D5
    PD5     ------> PSSI_PDCK
    PB8     ------> PSSI_RDY
    PH2     ------> PSSI_D11
    PD7     ------> PSSI_D0
    PE1     ------> PSSI_D8
    PD11     ------> PSSI_D15
    PF1     ------> PSSI_D7
    PG2     ------> PSSI_D6
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_4|GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_4|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_5
                          |GPIO_PIN_7|GPIO_PIN_11);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14|GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_2);

  /* USER CODE BEGIN PSSI_MspDeInit 1 */

  /* USER CODE END PSSI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */


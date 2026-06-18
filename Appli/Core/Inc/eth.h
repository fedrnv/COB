/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.h
  * @brief   This file contains all the function prototypes for
  *          the eth.c file
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
#ifndef __ETH_H__
#define __ETH_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ETH_HandleTypeDef heth1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* USER CODE BEGIN EV */
extern volatile uint32_t COB_ETH_DebugStage;
extern volatile uint32_t COB_ETH_LastErrorCode;
extern volatile uint32_t COB_ETH_LastDMAMR;
extern volatile uint32_t COB_ETH_MACCR;
extern volatile uint32_t COB_ETH_MACPFR;
extern volatile uint32_t COB_ETH_MACA0HR;
extern volatile uint32_t COB_ETH_MACA0LR;
extern volatile uint32_t COB_ETH_DMAISR;
extern volatile uint32_t COB_ETH_DMADSR;
extern volatile uint32_t COB_ETH_DMACCR;
extern volatile uint32_t COB_ETH_DMACTXCR;
extern volatile uint32_t COB_ETH_DMACRXCR;
extern volatile uint32_t COB_ETH_DMACSR;
extern volatile uint32_t COB_ETH_DMACMFCR;
extern volatile uint32_t COB_ETH_MMCTPCGR;
extern volatile uint32_t COB_ETH_MMCRUPGR;
extern volatile uint32_t COB_ETH_MMCRCRCEPR;
extern volatile uint32_t COB_ETH_MMCRAEPR;
extern volatile uint32_t COB_ETH_PhyAddr;
extern volatile uint32_t COB_ETH_PhyReadStatus;
extern volatile uint32_t COB_ETH_PHY_BCR;
extern volatile uint32_t COB_ETH_PHY_BSR;
extern volatile uint32_t COB_ETH_PHY_PHYSCSR;
/* USER CODE END EV */

void MX_ETH1_Init(void);

/* USER CODE BEGIN Prototypes */
void COB_ETH_UpdateDebugSnapshot(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ETH_H__ */

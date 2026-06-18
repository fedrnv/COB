/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    eth.c
  * @brief   This file provides code for the configuration
  *          of the ETH instances.
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
#include "eth.h"
#include "string.h"
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "lan8742.h"
/* USER CODE END Includes */

#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#pragma location=0x341F8000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_DMA_RX_CH_CNT][ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x341F80C0
ETH_DMADescTypeDef  DMATxDscrTab[ETH_DMA_TX_CH_CNT][ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x341F8000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_DMA_RX_CH_CNT][ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x341F80C0))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_DMA_TX_CH_CNT][ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_DMA_RX_CH_CNT][ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_DMA_TX_CH_CNT][ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */

#endif
ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT * 2U];
ETH_TxPacketConfig TxConfig;

/* USER CODE BEGIN 0 */
#define COB_ETH_PHY_MAX_ADDR 31U

volatile uint32_t COB_ETH_DebugStage = 0U;
volatile uint32_t COB_ETH_LastErrorCode = 0U;
volatile uint32_t COB_ETH_LastDMAMR = 0U;
volatile uint32_t COB_ETH_MACCR = 0U;
volatile uint32_t COB_ETH_MACPFR = 0U;
volatile uint32_t COB_ETH_MACISR = 0U;
volatile uint32_t COB_ETH_MACRXTXSR = 0U;
volatile uint32_t COB_ETH_MACDR = 0U;
volatile uint32_t COB_ETH_MACA0HR = 0U;
volatile uint32_t COB_ETH_MACA0LR = 0U;
volatile uint32_t COB_ETH_DMAISR = 0U;
volatile uint32_t COB_ETH_DMADSR = 0U;
volatile uint32_t COB_ETH_MTLISR = 0U;
volatile uint32_t COB_ETH_MTLTXQ0OMR = 0U;
volatile uint32_t COB_ETH_MTLTXQ0DR = 0U;
volatile uint32_t COB_ETH_MTLQ0ICSR = 0U;
volatile uint32_t COB_ETH_MTLRXQ0OMR = 0U;
volatile uint32_t COB_ETH_MTLRXQ0DR = 0U;
volatile uint32_t COB_ETH_MTLTXQ1OMR = 0U;
volatile uint32_t COB_ETH_MTLTXQ1DR = 0U;
volatile uint32_t COB_ETH_MTLQ1ICSR = 0U;
volatile uint32_t COB_ETH_MTLRXQ1OMR = 0U;
volatile uint32_t COB_ETH_MTLRXQ1DR = 0U;
volatile uint32_t COB_ETH_DMACCR = 0U;
volatile uint32_t COB_ETH_DMACTXCR = 0U;
volatile uint32_t COB_ETH_DMACRXCR = 0U;
volatile uint32_t COB_ETH_DMACTXDLAR = 0U;
volatile uint32_t COB_ETH_DMACRXDLAR = 0U;
volatile uint32_t COB_ETH_DMACTXDTPR = 0U;
volatile uint32_t COB_ETH_DMACRXDTPR = 0U;
volatile uint32_t COB_ETH_DMACCATXDR = 0U;
volatile uint32_t COB_ETH_DMACCARXDR = 0U;
volatile uint32_t COB_ETH_DMACCATXBR = 0U;
volatile uint32_t COB_ETH_DMACCARXBR = 0U;
volatile uint32_t COB_ETH_DMACSR = 0U;
volatile uint32_t COB_ETH_DMACMFCR = 0U;
volatile uint32_t COB_ETH_MMCTPCGR = 0U;
volatile uint32_t COB_ETH_MMCRUPGR = 0U;
volatile uint32_t COB_ETH_MMCRCRCEPR = 0U;
volatile uint32_t COB_ETH_MMCRAEPR = 0U;
volatile uint32_t COB_ETH_PhyAddr = 0xFFFFFFFFU;
volatile uint32_t COB_ETH_PhyReadStatus = 0U;
volatile uint32_t COB_ETH_PhyIdValid = 0U;
volatile uint32_t COB_ETH_PhyScanLastAddr = 0U;
volatile uint32_t COB_ETH_PhyScanFoundID1 = 0U;
volatile uint32_t COB_ETH_PhyScanFoundID2 = 0U;
volatile uint32_t COB_ETH_PHY_SMR = 0U;
volatile uint32_t COB_ETH_PHY_ID1 = 0U;
volatile uint32_t COB_ETH_PHY_ID2 = 0U;
volatile uint32_t COB_ETH_PHY_BCR = 0U;
volatile uint32_t COB_ETH_PHY_BSR = 0U;
volatile uint32_t COB_ETH_PHY_PHYSCSR = 0U;
volatile uint32_t COB_ETH_DMARxDscrTabAddr = 0U;
volatile uint32_t COB_ETH_DMATxDscrTabAddr = 0U;
volatile uint32_t COB_ETH_TxbufferAddr = 0U;
volatile uint32_t COB_ETH_TxDesc0_DESC0 = 0U;
volatile uint32_t COB_ETH_TxDesc0_DESC1 = 0U;
volatile uint32_t COB_ETH_TxDesc0_DESC2 = 0U;
volatile uint32_t COB_ETH_TxDesc0_DESC3 = 0U;
volatile uint32_t COB_ETH_RxDesc0_DESC0 = 0U;
volatile uint32_t COB_ETH_RxDesc0_DESC1 = 0U;
volatile uint32_t COB_ETH_RxDesc0_DESC2 = 0U;
volatile uint32_t COB_ETH_RxDesc0_DESC3 = 0U;

static uint32_t COB_ETH_IsValidPhyId(uint32_t id1, uint32_t id2)
{
  if ((id1 == 0U) || (id1 == 0xFFFFU) || (id2 == 0U) || (id2 == 0xFFFFU))
  {
    return 0U;
  }

  return 1U;
}

/* USER CODE END 0 */

ETH_HandleTypeDef heth1;

/* ETH1 init function */
void MX_ETH1_Init(void)
{

  /* USER CODE BEGIN ETH1_Init 0 */
  COB_ETH_DebugStage = 1U;

  /* USER CODE END ETH1_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH1_Init 1 */

  /* USER CODE END ETH1_Init 1 */

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  heth1.Instance = ETH1;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth1.Init.MACAddr = &MACAddr[0];
  heth1.Init.MediaInterface = HAL_ETH_RMII_MODE;
  for (int ch = 0; ch < ETH_DMA_CH_CNT; ch++)
  {
    heth1.Init.TxDesc[ch] = DMATxDscrTab[ch];
    heth1.Init.RxDesc[ch] = DMARxDscrTab[ch];
  }
  heth1.Init.RxBuffLen = 1536;

  /* USER CODE BEGIN MACADDRESS */
  MACAddr[0] = 0x02;
  MACAddr[1] = 0x00;
  MACAddr[2] = 0x00;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x01;
  MACAddr[5] = 0x88;

  /* USER CODE END MACADDRESS */

  COB_ETH_DebugStage = 90U;
  if (HAL_ETH_Init(&heth1) != HAL_OK)
  {
    COB_ETH_DebugStage = 900U;
    COB_ETH_LastErrorCode = heth1.ErrorCode;
    COB_ETH_LastDMAMR = heth1.Instance->DMAMR;
    COB_StatusLED_EthernetError();
    Error_Handler();
  }
  /* USER CODE BEGIN ETH1_Init 2 */
  COB_ETH_DebugStage = 100U;
  COB_ETH_LastErrorCode = heth1.ErrorCode;
  COB_ETH_LastDMAMR = heth1.Instance->DMAMR;

  /* USER CODE END ETH1_Init 2 */

}

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(ethHandle->Instance==ETH1)
  {
  /* USER CODE BEGIN ETH1_MspInit 0 */
    COB_ETH_DebugStage = 10U;

  /* USER CODE END ETH1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ETH1 | RCC_PERIPHCLK_ETH1PHY;
    PeriphClkInitStruct.Eth1ClockSelection = RCC_ETH1CLKSOURCE_HCLK;
    PeriphClkInitStruct.Eth1PhyInterfaceSelection = RCC_ETH1PHYIF_RMII;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      COB_ETH_DebugStage = 910U;
      Error_Handler();
    }

    /* ETH1 clock enable */
    COB_ETH_DebugStage = 20U;
    __HAL_RCC_ETH1_CLK_ENABLE();
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**ETH1 GPIO Configuration
    PF4     ------> ETH1_MDIO
    PF10     ------> ETH1_RMII_CRS_DV
    PF7     ------> ETH1_RMII_REF_CLK
    PF5     ------> ETH1_CLK
    PF15     ------> ETH1_RMII_RXD1
    PF14     ------> ETH1_RMII_RXD0
    PF11     ------> ETH1_RMII_TX_EN
    PF13     ------> ETH1_RMII_TXD1
    PF12     ------> ETH1_RMII_TXD0
    PG11     ------> ETH1_MDC
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_10|GPIO_PIN_7|GPIO_PIN_5
                          |GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_11|GPIO_PIN_13
                          |GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH1;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH1;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    COB_ETH_DebugStage = 30U;

    /* ETH1 interrupt Init */
    HAL_NVIC_SetPriority(ETH1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ETH1_IRQn);
  /* USER CODE BEGIN ETH1_MspInit 1 */
    COB_ETH_DebugStage = 40U;

  /* USER CODE END ETH1_MspInit 1 */
  }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* ethHandle)
{

  if(ethHandle->Instance==ETH1)
  {
  /* USER CODE BEGIN ETH1_MspDeInit 0 */

  /* USER CODE END ETH1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ETH1_CLK_DISABLE();
    __HAL_RCC_ETH1MAC_CLK_DISABLE();
    __HAL_RCC_ETH1TX_CLK_DISABLE();
    __HAL_RCC_ETH1RX_CLK_DISABLE();

    /**ETH1 GPIO Configuration
    PF4     ------> ETH1_MDIO
    PF10     ------> ETH1_RMII_CRS_DV
    PF7     ------> ETH1_RMII_REF_CLK
    PF5     ------> ETH1_CLK
    PF15     ------> ETH1_RMII_RXD1
    PF14     ------> ETH1_RMII_RXD0
    PF11     ------> ETH1_RMII_TX_EN
    PF13     ------> ETH1_RMII_TXD1
    PF12     ------> ETH1_RMII_TXD0
    PG11     ------> ETH1_MDC
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_4|GPIO_PIN_10|GPIO_PIN_7|GPIO_PIN_5
                          |GPIO_PIN_15|GPIO_PIN_14|GPIO_PIN_11|GPIO_PIN_13
                          |GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_11);

    /* ETH1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(ETH1_IRQn);
  /* USER CODE BEGIN ETH1_MspDeInit 1 */

  /* USER CODE END ETH1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void COB_ETH_UpdateDebugSnapshot(void)
{
  ETH_TypeDef *eth = heth1.Instance;
  uint32_t phy_value;

  if (eth == NULL)
  {
    return;
  }

  COB_ETH_LastErrorCode = heth1.ErrorCode;
  COB_ETH_LastDMAMR = eth->DMAMR;
  COB_ETH_MACCR = eth->MACCR;
  COB_ETH_MACPFR = eth->MACPFR;
  COB_ETH_MACISR = eth->MACISR;
  COB_ETH_MACRXTXSR = eth->MACRXTXSR;
  COB_ETH_MACDR = eth->MACDR;
  COB_ETH_MACA0HR = eth->MACA0HR;
  COB_ETH_MACA0LR = eth->MACA0LR;
  COB_ETH_DMAISR = eth->DMAISR;
  COB_ETH_DMADSR = eth->DMADSR;
  COB_ETH_MTLISR = eth->MTLISR;
  COB_ETH_MTLTXQ0OMR = eth->MTL_QUEUE[0].MTLTXQOMR;
  COB_ETH_MTLTXQ0DR = eth->MTL_QUEUE[0].MTLTXQDR;
  COB_ETH_MTLQ0ICSR = eth->MTL_QUEUE[0].MTLQICSR;
  COB_ETH_MTLRXQ0OMR = eth->MTL_QUEUE[0].MTLRXQOMR;
  COB_ETH_MTLRXQ0DR = eth->MTL_QUEUE[0].MTLRXQDR;
  COB_ETH_MTLTXQ1OMR = eth->MTL_QUEUE[1].MTLTXQOMR;
  COB_ETH_MTLTXQ1DR = eth->MTL_QUEUE[1].MTLTXQDR;
  COB_ETH_MTLQ1ICSR = eth->MTL_QUEUE[1].MTLQICSR;
  COB_ETH_MTLRXQ1OMR = eth->MTL_QUEUE[1].MTLRXQOMR;
  COB_ETH_MTLRXQ1DR = eth->MTL_QUEUE[1].MTLRXQDR;
  COB_ETH_DMACCR = eth->DMA_CH[0].DMACCR;
  COB_ETH_DMACTXCR = eth->DMA_CH[0].DMACTXCR;
  COB_ETH_DMACRXCR = eth->DMA_CH[0].DMACRXCR;
  COB_ETH_DMACTXDLAR = eth->DMA_CH[0].DMACTXDLAR;
  COB_ETH_DMACRXDLAR = eth->DMA_CH[0].DMACRXDLAR;
  COB_ETH_DMACTXDTPR = eth->DMA_CH[0].DMACTXDTPR;
  COB_ETH_DMACRXDTPR = eth->DMA_CH[0].DMACRXDTPR;
  COB_ETH_DMACCATXDR = eth->DMA_CH[0].DMACCATXDR;
  COB_ETH_DMACCARXDR = eth->DMA_CH[0].DMACCARXDR;
  COB_ETH_DMACCATXBR = eth->DMA_CH[0].DMACCATXBR;
  COB_ETH_DMACCARXBR = eth->DMA_CH[0].DMACCARXBR;
  COB_ETH_DMACSR = eth->DMA_CH[0].DMACSR;
  COB_ETH_DMACMFCR = eth->DMA_CH[0].DMACMFCR;
  COB_ETH_MMCTPCGR = eth->MMCTPCGR;
  COB_ETH_MMCRUPGR = eth->MMCRUPGR;
  COB_ETH_MMCRCRCEPR = eth->MMCRCRCEPR;
  COB_ETH_MMCRAEPR = eth->MMCRAEPR;
  COB_ETH_DMARxDscrTabAddr = (uint32_t)DMARxDscrTab;
  COB_ETH_DMATxDscrTabAddr = (uint32_t)DMATxDscrTab;
  COB_ETH_TxbufferAddr = (uint32_t)Txbuffer;
  COB_ETH_TxDesc0_DESC0 = DMATxDscrTab[0][0].DESC0;
  COB_ETH_TxDesc0_DESC1 = DMATxDscrTab[0][0].DESC1;
  COB_ETH_TxDesc0_DESC2 = DMATxDscrTab[0][0].DESC2;
  COB_ETH_TxDesc0_DESC3 = DMATxDscrTab[0][0].DESC3;
  COB_ETH_RxDesc0_DESC0 = DMARxDscrTab[0][0].DESC0;
  COB_ETH_RxDesc0_DESC1 = DMARxDscrTab[0][0].DESC1;
  COB_ETH_RxDesc0_DESC2 = DMARxDscrTab[0][0].DESC2;
  COB_ETH_RxDesc0_DESC3 = DMARxDscrTab[0][0].DESC3;

  if (COB_ETH_PhyAddr > COB_ETH_PHY_MAX_ADDR)
  {
    for (uint32_t addr = 0U; addr <= COB_ETH_PHY_MAX_ADDR; addr++)
    {
      uint32_t phy_id1 = 0U;
      uint32_t phy_id2 = 0U;

      COB_ETH_PhyScanLastAddr = addr;

      if ((HAL_ETH_ReadPHYRegister(&heth1, addr, LAN8742_PHYI1R, &phy_id1) == HAL_OK) &&
          (HAL_ETH_ReadPHYRegister(&heth1, addr, LAN8742_PHYI2R, &phy_id2) == HAL_OK) &&
          (COB_ETH_IsValidPhyId(phy_id1, phy_id2) != 0U))
      {
        COB_ETH_PhyAddr = addr;
        COB_ETH_PhyScanFoundID1 = phy_id1;
        COB_ETH_PhyScanFoundID2 = phy_id2;
        break;
      }
    }
  }

  if (COB_ETH_PhyAddr <= COB_ETH_PHY_MAX_ADDR)
  {
    COB_ETH_PhyReadStatus = 1U;

    if (HAL_ETH_ReadPHYRegister(&heth1, COB_ETH_PhyAddr, LAN8742_BCR, &phy_value) == HAL_OK)
    {
      COB_ETH_PHY_BCR = phy_value;
    }
    else
    {
      COB_ETH_PhyReadStatus = 0U;
    }

    if (HAL_ETH_ReadPHYRegister(&heth1, COB_ETH_PhyAddr, LAN8742_SMR, &phy_value) == HAL_OK)
    {
      COB_ETH_PHY_SMR = phy_value;
    }
    else
    {
      COB_ETH_PhyReadStatus = 0U;
    }

    if (HAL_ETH_ReadPHYRegister(&heth1, COB_ETH_PhyAddr, LAN8742_PHYI1R, &phy_value) == HAL_OK)
    {
      COB_ETH_PHY_ID1 = phy_value;
    }
    else
    {
      COB_ETH_PhyReadStatus = 0U;
    }

    if (HAL_ETH_ReadPHYRegister(&heth1, COB_ETH_PhyAddr, LAN8742_PHYI2R, &phy_value) == HAL_OK)
    {
      COB_ETH_PHY_ID2 = phy_value;
      COB_ETH_PhyIdValid = COB_ETH_IsValidPhyId(COB_ETH_PHY_ID1, COB_ETH_PHY_ID2);
    }
    else
    {
      COB_ETH_PhyReadStatus = 0U;
      COB_ETH_PhyIdValid = 0U;
    }

    if (HAL_ETH_ReadPHYRegister(&heth1, COB_ETH_PhyAddr, LAN8742_BSR, &phy_value) == HAL_OK)
    {
      COB_ETH_PHY_BSR = phy_value;
    }
    else
    {
      COB_ETH_PhyReadStatus = 0U;
    }

    if (HAL_ETH_ReadPHYRegister(&heth1, COB_ETH_PhyAddr, LAN8742_PHYSCSR, &phy_value) == HAL_OK)
    {
      COB_ETH_PHY_PHYSCSR = phy_value;
    }
    else
    {
      COB_ETH_PhyReadStatus = 0U;
    }
  }
  else
  {
    COB_ETH_PhyReadStatus = 0U;
    COB_ETH_PhyIdValid = 0U;
  }
}

/* USER CODE END 1 */

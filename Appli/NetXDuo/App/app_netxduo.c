/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_netxduo.c
  * @author  MCD Application Team
  * @brief   NetXDuo applicative file
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
#include "app_netxduo.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "eth.h"
#include "gpio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define COB_NETX_IP_ADDRESS        IP_ADDRESS(192, 168, 1, 188)
#define COB_NETX_NETWORK_MASK      IP_ADDRESS(255, 255, 255, 0)
#define COB_NETX_PACKET_SIZE       1536U
#define COB_NETX_PACKET_COUNT      16U
#define COB_NETX_PACKET_POOL_SIZE  ((COB_NETX_PACKET_SIZE + sizeof(NX_PACKET)) * COB_NETX_PACKET_COUNT)
#define COB_NETX_ARP_CACHE_SIZE    1024U
#define COB_NETX_IP_STACK_SIZE     2048U
#define COB_NETX_STATUS_STACK_SIZE 1024U
#define COB_NETX_LINK_TIMEOUT      (6U * TX_TIMER_TICKS_PER_SECOND)
#define COB_NETX_GRATUITOUS_ARP_PERIOD (1U * TX_TIMER_TICKS_PER_SECOND)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static NX_PACKET_POOL cob_packet_pool;
static NX_IP cob_ip;
static UCHAR *cob_packet_pool_memory;
static UCHAR *cob_arp_cache_memory;
static UCHAR *cob_ip_stack_memory;
static UCHAR *cob_status_stack_memory;
static TX_THREAD cob_status_thread;
volatile ULONG COB_NX_LinkStatus = 0U;
volatile ULONG COB_NX_IpAddress = 0U;
volatile ULONG COB_NX_NetworkMask = 0U;
volatile ULONG COB_NX_IpPacketsSent = 0U;
volatile ULONG COB_NX_IpPacketsReceived = 0U;
volatile ULONG COB_NX_IpBytesSent = 0U;
volatile ULONG COB_NX_IpBytesReceived = 0U;
volatile ULONG COB_NX_IpInvalidPackets = 0U;
volatile ULONG COB_NX_IpReceiveDropped = 0U;
volatile ULONG COB_NX_IpSendDropped = 0U;
volatile ULONG COB_NX_ArpRequestsSent = 0U;
volatile ULONG COB_NX_ArpRequestsReceived = 0U;
volatile ULONG COB_NX_ArpResponsesSent = 0U;
volatile ULONG COB_NX_ArpResponsesReceived = 0U;
volatile ULONG COB_NX_ArpDynamicEntries = 0U;
volatile ULONG COB_NX_ArpInvalidMessages = 0U;
volatile ULONG COB_NX_IcmpPingsSent = 0U;
volatile ULONG COB_NX_IcmpPingTimeouts = 0U;
volatile ULONG COB_NX_IcmpPingResponsesReceived = 0U;
volatile ULONG COB_NX_IcmpChecksumErrors = 0U;
volatile ULONG COB_NX_IcmpUnhandledMessages = 0U;
volatile ULONG COB_NX_PacketPoolMemoryAddr = 0U;
volatile ULONG COB_NX_PacketPoolPayloadAddr = 0U;
volatile ULONG COB_NX_InitStage = 0U;
volatile ULONG COB_NX_LastStatus = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID COB_NetXStatusThread(ULONG initial_input);

/* USER CODE END PFP */

/**
  * @brief  Application NetXDuo Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
  UINT ret = NX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN MX_NetXDuo_MEM_POOL */
  COB_NX_InitStage = 1U;
  if (tx_byte_allocate(byte_pool, (VOID **)&cob_packet_pool_memory,
                       COB_NETX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 10U;
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }
  COB_NX_PacketPoolMemoryAddr = (ULONG)cob_packet_pool_memory;

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_arp_cache_memory,
                       COB_NETX_ARP_CACHE_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 11U;
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_ip_stack_memory,
                       COB_NETX_IP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 12U;
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_status_stack_memory,
                       COB_NETX_STATUS_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 13U;
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }
  /* USER CODE END MX_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */
  nx_system_initialize();

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */
  ret = nx_packet_pool_create(&cob_packet_pool, "COB NetX packet pool",
                              COB_NETX_PACKET_SIZE, cob_packet_pool_memory,
                              COB_NETX_PACKET_POOL_SIZE);
  COB_NX_LastStatus = ret;
  if (ret != NX_SUCCESS)
  {
    COB_NX_InitStage = 20U;
    COB_StatusLED_EthernetError();
    return ret;
  }
  COB_NX_PacketPoolPayloadAddr = (ULONG)cob_packet_pool.nx_packet_pool_start;

  ret = nx_ip_create(&cob_ip, "COB IP", COB_NETX_IP_ADDRESS, COB_NETX_NETWORK_MASK,
                     &cob_packet_pool, nx_stm32_eth_driver,
                     cob_ip_stack_memory, COB_NETX_IP_STACK_SIZE, 1U);
  COB_NX_LastStatus = ret;
  if (ret != NX_SUCCESS)
  {
    COB_NX_InitStage = 21U;
    COB_StatusLED_EthernetError();
    return ret;
  }

  ret = nx_arp_enable(&cob_ip, cob_arp_cache_memory, COB_NETX_ARP_CACHE_SIZE);
  COB_NX_LastStatus = ret;
  if (ret != NX_SUCCESS)
  {
    COB_NX_InitStage = 22U;
    COB_StatusLED_EthernetError();
    return ret;
  }

  ret = nx_icmp_enable(&cob_ip);
  COB_NX_LastStatus = ret;
  if (ret != NX_SUCCESS)
  {
    COB_NX_InitStage = 23U;
    COB_StatusLED_EthernetError();
    return ret;
  }

  if (tx_thread_create(&cob_status_thread, "COB NetX status",
                       COB_NetXStatusThread, 0U,
                       cob_status_stack_memory, COB_NETX_STATUS_STACK_SIZE,
                       5U, 5U, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    COB_NX_InitStage = 24U;
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }
  COB_NX_InitStage = 30U;
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
static VOID COB_NetXStatusThread(ULONG initial_input)
{
  ULONG actual_status;
  ULONG unused;
  ULONG ticks_since_gratuitous_arp = 0U;

  (void)initial_input;

  if (nx_ip_status_check(&cob_ip, NX_IP_LINK_ENABLED, &actual_status, COB_NETX_LINK_TIMEOUT) == NX_SUCCESS)
  {
    COB_StatusLED_EthernetReady();
  }
  else
  {
    COB_StatusLED_EthernetError();
  }

  while (1)
  {
    if (nx_ip_status_check(&cob_ip, NX_IP_LINK_ENABLED, &actual_status, TX_NO_WAIT) == NX_SUCCESS)
    {
      COB_NX_LinkStatus = actual_status;
      COB_StatusLED_BlueToggle();
      ticks_since_gratuitous_arp += (TX_TIMER_TICKS_PER_SECOND / 2U);
      if (ticks_since_gratuitous_arp >= COB_NETX_GRATUITOUS_ARP_PERIOD)
      {
        (void)nx_arp_gratuitous_send(&cob_ip, NX_NULL);
        ticks_since_gratuitous_arp = 0U;
      }
    }
    else
    {
      COB_NX_LinkStatus = 0U;
      ticks_since_gratuitous_arp = 0U;
    }

    (void)nx_ip_address_get(&cob_ip, (ULONG *)&COB_NX_IpAddress, (ULONG *)&COB_NX_NetworkMask);
    COB_ETH_UpdateDebugSnapshot();

    (void)nx_ip_info_get(&cob_ip,
                         (ULONG *)&COB_NX_IpPacketsSent,
                         (ULONG *)&COB_NX_IpBytesSent,
                         (ULONG *)&COB_NX_IpPacketsReceived,
                         (ULONG *)&COB_NX_IpBytesReceived,
                         (ULONG *)&COB_NX_IpInvalidPackets,
                         (ULONG *)&COB_NX_IpReceiveDropped,
                         &unused,
                         (ULONG *)&COB_NX_IpSendDropped,
                         &unused,
                         &unused);

    (void)nx_arp_info_get(&cob_ip,
                          (ULONG *)&COB_NX_ArpRequestsSent,
                          (ULONG *)&COB_NX_ArpRequestsReceived,
                          (ULONG *)&COB_NX_ArpResponsesSent,
                          (ULONG *)&COB_NX_ArpResponsesReceived,
                          (ULONG *)&COB_NX_ArpDynamicEntries,
                          &unused,
                          &unused,
                          (ULONG *)&COB_NX_ArpInvalidMessages);

    (void)nx_icmp_info_get(&cob_ip,
                           (ULONG *)&COB_NX_IcmpPingsSent,
                           (ULONG *)&COB_NX_IcmpPingTimeouts,
                           &unused,
                           (ULONG *)&COB_NX_IcmpPingResponsesReceived,
                           (ULONG *)&COB_NX_IcmpChecksumErrors,
                           (ULONG *)&COB_NX_IcmpUnhandledMessages);

    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2U);
  }
}

/* USER CODE END 1 */

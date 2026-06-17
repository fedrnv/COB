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
  if (tx_byte_allocate(byte_pool, (VOID **)&cob_packet_pool_memory,
                       COB_NETX_PACKET_POOL_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_arp_cache_memory,
                       COB_NETX_ARP_CACHE_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_ip_stack_memory,
                       COB_NETX_IP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_status_stack_memory,
                       COB_NETX_STATUS_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
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
  if (ret != NX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return ret;
  }

  ret = nx_ip_create(&cob_ip, "COB IP", COB_NETX_IP_ADDRESS, COB_NETX_NETWORK_MASK,
                     &cob_packet_pool, nx_stm32_eth_driver,
                     cob_ip_stack_memory, COB_NETX_IP_STACK_SIZE, 1U);
  if (ret != NX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return ret;
  }

  ret = nx_arp_enable(&cob_ip, cob_arp_cache_memory, COB_NETX_ARP_CACHE_SIZE);
  if (ret != NX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return ret;
  }

  ret = nx_icmp_enable(&cob_ip);
  if (ret != NX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return ret;
  }

  if (tx_thread_create(&cob_status_thread, "COB NetX status",
                       COB_NetXStatusThread, 0U,
                       cob_status_stack_memory, COB_NETX_STATUS_STACK_SIZE,
                       5U, 5U, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    COB_StatusLED_EthernetError();
    return NX_NOT_SUCCESSFUL;
  }
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
static VOID COB_NetXStatusThread(ULONG initial_input)
{
  ULONG actual_status;

  (void)initial_input;

  if (nx_ip_status_check(&cob_ip, NX_IP_LINK_ENABLED, &actual_status, COB_NETX_LINK_TIMEOUT) == NX_SUCCESS)
  {
    COB_StatusLED_EthernetReady();
  }
  else
  {
    COB_StatusLED_EthernetError();
  }

  tx_thread_sleep(TX_WAIT_FOREVER);
}

/* USER CODE END 1 */

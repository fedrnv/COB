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
#include "cob_config.h"
#include "cob_ethernet_exchange.h"
#include "eth.h"
#include "gpio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define COB_NETX_NETWORK_MASK      IP_ADDRESS(255, 255, 255, 0)
#define COB_NETX_PACKET_SIZE       1536U
#define COB_NETX_PACKET_COUNT      16U
#define COB_NETX_PACKET_POOL_SIZE  ((COB_NETX_PACKET_SIZE + sizeof(NX_PACKET)) * COB_NETX_PACKET_COUNT)
#define COB_NETX_ARP_CACHE_SIZE    1024U
#define COB_NETX_IP_STACK_SIZE     2048U
#define COB_NETX_STATUS_STACK_SIZE 1024U
#define COB_NETX_EXCHANGE_STACK_SIZE 2048U
#define COB_NETX_LINK_TIMEOUT      TX_NO_WAIT
#define COB_NETX_STATUS_PERIOD     (TX_TIMER_TICKS_PER_SECOND / 10U)
#define COB_NETX_REINIT_PERIOD     (TX_TIMER_TICKS_PER_SECOND / 2U)
#define COB_NETX_EXCHANGE_RX_TIMEOUT (TX_TIMER_TICKS_PER_SECOND / 10U)
#define COB_NETX_ACTIVITY_PULSE_PERIOD (TX_TIMER_TICKS_PER_SECOND / 5U)
#define COB_NETX_GRATUITOUS_ARP_PERIOD (1U * TX_TIMER_TICKS_PER_SECOND)
#define COB_NETX_INFO_PERIOD       (3U * TX_TIMER_TICKS_PER_SECOND)
#define COB_NETX_EXCHANGE_PORT     1556U
#define COB_NETX_ADDITIONAL_SEND_PORT 50001U
#define COB_NETX_INFO_PORT         50101U
#define COB_NETX_EXCHANGE_RX_SIZE  512U
#define COB_NETX_FIRMWARE_VERSION  0x00000001UL
#define COB_NETX_CONFIG_VERSION    0x00000001UL

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
static UCHAR *cob_exchange_stack_memory;
static TX_THREAD cob_status_thread;
static TX_THREAD cob_exchange_thread;
static NX_UDP_SOCKET cob_exchange_socket;
static NX_UDP_SOCKET cob_info_socket;
static ULONG cob_exchange_peer_ip;
static UINT cob_exchange_peer_port;
static uint8_t cob_exchange_rx_buffer[COB_NETX_EXCHANGE_RX_SIZE];
static uint16_t cob_info_tx_index;
static volatile bool cob_netx_ready;
static volatile ULONG cob_exchange_led_pulse_ticks;
static bool cob_packet_pool_created;
static bool cob_ip_created;
static bool cob_exchange_socket_created;
static bool cob_info_socket_created;
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
volatile ULONG COB_NX_ExchangePacketsReceived = 0U;
volatile ULONG COB_NX_ExchangePacketsProcessed = 0U;
volatile ULONG COB_NX_ExchangePacketsSent = 0U;
volatile ULONG COB_NX_ExchangePacketsDropped = 0U;
volatile ULONG COB_NX_ExchangeLastStatus = 0U;
volatile ULONG COB_NX_ExchangeLastPeerIp = 0U;
volatile ULONG COB_NX_ExchangeLastPeerPort = 0U;
volatile ULONG COB_NX_ExchangeLastPeerAddressIndex = 0U;
volatile ULONG COB_NX_ExchangeLastRxLength = 0U;
volatile ULONG COB_NX_ExchangeLastTxLength = 0U;
volatile ULONG COB_NX_InfoPacketsSent = 0U;
volatile ULONG COB_NX_InfoPacketsDropped = 0U;
volatile ULONG COB_NX_InfoLastStatus = 0U;
volatile ULONG COB_NX_ReinitCount = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static VOID COB_NetXStatusThread(ULONG initial_input);
static VOID COB_NetXExchangeThread(ULONG initial_input);
static bool COB_NetXExchangeSend(const uint8_t *payload, uint16_t length, void *context);
static UINT COB_NetXInitializeNetwork(void);
static void COB_NetXDeinitializeNetwork(void);
static void COB_NetXUpdateDebugCounters(void);
static bool COB_NetXBroadcastInfo(void);
static bool COB_NetXSendUdpPacket(NX_UDP_SOCKET *socket, const uint8_t *payload,
                                  uint16_t length, ULONG ip_address, UINT port);
static void COB_NetXFillInfoPacket(COB_InfoPacket_t *packet);
static int16_t COB_NetXReadMcuTemperatureCentiC(void);
static void COB_NetXExchangeActivityPulse(void);
static void COB_NetXUpdateExchangeLed(ULONG elapsed_ticks);

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
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }
  COB_NX_PacketPoolMemoryAddr = (ULONG)cob_packet_pool_memory;

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_arp_cache_memory,
                       COB_NETX_ARP_CACHE_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 11U;
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_ip_stack_memory,
                       COB_NETX_IP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 12U;
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_status_stack_memory,
                       COB_NETX_STATUS_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 13U;
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_byte_allocate(byte_pool, (VOID **)&cob_exchange_stack_memory,
                       COB_NETX_EXCHANGE_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    COB_NX_InitStage = 14U;
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }
  /* USER CODE END MX_NetXDuo_MEM_POOL */
  /* USER CODE BEGIN 0 */
  nx_system_initialize();

  /* USER CODE END 0 */

  /* USER CODE BEGIN MX_NetXDuo_Init */
  COB_EthernetExchange_Init(COB_NetXExchangeSend, &cob_exchange_socket);

  if (tx_thread_create(&cob_status_thread, "COB NetX status",
                       COB_NetXStatusThread, 0U,
                       cob_status_stack_memory, COB_NETX_STATUS_STACK_SIZE,
                       5U, 5U, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    COB_NX_InitStage = 20U;
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }

  if (tx_thread_create(&cob_exchange_thread, "COB Exchange",
                       COB_NetXExchangeThread, 0U,
                       cob_exchange_stack_memory, COB_NETX_EXCHANGE_STACK_SIZE,
                       6U, 6U, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    COB_NX_InitStage = 21U;
    COB_StatusLED_ErrorSet(true);
    COB_StatusLED_ExchangeSet(false);
    return NX_NOT_SUCCESSFUL;
  }

  COB_NX_InitStage = 22U;
  ret = NX_SUCCESS;
  /* USER CODE END MX_NetXDuo_Init */

  return ret;
}

/* USER CODE BEGIN 1 */
static VOID COB_NetXStatusThread(ULONG initial_input)
{
  ULONG actual_status;
  ULONG ticks_since_gratuitous_arp = 0U;
  ULONG ticks_since_info = COB_NETX_INFO_PERIOD;
  UINT status;

  (void)initial_input;

  while (1)
  {
    if (!cob_netx_ready)
    {
      status = COB_NetXInitializeNetwork();
      COB_NX_LastStatus = status;
      if (status == NX_SUCCESS)
      {
        cob_netx_ready = true;
        ticks_since_info = COB_NETX_INFO_PERIOD;
        ticks_since_gratuitous_arp = 0U;
        cob_exchange_led_pulse_ticks = 0U;
        COB_StatusLED_ErrorSet(false);
        COB_StatusLED_ExchangeSet(true);
      }
      else
      {
        COB_NX_InitStage = 40U;
        COB_StatusLED_ErrorSet(true);
        COB_StatusLED_ExchangeSet(false);
        COB_NetXDeinitializeNetwork();
        tx_thread_sleep(COB_NETX_REINIT_PERIOD);
        continue;
      }
    }

    if (nx_ip_status_check(&cob_ip, NX_IP_LINK_ENABLED, &actual_status, COB_NETX_LINK_TIMEOUT) != NX_SUCCESS)
    {
      COB_NX_LinkStatus = 0U;
      cob_netx_ready = false;
      cob_exchange_led_pulse_ticks = 0U;
      COB_StatusLED_ErrorSet(true);
      COB_StatusLED_ExchangeSet(false);
      tx_thread_sleep(COB_NETX_EXCHANGE_RX_TIMEOUT);
      COB_NetXDeinitializeNetwork();
      tx_thread_sleep(COB_NETX_REINIT_PERIOD);
      continue;
    }

    COB_NX_LinkStatus = actual_status;
    COB_StatusLED_ErrorSet(false);
    COB_NetXUpdateDebugCounters();
    COB_NetXUpdateExchangeLed(COB_NETX_STATUS_PERIOD);

    if (ticks_since_info >= COB_NETX_INFO_PERIOD)
    {
      (void)COB_NetXBroadcastInfo();
      ticks_since_info = 0U;
    }

    ticks_since_gratuitous_arp += COB_NETX_STATUS_PERIOD;
    if (ticks_since_gratuitous_arp >= COB_NETX_GRATUITOUS_ARP_PERIOD)
    {
      (void)nx_arp_gratuitous_send(&cob_ip, NX_NULL);
      ticks_since_gratuitous_arp = 0U;
    }

    ticks_since_info += COB_NETX_STATUS_PERIOD;
    tx_thread_sleep(COB_NETX_STATUS_PERIOD);
  }
}

static VOID COB_NetXExchangeThread(ULONG initial_input)
{
  NX_PACKET *packet;
  ULONG bytes_copied;
  ULONG packet_length;
  ULONG peer_ip;
  UINT peer_protocol;
  UINT peer_port;
  UINT interface_index;
  UINT status;

  (void)initial_input;

  while (1)
  {
    if (!cob_netx_ready || !cob_exchange_socket_created)
    {
      tx_thread_sleep(COB_NETX_REINIT_PERIOD);
      continue;
    }

    status = nx_udp_socket_receive(&cob_exchange_socket, &packet, COB_NETX_EXCHANGE_RX_TIMEOUT);
    COB_NX_ExchangeLastStatus = status;
    if (status != NX_SUCCESS)
    {
      if (status != NX_NO_PACKET)
      {
        COB_NX_ExchangePacketsDropped++;
      }
      continue;
    }

    status = nx_udp_packet_info_extract(packet, &peer_ip, &peer_protocol, &peer_port, &interface_index);
    COB_NX_ExchangeLastStatus = status;
    if (status != NX_SUCCESS)
    {
      (void)nx_packet_release(packet);
      COB_NX_ExchangePacketsDropped++;
      COB_StatusLED_ErrorSet(true);
      continue;
    }
    COB_NetXExchangeActivityPulse();

    cob_exchange_peer_ip = peer_ip;
    cob_exchange_peer_port = peer_port;
    COB_NX_ExchangeLastPeerIp = peer_ip;
    COB_NX_ExchangeLastPeerPort = peer_port;
    COB_NX_ExchangeLastPeerAddressIndex = interface_index;
    (void)peer_protocol;

    packet_length = 0U;
    status = nx_packet_length_get(packet, &packet_length);
    COB_NX_ExchangeLastStatus = status;
    if ((status != NX_SUCCESS) || (packet_length > COB_NETX_EXCHANGE_RX_SIZE))
    {
      (void)nx_packet_release(packet);
      COB_NX_ExchangePacketsDropped++;
      COB_StatusLED_ErrorSet(true);
      continue;
    }

    bytes_copied = 0U;
    status = nx_packet_data_retrieve(packet, cob_exchange_rx_buffer, &bytes_copied);
    (void)nx_packet_release(packet);

    COB_NX_ExchangeLastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_ExchangePacketsDropped++;
      COB_StatusLED_ErrorSet(true);
      continue;
    }

    COB_NX_ExchangePacketsReceived++;
    COB_NX_ExchangeLastRxLength = bytes_copied;
    if (COB_EthernetExchange_ProcessPacket(cob_exchange_rx_buffer, (uint16_t)bytes_copied))
    {
      COB_NX_ExchangePacketsProcessed++;
    }
    else
    {
      COB_NX_ExchangePacketsDropped++;
      COB_StatusLED_ErrorSet(true);
    }
  }
}

static bool COB_NetXExchangeSend(const uint8_t *payload, uint16_t length, void *context)
{
  NX_UDP_SOCKET *socket = (NX_UDP_SOCKET *)context;
  UINT status;

  if (!cob_netx_ready || (socket == NX_NULL) || (payload == NULL) || (length == 0U) ||
      (cob_exchange_peer_ip == 0U) || (cob_exchange_peer_port == 0U))
  {
    COB_NX_ExchangePacketsDropped++;
    COB_StatusLED_ErrorSet(true);
    return false;
  }

  status = COB_NetXSendUdpPacket(socket, payload, length, cob_exchange_peer_ip, cob_exchange_peer_port) ?
           NX_SUCCESS : COB_NX_ExchangeLastStatus;
  if (status == NX_SUCCESS)
  {
    COB_NX_ExchangePacketsSent++;
    COB_NX_ExchangeLastTxLength = length;
    return true;
  }

  COB_NX_ExchangePacketsDropped++;
  COB_StatusLED_ErrorSet(true);
  return false;
}

static UINT COB_NetXInitializeNetwork(void)
{
  UINT status;

  COB_NX_ReinitCount++;
  COB_NX_InitStage = 50U;

  if (!cob_packet_pool_created)
  {
    status = nx_packet_pool_create(&cob_packet_pool, "COB NetX packet pool",
                                   COB_NETX_PACKET_SIZE, cob_packet_pool_memory,
                                   COB_NETX_PACKET_POOL_SIZE);
    COB_NX_LastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_InitStage = 51U;
      return status;
    }
    COB_NX_PacketPoolPayloadAddr = (ULONG)cob_packet_pool.nx_packet_pool_start;
    cob_packet_pool_created = true;
  }

  if (!cob_ip_created)
  {
    status = nx_ip_create(&cob_ip, "COB IP", COB_Config_GetIpAddressU32(), COB_NETX_NETWORK_MASK,
                          &cob_packet_pool, nx_stm32_eth_driver,
                          cob_ip_stack_memory, COB_NETX_IP_STACK_SIZE, 1U);
    COB_NX_LastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_InitStage = 52U;
      return status;
    }
    cob_ip_created = true;
  }

  status = nx_arp_enable(&cob_ip, cob_arp_cache_memory, COB_NETX_ARP_CACHE_SIZE);
  COB_NX_LastStatus = status;
  if (status != NX_SUCCESS)
  {
    COB_NX_InitStage = 53U;
    return status;
  }

  status = nx_icmp_enable(&cob_ip);
  COB_NX_LastStatus = status;
  if (status != NX_SUCCESS)
  {
    COB_NX_InitStage = 54U;
    return status;
  }

  status = nx_udp_enable(&cob_ip);
  COB_NX_LastStatus = status;
  if (status != NX_SUCCESS)
  {
    COB_NX_InitStage = 55U;
    return status;
  }

  if (!cob_exchange_socket_created)
  {
    status = nx_udp_socket_create(&cob_ip, &cob_exchange_socket, "COB Exchange",
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 8U);
    COB_NX_LastStatus = status;
    COB_NX_ExchangeLastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_InitStage = 56U;
      return status;
    }

    status = nx_udp_socket_bind(&cob_exchange_socket, COB_NETX_EXCHANGE_PORT, TX_NO_WAIT);
    COB_NX_LastStatus = status;
    COB_NX_ExchangeLastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_InitStage = 57U;
      (void)nx_udp_socket_delete(&cob_exchange_socket);
      return status;
    }
    cob_exchange_socket_created = true;
  }

  if (!cob_info_socket_created)
  {
    status = nx_udp_socket_create(&cob_ip, &cob_info_socket, "COB Info",
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 1U);
    COB_NX_LastStatus = status;
    COB_NX_InfoLastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_InitStage = 58U;
      return status;
    }

    status = nx_udp_socket_bind(&cob_info_socket, NX_ANY_PORT, TX_NO_WAIT);
    COB_NX_LastStatus = status;
    COB_NX_InfoLastStatus = status;
    if (status != NX_SUCCESS)
    {
      COB_NX_InitStage = 59U;
      (void)nx_udp_socket_delete(&cob_info_socket);
      return status;
    }
    cob_info_socket_created = true;
  }

  COB_NetXUpdateDebugCounters();
  COB_NX_InitStage = 60U;
  return NX_SUCCESS;
}

static void COB_NetXDeinitializeNetwork(void)
{
  if (cob_info_socket_created)
  {
    (void)nx_udp_socket_unbind(&cob_info_socket);
    (void)nx_udp_socket_delete(&cob_info_socket);
    cob_info_socket_created = false;
  }

  if (cob_exchange_socket_created)
  {
    (void)nx_udp_socket_unbind(&cob_exchange_socket);
    (void)nx_udp_socket_delete(&cob_exchange_socket);
    cob_exchange_socket_created = false;
  }

  if (cob_ip_created)
  {
    (void)nx_ip_delete(&cob_ip);
    cob_ip_created = false;
  }

  if (cob_packet_pool_created)
  {
    (void)nx_packet_pool_delete(&cob_packet_pool);
    cob_packet_pool_created = false;
  }

  COB_NX_LinkStatus = 0U;
}

static void COB_NetXUpdateDebugCounters(void)
{
  ULONG unused;

  if (!cob_ip_created)
  {
    return;
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
}

static bool COB_NetXBroadcastInfo(void)
{
  COB_InfoPacket_t info_packet;

  if (!cob_netx_ready || !cob_info_socket_created)
  {
    COB_NX_InfoLastStatus = NX_NOT_SUCCESSFUL;
    COB_NX_InfoPacketsDropped++;
    COB_StatusLED_ErrorSet(true);
    return false;
  }

  COB_NetXFillInfoPacket(&info_packet);
  if (COB_NetXSendUdpPacket(&cob_info_socket, (const uint8_t *)&info_packet,
                            sizeof(info_packet), NX_IP_LIMITED_BROADCAST,
                            COB_NETX_INFO_PORT))
  {
    COB_NX_InfoPacketsSent++;
    COB_NX_InfoLastStatus = NX_SUCCESS;
    COB_NetXExchangeActivityPulse();
    return true;
  }

  COB_NX_InfoLastStatus = COB_NX_ExchangeLastStatus;
  COB_NX_InfoPacketsDropped++;
  COB_StatusLED_ErrorSet(true);
  return false;
}

static bool COB_NetXSendUdpPacket(NX_UDP_SOCKET *socket, const uint8_t *payload,
                                  uint16_t length, ULONG ip_address, UINT port)
{
  NX_PACKET *packet;
  UINT status;

  status = nx_packet_allocate(&cob_packet_pool, &packet, NX_UDP_PACKET, TX_NO_WAIT);
  COB_NX_ExchangeLastStatus = status;
  if (status != NX_SUCCESS)
  {
    COB_StatusLED_ErrorSet(true);
    return false;
  }

  status = nx_packet_data_append(packet, (VOID *)payload, length, &cob_packet_pool, TX_NO_WAIT);
  COB_NX_ExchangeLastStatus = status;
  if (status != NX_SUCCESS)
  {
    (void)nx_packet_release(packet);
    COB_StatusLED_ErrorSet(true);
    return false;
  }

  status = nx_udp_socket_send(socket, packet, ip_address, port);
  COB_NX_ExchangeLastStatus = status;
  if (status != NX_SUCCESS)
  {
    (void)nx_packet_release(packet);
    COB_StatusLED_ErrorSet(true);
    return false;
  }

  return true;
}

static void COB_NetXFillInfoPacket(COB_InfoPacket_t *packet)
{
  ULONG ip_address = 0U;
  ULONG network_mask = 0U;

  (void)nx_ip_address_get(&cob_ip, &ip_address, &network_mask);

  packet->Id = COB_INFO_SEND_ID;
  packet->Index = cob_info_tx_index++;
  packet->Length = (uint16_t)sizeof(*packet);
  packet->DeviceId[0] = HAL_GetUIDw0();
  packet->DeviceId[1] = HAL_GetUIDw1();
  packet->DeviceId[2] = HAL_GetUIDw2();
  packet->CurrentIpAddress[0] = (uint8_t)((ip_address >> 24) & 0xFFU);
  packet->CurrentIpAddress[1] = (uint8_t)((ip_address >> 16) & 0xFFU);
  packet->CurrentIpAddress[2] = (uint8_t)((ip_address >> 8) & 0xFFU);
  packet->CurrentIpAddress[3] = (uint8_t)(ip_address & 0xFFU);
  packet->AdditionalSendPort = COB_NETX_ADDITIONAL_SEND_PORT;
  packet->CurrentReceivePort = COB_NETX_EXCHANGE_PORT;
  packet->McuTemperatureCentiC = COB_NetXReadMcuTemperatureCentiC();
  packet->CurrentStatus = 0U;
  packet->Version = COB_NETX_FIRMWARE_VERSION;
  packet->Config = COB_NETX_CONFIG_VERSION;
  (void)network_mask;
}

static int16_t COB_NetXReadMcuTemperatureCentiC(void)
{
  return 0;
}

static void COB_NetXExchangeActivityPulse(void)
{
  if (!cob_netx_ready)
  {
    return;
  }

  cob_exchange_led_pulse_ticks = COB_NETX_ACTIVITY_PULSE_PERIOD;
  COB_StatusLED_ExchangeSet(false);
}

static void COB_NetXUpdateExchangeLed(ULONG elapsed_ticks)
{
  if (!cob_netx_ready)
  {
    COB_StatusLED_ExchangeSet(false);
    return;
  }

  if (cob_exchange_led_pulse_ticks > 0U)
  {
    if (cob_exchange_led_pulse_ticks > elapsed_ticks)
    {
      cob_exchange_led_pulse_ticks -= elapsed_ticks;
      COB_StatusLED_ExchangeSet(false);
      return;
    }

    cob_exchange_led_pulse_ticks = 0U;
  }

  COB_StatusLED_ExchangeSet(true);
}

/* USER CODE END 1 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
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
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gpio.h"
#include "extmem_manager.h"
#include "stm32_extmem.h"
#include "stm32_sfdp_driver_api.h"
#include "stm32_sal_xspi_api.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define COB_LED_THREAD_STACK_SIZE 1024U
#define COB_FLASH_THREAD_STACK_SIZE 2048U
#define COB_PSRAM_THREAD_STACK_SIZE 1024U
#define COB_LED_THREAD_PRIORITY   15U
#define COB_FLASH_THREAD_PRIORITY 10U
#define COB_PSRAM_THREAD_PRIORITY 11U
#define COB_FLASH_TEST_CAPACITY_BYTES (32UL * 1024UL * 1024UL)
#define COB_PSRAM_TEST_WORD_COUNT 1024U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD COB_LedThread;
static TX_THREAD COB_FlashThread;
static TX_THREAD COB_PsramThread;
static ULONG COB_LedThreadStack[COB_LED_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG COB_FlashThreadStack[COB_FLASH_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG COB_PsramThreadStack[COB_PSRAM_THREAD_STACK_SIZE / sizeof(ULONG)];
extern volatile uint32_t COB_FlashTestStage;
extern volatile uint32_t COB_FlashTestPassed;
extern volatile uint32_t COB_FlashTestValue;
extern volatile uint32_t COB_FlashTestAddress;
extern volatile int32_t COB_FlashTestLastStatus;
extern volatile int32_t COB_FlashDriverStatus;
extern volatile uint32_t COB_FlashInfoSizePower;
extern volatile uint32_t COB_FlashInfoPageSize;
extern volatile uint32_t COB_FlashInfoErase1SizePower;
extern volatile uint32_t COB_FlashInfoErase2SizePower;
extern volatile uint32_t COB_FlashInfoErase3SizePower;
extern volatile uint32_t COB_FlashInfoErase4SizePower;
extern volatile int32_t COB_FlashJedecNcs1CommandStatus;
extern volatile int32_t COB_FlashJedecNcs1ReceiveStatus;
extern volatile uint32_t COB_FlashJedecNcs1Word0;
extern volatile uint32_t COB_FlashJedecNcs1Word1;
extern volatile int32_t COB_FlashJedecNcs2CommandStatus;
extern volatile int32_t COB_FlashJedecNcs2ReceiveStatus;
extern volatile uint32_t COB_FlashJedecNcs2Word0;
extern volatile uint32_t COB_FlashJedecNcs2Word1;
extern volatile int32_t COB_FlashSfdpNcs1CommandStatus;
extern volatile int32_t COB_FlashSfdpNcs1ReceiveStatus;
extern volatile uint32_t COB_FlashSfdpNcs1Word0;
extern volatile int32_t COB_FlashSfdpNcs2CommandStatus;
extern volatile int32_t COB_FlashSfdpNcs2ReceiveStatus;
extern volatile uint32_t COB_FlashSfdpNcs2Word0;
extern volatile uint32_t COB_FlashXspi2ErrorCode;
extern volatile uint32_t COB_FlashXspi2State;
extern volatile uint32_t COB_PsramTestStage;
extern volatile uint32_t COB_PsramTestPassed;
extern volatile uint32_t COB_PsramTestBaseAddress;
extern volatile uint32_t COB_PsramTestValue;
extern volatile uint32_t COB_PsramTestErrors;
extern volatile uint32_t COB_PsramTestFirstBadIndex;
extern volatile uint32_t COB_PsramTestExpected;
extern volatile uint32_t COB_PsramTestActual;
extern volatile int32_t COB_PsramTestLastStatus;
extern volatile int32_t COB_PsramMapStatus;
extern volatile int32_t COB_PsramWrapStatus;
extern volatile int32_t COB_PsramEnableMapStatus;
extern volatile uint32_t COB_PsramXspi1ErrorCode;
extern volatile uint32_t COB_PsramXspi1State;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void COB_LedThreadEntry(ULONG thread_input);
static void COB_FlashThreadEntry(ULONG thread_input);
static void COB_PsramThreadEntry(ULONG thread_input);
static void COB_ProbeFlashJedecId(uint32_t chip_select);
static HAL_StatusTypeDef COB_XSPI_SendSimpleCommand(uint32_t chip_select, uint32_t instruction);
static void COB_ProbeFlashSfdpSignature(uint32_t chip_select);
static uint32_t COB_GenerateTestValue(void);
static uint32_t COB_RunFlashSelfTest(void);
static uint32_t COB_RunPsramSelfTest(void);

/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  /* USER CODE END App_ThreadX_MEM_POOL */
  /* USER CODE BEGIN App_ThreadX_Init */
  (void)memory_ptr;

  ret = tx_thread_create(&COB_LedThread,
                         "COB LED",
                         COB_LedThreadEntry,
                         0U,
                         COB_LedThreadStack,
                         sizeof(COB_LedThreadStack),
                         COB_LED_THREAD_PRIORITY,
                         COB_LED_THREAD_PRIORITY,
                         TX_NO_TIME_SLICE,
                         TX_AUTO_START);
  if (ret == TX_SUCCESS)
  {
    ret = tx_thread_create(&COB_FlashThread,
                           "COB Flash Test",
                           COB_FlashThreadEntry,
                           0U,
                           COB_FlashThreadStack,
                           sizeof(COB_FlashThreadStack),
                           COB_FLASH_THREAD_PRIORITY,
                           COB_FLASH_THREAD_PRIORITY,
                           TX_NO_TIME_SLICE,
                           TX_AUTO_START);
  }
  if (ret == TX_SUCCESS)
  {
    ret = tx_thread_create(&COB_PsramThread,
                           "COB PSRAM Test",
                           COB_PsramThreadEntry,
                           0U,
                           COB_PsramThreadStack,
                           sizeof(COB_PsramThreadStack),
                           COB_PSRAM_THREAD_PRIORITY,
                           COB_PSRAM_THREAD_PRIORITY,
                           TX_NO_TIME_SLICE,
                           TX_AUTO_START);
  }
  /* USER CODE END App_ThreadX_Init */

  return ret;
}

  /**
  * @brief  Function that implements the kernel's initialization.
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN Before_Kernel_Start */

  /* USER CODE END Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN Kernel_Start_Error */

  /* USER CODE END Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */
static HAL_StatusTypeDef COB_XSPI_SendSimpleCommand(uint32_t chip_select, uint32_t instruction)
{
  XSPI_RegularCmdTypeDef command = {0};
  HAL_StatusTypeDef status;

  (void)HAL_XSPI_Abort(&hxspi2);
  MODIFY_REG(hxspi2.Instance->CR, XSPI_CR_CSSEL, chip_select);

  command.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  command.IOSelect = HAL_XSPI_SELECT_IO_7_0;
  command.Instruction = instruction;
  command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
  command.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  command.AddressMode = HAL_XSPI_ADDRESS_NONE;
  command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  command.DataMode = HAL_XSPI_DATA_NONE;
  command.DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE;
  command.DummyCycles = 0U;
  command.DQSMode = HAL_XSPI_DQS_DISABLE;

  status = HAL_XSPI_Command(&hxspi2, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  COB_FlashXspi2ErrorCode = hxspi2.ErrorCode;
  COB_FlashXspi2State = (uint32_t)hxspi2.State;

  return status;
}

static void COB_ProbeFlashJedecId(uint32_t chip_select)
{
  XSPI_RegularCmdTypeDef command = {0};
  uint8_t id[8] = {0};
  HAL_StatusTypeDef command_status;
  HAL_StatusTypeDef receive_status = HAL_ERROR;

  (void)HAL_XSPI_Abort(&hxspi2);
  MODIFY_REG(hxspi2.Instance->CR, XSPI_CR_CSSEL, chip_select);

  command.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  command.IOSelect = HAL_XSPI_SELECT_IO_7_0;
  command.Instruction = 0x9FU;
  command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
  command.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  command.AddressMode = HAL_XSPI_ADDRESS_NONE;
  command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  command.DataMode = HAL_XSPI_DATA_1_LINE;
  command.DataLength = 6U;
  command.DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE;
  command.DummyCycles = 0U;
  command.DQSMode = HAL_XSPI_DQS_DISABLE;

  command_status = HAL_XSPI_Command(&hxspi2, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  if (command_status == HAL_OK)
  {
    receive_status = HAL_XSPI_Receive(&hxspi2, id, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }
  COB_FlashXspi2ErrorCode = hxspi2.ErrorCode;
  COB_FlashXspi2State = (uint32_t)hxspi2.State;

  if (chip_select == HAL_XSPI_CSSEL_NCS1)
  {
    COB_FlashJedecNcs1CommandStatus = (int32_t)command_status;
    COB_FlashJedecNcs1ReceiveStatus = (int32_t)receive_status;
    COB_FlashJedecNcs1Word0 = ((uint32_t)id[0] << 24) | ((uint32_t)id[1] << 16) |
                              ((uint32_t)id[2] << 8) | (uint32_t)id[3];
    COB_FlashJedecNcs1Word1 = ((uint32_t)id[4] << 8) | (uint32_t)id[5];
  }
  else
  {
    COB_FlashJedecNcs2CommandStatus = (int32_t)command_status;
    COB_FlashJedecNcs2ReceiveStatus = (int32_t)receive_status;
    COB_FlashJedecNcs2Word0 = ((uint32_t)id[0] << 24) | ((uint32_t)id[1] << 16) |
                              ((uint32_t)id[2] << 8) | (uint32_t)id[3];
    COB_FlashJedecNcs2Word1 = ((uint32_t)id[4] << 8) | (uint32_t)id[5];
  }
}

static void COB_ProbeFlashSfdpSignature(uint32_t chip_select)
{
  XSPI_RegularCmdTypeDef command = {0};
  uint8_t sfdp[4] = {0};
  HAL_StatusTypeDef command_status;
  HAL_StatusTypeDef receive_status = HAL_ERROR;

  (void)HAL_XSPI_Abort(&hxspi2);
  MODIFY_REG(hxspi2.Instance->CR, XSPI_CR_CSSEL, chip_select);

  command.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  command.IOSelect = HAL_XSPI_SELECT_IO_7_0;
  command.Instruction = 0x5AU;
  command.InstructionMode = HAL_XSPI_INSTRUCTION_1_LINE;
  command.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS;
  command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_DISABLE;
  command.AddressMode = HAL_XSPI_ADDRESS_1_LINE;
  command.AddressWidth = HAL_XSPI_ADDRESS_24_BITS;
  command.Address = 0U;
  command.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_DISABLE;
  command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  command.DataMode = HAL_XSPI_DATA_1_LINE;
  command.DataLength = sizeof(sfdp);
  command.DataDTRMode = HAL_XSPI_DATA_DTR_DISABLE;
  command.DummyCycles = 8U;
  command.DQSMode = HAL_XSPI_DQS_DISABLE;

  command_status = HAL_XSPI_Command(&hxspi2, &command, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  if (command_status == HAL_OK)
  {
    receive_status = HAL_XSPI_Receive(&hxspi2, sfdp, HAL_XSPI_TIMEOUT_DEFAULT_VALUE);
  }
  COB_FlashXspi2ErrorCode = hxspi2.ErrorCode;
  COB_FlashXspi2State = (uint32_t)hxspi2.State;

  if (chip_select == HAL_XSPI_CSSEL_NCS1)
  {
    COB_FlashSfdpNcs1CommandStatus = (int32_t)command_status;
    COB_FlashSfdpNcs1ReceiveStatus = (int32_t)receive_status;
    COB_FlashSfdpNcs1Word0 = ((uint32_t)sfdp[0] << 24) | ((uint32_t)sfdp[1] << 16) |
                              ((uint32_t)sfdp[2] << 8) | (uint32_t)sfdp[3];
  }
  else
  {
    COB_FlashSfdpNcs2CommandStatus = (int32_t)command_status;
    COB_FlashSfdpNcs2ReceiveStatus = (int32_t)receive_status;
    COB_FlashSfdpNcs2Word0 = ((uint32_t)sfdp[0] << 24) | ((uint32_t)sfdp[1] << 16) |
                              ((uint32_t)sfdp[2] << 8) | (uint32_t)sfdp[3];
  }
}

static uint32_t COB_GenerateTestValue(void)
{
  static uint32_t seed = 0x434F4201U;

  seed ^= HAL_GetTick();
  seed = (seed * 1664525U) + 1013904223U;
  seed ^= (uint32_t)&seed;

  return seed;
}

static uint32_t COB_RunFlashSelfTest(void)
{
  EXTMEM_NOR_SFDP_FlashInfoTypeDef info = {0};
  uint8_t write_buffer[64] = {0};
  uint8_t read_buffer[sizeof(write_buffer)] = {0};
  uint32_t flash_size;
  uint32_t erase_size;
  int length;
  EXTMEM_StatusTypeDef status;
  EXTMEM_DRIVER_NOR_SFDP_StatusTypeDef driver_status;

  COB_FlashTestStage = 1U;
  (void)COB_XSPI_SendSimpleCommand(HAL_XSPI_CSSEL_NCS1, 0xABU);
  (void)COB_XSPI_SendSimpleCommand(HAL_XSPI_CSSEL_NCS1, 0x66U);
  (void)COB_XSPI_SendSimpleCommand(HAL_XSPI_CSSEL_NCS1, 0x99U);
  tx_thread_sleep(2U);
  (void)COB_XSPI_SendSimpleCommand(HAL_XSPI_CSSEL_NCS2, 0xABU);
  (void)COB_XSPI_SendSimpleCommand(HAL_XSPI_CSSEL_NCS2, 0x66U);
  (void)COB_XSPI_SendSimpleCommand(HAL_XSPI_CSSEL_NCS2, 0x99U);
  tx_thread_sleep(2U);
  COB_ProbeFlashJedecId(HAL_XSPI_CSSEL_NCS1);
  COB_ProbeFlashJedecId(HAL_XSPI_CSSEL_NCS2);
  COB_ProbeFlashSfdpSignature(HAL_XSPI_CSSEL_NCS1);
  COB_ProbeFlashSfdpSignature(HAL_XSPI_CSSEL_NCS2);
  MODIFY_REG(hxspi2.Instance->CR, XSPI_CR_CSSEL, hxspi2.Init.MemorySelect);

  driver_status = EXTMEM_DRIVER_NOR_SFDP_Init(extmem_list_config[EXTMEMORY_2].Handle,
                                              extmem_list_config[EXTMEMORY_2].ConfigType,
                                              HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_XSPI2),
                                              &extmem_list_config[EXTMEMORY_2].NorSfdpObject);
  COB_FlashDriverStatus = (int32_t)driver_status;
  COB_FlashTestLastStatus = (driver_status == EXTMEM_DRIVER_NOR_SFDP_OK) ? 0 : EXTMEM_ERROR_DRIVER;
  if (driver_status != EXTMEM_DRIVER_NOR_SFDP_OK)
  {
    COB_FlashTestStage = 10U;
    printf("FLASH TEST FAIL: NOR SFDP init driver_status=%ld\r\n", (long)driver_status);
    return 0U;
  }

  COB_FlashTestStage = 2U;
  status = EXTMEM_GetInfo(EXTMEMORY_2, &info);
  COB_FlashTestLastStatus = (int32_t)status;
  COB_FlashInfoSizePower = info.FlashSize;
  COB_FlashInfoPageSize = info.PageSize;
  COB_FlashInfoErase1SizePower = info.EraseType1Size;
  COB_FlashInfoErase2SizePower = info.EraseType2Size;
  COB_FlashInfoErase3SizePower = info.EraseType3Size;
  COB_FlashInfoErase4SizePower = info.EraseType4Size;
  if (status != EXTMEM_OK)
  {
    COB_FlashTestStage = 11U;
    printf("FLASH TEST FAIL: get info status=%ld flash_size_pow=%u\r\n",
           (long)status, info.FlashSize);
    return 0U;
  }
  if ((info.FlashSize == 0U) || (info.FlashSize >= 31U))
  {
    COB_FlashTestStage = 12U;
    printf("FLASH TEST FAIL: invalid flash_size_pow=%u\r\n", info.FlashSize);
    return 0U;
  }

  flash_size = 1UL << info.FlashSize;
  if (flash_size > COB_FLASH_TEST_CAPACITY_BYTES)
  {
    flash_size = COB_FLASH_TEST_CAPACITY_BYTES;
  }

  erase_size = info.EraseType1Size;
  if ((erase_size == 0U) || ((info.EraseType2Size != 0U) && (info.EraseType2Size < erase_size)))
  {
    erase_size = info.EraseType2Size;
  }
  if ((erase_size == 0U) || ((info.EraseType3Size != 0U) && (info.EraseType3Size < erase_size)))
  {
    erase_size = info.EraseType3Size;
  }
  if ((erase_size == 0U) || ((info.EraseType4Size != 0U) && (info.EraseType4Size < erase_size)))
  {
    erase_size = info.EraseType4Size;
  }
  if ((erase_size == 0U) || (erase_size > flash_size))
  {
    COB_FlashTestStage = 13U;
    printf("FLASH TEST FAIL: invalid erase_size=%lu flash_size=%lu\r\n",
           (unsigned long)erase_size, (unsigned long)flash_size);
    return 0U;
  }

  COB_FlashTestAddress = flash_size - erase_size;
  COB_FlashTestValue = COB_GenerateTestValue();
  length = snprintf((char *)write_buffer, sizeof(write_buffer),
                    "COB flash test: %lu\r\n",
                    (unsigned long)COB_FlashTestValue);
  if ((length <= 0) || ((uint32_t)length >= sizeof(write_buffer)))
  {
    printf("FLASH TEST FAIL: snprintf length=%d\r\n", length);
    return 0U;
  }

  COB_FlashTestStage = 20U;
  status = EXTMEM_EraseSector(EXTMEMORY_2, COB_FlashTestAddress, erase_size);
  COB_FlashTestLastStatus = (int32_t)status;
  if (status != EXTMEM_OK)
  {
    printf("FLASH TEST FAIL: erase status=%ld addr=0x%08lX size=%lu\r\n",
           (long)status,
           (unsigned long)COB_FlashTestAddress,
           (unsigned long)erase_size);
    return 0U;
  }

  COB_FlashTestStage = 30U;
  status = EXTMEM_Write(EXTMEMORY_2, COB_FlashTestAddress, write_buffer, sizeof(write_buffer));
  COB_FlashTestLastStatus = (int32_t)status;
  if (status != EXTMEM_OK)
  {
    printf("FLASH TEST FAIL: write status=%ld addr=0x%08lX\r\n",
           (long)status,
           (unsigned long)COB_FlashTestAddress);
    return 0U;
  }

  COB_FlashTestStage = 40U;
  status = EXTMEM_Read(EXTMEMORY_2, COB_FlashTestAddress, read_buffer, sizeof(read_buffer));
  COB_FlashTestLastStatus = (int32_t)status;
  if (status != EXTMEM_OK)
  {
    printf("FLASH TEST FAIL: read status=%ld addr=0x%08lX\r\n",
           (long)status,
           (unsigned long)COB_FlashTestAddress);
    return 0U;
  }

  COB_FlashTestStage = 50U;
  if (memcmp(write_buffer, read_buffer, sizeof(write_buffer)) != 0)
  {
    printf("FLASH TEST FAIL: compare addr=0x%08lX wrote=\"%s\" read=\"%s\"\r\n",
           (unsigned long)COB_FlashTestAddress,
           (char *)write_buffer,
           (char *)read_buffer);
    return 0U;
  }

  COB_FlashTestStage = 100U;
  printf("FLASH TEST OK: value=%lu addr=0x%08lX text=\"%s\"\r\n",
         (unsigned long)COB_FlashTestValue,
         (unsigned long)COB_FlashTestAddress,
         (char *)read_buffer);
  return 1U;
}

static uint32_t COB_RunPsramSelfTest(void)
{
  volatile uint32_t *psram;
  EXTMEM_StatusTypeDef status;
  uint32_t base_address = 0U;
  uint32_t errors = 0U;

  /*
   * PSRAM debug safety:
   * Do not replace the SAL_XSPI_EnableMapMode() call below with
   * HAL_XSPI_HyperbusCmd() + HAL_XSPI_MemoryMapped() and then access
   * XSPI1_BASE from this auto-start ThreadX test.
   *
   * That was tested and can hang the target before debugger watchers are read.
   * The dangerous part is not the HAL call itself, but the first CPU read/write
   * through the memory-mapped XSPI1 window and the following __DSB(). If the
   * PSRAM HyperBus timing/RWDS/latency is wrong, the AXI transaction may never
   * complete. After that ST-LINK/GDB may not reconnect until board power is
   * cycled.
   *
   * If real memory-mapped PSRAM testing is needed, keep it behind a manual
   * debugger flag, use a timeout-capable configuration, disable prefetch, touch
   * only one word first, and always abort/disable memory-mapped mode before
   * leaving the test.
   */
  COB_PsramTestStage = 1U;
  COB_PsramMapStatus = (int32_t)EXTMEM_MemoryMappedMode(EXTMEMORY_1, EXTMEM_DISABLE);
  COB_PsramWrapStatus = (int32_t)HAL_OK;

  COB_PsramTestStage = 3U;
  COB_PsramEnableMapStatus = (int32_t)SAL_XSPI_EnableMapMode(&extmem_list_config[EXTMEMORY_1].PsramObject.psram_private.SALObject,
                                                             extmem_list_config[EXTMEMORY_1].PsramObject.psram_public.Read_command,
                                                             extmem_list_config[EXTMEMORY_1].PsramObject.psram_public.Read_DummyCycle,
                                                             extmem_list_config[EXTMEMORY_1].PsramObject.psram_public.Write_command,
                                                             extmem_list_config[EXTMEMORY_1].PsramObject.psram_public.Write_DummyCycle);
  COB_PsramXspi1ErrorCode = hxspi1.ErrorCode;
  COB_PsramXspi1State = (uint32_t)hxspi1.State;
  if (COB_PsramEnableMapStatus != (int32_t)HAL_OK)
  {
    COB_PsramTestStage = 12U;
    COB_PsramTestLastStatus = COB_PsramEnableMapStatus;
    printf("PSRAM TEST FAIL: enable map status=%ld xspi_error=0x%08lX state=%lu\r\n",
           (long)COB_PsramEnableMapStatus,
           (unsigned long)COB_PsramXspi1ErrorCode,
           (unsigned long)COB_PsramXspi1State);
    return 0U;
  }
  COB_PsramTestLastStatus = EXTMEM_OK;

  COB_PsramTestStage = 2U;
  status = EXTMEM_GetMapAddress(EXTMEMORY_1, &base_address);
  COB_PsramTestLastStatus = (int32_t)status;
  COB_PsramTestBaseAddress = base_address;
  if ((status != EXTMEM_OK) || (base_address == 0U))
  {
    COB_PsramTestStage = 11U;
    printf("PSRAM TEST FAIL: get map address status=%ld base=0x%08lX\r\n",
           (long)status,
           (unsigned long)base_address);
    return 0U;
  }

  psram = (volatile uint32_t *)base_address;
  COB_PsramTestValue = 0x5A5AA5A5UL ^ HAL_GetTick();

  /*
   * This block must only run after memory-mapped PSRAM has been proven safe.
   * A failed or half-configured XSPI1 memory-mapped mode can hang on the write
   * below or on the __DSB(), which makes the debugger appear stuck.
   */
  COB_PsramTestStage = 20U;
  for (uint32_t i = 0U; i < COB_PSRAM_TEST_WORD_COUNT; i++)
  {
    psram[i] = 0xA5A50000UL ^ i ^ COB_PsramTestValue;
  }

  __DSB();

  COB_PsramTestStage = 30U;
  for (uint32_t i = 0U; i < COB_PSRAM_TEST_WORD_COUNT; i++)
  {
    uint32_t expected = 0xA5A50000UL ^ i ^ COB_PsramTestValue;
    uint32_t actual = psram[i];

    if (actual != expected)
    {
      if (errors == 0U)
      {
        COB_PsramTestFirstBadIndex = i;
        COB_PsramTestExpected = expected;
        COB_PsramTestActual = actual;
      }
      errors++;
    }
  }

  COB_PsramTestErrors = errors;
  if (errors != 0U)
  {
    COB_PsramTestStage = 40U;
    printf("PSRAM TEST FAIL: errors=%lu first=%lu expected=0x%08lX actual=0x%08lX base=0x%08lX\r\n",
           (unsigned long)errors,
           (unsigned long)COB_PsramTestFirstBadIndex,
           (unsigned long)COB_PsramTestExpected,
           (unsigned long)COB_PsramTestActual,
           (unsigned long)COB_PsramTestBaseAddress);
    return 0U;
  }

  COB_PsramTestStage = 100U;
  printf("PSRAM TEST OK: base=0x%08lX words=%lu\r\n",
         (unsigned long)COB_PsramTestBaseAddress,
         (unsigned long)COB_PSRAM_TEST_WORD_COUNT);
  return 1U;
}

static void COB_FlashThreadEntry(ULONG thread_input)
{
  (void)thread_input;

  COB_FlashTestPassed = COB_RunFlashSelfTest();

  while (1)
  {
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }
}

static void COB_PsramThreadEntry(ULONG thread_input)
{
  (void)thread_input;

  COB_PsramTestPassed = COB_RunPsramSelfTest();

  while (1)
  {
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND);
  }
}

static void COB_LedThreadEntry(ULONG thread_input)
{
  (void)thread_input;

  while (1)
  {
    if ((COB_FlashTestPassed != 0U) && (COB_PsramTestPassed != 0U))
    {
      COB_StatusLED_TestPassToggle();
    }
    else
    {
      COB_StatusLED_TestFailToggle();
    }

    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2U);
  }
}

/* USER CODE END 1 */

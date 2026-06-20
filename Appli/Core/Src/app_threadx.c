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
#define COB_LED_THREAD_PRIORITY   15U
#define COB_FLASH_THREAD_PRIORITY 10U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static TX_THREAD COB_LedThread;
static TX_THREAD COB_FlashThread;
static ULONG COB_LedThreadStack[COB_LED_THREAD_STACK_SIZE / sizeof(ULONG)];
static ULONG COB_FlashThreadStack[COB_FLASH_THREAD_STACK_SIZE / sizeof(ULONG)];
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void COB_LedThreadEntry(ULONG thread_input);
static void COB_FlashThreadEntry(ULONG thread_input);
static uint32_t COB_GenerateTestValue(void);
static uint32_t COB_RunFlashSelfTest(void);

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

static void COB_FlashThreadEntry(ULONG thread_input)
{
  (void)thread_input;

  COB_FlashTestPassed = COB_RunFlashSelfTest();

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
    if (COB_FlashTestPassed != 0U)
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

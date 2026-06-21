/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    cob_psram.c
  * @brief   PSRAM indirect read/write helpers.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "cob_psram.h"
#include "xspi.h"

#define COB_PSRAM_TIMEOUT_MS 100U

static HAL_StatusTypeDef COB_PSRAM_Command(uint32_t address, uint32_t size, uint32_t address_space)
{
  XSPI_HyperbusCmdTypeDef command = {0};
  HAL_StatusTypeDef status;

  if (size == 0U)
  {
    return HAL_ERROR;
  }

  if (HAL_XSPI_IsMemoryMapped(&hxspi1) != 0U)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  command.AddressSpace = address_space;
  command.Address = address;
  command.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  command.DataLength = size;
  command.DQSMode = HAL_XSPI_DQS_ENABLE;
  command.DataMode = HAL_XSPI_DATA_8_LINES;

  hxspi1.ErrorCode = HAL_XSPI_ERROR_NONE;
  status = HAL_XSPI_HyperbusCmd(&hxspi1, &command, COB_PSRAM_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_Write(uint32_t address, const uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  status = COB_PSRAM_Command(address, size, HAL_XSPI_MEMORY_ADDRESS_SPACE);
  if (status == HAL_OK)
  {
    status = HAL_XSPI_Transmit(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_Read(uint32_t address, uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  status = COB_PSRAM_Command(address, size, HAL_XSPI_MEMORY_ADDRESS_SPACE);
  if (status == HAL_OK)
  {
    status = HAL_XSPI_Receive(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_ReadRegister(uint32_t address, uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  status = COB_PSRAM_Command(address, size, HAL_XSPI_REGISTER_ADDRESS_SPACE);
  if (status == HAL_OK)
  {
    status = HAL_XSPI_Receive(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

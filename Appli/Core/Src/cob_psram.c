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

extern volatile uint32_t COB_PsramIoStage;
extern volatile uint32_t COB_PsramXspi1Cr;
extern volatile uint32_t COB_PsramXspi1Sr;
extern volatile uint32_t COB_PsramXspi1ErrorCode;
extern volatile uint32_t COB_PsramXspi1State;

#define COB_PSRAM_TIMEOUT_MS 100U
#define COB_PSRAM_READ_ALT_CMD 0x00U
#define COB_PSRAM_WRITE_ALT_CMD 0x80U
#define COB_PSRAM_READ_CMD 0x20U
#define COB_PSRAM_WRITE_CMD 0xA0U
#define COB_PSRAM_READ_REG_CMD 0x40U
#define COB_PSRAM_READ_DUMMY_CYCLES 4U
#define COB_PSRAM_WRITE_DUMMY_CYCLES 0U

static uint32_t COB_PSRAM_Instruction(uint32_t opcode)
{
  return ((opcode & 0xFFU) << 8) | ((~opcode) & 0xFFU);
}

static HAL_StatusTypeDef COB_PSRAM_CommandEx(uint32_t instruction, uint32_t address,
                                             uint32_t size, uint32_t dummy_cycles,
                                             uint32_t dqs_mode)
{
  XSPI_RegularCmdTypeDef command = {0};
  HAL_StatusTypeDef status;

  if (size == 0U)
  {
    return HAL_ERROR;
  }

  if (HAL_XSPI_IsMemoryMapped(&hxspi1) != 0U)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  command.OperationType = HAL_XSPI_OPTYPE_COMMON_CFG;
  command.IOSelect = HAL_XSPI_SELECT_IO_7_0;
  command.Instruction = COB_PSRAM_Instruction(instruction);
  command.InstructionMode = HAL_XSPI_INSTRUCTION_8_LINES;
  command.InstructionWidth = HAL_XSPI_INSTRUCTION_16_BITS;
  command.InstructionDTRMode = HAL_XSPI_INSTRUCTION_DTR_ENABLE;
  command.AddressMode = HAL_XSPI_ADDRESS_8_LINES;
  command.Address = address;
  command.AddressWidth = HAL_XSPI_ADDRESS_32_BITS;
  command.AddressDTRMode = HAL_XSPI_ADDRESS_DTR_ENABLE;
  command.AlternateBytesMode = HAL_XSPI_ALT_BYTES_NONE;
  command.DataMode = HAL_XSPI_DATA_8_LINES;
  command.DataLength = size;
  command.DataDTRMode = HAL_XSPI_DATA_DTR_ENABLE;
  command.DummyCycles = dummy_cycles;
  command.DQSMode = dqs_mode;

  COB_PsramXspi1Cr = hxspi1.Instance->CR;
  COB_PsramXspi1Sr = hxspi1.Instance->SR;
  COB_PsramXspi1ErrorCode = hxspi1.ErrorCode;
  COB_PsramXspi1State = (uint32_t)hxspi1.State;

  hxspi1.ErrorCode = HAL_XSPI_ERROR_NONE;
  status = HAL_XSPI_Command(&hxspi1, &command, COB_PSRAM_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

static HAL_StatusTypeDef COB_PSRAM_Command(uint32_t instruction, uint32_t address,
                                           uint32_t size, uint32_t dummy_cycles)
{
  return COB_PSRAM_CommandEx(instruction, address, size, dummy_cycles, HAL_XSPI_DQS_ENABLE);
}

HAL_StatusTypeDef COB_PSRAM_Write(uint32_t address, const uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  COB_PsramIoStage = 100U;
  status = COB_PSRAM_CommandEx(COB_PSRAM_WRITE_CMD,
                               address,
                               size,
                               COB_PSRAM_WRITE_DUMMY_CYCLES,
                               HAL_XSPI_DQS_DISABLE);
  COB_PsramIoStage = 101U;
  if (status == HAL_OK)
  {
    COB_PsramIoStage = 102U;
    status = HAL_XSPI_Transmit(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
    COB_PsramIoStage = 103U;
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_Read(uint32_t address, uint8_t *data, uint32_t size)
{
  return COB_PSRAM_ReadWithDummy(address, data, size, COB_PSRAM_READ_DUMMY_CYCLES);
}

HAL_StatusTypeDef COB_PSRAM_ReadWithDummy(uint32_t address, uint8_t *data, uint32_t size, uint32_t dummy_cycles)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  COB_PsramIoStage = 200U + dummy_cycles;
  status = COB_PSRAM_Command(COB_PSRAM_READ_CMD, address, size, dummy_cycles);
  COB_PsramIoStage = 210U + dummy_cycles;
  if (status == HAL_OK)
  {
    COB_PsramIoStage = 220U + dummy_cycles;
    status = HAL_XSPI_Receive(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
    COB_PsramIoStage = 230U + dummy_cycles;
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_ReadNoDqs(uint32_t address, uint8_t *data, uint32_t size)
{
  return COB_PSRAM_ReadNoDqsWithDummy(address, data, size, COB_PSRAM_READ_DUMMY_CYCLES);
}

HAL_StatusTypeDef COB_PSRAM_ReadNoDqsWithDummy(uint32_t address, uint8_t *data, uint32_t size, uint32_t dummy_cycles)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  COB_PsramIoStage = 300U + dummy_cycles;
  status = COB_PSRAM_CommandEx(COB_PSRAM_READ_CMD,
                               address,
                               size,
                               dummy_cycles,
                               HAL_XSPI_DQS_DISABLE);
  COB_PsramIoStage = 310U + dummy_cycles;
  if (status == HAL_OK)
  {
    COB_PsramIoStage = 320U + dummy_cycles;
    status = HAL_XSPI_Receive(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
    COB_PsramIoStage = 330U + dummy_cycles;
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_WriteAlt(uint32_t address, const uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  COB_PsramIoStage = 400U;
  status = COB_PSRAM_CommandEx(COB_PSRAM_WRITE_ALT_CMD,
                               address,
                               size,
                               COB_PSRAM_WRITE_DUMMY_CYCLES,
                               HAL_XSPI_DQS_DISABLE);
  COB_PsramIoStage = 401U;
  if (status == HAL_OK)
  {
    COB_PsramIoStage = 402U;
    status = HAL_XSPI_Transmit(&hxspi1, data, COB_PSRAM_TIMEOUT_MS);
    COB_PsramIoStage = 403U;
  }

  if (status != HAL_OK)
  {
    (void)HAL_XSPI_Abort(&hxspi1);
  }

  return status;
}

HAL_StatusTypeDef COB_PSRAM_ReadAlt(uint32_t address, uint8_t *data, uint32_t size)
{
  HAL_StatusTypeDef status;

  if (data == NULL)
  {
    return HAL_ERROR;
  }

  status = COB_PSRAM_Command(COB_PSRAM_READ_ALT_CMD, address, size, COB_PSRAM_READ_DUMMY_CYCLES);
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

  status = COB_PSRAM_Command(COB_PSRAM_READ_REG_CMD, address, size, COB_PSRAM_READ_DUMMY_CYCLES);
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

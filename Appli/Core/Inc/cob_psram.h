/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    cob_psram.h
  * @brief   PSRAM indirect read/write helpers.
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef COB_PSRAM_H
#define COB_PSRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32n6xx_hal.h"
#include <stdint.h>

HAL_StatusTypeDef COB_PSRAM_Write(uint32_t address, const uint8_t *data, uint32_t size);
HAL_StatusTypeDef COB_PSRAM_Read(uint32_t address, uint8_t *data, uint32_t size);
HAL_StatusTypeDef COB_PSRAM_ReadNoDqs(uint32_t address, uint8_t *data, uint32_t size);
HAL_StatusTypeDef COB_PSRAM_WriteAlt(uint32_t address, const uint8_t *data, uint32_t size);
HAL_StatusTypeDef COB_PSRAM_ReadAlt(uint32_t address, uint8_t *data, uint32_t size);
HAL_StatusTypeDef COB_PSRAM_ReadRegister(uint32_t address, uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* COB_PSRAM_H */

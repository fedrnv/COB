#include "cob_config.h"

#include "stm32_extmem.h"
#include "stm32_extmem_conf.h"
#include "stm32n6xx_hal.h"

#include <string.h>

#define COB_CONFIG_MAGIC          0x43464743UL /* "CFGC" */
#define COB_CONFIG_VERSION        1U
#define COB_CONFIG_MIN_ERASE_SIZE 4096UL

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t length;
  uint32_t crc32;
  uint32_t reserved;
  COB_Config_t config;
} COB_ConfigStorageRecord_t;

static COB_Config_t cob_config;
static bool cob_config_ram_valid;
static uint32_t cob_config_flash_address;
static uint32_t cob_config_flash_erase_size;

volatile uint32_t COB_ConfigStage = 0U;
volatile int32_t COB_ConfigLastStatus = 0;
volatile uint32_t COB_ConfigFlashAddress = 0U;
volatile uint32_t COB_ConfigFlashEraseSize = 0U;
volatile uint32_t COB_ConfigLoadedFromFlash = 0U;
volatile uint32_t COB_ConfigUsingDefaults = 0U;
volatile uint32_t COB_ConfigSavedToFlash = 0U;

static bool COB_Config_LoadFromFlash(COB_Config_t *config);
static void COB_Config_LoadDefaults(COB_Config_t *config);
static bool COB_Config_GetFlashLayout(uint32_t *address, uint32_t *erase_size);
static uint32_t COB_Config_SelectEraseSize(const EXTMEM_NOR_SFDP_FlashInfoTypeDef *info);
static bool COB_Config_IsValid(const COB_Config_t *config);
static uint32_t COB_Config_Crc32(const uint8_t *data, uint32_t length);
static uint32_t COB_Config_HashUid(void);

void COB_Config_Init(void)
{
  COB_ConfigStage = 1U;
  if (cob_config_ram_valid)
  {
    return;
  }

  COB_ConfigLoadedFromFlash = 0U;
  COB_ConfigUsingDefaults = 0U;
  COB_ConfigSavedToFlash = 0U;

  if (COB_Config_LoadFromFlash(&cob_config))
  {
    COB_ConfigLoadedFromFlash = 1U;
    cob_config_ram_valid = true;
    COB_ConfigStage = 10U;
    return;
  }

  COB_Config_LoadDefaults(&cob_config);
  COB_ConfigUsingDefaults = 1U;
  cob_config_ram_valid = true;
  COB_ConfigStage = 20U;
}

bool COB_Config_IsRamValid(void)
{
  return cob_config_ram_valid;
}

const COB_Config_t *COB_Config_Get(void)
{
  if (!cob_config_ram_valid)
  {
    COB_Config_Init();
  }

  return &cob_config;
}

uint32_t COB_Config_GetIpAddressU32(void)
{
  const COB_Config_t *config = COB_Config_Get();

  return ((uint32_t)config->addresses.ip_address[0] << 24) |
         ((uint32_t)config->addresses.ip_address[1] << 16) |
         ((uint32_t)config->addresses.ip_address[2] << 8) |
         (uint32_t)config->addresses.ip_address[3];
}

void COB_Config_GetMacAddress(uint8_t mac_address[COB_CONFIG_MAC_OCTET_COUNT])
{
  const COB_Config_t *config = COB_Config_Get();

  memcpy(mac_address, config->addresses.mac_address, COB_CONFIG_MAC_OCTET_COUNT);
}

bool COB_Config_SaveToFlash(void)
{
  COB_ConfigStorageRecord_t record;
  EXTMEM_StatusTypeDef status;
  uint32_t address;
  uint32_t erase_size;

  if (!cob_config_ram_valid)
  {
    return false;
  }

  COB_ConfigStage = 30U;
  if (!COB_Config_GetFlashLayout(&address, &erase_size))
  {
    return false;
  }

  memset(&record, 0xFF, sizeof(record));
  record.magic = COB_CONFIG_MAGIC;
  record.version = COB_CONFIG_VERSION;
  record.length = (uint16_t)sizeof(record.config);
  record.config = cob_config;
  record.crc32 = COB_Config_Crc32((const uint8_t *)&record.config, sizeof(record.config));

  status = EXTMEM_EraseSector(EXTMEMORY_2, address, erase_size);
  COB_ConfigLastStatus = (int32_t)status;
  if (status != EXTMEM_OK)
  {
    COB_ConfigStage = 31U;
    return false;
  }

  status = EXTMEM_Write(EXTMEMORY_2, address, (const uint8_t *)&record, sizeof(record));
  COB_ConfigLastStatus = (int32_t)status;
  if (status != EXTMEM_OK)
  {
    COB_ConfigStage = 32U;
    return false;
  }

  COB_ConfigSavedToFlash = 1U;
  COB_ConfigStage = 33U;
  return true;
}

static bool COB_Config_LoadFromFlash(COB_Config_t *config)
{
  COB_ConfigStorageRecord_t record;
  EXTMEM_StatusTypeDef status;
  uint32_t address;
  uint32_t erase_size;
  uint32_t crc32;

  COB_ConfigStage = 2U;
  if (!COB_Config_GetFlashLayout(&address, &erase_size))
  {
    return false;
  }
  (void)erase_size;

  memset(&record, 0, sizeof(record));
  status = EXTMEM_Read(EXTMEMORY_2, address, (uint8_t *)&record, sizeof(record));
  COB_ConfigLastStatus = (int32_t)status;
  if (status != EXTMEM_OK)
  {
    COB_ConfigStage = 3U;
    return false;
  }

  if ((record.magic != COB_CONFIG_MAGIC) ||
      (record.version != COB_CONFIG_VERSION) ||
      (record.length != sizeof(record.config)))
  {
    COB_ConfigStage = 4U;
    return false;
  }

  crc32 = COB_Config_Crc32((const uint8_t *)&record.config, sizeof(record.config));
  if (crc32 != record.crc32)
  {
    COB_ConfigStage = 5U;
    return false;
  }

  if (!COB_Config_IsValid(&record.config))
  {
    COB_ConfigStage = 6U;
    return false;
  }

  *config = record.config;
  return true;
}

static void COB_Config_LoadDefaults(COB_Config_t *config)
{
  uint32_t uid_hash = COB_Config_HashUid();

  memset(config, 0, sizeof(*config));
  config->addresses.ip_address[0] = 192U;
  config->addresses.ip_address[1] = 168U;
  config->addresses.ip_address[2] = 1U;
  config->addresses.ip_address[3] = 188U;

  config->addresses.mac_address[0] = 0x02U;
  config->addresses.mac_address[1] = 0xFDU;
  config->addresses.mac_address[2] = (uint8_t)((uid_hash >> 24) & 0xFFU);
  config->addresses.mac_address[3] = (uint8_t)((uid_hash >> 16) & 0xFFU);
  config->addresses.mac_address[4] = (uint8_t)((uid_hash >> 8) & 0xFFU);
  config->addresses.mac_address[5] = (uint8_t)(uid_hash & 0xFFU);
}

static bool COB_Config_GetFlashLayout(uint32_t *address, uint32_t *erase_size)
{
  EXTMEM_NOR_SFDP_FlashInfoTypeDef info = {0};
  EXTMEM_StatusTypeDef status;
  uint32_t flash_size;
  uint32_t selected_erase_size;

  if ((address == NULL) || (erase_size == NULL))
  {
    return false;
  }

  if ((cob_config_flash_address != 0U) && (cob_config_flash_erase_size != 0U))
  {
    *address = cob_config_flash_address;
    *erase_size = cob_config_flash_erase_size;
    return true;
  }

  status = EXTMEM_GetInfo(EXTMEMORY_2, &info);
  COB_ConfigLastStatus = (int32_t)status;
  if ((status != EXTMEM_OK) || (info.FlashSize == 0U) || (info.FlashSize >= 31U))
  {
    COB_ConfigStage = 7U;
    return false;
  }

  flash_size = 1UL << info.FlashSize;
  selected_erase_size = COB_Config_SelectEraseSize(&info);
  if ((selected_erase_size < COB_CONFIG_MIN_ERASE_SIZE) || (selected_erase_size > flash_size))
  {
    COB_ConfigStage = 8U;
    return false;
  }

  cob_config_flash_address = flash_size - selected_erase_size;
  cob_config_flash_erase_size = selected_erase_size;
  COB_ConfigFlashAddress = cob_config_flash_address;
  COB_ConfigFlashEraseSize = cob_config_flash_erase_size;

  *address = cob_config_flash_address;
  *erase_size = cob_config_flash_erase_size;
  return true;
}

static uint32_t COB_Config_SelectEraseSize(const EXTMEM_NOR_SFDP_FlashInfoTypeDef *info)
{
  uint32_t erase_size = info->EraseType1Size;

  if ((erase_size == 0U) || ((info->EraseType2Size != 0U) && (info->EraseType2Size < erase_size)))
  {
    erase_size = info->EraseType2Size;
  }
  if ((erase_size == 0U) || ((info->EraseType3Size != 0U) && (info->EraseType3Size < erase_size)))
  {
    erase_size = info->EraseType3Size;
  }
  if ((erase_size == 0U) || ((info->EraseType4Size != 0U) && (info->EraseType4Size < erase_size)))
  {
    erase_size = info->EraseType4Size;
  }

  return erase_size;
}

static bool COB_Config_IsValid(const COB_Config_t *config)
{
  const uint8_t *ip = config->addresses.ip_address;
  const uint8_t *mac = config->addresses.mac_address;
  bool mac_all_zero = true;
  bool mac_all_ff = true;

  if ((ip[0] == 0U) || (ip[0] >= 224U) || (ip[3] == 0U) || (ip[3] == 255U))
  {
    return false;
  }

  for (uint32_t i = 0U; i < COB_CONFIG_MAC_OCTET_COUNT; i++)
  {
    if (mac[i] != 0U)
    {
      mac_all_zero = false;
    }
    if (mac[i] != 0xFFU)
    {
      mac_all_ff = false;
    }
  }

  if (mac_all_zero || mac_all_ff || ((mac[0] & 0x01U) != 0U))
  {
    return false;
  }

  return true;
}

static uint32_t COB_Config_Crc32(const uint8_t *data, uint32_t length)
{
  uint32_t crc = 0xFFFFFFFFUL;

  for (uint32_t i = 0U; i < length; i++)
  {
    crc ^= data[i];
    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
      if ((crc & 1U) != 0U)
      {
        crc = (crc >> 1) ^ 0xEDB88320UL;
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return ~crc;
}

static uint32_t COB_Config_HashUid(void)
{
  uint32_t hash = 2166136261UL;
  uint32_t uid[3];

  uid[0] = HAL_GetUIDw0();
  uid[1] = HAL_GetUIDw1();
  uid[2] = HAL_GetUIDw2();

  for (uint32_t word = 0U; word < 3U; word++)
  {
    for (uint32_t shift = 0U; shift < 32U; shift += 8U)
    {
      hash ^= (uid[word] >> shift) & 0xFFU;
      hash *= 16777619UL;
    }
  }

  return hash;
}

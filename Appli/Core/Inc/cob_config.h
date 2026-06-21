#ifndef COB_CONFIG_H
#define COB_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define COB_CONFIG_IP_OCTET_COUNT  4U
#define COB_CONFIG_MAC_OCTET_COUNT 6U

typedef struct {
  uint8_t ip_address[COB_CONFIG_IP_OCTET_COUNT];
  uint8_t mac_address[COB_CONFIG_MAC_OCTET_COUNT];
} COB_ConfigAddresses_t;

typedef struct {
  COB_ConfigAddresses_t addresses;
} COB_Config_t;

void COB_Config_Init(void);
bool COB_Config_IsRamValid(void);
const COB_Config_t *COB_Config_Get(void);
uint32_t COB_Config_GetIpAddressU32(void);
void COB_Config_GetMacAddress(uint8_t mac_address[COB_CONFIG_MAC_OCTET_COUNT]);
bool COB_Config_SaveToFlash(void);

#ifdef __cplusplus
}
#endif

#endif /* COB_CONFIG_H */

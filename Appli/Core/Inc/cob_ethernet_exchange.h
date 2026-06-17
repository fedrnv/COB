#ifndef COB_ETHERNET_EXCHANGE_H
#define COB_ETHERNET_EXCHANGE_H

#include <stdbool.h>
#include <stdint.h>
#include "cob_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*COB_EthernetSendFn)(const uint8_t *payload, uint16_t length, void *context);

void COB_EthernetExchange_Init(COB_EthernetSendFn send_fn, void *send_context);
bool COB_EthernetExchange_ProcessPacket(const uint8_t *payload, uint16_t length);
bool COB_EthernetExchange_SendInfo(void);

const COB_CoefficientPacket_t *COB_EthernetExchange_GetCoefficientPacket(void);
const COB_ControlPacket_t *COB_EthernetExchange_GetControlPacket(void);
const COB_AddressPacket_t *COB_EthernetExchange_GetAddressPacket(void);
const COB_InfoPacket_t *COB_EthernetExchange_GetInfoPacket(void);

#ifdef __cplusplus
}
#endif

#endif /* COB_ETHERNET_EXCHANGE_H */

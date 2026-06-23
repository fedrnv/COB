# COB Exchange Protocol

This document describes the COB Ethernet exchange protocol. Packet structures are
defined in `Appli/Core/Inc/cob_protocol_packets.h`; any packet layout change must
be documented here in the same change set.

## Transport

- Physical/link layer: Ethernet.
- IP stack: NetX Duo.
- Application transport: UDP.
- Command receive UDP port: 1556.
- Ping broadcast UDP port: 50101.
- Subscription info destination: IP address and UDP port received in
  `COB_SubscriptionPacket_t`.
- Application packet byte order: native MCU little-endian for integer fields.
- Packet layout: packed C structs, no padding between fields.
- Common header size: 6 bytes.

The device acts as a UDP server for command exchange: it listens on the command
receive port and does not require a persistent connection. Command responses are
sent to the source IPv4 address and UDP port of the received datagram.

The device sends a ping broadcast packet every 3 seconds to
`255.255.255.255:50101` while there are no active subscriptions. After at least
one subscription is active, ping broadcast is stopped and info packets are sent
to active subscription addresses once per second instead.

Current default board IPv4 address is `192.168.1.188/24`.

Current default MAC address is generated as `02:FD:<hash32 bytes>`, where
`hash32` is FNV-1a over the STM32 96-bit UID.

Current firmware RX buffer limit is 512 bytes per UDP datagram. Larger datagrams
are dropped before protocol parsing.

If Ethernet/IP/UDP initialization fails, firmware does not stop. Network
initialization is retried every 0.5 seconds. If link check fails later, NetX
objects are deleted and created again by the same 0.5 second retry loop.

## Runtime Indicators

The current firmware uses four status LED channels:

| LED | Color/role | Behavior |
| --- | --- | --- |
| 3 | Red error | Turns on on Ethernet/UDP/runtime exchange errors. Turns off again after a healthy Ethernet link/status cycle. |
| 5 | Green exchange | Turns on after Ethernet/UDP initialization succeeds. Turns off when initialization/link is lost. |
| 5 | Green activity pulse | Turns off for 0.1 seconds on packet activity, rate-limited to one pulse per 0.3 seconds. |
| 6 | Mode | Reserved for future mode indication. |
| 7 | Mode | Reserved for future mode indication. |

## Exchange Flow

1. Device broadcasts `COB_PingPacket_t` to UDP port 50101 every 3 seconds until
   at least one subscription is active.
2. Host sends one COB command or subscription packet to board UDP port 1556.
3. Firmware extracts the source IPv4 address and source UDP port from the
   datagram metadata.
4. Firmware parses the packet by `Id`.
5. If the command requires an acknowledgment or data response, firmware sends
   response packet(s) back to the datagram source.
6. If a subscription is accepted, firmware sends `COB_InfoPacket_t` to each
   subscribed IP address and port once per second.

## Common Header

All packets start with:

| Field | Type | Size | Description |
| --- | --- | ---: | --- |
| `Id` | `uint16_t` | 2 | Packet type identifier. |
| `Index` | `uint16_t` | 2 | Packet sequence index. |
| `Length` | `uint16_t` | 2 | Total packet length in bytes, including header. |

## Receive Packet IDs

| ID | Name | Packet |
| ---: | --- | --- |
| 0 | `COB_START_MEAS_RCV_ID` | `COB_StartMeasurePacket_t` |
| 1 | `COB_ASCAN_RCV_ID` | Reserved for A-scan receive packet. |
| 2 | `COB_COEFF_RCV_ID` | `COB_CoefficientPacket_t` |
| 3 | `COB_CONTROL_RCV_ID` | `COB_ControlPacket_t` |
| 4 | `COB_ADDRESS_RCV_ID` | `COB_AddressPacket_t` |
| 5 | `COB_OPERATION_RCV_ID` | `COB_OperationPacket_t` |
| 6 | `COB_NOT_RCV_ID` | No-operation packet. |
| 7 | `COB_SUBSCRIPTION_RCV_ID` | `COB_SubscriptionPacket_t` |

## Send Packet IDs

| ID | Name | Packet |
| ---: | --- | --- |
| 0 | `COB_DATA_SEND_ID` | Reserved for measurement data. |
| 1 | `COB_ASCAN_SEND_ID` | Reserved for A-scan send packet. |
| 2 | `COB_COEFF_ECHO_ID` | `COB_CoefficientEchoPacket_t` |
| 3 | `COB_CONTROL_ECHO_ID` | `COB_ControlEchoPacket_t` |
| 4 | `COB_ADDRESS_ECHO_ID` | `COB_AddressEchoPacket_t` |
| 5 | `COB_OPERATION_ECHO_ID` | `COB_OperationEchoPacket_t` |
| 6 | `COB_PING_SEND_ID` | `COB_PingPacket_t`, broadcast ping. |
| 7 | `COB_SUBSCRIPTION_ACK_ID` | `COB_SubscriptionAckPacket_t` |
| 8 | `COB_INFO_SEND_ID` | `COB_InfoPacket_t`, unicast subscription info. |
| 11 | `COB_COEFF_BACK_ID` | `COB_CoefficientPacket_t` |
| 12 | `COB_CONTROL_BACK_ID` | `COB_ControlPacket_t` |
| 13 | `COB_ADDRESS_BACK_ID` | `COB_AddressPacket_t` |

## Packet Structures

### `COB_StartMeasurePacket_t`

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Must be `COB_START_MEAS_RCV_ID`. |
| `Index` | `uint16_t` | Packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_StartMeasurePacket_t)`. |
| `MeasNumber` | `uint16_t` | Measurement number/request counter. |

### `COB_CoefficientPacket_t`

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Receive: `COB_COEFF_RCV_ID`; response backup: `COB_COEFF_BACK_ID`. |
| `Index` | `uint16_t` | Packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_CoefficientPacket_t)`. |
| `Reserve` | `uint16_t` | Reserved. |
| `CoefGSMM` | `float` | GSM multiplier coefficient. |
| `CoefGSMA` | `float` | GSM additive coefficient. |
| `CoefTempM` | `uint32_t` | Temperature multiplier coefficient. |
| `CoefTempA` | `uint32_t` | Temperature additive coefficient. |

### `COB_ControlPacket_t`

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Receive: `COB_CONTROL_RCV_ID`; response backup: `COB_CONTROL_BACK_ID`. |
| `Index` | `uint16_t` | Packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_ControlPacket_t)`. |
| `MeasPeriod` | `uint16_t` | Measurement period. |
| `DeviceMode` | `uint16_t` | Device operating mode. |
| `Options` | `uint16_t` | Device option bit mask. |
| `GeneratorParameters.NumberTP` | `uint8_t` | Number of generator pulses. |
| `GeneratorParameters.GenOptions` | `uint8_t` | Generator option bit mask. |
| `GeneratorParameters.TPPeriod` | `uint16_t` | Generator pulse period. |
| `GeneratorParameters.GuardIntreval` | `uint16_t` | Guard interval. |
| `GeneratorParameters.DelayGen` | `uint16_t` | Generator delay. |
| `AmplifierParameteters.Gain` | `uint16_t` | Amplifier gain. |
| `AmplifierParameteters.AutoGainControl` | `uint16_t` | Automatic gain control flag. |
| `SignalProcessingParameters.StartIndex` | `uint16_t` | Processing start sample index. |
| `SignalProcessingParameters.EndIndex` | `uint16_t` | Processing end sample index. |
| `SignalProcessingParameters.FiltersActivity` | `uint16_t` | Active filters bit mask. |
| `SignalProcessingParameters.LPFWindow` | `uint16_t` | Low-pass filter window. |
| `SignalProcessingParameters.RepDepth` | `uint16_t` | Repetition depth. |
| `SignalProcessingParameters.FIRspPointsN` | `uint16_t` | FIR points count. |
| `SignalProcessingParameters.EmaAlpha` | `uint16_t` | EMA alpha value. |
| `SignalProcessingParameters.ATAvWindow` | `uint16_t` | ATA averaging window. |

### `COB_AddressPacket_t`

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Receive: `COB_ADDRESS_RCV_ID`; response backup: `COB_ADDRESS_BACK_ID`. |
| `Index` | `uint16_t` | Packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_AddressPacket_t)`. |
| `MACAddress` | `uint8_t[6]` | Device MAC address. |
| `IPAddress` | `uint8_t[4]` | Device IPv4 address. |
| `IPAddressDst` | `uint8_t[4]` | Primary destination IPv4 address. |
| `SendPort` | `uint16_t` | Primary destination port. |
| `IPAddressDst2` | `uint8_t[4]` | Secondary destination IPv4 address. |
| `SendPort2` | `uint16_t` | Secondary destination port. |
| `Reserve` | `uint16_t` | Reserved. |
| `MCID` | `uint32_t` | Module/controller identifier. |

### `COB_OperationPacket_t`

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Must be `COB_OPERATION_RCV_ID`. |
| `Index` | `uint16_t` | Packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_OperationPacket_t)`. |
| `Reserve` | `uint8_t` | Reserved. |
| `OperationType` | `uint8_t` | Operation command. |
| `OperationParam` | `uint32_t` | Operation parameter. |

### `COB_PingPacket_t`

This packet is sent every 3 seconds as UDP broadcast to port 50101 while there
are no active subscriptions.

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Must be `COB_PING_SEND_ID`. |
| `Index` | `uint16_t` | Ping packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_PingPacket_t)`. |
| `DeviceId` | `uint32_t[3]` | STM32 96-bit unique device ID: `HAL_GetUIDw0/1/2`. |
| `CurrentIpAddress` | `uint8_t[4]` | Current board IPv4 address in human byte order. |
| `CurrentReceivePort` | `uint16_t` | Current command receive port. Default: 1556. |
| `PingBroadcastPort` | `uint16_t` | UDP port used for this ping broadcast. Default: 50101. |
| `McuTemperatureCentiC` | `int16_t` | MCU temperature in 0.01 deg C units. Currently `0` until ADC temperature measurement is connected. |
| `CurrentStatus` | `uint16_t` | Current status. Currently only `0` is defined. |
| `Version` | `uint32_t` | Firmware version. Currently always `0x00000001`, displayed as `0.0.0.1`. |
| `Config` | `uint32_t` | Configuration format/version. Currently always `0x00000001`, displayed as `0.0.0.1`. |

### `COB_SubscriptionPacket_t`

This packet creates a RAM-only subscription. Subscriptions are not saved to
flash and are cleared by reboot. Up to three unique IP:port pairs can be active.

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Must be `COB_SUBSCRIPTION_RCV_ID`. |
| `Index` | `uint16_t` | Packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_SubscriptionPacket_t)`. |
| `IPAddress` | `uint8_t[4]` | Destination IPv4 address for info packets in human byte order. |
| `Port` | `uint16_t` | Destination UDP port for info packets. |

### `COB_SubscriptionAckPacket_t`

This packet is sent back to the source IPv4 address and source UDP port of the
subscription datagram.

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Must be `COB_SUBSCRIPTION_ACK_ID`. |
| `Index` | `uint16_t` | Ack packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_SubscriptionAckPacket_t)`. |
| `Status` | `uint8_t` | `1` accepted, `0` rejected. |
| `ErrorCode` | `uint8_t` | `0` none, `1` maximum subscription count exceeded. |
| `SubscriptionCount` | `uint8_t` | Current active subscription count. |
| `Reserved` | `uint8_t` | Reserved, always `0`. |

### `COB_InfoPacket_t`

This packet is sent once per second to each active subscription while at least
one subscription is active.

| Field | Type | Description |
| --- | --- | --- |
| `Id` | `uint16_t` | Must be `COB_INFO_SEND_ID`. |
| `Index` | `uint16_t` | Info packet sequence index. |
| `Length` | `uint16_t` | `sizeof(COB_InfoPacket_t)`. |
| `DeviceId` | `uint32_t` | 32-bit FNV-1a hash over the STM32 96-bit UID. |
| `DeviceType` | `uint8_t` | Device type. Currently always `1`. |
| `DeviceStatus` | `uint8_t` | Device status. Currently always `101`. |
| `ErrorCode` | `uint8_t` | Error code. Currently always `0`. |
| `SubscriptionCount` | `uint8_t` | Current active subscription count. |
| `TemperatureCentiC` | `int16_t` | Temperature in 0.01 deg C units. Currently `0`. |
| `Reserved` | `uint16_t` | Reserved, always `0`. |
| `SentInfoCounter` | `uint32_t` | Count of successfully sent info packets including this packet. |

### Echo Packets

Echo packets acknowledge accepted configuration packets:

| Packet | Status field |
| --- | --- |
| `COB_CoefficientEchoPacket_t` | `CoefficientStatus` |
| `COB_ControlEchoPacket_t` | `ErrorParameters` |
| `COB_AddressEchoPacket_t` | `AddressStatus` |
| `COB_OperationEchoPacket_t` | `OperationStatus` |

## Operation Commands

| Value | Name | Description |
| ---: | --- | --- |
| 1 | `COB_OPERATION_READ_RAM` | Read configuration from RAM. |
| 2 | `COB_OPERATION_READ_FLASH` | Read configuration from flash. |
| 3 | `COB_OPERATION_SAVE_FLASH` | Save configuration to flash. |
| 4 | `COB_OPERATION_RESET_FLASH` | Reset flash configuration. |
| 5 | `COB_OPERATION_RESET_DEFAULT` | Reset configuration to defaults. |
| 6 | `COB_OPERATION_RESET_MODULE` | Reset module. |
| 7 | `COB_OPERATION_CALIBRATION` | Start calibration. |
| 8 | `COB_OPERATION_FIRMWARE_UPD` | Firmware update operation. |
| 70 | `COB_OPERATION_AIR_CALIBRATION` | Start air calibration. |

## Operation Parameters

| Value | Name | Description |
| ---: | --- | --- |
| 10 | `COB_OPERATION_PACK_ALL` | Return all stored configuration packets. |
| 11 | `COB_OPERATION_PACK_COEFF` | Return coefficient packet. |
| 12 | `COB_OPERATION_PACK_CONTROL` | Return control packet. |
| 13 | `COB_OPERATION_PACK_ADDR` | Return address packet. |
| 20 | `COB_OPERATION_PACK_RESET` | Reset packet group. |
| 30 | `COB_OPERATION_PACK_FIRMW` | Firmware packet group. |

## Operation Status

| Value | Name | Description |
| ---: | --- | --- |
| 0 | `COB_OPERATION_NOT_OK` | Operation failed or was not accepted. |
| 1 | `COB_OPERATION_DATA_OK` | Operation accepted/data available. |
| 2 | `COB_OPERATION_CAL_OK` | Calibration completed. |
| 3 | `COB_OPERATION_RESET_OK` | Reset completed. |
| 4 | `COB_OPERATION_FIRMWARE_OK` | Firmware operation completed. |

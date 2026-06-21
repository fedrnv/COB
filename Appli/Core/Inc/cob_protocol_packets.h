#ifndef COB_PROTOCOL_PACKETS_H
#define COB_PROTOCOL_PACKETS_H

/*
 * Packet layout contract for COB Ethernet exchange.
 *
 * Any change in this file must be reflected in
 * docs/COB_exchange_protocol.md in the same change set.
 */

#include <stdint.h>

#if defined(__GNUC__)
#define COB_PACKED __attribute__((packed))
#else
#define COB_PACKED
#endif

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
} COB_PACKED COB_Header_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t MeasNumber;
} COB_PACKED COB_StartMeasurePacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t Reserve;
  float CoefGSMM;
  float CoefGSMA;
  uint32_t CoefTempM;
  uint32_t CoefTempA;
} COB_PACKED COB_CoefficientPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t CoefficientStatus;
} COB_PACKED COB_CoefficientEchoPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t MeasPeriod;
  uint16_t DeviceMode;
  uint16_t Options;
  struct {
    uint8_t NumberTP;
    uint8_t GenOptions;
    uint16_t TPPeriod;
    uint16_t GuardIntreval;
    uint16_t DelayGen;
  } COB_PACKED GeneratorParameters;
  struct {
    uint16_t Gain;
    uint16_t AutoGainControl;
  } COB_PACKED AmplifierParameteters;
  struct {
    uint16_t StartIndex;
    uint16_t EndIndex;
    uint16_t FiltersActivity;
    uint16_t LPFWindow;
    uint16_t RepDepth;
    uint16_t FIRspPointsN;
    uint16_t EmaAlpha;
    uint16_t ATAvWindow;
  } COB_PACKED SignalProcessingParameters;
} COB_PACKED COB_ControlPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t ErrorParameters;
} COB_PACKED COB_ControlEchoPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint8_t MACAddress[6];
  uint8_t IPAddress[4];
  uint8_t IPAddressDst[4];
  uint16_t SendPort;
  uint8_t IPAddressDst2[4];
  uint16_t SendPort2;
  uint16_t Reserve;
  uint32_t MCID;
} COB_PACKED COB_AddressPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t AddressStatus;
} COB_PACKED COB_AddressEchoPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint8_t Reserve;
  uint8_t OperationType;
  uint32_t OperationParam;
} COB_PACKED COB_OperationPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint16_t OperationStatus;
  uint32_t Reserve;
} COB_PACKED COB_OperationEchoPacket_t;

typedef struct {
  uint16_t Id;
  uint16_t Index;
  uint16_t Length;
  uint32_t DeviceId[3];
  uint8_t CurrentIpAddress[4];
  uint16_t AdditionalSendPort;
  uint16_t CurrentReceivePort;
  int16_t McuTemperatureCentiC;
  uint16_t CurrentStatus;
  uint32_t Version;
  uint32_t Config;
} COB_PACKED COB_InfoPacket_t;

#endif /* COB_PROTOCOL_PACKETS_H */

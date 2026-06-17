#ifndef COB_PROTOCOL_H
#define COB_PROTOCOL_H

#include <stdint.h>

#if defined(__GNUC__)
#define COB_PACKED __attribute__((packed))
#else
#define COB_PACKED
#endif

#define COB_START_MEAS_RCV_ID        0U
#define COB_ASCAN_RCV_ID             1U
#define COB_COEFF_RCV_ID             2U
#define COB_CONTROL_RCV_ID           3U
#define COB_ADDRESS_RCV_ID           4U
#define COB_OPERATION_RCV_ID         5U
#define COB_NOT_RCV_ID               6U

#define COB_DATA_SEND_ID             0U
#define COB_ASCAN_SEND_ID            1U
#define COB_COEFF_ECHO_ID            2U
#define COB_CONTROL_ECHO_ID          3U
#define COB_ADDRESS_ECHO_ID          4U
#define COB_OPERATION_ECHO_ID        5U
#define COB_INFO_SEND_ID             6U
#define COB_COEFF_BACK_ID            11U
#define COB_CONTROL_BACK_ID          12U
#define COB_ADDRESS_BACK_ID          13U

#define COB_OPERATION_READ_RAM       1U
#define COB_OPERATION_READ_FLASH     2U
#define COB_OPERATION_SAVE_FLASH     3U
#define COB_OPERATION_RESET_FLASH    4U
#define COB_OPERATION_RESET_DEFAULT  5U
#define COB_OPERATION_RESET_MODULE   6U
#define COB_OPERATION_CALIBRATION    7U
#define COB_OPERATION_FIRMWARE_UPD   8U
#define COB_OPERATION_AIR_CALIBRATION 70U

#define COB_OPERATION_PACK_ALL       10U
#define COB_OPERATION_PACK_COEFF     11U
#define COB_OPERATION_PACK_CONTROL   12U
#define COB_OPERATION_PACK_ADDR      13U
#define COB_OPERATION_PACK_RESET     20U
#define COB_OPERATION_PACK_FIRMW     30U

#define COB_OPERATION_NOT_OK         0U
#define COB_OPERATION_DATA_OK        1U
#define COB_OPERATION_CAL_OK         2U
#define COB_OPERATION_RESET_OK       3U
#define COB_OPERATION_FIRMWARE_OK    4U

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
  uint16_t FirmwareVers;
  uint32_t BAW_STM32_ID;
  uint32_t EEPROMID;
  uint32_t MCID;
  uint8_t DeviceStatus;
  uint8_t FirmwareUpdateCnt;
  int16_t TempuC;
  int16_t TempExtSensor;
  uint16_t Vin1;
  uint8_t cpu_load;
  uint8_t Mode;
  uint16_t Vin2;
  uint16_t TempIRSensor;
  uint16_t Gain;
  uint8_t BoardConnStatus;
} COB_PACKED COB_InfoPacket_t;

#endif /* COB_PROTOCOL_H */

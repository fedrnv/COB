#include "cob_ethernet_exchange.h"

#include "cob_config.h"

#include <string.h>

typedef struct {
  COB_StartMeasurePacket_t start_measure;
  COB_CoefficientPacket_t coefficient;
  COB_ControlPacket_t control;
  COB_AddressPacket_t address;
  COB_OperationPacket_t operation;
  COB_PingPacket_t ping;
  uint16_t tx_index;
  COB_EthernetSendFn send_fn;
  void *send_context;
  bool start_measure_valid;
  bool coefficient_valid;
  bool control_valid;
  bool address_valid;
  bool operation_valid;
} COB_EthernetExchangeState_t;

static COB_EthernetExchangeState_t exchange;

static uint16_t clamp_copy_len(uint16_t packet_len, uint16_t storage_len);
static bool send_packet(void *packet, uint16_t length, uint16_t packet_id);
static bool send_coefficient_echo(uint16_t status);
static bool send_control_echo(uint16_t error_parameters);
static bool send_address_echo(uint16_t status);
static bool send_operation_echo(uint16_t status);
static bool send_ram_packet(uint32_t operation_param);
static void load_defaults(void);

void COB_EthernetExchange_Init(COB_EthernetSendFn send_fn, void *send_context)
{
  memset(&exchange, 0, sizeof(exchange));
  exchange.send_fn = send_fn;
  exchange.send_context = send_context;
  load_defaults();
}

bool COB_EthernetExchange_ProcessPacket(const uint8_t *payload, uint16_t length)
{
  COB_Header_t header;

  if ((payload == NULL) || (length < sizeof(header))) {
    return false;
  }

  memcpy(&header, payload, sizeof(header));

  switch (header.Id) {
  case COB_START_MEAS_RCV_ID:
    memcpy(&exchange.start_measure, payload, clamp_copy_len(length, sizeof(exchange.start_measure)));
    exchange.start_measure.Id = COB_START_MEAS_RCV_ID;
    exchange.start_measure.Length = sizeof(exchange.start_measure);
    exchange.start_measure_valid = true;
    return true;

  case COB_COEFF_RCV_ID:
    memcpy(&exchange.coefficient, payload, clamp_copy_len(length, sizeof(exchange.coefficient)));
    exchange.coefficient.Id = COB_COEFF_RCV_ID;
    exchange.coefficient.Length = sizeof(exchange.coefficient);
    exchange.coefficient_valid = true;
    return send_coefficient_echo(1U);

  case COB_CONTROL_RCV_ID:
    memcpy(&exchange.control, payload, clamp_copy_len(length, sizeof(exchange.control)));
    exchange.control.Id = COB_CONTROL_RCV_ID;
    exchange.control.Length = sizeof(exchange.control);
    exchange.control_valid = true;
    return send_control_echo(0U);

  case COB_ADDRESS_RCV_ID:
    memcpy(&exchange.address, payload, clamp_copy_len(length, sizeof(exchange.address)));
    exchange.address.Id = COB_ADDRESS_RCV_ID;
    exchange.address.Length = sizeof(exchange.address);
    exchange.address_valid = true;
    return send_address_echo(1U);

  case COB_OPERATION_RCV_ID:
    memcpy(&exchange.operation, payload, clamp_copy_len(length, sizeof(exchange.operation)));
    exchange.operation.Id = COB_OPERATION_RCV_ID;
    exchange.operation.Length = sizeof(exchange.operation);
    exchange.operation_valid = true;

    if ((exchange.operation.OperationType == COB_OPERATION_READ_RAM) ||
        (exchange.operation.OperationType == COB_OPERATION_READ_FLASH)) {
      bool sent_data = send_ram_packet(exchange.operation.OperationParam);
      (void)send_operation_echo(sent_data ? COB_OPERATION_DATA_OK : COB_OPERATION_NOT_OK);
      return sent_data;
    }

    return send_operation_echo(COB_OPERATION_DATA_OK);

  case COB_NOT_RCV_ID:
    return true;

  default:
    return false;
  }
}

bool COB_EthernetExchange_SendPing(void)
{
  exchange.ping.Id = COB_PING_SEND_ID;
  exchange.ping.Length = sizeof(exchange.ping);
  return send_packet(&exchange.ping, sizeof(exchange.ping), COB_PING_SEND_ID);
}

const COB_CoefficientPacket_t *COB_EthernetExchange_GetCoefficientPacket(void)
{
  return &exchange.coefficient;
}

const COB_ControlPacket_t *COB_EthernetExchange_GetControlPacket(void)
{
  return &exchange.control;
}

const COB_AddressPacket_t *COB_EthernetExchange_GetAddressPacket(void)
{
  return &exchange.address;
}

const COB_PingPacket_t *COB_EthernetExchange_GetPingPacket(void)
{
  return &exchange.ping;
}

static uint16_t clamp_copy_len(uint16_t packet_len, uint16_t storage_len)
{
  return (packet_len < storage_len) ? packet_len : storage_len;
}

static bool send_packet(void *packet, uint16_t length, uint16_t packet_id)
{
  COB_Header_t *header = (COB_Header_t *)packet;

  header->Id = packet_id;
  header->Index = exchange.tx_index;
  header->Length = length;

  if (exchange.send_fn == NULL) {
    exchange.tx_index++;
    return false;
  }

  bool sent = exchange.send_fn((const uint8_t *)packet, length, exchange.send_context);
  if (sent) {
    exchange.tx_index++;
  }

  return sent;
}

static bool send_coefficient_echo(uint16_t status)
{
  COB_CoefficientEchoPacket_t echo = {
    .Id = COB_COEFF_ECHO_ID,
    .Length = sizeof(echo),
    .CoefficientStatus = status,
  };

  return send_packet(&echo, sizeof(echo), COB_COEFF_ECHO_ID);
}

static bool send_control_echo(uint16_t error_parameters)
{
  COB_ControlEchoPacket_t echo = {
    .Id = COB_CONTROL_ECHO_ID,
    .Length = sizeof(echo),
    .ErrorParameters = error_parameters,
  };

  return send_packet(&echo, sizeof(echo), COB_CONTROL_ECHO_ID);
}

static bool send_address_echo(uint16_t status)
{
  COB_AddressEchoPacket_t echo = {
    .Id = COB_ADDRESS_ECHO_ID,
    .Length = sizeof(echo),
    .AddressStatus = status,
  };

  return send_packet(&echo, sizeof(echo), COB_ADDRESS_ECHO_ID);
}

static bool send_operation_echo(uint16_t status)
{
  COB_OperationEchoPacket_t echo = {
    .Id = COB_OPERATION_ECHO_ID,
    .Length = sizeof(echo),
    .OperationStatus = status,
    .Reserve = 0x4F4B0000UL | status,
  };

  return send_packet(&echo, sizeof(echo), COB_OPERATION_ECHO_ID);
}

static bool send_ram_packet(uint32_t operation_param)
{
  bool sent = false;

  if ((operation_param == COB_OPERATION_PACK_ALL) || (operation_param == COB_OPERATION_PACK_COEFF)) {
    if (exchange.coefficient_valid) {
      COB_CoefficientPacket_t packet = exchange.coefficient;
      sent |= send_packet(&packet, sizeof(packet), COB_COEFF_BACK_ID);
    }
  }

  if ((operation_param == COB_OPERATION_PACK_ALL) || (operation_param == COB_OPERATION_PACK_CONTROL)) {
    if (exchange.control_valid) {
      COB_ControlPacket_t packet = exchange.control;
      sent |= send_packet(&packet, sizeof(packet), COB_CONTROL_BACK_ID);
    }
  }

  if ((operation_param == COB_OPERATION_PACK_ALL) || (operation_param == COB_OPERATION_PACK_ADDR)) {
    if (exchange.address_valid) {
      COB_AddressPacket_t packet = exchange.address;
      sent |= send_packet(&packet, sizeof(packet), COB_ADDRESS_BACK_ID);
    }
  }

  return sent;
}

static void load_defaults(void)
{
  const COB_Config_t *config = COB_Config_Get();

  exchange.coefficient = (COB_CoefficientPacket_t) {
    .Id = COB_COEFF_RCV_ID,
    .Length = sizeof(exchange.coefficient),
    .Reserve = 0xA55AU,
    .CoefGSMM = 1.0f,
    .CoefGSMA = 2.0f,
    .CoefTempM = 100U,
    .CoefTempA = 25U,
  };
  exchange.coefficient_valid = true;

  exchange.control = (COB_ControlPacket_t) {
    .Id = COB_CONTROL_RCV_ID,
    .Length = sizeof(exchange.control),
    .MeasPeriod = 100U,
    .DeviceMode = 1U,
    .Options = 0x0002U,
    .GeneratorParameters = {
      .NumberTP = 4U,
      .GenOptions = 0x02U,
      .TPPeriod = 1000U,
      .GuardIntreval = 250U,
      .DelayGen = 10U,
    },
    .AmplifierParameteters = {
      .Gain = 32U,
      .AutoGainControl = 1U,
    },
    .SignalProcessingParameters = {
      .StartIndex = 10U,
      .EndIndex = 400U,
      .FiltersActivity = 0x001FU,
      .LPFWindow = 8U,
      .RepDepth = 4U,
      .FIRspPointsN = 16U,
      .EmaAlpha = 64U,
      .ATAvWindow = 10U,
    },
  };
  exchange.control_valid = true;

  exchange.address = (COB_AddressPacket_t) {
    .Id = COB_ADDRESS_RCV_ID,
    .Length = sizeof(exchange.address),
    .MACAddress = {
      config->addresses.mac_address[0],
      config->addresses.mac_address[1],
      config->addresses.mac_address[2],
      config->addresses.mac_address[3],
      config->addresses.mac_address[4],
      config->addresses.mac_address[5],
    },
    .IPAddress = {
      config->addresses.ip_address[0],
      config->addresses.ip_address[1],
      config->addresses.ip_address[2],
      config->addresses.ip_address[3],
    },
    .IPAddressDst = {192U, 168U, 1U, 10U},
    .SendPort = 1556U,
    .IPAddressDst2 = {192U, 168U, 1U, 11U},
    .SendPort2 = 1556U,
    .Reserve = 0x1234U,
    .MCID = 0x4E365F45UL,
  };
  exchange.address_valid = true;

  exchange.ping = (COB_PingPacket_t) {
    .Id = COB_PING_SEND_ID,
    .Length = (uint16_t)sizeof(exchange.ping),
    .DeviceId = {0U, 0U, 0U},
    .CurrentIpAddress = {0U, 0U, 0U, 0U},
    .CurrentReceivePort = 1556U,
    .PingBroadcastPort = 50101U,
    .McuTemperatureCentiC = 0,
    .CurrentStatus = 0U,
  };
}

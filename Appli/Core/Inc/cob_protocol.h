#ifndef COB_PROTOCOL_H
#define COB_PROTOCOL_H

#include <stdint.h>

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

#include "cob_protocol_packets.h"

#endif /* COB_PROTOCOL_H */

#pragma once
#include <uavcan.protocol.file.Read_req.h>
#include <uavcan.protocol.file.Read_res.h>

#define UAVCAN_PROTOCOL_FILE_READ_ID 48
#define UAVCAN_PROTOCOL_FILE_READ_SIGNATURE (0x8DCDCA939F33F678ULL)

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
#include <canard/cxx_wrappers.h>
SERVICE_MESSAGE_CXX_IFACE(uavcan_protocol_file_Read, UAVCAN_PROTOCOL_FILE_READ_ID, UAVCAN_PROTOCOL_FILE_READ_SIGNATURE, UAVCAN_PROTOCOL_FILE_READ_REQUEST_MAX_SIZE, UAVCAN_PROTOCOL_FILE_READ_RESPONSE_MAX_SIZE);
#endif

#pragma once
#include <uavcan.protocol.file.GetInfo_req.h>
#include <uavcan.protocol.file.GetInfo_res.h>

#define UAVCAN_PROTOCOL_FILE_GETINFO_ID 45
#define UAVCAN_PROTOCOL_FILE_GETINFO_SIGNATURE (0x5004891EE8A27531ULL)

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
#include <canard/cxx_wrappers.h>
SERVICE_MESSAGE_CXX_IFACE(uavcan_protocol_file_GetInfo, UAVCAN_PROTOCOL_FILE_GETINFO_ID, UAVCAN_PROTOCOL_FILE_GETINFO_SIGNATURE, UAVCAN_PROTOCOL_FILE_GETINFO_REQUEST_MAX_SIZE, UAVCAN_PROTOCOL_FILE_GETINFO_RESPONSE_MAX_SIZE);
#endif

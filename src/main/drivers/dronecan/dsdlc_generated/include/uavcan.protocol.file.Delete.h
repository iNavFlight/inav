#pragma once
#include <uavcan.protocol.file.Delete_req.h>
#include <uavcan.protocol.file.Delete_res.h>

#define UAVCAN_PROTOCOL_FILE_DELETE_ID 47
#define UAVCAN_PROTOCOL_FILE_DELETE_SIGNATURE (0x78648C99170B47AAULL)

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
#include <canard/cxx_wrappers.h>
SERVICE_MESSAGE_CXX_IFACE(uavcan_protocol_file_Delete, UAVCAN_PROTOCOL_FILE_DELETE_ID, UAVCAN_PROTOCOL_FILE_DELETE_SIGNATURE, UAVCAN_PROTOCOL_FILE_DELETE_REQUEST_MAX_SIZE, UAVCAN_PROTOCOL_FILE_DELETE_RESPONSE_MAX_SIZE);
#endif

#pragma once
#include <uavcan.protocol.file.Write_req.h>
#include <uavcan.protocol.file.Write_res.h>

#define UAVCAN_PROTOCOL_FILE_WRITE_ID 49
#define UAVCAN_PROTOCOL_FILE_WRITE_SIGNATURE (0x515AA1DC77E58429ULL)

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
#include <canard/cxx_wrappers.h>
SERVICE_MESSAGE_CXX_IFACE(uavcan_protocol_file_Write, UAVCAN_PROTOCOL_FILE_WRITE_ID, UAVCAN_PROTOCOL_FILE_WRITE_SIGNATURE, UAVCAN_PROTOCOL_FILE_WRITE_REQUEST_MAX_SIZE, UAVCAN_PROTOCOL_FILE_WRITE_RESPONSE_MAX_SIZE);
#endif

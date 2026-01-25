#pragma once
#include <uavcan.protocol.param.GetSet_req.h>
#include <uavcan.protocol.param.GetSet_res.h>

#define UAVCAN_PROTOCOL_PARAM_GETSET_ID 11
#define UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE (0xA7B622F939D1A4D5ULL)

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
#include <canard/cxx_wrappers.h>
SERVICE_MESSAGE_CXX_IFACE(uavcan_protocol_param_GetSet, UAVCAN_PROTOCOL_PARAM_GETSET_ID, UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE, UAVCAN_PROTOCOL_PARAM_GETSET_REQUEST_MAX_SIZE, UAVCAN_PROTOCOL_PARAM_GETSET_RESPONSE_MAX_SIZE);
#endif

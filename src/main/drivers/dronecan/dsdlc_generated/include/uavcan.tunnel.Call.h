#pragma once
#include <uavcan.tunnel.Call_req.h>
#include <uavcan.tunnel.Call_res.h>

#define UAVCAN_TUNNEL_CALL_ID 63
#define UAVCAN_TUNNEL_CALL_SIGNATURE (0xDB11EDC510502658ULL)

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
#include <canard/cxx_wrappers.h>
SERVICE_MESSAGE_CXX_IFACE(uavcan_tunnel_Call, UAVCAN_TUNNEL_CALL_ID, UAVCAN_TUNNEL_CALL_SIGNATURE, UAVCAN_TUNNEL_CALL_REQUEST_MAX_SIZE, UAVCAN_TUNNEL_CALL_RESPONSE_MAX_SIZE);
#endif

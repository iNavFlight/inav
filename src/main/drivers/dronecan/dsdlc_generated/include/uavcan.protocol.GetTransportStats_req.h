#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>


#define UAVCAN_PROTOCOL_GETTRANSPORTSTATS_REQUEST_MAX_SIZE 0
#define UAVCAN_PROTOCOL_GETTRANSPORTSTATS_REQUEST_SIGNATURE (0xBE6F76A7EC312B04ULL)
#define UAVCAN_PROTOCOL_GETTRANSPORTSTATS_REQUEST_ID 4

#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
class uavcan_protocol_GetTransportStats_cxx_iface;
#endif

struct uavcan_protocol_GetTransportStatsRequest {
#if defined(__cplusplus) && defined(DRONECAN_CXX_WRAPPERS)
    using cxx_iface = uavcan_protocol_GetTransportStats_cxx_iface;
#endif
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_GetTransportStatsRequest_encode(struct uavcan_protocol_GetTransportStatsRequest* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_GetTransportStatsRequest_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_GetTransportStatsRequest* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_GetTransportStatsRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_GetTransportStatsRequest* msg, bool tao);
static inline bool _uavcan_protocol_GetTransportStatsRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_GetTransportStatsRequest* msg, bool tao);
void _uavcan_protocol_GetTransportStatsRequest_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_GetTransportStatsRequest* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

}

/*
 decode uavcan_protocol_GetTransportStatsRequest, return true on failure, false on success
*/
bool _uavcan_protocol_GetTransportStatsRequest_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_GetTransportStatsRequest* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_GetTransportStatsRequest sample_uavcan_protocol_GetTransportStatsRequest_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif

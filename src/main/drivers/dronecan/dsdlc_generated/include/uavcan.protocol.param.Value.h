#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <canard.h>
#include <uavcan.protocol.param.Empty.h>


#define UAVCAN_PROTOCOL_PARAM_VALUE_MAX_SIZE 130
#define UAVCAN_PROTOCOL_PARAM_VALUE_SIGNATURE (0x29F14BF484727267ULL)

enum uavcan_protocol_param_Value_type_t {
    UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY,
    UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE,
    UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE,
    UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE,
    UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE,
};


struct uavcan_protocol_param_Value {
    enum uavcan_protocol_param_Value_type_t union_tag;
    union {
        struct uavcan_protocol_param_Empty empty;
        int64_t integer_value;
        float real_value;
        uint8_t boolean_value;
        struct { uint8_t len; uint8_t data[128]; }string_value;
    };
};

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t uavcan_protocol_param_Value_encode(struct uavcan_protocol_param_Value* msg, uint8_t* buffer
#if CANARD_ENABLE_TAO_OPTION
    , bool tao
#endif
);
bool uavcan_protocol_param_Value_decode(const CanardRxTransfer* transfer, struct uavcan_protocol_param_Value* msg);

#if defined(CANARD_DSDLC_INTERNAL)
static inline void _uavcan_protocol_param_Value_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_param_Value* msg, bool tao);
static inline bool _uavcan_protocol_param_Value_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_param_Value* msg, bool tao);
void _uavcan_protocol_param_Value_encode(uint8_t* buffer, uint32_t* bit_ofs, struct uavcan_protocol_param_Value* msg, bool tao) {
    (void)buffer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;

    uint8_t union_tag = msg->union_tag;
    canardEncodeScalar(buffer, *bit_ofs, 3, &union_tag);
    *bit_ofs += 3;

    switch(msg->union_tag) {
        case UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY: {
            _uavcan_protocol_param_Empty_encode(buffer, bit_ofs, &msg->empty, tao);
            break;
        }
        case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE: {
            canardEncodeScalar(buffer, *bit_ofs, 64, &msg->integer_value);
            *bit_ofs += 64;
            break;
        }
        case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE: {
            canardEncodeScalar(buffer, *bit_ofs, 32, &msg->real_value);
            *bit_ofs += 32;
            break;
        }
        case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE: {
            canardEncodeScalar(buffer, *bit_ofs, 8, &msg->boolean_value);
            *bit_ofs += 8;
            break;
        }
        case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE: {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
            const uint8_t string_value_len = msg->string_value.len > 128 ? 128 : msg->string_value.len;
#pragma GCC diagnostic pop
            if (!tao) {
                canardEncodeScalar(buffer, *bit_ofs, 8, &string_value_len);
                *bit_ofs += 8;
            }
            for (size_t i=0; i < string_value_len; i++) {
                canardEncodeScalar(buffer, *bit_ofs, 8, &msg->string_value.data[i]);
                *bit_ofs += 8;
            }
            break;
        }
    }
}

/*
 decode uavcan_protocol_param_Value, return true on failure, false on success
*/
bool _uavcan_protocol_param_Value_decode(const CanardRxTransfer* transfer, uint32_t* bit_ofs, struct uavcan_protocol_param_Value* msg, bool tao) {
    (void)transfer;
    (void)bit_ofs;
    (void)msg;
    (void)tao;
    uint8_t union_tag;
    canardDecodeScalar(transfer, *bit_ofs, 3, false, &union_tag);
    *bit_ofs += 3;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    if (union_tag >= 5) {
        return true; /* invalid value */
    }
#pragma GCC diagnostic pop
    msg->union_tag = (enum uavcan_protocol_param_Value_type_t)union_tag;

    switch(msg->union_tag) {
        case UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY: {
            if (_uavcan_protocol_param_Empty_decode(transfer, bit_ofs, &msg->empty, tao)) {return true;}
            break;
        }

        case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE: {
            canardDecodeScalar(transfer, *bit_ofs, 64, true, &msg->integer_value);
            *bit_ofs += 64;
            break;
        }

        case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE: {
            canardDecodeScalar(transfer, *bit_ofs, 32, true, &msg->real_value);
            *bit_ofs += 32;
            break;
        }

        case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE: {
            canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->boolean_value);
            *bit_ofs += 8;
            break;
        }

        case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE: {
            if (!tao) {
                canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->string_value.len);
                *bit_ofs += 8;
            } else {
                msg->string_value.len = ((transfer->payload_len*8)-*bit_ofs)/8;
            }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
            if (msg->string_value.len > 128) {
                return true; /* invalid value */
            }
#pragma GCC diagnostic pop
            for (size_t i=0; i < msg->string_value.len; i++) {
                canardDecodeScalar(transfer, *bit_ofs, 8, false, &msg->string_value.data[i]);
                *bit_ofs += 8;
            }
            break;
        }

    }
    return false; /* success */
}
#endif
#ifdef CANARD_DSDLC_TEST_BUILD
struct uavcan_protocol_param_Value sample_uavcan_protocol_param_Value_msg(void);
#endif
#ifdef __cplusplus
} // extern "C"

#ifdef DRONECAN_CXX_WRAPPERS
#include <canard/cxx_wrappers.h>
#endif
#endif

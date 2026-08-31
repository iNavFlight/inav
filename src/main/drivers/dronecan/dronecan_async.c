#include "platform.h"
#if defined(USE_DRONECAN)

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "build/atomic.h"

#include "common/log.h"
#include "common/time.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/nvic.h"

#include "libcanard/canard.h"

#include <dronecan_msgs.h>

#include "dronecan.h"
#include "dronecan_async.h"

extern CanardInstance canard; /* the FC's own canard instance, owned by dronecan.c */

dronecanAsyncSlot_t dronecanAsyncSlot = { .state = DRONECAN_ASYNC_IDLE };

/*
    Send an asynchronous request for data from a dronecan node such as
    a configuration parameter or the node info
*/
bool dronecanAsyncRequest(uint8_t service_id, uint8_t node_id, const void *payload)
{
    if (dronecanAsyncSlot.state == DRONECAN_ASYNC_PENDING &&
        millis() - dronecanAsyncSlot.requested_at_ms < DRONECAN_ASYNC_TIMEOUT_MS) {
        return false;
    }

    // PARAM_GETSET_REQUEST is the largest payload; zero-init prevents garbage in UAVCAN reserved bits
    uint8_t buffer[UAVCAN_PROTOCOL_PARAM_GETSET_REQUEST_MAX_SIZE];
    memset(buffer, 0, sizeof(buffer));
    uint16_t len = 0;
    uint64_t signature = 0;
    const uint8_t *buf_ptr = NULL;

    switch (service_id) {
        case DRONECAN_SERVICE_GETNODEINFO:
            signature = UAVCAN_PROTOCOL_GETNODEINFO_SIGNATURE;
            len = 0;
            break;

        case DRONECAN_SERVICE_PARAM_GETSET: {
            if (!payload) return false;
            const dronecanParamRequest_t *req = (const dronecanParamRequest_t *)payload;
            struct uavcan_protocol_param_GetSetRequest getset;
            memset(&getset, 0, sizeof(getset));
            getset.index = req->index;
            if (req->is_write) {
                getset.value.union_tag = (enum uavcan_protocol_param_Value_type_t)req->value_type;
                switch (req->value_type) {
                    case DRONECAN_PARAM_TYPE_INT:
                        getset.value.integer_value = req->value_int;
                        break;
                    case DRONECAN_PARAM_TYPE_FLOAT:
                        getset.value.real_value = req->value_float;
                        break;
                    case DRONECAN_PARAM_TYPE_BOOL:
                        getset.value.boolean_value = req->value_bool;
                        break;
                    case DRONECAN_PARAM_TYPE_STRING: {
                        uint8_t slen = req->value_str_len < sizeof(getset.value.string_value.data)
                                       ? req->value_str_len : sizeof(getset.value.string_value.data);
                        getset.value.string_value.len = slen;
                        memcpy(getset.value.string_value.data, req->value_str, slen);
                        break;
                    }
                    default:
                        getset.value.union_tag = UAVCAN_PROTOCOL_PARAM_VALUE_EMPTY;
                        break;
                }
            }
            uint8_t nlen = req->req_name_len < sizeof(getset.name.data)
                           ? req->req_name_len : sizeof(getset.name.data);
            getset.name.len = nlen;
            memcpy(getset.name.data, req->req_name, nlen);
            len = uavcan_protocol_param_GetSetRequest_encode(&getset, buffer);
            buf_ptr = buffer;
            signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
            break;
        }

        case DRONECAN_SERVICE_EXECUTE_OPCODE: {
            if (!payload) return false;
            const uint8_t *opcode = (const uint8_t *)payload;
            struct uavcan_protocol_param_ExecuteOpcodeRequest req;
            memset(&req, 0, sizeof(req));
            req.opcode = *opcode;
            req.argument = 0;
            len = uavcan_protocol_param_ExecuteOpcodeRequest_encode(&req, buffer);
            buf_ptr = buffer;
            signature = UAVCAN_PROTOCOL_PARAM_EXECUTEOPCODE_SIGNATURE;
            break;
        }

        case DRONECAN_SERVICE_RESTART_NODE: {
            struct uavcan_protocol_RestartNodeRequest req;
            memset(&req, 0, sizeof(req));
            req.magic_number = UAVCAN_PROTOCOL_RESTARTNODE_REQUEST_MAGIC_NUMBER;
            len = uavcan_protocol_RestartNodeRequest_encode(&req, buffer);
            buf_ptr = buffer;
            signature = UAVCAN_PROTOCOL_RESTARTNODE_SIGNATURE;
            break;
        }

        default:
            return false;
    }

    // buf_ptr remains NULL only for GETNODEINFO (zero-length request); libcanard accepts NULL with len=0
    int16_t res;
    ATOMIC_BLOCK(NVIC_PRIO_CAN) {
        res = canardRequestOrRespond(&canard, node_id, signature, service_id,
            &dronecanAsyncSlot.transfer_id, CANARD_TRANSFER_PRIORITY_MEDIUM, CanardRequest,
            buf_ptr, len);
    }

    if (res < 0) {
        LOG_WARNING(CAN, "dronecanAsyncRequest: service %u node %u failed: %d", service_id, node_id, res);
        return false;
    }

    dronecanAsyncSlot.state = DRONECAN_ASYNC_PENDING;
    dronecanAsyncSlot.seq++;
    dronecanAsyncSlot.service_id = service_id;
    dronecanAsyncSlot.node_id = node_id;
    dronecanAsyncSlot.requested_at_ms = millis();
    return true;
}

void dronecanAsyncCheckTimeout(void)
{
    // Check for and expire any pending async requests that have timed out.
    if (dronecanAsyncSlot.state == DRONECAN_ASYNC_PENDING &&
        millis() - dronecanAsyncSlot.requested_at_ms >= DRONECAN_ASYNC_TIMEOUT_MS) {
        dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
    }
}

/*
    Handle responses for any pending async service request
    (GETNODEINFO, PARAM_GETSET, EXECUTE_OPCODE, RESTART_NODE).
    A single handler serialises all on-demand service requests through
    one shared slot, avoiding the need for per-service response queues.
*/
void dronecanAsyncHandleServiceResponse(CanardInstance *ins, CanardRxTransfer *transfer)
{
    UNUSED(ins);

    if (dronecanAsyncSlot.state != DRONECAN_ASYNC_PENDING) // timed out or already received
        return;
    if (transfer->data_type_id != dronecanAsyncSlot.service_id) // response service_id does not match the pending request
        return;
    if (transfer->source_node_id != dronecanAsyncSlot.node_id) // response received for different node_id
        return;
    // UAVCAN requires matching transfer_id to guard against stale frames (e.g. after bus-off recovery).
    // canardRequestOrRespond increments the slot's transfer_id after sending, so the in-flight id is (transfer_id-1) mod 32.
    if (transfer->transfer_id != ((dronecanAsyncSlot.transfer_id - 1) & 0x1F))
        return;

    switch (dronecanAsyncSlot.service_id) {
        case DRONECAN_SERVICE_GETNODEINFO: {
            struct uavcan_protocol_GetNodeInfoResponse resp;
            if (uavcan_protocol_GetNodeInfoResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "GetNodeInfoResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanGetNodeInfoResult_t *r = &dronecanAsyncSlot.result.node_info;
            uint8_t len = resp.name.len < (sizeof(r->name) - 1) ? resp.name.len : (sizeof(r->name) - 1);
            r->name_len = len;
            memcpy(r->name, resp.name.data, len);
            r->name[len] = '\0';
            r->sw_major = resp.software_version.major;
            r->sw_minor = resp.software_version.minor;
            r->sw_optional_field_flags = resp.software_version.optional_field_flags;
            r->sw_vcs_commit = (resp.software_version.optional_field_flags &
                                UAVCAN_PROTOCOL_SOFTWAREVERSION_OPTIONAL_FIELD_FLAG_VCS_COMMIT)
                               ? resp.software_version.vcs_commit : 0;
            r->hw_major = resp.hardware_version.major;
            r->hw_minor = resp.hardware_version.minor;
            memcpy(r->hw_unique_id, resp.hardware_version.unique_id, 16);
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        case DRONECAN_SERVICE_PARAM_GETSET: {
            struct uavcan_protocol_param_GetSetResponse resp;
            if (uavcan_protocol_param_GetSetResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "ParamGetSetResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
            uint8_t name_len = resp.name.len < (sizeof(r->name) - 1) ? resp.name.len : (sizeof(r->name) - 1);
            r->name_len = name_len;
            memcpy(r->name, resp.name.data, name_len);
            r->name[name_len] = '\0';
            r->type = (uint8_t)resp.value.union_tag;
            switch (resp.value.union_tag) {
                case UAVCAN_PROTOCOL_PARAM_VALUE_INTEGER_VALUE:
                    r->value_int = resp.value.integer_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_VALUE_REAL_VALUE:
                    r->value_float = resp.value.real_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_VALUE_BOOLEAN_VALUE:
                    r->value_bool = resp.value.boolean_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_VALUE_STRING_VALUE: {
                    uint8_t slen = resp.value.string_value.len < (sizeof(r->value_str) - 1)
                                   ? resp.value.string_value.len : (sizeof(r->value_str) - 1);
                    r->value_str_len = slen;
                    memcpy(r->value_str, resp.value.string_value.data, slen);
                    r->value_str[slen] = '\0';
                    break;
                }
                default:
                    r->type = DRONECAN_PARAM_TYPE_EMPTY;
                    break;
            }
            r->min_type = DRONECAN_PARAM_TYPE_EMPTY;
            r->min_int  = 0;
            r->min_float = 0.0f;
            switch (resp.min_value.union_tag) {
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE:
                    r->min_type = DRONECAN_PARAM_TYPE_INT;
                    r->min_int  = resp.min_value.integer_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE:
                    r->min_type  = DRONECAN_PARAM_TYPE_FLOAT;
                    r->min_float = resp.min_value.real_value;
                    break;
                default:
                    break;
            }
            r->max_type  = DRONECAN_PARAM_TYPE_EMPTY;
            r->max_int   = 0;
            r->max_float = 0.0f;
            switch (resp.max_value.union_tag) {
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_INTEGER_VALUE:
                    r->max_type = DRONECAN_PARAM_TYPE_INT;
                    r->max_int  = resp.max_value.integer_value;
                    break;
                case UAVCAN_PROTOCOL_PARAM_NUMERICVALUE_REAL_VALUE:
                    r->max_type  = DRONECAN_PARAM_TYPE_FLOAT;
                    r->max_float = resp.max_value.real_value;
                    break;
                default:
                    break;
            }
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        case DRONECAN_SERVICE_EXECUTE_OPCODE: {
            struct uavcan_protocol_param_ExecuteOpcodeResponse resp;
            if (uavcan_protocol_param_ExecuteOpcodeResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "ExecuteOpcodeResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanAsyncSlot.result.simple.ok = resp.ok;
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        case DRONECAN_SERVICE_RESTART_NODE: {
            struct uavcan_protocol_RestartNodeResponse resp;
            if (uavcan_protocol_RestartNodeResponse_decode(transfer, &resp)) {
                LOG_WARNING(CAN, "RestartNodeResponse decode failed");
                dronecanAsyncSlot.state = DRONECAN_ASYNC_ERROR;
                return;
            }
            dronecanAsyncSlot.result.simple.ok = resp.ok;
            dronecanAsyncSlot.state = DRONECAN_ASYNC_READY;
            break;
        }

        default:
            break;
    }
}

#endif // USE_DRONECAN

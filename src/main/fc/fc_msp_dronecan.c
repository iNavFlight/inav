/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include "platform.h"

#ifdef USE_DRONECAN

#include "common/streambuf.h"

#include "drivers/time.h"

#include "fc/fc_msp_dronecan.h"

bool mspParseDronecanParamGetSetRequest(sbuf_t *src, dronecanParamRequest_t *req)
{
    if (sbufBytesRemaining(src) < 3) { // index(2) + is_write(1) minimum
        return false;
    }

    memset(req, 0, sizeof(*req));
    req->index    = sbufReadU16(src);
    req->is_write = sbufReadU8(src);

    if (req->is_write) {
        if (sbufBytesRemaining(src) < 1) { // value_type
            return false;
        }
        req->value_type = sbufReadU8(src);
        switch (req->value_type) {
            case DRONECAN_PARAM_TYPE_INT: {
                if (sbufBytesRemaining(src) < 8) {
                    return false;
                }
                uint64_t tmp;
                sbufReadData(src, &tmp, sizeof(tmp));
                sbufAdvance(src, sizeof(tmp));
                req->value_int = (int64_t)tmp;
                break;
            }
            case DRONECAN_PARAM_TYPE_FLOAT: {
                if (sbufBytesRemaining(src) < 4) {
                    return false;
                }
                uint32_t raw = sbufReadU32(src);
                memcpy(&req->value_float, &raw, 4);
                break;
            }
            case DRONECAN_PARAM_TYPE_BOOL:
                if (sbufBytesRemaining(src) < 1) {
                    return false;
                }
                req->value_bool = sbufReadU8(src);
                break;
            case DRONECAN_PARAM_TYPE_STRING:
                if (sbufBytesRemaining(src) < 1) {
                    return false;
                }
                req->value_str_len = sbufReadU8(src);
                if (req->value_str_len > sizeof(req->value_str)) {
                    req->value_str_len = sizeof(req->value_str);
                }
                if (sbufBytesRemaining(src) < req->value_str_len) {
                    return false;
                }
                sbufReadData(src, req->value_str, req->value_str_len);
                sbufAdvance(src, req->value_str_len);
                break;
            default: // includes DRONECAN_PARAM_TYPE_EMPTY, which is nonsensical on a write
                return false;
        }
    }

    if (sbufBytesRemaining(src) >= 1) {
        req->req_name_len = sbufReadU8(src);
        if (req->req_name_len > sizeof(req->req_name)) {
            req->req_name_len = sizeof(req->req_name);
        }
        if (sbufBytesRemaining(src) < req->req_name_len) {
            return false;
        }
        sbufReadData(src, req->req_name, req->req_name_len);
        sbufAdvance(src, req->req_name_len);
    }

    return true;
}

void mspSerializeDronecanNodes(sbuf_t *dst)
{
    uint8_t count = dronecanGetNodeCount();
    sbufWriteU8(dst, count);
    for (uint8_t i = 0; i < count; i++) {
        const dronecanNodeInfo_t *node = dronecanGetNode(i);
        sbufWriteU8(dst,  node->nodeID);
        sbufWriteU8(dst,  node->health);
        sbufWriteU8(dst,  node->mode);
        sbufWriteU32(dst, millis() - node->last_seen_ms);
        sbufWriteU32(dst, node->uptime_sec);
        sbufWriteU16(dst, node->vendor_status_code);
    }
}

void mspHandleDronecanAsyncRequest(sbuf_t *src, sbuf_t *dst, mspResult_e *ret)
{
    if (sbufBytesRemaining(src) < 3) {
        *ret = MSP_RESULT_ERROR;
        return;
    }
    uint8_t service_id = (uint8_t)sbufReadU16(src); // MSP uses u16 for protocol compat; UAVCAN service IDs are 8-bit
    uint8_t nodeID = sbufReadU8(src);

    if (dronecanGetState() != STATE_DRONECAN_NORMAL) {
        sbufWriteU8(dst, DRONECAN_STATE_NOT_READY);
        sbufWriteU8(dst, 0);
        *ret = MSP_RESULT_ACK;
        return;
    }

    bool accepted = false;
    if (service_id == DRONECAN_SERVICE_GETNODEINFO) {
        accepted = dronecanAsyncRequest(service_id, nodeID, NULL);
    } else if (service_id == DRONECAN_SERVICE_PARAM_GETSET) {
        dronecanParamRequest_t req;
        if (!mspParseDronecanParamGetSetRequest(src, &req)) {
            *ret = MSP_RESULT_ERROR;
            return;
        }
        accepted = dronecanAsyncRequest(service_id, nodeID, &req);
    } else if (service_id == DRONECAN_SERVICE_EXECUTE_OPCODE) {
        if (sbufBytesRemaining(src) < 1) {
            *ret = MSP_RESULT_ERROR;
            return;
        }
        uint8_t opcode = sbufReadU8(src);
        accepted = dronecanAsyncRequest(service_id, nodeID, &opcode);
    } else if (service_id == DRONECAN_SERVICE_RESTART_NODE) {
        accepted = dronecanAsyncRequest(service_id, nodeID, NULL);
    }

    sbufWriteU8(dst, accepted ? 0 : 1); // 0=accepted, 1=busy or unrecognised service_id
    sbufWriteU8(dst, dronecanAsyncSlot.seq);
    *ret = MSP_RESULT_ACK;
}

void mspSerializeDronecanAsyncResult(sbuf_t *dst)
{
    sbufWriteU8(dst, (uint8_t)dronecanAsyncSlot.state);
    sbufWriteU8(dst, dronecanAsyncSlot.seq);
    sbufWriteU16(dst, dronecanAsyncSlot.service_id);
    sbufWriteU8(dst, dronecanAsyncSlot.node_id);

    if (dronecanAsyncSlot.state == DRONECAN_ASYNC_READY) {
        switch (dronecanAsyncSlot.service_id) {
            case DRONECAN_SERVICE_GETNODEINFO: {
                const dronecanGetNodeInfoResult_t *r = &dronecanAsyncSlot.result.node_info;
                sbufWriteU8(dst, r->name_len);
                sbufWriteDataSafe(dst, r->name, r->name_len);
                sbufWriteU8(dst,  r->sw_major);
                sbufWriteU8(dst,  r->sw_minor);
                sbufWriteU8(dst,  r->sw_optional_field_flags);
                sbufWriteU32(dst, r->sw_vcs_commit);
                sbufWriteU8(dst,  r->hw_major);
                sbufWriteU8(dst,  r->hw_minor);
                sbufWriteDataSafe(dst, r->hw_unique_id, 16);
                break;
            }
            case DRONECAN_SERVICE_PARAM_GETSET: {
                const dronecanParamResult_t *r = &dronecanAsyncSlot.result.param;
                sbufWriteU8(dst, r->name_len);
                sbufWriteDataSafe(dst, r->name, r->name_len);
                sbufWriteU8(dst, r->type);
                switch (r->type) {
                    case DRONECAN_PARAM_TYPE_INT: {
                        uint64_t tmp;
                        memcpy(&tmp, &r->value_int, sizeof(tmp));
                        sbufWriteData(dst, &tmp, sizeof(tmp));
                        break;
                    }
                    case DRONECAN_PARAM_TYPE_FLOAT: {
                        uint32_t raw;
                        memcpy(&raw, &r->value_float, 4);
                        sbufWriteU32(dst, raw);
                        break;
                    }
                    case DRONECAN_PARAM_TYPE_BOOL:
                        sbufWriteU8(dst, r->value_bool);
                        break;
                    case DRONECAN_PARAM_TYPE_STRING:
                        sbufWriteU8(dst, r->value_str_len);
                        sbufWriteDataSafe(dst, r->value_str, r->value_str_len);
                        break;
                    default:
                        break;
                }
                sbufWriteU8(dst, r->min_type);
                if (r->min_type == DRONECAN_PARAM_TYPE_INT) {
                    uint64_t utmp;
                    memcpy(&utmp, &r->min_int, sizeof(utmp));
                    sbufWriteData(dst, &utmp, sizeof(utmp));
                } else if (r->min_type == DRONECAN_PARAM_TYPE_FLOAT) {
                    uint32_t raw;
                    memcpy(&raw, &r->min_float, 4);
                    sbufWriteU32(dst, raw);
                }
                sbufWriteU8(dst, r->max_type);
                if (r->max_type == DRONECAN_PARAM_TYPE_INT) {
                    uint64_t utmp;
                    memcpy(&utmp, &r->max_int, sizeof(utmp));
                    sbufWriteData(dst, &utmp, sizeof(utmp));
                } else if (r->max_type == DRONECAN_PARAM_TYPE_FLOAT) {
                    uint32_t raw;
                    memcpy(&raw, &r->max_float, 4);
                    sbufWriteU32(dst, raw);
                }
                break;
            }
            case DRONECAN_SERVICE_EXECUTE_OPCODE:
            case DRONECAN_SERVICE_RESTART_NODE:
                sbufWriteU8(dst, dronecanAsyncSlot.result.simple.ok ? 1 : 0);
                break;
        }
        dronecanAsyncSlot.state = DRONECAN_ASYNC_IDLE;
    } else if (dronecanAsyncSlot.state == DRONECAN_ASYNC_ERROR) {
        dronecanAsyncSlot.state = DRONECAN_ASYNC_IDLE;
    }
}

#endif

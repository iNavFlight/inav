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

#pragma once

#include <stdbool.h>

#include "platform.h"

#ifdef USE_DRONECAN

#include "drivers/dronecan/dronecan.h"
#include "msp/msp.h"

/* Parses the DRONECAN_SERVICE_PARAM_GETSET body of an
 * MSP2_INAV_DRONECAN_ASYNC_REQUEST payload (everything after service_id and
 * nodeID) into *req. Returns false, leaving *ret-handling to the caller, if
 * the payload is truncated for the declared is_write/value_type/name
 * lengths -- never dispatches a request built from a short read. */
bool mspParseDronecanParamGetSetRequest(sbuf_t *src, dronecanParamRequest_t *req);

/* MSP2_INAV_DRONECAN_NODES: serializes the DroneCAN node table to dst. */
void mspSerializeDronecanNodes(sbuf_t *dst);

/* MSP2_INAV_DRONECAN_ASYNC_REQUEST: parses service_id/nodeID and the
 * per-service payload from src, kicks off the async request, and writes the
 * accepted/seq reply to dst. Sets *ret on both the error and success paths,
 * matching the calling convention of fc_msp.c's command switch. */
void mspHandleDronecanAsyncRequest(sbuf_t *src, sbuf_t *dst, mspResult_e *ret);

/* MSP2_INAV_DRONECAN_ASYNC_RESULT: serializes the current dronecanAsyncSlot
 * state/result to dst, then clears a READY or ERROR slot back to IDLE. */
void mspSerializeDronecanAsyncResult(sbuf_t *dst);

#endif

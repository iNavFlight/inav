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

#include "platform.h"

#include "target/common.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/* Single external-linkage definition TU for the MAVLink helpers. All
 * other TUs include the MAVLink headers with MAVLINK_SEPARATE_HELPERS set
 * (see rx/mavlink.h, mavlink/mavlink_types.h) and get only the extern
 * prototypes from protocol.h, so each helper — and its local static data,
 * e.g. the mavlink_message_crcs[] table — exists exactly once. */
#ifndef MAVLINK_COMM_NUM_BUFFERS
#define MAVLINK_COMM_NUM_BUFFERS MAX_MAVLINK_PORTS
#endif
#define MAVLINK_SEPARATE_HELPERS
#include "storm32/mavlink.h"
#include "mavlink_helpers.h"

#pragma GCC diagnostic pop

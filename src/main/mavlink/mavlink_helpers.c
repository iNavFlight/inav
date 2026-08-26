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

/* Single translation unit that compiles the MAVLink helper definitions
 * (mavlink_get_msg_entry, mavlink_parse_char, ...) with external linkage.
 * All other TUs include the MAVLink headers with MAVLINK_SEPARATE_HELPERS
 * set (see rx/mavlink.h and mavlink/mavlink_types.h), so they see only the
 * extern prototypes from protocol.h and no longer each instantiate a
 * private static copy of every helper — including the ~3.7 KB
 * mavlink_message_crcs[] table inside mavlink_get_msg_entry(), which was
 * duplicated once per including TU. */
#define MAVLINK_COMM_NUM_BUFFERS MAX_MAVLINK_PORTS
#define MAVLINK_SEPARATE_HELPERS
#include "storm32/mavlink.h"
#include "mavlink_helpers.h"

#pragma GCC diagnostic pop

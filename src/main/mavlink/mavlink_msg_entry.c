/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mavlink/mavlink_types.h"

#if defined(USE_TELEMETRY_MAVLINK) || defined(USE_SERIALRX_MAVLINK)

/*
 * The single instantiation of the MAVLink helpers for the whole firmware.
 * MAVLINK_SEPARATE_HELPERS (see mavlink_msg_entry.h) leaves MAVLINK_HELPER
 * empty and stops protocol.h including these, so this is the only place they
 * are defined, with external linkage.
 */
#include "mavlink_helpers.h"

#endif

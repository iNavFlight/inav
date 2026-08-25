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

#pragma once

/*
 * protocol.h compiles the MAVLink helpers as MAVLINK_HELPER = static and
 * includes them into every translation unit, so each one gets a private copy of
 * every helper it touches - and of the static data inside them. Nothing warns
 * about it: the copies are static data rather than unused code, so no
 * diagnostic fires and nothing shows up in a diff.
 *
 * The message table is the expensive case. mavlink_get_msg_entry() holds it as
 * a function-local static, and two callers - routing and runtime - meant two
 * copies of a 4044 byte table.
 *
 * MAVLINK_SEPARATE_HELPERS is the library's own switch for this: protocol.h
 * then emits declarations only, and mavlink_msg_entry.c compiles the helpers
 * once with external linkage. This header must be included before
 * storm32/mavlink.h so the switch is set before protocol.h is reached.
 */
#define MAVLINK_SEPARATE_HELPERS

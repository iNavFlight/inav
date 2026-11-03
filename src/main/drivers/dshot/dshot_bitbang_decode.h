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

#include <stdbool.h>
#include <stdint.h>

// Number of samples taken of every GCR bit of the ESC reply; the decoder's bit timing
// assumes this rate
#define DSHOT_BITBANG_TELEMETRY_OVERSAMPLE 3

/*
 * Decode one ESC reply out of a sampled GPIO port.
 *
 * buffer holds count consecutive reads of the port's input data register, one per
 * DSHOT_BITBANG_TELEMETRY_OVERSAMPLE-th of a GCR bit, and bit is the pin to look at.
 * idleHigh says which level the line rests at between frames (true for the inverted
 * signalling bidirectional DSHOT normally uses).
 *
 * Returns the 16-bit telemetry value with its checksum removed, DSHOT_TELEMETRY_INVALID
 * for a reply that did not decode or failed its checksum, or DSHOT_TELEMETRY_NOEDGE when
 * the line never left idle (the ESC did not reply).
 */
uint32_t dshotBitbangDecode(const uint16_t *buffer, uint32_t count, uint32_t bit, bool idleHigh);

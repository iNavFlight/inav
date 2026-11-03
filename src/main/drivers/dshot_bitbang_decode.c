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
 *
 * Ported from Betaflight's dshot_bitbang_decode.c (decode_bb, GPLv3).
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_DSHOT_BITBANG

#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bidir_dshot.h"
#include "drivers/dshot_bitbang_decode.h"
#include "drivers/time.h"

// A reply is 21 GCR bits; allow two bits of timing slack either way
#define MIN_VALID_BBSAMPLES ((21 - 2) * DSHOT_BITBANG_TELEMETRY_OVERSAMPLE)
#define MAX_VALID_BBSAMPLES ((21 + 2) * DSHOT_BITBANG_TELEMETRY_OVERSAMPLE)

// How often the leading idle run is re-measured to update preambleSkip
#define MARGIN_CHECK_INTERVAL_US 500000

// Samples of idle to keep ahead of the first edge when skipping the preamble
#define DSHOT_TELEMETRY_START_MARGIN 5

/*
 * The ESC answers about 30 us after the frame, so the capture starts with a run of idle
 * samples. Their count is measured over MARGIN_CHECK_INTERVAL_US and the decoder then
 * jumps most of that run on later replies instead of scanning it. Shared by every motor:
 * all ESCs on a board answer with about the same delay.
 */
static uint32_t minMargin = UINT32_MAX;
static timeUs_t nextMarginCheckUs = 0;
static uint32_t preambleSkip = 0;

// The GCR run lengths were packed MSB first into value as 1000... groups; strip the start
// bit, map the four GCR quintets to nibbles and verify the checksum
static uint32_t decodeGcrValue(uint32_t value)
{
#define iv 0xffffffff
    value &= 0xfffff;
    static const uint32_t decode[32] = {
        iv, iv, iv, iv, iv, iv, iv, iv, iv, 9, 10, 11, iv, 13, 14, 15,
        iv, iv, 2, 3, iv, 5, 6, 7, iv, 0, 8, 1, iv, 4, 12, iv };
#undef iv

    uint32_t decodedValue = decode[value & 0x1f];
    decodedValue |= decode[(value >> 5) & 0x1f] << 4;
    decodedValue |= decode[(value >> 10) & 0x1f] << 8;
    decodedValue |= decode[(value >> 15) & 0x1f] << 12;

    uint32_t csum = decodedValue;
    csum = csum ^ (csum >> 8); // xor bytes
    csum = csum ^ (csum >> 4); // xor nibbles

    if ((csum & 0xf) != 0xf || decodedValue > 0xffff) {
        return DSHOT_TELEMETRY_INVALID;
    }

    return decodedValue >> 4;
}

uint32_t dshotBitbangDecode(const uint16_t *buffer, uint32_t count, uint32_t bit, bool idleHigh)
{
    const timeUs_t now = micros();
    const uint16_t mask = 1 << bit;
    // The loops below are written for idle-high lines: a reply starts with the first low
    // sample. Flipping the pin's sample makes an idle-low line look the same to them.
    const uint16_t flip = idleHigh ? 0 : mask;

    uint16_t lastValue = 0;
    uint32_t value = 0;

    const uint16_t *p = buffer;
    const uint16_t *endP = p + count - MIN_VALID_BBSAMPLES;

    // Jump forward in the buffer to just before where the first low sample is expected
    p += preambleSkip;

    // Eliminate the leading idle level by looking for the first low sample.
    // Manual loop unrolling and branch hinting to produce faster code.
    while (p < endP) {
        if (__builtin_expect(!((*p++ ^ flip) & mask), 0) ||
            __builtin_expect(!((*p++ ^ flip) & mask), 0) ||
            __builtin_expect(!((*p++ ^ flip) & mask), 0) ||
            __builtin_expect(!((*p++ ^ flip) & mask), 0)) {
            break;
        }
    }

    const uint32_t startMargin = p - buffer;

    if (p >= endP) {
        // No reply is fine: a busy ESC may skip one. Caller sees DSHOT_TELEMETRY_NOEDGE.
        // Widen the start margin in case the preamble skip went past a late reply.
        if (preambleSkip > 0) {
            preambleSkip--;
        }
        return DSHOT_TELEMETRY_NOEDGE;
    }

    const int remaining = MIN((int)(count - startMargin), MAX_VALID_BBSAMPLES);
    const uint16_t *oldP = p;
    uint32_t bits = 0;
    endP = p + remaining;

    while (endP > p) {
        // Look for the next edge. Manual loop unrolling and branch hinting to produce faster code.
        if (__builtin_expect(((*p++ ^ flip) & mask) != lastValue, 0) ||
            __builtin_expect(((*p++ ^ flip) & mask) != lastValue, 0) ||
            __builtin_expect(((*p++ ^ flip) & mask) != lastValue, 0) ||
            __builtin_expect(((*p++ ^ flip) & mask) != lastValue, 0)) {
            if (endP > p) {
                // A level of length n gets decoded to a sequence of bits of
                // the form 1000 with a length of (n+1) / 3 to account for 3x
                // oversampling.
                const int len = MAX((int)(p - oldP + 1) / DSHOT_BITBANG_TELEMETRY_OVERSAMPLE, 1);
                bits += len;
                value <<= len;
                value |= 1 << (len - 1);
                oldP = p;
                lastValue = (*(p - 1) ^ flip) & mask;
            }
        }
    }

    // The last run has no closing edge, the line just stays at its final level: its length
    // is whatever is left of the 21 bits
    if (bits < 18) {
        return DSHOT_TELEMETRY_NOEDGE;
    }

    const int nlen = 21 - bits;
    if (nlen < 0) {
        return DSHOT_TELEMETRY_NOEDGE;
    }

    // Data appears valid: track how early the reply came
    if (startMargin < minMargin) {
        minMargin = startMargin;
    }

    if (cmpTimeUs(now, nextMarginCheckUs) >= 0) {
        nextMarginCheckUs += MARGIN_CHECK_INTERVAL_US;

        // Handle a skipped check
        if (nextMarginCheckUs < now) {
            nextMarginCheckUs = now + DSHOT_TELEMETRY_START_MARGIN;
        }

        if (minMargin > DSHOT_TELEMETRY_START_MARGIN) {
            preambleSkip = minMargin - DSHOT_TELEMETRY_START_MARGIN;
        } else {
            preambleSkip = 0;
        }

        minMargin = UINT32_MAX;
    }

    if (nlen > 0) {
        value <<= nlen;
        value |= 1 << (nlen - 1);
    }

    return decodeGcrValue(value);
}

#endif // USE_DSHOT_BITBANG

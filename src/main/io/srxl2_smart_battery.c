/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/* Smart Battery wire layout and measurements: docs/Spektrum Smart ESC.md. */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_MOTOR_SRXL2

#include "io/srxl2_smart_battery.h"

static uint16_t le16(const uint8_t *p)
{
    return p[0] | ((uint16_t)p[1] << 8);
}

static bool isPageFresh(const srxl2SmartBatteryPage_t *page, uint32_t now)
{
    return page->valid && (uint32_t)(now - page->receivedMs) <= SRXL2_SMART_BATTERY_STALE_MS;
}

void srxl2SmartBatteryReceive(srxl2SmartBatteryState_t *state, const uint8_t payload[16], uint32_t nowMs)
{
    if (payload[0] != SRXL2_TELEM_SENSOR_SMART_BATTERY) {
        return;
    }
    srxl2SmartBatterySource_t *source = NULL;
    for (unsigned i = 0; i < SRXL2_SMART_BATTERY_SLOTS; i++) {
        if (state->source[i].used && state->source[i].secondaryId == payload[1]
            && state->source[i].batteryId == (payload[2] & 0x0F)) {
            source = &state->source[i];
            break;
        }
    }
    if (!source) {
        for (unsigned i = 0; i < SRXL2_SMART_BATTERY_SLOTS; i++) {
            if (!state->source[i].used) {
                source = &state->source[i];
                source->used = true;
                source->secondaryId = payload[1];
                source->batteryId = payload[2] & 0x0F;
                break;
            }
        }
    }
    if (!source) {
        // Keep the existing slot identities stable for telemetry consumers.
        state->droppedSources++;
        return;
    }
    const uint8_t type = payload[2] >> 4;
    const unsigned pageIndex = type <= 3 ? type : type == 8 ? 4 : type == 9 ? 5 : 6;
    srxl2SmartBatteryPage_t *page = &source->pages[pageIndex];
    memcpy(page->payload, payload, sizeof(page->payload));
    page->receivedMs = nowMs;
    page->count++;
    page->valid = true;
    if (type >= 1 && type <= 3) {
        for (unsigned i = 0; i < 6; i++) {
            if (le16(&payload[4 + 2 * i]) != UINT16_MAX) {
                const uint8_t count = (type - 1) * 6 + i + 1;
                if (count > source->observedCellCount) {
                    source->observedCellCount = count;
                }
            }
        }
    }
}

void srxl2SmartBatteryInvalidate(srxl2SmartBatteryState_t *state)
{
    // Preserve raw evidence and slot identity, but never resurrect old values
    // merely because unrelated ESC telemetry restores the link.
    for (unsigned i = 0; i < SRXL2_SMART_BATTERY_SLOTS; i++) {
        for (unsigned p = 0; p < SRXL2_SMART_BATTERY_PAGE_COUNT; p++) {
            state->source[i].pages[p].valid = false;
        }
    }
}

bool srxl2SmartBatteryGet(const srxl2SmartBatteryState_t *state, uint8_t slot, uint32_t nowMs, srxl2SmartBatteryTelemetry_t *out)
{
    if (!out) {
        return false;
    }
    memset(out, 0, sizeof(*out));
    if (slot >= SRXL2_SMART_BATTERY_SLOTS || !state->source[slot].used) {
        return false;
    }
    const srxl2SmartBatterySource_t *source = &state->source[slot];
    out->secondaryId = source->secondaryId;
    out->batteryId = source->batteryId;
    out->cellCount = source->observedCellCount;
    const srxl2SmartBatteryPage_t *id = &source->pages[4];
    if (isPageFresh(id, nowMs) && id->payload[4] > 0 && id->payload[4] <= SRXL2_SMART_BATTERY_MAX_CELLS) {
        out->cellCount = id->payload[4];
        out->cellCountValid = true;
    }

    // Avian Lite reports placeholder cells without a battery data connection.
    // Require populated capacity metadata; this is not a published presence flag.
    const srxl2SmartBatteryPage_t *limits = &source->pages[5];
    const uint16_t capacity = le16(&limits->payload[4]);
    out->metadataAvailable = isPageFresh(limits, nowMs) && capacity != 0 && capacity != UINT16_MAX;
    if (!out->metadataAvailable) {
        return false;
    }

    uint32_t temperatureAge = UINT32_MAX;
    for (unsigned p = 1; p <= 3; p++) {
        const srxl2SmartBatteryPage_t *page = &source->pages[p];
        if (!isPageFresh(page, nowMs)) {
            continue;
        }
        bool hasCells = false;
        for (unsigned i = 0; i < 6; i++) {
            const unsigned cell = (p - 1) * 6 + i;
            const uint16_t mv = le16(&page->payload[4 + 2 * i]);
            if (mv != UINT16_MAX && (!out->cellCountValid || cell < out->cellCount)) {
                out->cellMv[cell] = mv;
                out->validCells |= 1u << cell;
                hasCells = true;
            }
        }
        // An all-FF page must not report -1 C; a populated cell page may.
        const int16_t temp = page->payload[3] < 128 ? page->payload[3] : (int16_t)page->payload[3] - 256;
        const uint32_t age = nowMs - page->receivedMs;
        if (hasCells && age < temperatureAge && temp > -128 && temp < 127) {
            // Exclude signed extrema conservatively, not as verified sentinels.
            out->temperatureDeciC = temp * 10;
            out->temperatureValid = true;
            temperatureAge = age;
        }
    }
    return out->validCells != 0 || out->temperatureValid;
}
#endif

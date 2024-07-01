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

#include <stdint.h>

#include "platform.h"

#include "build/debug.h"

#include "common/utils.h"
#include "common/time.h"

#include "telemetry/sbus2.h"

#include "rx/sbus.h"

#ifdef USE_SBUS2_TELEMETRY

const uint8_t sbus2SlotIds[SBUS2_TELEMETRY_PAGES][SBUS2_TELEMETRY_SLOTS] = {
    {0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3},
    {0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3},
    {0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB},
    {0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB}
};


sbus2_telemetry_frame_t sbusTelemetryData[SBUS2_TELEMETRY_PAGES][SBUS2_TELEMETRY_SLOTS] = {{}};
uint8_t sbusTelemetryDataStatus[SBUS2_TELEMETRY_PAGES][SBUS2_TELEMETRY_SLOTS] = {{}};

void handleSbus2Telemetry(timeUs_t currentTimeUs) 
{
    UNUSED(currentTimeUs);

    // update telemetry info
}

void taskSendSbus2Telemetry(timeUs_t currentTimeUs)
{
    uint8_t telemetryPage = sbusGetCurrentTelemetryPage();
    uint8_t lastFrame = sbusGetLastFrameTime();
    timeUs_t elapsedTime = currentTimeUs - lastFrame - MS2US(2);

    // 2ms after sbus2 frame = slot 0
    // +660us for next slot
    if(elapsedTime > 0) {
        uint8_t slot = elapsedTime % 660;
        if(slot < SBUS2_TELEMETRY_SLOTS) {
            if(sbusTelemetryDataStatus[telemetryPage][slot] != 0) {
                sbusTelemetryData[telemetryPage][slot].slotId = sbus2SlotIds[telemetryPage][slot];
                // send 
                sbusTelemetryDataStatus[telemetryPage][slot] = 0;
            }
        }
    }
}

#endif

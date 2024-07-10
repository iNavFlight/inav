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

#include "telemetry/telemetry.h"
#include "telemetry/sbus2.h"

#include "rx/sbus.h"

#ifdef USE_SBUS2_TELEMETRY

const uint8_t sbus2SlotIds[SBUS2_SLOT_COUNT] = {
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB
};


sbus2_telemetry_frame_t sbusTelemetryData[SBUS2_SLOT_COUNT] = {};
uint8_t sbusTelemetryDataUsed[SBUS2_SLOT_COUNT] = {};
timeUs_t sbusTelemetryDataLastSent[SBUS2_SLOT_COUNT] = {};

void handleSbus2Telemetry(timeUs_t currentTimeUs) 
{
    UNUSED(currentTimeUs);

    // TODO: placeholder

    int16_t temp = 42 | 0x4000;
    sbusTelemetryData[0].payload.temp125.tempHigh = ((temp >> 8) & 0xFF);
    sbusTelemetryData[0].payload.temp125.tempLow = (temp & 0xFF);
    sbusTelemetryDataUsed[0] = 1;

    int16_t rpm = 9000;
    sbusTelemetryData[1].payload.rpm.rpmHigh = (rpm >> 8) & 0xFF;
    sbusTelemetryData[1].payload.rpm.rpmLow = rpm & 0xFF;
    sbusTelemetryDataUsed[1] = 1;
    // P1S0: TEMP
    // P1S1: RPM
    // update telemetry info
}

void taskSendSbus2Telemetry(timeUs_t currentTimeUs)
{
    if(!telemetrySharedPort || rxConfig()->receiverType != RX_TYPE_SERIAL || rxConfig()->serialrx_provider != SERIALRX_SBUS2)
    {
        return;
    }

    uint8_t telemetryPage = sbusGetCurrentTelemetryPage();
    uint8_t lastFrame = sbusGetLastFrameTime();
    timeUs_t elapsedTime = currentTimeUs - lastFrame - MS2US(2);

    // 2ms after sbus2 frame = slot 0
    // +660us for next slot
    if(elapsedTime > MS2US(2)) {
        uint8_t slot = elapsedTime % 660;
        int slotIndex = (telemetryPage * SBUS2_TELEMETRY_SLOTS) + slot;
        if(slot < SBUS2_TELEMETRY_SLOTS && slotIndex < SBUS2_SLOT_COUNT && (currentTimeUs - sbusTelemetryDataLastSent[slotIndex]) > MS2US(2)) {
            if(sbusTelemetryDataUsed[slotIndex] != 0) {
                sbusTelemetryData[slotIndex].slotId = sbus2SlotIds[slotIndex];
                // send
                serialWriteBuf(telemetrySharedPort, (const uint8_t *)&sbusTelemetryData[slotIndex], sizeof(sbus2_telemetry_frame_t));
                sbusTelemetryDataLastSent[slotIndex] = currentTimeUs;
            }
        }
    }
}

#endif

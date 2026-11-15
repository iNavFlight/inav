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

#include "drivers/io_types.h"

struct timerHardware_s;

#ifdef USE_DSHOT_BITBANG

typedef enum {
    DSHOT_BITBANG_STATUS_OK = 0,
    DSHOT_BITBANG_STATUS_NO_PACER,          // no TMR1/TMR8 channel left on a timer nothing else uses
    DSHOT_BITBANG_STATUS_NO_DMA,            // no free DMA channel
    DSHOT_BITBANG_STATUS_TOO_MANY_PORTS,    // motors spread over more GPIO ports than there are groups
} dshotBitbangStatus_e;

// Selected for this boot by the DSHOT driver (dshotInit()); bitRateHz is the
// DSHOT bit rate, 150000 / 300000 / 600000
void dshotBitbangInit(uint32_t bitRateHz);
bool dshotBitbangIsActive(void);
dshotBitbangStatus_e dshotBitbangGetStatus(void);

// Per motor at output init: only the pin is taken, the timer channel of the output stays free
bool dshotBitbangMotorConfig(const struct timerHardware_s *timHw, uint8_t motorIndex, bool enableOutput);
bool dshotBitbangMotorIsConfigured(uint8_t motorIndex);
// After every other user of timers and DMA has had its pick: groups the motor pins by GPIO
// port and gives each group a pacer channel and a DMA channel. On false nothing sends;
// the motor driver then releases this driver and falls back to its timers.
bool dshotBitbangPostInit(void);
// The output a motor was handed over with, for that fallback; false if it never was
bool dshotBitbangGetMotorOutput(uint8_t motorIndex, const struct timerHardware_s **timHw, bool *enableOutput);
// Give back every timer and DMA channel taken and go inactive
void dshotBitbangRelease(void);

// One motor update: the packets of every motor (checksum included, MSB first), then the frame
void dshotBitbangWrite(uint8_t motorIndex, uint16_t packet);
void dshotBitbangUpdateComplete(void);
// Decodes the replies to the last frame and publishes them; false while the reply window
// of that frame is still open, so the caller skips this update
bool dshotBitbangDecodeTelemetry(void);

// Finish the exchange in flight and leave the lines at idle, driven (Stop) or held only by
// their pulls (Disconnect). The next frame takes the pins back either way.
void dshotBitbangStop(void);
void dshotBitbangDisconnect(void);

// ESC passthrough: the 4-way interface drives the pin itself in between
ioTag_t dshotBitbangGetMotorPinTag(uint8_t motorIndex);
void dshotBitbangRestoreMotorPin(uint8_t motorIndex);

#endif

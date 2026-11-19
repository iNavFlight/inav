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
#include "drivers/pwm_mapping.h"    // motorPwmProtocolTypes_e

struct timerHardware_s;

// DSHOT output, used by the motor driver (pwm_output.c) for the DSHOT protocols. The DSHOT
// commands (sendDShotCommand(), initDShotCommands()) are declared in pwm_output.h with the
// rest of the motor API and implemented here as well.
#ifdef USE_DSHOT

// Once, from pwmMotorPreconfigure(): fixes the bit rate and the frame rate, and on AT32
// hands bidirectional DSHOT to the bitbang driver (dshot_bitbang.c)
void dshotInit(motorPwmProtocolTypes_e protocol, uint32_t updateRateHz);

// Per motor at output init: the timer channel of the output takes the pin, or the bitbang
// driver does. True when the motor got its output; frames only go out on a timer channel
// once its DMA is set up as well.
bool dshotMotorConfig(const struct timerHardware_s *timHw, uint8_t motorIndex, bool enableOutput);
// After every other timer and DMA user has had its pick (fc_init): the bitbang driver takes
// what is left, or the motors go back to plain DSHOT on their timers. False if that fails too.
bool dshotMotorPostInit(void);

// Throttle for the next frame, 11-bit range [0;2047]; sent by dshotCompleteMotorUpdate()
void dshotWriteMotor(uint8_t motorIndex, uint16_t value);
// Set the telemetry bit of the motor's next frame (serial ESC telemetry request)
void dshotRequestTelemetry(uint8_t motorIndex);
// One motor update, rate limited to the frame rate: the replies to the last frame are
// decoded first, then a pending DSHOT command or the throttle frame goes out. With
// motorsEnabled false the 4-way interface owns the pins and the bitbang driver sends nothing.
void dshotCompleteMotorUpdate(bool motorsEnabled);

// Motors disabled (pwmDisableMotors()): finish the bitbang exchange in flight
void dshotStop(void);
// Around a flash write, which stalls the CPU: plain DSHOT replays a zero-throttle frame by
// circular DMA, bidirectional DSHOT finishes the exchange in flight and leaves the pins to
// their pulls until the next frame
void dshotSetDMACircular(bool circular);

// ESC passthrough: the 4-way interface drives the pin itself in between
ioTag_t dshotGetMotorPinTag(uint8_t motorIndex);
void dshotRestoreMotorPin(uint8_t motorIndex);

#endif

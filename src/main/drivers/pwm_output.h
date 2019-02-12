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

#pragma once

#include "drivers/io_types.h"
#include "drivers/time.h"

typedef enum {
    PWM_TYPE_STANDARD = 0,
    PWM_TYPE_ONESHOT125,
    PWM_TYPE_ONESHOT42,
    PWM_TYPE_MULTISHOT,
    PWM_TYPE_BRUSHED,
    PWM_TYPE_DSHOT150,
    PWM_TYPE_DSHOT300,
    PWM_TYPE_DSHOT600,
    PWM_TYPE_DSHOT1200,
} motorPwmProtocolTypes_e;

void pwmWriteMotor(uint8_t index, uint16_t value);
void pwmShutdownPulsesForAllMotors(uint8_t motorCount);
void pwmCompleteDshotUpdate(uint8_t motorCount);
bool isMotorProtocolDshot(void);

void pwmWriteServo(uint8_t index, uint16_t value);

void pwmDisableMotors(void);
void pwmEnableMotors(void);
struct timerHardware_s;
bool pwmMotorConfig(const struct timerHardware_s *timerHardware, uint8_t motorIndex, uint16_t motorPwmRate, motorPwmProtocolTypes_e proto, bool enableOutput);
bool pwmServoConfig(const struct timerHardware_s *timerHardware, uint8_t servoIndex, uint16_t servoPwmRate, uint16_t servoCenterPulse, bool enableOutput);
void pwmWriteBeeper(bool onoffBeep);
void beeperPwmInit(ioTag_t tag, uint16_t frequency);
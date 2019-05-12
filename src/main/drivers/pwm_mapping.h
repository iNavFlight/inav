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
#include "flight/mixer.h"
#include "flight/servos.h"

#if defined(TARGET_MOTOR_COUNT)
#define MAX_PWM_MOTORS TARGET_MOTOR_COUNT
#define MAX_PWM_SERVOS 8
#define MAX_MOTORS  TARGET_MOTOR_COUNT
#define MAX_SERVOS  8

#else
#define MAX_PWM_MOTORS  12
#define MAX_PWM_SERVOS  8
#define MAX_MOTORS  12
#define MAX_SERVOS  8
#endif

#define PWM_TIMER_HZ    1000000

#define PULSE_1MS   (1000)      // 1ms pulse width

#define MAX_INPUTS  8

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
    PWM_TYPE_SERIALSHOT,
} motorPwmProtocolTypes_e;

typedef enum {
    PWM_INIT_ERROR_NONE = 0,
    PWM_INIT_ERROR_TOO_MANY_MOTORS,
    PWM_INIT_ERROR_TOO_MANY_SERVOS,
    PWM_INIT_ERROR_NOT_ENOUGH_MOTOR_OUTPUTS,
    PWM_INIT_ERROR_NOT_ENOUGH_SERVO_OUTPUTS,
    PWM_INIT_ERROR_TIMER_INIT_FAILED,
} pwmInitError_e;

typedef struct rangefinderIOConfig_s {
    ioTag_t triggerTag;
    ioTag_t echoTag;
} rangefinderIOConfig_t;

typedef struct {
    bool usesHwTimer;
    bool isDSHOT;
    bool isSerialShot;
} motorProtocolProperties_t;

// This indexes into the read-only hardware definition structure, timerHardware_t
enum {
    PWM1 = 0,
    PWM2,
    PWM3,
    PWM4,
    PWM5,
    PWM6,
    PWM7,
    PWM8,
    PWM9,
    PWM10,
    PWM11,
    PWM12,
    PWM13,
    PWM14,
    PWM15,
    PWM16,
    PWM17,
    PWM18,
    PWM19,
    PWM20
};

bool pwmMotorAndServoInit(void);
const motorProtocolProperties_t * getMotorProtocolProperties(motorPwmProtocolTypes_e proto);
pwmInitError_e getPwmInitError(void);
const char * getPwmInitErrorMessage(void);

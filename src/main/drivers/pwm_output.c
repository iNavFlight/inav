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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"
#include "build/debug.h"

#include "drivers/io.h"
#include "timer.h"
#include "pwm_mapping.h"
#include "pwm_output.h"
#include "io_pca9685.h"

#include "io/pwmdriver_i2c.h"

#include "config/feature.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#define MULTISHOT_5US_PW    (MULTISHOT_TIMER_MHZ * 5)
#define MULTISHOT_20US_MULT (MULTISHOT_TIMER_MHZ * 20 / 1000.0f)

typedef void (*pwmWriteFuncPtr)(uint8_t index, uint16_t value);  // function pointer used to write motors

typedef struct {
    volatile timCCR_t *ccr;
    TIM_TypeDef *tim;
    uint16_t period;
    pwmWriteFuncPtr pwmWritePtr;
} pwmOutputPort_t;

static pwmOutputPort_t pwmOutputPorts[MAX_PWM_OUTPUT_PORTS];

static pwmOutputPort_t *motors[MAX_PWM_MOTORS];
static pwmOutputPort_t *servos[MAX_PWM_SERVOS];

#ifdef BEEPER_PWM
static pwmOutputPort_t  beeperPwmPort;
static pwmOutputPort_t *beeperPwm;
static uint16_t beeperFrequency = 0;
#endif

static uint8_t allocatedOutputPortCount = 0;

static bool pwmMotorsEnabled = true;

static void pwmOutConfigTimer(pwmOutputPort_t * p, const timerHardware_t *timerHardware, uint8_t mhz, uint16_t period, uint16_t value)
{
    timerConfigBase(timerHardware->tim, period, mhz);
    timerPWMConfigChannel(timerHardware->tim, timerHardware->channel, timerHardware->output & TIMER_OUTPUT_N_CHANNEL, timerHardware->output & TIMER_OUTPUT_INVERTED, value);

    if (timerHardware->output & TIMER_OUTPUT_ENABLED) {
        timerPWMStart(timerHardware->tim, timerHardware->channel, timerHardware->output & TIMER_OUTPUT_N_CHANNEL);
    }

    timerEnable(timerHardware->tim);

    p->ccr = timerCCR(timerHardware->tim, timerHardware->channel);
    p->period = period;
    p->tim = timerHardware->tim;

    *p->ccr = 0;
}

static pwmOutputPort_t *pwmOutConfigMotor(const timerHardware_t *timerHardware, uint8_t mhz, uint16_t period, uint16_t value, bool enableOutput)
{
    if (allocatedOutputPortCount >= MAX_PWM_OUTPUT_PORTS) {
        DEBUG_TRACE("Attempt to allocate PWM output beyond MAX_PWM_OUTPUT_PORTS");
        return NULL;
    }

    pwmOutputPort_t *p = &pwmOutputPorts[allocatedOutputPortCount++];

    const IO_t io = IOGetByTag(timerHardware->tag);
    IOInit(io, OWNER_MOTOR, RESOURCE_OUTPUT, allocatedOutputPortCount);

    if (enableOutput) {
        IOConfigGPIOAF(io, IOCFG_AF_PP, timerHardware->alternateFunction);
    }
    else {
        // If PWM outputs are disabled - configure as GPIO and drive low
        IOConfigGPIO(io, IOCFG_OUT_OD);
        IOLo(io);
    }

    pwmOutConfigTimer(p, timerHardware, mhz, period, value);
    return p;
}

static void pwmWriteBrushed(uint8_t index, uint16_t value)
{
    *motors[index]->ccr = (value - 1000) * motors[index]->period / 1000;
}

#ifndef BRUSHED_MOTORS
static void pwmWriteStandard(uint8_t index, uint16_t value)
{
    *motors[index]->ccr = value;
}

static void pwmWriteOneShot125(uint8_t index, uint16_t value)
{
    *motors[index]->ccr = lrintf((float)(value * ONESHOT125_TIMER_MHZ/8.0f));
}

static void pwmWriteOneShot42(uint8_t index, uint16_t value)
{
    *motors[index]->ccr = lrintf((float)(value * ONESHOT42_TIMER_MHZ/24.0f));
}

static void pwmWriteMultiShot(uint8_t index, uint16_t value)
{
    *motors[index]->ccr = lrintf(((float)(value-1000) * MULTISHOT_20US_MULT) + MULTISHOT_5US_PW);
}
#endif

void pwmWriteMotor(uint8_t index, uint16_t value)
{
    if (motors[index] && index < MAX_MOTORS && pwmMotorsEnabled) {
        motors[index]->pwmWritePtr(index, value);
    }
}

void pwmShutdownPulsesForAllMotors(uint8_t motorCount)
{
    for (int index = 0; index < motorCount; index++) {
        // Set the compare register to 0, which stops the output pulsing if the timer overflows
        *motors[index]->ccr = 0;
    }
}

void pwmDisableMotors(void)
{
    pwmMotorsEnabled = false;
}

void pwmEnableMotors(void)
{
    pwmMotorsEnabled = true;
}

bool isMotorBrushed(uint16_t motorPwmRate)
{
    return (motorPwmRate > 500);
}

bool pwmMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, uint16_t motorPwmRate, uint16_t idlePulse, motorPwmProtocolTypes_e proto, bool enableOutput)
{
    uint32_t timerMhzCounter;
    pwmWriteFuncPtr pwmWritePtr;

    switch (proto) {
#ifdef BRUSHED_MOTORS
    default:
#endif
    case PWM_TYPE_BRUSHED:
        timerMhzCounter = PWM_BRUSHED_TIMER_MHZ;
        pwmWritePtr = pwmWriteBrushed;
        idlePulse = 0;
        break;
#ifndef BRUSHED_MOTORS
    case PWM_TYPE_ONESHOT125:
        timerMhzCounter = ONESHOT125_TIMER_MHZ;
        pwmWritePtr = pwmWriteOneShot125;
        break;

    case PWM_TYPE_ONESHOT42:
        timerMhzCounter = ONESHOT42_TIMER_MHZ;
        pwmWritePtr = pwmWriteOneShot42;
        break;

    case PWM_TYPE_MULTISHOT:
        timerMhzCounter = MULTISHOT_TIMER_MHZ;
        pwmWritePtr = pwmWriteMultiShot;
        break;

    case PWM_TYPE_STANDARD:
    default:
        timerMhzCounter = PWM_TIMER_MHZ;
        pwmWritePtr = pwmWriteStandard;
        break;
#endif
    }

    const uint32_t hz = timerMhzCounter * 1000000;

    pwmOutputPort_t * port = pwmOutConfigMotor(timerHardware, timerMhzCounter, hz / motorPwmRate, idlePulse, enableOutput);

    if (port) {
        motors[motorIndex] = port;
        motors[motorIndex]->pwmWritePtr = pwmWritePtr;
        return true;
    }

    return false;
}

bool pwmServoConfig(const timerHardware_t *timerHardware, uint8_t servoIndex, uint16_t servoPwmRate, uint16_t servoCenterPulse, bool enableOutput)
{
    pwmOutputPort_t * port = pwmOutConfigMotor(timerHardware, PWM_TIMER_MHZ, 1000000 / servoPwmRate, servoCenterPulse, enableOutput);

    if (port) {
        servos[servoIndex] = port;
        return true;
    }

    return false;
}

void pwmWriteServo(uint8_t index, uint16_t value)
{
#ifdef USE_PWM_SERVO_DRIVER

    /*
     *  If PCA9685 is enabled and but not detected, we do not want to write servo
     * output anywhere
     */
    if (feature(FEATURE_PWM_SERVO_DRIVER) && STATE(PWM_DRIVER_AVAILABLE)) {
        pwmDriverSetPulse(index, value);
    } else if (!feature(FEATURE_PWM_SERVO_DRIVER) && servos[index] && index < MAX_SERVOS) {
        *servos[index]->ccr = value;
    }

#else
    if (servos[index] && index < MAX_SERVOS) {
        *servos[index]->ccr = value;
    }
#endif
}

#ifdef BEEPER_PWM
void pwmWriteBeeper(bool onoffBeep)
{
    if (beeperPwm == NULL)
        return;

    if (onoffBeep == true) {
        *beeperPwm->ccr = (1000000 / beeperFrequency) / 2;
    } else {
        *beeperPwm->ccr = 0;
    }
}

void beeperPwmInit(ioTag_t tag, uint16_t frequency)
{
    const timerHardware_t *timer = timerGetByTag(tag, TIM_USE_BEEPER);
    if (timer) {
        beeperPwm = &beeperPwmPort;
        beeperFrequency = frequency;
        IOConfigGPIOAF(IOGetByTag(tag), IOCFG_AF_PP, timer->alternateFunction);
        pwmOutConfigTimer(beeperPwm, timer, PWM_TIMER_MHZ, 1000000 / beeperFrequency, (1000000 / beeperFrequency) / 2);
    }
    else {
        beeperPwm = NULL;
    }
}
#endif

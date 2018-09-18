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

#define MULTISHOT_5US_PW    (MULTISHOT_TIMER_HZ * 5 / 1000000.0f)
#define MULTISHOT_20US_MULT (MULTISHOT_TIMER_HZ * 20 / 1000000.0f / 1000.0f)

typedef void (*pwmWriteFuncPtr)(uint8_t index, uint16_t value);  // function pointer used to write motors

typedef struct {
    TCH_t * tch;
    volatile timCCR_t *ccr;
    uint16_t period;
    pwmWriteFuncPtr pwmWritePtr;
    float pulseOffset;
    float pulseScale;
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

static void pwmOutConfigTimer(pwmOutputPort_t * p, TCH_t * tch, uint32_t hz, uint16_t period, uint16_t value)
{
    p->tch = tch;
    
    timerConfigBase(p->tch, period, hz);
    timerPWMConfigChannel(p->tch, value);

    if (p->tch->timHw->output & TIMER_OUTPUT_ENABLED) {
        timerPWMStart(p->tch);
    }

    timerEnable(p->tch);

    p->period = period;
    p->ccr = timerCCR(p->tch);
    *p->ccr = 0;
}

static pwmOutputPort_t *pwmOutConfigMotor(const timerHardware_t *timHw, uint32_t hz, uint16_t period, uint16_t value, bool enableOutput)
{
    if (allocatedOutputPortCount >= MAX_PWM_OUTPUT_PORTS) {
        DEBUG_TRACE("Attempt to allocate PWM output beyond MAX_PWM_OUTPUT_PORTS");
        return NULL;
    }

    // Attempt to allocate TCH
    TCH_t * tch = timerGetTCH(timHw);
    if (tch == NULL) {
        return NULL;
    }

    pwmOutputPort_t *p = &pwmOutputPorts[allocatedOutputPortCount++];

    const IO_t io = IOGetByTag(timHw->tag);
    IOInit(io, OWNER_MOTOR, RESOURCE_OUTPUT, allocatedOutputPortCount);

    if (enableOutput) {
        IOConfigGPIOAF(io, IOCFG_AF_PP, timHw->alternateFunction);
    }
    else {
        // If PWM outputs are disabled - configure as GPIO and drive low
        IOConfigGPIO(io, IOCFG_OUT_OD);
        IOLo(io);
    }

    pwmOutConfigTimer(p, tch, hz, period, value);
    return p;
}

static void pwmWriteStandard(uint8_t index, uint16_t value)
{
    *motors[index]->ccr = lrintf((value * motors[index]->pulseScale) + motors[index]->pulseOffset);
}

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

static pwmOutputPort_t * motorConfigPwm(const timerHardware_t *timerHardware, float sMin, float sLen, uint32_t motorPwmRateHz, bool enableOutput)
{
    const uint32_t baseClockHz = timerGetBaseClockHW(timerHardware);
    const uint32_t prescaler = ((baseClockHz / motorPwmRateHz) + 0xffff) / 0x10000; /* rounding up */
    const uint32_t timerHz = baseClockHz / prescaler;
    const uint32_t period = timerHz / motorPwmRateHz;

    pwmOutputPort_t * port = pwmOutConfigMotor(timerHardware, timerHz, period, 0, enableOutput);

    if (port) {
        port->pulseScale = ((sLen == 0) ? period : (sLen * timerHz)) / 1000.0f;
        port->pulseOffset = (sMin * timerHz) - (port->pulseScale * 1000);
    }

    return port;
}

bool pwmMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, uint16_t motorPwmRate, motorPwmProtocolTypes_e proto, bool enableOutput)
{
    pwmOutputPort_t * port = NULL;
    pwmWriteFuncPtr pwmWritePtr;

#ifdef BRUSHED_MOTORS
    proto = PWM_TYPE_BRUSHED;   // Override proto
#endif

    switch (proto) {
    case PWM_TYPE_BRUSHED:
        port = motorConfigPwm(timerHardware, 0.0f, 0.0f, motorPwmRate, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;
    case PWM_TYPE_ONESHOT125:
        port = motorConfigPwm(timerHardware, 125e-6f, 125e-6f, motorPwmRate, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;

    case PWM_TYPE_ONESHOT42:
        port = motorConfigPwm(timerHardware, 42e-6f, 42e-6f, motorPwmRate, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;

    case PWM_TYPE_MULTISHOT:
        port = motorConfigPwm(timerHardware, 5e-6f, 20e-6f, motorPwmRate, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;

#ifdef USE_DSHOT
    case PWM_TYPE_DSHOT1200:
    case PWM_TYPE_DSHOT600:
    case PWM_TYPE_DSHOT300:
    case PWM_TYPE_DSHOT150:
        port = NULL;
        break;
#endif

    default:
    case PWM_TYPE_STANDARD:
        port = motorConfigPwm(timerHardware, 1e-3f, 1e-3f, motorPwmRate, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;
    }

    if (port) {
        port->pwmWritePtr = pwmWritePtr;
        motors[motorIndex] = port;
        return true;
    }

    return false;
}

bool pwmServoConfig(const timerHardware_t *timerHardware, uint8_t servoIndex, uint16_t servoPwmRate, uint16_t servoCenterPulse, bool enableOutput)
{
    pwmOutputPort_t * port = pwmOutConfigMotor(timerHardware, PWM_TIMER_HZ, PWM_TIMER_HZ / servoPwmRate, servoCenterPulse, enableOutput);

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
    beeperPwm = NULL;

    const timerHardware_t *timHw = timerGetByTag(tag, TIM_USE_BEEPER);

    if (timHw) {
        // Attempt to allocate TCH
        TCH_t * tch = timerGetTCH(timHw);
        if (tch == NULL) {
            return NULL;
        }

        beeperPwm = &beeperPwmPort;
        beeperFrequency = frequency;
        IOConfigGPIOAF(IOGetByTag(tag), IOCFG_AF_PP, timHw->alternateFunction);
        pwmOutConfigTimer(beeperPwm, tch, PWM_TIMER_HZ, 1000000 / beeperFrequency, (1000000 / beeperFrequency) / 2);
    }
}
#endif

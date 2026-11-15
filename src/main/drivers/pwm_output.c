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

#if !defined(SITL_BUILD) && !defined(RP2350)

#include "common/log.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/dshot/dshot.h"
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/timer.h"

#include "io/motor_srxl2.h"
#include "io/servo_sbus.h"

#define MULTISHOT_5US_PW    (MULTISHOT_TIMER_HZ * 5 / 1000000.0f)
#define MULTISHOT_20US_MULT (MULTISHOT_TIMER_HZ * 20 / 1000000.0f / 1000.0f)

typedef void (*pwmWriteFuncPtr)(uint8_t index, uint16_t value);  // function pointer used to write motors

typedef struct {
    TCH_t * tch;
    bool configured;
    uint16_t value;

    // PWM parameters
    volatile timCCR_t *ccr;         // Shortcut for timer CCR register
    float pulseOffset;
    float pulseScale;
} pwmOutputPort_t;

static pwmOutputPort_t pwmOutputPorts[MAX_PWM_OUTPUTS];

// NULL for a motor without a PWM port: not configured, or driven by DSHOT (dshot/dshot.c)
static pwmOutputPort_t *       motors[MAX_MOTORS];
static motorPwmProtocolTypes_e initMotorProtocol;
static pwmWriteFuncPtr         motorWritePtr = NULL;    // Function to write value to motors

static pwmOutputPort_t *       servos[MAX_SERVOS];
static pwmWriteFuncPtr         servoWritePtr = NULL;    // Function to write value to motors

static pwmOutputPort_t  beeperPwmPort;
static pwmOutputPort_t *beeperPwm;
static uint16_t beeperFrequency = 0;

static uint8_t allocatedOutputPortCount = 0;

static bool pwmMotorsEnabled = true;

static void pwmOutConfigTimer(pwmOutputPort_t * p, TCH_t * tch, uint32_t hz, uint16_t period, uint16_t value)
{
    p->tch = tch;

    timerConfigBase(p->tch, period, hz);
    timerPWMConfigChannel(p->tch, value);
    timerPWMStart(p->tch);

    timerEnable(p->tch);

    p->ccr = timerCCR(p->tch);
    *p->ccr = 0;
}

static pwmOutputPort_t *pwmOutAllocatePort(void)
{
    if (allocatedOutputPortCount >= MAX_PWM_OUTPUTS) {
        LOG_ERROR(PWM, "Attempt to allocate PWM output beyond MAX_PWM_OUTPUT_PORTS");
        return NULL;
    }

    pwmOutputPort_t *p = &pwmOutputPorts[allocatedOutputPortCount++];

    p->tch = NULL;
    p->configured = false;

    return p;
}

static pwmOutputPort_t *pwmOutConfig(const timerHardware_t *timHw, resourceOwner_e owner, uint32_t hz, uint16_t period, uint16_t value, bool enableOutput)
{
    // Attempt to allocate TCH
    TCH_t * tch = timerGetTCH(timHw);
    if (tch == NULL) {
        return NULL;
    }

    // Allocate motor output port
    pwmOutputPort_t *p = pwmOutAllocatePort();
    if (p == NULL) {
        return NULL;
    }

    const IO_t io = IOGetByTag(timHw->tag);
    IOInit(io, owner, RESOURCE_OUTPUT, allocatedOutputPortCount);

    pwmOutConfigTimer(p, tch, hz, period, value);

    if (enableOutput) {
        IOConfigGPIOAF(io, IOCFG_AF_PP, timHw->alternateFunction);
    }
    else {
        // If PWM outputs are disabled - configure as GPIO and drive low
        IOConfigGPIO(io, IOCFG_OUT_OD);
        IOLo(io);
    }

    return p;
}

static void pwmWriteNull(uint8_t index, uint16_t value)
{
    (void)index;
    (void)value;
}

static void pwmWriteStandard(uint8_t index, uint16_t value)
{
    if (motors[index]) {
        *(motors[index]->ccr) = lrintf((value * motors[index]->pulseScale) + motors[index]->pulseOffset);
    }
}

void pwmWriteMotor(uint8_t index, uint16_t value)
{
    if (motorWritePtr && index < MAX_MOTORS && pwmMotorsEnabled) {
        motorWritePtr(index, value);
    }
}

void pwmShutdownPulsesForAllMotors(uint8_t motorCount)
{
    for (int index = 0; index < motorCount; index++) {
        // Set the compare register to 0, which stops the output pulsing if the timer overflows
        if (motors[index]) {
            *(motors[index]->ccr) = 0;
        }
    }
}

void pwmDisableMotors(void)
{
    pwmMotorsEnabled = false;
#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {
        dshotStop();
    }
#endif
}

void pwmEnableMotors(void)
{
    pwmMotorsEnabled = true;
}

void pwmSetMotorDMACircular(bool circular)
{
#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {
        dshotSetDMACircular(circular);
    }
#else
    UNUSED(circular);
#endif
}

bool isMotorBrushed(uint16_t motorPwmRateHz)
{
    return (motorPwmRateHz > 500);
}

static pwmOutputPort_t * motorConfigPwm(const timerHardware_t *timerHardware, float sMin, float sLen, uint32_t motorPwmRateHz, bool enableOutput)
{
    const uint32_t baseClockHz = timerGetBaseClockHW(timerHardware);
    const uint32_t prescaler = ((baseClockHz / motorPwmRateHz) + 0xffff) / 0x10000; /* rounding up */
    const uint32_t timerHz = baseClockHz / prescaler;
    const uint32_t period = timerHz / motorPwmRateHz;

    pwmOutputPort_t * port = pwmOutConfig(timerHardware, OWNER_MOTOR, timerHz, period, 0, enableOutput);

    if (port) {
        port->pulseScale = ((sLen == 0) ? period : (sLen * timerHz)) / 1000.0f;
        port->pulseOffset = (sMin * timerHz) - (port->pulseScale * 1000);
        port->configured = true;
    }

    return port;
}

bool isMotorProtocolDshot(void)
{
    // We look at cached `initMotorProtocol` to make sure we are consistent with the initialized config
    // motorConfig()->motorPwmProtocol may change at run time which will cause uninitialized structures to be used
    return getMotorProtocolProperties(initMotorProtocol)->isDSHOT;
}

bool isMotorProtocolDigital(void)
{
    return isMotorProtocolDshot();
}

void pwmRequestMotorTelemetry(int motorIndex)
{
#ifdef USE_DSHOT
    if (isMotorProtocolDigital()) {
        dshotRequestTelemetry(motorIndex);
    }
#else
    UNUSED(motorIndex);
#endif
}

void pwmCompleteMotorUpdate(void)
{
#ifdef USE_DSHOT
    // This only makes sense for digital motor protocols
    if (isMotorProtocolDigital()) {
        dshotCompleteMotorUpdate(pwmMotorsEnabled);
    }
#endif
}

void pwmMotorPreconfigure(void)
{
    // Keep track of initial motor protocol
    initMotorProtocol = motorConfig()->motorPwmProtocol;

#ifdef BRUSHED_MOTORS
    initMotorProtocol = PWM_TYPE_BRUSHED;   // Override proto
#endif

    // Protocol-specific configuration
    switch (initMotorProtocol) {
        default:
            motorWritePtr = pwmWriteNull;
            break;

        case PWM_TYPE_STANDARD:
        case PWM_TYPE_BRUSHED:
        case PWM_TYPE_ONESHOT125:
        case PWM_TYPE_MULTISHOT:
            motorWritePtr = pwmWriteStandard;
            break;

#ifdef USE_DSHOT
        case PWM_TYPE_DSHOT600:
        case PWM_TYPE_DSHOT300:
        case PWM_TYPE_DSHOT150:
            dshotInit(initMotorProtocol, getEscUpdateFrequency());
            motorWritePtr = dshotWriteMotor;
            break;
#endif

#ifdef USE_MOTOR_SRXL2
        case PWM_TYPE_SRXL2:
            /* Nothing to fall back on if this fails: the pin is a UART pin, not a
             * timer output, so there is no PWM to degrade to. Leaving
             * motorWritePtr null keeps the motor unwritten, which is the honest
             * outcome of a port that was never assigned. */
            if (srxl2MotorInitialize()) {
                srxl2MotorSetReverseChannel(motorConfig()->srxl2ReverseChannel);
                srxl2MotorSetTelemetryRate(motorConfig()->srxl2TelemetryRate);
                motorWritePtr = srxl2MotorUpdate;
            }
            break;
#endif
    }
}

/**
 * This function return the PWM frequency based on ESC protocol. We allow customer rates only for Brushed motors
 */
uint32_t getEscUpdateFrequency(void) {
    switch (initMotorProtocol) {
        case PWM_TYPE_BRUSHED:
            return motorConfig()->motorPwmRate;

        case PWM_TYPE_STANDARD:
            return 400;

        case PWM_TYPE_MULTISHOT:
            return 2000;

        case PWM_TYPE_DSHOT150:
            return 4000;

        case PWM_TYPE_DSHOT300:
            return 8000;

        case PWM_TYPE_DSHOT600:
            return 16000;

        case PWM_TYPE_ONESHOT125:
        default:
            return 1000;

    }
}

bool pwmMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, bool enableOutput)
{
    switch (initMotorProtocol) {
    case PWM_TYPE_BRUSHED:
        motors[motorIndex] = motorConfigPwm(timerHardware, 0.0f, 0.0f, getEscUpdateFrequency(), enableOutput);
        break;

    case PWM_TYPE_ONESHOT125:
        motors[motorIndex] = motorConfigPwm(timerHardware, 125e-6f, 125e-6f, getEscUpdateFrequency(), enableOutput);
        break;

    case PWM_TYPE_MULTISHOT:
        motors[motorIndex] = motorConfigPwm(timerHardware, 5e-6f, 20e-6f, getEscUpdateFrequency(), enableOutput);
        break;

#ifdef USE_DSHOT
    case PWM_TYPE_DSHOT600:
    case PWM_TYPE_DSHOT300:
    case PWM_TYPE_DSHOT150:
        return dshotMotorConfig(timerHardware, motorIndex, enableOutput);
#endif

    case PWM_TYPE_STANDARD:
        motors[motorIndex] = motorConfigPwm(timerHardware, 1e-3f, 1e-3f, getEscUpdateFrequency(), enableOutput);
        break;

    default:
        motors[motorIndex] = NULL;
        break;
    }

    return (motors[motorIndex] != NULL);
}

// After every other timer and DMA user has had its pick (fc_init)
bool pwmMotorPostInit(void)
{
#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {
        return dshotMotorPostInit();
    }
#endif
    return true;
}

// Helper function for ESC passthrough
ioTag_t pwmGetMotorPinTag(int motorIndex)
{
#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {
        return dshotGetMotorPinTag(motorIndex);
    }
#endif
    if (motors[motorIndex]) {
        return motors[motorIndex]->tch->timHw->tag;
    }
    else {
        return IOTAG_NONE;
    }
}

// Helper function for ESC passthrough: hand the pin back to its timer after the 4-way
// interface drove it as plain GPIO. The alternate function has to be given explicitly:
// on HAL targets a bare IOConfigGPIO(IOCFG_AF_PP) programs AF0 and the timer loses the pin.
void pwmRestoreMotorPin(int motorIndex)
{
#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {
        dshotRestoreMotorPin(motorIndex);
        return;
    }
#endif
    if (motors[motorIndex]) {
        const timerHardware_t *timHw = motors[motorIndex]->tch->timHw;
        IOConfigGPIOAF(IOGetByTag(timHw->tag), IOCFG_AF_PP, timHw->alternateFunction);
    }
}

static void pwmServoWriteStandard(uint8_t index, uint16_t value)
{
    if (index < MAX_SERVOS && servos[index]) {
        *servos[index]->ccr = value;
    }
}

#ifdef USE_SERVO_SBUS
static void sbusPwmWriteStandard(uint8_t index, uint16_t value)
{
    pwmServoWriteStandard(index, value);
    sbusServoUpdate(index, value);
}
#endif

void pwmServoPreconfigure(void)
{
    // Protocol-specific configuration
    switch (servoConfig()->servo_protocol) {
        default:
        case SERVO_TYPE_PWM:
            servoWritePtr = pwmServoWriteStandard;
            break;

#ifdef USE_SERVO_SBUS
        case SERVO_TYPE_SBUS:
            sbusServoInitialize();
            servoWritePtr = sbusServoUpdate;
            break;

        case SERVO_TYPE_SBUS_PWM:
            sbusServoInitialize();
            servoWritePtr = sbusPwmWriteStandard;
            break;
#endif
    }
}

bool pwmServoConfig(const timerHardware_t *timerHardware, uint8_t servoIndex, uint16_t servoPwmRate, uint16_t servoCenterPulse, bool enableOutput)
{
    pwmOutputPort_t * port = pwmOutConfig(timerHardware, OWNER_SERVO, PWM_TIMER_HZ, PWM_TIMER_HZ / servoPwmRate, servoCenterPulse, enableOutput);

    if (port) {
        servos[servoIndex] = port;
        return true;
    }

    return false;
}

void pwmWriteServo(uint8_t index, uint16_t value)
{
    if (servoWritePtr && index < MAX_SERVOS) {
        servoWritePtr(index, value);
    }
}

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

bool beeperPwmInit(ioTag_t tag, uint16_t frequency)
{
    beeperPwm = NULL;

    const timerHardware_t *timHw = timerGetByTag(tag, TIM_USE_BEEPER);

    if (timHw) {
        // Attempt to allocate TCH
        TCH_t * tch = timerGetTCH(timHw);
        if (tch == NULL) {
            return false;
        }

        beeperPwm = &beeperPwmPort;
        beeperFrequency = frequency;
        IOConfigGPIOAF(IOGetByTag(tag), IOCFG_AF_PP, timHw->alternateFunction);
        pwmOutConfigTimer(beeperPwm, tch, PWM_TIMER_HZ, 1000000 / beeperFrequency, (1000000 / beeperFrequency) / 2);
        return true;
    }

    return false;
}

#endif

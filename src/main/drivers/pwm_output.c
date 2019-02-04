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
#include <string.h>

#include "platform.h"
#include "build/debug.h"

#include "common/maths.h"

#include "drivers/io.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/io_pca9685.h"

#include "io/pwmdriver_i2c.h"

#include "config/feature.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#define MULTISHOT_5US_PW    (MULTISHOT_TIMER_HZ * 5 / 1000000.0f)
#define MULTISHOT_20US_MULT (MULTISHOT_TIMER_HZ * 20 / 1000000.0f / 1000.0f)

#ifdef USE_DSHOT
#define MOTOR_DSHOT1200_HZ    24000000
#define MOTOR_DSHOT600_HZ     12000000
#define MOTOR_DSHOT300_HZ     6000000
#define MOTOR_DSHOT150_HZ     3000000


#define DSHOT_MOTOR_BIT_0       7
#define DSHOT_MOTOR_BIT_1       14
#define DSHOT_MOTOR_BITLENGTH   19

#define DSHOT_DMA_BUFFER_SIZE   18 /* resolution + frame reset (2us) */
#endif

typedef void (*pwmWriteFuncPtr)(uint8_t index, uint16_t value);  // function pointer used to write motors

typedef struct {
    TCH_t * tch;
    pwmWriteFuncPtr pwmWritePtr;
    bool configured;
    uint16_t value;                 // Used to keep track of last motor value

    // PWM parameters
    volatile timCCR_t *ccr;         // Shortcut for timer CCR register
    float pulseOffset;
    float pulseScale;

#ifdef USE_DSHOT
    // DSHOT parameters
    uint32_t dmaBuffer[DSHOT_DMA_BUFFER_SIZE] __attribute__ ((aligned (4)));
#endif
} pwmOutputPort_t;

static pwmOutputPort_t pwmOutputPorts[MAX_PWM_OUTPUT_PORTS];

static pwmOutputPort_t *motors[MAX_PWM_MOTORS];
static pwmOutputPort_t *servos[MAX_PWM_SERVOS];

#ifdef USE_DSHOT

static bool isProtocolDshot = false;
static timeUs_t dshotMotorUpdateIntervalUs = 0;
static timeUs_t dshotMotorLastUpdateUs;
#endif

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
    timerPWMStart(p->tch);

    timerEnable(p->tch);

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

    pwmOutputPort_t * port = pwmOutConfigMotor(timerHardware, timerHz, period, 0, enableOutput);

    if (port) {
        port->pulseScale = ((sLen == 0) ? period : (sLen * timerHz)) / 1000.0f;
        port->pulseOffset = (sMin * timerHz) - (port->pulseScale * 1000);
        port->configured = true;
    }

    return port;
}

#ifdef USE_DSHOT
uint32_t getDshotHz(motorPwmProtocolTypes_e pwmProtocolType)
{
    switch (pwmProtocolType) {
        case(PWM_TYPE_DSHOT1200):
            return MOTOR_DSHOT1200_HZ;
        case(PWM_TYPE_DSHOT600):
            return MOTOR_DSHOT600_HZ;
        case(PWM_TYPE_DSHOT300):
            return MOTOR_DSHOT300_HZ;
        default:
        case(PWM_TYPE_DSHOT150):
            return MOTOR_DSHOT150_HZ;
    }
}

static pwmOutputPort_t * motorConfigDshot(const timerHardware_t * timerHardware, motorPwmProtocolTypes_e proto, uint16_t motorPwmRateHz, bool enableOutput)
{
    // Try allocating new port
    pwmOutputPort_t * port = pwmOutConfigMotor(timerHardware, getDshotHz(proto), DSHOT_MOTOR_BITLENGTH, 0, enableOutput);

    if (!port) {
        return NULL;
    }

    // Keep track of motor update interval
    const timeUs_t motorIntervalUs = 1000000 / motorPwmRateHz;
    dshotMotorUpdateIntervalUs = MAX(dshotMotorUpdateIntervalUs, motorIntervalUs);

    // Configure timer DMA
    if (timerPWMConfigChannelDMA(port->tch, port->dmaBuffer, DSHOT_DMA_BUFFER_SIZE)) {
        // Only mark as DSHOT channel if DMA was set successfully
        memset(port->dmaBuffer, 0, sizeof(port->dmaBuffer));
        port->configured = true;
    }

    return port;
}

static void pwmWriteDshot(uint8_t index, uint16_t value)
{
    // DMA operation might still be running. Cache value for future use
    motors[index]->value = value;
}

static void loadDmaBufferDshot(uint32_t * dmaBuffer, uint16_t packet)
{
    for (int i = 0; i < 16; i++) {
        dmaBuffer[i] = (packet & 0x8000) ? DSHOT_MOTOR_BIT_1 : DSHOT_MOTOR_BIT_0;  // MSB first
        packet <<= 1;
    }
}

static uint16_t prepareDshotPacket(const uint16_t value, bool requestTelemetry)
{
    uint16_t packet = (value << 1) | (requestTelemetry ? 1 : 0);

    // compute checksum
    int csum = 0;
    int csum_data = packet;
    for (int i = 0; i < 3; i++) {
        csum ^=  csum_data;   // xor data by nibbles
        csum_data >>= 4;
    }
    csum &= 0xf;

    // append checksum
    packet = (packet << 4) | csum;

    return packet;
}

void pwmCompleteDshotUpdate(uint8_t motorCount)
{
    // Get latest REAL time
    timeUs_t currentTimeUs = micros();

    // Enforce motor update rate
    if (!isProtocolDshot || (dshotMotorUpdateIntervalUs == 0) || ((currentTimeUs - dshotMotorLastUpdateUs) <= dshotMotorUpdateIntervalUs)) {
        return;
    }

    dshotMotorLastUpdateUs = currentTimeUs;

    // Generate DMA buffers
    for (int index = 0; index < motorCount; index++) {
        if (motors[index] && motors[index]->configured) {
            // TODO: ESC telemetry
            uint16_t packet = prepareDshotPacket(motors[index]->value, false);

            loadDmaBufferDshot(motors[index]->dmaBuffer, packet);
            timerPWMPrepareDMA(motors[index]->tch, DSHOT_DMA_BUFFER_SIZE);
        }
    }

    // Start DMA on all timers
    for (int index = 0; index < motorCount; index++) {
        if (motors[index] && motors[index]->configured) {
            timerPWMStartDMA(motors[index]->tch);
        }
    }
}

bool isMotorProtocolDshot(void)
{
    return isProtocolDshot;
}
#endif

bool pwmMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, uint16_t motorPwmRateHz, motorPwmProtocolTypes_e proto, bool enableOutput)
{
    pwmOutputPort_t * port = NULL;
    pwmWriteFuncPtr pwmWritePtr;

#ifdef BRUSHED_MOTORS
    proto = PWM_TYPE_BRUSHED;   // Override proto
#endif

    switch (proto) {
    case PWM_TYPE_BRUSHED:
        port = motorConfigPwm(timerHardware, 0.0f, 0.0f, motorPwmRateHz, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;
    case PWM_TYPE_ONESHOT125:
        port = motorConfigPwm(timerHardware, 125e-6f, 125e-6f, motorPwmRateHz, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;

    case PWM_TYPE_ONESHOT42:
        port = motorConfigPwm(timerHardware, 42e-6f, 42e-6f, motorPwmRateHz, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;

    case PWM_TYPE_MULTISHOT:
        port = motorConfigPwm(timerHardware, 5e-6f, 20e-6f, motorPwmRateHz, enableOutput);
        pwmWritePtr = pwmWriteStandard;
        break;

#ifdef USE_DSHOT
    case PWM_TYPE_DSHOT1200:
    case PWM_TYPE_DSHOT600:
    case PWM_TYPE_DSHOT300:
    case PWM_TYPE_DSHOT150:
        port = motorConfigDshot(timerHardware, proto, motorPwmRateHz, enableOutput);
        if (port) {
            isProtocolDshot = true;
            pwmWritePtr = pwmWriteDshot;
        }
        break;
#endif

    default:
    case PWM_TYPE_STANDARD:
        port = motorConfigPwm(timerHardware, 1e-3f, 1e-3f, motorPwmRateHz, enableOutput);
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

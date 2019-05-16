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
#include <string.h>

#include "platform.h"

#include "build/debug.h"
#include "common/log.h"
#include "common/memory.h"

#include "config/feature.h"

#include "fc/config.h"

#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/timer.h"
#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"
//#include "drivers/rx_pwm.h"

#include "sensors/rangefinder.h"

#include "io/serial.h"

enum {
    MAP_TO_NONE,
    MAP_TO_MOTOR_OUTPUT,
    MAP_TO_SERVO_OUTPUT,
};

typedef struct {
    int maxTimMotorCount;
    int maxTimServoCount;
    const timerHardware_t * timMotors[MAX_PWM_OUTPUT_PORTS];
    const timerHardware_t * timServos[MAX_PWM_OUTPUT_PORTS];
} timMotorServoHardware_t;

static pwmInitError_e pwmInitError = PWM_INIT_ERROR_NONE;

static const char * pwmInitErrorMsg[] = {
    /* PWM_INIT_ERROR_NONE */                     "No error",
    /* PWM_INIT_ERROR_TOO_MANY_MOTORS */          "Mixer defines too many motors",
    /* PWM_INIT_ERROR_TOO_MANY_SERVOS */          "Mixer defines too many servos",
    /* PWM_INIT_ERROR_NOT_ENOUGH_MOTOR_OUTPUTS */ "Not enough motor outputs/timers",
    /* PWM_INIT_ERROR_NOT_ENOUGH_SERVO_OUTPUTS */ "Not enough servo outputs/timers",
    /* PWM_INIT_ERROR_TIMER_INIT_FAILED */        "Output timer init failed"
};

static const motorProtocolProperties_t motorProtocolProperties[] = {
    [PWM_TYPE_STANDARD]     = { .usesHwTimer = true,    .isDSHOT = false,   .isSerialShot = false },
    [PWM_TYPE_ONESHOT125]   = { .usesHwTimer = true,    .isDSHOT = false,   .isSerialShot = false },
    [PWM_TYPE_ONESHOT42]    = { .usesHwTimer = true,    .isDSHOT = false,   .isSerialShot = false },
    [PWM_TYPE_MULTISHOT]    = { .usesHwTimer = true,    .isDSHOT = false,   .isSerialShot = false },
    [PWM_TYPE_BRUSHED]      = { .usesHwTimer = true,    .isDSHOT = false,   .isSerialShot = false },
    [PWM_TYPE_DSHOT150]     = { .usesHwTimer = true,    .isDSHOT = true,    .isSerialShot = false },
    [PWM_TYPE_DSHOT300]     = { .usesHwTimer = true,    .isDSHOT = true,    .isSerialShot = false },
    [PWM_TYPE_DSHOT600]     = { .usesHwTimer = true,    .isDSHOT = true,    .isSerialShot = false },
    [PWM_TYPE_DSHOT1200]    = { .usesHwTimer = true,    .isDSHOT = true,    .isSerialShot = false },
    [PWM_TYPE_SERIALSHOT]   = { .usesHwTimer = false,   .isDSHOT = false,   .isSerialShot = true  },
};

pwmInitError_e getPwmInitError(void)
{
    return pwmInitError;
}

const char * getPwmInitErrorMessage(void)
{
    return pwmInitErrorMsg[pwmInitError];
}

const motorProtocolProperties_t * getMotorProtocolProperties(motorPwmProtocolTypes_e proto)
{
    return &motorProtocolProperties[proto];
}

static bool checkPwmTimerConflicts(const timerHardware_t *timHw)
{
    serialPortPins_t uartPins;

#if defined(USE_UART2)
    uartGetPortPins(UARTDEV_2, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART2) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART3)
    uartGetPortPins(UARTDEV_3, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART3) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART4)
    uartGetPortPins(UARTDEV_4, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART4) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART5)
    uartGetPortPins(UARTDEV_5, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART5) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART6)
    uartGetPortPins(UARTDEV_6, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART6) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART7)
    uartGetPortPins(UARTDEV_7, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART7) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART8)
    uartGetPortPins(UARTDEV_8, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART8) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_SOFTSERIAL1)
    if (feature(FEATURE_SOFTSERIAL)) {
        const timerHardware_t *ssrx = timerGetByTag(IO_TAG(SOFTSERIAL_1_RX_PIN), TIM_USE_ANY);
        const timerHardware_t *sstx = timerGetByTag(IO_TAG(SOFTSERIAL_1_TX_PIN), TIM_USE_ANY);
        if ((ssrx != NULL && ssrx->tim == timHw->tim) || (sstx != NULL && sstx->tim == timHw->tim)) {
            return true;
        }
    }
#endif

#if defined(USE_SOFTSERIAL2)
    if (feature(FEATURE_SOFTSERIAL)) {
        const timerHardware_t *ssrx = timerGetByTag(IO_TAG(SOFTSERIAL_2_RX_PIN), TIM_USE_ANY);
        const timerHardware_t *sstx = timerGetByTag(IO_TAG(SOFTSERIAL_2_TX_PIN), TIM_USE_ANY);
        if ((ssrx != NULL && ssrx->tim == timHw->tim) || (sstx != NULL && sstx->tim == timHw->tim)) {
            return true;
        }
    }
#endif

#if defined(USE_LED_STRIP)
    if (feature(FEATURE_LED_STRIP)) {
        const timerHardware_t * ledTimHw = timerGetByTag(IO_TAG(WS2811_PIN), TIM_USE_ANY);
        if (ledTimHw != NULL && timHw->tim == ledTimHw->tim) {
            return true;
        }
    }
#endif

#if defined(USE_ADC)
#if defined(ADC_CHANNEL_1_PIN)
    if (timHw->tag == IO_TAG(ADC_CHANNEL_1_PIN)) {
        return true;
    }
#endif
#if defined(ADC_CHANNEL_2_PIN)
    if (timHw->tag == IO_TAG(ADC_CHANNEL_2_PIN)) {
        return true;
    }
#endif
#if defined(ADC_CHANNEL_3_PIN)
    if (timHw->tag == IO_TAG(ADC_CHANNEL_3_PIN)) {
        return true;
    }
#endif
#if defined(ADC_CHANNEL_4_PIN)
    if (timHw->tag == IO_TAG(ADC_CHANNEL_4_PIN)) {
        return true;
    }
#endif
#endif


#ifdef USE_RANGEFINDER_HCSR04
    // HC-SR04 has a dedicated connection to FC and require two pins
    if (rangefinderConfig()->rangefinder_hardware == RANGEFINDER_HCSR04) {
        const rangefinderHardwarePins_t *rangefinderHardwarePins = rangefinderGetHardwarePins();
        if (rangefinderHardwarePins && (timHw->tag == rangefinderHardwarePins->triggerTag || timHw->tag == rangefinderHardwarePins->echoTag)) {
            return true;
        }
    }
#endif

    return false;
}

void pwmBuildTimerOutputList(timMotorServoHardware_t * timOutputs, bool isMixerUsingServos)
{
    timOutputs->maxTimMotorCount = 0;
    timOutputs->maxTimServoCount = 0;

    for (int idx = 0; idx < timerHardwareCount; idx++) {
        const timerHardware_t *timHw = &timerHardware[idx];
        int type = MAP_TO_NONE;

        // Check for known conflicts (i.e. UART, LEDSTRIP, Rangefinder and ADC)
        if (checkPwmTimerConflicts(timHw)) {
            LOG_W(PWM, "Timer output %d skipped", idx);
            continue;
        }

        // Determine if timer belongs to motor/servo
        if (mixerConfig()->platformType == PLATFORM_MULTIROTOR || mixerConfig()->platformType == PLATFORM_TRICOPTER) {
            // Multicopter
            // We enable mapping to servos if mixer is actually using them
            if (isMixerUsingServos && timHw->usageFlags & TIM_USE_MC_SERVO) {
                type = MAP_TO_SERVO_OUTPUT;
            }
            else if (timHw->usageFlags & TIM_USE_MC_MOTOR) {
                type = MAP_TO_MOTOR_OUTPUT;
            }
        } else {
            // Fixed wing or HELI (one/two motors and a lot of servos
            if (timHw->usageFlags & TIM_USE_FW_SERVO) {
                type = MAP_TO_SERVO_OUTPUT;
            }
            else if (timHw->usageFlags & TIM_USE_FW_MOTOR) {
                type = MAP_TO_MOTOR_OUTPUT;
            }
        }

        switch(type) {
            case MAP_TO_MOTOR_OUTPUT:
                timOutputs->timMotors[timOutputs->maxTimMotorCount++] = timHw;
                break;
            case MAP_TO_SERVO_OUTPUT:
                timOutputs->timServos[timOutputs->maxTimServoCount++] = timHw;
                break;
            default:
                break;
        }
    }
}

static bool motorsUseHardwareTimers(void)
{
    return getMotorProtocolProperties(motorConfig()->motorPwmProtocol)->usesHwTimer;
}

static bool servosUseHardwareTimers(void)
{
    return !feature(FEATURE_PWM_SERVO_DRIVER);
}

static void pwmInitMotors(timMotorServoHardware_t * timOutputs)
{
    const int motorCount = getMotorCount();

    // Check if too many motors
    if (motorCount > MAX_MOTORS) {
        pwmInitError = PWM_INIT_ERROR_TOO_MANY_MOTORS;
        LOG_E(PWM, "Too many motors. Mixer requested %d, max %d", motorCount, MAX_MOTORS);
        return;
    }

    // Do the pre-configuration. For motors w/o hardware timers this should be sufficient
    pwmMotorPreconfigure();

    // Now if we need to configure individual motor outputs - do that
    if (!motorsUseHardwareTimers()) {
        LOG_I(PWM, "Skipped timer init for motors");
        return;
    }

    // If mixer requests more motors than we have timer outputs - throw an error
    if (motorCount > timOutputs->maxTimMotorCount) {
        pwmInitError = PWM_INIT_ERROR_NOT_ENOUGH_MOTOR_OUTPUTS;
        LOG_E(PWM, "Not enough motor outputs. Mixer requested %d, outputs %d", motorCount, timOutputs->maxTimMotorCount);
        return;
    }

    // Finally initialize individual motor outputs
    for (int idx = 0; idx < motorCount; idx++) {
        const timerHardware_t *timHw = timOutputs->timMotors[idx];
        if (!pwmMotorConfig(timHw, idx, feature(FEATURE_PWM_OUTPUT_ENABLE))) {
            pwmInitError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
            LOG_E(PWM, "Timer allocation failed for motor %d", idx);
            return;
        }
    }
}

static void pwmInitServos(timMotorServoHardware_t * timOutputs)
{
    const int servoCount = getServoCount();

    if (!isMixerUsingServos()) {
        LOG_I(PWM, "Mixer does not use servos");
        return;
    }

    // Check if too many servos
    if (servoCount > MAX_SERVOS) {
        pwmInitError = PWM_INIT_ERROR_TOO_MANY_SERVOS;
        LOG_E(PWM, "Too many servos. Mixer requested %d, max %d", servoCount, MAX_SERVOS);
        return;
    }

    // Do the pre-configuration. This should configure non-timer PWM drivers
    pwmServoPreconfigure();

    // Check if we need to init timer output for servos
    if (!servosUseHardwareTimers()) {
        // External PWM servo driver
        LOG_I(PWM, "Skipped timer init for servos - using external servo driver");
        return;
    }

    // If mixer requests more servos than we have timer outputs - throw an error
    if (servoCount > timOutputs->maxTimServoCount) {
        pwmInitError = PWM_INIT_ERROR_NOT_ENOUGH_SERVO_OUTPUTS;
        LOG_E(PWM, "Too many servos. Mixer requested %d, timer outputs %d", servoCount, timOutputs->maxTimServoCount);
        return;
    }

    // Configure individual servo outputs
    for (int idx = 0; idx < servoCount; idx++) {
        const timerHardware_t *timHw = timOutputs->timServos[idx];

        if (!pwmServoConfig(timHw, idx, servoConfig()->servoPwmRate, servoConfig()->servoCenterPulse, feature(FEATURE_PWM_OUTPUT_ENABLE))) {
            pwmInitError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
            LOG_E(PWM, "Timer allocation failed for servo %d", idx);
            return;
        }
    }
}


bool pwmMotorAndServoInit(void)
{
    timMotorServoHardware_t timOutputs;

    // Build temporary timer mappings for motor and servo
    pwmBuildTimerOutputList(&timOutputs, isMixerUsingServos());

    // At this point we have built tables of timers suitable for motor and servo mappings
    // Now we can actually initialize them according to motor/servo count from mixer
    pwmInitMotors(&timOutputs);
    pwmInitServos(&timOutputs);

    return (pwmInitError == PWM_INIT_ERROR_NONE);
}

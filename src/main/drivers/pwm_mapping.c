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

#if !defined(SITL_BUILD)

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

#include "sensors/rangefinder.h"

#include "io/serial.h"
#include "io/servo_sbus.h"

enum {
    MAP_TO_NONE,
    MAP_TO_MOTOR_OUTPUT,
    MAP_TO_SERVO_OUTPUT,
    MAP_TO_LED_OUTPUT
};

typedef struct {
    int maxTimMotorCount;
    int maxTimServoCount;
    const timerHardware_t * timMotors[MAX_PWM_OUTPUTS];
    const timerHardware_t * timServos[MAX_PWM_OUTPUTS];
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
    [PWM_TYPE_STANDARD]     = { .usesHwTimer = true,    .isDSHOT = false },
    [PWM_TYPE_ONESHOT125]   = { .usesHwTimer = true,    .isDSHOT = false },
    [PWM_TYPE_MULTISHOT]    = { .usesHwTimer = true,    .isDSHOT = false },
    [PWM_TYPE_BRUSHED]      = { .usesHwTimer = true,    .isDSHOT = false },
    [PWM_TYPE_DSHOT150]     = { .usesHwTimer = true,    .isDSHOT = true },
    [PWM_TYPE_DSHOT300]     = { .usesHwTimer = true,    .isDSHOT = true },
    [PWM_TYPE_DSHOT600]     = { .usesHwTimer = true,    .isDSHOT = true },
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
    UNUSED(uartPins);

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

#if defined(USE_UART9)
    uartGetPortPins(UARTDEV_9, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART9) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
        return true;
    }
#endif

#if defined(USE_UART10)
    uartGetPortPins(UARTDEV_10, &uartPins);
    if (doesConfigurationUsePort(SERIAL_PORT_USART10) && (timHw->tag == uartPins.txPin || timHw->tag == uartPins.rxPin)) {
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
        for (int i = 0; i < timerHardwareCount; i++) {
            if (timHw->tim == timerHardware[i].tim && timerHardware[i].usageFlags & TIM_USE_LED) {
				return true;
            }
        }

        //const timerHardware_t * ledTimHw = timerGetByTag(IO_TAG(WS2811_PIN), TIM_USE_ANY);
        //if (ledTimHw != NULL && timHw->tim == ledTimHw->tim) {
        //    return true;
        //}
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
#if defined(ADC_CHANNEL_5_PIN)
    if (timHw->tag == IO_TAG(ADC_CHANNEL_5_PIN)) {
        return true;
    }
#endif
#if defined(ADC_CHANNEL_6_PIN)
    if (timHw->tag == IO_TAG(ADC_CHANNEL_6_PIN)) {
        return true;
    }
#endif
#endif

    return false;
}

static void timerHardwareOverride(timerHardware_t * timer) {
    switch (timerOverrides(timer2id(timer->tim))->outputMode) {
        case OUTPUT_MODE_MOTORS:
            timer->usageFlags &= ~(TIM_USE_SERVO|TIM_USE_LED);
            timer->usageFlags |= TIM_USE_MOTOR;
            break;
        case OUTPUT_MODE_SERVOS:
            timer->usageFlags &= ~(TIM_USE_MOTOR|TIM_USE_LED);
            timer->usageFlags |= TIM_USE_SERVO;
            break;
        case OUTPUT_MODE_LED:
            timer->usageFlags &= ~(TIM_USE_MOTOR|TIM_USE_SERVO);
            timer->usageFlags |= TIM_USE_LED;
            break;
    }
}

bool pwmHasMotorOnTimer(timMotorServoHardware_t * timOutputs, HAL_Timer_t *tim)
{
    for (int i = 0; i < timOutputs->maxTimMotorCount; ++i) {
        if (timOutputs->timMotors[i]->tim == tim) {
            return true;
        }
    }

    return false;
}

bool pwmHasServoOnTimer(timMotorServoHardware_t * timOutputs, HAL_Timer_t *tim)
{
    for (int i = 0; i < timOutputs->maxTimServoCount; ++i) {
        if (timOutputs->timServos[i]->tim == tim) {
            return true;
        }
    }

    return false;
}

uint8_t pwmClaimTimer(HAL_Timer_t *tim, uint32_t usageFlags) {
    uint8_t changed = 0;
    for (int idx = 0; idx < timerHardwareCount; idx++) {
        timerHardware_t *timHw = &timerHardware[idx];
        if (timHw->tim == tim && timHw->usageFlags != usageFlags) {
            timHw->usageFlags = usageFlags;
            changed++;
        }
    }

    return changed;
}

void pwmEnsureEnoughtMotors(uint8_t motorCount)
{
    uint8_t motorOnlyOutputs = 0;

    for (int idx = 0; idx < timerHardwareCount; idx++) {
        timerHardware_t *timHw = &timerHardware[idx];

        timerHardwareOverride(timHw);

        if (checkPwmTimerConflicts(timHw)) {
            continue;
        }

        if (TIM_IS_MOTOR_ONLY(timHw->usageFlags)) {
            motorOnlyOutputs++;
            motorOnlyOutputs += pwmClaimTimer(timHw->tim, timHw->usageFlags);
        }
    }

    for (int idx = 0; idx < timerHardwareCount; idx++) {
        timerHardware_t *timHw = &timerHardware[idx];

        if (checkPwmTimerConflicts(timHw)) {
            continue;
        }

        if (TIM_IS_MOTOR(timHw->usageFlags) && !TIM_IS_MOTOR_ONLY(timHw->usageFlags)) {
            if (motorOnlyOutputs < motorCount) {
                timHw->usageFlags &= ~TIM_USE_SERVO;
                timHw->usageFlags |= TIM_USE_MOTOR;
                motorOnlyOutputs++;
                motorOnlyOutputs += pwmClaimTimer(timHw->tim, timHw->usageFlags);
            } else {
                timHw->usageFlags &= ~TIM_USE_MOTOR;
                pwmClaimTimer(timHw->tim, timHw->usageFlags);
            }
        }
    }
}

void pwmBuildTimerOutputList(timMotorServoHardware_t * timOutputs, bool isMixerUsingServos)
{
    UNUSED(isMixerUsingServos);
    timOutputs->maxTimMotorCount = 0;
    timOutputs->maxTimServoCount = 0;

    uint8_t motorCount = getMotorCount();
    uint8_t motorIdx = 0;

    pwmEnsureEnoughtMotors(motorCount);

    for (int idx = 0; idx < timerHardwareCount; idx++) {
        timerHardware_t *timHw = &timerHardware[idx];

        int type = MAP_TO_NONE;

        // Check for known conflicts (i.e. UART, LEDSTRIP, Rangefinder and ADC)
        if (checkPwmTimerConflicts(timHw)) {
            LOG_WARNING(PWM, "Timer output %d skipped", idx);
            continue;
        }

        // Make sure first motorCount motor outputs get assigned to motor
        if (TIM_IS_MOTOR(timHw->usageFlags) && (motorIdx < motorCount)) {
            timHw->usageFlags &= ~TIM_USE_SERVO;
            pwmClaimTimer(timHw->tim, timHw->usageFlags);
            motorIdx += 1;
        }

        if (TIM_IS_SERVO(timHw->usageFlags) && !pwmHasMotorOnTimer(timOutputs, timHw->tim)) {
            type = MAP_TO_SERVO_OUTPUT;
        } else if (TIM_IS_MOTOR(timHw->usageFlags) && !pwmHasServoOnTimer(timOutputs, timHw->tim)) {
            type = MAP_TO_MOTOR_OUTPUT;
        } else if (TIM_IS_LED(timHw->usageFlags) && !pwmHasMotorOnTimer(timOutputs, timHw->tim) && !pwmHasServoOnTimer(timOutputs, timHw->tim)) {
            type = MAP_TO_LED_OUTPUT;
        }

        switch(type) {
            case MAP_TO_MOTOR_OUTPUT:
                timHw->usageFlags &= TIM_USE_MOTOR;
                timOutputs->timMotors[timOutputs->maxTimMotorCount++] = timHw;
                pwmClaimTimer(timHw->tim, timHw->usageFlags);
                break;
            case MAP_TO_SERVO_OUTPUT:
                timHw->usageFlags &= TIM_USE_SERVO;
                timOutputs->timServos[timOutputs->maxTimServoCount++] = timHw;
                pwmClaimTimer(timHw->tim, timHw->usageFlags);
                break;
            case MAP_TO_LED_OUTPUT:
                timHw->usageFlags &= TIM_USE_LED;
                pwmClaimTimer(timHw->tim, timHw->usageFlags);
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
    return servoConfig()->servo_protocol == SERVO_TYPE_PWM ||
        servoConfig()->servo_protocol == SERVO_TYPE_SBUS_PWM;
}

static void pwmInitMotors(timMotorServoHardware_t * timOutputs)
{
    const int motorCount = getMotorCount();

    // Check if too many motors
    if (motorCount > MAX_MOTORS) {
        pwmInitError = PWM_INIT_ERROR_TOO_MANY_MOTORS;
        LOG_ERROR(PWM, "Too many motors. Mixer requested %d, max %d", motorCount, MAX_MOTORS);
        return;
    }

    // Do the pre-configuration. For motors w/o hardware timers this should be sufficient
    pwmMotorPreconfigure();

    // Now if we need to configure individual motor outputs - do that
    if (!motorsUseHardwareTimers()) {
        LOG_INFO(PWM, "Skipped timer init for motors");
        return;
    }

    // If mixer requests more motors than we have timer outputs - throw an error
    if (motorCount > timOutputs->maxTimMotorCount) {
        pwmInitError = PWM_INIT_ERROR_NOT_ENOUGH_MOTOR_OUTPUTS;
        LOG_ERROR(PWM, "Not enough motor outputs. Mixer requested %d, outputs %d", motorCount, timOutputs->maxTimMotorCount);
        return;
    }

    // Finally initialize individual motor outputs
    for (int idx = 0; idx < motorCount; idx++) {
        const timerHardware_t *timHw = timOutputs->timMotors[idx];
        if (!pwmMotorConfig(timHw, idx, feature(FEATURE_PWM_OUTPUT_ENABLE))) {
            pwmInitError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
            LOG_ERROR(PWM, "Timer allocation failed for motor %d", idx);
            return;
        }
    }
}

static void pwmInitServos(timMotorServoHardware_t * timOutputs)
{
    const int servoCount = getServoCount();

    if (!isMixerUsingServos()) {
        LOG_INFO(PWM, "Mixer does not use servos");
        return;
    }

    // Check if too many servos
    if (servoCount > MAX_SERVOS) {
        pwmInitError = PWM_INIT_ERROR_TOO_MANY_SERVOS;
        LOG_ERROR(PWM, "Too many servos. Mixer requested %d, max %d", servoCount, MAX_SERVOS);
        return;
    }

    // Do the pre-configuration. This should configure non-timer PWM drivers
    pwmServoPreconfigure();

    // Check if we need to init timer output for servos
    if (!servosUseHardwareTimers()) {
        // External PWM servo driver
        LOG_INFO(PWM, "Skipped timer init for servos - using external servo driver");
        return;
    }


    // If mixer requests more servos than we have timer outputs - throw an error
    uint16_t maxServos = timOutputs->maxTimServoCount;
    if (servoConfig()->servo_protocol == SERVO_TYPE_SBUS_PWM) {
        maxServos = MAX(SERVO_SBUS_MAX_SERVOS, timOutputs->maxTimServoCount);
    }

    if (servoCount > maxServos) {
        pwmInitError = PWM_INIT_ERROR_NOT_ENOUGH_SERVO_OUTPUTS;
        LOG_ERROR(PWM, "Too many servos. Mixer requested %d, timer outputs %d", servoCount, timOutputs->maxTimServoCount);
        return;
    }

    // Configure individual servo outputs
    for (int idx = 0; idx < MIN(servoCount, timOutputs->maxTimServoCount); idx++) {
        const timerHardware_t *timHw = timOutputs->timServos[idx];

        if (!pwmServoConfig(timHw, idx, servoConfig()->servoPwmRate, servoConfig()->servoCenterPulse, feature(FEATURE_PWM_OUTPUT_ENABLE))) {
            pwmInitError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
            LOG_ERROR(PWM, "Timer allocation failed for servo %d", idx);
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

#endif

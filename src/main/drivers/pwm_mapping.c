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
#include "fc/runtime_config.h"

#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/timer.h"
#include "drivers/logging.h"
#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
//#include "drivers/rx_pwm.h"

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

const motorProtocolProperties_t * getMotorProtocolProperties(motorPwmProtocolTypes_e proto)
{
    return &motorProtocolProperties[proto];
}

static bool checkPwmTimerConflicts(const timerHardware_t *timHw)
{
#if defined(USE_UART2)
    if (doesConfigurationUsePort(SERIAL_PORT_USART2) && (timHw->tag == IO_TAG(UART2_TX_PIN) || timHw->tag == IO_TAG(UART2_RX_PIN))) {
        return true;
    }
#endif

#if defined(USE_UART3)
    if (doesConfigurationUsePort(SERIAL_PORT_USART3) && (timHw->tag == IO_TAG(UART3_TX_PIN) || timHw->tag == IO_TAG(UART3_RX_PIN))) {
        return true;
    }
#endif

#if defined(USE_UART4)
    if (doesConfigurationUsePort(SERIAL_PORT_USART4) && (timHw->tag == IO_TAG(UART4_TX_PIN) || timHw->tag == IO_TAG(UART4_RX_PIN))) {
        return true;
    }
#endif

#if defined(USE_UART5)
    if (doesConfigurationUsePort(SERIAL_PORT_USART5) && (timHw->tag == IO_TAG(UART5_TX_PIN) || timHw->tag == IO_TAG(UART5_RX_PIN))) {
        return true;
    }
#endif

#if defined(USE_UART6)
    if (doesConfigurationUsePort(SERIAL_PORT_USART6) && (timHw->tag == IO_TAG(UART6_TX_PIN) || timHw->tag == IO_TAG(UART6_RX_PIN))) {
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

void pwmBuildTimerOutputList(timMotorServoHardware_t * timOutputs)
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
            if (timHw->usageFlags & TIM_USE_MC_SERVO) {
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


bool pwmMotorAndServoInit(void)
{
    timMotorServoHardware_t timOutputs;

    // Build temporary timer mappings for motor and servo
    pwmBuildTimerOutputList(&timOutputs);

    const int servoCount = getServoCount();
    const int motorCount = getMotorCount();

    // Disable the arming blocked flag, everything is ok
    DISABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);

    // At this point we have built tables of timers suitable for motor and servo mappings
    // Now we can actually initialize them according to motor/servo count from mixer
    if (motorCount > MAX_MOTORS) {
        // Too many motors
        ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
        LOG_E(PWM, "Too many motors. Mixer requested %d, max %d", motorCount, MAX_MOTORS);
    }
    else {
        // Do the pre-configuration. For motors w/o hardware timers this should be sufficient
        pwmMotorPreconfigure();

        // Now if we need to configure individual motor outputs - do that
        if (getMotorProtocolProperties(motorConfig()->motorPwmProtocol)->usesHwTimer) {
            if (motorCount > timOutputs.maxTimMotorCount) {
                ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
                LOG_E(PWM, "Too many motors. Mixer requested %d, timer outputs %d", motorCount, timOutputs.maxTimMotorCount);
            }
            else {
                for (int idx = 0; idx < motorCount; idx++) {
                    const timerHardware_t *timHw = timOutputs.timMotors[idx];
                    if (!pwmMotorConfig(timHw, idx, feature(FEATURE_PWM_OUTPUT_ENABLE))) {
                        ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
                        LOG_E(PWM, "Timer allocation failed for motor %d", idx);
                    }
                }
            }
        }
        else {
            LOG_I(PWM, "Skipped timer init for motors");
        }
    }

    // Initialize servos:
    if (isMixerUsingServos()) {
        if (servoCount > MAX_SERVOS) {
            // Too many servos
            ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
            LOG_E(PWM, "Too many servos. Mixer requested %d, max %d", servoCount, MAX_SERVOS);
        }
        else {
            // Do the pre-configuration. This should configure non-timer PWM drivers
            if (!feature(FEATURE_PWM_SERVO_DRIVER)) {
                if (servoCount > timOutputs.maxTimServoCount) {
                    ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
                    LOG_E(PWM, "Too many servos. Mixer requested %d, timer outputs %d", servoCount, timOutputs.maxTimServoCount);
                }
                else {
                    for (int idx = 0; idx < servoCount; idx++) {
                        const timerHardware_t *timHw = timOutputs.timServos[idx];

                        if (!pwmServoConfig(timHw, idx, servoConfig()->servoPwmRate, servoConfig()->servoCenterPulse, feature(FEATURE_PWM_OUTPUT_ENABLE))) {
                            ENABLE_ARMING_FLAG(ARMING_DISABLED_PWM_OUTPUT_ERROR);
                            LOG_E(PWM, "Timer allocation failed for servo %d", idx);
                        }
                    }
                }

            }
            else {
                // External PWM servo driver
                LOG_I(PWM, "Skipped timer init for servos - using FEATURE_PWM_SERVO_DRIVER");
            }
        }
    }

    return true;
}

/*
pwmIOConfiguration_t *pwmInit(drv_pwm_config_t *init)
{


        // If timer not mapped - skip
        if (type == MAP_TO_NONE)
            continue;

        if (type == MAP_TO_PPM_INPUT) {
#if defined(USE_RX_PPM)
            ppmInConfig(timerHardwarePtr);
            pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_PPM;
            pwmIOConfiguration.ppmInputCount++;

            addBootlogEvent6(BOOT_EVENT_TIMER_CH_MAPPED, BOOT_EVENT_FLAGS_NONE, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 0);
#endif
        } else if (type == MAP_TO_PWM_INPUT) {
#if defined(USE_RX_PWM)
            pwmInConfig(timerHardwarePtr, channelIndex);
            pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_PWM;
            pwmIOConfiguration.pwmInputCount++;
            channelIndex++;

            addBootlogEvent6(BOOT_EVENT_TIMER_CH_MAPPED, BOOT_EVENT_FLAGS_NONE, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 1);
#endif
        } else if (type == MAP_TO_MOTOR_OUTPUT) {
            // Check if we already configured maximum supported number of motors and skip the rest
            if (pwmIOConfiguration.motorCount >= MAX_MOTORS) {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 2);
                continue;
            }

            if (pwmMotorConfig(timerHardwarePtr, pwmIOConfiguration.motorCount, motorConfig()->motorPwmRate, init->enablePWMOutput)) {
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_MOTOR;
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].index = pwmIOConfiguration.motorCount;
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].timerHardware = timerHardwarePtr;

                pwmIOConfiguration.motorCount++;

                addBootlogEvent6(BOOT_EVENT_TIMER_CH_MAPPED, BOOT_EVENT_FLAGS_NONE, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 2);
            }
            else {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 2);
                continue;
            }
        } else if (type == MAP_TO_SERVO_OUTPUT) {
            if (pwmIOConfiguration.servoCount >=  MAX_SERVOS) {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
                continue;
            }

            if (pwmServoConfig(timerHardwarePtr, pwmIOConfiguration.servoCount, init->servoPwmRate, init->servoCenterPulse, init->enablePWMOutput)) {
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_SERVO;
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].index = pwmIOConfiguration.servoCount;
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].timerHardware = timerHardwarePtr;

                pwmIOConfiguration.servoCount++;

                addBootlogEvent6(BOOT_EVENT_TIMER_CH_MAPPED, BOOT_EVENT_FLAGS_NONE, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            }
            else {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
                continue;
            }
        } else {
            continue;
        }

        pwmIOConfiguration.ioCount++;
    }

    return &pwmIOConfiguration;
}
*/
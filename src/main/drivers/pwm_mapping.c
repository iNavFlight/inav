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

#include "common/memory.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/timer.h"
#include "drivers/logging.h"

#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/rx_pwm.h"

enum {
    MAP_TO_NONE,
    MAP_TO_PPM_INPUT,
    MAP_TO_PWM_INPUT,
    MAP_TO_MOTOR_OUTPUT,
    MAP_TO_SERVO_OUTPUT,
};

static pwmIOConfiguration_t pwmIOConfiguration;

pwmIOConfiguration_t *pwmGetOutputConfiguration(void)
{
    return &pwmIOConfiguration;
}

bool CheckGPIOPin(ioTag_t tag, GPIO_TypeDef *gpio, uint16_t pin)
{
    return IO_GPIOBYTAG(tag) == gpio && IO_PINBYTAG(tag) == pin;
}

bool CheckGPIOPinSource(ioTag_t tag, GPIO_TypeDef *gpio, uint16_t pin)
{
    return IO_GPIOBYTAG(tag) == gpio && IO_GPIO_PinSource(IOGetByTag(tag)) == pin;
}

pwmIOConfiguration_t *pwmInit(drv_pwm_config_t *init)
{
    memset(&pwmIOConfiguration, 0, sizeof(pwmIOConfiguration));
    pwmIOConfiguration.ioConfigurations = memAllocate(sizeof(pwmPortConfiguration_t) * timerHardwareCount, OWNER_PWMIO);

#if defined(USE_RX_PWM) || defined(USE_RX_PPM)
    int channelIndex = 0;
#endif

    for (int timerIndex = 0; timerIndex < timerHardwareCount; timerIndex++) {
        const timerHardware_t *timerHardwarePtr = &timerHardware[timerIndex];
        int type = MAP_TO_NONE;

#if defined(STM32F3) && defined(USE_UART3)
        // skip UART3 ports (PB10/PB11)
        if (init->useUART3 && (timerHardwarePtr->tag == IO_TAG(UART3_TX_PIN) || timerHardwarePtr->tag == IO_TAG(UART3_RX_PIN))) {
            addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            continue;
        }
#endif

#if defined(UART6_TX_PIN) || defined(UART6_RX_PIN)
        if (init->useUART6 && (timerHardwarePtr->tag == IO_TAG(UART6_TX_PIN) || timerHardwarePtr->tag == IO_TAG(UART6_RX_PIN))) {
            addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            continue;
        }
#endif

#if defined(USE_SOFTSERIAL1)
        if (init->useSoftSerial) {
            const timerHardware_t *ss1TimerHardware = timerGetByTag(IO_TAG(SOFTSERIAL_1_RX_PIN), TIM_USE_ANY);
            if (ss1TimerHardware != NULL && ss1TimerHardware->tim == timerHardwarePtr->tim) {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
                continue;
            }
        }
#endif

#if defined(USE_SOFTSERIAL2)
        if (init->useSoftSerial) {
            const timerHardware_t *ss2TimerHardware = timerGetByTag(IO_TAG(SOFTSERIAL_2_RX_PIN), TIM_USE_ANY);
            if (ss2TimerHardware != NULL && ss2TimerHardware->tim == timerHardwarePtr->tim) {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
                continue;
            }
        }
#endif

#if defined(USE_LED_STRIP)
        // skip LED Strip output
        if (init->useLEDStrip) {
            const timerHardware_t * ledTimHw = timerGetByTag(IO_TAG(WS2811_PIN), TIM_USE_ANY);
            if (ledTimHw != NULL && timerHardwarePtr->tim == ledTimHw->tim) {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
                continue;
            }
        }
#endif

#ifdef VBAT_ADC_PIN
        if (init->useVbat && timerHardwarePtr->tag == IO_TAG(VBAT_ADC_PIN)) {
            addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            continue;
        }
#endif

#ifdef RSSI_ADC_PIN
        if (init->useRSSIADC && timerHardwarePtr->tag == IO_TAG(RSSI_ADC_PIN)) {
            addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            continue;
        }
#endif

#ifdef CURRENT_METER_ADC_PIN
        if (init->useCurrentMeterADC && timerHardwarePtr->tag == IO_TAG(CURRENT_METER_ADC_PIN)) {
            addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            continue;
        }
#endif

#ifdef USE_RANGEFINDER_HCSR04
        if (init->useTriggerRangefinder &&
            (
                timerHardwarePtr->tag == init->rangefinderIOConfig.triggerTag ||
                timerHardwarePtr->tag == init->rangefinderIOConfig.echoTag
            )) {
            addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 3);
            continue;
        }
#endif

        // Handle timer mapping to PWM/PPM inputs
        if (init->useSerialRx) {
            type = MAP_TO_NONE;
        }
        else if (init->useParallelPWM && (timerHardwarePtr->usageFlags & TIM_USE_PWM)) {
            type = MAP_TO_PWM_INPUT;
        }
        else if (init->usePPM && (timerHardwarePtr->usageFlags & TIM_USE_PPM)) {
            type = MAP_TO_PPM_INPUT;
        }

        // Handle outputs - may override the PWM/PPM inputs
        if (init->flyingPlatformType == PLATFORM_MULTIROTOR || init->flyingPlatformType == PLATFORM_TRICOPTER) {
            // Multicopter
            if (init->useServoOutputs && (timerHardwarePtr->usageFlags & TIM_USE_MC_SERVO)) {
                type = MAP_TO_SERVO_OUTPUT;
            }
            else if (timerHardwarePtr->usageFlags & TIM_USE_MC_MOTOR) {
                type = MAP_TO_MOTOR_OUTPUT;
            }
        } else {
            // Fixed wing or HELI (one/two motors and a lot of servos
            if (timerHardwarePtr->usageFlags & TIM_USE_FW_SERVO) {
                type = MAP_TO_SERVO_OUTPUT;
            }
            else if (timerHardwarePtr->usageFlags & TIM_USE_FW_MOTOR) {
                type = MAP_TO_MOTOR_OUTPUT;
            }
        }

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
            /* Check if we already configured maximum supported number of motors and skip the rest */
            if (pwmIOConfiguration.motorCount >= MAX_MOTORS) {
                addBootlogEvent6(BOOT_EVENT_TIMER_CH_SKIPPED, BOOT_EVENT_FLAGS_WARNING, timerIndex, pwmIOConfiguration.motorCount, pwmIOConfiguration.servoCount, 2);
                continue;
            }

            if (pwmMotorConfig(timerHardwarePtr, pwmIOConfiguration.motorCount, init->motorPwmRate, init->pwmProtocolType, init->enablePWMOutput)) {
                if (init->useFastPwm) {
                    pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_MOTOR | PWM_PF_OUTPUT_PROTOCOL_FASTPWM | PWM_PF_OUTPUT_PROTOCOL_PWM;
                } else if (init->pwmProtocolType == PWM_TYPE_BRUSHED) {
                    pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_MOTOR | PWM_PF_MOTOR_MODE_BRUSHED | PWM_PF_OUTPUT_PROTOCOL_PWM;
                } else {
                    pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_MOTOR | PWM_PF_OUTPUT_PROTOCOL_PWM ;
                }

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
                pwmIOConfiguration.ioConfigurations[pwmIOConfiguration.ioCount].flags = PWM_PF_SERVO | PWM_PF_OUTPUT_PROTOCOL_PWM;
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

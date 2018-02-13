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

#define TARGET_BOARD_IDENTIFIER "OLI1" // Olimexino

//#define OLIMEXINO_UNCUT_LED1_E_JUMPER
//#define OLIMEXINO_UNCUT_LED2_E_JUMPER

#ifdef OLIMEXINO_UNCUT_LED1_E_JUMPER
#define LED0                    PA5 // D13, PA5/SPI1_SCK/ADC5 - "LED1" on silkscreen, Green
#endif

#ifdef OLIMEXINO_UNCUT_LED2_E_JUMPER
// "LED2" is using one of the PWM pins (CH2/PWM2), so we must not use PWM2 unless the jumper is cut.  @See pwmInit()
#define LED1                    PA1 // D3, PA1/USART2_RTS/ADC1/TIM2_CH3 - "LED2" on silkscreen, Yellow
#endif

#define USE_GYRO
#define USE_FAKE_GYRO
#define USE_GYRO_MPU6050

#define USE_ACC
#define USE_FAKE_ACC
#define USE_ACC_MPU6050

#define MPU6050_I2C_BUS     BUS_I2C2

#define USE_BARO
#define BARO_I2C_BUS        BUS_I2C2
#define USE_BARO_BMP085
#define USE_BARO_BMP280

#define USE_MAG
#define MAG_I2C_BUS         BUS_I2C2
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_MAG3110

// #define USE_RANGEFINDER
// #define USE_RANGEFINDER_HCSR04
#define RANGEFINDER_HCSR04_TRIGGER_PIN       PB0 // RX7 (PB0) - only 3.3v ( add a 1K Ohms resistor )
#define RANGEFINDER_HCSR04_ECHO_PIN          PB1 // RX8 (PB1) - only 3.3v ( add a 1K Ohms resistor )

#define USE_UART1
#define USE_UART2
#define USE_SOFTSERIAL1
#define USE_SOFTSERIAL2
#define SERIAL_PORT_COUNT       4

#define SOFTSERIAL_1_RX_PIN     PA6
#define SOFTSERIAL_1_TX_PIN     PA7
#define SOFTSERIAL_2_RX_PIN     PB0
#define SOFTSERIAL_2_TX_PIN     PB1

#define USE_I2C
#define USE_I2C_DEVICE_2

#define USE_ADC
#define ADC_CHANNEL_1_PIN               PB1
#define ADC_CHANNEL_2_PIN               PA4
#define ADC_CHANNEL_3_PIN               PA1
#define ADC_CHANNEL_4_PIN               PA5
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_1
#define VBAT_ADC_CHANNEL                ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS    10

// IO - assuming all IOs on smt32f103rb LQFP64 package
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define USABLE_TIMER_CHANNEL_COUNT 14
#define USED_TIMERS             (TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(4))

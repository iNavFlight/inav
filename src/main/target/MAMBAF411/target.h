/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#define TARGET_BOARD_IDENTIFIER "MB41"
#define USBD_PRODUCT_STRING     "MAMBA411"

// ******* LEDs and BEEPER ********

#define LED0_PIN                PC13
#define LED1_PIN                PC14

#define USE_BEEPER
#define BEEPER_PIN              PB2
#define BEEPER_INVERTED

#define ENABLE_DSHOT_DMAR       DSHOT_DMAR_AUTO
#define DSHOT_BITBANG_DEFAULT   DSHOT_BITBANG_OFF


// ******* GYRO and ACC ********

//#define USE_EXTI
//#define USE_GYRO_EXTI
//#define GYRO_1_EXTI_PIN         PB0
//#define USE_MPU_DATA_READY_SIGNAL

//#define GYRO_1_CS_PIN           PA4
//#define GYRO_1_SPI_INSTANCE     SPI1

//#define USE_GYRO
//#define USE_GYRO_SPI_MPU6000
//#define GYRO_1_ALIGN            CW180_DEG

//#define USE_ACC
//#define USE_ACC_SPI_MPU6000
// *************** SPI Gyro & ACC **********************
#define USE_EXTI
#define GYRO_INT_EXTI           PB0
#define USE_MPU_DATA_READY_SIGNAL

#define USE_IMU_MPU6000
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_BUS         BUS_SPI1
#define IMU_MPU6000_ALIGN       CW180_DEG



// ******* OSD ********

#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// ******* SERIAL ********

#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

//#define USE_SOFTSERIAL1
//#define USE_SOFTSERIAL2

#define SERIAL_PORT_COUNT       3 //VCP, USART1, USART2, SOFTSERIAL x 2

#define USE_ESCSERIAL
#define ESCSERIAL_TIMER_TX_PIN  PB9  // (HARDARE=0,PPM)

// ******* INVERTER PIN ********
#define USE_UART_INVERTER
#define INVERTER_PIN_UART1      PB10

// ******* SPI ********

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

// ******* ADC ********

#define USE_ADC
#define DEFAULT_VOLTAGE_METER_SOURCE VOLTAGE_METER_ADC
#define ADC_INSTANCE            ADC1  // Default added
#define ADC1_DMA_OPT            0     // DMA 2 Stream 0 Channel 0
#define VBAT_ADC_PIN            PA0
#define RSSI_ADC_PIN            PB1
#define CURRENT_METER_ADC_PIN   PA1

// ******* FEATURES ********

#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define SERIALRX_UART           SERIAL_PORT_USART1
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define DEFAULT_FEATURES                (FEATURE_OSD | FEATURE_TELEMETRY)
#define DEFAULT_VOLTAGE_METER_SOURCE    VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE    CURRENT_METER_ADC

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS            8
#define TARGET_MOTOR_COUNT              4   

#define USABLE_TIMER_CHANNEL_COUNT 8
#define USED_TIMERS             ( TIM_N(1) | TIM_N(2) | TIM_N(3) | TIM_N(4) | TIM_N(10) | TIM_N(11) )
// ESC-related features
#define USE_DSHOT
#define USE_SERIALSHOT
//#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
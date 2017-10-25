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
#define TARGET_BOARD_IDENTIFIER "WINGF4"

#define USBD_PRODUCT_STRING     "FURIOUS WINGF4"

#define LED0                    PC10 // Blue LED
// #define LED1                    PC10 // Red LED
// #define LED2                    PC10 // Green LED

#define BEEPER                  PC3
#define BEEPER_INVERTED

// MPU interrupt
#define USE_EXTI
#define MPU_INT_EXTI            PC4
#define USE_MPU_DATA_READY_SIGNAL

#define MPU6500_CS_PIN          PC0
#define MPU6500_SPI_INSTANCE    SPI3

#define ACC
#define USE_ACC_SPI_MPU6500
#define ACC_MPU6500_ALIGN       CW180_DEG

#define GYRO
#define USE_GYRO_SPI_MPU6500
#define GYRO_MPU6500_ALIGN      CW180_DEG

#define MAG
#define USE_MAG_AK8963

#define MAG_AK8963_ALIGN        CW180_DEG

#define BARO
#define USE_BARO_MS5611
#define USE_BARO_SPI_MS5611
#define MS56XX_CS_PIN           PC5
#define MS56XX_SPI_INSTANCE     SPI3

#define OSD
#define USE_MAX7456
#define MAX7456_SPI_CS_PIN      PA4
#define MAX7456_SPI_INSTANCE    SPI1

#define USB_IO
#define USE_VCP
// #define VBUS_SENSING_PIN        PA9

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define INVERTER_PIN_UART3      PA8

#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PD2
#define UART5_TX_PIN            PC11

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       7 //VCP, UART1, UART2, UART3, UART4, UART5, UART6

#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            NONE
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            NONE
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

#define USE_I2C
#define I2C_DEVICE              (I2CDEV_2)
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define BOARD_HAS_VOLTAGE_DIVIDER
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC2
#define ADC_CHANNEL_2_PIN               PC1
#define VBAT_ADC_CHANNEL                ADC_CHN_2
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_1

#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_BLACKBOX)

// Number of available PWM outputs
#define MAX_PWM_OUTPUT_PORTS       6
#define USABLE_TIMER_CHANNEL_COUNT 6

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define USED_TIMERS             ( TIM_N(3) | TIM_N(4) | TIM_N(8))


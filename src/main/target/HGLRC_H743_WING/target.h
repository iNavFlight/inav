/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */


#pragma once

#define TARGET_BOARD_IDENTIFIER "HG7W"
#define USBD_PRODUCT_STRING     "HGLRC_H743_WING"

#define USE_TARGET_CONFIG

#define LED0                    PE6
#define LED1                    PE4

#define BEEPER                  PE5
#define BEEPER_INVERTED

#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PD5
#define UART2_RX_PIN            PD6

#define USE_UART3
#define UART3_TX_PIN            PD8
#define UART3_RX_PIN            PD9

#define USE_UART4
#define UART4_TX_PIN            PD1
#define UART4_RX_PIN            PA1

#define USE_UART5
#define UART5_TX_PIN            PB6
#define UART5_RX_PIN            PB5

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define SERIAL_PORT_COUNT       7

#define USE_SPI

// SPI1 (IMU)
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7
#define SPI1_NSS_PIN            PA4

// SPI2 (OSD)
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15
#define SPI2_NSS_PIN            PB12

// SPI3 (Flash)
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB2
#define SPI3_NSS_PIN            PA15

#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PA4
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

#define USE_I2C

#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB7

#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_DPS310
#define USE_BARO_SPL06

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define USE_RANGEFINDER
#define USE_RANGEFINDER_US42
#define RANGEFINDER_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1
#define TEMPERATURE_I2C_BUS     BUS_I2C1

#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT

#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define ADC_CHANNEL_3_PIN           PC4

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define AIRSPEED_ADC_CHANNEL        ADC_CHN_3

#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PE2
#define PINIO2_PIN                  PE1

#define DEFAULT_FEATURES            (FEATURE_TX_PROF_SEL | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_BLACKBOX)
#define CURRENT_METER_SCALE         200
#define VBAT_SCALE_DEFAULT          2100

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS 13

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define DEFAULT_RX_TYPE   RX_TYPE_SERIAL
#define SERIALRX_PROVIDER SERIALRX_CRSF
#define SERIALRX_UART     SERIAL_PORT_USART2

#define GPS_UART          SERIAL_PORT_USART3

#define USE_LED_STRIP
#define WS2811_PIN        PA0

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

#define TARGET_BOARD_IDENTIFIER   "LUFC"

#define USBD_PRODUCT_STRING       "TBS Lucid FC"

#define LED0                      PC14
#define LED1                      PC15

#define BEEPER_INVERTED
#define BEEPER                    PC13

#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN              PA9
#define UART1_RX_PIN              PA10

#define USE_UART2
#define UART2_TX_PIN              NONE
#define UART2_RX_PIN              PB0

#define USE_UART3
#define UART3_TX_PIN              PB11
#define UART3_RX_PIN              PB10

#define USE_UART4
#define UART4_TX_PIN              PH3
#define UART4_RX_PIN              PH2

#define USE_UART5
#define UART5_TX_PIN              PB9
#define UART5_RX_PIN              PD2

#define USE_UART7
#define UART7_TX_PIN              PB4
#define UART7_RX_PIN              PB3

#define USE_UART8
#define UART8_TX_PIN              PC2
#define UART8_RX_PIN              PC3

#define SERIAL_PORT_COUNT         8

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN              PA5
#define SPI1_MISO_PIN             PA6
#define SPI1_MOSI_PIN             PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN              PB13
#define SPI2_MISO_PIN             PB14
#define SPI2_MOSI_PIN             PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN              PC10
#define SPI3_MISO_PIN             PC11
#define SPI3_MOSI_PIN             PC12

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN         CW0_DEG_FLIP
#define MPU6000_SPI_BUS           BUS_SPI1
#define MPU6000_CS_PIN            PA4

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN        CW270_DEG_FLIP
#define ICM42605_CS_PIN           PA4
#define ICM42605_SPI_BUS          BUS_SPI1

#define USE_MAX7456
#define MAX7456_SPI_BUS           BUS_SPI2
#define MAX7456_CS_PIN            PB12

#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS            BUS_SPI3
#define M25P16_CS_PIN             PA15

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                  PB6
#define I2C1_SDA                  PC7

#define USE_BARO
#define USE_BARO_BMP388
#define BARO_I2C_BUS              BUS_I2C1

#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS               BUS_I2C1

#define USE_LED_STRIP
#define WS2811_PIN                PA8

#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                PC5  // Camera Control

#define USE_ADC
#define ADC_INSTANCE              ADC1
#define ADC1_DMA_STREAM           DMA2_CHANNEL7
#define ADC_CHANNEL_1_PIN         PC0
#define ADC_CHANNEL_2_PIN         PC1
#define VBAT_ADC_CHANNEL          ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL ADC_CHN_2

#define TARGET_IO_PORTA           0xffff
#define TARGET_IO_PORTB           0xffff
#define TARGET_IO_PORTC           0xffff
#define TARGET_IO_PORTD           (BIT(2) | BIT(12) | BIT(13))
#define TARGET_IO_PORTH           (BIT(2) | BIT(3))

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define MAX_PWM_OUTPUT_PORTS 6

#define DEFAULT_FEATURES                                                      \
    (FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_OSD | \
     FEATURE_LED_STRIP)

#define DEFAULT_RX_TYPE   RX_TYPE_SERIAL
#define SERIALRX_PROVIDER SERIALRX_CRSF
#define SERIALRX_UART     SERIAL_PORT_USART5
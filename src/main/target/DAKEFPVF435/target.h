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

#define TARGET_BOARD_IDENTIFIER "DAK4"
#define USBD_PRODUCT_STRING  "DAKEFPV F435"

#define LED0                    PA15

#define BEEPER                  PC3
#define BEEPER_INVERTED


// Buses

#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2 
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN   	    PB4
#define SPI3_MOSI_PIN   	    PB5

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9
// #define USE_I2C_PULLUP

#define DEFAULT_I2C_BUS         BUS_I2C1

// Gyro
#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN       CW90_DEG
#define MPU6500_CS_PIN          PA4
#define MPU6500_SPI_BUS         BUS_SPI1

// ICM42605/ICM42688P
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW90_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PA4

//Baro 
#define USE_BARO
#define USE_BARO_BMP280
#define BMP280_I2C_BUS          DEFAULT_I2C_BUS

// Other sensors

#define USE_BARO
#define BARO_I2C_BUS            DEFAULT_I2C_BUS
#define USE_BARO_ALL

#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C_BUS
#define USE_MAG_ALL

// OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// Flash
  
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS          BUS_SPI3
#define M25P16_CS_PIN           PB1

#define USE_FLASH_W25N01G
#define W25N01G_SPI_BUS         BUS_SPI3
#define W25N01G_CS_PIN          PB1

// UARTs
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN         PB6
#define UART1_RX_PIN         PB7

// USART3 RX only
#define USE_UART3
#define UART3_RX_PIN         PB11
#define UART3_TX_PIN         PC13 // PC13 is just for compilation pass

#define USE_UART4
#define UART4_TX_PIN         PC10
#define UART4_RX_PIN         PC11

#define USE_UART5
#define UART5_TX_PIN         PC12
#define UART5_RX_PIN         PD2

#define USE_UART6
#define UART6_TX_PIN         PC6
#define UART6_RX_PIN         PC7

#define SERIAL_PORT_COUNT       6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART5
#define GPS_UART                SERIAL_PORT_USART1

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC1
#define ADC_CHANNEL_2_PIN           PC2
#define ADC_CHANNEL_3_PIN           PC0
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define RSSI_ADC_CHANNEL            ADC_CHN_2
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_3

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY )

#define USE_LED_STRIP
#define WS2811_PIN                  PC8

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA             0xffff
#define TARGET_IO_PORTB             0xffff
#define TARGET_IO_PORTC             0xffff
#define TARGET_IO_PORTD             0xffff
#define TARGET_IO_PORTE             0xffff
#define TARGET_IO_PORTH             0xffff

#define USE_DSHOT
#define USE_ESC_SENSOR
#define MAX_PWM_OUTPUT_PORTS    11

// PINIO
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN           PB0
#define PINIO2_PIN           PB10

// VBAT 10K/160K
#define VBAT_SCALE_DEFAULT 1094

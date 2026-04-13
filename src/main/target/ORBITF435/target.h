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

#ifdef ORBITF435
#define TARGET_BOARD_IDENTIFIER "ORB4"
#define USBD_PRODUCT_STRING     "ORBITF435"
#else
#define TARGET_BOARD_IDENTIFIER "ORB4SD"
#define USBD_PRODUCT_STRING     "ORBITF435_SD"
#endif

/*** Indicators ***/
#define LED0                    PC13  //Blue  

#define BEEPER                  PC15
#define BEEPER_INVERTED

/*** PINIO ***/
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PC8
#define PINIO1_FLAGS                PINIO_FLAGS_INVERTED

// *************** UART *****************************
#define USE_VCP
#define USB_DETECT_PIN          PC9
#define USE_UART_INVERTER

#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6

#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3
#define INVERTER_PIN_UART2_RX   PB2

#define UART3_TX_PIN            PC10 // No connection
#define UART3_RX_PIN            PC11 // ESC TLM

#define UART4_TX_PIN            PH3
#define UART4_RX_PIN            PH2 

#define UART5_TX_PIN            PC12
#define UART5_RX_PIN            PD2  

#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define SERIAL_PORT_COUNT       7 //VCP, UART1, UART2, UART3, UART4, UART5, UART6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART2

// *************** Gyro & ACC **********************
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_ICM42605        //Using ICM42688
#define IMU_ICM42688_ALIGN      CW270_DEG
#define ICM42688_CS_PIN         PA4
#define ICM42688_SPI_BUS        BUS_SPI1

// *************** I2C(Baro & I2C) **************************
#define USE_I2C
#define USE_BARO
#define USE_MAG
#define USE_RANGEFINDER

#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_BMP085
#define USE_BARO_DPS310
#define USE_BARO_SPL06

#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1
#define RANGEFINDER_I2C_BUS     BUS_I2C1

// *************** Internal SD card && FLASH **************************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

#if defined(ORBITF435_SD)
//SDCARD Definations
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI3
#define SDCARD_CS_PIN           PC14
#define SDCARD_DETECT_INVERTED
#define SDCARD_DETECT_PIN       PA8
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT
#else
//FLASHFS Definations
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS                  BUS_SPI3
#define M25P16_CS_PIN                   PB15
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#endif

// *************** OSD *****************************
#define USE_SPI_DEVICE_2
#define USE_OSD

#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** ADC *****************************

#define USE_ADC
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define ADC_CHANNEL_3_PIN           PC5

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

#define VBAT_SCALE_DEFAULT      1010
#define CURRENT_METER_SCALE     125

// *************** LED *****************************
#define USE_LED_STRIP
#define WS2811_PIN PA0

#define DEFAULT_FEATURES                (FEATURE_TX_PROF_SEL  | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT  | FEATURE_BLACKBOX | FEATURE_TELEMETRY | FEATURE_OSD | FEATURE_LED_STRIP)


#define USE_DSHOT
#define USE_SERIALSHOT
#define USE_ESCSERIAL
#define USE_ESC_SENSOR
#define USE_RPM_FILTER
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define MAX_PWM_OUTPUT_PORTS        11

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE BIT(2)
#define TARGET_IO_PORTH (BIT(1)|BIT(2)|BIT(3))
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

#define TARGET_BOARD_IDENTIFIER "SDH7V2"
#define USBD_PRODUCT_STRING     "SDMODELH7V2"

#define USE_TARGET_CONFIG

// *************** LED / BEEPER **********************
#define LED0                    PC2

#define BEEPER                  PC13
#define BEEPER_INVERTED

// *************** SPI1 - SD Card ********************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

// *************** SPI2 - OSD (MAX7456) **************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** SPI4 - IMU (MPU6000) **************
#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN            PE2
#define SPI4_MISO_PIN           PE5
#define SPI4_MOSI_PIN           PE6

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW270_DEG
#define MPU6000_SPI_BUS         BUS_SPI4
#define MPU6000_CS_PIN          PE4
#define MPU6000_EXTI_PIN        PE1

// *************** I2C1 - Baro / Mag *****************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_MS5611

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_IST8310
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     BUS_I2C1

// *************** SD Card (SPI) *********************
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI1
#define SDCARD_CS_PIN           PA4
#define SDCARD_DETECT_PIN       PA3
#define SDCARD_DETECT_INVERTED

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** UART ******************************
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
#define UART4_RX_PIN            PD0

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

// UART7 RX-only (ESC sensor); TX pin not routed but AF mapping requires a valid pin
#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define SERIAL_PORT_COUNT       7   // VCP, UART1-4, UART6, UART7

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** ADC *******************************
#define USE_ADC
#define ADC_INSTANCE            ADC1

#define ADC_CHANNEL_1_PIN       PC0
#define ADC_CHANNEL_2_PIN       PC1
#define ADC_CHANNEL_3_PIN       PC5

#define VBAT_ADC_CHANNEL        ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL        ADC_CHN_3

#define VBAT_SCALE_DEFAULT      1090
#define CURRENT_METER_SCALE     168

// *************** USB detect ************************
#define USB_DETECT_PIN          PA8

// *************** PINIO *****************************
#define USE_PINIO
#define USE_PINIOBOX

// VTX power
#define PINIO1_PIN              PB11
#define PINIO1_FLAGS            PINIO_FLAGS_INVERTED

// Cam pin
#define PINIO2_PIN              PE9
#define PINIO2_FLAGS            PINIO_FLAGS_INVERTED

// Bluetooth (off on arm)
#define PINIO3_PIN              PE13
#define PINIO3_FLAGS            PINIO_FLAGS_INVERTED

// *************** LED STRIP *************************
#define USE_LED_STRIP
#define WS2811_PIN              PD12

// *************** DEFAULT FEATURES ******************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

// *************** SERIAL 4WAY ***********************
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// *************** ESC SENSOR ************************
#define USE_ESC_SENSOR

// *************** DSHOT *****************************
#define USE_DSHOT

// *************** IO PORT MASK **********************
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define MAX_PWM_OUTPUT_PORTS    8

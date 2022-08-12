/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "SB73"
#define USBD_PRODUCT_STRING  "SpeedyBeeF7V3"
#define USE_TARGET_CONFIG

#define LED0                PC13

// *************** UART *****************************
#define USB_IO
#define USE_VCP
#define VBUS_SENSING_PIN          C01
#define VBUS_SENSING_ENABLED

#define USE_UART1
#define UART1_RX_PIN            A10
#define UART1_TX_PIN            A09

#define USE_UART2
#define UART2_RX_PIN            A03
#define UART2_TX_PIN            A02

#define USE_UART3
#define UART3_RX_PIN            B11
#define UART3_TX_PIN            B10

#define USE_UART4
#define UART4_RX_PIN            A01

#define USE_UART6
#define UART6_RX_PIN            C07
#define UART6_TX_PIN            C06

#define SERIAL_PORT_COUNT       6

// *************** Gyro & ACC **********************
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW0FLIP

#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            A05
#define SPI1_MISO_PIN           A06
#define SPI1_MOSI_PIN           A07

#define MPU6000_CS_PIN           C04
#define MPU6000_SPI_BUS     BUS_SPI1

#define USE_EXTI
#define GYRO_INT_EXTI         CO4
#define USE_MPU_DATA_READY_SIGNAL

#define USE_BEEPER
#define BEEPER              C13
#define BEEPER_INVERTED

// *************** I2C(Baro & I2C) **************************
#define USE_I2C

#define USE_I2C_DEVICE_1
#define I2C1_SCL                B08
#define I2C1_SDA                B09

// Baro
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_DPS310
#define BARO_I2C_BUS          	BUS_I2C1	

// Mag
#define USE_MAG
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_LIS3MDL
#define MAG_I2C_BUS             BUS_I2C2

// *************** Flash **************************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            C10
#define SPI3_MISO_PIN           C11
#define SPI3_MOSI_PIN           C12

#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN            PA14
#define M25P16_SPI_BUS      BUS_SPI3
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** OSD *****************************
#define USE_OSD
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            B13
#define SPI2_MISO_PIN           B14
#define SPI2_MOSI_PIN           B15

#define USE_MAX7456
#define MAX7456_SPI_BUS    BUS_SPI2
#define MAX7456_CS_PIN      PB12

// *************** ADC *****************************
#define USE_ADC
#define ADC_CHANNEL_1_PIN           C02
#define ADC_CHANNEL_2_PIN           C00
#define ADC_CHANNEL_3_PIN           C01

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_3
#define RSSI_ADC_CHANNEL            ADC_CHN_2

// *************** LED *****************************
#define USE_LED_STRIP
#define WS2811_PIN A14

// ********** Optiical Flow adn Lidar **************
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP



#define DEFAULT_FEATURES            (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_OSD | FEATURE_BLACKBOX)
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_SERIALSHOT

#define SERIALRX_UART           SERIAL_PORT_USART1
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define MAX_PWM_OUTPUT_PORTS        4

#define CURRENT_METER_SCALE   102

#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  C09
#define PINIO1_FLAGS                PINIO_FLAGS_INVERTED

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

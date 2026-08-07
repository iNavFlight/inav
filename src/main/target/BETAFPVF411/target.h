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

#define TARGET_BOARD_IDENTIFIER "B411"
#define USBD_PRODUCT_STRING     "BETAFPVF411"

#define LED0                    PC13

#define BEEPER                  PB2
#define BEEPER_INVERTED

// *************** SPI **********************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN           PA5
#define SPI1_MISO_PIN   	   PA6
#define SPI1_MOSI_PIN   	   PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN           PB13
#define SPI2_MISO_PIN  		   PB14
#define SPI2_MOSI_PIN  		   PB15

// *************** SPI Gyro & ACC **********************
#define USE_IMU_MPU6000
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_BUS         BUS_SPI1
#define IMU_MPU6000_ALIGN       CW180_DEG

#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN                CW90_DEG
#define BMI270_SPI_BUS                  BUS_SPI1
#define BMI270_CS_PIN                   PA4

// *************** Baro *****************************

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define USE_BARO
#define BARO_I2C_BUS		    BUS_I2C1
#define USE_BARO_ALL

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

// *************** SPI OSD *****************************
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** SPI FLASH **************************
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PB2
#define M25P16_SPI_BUS          BUS_SPI2
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN     PB3
#define SOFTSERIAL_1_RX_PIN     PB3

#define USE_SOFTSERIAL2
#define SOFTSERIAL_2_TX_PIN     PB10
#define SOFTSERIAL_2_RX_PIN     PB10

#define SERIAL_PORT_COUNT       5

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART1

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                    ADC1
#define ADC_CHANNEL_1_PIN               PB0
#define ADC_CHANNEL_2_PIN               PB1

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2

#define VBAT_SCALE_DEFAULT              1100

// *************** LED2812 ************************
#define USE_LED_STRIP
#define WS2811_PIN                      PA8

// ***************  OTHERS *************************
#define DEFAULT_FEATURES                (FEATURE_TX_PROF_SEL | FEATURE_OSD | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_BLACKBOX)

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff

#define MAX_PWM_OUTPUT_PORTS       4

/*
 * This file is part of INAV.
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

#define TARGET_BOARD_IDENTIFIER "FW4N"
#define USBD_PRODUCT_STRING "FLYWOOF405NANO"

#define LED0                PA9

#define BEEPER              PC13
#define BEEPER_INVERTED

#define USE_DSHOT
#define USE_ESC_SENSOR

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW270_DEG
#define MPU6000_CS_PIN          SPI4_NSS_PIN
#define MPU6000_SPI_BUS         BUS_SPI4

#define USB_IO
#define USE_VCP
#define VBUS_SENSING_ENABLED
#define VBUS_SENSING_PIN          PA8 ---------------------

#define USE_UART1
#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PD5
#define UART2_RX_PIN            PD6

#define USE_UART3
#define UART3_TX_PIN            PB18
#define UART3_RX_PIN            PB11

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define SERIAL_PORT_COUNT 7 //VCP,UART1,UART2,UART3,UART4,UART5,UART6

#define USE_SPI
#define USE_SPI_DEVICE_1   //FLASH
#define USE_SPI_DEVICE_2   //OSD
#define USE_SPI_DEVICE_4   //ICM20689

#define SPI1_NSS_PIN            PA4               
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define SPI3_NSS_PIN            PB12              
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC23

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define USE_ADC
#define ADC_CHANNEL_1_PIN           PC2
#define ADC_CHANNEL_2_PIN           PC3
#define ADC_CHANNEL_3_PIN           PC0

#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_1
#define VBAT_ADC_CHANNEL            ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          SPI2_NSS_PIN

#define M25P16_CS_PIN           SPI1_NSS_PIN
#define M25P16_SPI_BUS          BUS_SPI1
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

//External I2C bus is the same as interal one
#define MAG_I2C_BUS             BUS_I2C1
#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define RANGEFINDER_I2C_BUS     BUS_I2C1
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define USE_BARO_BMP280
#define BARO_I2C_BUS            BUS_I2C1

#define USE_MAG
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_MAG3110
#define USE_MAG_IST8310
#define USE_MAG_IST8308
#define USE_MAG_LIS3MDL


#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_TELEMETRY | FEATURE_OSD | FEATURE_VBAT | FEATURE_BLACKBOX)
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_UART           SERIAL_PORT_USART1
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define USE_LED_STRIP
#define WS2811_PIN                      PA9 

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS       8


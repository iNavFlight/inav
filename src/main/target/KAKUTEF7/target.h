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

#ifdef KAKUTEF7MINI
#define TARGET_BOARD_IDENTIFIER "KF7M"
#define USBD_PRODUCT_STRING "KakuteF7-Mini"
#else
#define TARGET_BOARD_IDENTIFIER "KTF7"
#define USBD_PRODUCT_STRING "KakuteF7"
#endif

#define LED0                PA2

#define BEEPER              PD15
#define BEEPER_INVERTED

#define USE_DSHOT
#define USE_ESC_SENSOR



// ICM-20689
#define USE_IMU_ICM20689
#define IMU_ICM20689_ALIGN       CW270_DEG
#define ICM20689_CS_PIN          SPI4_NSS_PIN
#define ICM20689_SPI_BUS         BUS_SPI4

#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW270_DEG
#define MPU6000_CS_PIN          SPI4_NSS_PIN
#define MPU6000_SPI_BUS         BUS_SPI4


#define USB_IO
#define USE_VCP
#define VBUS_SENSING_ENABLED
#define VBUS_SENSING_PIN          PA8

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PD5
#define UART2_RX_PIN            PD6

#define USE_UART3
#define UART3_TX_PIN            PB10
#define UART3_RX_PIN            PB11

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define USE_UART7
#define UART7_TX_PIN            NONE
#define UART7_RX_PIN            PE7

#define SERIAL_PORT_COUNT 7 //VCP,UART1,UART2,UART3,UART4,UART6,UART7

#define USE_SPI
#define USE_SPI_DEVICE_1   //SD Card
#define USE_SPI_DEVICE_2   //OSD
#define USE_SPI_DEVICE_4   //ICM20689

#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define SPI4_NSS_PIN            PE4
#define SPI4_SCK_PIN            PE2
#define SPI4_MISO_PIN           PE5
#define SPI4_MOSI_PIN           PE6


#ifndef KAKUTEF7HDV
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          SPI2_NSS_PIN
#endif

#if defined(KAKUTEF7MINI)
#define M25P16_CS_PIN           SPI1_NSS_PIN
#define M25P16_SPI_BUS          BUS_SPI1
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#else
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI1
#define SDCARD_CS_PIN           SPI1_NSS_PIN
#define SDCARD_DETECT_INVERTED
#define SDCARD_DETECT_PIN       PD8
#endif

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7

#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06
#define BARO_I2C_BUS            BUS_I2C1

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C1

#define RANGEFINDER_I2C_BUS     BUS_I2C1

#define USE_ADC
#define ADC_CHANNEL_1_PIN           PC2
#define ADC_CHANNEL_2_PIN           PC3
#define ADC_CHANNEL_3_PIN           PC5

#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_1
#define VBAT_ADC_CHANNEL            ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_TELEMETRY | FEATURE_OSD | FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_BLACKBOX)
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_UART           SERIAL_PORT_USART6
#define SERIALRX_PROVIDER       SERIALRX_SBUS

#define USE_LED_STRIP
#define WS2811_PIN                      PD12   //TIM4_CH1

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS       6

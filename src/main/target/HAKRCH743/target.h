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

#define TARGET_BOARD_IDENTIFIER "HK743"
#define USBD_PRODUCT_STRING  "HAKRCH743"

#define USE_TARGET_CONFIG

/*** Indicators ***/
#define LED0                    PE3
#define LED1                    PE4
#define BEEPER                  PE9
#define BEEPER_INVERTED

/*** SPI/I2C bus ***/
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_NSS1_PIN           PC15
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PD7

#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PE2
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

#define USE_SPI_DEVICE_4
#define SPI4_NSS_PIN            PC13
#define SPI4_SCK_PIN            PE12
#define SPI4_MISO_PIN           PE13
#define SPI4_MOSI_PIN           PE14

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6    
#define I2C1_SDA                PB7

#define USE_I2C_DEVICE_2
#define I2C2_SCL            PB10
#define I2C2_SDA            PB11 

/*** IMU sensors ***/

#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS
#define USE_SPI
#define USE_IMU_ICM42605

// IMU0 ICM42688P/ICM42605
#define ICM42688_0_CS_PIN          PC15
#define ICM42688_0_SPI_BUS         BUS_SPI1
#define IMU_ICM42688_0_ALIGN       CW0_DEG


// IMU1 ICM42688P/ICM42605
#define ICM42688_1_CS_PIN         PC13
#define ICM42688_1_SPI_BUS        BUS_SPI4
#define IMU_ICM42688_1_ALIGN      CW270_DEG


/*** OSD ***/
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PE2


// *** PINIO ***
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PD10
#define PINIO2_PIN              PD11

/*** Serial ports ***/
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PD6
#define UART2_TX_PIN            PD5

#define USE_UART3
#define UART3_RX_PIN            PD9
#define UART3_TX_PIN            PD8

#define USE_UART4
#define UART4_RX_PIN            PB8
#define UART4_TX_PIN            PB9

#define USE_UART5
#define UART5_RX_PIN            PB12
#define UART5_TX_PIN            PB13

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define USE_UART7
#define UART7_RX_PIN            PE7
#define UART7_TX_PIN            PE8

#define USE_UART8
#define UART8_RX_PIN            PE0
#define UART8_TX_PIN            PE1

#define SERIAL_PORT_COUNT       9      // VCP, UART1, UART2, UART3, UART4, UART5, UART6, UART7, UART8
#define DEFAULT_RX_TYPE                 RX_TYPE_SERIAL
#define SERIALRX_PROVIDER               SERIALRX_SBUS
#define SERIALRX_UART                   SERIAL_PORT_USART7

/*** BARO & MAG ***/
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_SPL06
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C2
#define USE_MAG_ALL

/*** ADC ***/
#define USE_ADC
#define ADC_INSTANCE                    ADC1
#define ADC_CHANNEL_1_PIN               PC0
#define ADC_CHANNEL_2_PIN               PC1
#define ADC_CHANNEL_3_PIN               PC5

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL ADC_CHN_3

/*** LED STRIP ***/
#define USE_LED_STRIP
#define WS2811_PIN                      PA8

// *************** SDIO SD BLACKBOX*******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT


#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

/*** Timer/PWM output ***/
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define MAX_PWM_OUTPUT_PORTS            12
#define USE_DSHOT
#define USE_ESC_SENSOR

/*** Used pins ***/
#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
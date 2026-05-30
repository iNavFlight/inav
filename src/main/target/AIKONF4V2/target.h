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

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_BLACKBOX | FEATURE_SOFTSERIAL)

#define TARGET_BOARD_IDENTIFIER "AIK4"
#define USBD_PRODUCT_STRING     "AIKONF4V2"

// Beeper
#define USE_BEEPER
#define BEEPER                  PB5
#define BEEPER_INVERTED
// Leds
#define USE_LED_STRIP
#define WS2811_PIN              PB6
#define LED0                    PB4
// UARTs
#define USB_IO
#define USE_VCP
#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9
#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3
#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10
#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1
#define SERIAL_PORT_COUNT       5   //VCP, UART1, UART2, UART3, UART4
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1
// SPI
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7
#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15
#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12
// I2C
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9
#define DEFAULT_I2C_BUS         BUS_I2C1
#define EXTERNAL_I2C_BUS        DEFAULT_I2C_BUS
// MAG
#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C_BUS
#define USE_MAG_ALL
// ADC
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC1
#define ADC_CHANNEL_2_PIN               PC2
#define ADC_CHANNEL_3_PIN               PC3
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_1
#define VBAT_ADC_CHANNEL                ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3
#define ADC_INSTANCE ADC1
// Gyro & ACC
#define USE_IMU_MPU6000
#define MPU6000_CS_PIN          SPI1_NSS_PIN
#define MPU6000_SPI_BUS         BUS_SPI1
#define IMU_MPU6000_ALIGN       CW0_DEG
#define USE_IMU_ICM42605
#define ICM42605_CS_PIN         SPI1_NSS_PIN
#define ICM42605_SPI_BUS        BUS_SPI1
#define IMU_ICM42605_ALIGN      CW90_DEG
// OSD
#define USE_MAX7456
#define MAX7456_CS_PIN          SPI3_NSS_PIN
#define MAX7456_SPI_BUS         BUS_SPI3
// Baro
#define USE_BARO
#define BARO_I2C_BUS            DEFAULT_I2C_BUS
#define USE_BARO_ALL
// Blackbox
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define M25P16_CS_PIN           SPI2_NSS_PIN
#define M25P16_SPI_BUS          BUS_SPI2
#define USE_FLASHFS
#define USE_FLASH_M25P16
// Others
#define TEMPERATURE_I2C_BUS     DEFAULT_I2C_BUS
#define PITOT_I2C_BUS           DEFAULT_I2C_BUS
#define RANGEFINDER_I2C_BUS     DEFAULT_I2C_BUS

#define MAX_PWM_OUTPUT_PORTS    4
#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_SERIALSHOT
#define USE_DSHOT
#define USE_ESC_SENSOR
#define VOLTAGE_METER_SCALE 110
#define CURRENT_METER_SCALE 400

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff
#define TARGET_IO_PORTF         0xffff


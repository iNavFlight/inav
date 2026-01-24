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

#define TARGET_BOARD_IDENTIFIER "AK4V3"
#define USBD_PRODUCT_STRING     "AIKONF4V3"

// *************** LED, BEEPER ***************
#define LED0                    PB4
#define BEEPER                  PB5
#define BEEPER_INVERTED

// *************** IMU generic *****************
#define USE_EXTI

// *************** SPI1: Gyro & ACC  *******************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW270_DEG
#define ICM42605_CS_PIN         PA4
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_EXTI_PIN       PC4

#define USE_IMU_ICM42688P
#define IMU_ICM42688P_ALIGN     CW270_DEG
#define ICM42688P_CS_PIN        PA4
#define ICM42688P_SPI_BUS       BUS_SPI1
#define ICM42688P_EXTI_PIN      PC4

// *************** SPI2: Flash **********************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PB12
#define M25P16_SPI_BUS          BUS_SPI2
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** SPI3: OSD ***********************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PA15

// *************** I2C: Baro, Mag, External ****************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define BARO_I2C_BUS            DEFAULT_I2C_BUS
#define USE_BARO_BMP280
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C_BUS
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     DEFAULT_I2C_BUS
#define PITOT_I2C_BUS           DEFAULT_I2C_BUS

// INAV Navigation: Rangefinder support
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define RANGEFINDER_I2C_BUS     DEFAULT_I2C_BUS

// *************** UART *****************************
#define USB_IO
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART3
#define UART3_RX_PIN            PB11
#define UART3_TX_PIN            PB10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PD2
#define UART5_TX_PIN            NONE

#define SERIAL_PORT_COUNT       6   // VCP, UART1, UART2, UART3, UART4, UART5

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1

// Spektrum receiver bind support
#define USE_SPEKTRUM_BIND
#define BIND_PIN                PA10  // UART1_RX

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE            ADC3
#define ADC3_DMA_STREAM         DMA2_Stream1

#define ADC_CHANNEL_1_PIN       PC1
#define ADC_CHANNEL_2_PIN       PC2
#define ADC_CHANNEL_3_PIN       PC3

#define VBAT_ADC_CHANNEL        ADC_CHN_2
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_1
#define RSSI_ADC_CHANNEL        ADC_CHN_3

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PB7     // 10V BEC
#define PINIO2_PIN              PB3     // Camera control

// *************** LED STRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN              PB6

// *************** OTHERS ***************************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_BLACKBOX | FEATURE_TX_PROF_SEL)

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff

#define MAX_PWM_OUTPUT_PORTS    6

#define USE_DSHOT
#define USE_ESC_SENSOR

// BLHeli passthrough for ESC configuration/updates
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

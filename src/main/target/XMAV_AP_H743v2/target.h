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

#define TARGET_BOARD_IDENTIFIER "X743"

#define USBD_PRODUCT_STRING  "AP-H743v2"

// *************** LED **********************
#define LED0                    PD15
#define LED1                    PD11
#define LED2                    PB15

// *************** SPI: Gyro & ACC & OSD **********************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_3
#define USE_SPI_DEVICE_4

#define SPI1_SCK_PIN        PA5
#define SPI1_MISO_PIN   	PA6
#define SPI1_MOSI_PIN   	PA7

#define SPI3_SCK_PIN        PB3
#define SPI3_MISO_PIN   	PB4
#define SPI3_MOSI_PIN   	PB2

#define SPI4_SCK_PIN        PE2
#define SPI4_MISO_PIN   	PE5
#define SPI4_MOSI_PIN   	PE6

#define USE_IMU_BMI088
#define IMU_BMI088_ALIGN        CW90_DEG
#define BMI088_SPI_BUS          BUS_SPI1
#define BMI088_GYRO_CS_PIN      PA3
#define BMI088_ACC_CS_PIN       PA2

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          PA15

// *************** UART *****************************
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
#define UART4_RX_PIN            PB9
#define UART4_TX_PIN            PB8

#define USE_UART5
#define UART5_RX_PIN            PB12
#define UART5_TX_PIN            PB13
#define INVERTER_PIN_UART5_RX   PD14

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6 

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define USE_UART8
#define UART8_RX_PIN            PE0
#define UART8_TX_PIN            PE1

#define SERIAL_PORT_COUNT       9      //VCP, UART1, UART2, UART3, UART4, UART5, UART6, UART7, UART8

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART5

// *************** I2C: BARO & MAG ****************************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define USE_I2C_DEVICE_4
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7
#define I2C4_SCL                PD12
#define I2C4_SDA                PD13

#define USE_BARO
#define USE_BARO_ALL
#define BARO_I2C_BUS            BUS_I2C1

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C4
#define PITOT_I2C_BUS           BUS_I2C4

// *************** ENABLE OPTICAL FLOW & RANGEFINDER *****************************
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

// *************** SDIO SD BLACKBOX*******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_4_PIN           PC4
#define ADC_CHANNEL_5_PIN           PC5
#define VBAT_ADC_CHANNEL            ADC_CHN_4
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_5
#define VBAT_SCALE_DEFAULT          1010
#define CURRENT_METER_SCALE         402

#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_OSD | FEATURE_TELEMETRY)

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS       8

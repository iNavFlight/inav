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

#define TARGET_BOARD_IDENTIFIER "M405"

#define USBD_PRODUCT_STRING  "MICOAIR405V2"

// *************** LED **********************
#define LED0                    PC5
#define LED1                    PC4
#define LED2                    PA8

// *************** SPI: Gyro & ACC & BARO & OSD & SDCARD **********************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_2
#define USE_SPI_DEVICE_3

#define SPI1_SCK_PIN        PA5
#define SPI1_MISO_PIN   	PA6
#define SPI1_MOSI_PIN   	PA7

#define SPI2_SCK_PIN        PB13
#define SPI2_MISO_PIN   	PC2
#define SPI2_MOSI_PIN   	PC3

#define SPI3_SCK_PIN        PC10
#define SPI3_MISO_PIN       PC11
#define SPI3_MOSI_PIN       PC12

#define USE_IMU_BMI088
#define IMU_BMI088_ALIGN        CW270_DEG
#define BMI088_SPI_BUS          BUS_SPI2
#define BMI088_GYRO_CS_PIN      PC14
#define BMI088_ACC_CS_PIN       PC13

#define USE_BARO
#define USE_BARO_SPL06
#define SPL06_SPI_BUS           BUS_SPI2
#define SPL06_CS_PIN            PA4

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PB12

#define USE_BLACKBOX
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI3
#define SDCARD_CS_PIN           PC9
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** UART *****************************
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

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6 
#define INVERTER_PIN_UART6_RX   PC15

#define SERIAL_PORT_COUNT       7      //VCP, UART1, UART2, UART3, UART4, UART5, UART6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART6
#define USE_UART_INVERTER

// *************** I2C ****************************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C_BUS
#define USE_MAG_ALL

// *************** ENABLE OPTICAL FLOW & RANGEFINDER *****************************
#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define VBAT_SCALE_DEFAULT          2121
#define CURRENT_METER_SCALE         402

#define DEFAULT_FEATURES        (FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_OSD | FEATURE_TELEMETRY)

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS       10

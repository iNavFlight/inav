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

#define TARGET_BOARD_IDENTIFIER "ORWG"
#define USBD_PRODUCT_STRING     "ORQAH743WING"

#define USE_TARGET_CONFIG
#define USE_TARGET_HARDWARE_DESCRIPTORS

#define LED0                    PC14
#define LED1                    PC15

// *************** Beeper **********************
#define BEEPER                  PB9
#define BEEPER_PWM_FREQUENCY    2700

// *************** IMU generic ***********************
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS
#define USE_IMU_ICM42605

// *************** SPI1 IMU1 ICM42688 ******************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define IMU1_ALIGN              CW180_DEG
#define IMU1_SPI_BUS            BUS_SPI1
#define IMU1_CS_PIN             PC8

// *************** SPI3 IMU2 ICM42688 ******************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define IMU2_ALIGN              CW0_DEG
#define IMU2_SPI_BUS            BUS_SPI3
#define IMU2_CS_PIN             PE12

// *************** SPI2 OSD ***********************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PD3
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12
#define MAX7456_SPI_SPEED       BUS_SPEED_INITIALIZATION


// *************** SDIO SD BLACKBOX*******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_2
#define SDCARD_SDIO_4BIT
#define SDCARD_SDIO_NORMAL_SPEED
#define SDCARD_SDIO2_CMD_ALT

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** I2C /Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL PB8
#define I2C1_SDA PB7

#define USE_I2C_DEVICE_2
#define I2C2_SCL PB10
#define I2C2_SDA PB11

#define USE_I2C_DEVICE_4
#define I2C4_SCL PD12
#define I2C4_SDA PD13

#define USE_BARO
#define BARO_I2C_BUS BUS_I2C4
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C2
#define PITOT_I2C_BUS           BUS_I2C2

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS BUS_I2C1

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN PB6
#define UART1_RX_PIN PA10

#define USE_UART2
#define UART2_TX_PIN PD5
#define UART2_RX_PIN PD6

#define USE_UART6
#define UART6_TX_PIN PC6
#define UART6_RX_PIN PC7

#define USE_UART7
#define UART7_TX_PIN PE8
#define UART7_RX_PIN PE7

#define USE_UART8
#define UART8_TX_PIN PE1
#define UART8_RX_PIN PE0

#define SERIAL_PORT_COUNT 6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_GHST
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC5  //VBAT1
#define ADC_CHANNEL_2_PIN           PC4  //CURR1
#define ADC_CHANNEL_3_PIN           PC0  //RSSI
#define ADC_CHANNEL_5_PIN           PA3  //VB2
#define ADC_CHANNEL_6_PIN           PA2  //CU2

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PC13  //Camera Switch  
#define PINIO2_PIN                  PE3    
#define PINIO3_PIN                  PD4    
#define PINIO4_PIN                  PE4    

// *************** LEDSTRIP ************************
#define DEFAULT_FEATURES            (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX | FEATURE_GPS)
#define CURRENT_METER_SCALE         220     
#define VBAT_SCALE_DEFAULT          1792     

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS        14
#define USE_DSHOT
#define USE_ESC_SENSOR

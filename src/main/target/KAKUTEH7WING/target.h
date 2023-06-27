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

#define TARGET_BOARD_IDENTIFIER "KH7W"
#define USBD_PRODUCT_STRING     "KAKUTEH7WING"

#define USE_HARDWARE_PREBOOT_SETUP

#define USE_TARGET_CONFIG

#define LED0                    PC15
#define LED1                    PC14

#define BEEPER                  PB9
#define BEEPER_INVERTED

// *************** IMU generic ***********************
// #define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

// *************** SPI1 IMU0 BMI088 ******************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_IMU_BMI088

#define IMU_BMI088_ALIGN        CW0_DEG
#define BMI088_SPI_BUS          BUS_SPI1

#define BMI088_GYRO_CS_PIN      PC9
#define BMI088_GYRO_EXTI_PIN    PD10
#define BMI088_ACC_CS_PIN       PC8
#define BMI088_ACC_EXTI_PIN     PD11

// *************** SPI3 IMU1  ICM42688 ************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN       CW90_DEG
#define ICM42605_SPI_BUS         BUS_SPI3
#define ICM42605_CS_PIN          PE12
#define ICM42605_EXTI_PIN        PE15

// *************** SPI2 OSD ***********************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PD3
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** SDIO SD BLACKBOX*******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_2
#define SDCARD_SDIO_4BIT
#define SDCARD_SDIO_NORMAL_SPPED
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
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06

#define USE_MAG
#define MAG_I2C_BUS BUS_I2C1
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_IST8308
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL
#define USE_MAG_VCM5883

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS BUS_I2C1

// *************** UART *****************************
#define USE_VCP
#define VBUS_SENSING_PIN        PA9
#define VBUS_SENSING_ENABLED

#define USE_UART1
#define UART1_TX_PIN PB6
#define UART1_RX_PIN PA10

#define USE_UART2
#define UART2_TX_PIN PD5
#define UART2_RX_PIN PD6

#define USE_UART3
#define UART3_TX_PIN PD8
#define UART3_RX_PIN PD9

#define USE_UART5
#define UART5_TX_PIN PB13
#define UART5_RX_PIN PD2

#define USE_UART6
#define UART6_TX_PIN PC6
#define UART6_RX_PIN PC7

#define USE_UART7
#define UART7_TX_PIN PE8
#define UART7_RX_PIN PE7

#define USE_UART8
#define UART8_TX_PIN PE1
#define UART8_RX_PIN PE0

#define SERIAL_PORT_COUNT 8

#define DEFAULT_RX_TYPE     RX_TYPE_SERIAL
#define SERIALRX_PROVIDER   SERIALRX_SBUS
#define SERIALRX_UART       SERIAL_PORT_USART6

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
#define PINIO1_PIN                  PC13  // VTX power switcher
#define PINIO2_PIN                  PE3   // 2xCamera switcher
#define PINIO3_PIN                  PD4   // User1
#define PINIO4_PIN                  PE4   // User2

// *************** LEDSTRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN                  PA15

#define DEFAULT_FEATURES            (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)
#define CURRENT_METER_SCALE         3660     // 36.6
#define VBAT_SCALE_DEFAULT          1818     // 18.18
#define VBAT_SCALE_DEFAULT2         1100     // 11.0

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS        14
#define USE_DSHOT
#define USE_ESC_SENSOR

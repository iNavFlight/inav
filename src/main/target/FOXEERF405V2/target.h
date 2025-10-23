/*
 * This file is part of INAV Project.
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

#define TARGET_BOARD_IDENTIFIER "FX42"

#define USBD_PRODUCT_STRING     "FOXEERF405V2"

// Indicators
#define LED0                    PA13
#define LED1                    PA14

#define BEEPER                  PC15
#define BEEPER_INVERTED

// WS2812
#define USE_LED_STRIP
#define WS2811_PIN				PB7

// Gyro & ACC
#define USE_SPI

//Gyro
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

//Flash
#define USE_SPI_DEVICE_2					
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

//OSD
#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PC14
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

// I2C
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

//Gyro sensors
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW270_DEG
#define MPU6000_CS_PIN          SPI1_NSS_PIN
#define MPU6000_SPI_BUS         BUS_SPI1

#define USE_IMU_MPU6500
#define IMU_MPU6500_ALIGN       CW270_DEG
#define MPU6500_CS_PIN          SPI1_NSS_PIN
#define MPU6500_SPI_BUS         BUS_SPI1

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN       CW270_DEG
#define ICM42605_CS_PIN          SPI1_NSS_PIN
#define ICM42605_SPI_BUS         BUS_SPI1


#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          SPI3_NSS_PIN

// Onboard Flash
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS          BUS_SPI2
#define M25P16_CS_PIN           SPI2_NSS_PIN



#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_ALL
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

// Serial ports
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART3
#define UART3_RX_PIN            PC11
#define UART3_TX_PIN            PC10

#define USE_UART4
#define UART4_RX_PIN            PA1
#define UART4_TX_PIN            PA0

#define USE_UART5
#define UART5_RX_PIN            PD2
#define UART5_TX_PIN            PC12

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       7

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF

// ADC
#define USE_ADC
#define ADC_CHANNEL_1_PIN       PC0
#define ADC_CHANNEL_2_PIN       PC1
#define ADC_CHANNEL_3_PIN       PC5

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define CURRENT_METER_SCALE   70
#define RSSI_ADC_CHANNEL            ADC_CHN_3

// SET
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT )
#define USE_DSHOT
#define USE_DSHOT_DMAR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define MAX_PWM_OUTPUT_PORTS    9
#define TARGET_MOTOR_COUNT      8

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff

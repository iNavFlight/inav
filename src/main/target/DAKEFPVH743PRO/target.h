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

#define TARGET_BOARD_IDENTIFIER "DAK7"
#define USBD_PRODUCT_STRING  "DAKEFPV H743PRO"

#define LED0                    PD10
#define LED1                    PD11
#define LED2                    PA8

#define BEEPER                  PC2
#define BEEPER_INVERTED
#define USE_DSHOT_DMAR
// Buses
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN	        PA6
#define SPI1_MOSI_PIN	        PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN	        PB14
#define SPI2_MOSI_PIN	        PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN            PE12
#define SPI4_MISO_PIN           PE13
#define SPI4_MOSI_PIN           PE14

#define USE_I2C
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define DEFAULT_I2C_BUS         BUS_I2C2

// GYRO
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS     // Don't use common busdev descriptors for IMU
#define USE_IMU_MPU6500
#define USE_IMU_MPU6000

#define USE_IMU_ICM42605
#define USE_IMU_BMI270

// IMU_1 is verified to work on OBF4V6 and Omnibus Fireworks board
# define IMU_1_CS_PIN            PA4
# define IMU_1_SPI_BUS           BUS_SPI1
# define IMU_1_ALIGN             CW0_DEG
// IMU_2 is sketchy and was not verified on actual hardware
# define IMU_2_CS_PIN            PB1
# define IMU_2_SPI_BUS           BUS_SPI4
# define IMU_2_ALIGN             CW180_DEG

// M25P256 flash
#define USE_FLASHFS
#define USE_FLASH_M25P16

#define M25P16_SPI_BUS          BUS_SPI3
#define M25P16_CS_PIN           PA15

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// CAMERA_CONTROL
#define CAMERA_CONTROL_PIN      PE5

// Serial ports
#define USE_VCP

#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6
#define USE_UART7
#define USE_UART8
#define UART1_TX_PIN         PA9
#define UART1_RX_PIN         PA10
#define UART2_TX_PIN         PD5
#define UART2_RX_PIN         PD6
#define UART3_TX_PIN         PD8
#define UART3_RX_PIN         PD9
#define UART4_TX_PIN         PB8
#define UART4_RX_PIN         PB9
#define UART5_TX_PIN         PB6
#define UART5_RX_PIN         PB5
#define UART6_TX_PIN         PC6
#define UART6_RX_PIN         PC7
#define UART7_TX_PIN         PE8
#define UART7_RX_PIN         PE7
#define UART8_TX_PIN         PE1
#define UART8_RX_PIN         PE0

#define SERIAL_PORT_COUNT       9

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART5
#define GPS_UART                SERIAL_PORT_USART1

//Baro 
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_DPS310
#define BARO_I2C_BUS            DEFAULT_I2C_BUS

// Mag
#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C_BUS
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     DEFAULT_I2C_BUS

#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define RANGEFINDER_I2C_BUS     DEFAULT_I2C_BUS

// unkonw
#define PITOT_I2C_BUS           DEFAULT_I2C_BUS

// ADC
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC_CHANNEL_1_PIN           PC1
#define ADC_CHANNEL_2_PIN           PC5
#define ADC_CHANNEL_3_PIN           PC0
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define RSSI_ADC_CHANNEL            ADC_CHN_2
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_3

#define USE_LED_STRIP
#define WS2811_PIN                  PB0

// unkonw
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_LED_STRIP | FEATURE_GPS)

// unkonw
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define USE_DSHOT
#define USE_ESC_SENSOR

#define MAX_PWM_OUTPUT_PORTS    12

// PINIO
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN           PE2
#define PINIO2_PIN           PE3
#define PINIO3_PIN           PE4

// VBAT 10K/160K
#define VBAT_SCALE_DEFAULT 1600

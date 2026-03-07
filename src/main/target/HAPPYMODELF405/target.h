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

#define TARGET_BOARD_IDENTIFIER "HMF4"
#define USBD_PRODUCT_STRING  "HAPPYMODEL F405"

#define LED0                    PB5

#define BEEPER                  PB4
#define BEEPER_INVERTED

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
#define SPI3_MISO_PIN	        PC11
#define SPI3_MOSI_PIN	        PC12

#define USE_I2C
#define USE_I2C_DEVICE_2
#define I2C1_SCL                PB10
#define I2C1_SDA                PB11


// Gyro
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW0_DEG
#define MPU6000_CS_PIN          PA4
#define MPU6000_SPI_BUS         BUS_SPI1

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN               CW0_DEG
#define ICM42605_SPI_BUS                 BUS_SPI1
#define ICM42605_CS_PIN                  PA4

#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN      CW0_DEG
#define BMI270_CS_PIN         PA4
#define BMI270_SPI_BUS        BUS_SPI1

//Baro 
#define USE_BARO
#define BARO_I2C_BUS             BUS_I2C2
#define USE_BARO_BMP280
#define USE_BARO_SPL06

// M25P16 flash
#define USE_FLASHFS
#define USE_FLASH_M25P16

#define M25P16_SPI_BUS          BUS_SPI3
#define M25P16_CS_PIN           PA15

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// Serial ports
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PA10
#define UART1_TX_PIN            PA9

#define USE_UART2
#define UART2_RX_PIN            PA3
#define UART2_TX_PIN            PA2

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define SERIAL_PORT_COUNT       4

// Mag
#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C2
#define USE_MAG_ALL

// ADC
#define USE_ADC
#define ADC_CHANNEL_1_PIN           PC1
#define ADC_CHANNEL_2_PIN           PC2
#define VBAT_ADC_CHANNEL            ADC_CHN_2
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_1

#define CURRENT_METER_SCALE  470

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY )

#define USE_LED_STRIP
#define WS2811_PIN                  PB6

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff

#define USE_DSHOT
#define USE_ESC_SENSOR

#define MAX_PWM_OUTPUT_PORTS    4

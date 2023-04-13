/*
 * This file is part of INAV Project.
 *
 * INAV Project is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV Project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV Project.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER "IB7A"

#define USBD_PRODUCT_STRING     "IFLIGHT_BLITZ_F7_AIO"

#define LED0                    PD15

#define BEEPER                  PD3
#define BEEPER_INVERTED

// *************** IMU generic ***********************
//#define USE_DUAL_GYRO
//#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

// ******************* SPI *************************
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PD6

#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN            PE2
#define SPI4_MISO_PIN           PE5
#define SPI4_MOSI_PIN           PE6


// *************** Gyro & ACC **********************

#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN        CW0_DEG
#define BMI270_SPI_BUS          BUS_SPI1
#define BMI270_CS_PIN           PA4

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PA4

// *************** I2C/Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define USE_I2C_DEVICE_4
#define I2C2_SCL                PD12
#define I2C2_SDA                PD13

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C4
#define USE_BARO_BMP280
#define USE_BARO_DPS310

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_IST8308
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1

// *************** Blackbox **************************

#define USE_FLASHFS
#define USE_FLASH_M25P16

#define M25P16_SPI_BUS          BUS_SPI3
#define M25P16_CS_PIN           PA15
//#define M25P16_SPI_SHARED

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** OSD *****************************

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI4
#define MAX7456_CS_PIN          PE4

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
#define UART5_TX_PIN            PC12

#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6

#define USE_UART7
#define UART7_RX_PIN            PE7
#define UART7_TX_PIN            PE8

#define USE_UART8
#define UART8_RX_PIN            PE0
#define UART8_TX_PIN            PE1

#define SERIAL_PORT_COUNT       9

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART3

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PC3
#define ADC_CHANNEL_2_PIN           PC2
#define ADC_CHANNEL_3_PIN           PC5
#define ADC_CHANNEL_4_PIN           PC1
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3
#define AIRSPEED_ADC_CHANNEL        ADC_CHN_4
#define CURRENT_METER_SCALE         200

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS         BUS_I2C1

#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_OSD | FEATURE_BLACKBOX | FEATURE_LED_STRIP )

#define USE_LED_STRIP
#define WS2811_PIN              PC9

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define MAX_PWM_OUTPUT_PORTS    8
#define USE_DSHOT
#define USE_ESC_SENSOR

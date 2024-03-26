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

#ifdef ZEEZF7V3
#define TARGET_BOARD_IDENTIFIER "ZEF7V3"
#define USBD_PRODUCT_STRING     "ZEEZF7V3"
#endif
#ifdef ZEEZF7V2
#define TARGET_BOARD_IDENTIFIER "ZEF7V2"
#define USBD_PRODUCT_STRING     "ZEEZF7V2"
#endif
#ifdef ZEEZF7
#define TARGET_BOARD_IDENTIFIER "ZEF7"
#define USBD_PRODUCT_STRING     "ZEEZF7"
#endif

#define USE_TARGET_CONFIG

#define LED0                    PC14
#define LED1                    PC15

#ifdef ZEEZF7V3
#define BEEPER                  PC4
#define BEEPER_INVERTED
#else
#define BEEPER                  PB2
#define BEEPER_INVERTED
#endif

// *************** Gyro & ACC **********************
#define USE_SPI

#ifdef ZEEZF7V3

#define USE_SPI_DEVICE_3

#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PB5

// *************** SPI3 IMU0 ICM42688P **************
#define USE_IMU_ICM42605

#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_SPI_BUS        BUS_SPI3
#define ICM42605_CS_PIN         SPI3_NSS_PIN

// *************** SPI3 IMU0 BMI270 **************
#define USE_IMU_BMI270

#define IMU_BMI270_ALIGN      CW270_DEG
#define BMI270_SPI_BUS        BUS_SPI3
#define BMI270_CS_PIN         SPI3_NSS_PIN
#endif

#ifdef ZEEZF7V2
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define MPU6000_CS_PIN          SPI1_NSS_PIN
#define MPU6000_SPI_BUS         BUS_SPI1
#define IMU_MPU6000_ALIGN       CW0_DEG
#define USE_IMU_MPU6000
#endif


#ifdef ZEEZF7
#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define MPU6000_CS_PIN          SPI2_NSS_PIN
#define MPU6000_SPI_BUS         BUS_SPI2

#define IMU_MPU6000_ALIGN       CW0_DEG_FLIP
#define USE_IMU_MPU6000
#endif

// *************** I2C/Baro/Mag *********************

#ifdef ZEEZF7
// Target has no I2C but can use MSP devices, such as those provided on Mateksys MQ8-CAN/MSP
// Which contains: GPS SAM-M8Q, Compass QMC5883L, Barometer DPS310
// See: http://www.mateksys.com/?portfolio=m8q-can
#define USE_BARO
#define USE_BARO_DPS310

#define USE_MAG
#define USE_MAG_QMC5883
#endif

#if defined ZEEZF7V2 ||  defined ZEEZF7V3
#define USE_I2C
#define USE_BARO
#define USE_BARO_BMP280
#define USE_BARO_BMP388
#define USE_BARO_DPS310

#define BARO_I2C_BUS            BUS_I2C1

#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

// External I2C Pads -- I2C3
#define USE_I2C_DEVICE_3
#define I2C3_SCL                PA8
#define I2C3_SDA                PC9

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C3
#define USE_MAG_ALL
#endif

// *************** Flash ****************************

#if defined ZEEZF7V2 ||  defined ZEEZF7V3
#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define M25P16_SPI_BUS          BUS_SPI2
#define M25P16_CS_PIN           SPI2_NSS_PIN

#define W25N01G_SPI_BUS         BUS_SPI2
#define W25N01G_CS_PIN          SPI2_NSS_PIN
#ifdef ZEEZF7V3
#define SDCARD_SPI_BUS          BUS_SPI2
#define SDCARD_CS_PIN           SPI2_NSS_PIN

#define USE_SDCARD
#define USE_SDCARD_SPI
#endif

#endif

#ifdef ZEEZF7
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define M25P16_SPI_BUS          BUS_SPI1
#define M25P16_CS_PIN           SPI1_NSS_PIN

#define W25N01G_SPI_BUS         BUS_SPI1
#define W25N01G_CS_PIN          SPI1_NSS_PIN

#endif

#define USE_FLASHFS
#define USE_FLASH_M25P16
#define USE_FLASH_W25N01G
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** OSD *****************************
#define USE_MAX7456

#if defined ZEEZF7 ||  defined ZEEZF7V2
#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PB5

#define MAX7456_SPI_BUS         BUS_SPI3
#define MAX7456_CS_PIN          SPI3_NSS_PIN
#else
#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN            PA4
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7   

#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          SPI1_NSS_PIN
#endif

// *************** PINIO ***************************

#ifdef ZEEZF7
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PB11 // VTX power switcher
#endif

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

#if defined ZEEZF7V2 || defined ZEEZF7V3
#define SERIAL_PORT_COUNT       6
#else
#define USE_UART6
#define UART6_RX_PIN            PC7
#define UART6_TX_PIN            PC6
#define SERIAL_PORT_COUNT       7
#endif

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART4

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0

#if defined ZEEZF7V2 || defined ZEEZF7V3
#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#else
#define ADC_CHANNEL_1_PIN           PC2
#define ADC_CHANNEL_2_PIN           PC3
#endif

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2

#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_OSD | FEATURE_LED_STRIP)

#if defined ZEEZF7V2 ||  defined ZEEZF7V3
#define CURRENT_METER_SCALE     250
#endif

// ********** Optical Flow and Lidar **************

#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define USE_OPFLOW
#define USE_OPFLOW_MSP

// *************** LED *****************************

#define USE_LED_STRIP
#define WS2811_PIN              PB0

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#if defined ZEEZF7V2 ||  defined ZEEZF7V3
#define MAX_PWM_OUTPUT_PORTS    8
#else
#define MAX_PWM_OUTPUT_PORTS    4
#endif

#define USE_DSHOT
#define USE_ESC_SENSOR

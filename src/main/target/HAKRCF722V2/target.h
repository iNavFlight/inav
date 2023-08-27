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

#define TARGET_BOARD_IDENTIFIER "HK7V2"
#define USBD_PRODUCT_STRING  "HAKRCF722V2"

#define USE_TARGET_CONFIG

/*** Indicators ***/
#define LED0                    PC14
#define LED1                    PC15

#define BEEPER                  PC13
#define BEEPER_INVERTED

// *** SPI & I2C ***
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_NSS1_PIN           PA4
#define SPI1_NSS2_PIN           PB2
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6 
#define SPI1_MOSI_PIN           PA7

#define USE_SPI_DEVICE_2 
#define SPI2_NSS_PIN            PB12
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_SPI_DEVICE_3
#define SPI3_NSS_PIN            PA15
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PB5

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

// *** IMU sensors ***

#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS
#define USE_DUAL_GYRO

#define GYRO_SPI_BUS         BUS_SPI1
#define GYRO1_CS_PIN          SPI1_NSS1_PIN
#define GYRO2_CS_PIN          SPI1_NSS2_PIN

// MPU6000
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW90_DEG

// ICM42605/ICM42688P
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN       CW90_DEG

//BMI270
#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN        CW90_DEG

// *** SPI2 OSD ***
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          SPI2_NSS_PIN

// *** SPI3 FLASH ***
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           SPI3_NSS_PIN
#define M25P16_SPI_BUS          BUS_SPI3

// *** UART ***
#define USE_VCP

#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6

#define UART1_TX_PIN            PB6
#define UART1_RX_PIN            PB7

#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define UART3_TX_PIN            NONE
#define UART3_RX_PIN            PB11

#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PA1

#define UART5_TX_PIN            PC12
#define UART5_RX_PIN            PD2

#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define SERIAL_PORT_COUNT       7


#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART2

// *** I2C/Baro/Mag/Pitot ***
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_SPL06
#define USE_BARO_DPS310

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define BNO055_I2C_BUS          BUS_I2C1

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL

#define PITOT_I2C_BUS           BUS_I2C1

// *** PINIO ***
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PC0
#define PINIO2_PIN              PC2

// *** ADC ***
#define USE_ADC
//#define ADC_INSTANCE                ADC3
//#define ADC1_DMA_STREAM             DMA2_Stream0
#define ADC_CHANNEL_1_PIN           PC1
#define ADC_CHANNEL_2_PIN           PC3
#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2


// *** LED2812 ***
#define USE_LED_STRIP
#define WS2811_PIN                      PB3

// *** OTHERS ***
#define DEFAULT_FEATURES        (FEATURE_TX_PROF_SEL | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_SOFTSERIAL )


#define USE_DSHOT
#define USE_ESC_SENSOR

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))

#define MAX_PWM_OUTPUT_PORTS    8

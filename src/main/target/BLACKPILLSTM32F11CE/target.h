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

#define TARGET_BOARD_IDENTIFIER "ST41"
#define USBD_PRODUCT_STRING     "STM32F411CE"

#define LED0                    PC13
#define BEEPER                  PC14
#define BEEPER_INVERTED

// *************** SPI **********************
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN           PA5
#define SPI1_MISO_PIN          PA6
#define SPI1_MOSI_PIN          PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN           PB13
#define SPI2_MISO_PIN          PB14
#define SPI2_MOSI_PIN          PB15


// *************** SPI Gyro & ACC ********************** 
// SPI1 için PA4 pini (slave chip select pin) CS pinidir. SPI1_NSS pin
// SPI2 için PB12 pini (slave chip select pin) CS pinidir. SPI2_NSS pin

// SDA/SDI: connect to board MOSI.
// SCL/SCLK: connect to board SCK.
// AD0/SDO: connect to board MISO.
// nCS: connect to board chip select pin NSS
// https://arduinka.biz.ua/image/cache/catalog/gy-6500-2-500x500.jpg

#define USE_IMU_MPU9250
#define MPU9250_SPI_BUS         BUS_SPI2
#define MPU9250_CS_PIN          PB12
#define IMU_MPU9250_ALIGN       CW0_DEG

#define USE_IMU_MPU6000
#define MPU6000_SPI_BUS         BUS_SPI2
#define MPU6000_CS_PIN          PB12
#define IMU_MPU6000_ALIGN       CW0_DEG

#define USE_IMU_MPU6500
#define MPU6500_SPI_BUS         BUS_SPI2
#define MPU6500_CS_PIN          PB12
#define IMU_MPU6500_ALIGN       CW0_DEG

// *************** Gyro/Acel/Baro/Mag *****************************

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define USE_IMU_MPU6050
#define IMU_MPU6050_ALIGN       CW0_DEG
#define MPU6050_I2C_BUS         BUS_I2C1

#define USE_EXTI
#define GYRO_INT_EXTI           PA14
// #define USE_MPU_DATA_READY_SIGNAL
#define USE_MAG_DATA_READY_SIGNAL
#define ENSURE_MAG_DATA_READY_IS_HIGH

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_BMP085
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_SPL06

#define BNO055_I2C_BUS          BUS_I2C1

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_IST8310
#define USE_MAG_IST8308
#define USE_MAG_MAG3110
#define USE_MAG_LIS3MDL

#define MAG_HMC5883_ALIGN CW0_DEG

// *************** SPI OSD *****************************
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** SPI FLASH **************************
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_CS_PIN           PA4
#define M25P16_SPI_BUS          BUS_SPI1
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN     PB6     // Clash with TX2, possible to use as S.Port or VTX control
#define SOFTSERIAL_1_RX_PIN     PB7

#define SERIAL_PORT_COUNT       4       // VCP, USART1, USART2, SS1

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define SERIALRX_UART           SERIAL_PORT_USART1


// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                    ADC1
#define ADC_CHANNEL_1_PIN               PA1
#define ADC_CHANNEL_2_PIN               PB1
#define ADC_CHANNEL_3_PIN               PB0

#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_1
#define VBAT_ADC_CHANNEL                ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

// *************** LED2812 ************************

#define USE_LED_STRIP
#define WS2811_PIN                      PB5

// ***************  OTHERS FEATURE_OSD*************************
#define DEFAULT_FEATURES (FEATURE_TX_PROF_SEL | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_SOFTSERIAL )

#define VBAT_SCALE_DEFAULT      1100
#define CURRENT_METER_SCALE     250

#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD        (BIT(2))


#define MAX_PWM_OUTPUT_PORTS       6

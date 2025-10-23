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

#define TARGET_BOARD_IDENTIFIER "H743"

#if defined(MATEKH743HD)
  #define USBD_PRODUCT_STRING     "MATEKH743HD"
#else
  #define USBD_PRODUCT_STRING     "MATEKH743"
#endif

#define USE_TARGET_CONFIG

#define LED0                    PE3
#define LED1                    PE4

#define BEEPER                  PA15
#define BEEPER_INVERTED
#define BEEPER_PWM_FREQUENCY    2500

// *************** IMU generic ***********************
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS



// *************** SPI1 IMU0 MPU6000 ****************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PD7

#define USE_IMU_MPU6000

#define IMU_MPU6000_ALIGN       CW0_DEG_FLIP
#define MPU6000_SPI_BUS          BUS_SPI1
#define MPU6000_CS_PIN          PC15

// *************** SPI4 IMU1  ICM20602 **************
#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN            PE12
#define SPI4_MISO_PIN           PE13
#define SPI4_MOSI_PIN           PE14

#define USE_IMU_MPU6500

#define IMU_MPU6500_ALIGN       CW0_DEG_FLIP
#define MPU6500_SPI_BUS         BUS_SPI4
#define MPU6500_CS_PIN          PE11

// *************** SPI4 IMU2 ICM42605 **************
#define USE_IMU_ICM42605

#define IMU_ICM42605_ALIGN      CW90_DEG_FLIP
#define ICM42605_SPI_BUS        BUS_SPI4
#define ICM42605_CS_PIN         PC13

// *************** SPI2 OSD ***********************
  #define USE_SPI_DEVICE_2
  #define SPI2_SCK_PIN            PB13
  #define SPI2_MISO_PIN           PB14
  #define SPI2_MOSI_PIN           PB15

#if defined(MATEKH743)
  #define USE_MAX7456
  #define MAX7456_SPI_BUS         BUS_SPI2
  #define MAX7456_CS_PIN          PB12
#endif

// *************** SPI3 SPARE for external RM3100 ***********
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5

#define USE_MAG_RM3100
#define RM3100_CS_PIN           PE2   //CS2 pad
//                              PD4   //CS1 pad
#define RM3100_SPI_BUS          BUS_SPI3

// *************** I2C /Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7

#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C2
#define PITOT_I2C_BUS           BUS_I2C2

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     BUS_I2C1

// *************** UART *****************************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PD5
#define UART2_RX_PIN            PD6

#define USE_UART3
#define UART3_TX_PIN            PD8
#define UART3_RX_PIN            PD9

#define USE_UART4
#define UART4_TX_PIN            PB9
#define UART4_RX_PIN            PB8

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define USE_UART8
#define UART8_TX_PIN            PE1
#define UART8_RX_PIN            PE0

#define USE_SOFTSERIAL1
#define SOFTSERIAL_1_TX_PIN      PC6  //TX6 pad
#define SOFTSERIAL_1_RX_PIN      PC6  //TX6 pad

#define SERIAL_PORT_COUNT       9

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART6

// *************** SDIO SD BLACKBOX*******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC0  //ADC123 VBAT1
#define ADC_CHANNEL_2_PIN           PC1  //ADC123 CURR1
#define ADC_CHANNEL_3_PIN           PC5  //ADC12  RSSI
#define ADC_CHANNEL_4_PIN           PC4  //ADC12  AirS
#define ADC_CHANNEL_5_PIN           PA4  //ADC12  VB2
#define ADC_CHANNEL_6_PIN           PA7  //ADC12  CU2

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3
#define AIRSPEED_ADC_CHANNEL        ADC_CHN_4

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PD10  // VTX power switcher
#define PINIO2_PIN                  PD11  // 2xCamera switcher

// *************** LEDSTRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN                  PA8

#define DEFAULT_FEATURES            (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)
#define CURRENT_METER_SCALE         250

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA (0xffff & ~(BIT(14) | BIT(13)))
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS        15
#define USE_DSHOT
#define USE_ESC_SENSOR


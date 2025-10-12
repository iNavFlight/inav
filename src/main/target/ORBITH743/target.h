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

#define TARGET_BOARD_IDENTIFIER "ORB7"
#define USBD_PRODUCT_STRING     "ORBITH743"

#define USE_TARGET_CONFIG

#define LED0                    PE3
#define LED1                    PE4

#define BEEPER                  PB7
#define BEEPER_INVERTED


// *************** SPI *****************************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define USE_SPI_DEVICE_2
#define USE_SPI_DEVICE_3
#define USE_SPI_DEVICE_4

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

#define SPI2_SCK_PIN            PB10
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

#define SPI4_SCK_PIN            PE12
#define SPI4_MISO_PIN           PE13
#define SPI4_MOSI_PIN           PE14

// *************** I2C **************************
#define USE_I2C
#define USE_I2C_DEVICE_1

#define I2C1_SCL PB8
#define I2C1_SDA PB9

// *************** UART *****************************
#define USE_VCP
#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6
#define USE_UART7
#define USE_UART8

#define UART1_TX_PIN PB14
#define UART1_RX_PIN PA10

#define UART2_TX_PIN PD5
#define UART2_RX_PIN PD6

#define UART3_TX_PIN PD8
#define UART3_RX_PIN PD9

#define UART4_TX_PIN PD1
#define UART4_RX_PIN PD0

#define UART5_TX_PIN PB13
#define UART5_RX_PIN PB12

#define UART6_TX_PIN PC6
#define UART6_RX_PIN PC7

#define UART7_TX_PIN PE8
#define UART7_RX_PIN PE7

#define UART8_TX_PIN PE1
#define UART8_RX_PIN PE0

#define SERIAL_PORT_COUNT 9 //VCP, UART1, UART2, UART3, UART4, UART5, UART6, UART7, UART8

#define DEFAULT_RX_TYPE     RX_TYPE_SERIAL
#define SERIALRX_PROVIDER   SERIALRX_CRSF
#define SERIALRX_UART       SERIAL_PORT_USART5


// *************** Gyro & ACC **********************
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

#define USE_IMU_ICM42605

#define IMU_ICM42688_ALIGN_1      CW270_DEG
#define ICM42688_CS_PIN_1         PE11
#define ICM42688_SPI_BUS_1        BUS_SPI1
#define ICM42688_EXTI_PIN_1       PE10

#define IMU_ICM42688_ALIGN_2      CW270_DEG
#define ICM42688_CS_PIN_2         PE9
#define ICM42688_SPI_BUS_2        BUS_SPI4
#define ICM42688_EXTI_PIN_2       PB2


// *************** OSD *****************************
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PE6

// *************** FLASH ***************************
#define W25N01G_SPI_BUS BUS_SPI3
#define W25N01G_CS_PIN  PD3

#define USE_BLACKBOX
#define USE_FLASHFS
#define USE_FLASH_W25N01G
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** Baro/Mag *********************

#define USE_BARO
#define BARO_I2C_BUS BUS_I2C1
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_DPS310
#define USE_BARO_SPL06

//#define DPS310_I2C_ADDR 0x77 // 0x77 is for test board
#define DPS310_I2C_ADDR 0x76

#define USE_MAG
#define MAG_I2C_BUS BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS BUS_I2C1


// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC0  //ADC123 VBAT1
#define ADC_CHANNEL_2_PIN           PC1  //ADC123 CURR1
#define ADC_CHANNEL_3_PIN           PC5  //ADC12  RSSI
#define ADC_CHANNEL_4_PIN           PC4  //ADC12  VBAT2
#define ADC_CHANNEL_5_PIN           PA4  //ADC12  CURR2

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PE2  // VTX power switcher
#define PINIO2_PIN                  PD2  // 2xCamera switcher
#define PINIO1_FLAGS				PINIO_FLAGS_INVERTED
#define PINIO2_FLAGS				PINIO_FLAGS_INVERTED

// *************** LEDSTRIP ************************
#define USE_LED_STRIP
#define WS2811_PIN                  PA8

#define DEFAULT_FEATURES            (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)
#define CURRENT_METER_SCALE         125
#define VBAT_SCALE_DEFAULT          1010


#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff
#define TARGET_IO_PORTE 0xffff

#define MAX_PWM_OUTPUT_PORTS        15
#define USE_DSHOT
#define USE_ESC_SENSOR

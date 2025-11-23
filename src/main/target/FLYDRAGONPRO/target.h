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

#define TARGET_BOARD_IDENTIFIER "RTFL"

#define USBD_PRODUCT_STRING  "FLYDRAGONPRO"

// indicator led on this board is a single WS2812B LED
// no traditional indicator leds
#define LED0                    NONE
#define LED1                    NONE
#define USE_LED_STRIP
#define WS2811_PIN              PB8

#define BEEPER                  PA8
#define BEEPER_INVERTED

#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

// ICM42605 variant
#define USE_IMU_ICM42605 // is actually ICM42688P
#define IMU_ICM42605_ALIGN     CW0_DEG
#define ICM42605_CS_PIN        PB0
#define ICM42605_EXTI_PIN      PB3
#define ICM42605_SPI_BUS       BUS_SPI1

// MPU6000 variant
#define USE_IMU_MPU6000
#define IMU_MPU6000_ALIGN       CW90_DEG
#define MPU6000_CS_PIN          PB0
#define MPU6000_SPI_BUS         BUS_SPI1


// *************** I2C /Baro/Mag *********************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7
#define DEFAULT_I2C BUS_I2C1

#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_SPL06
#define SPL06_I2C_ADDR 118

#define TEMPERATURE_I2C_BUS     DEFAULT_I2C

#define PITOT_I2C_BUS           DEFAULT_I2C

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     DEFAULT_I2C

// *************** SPI2 Blackbox *******************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_FLASHFS
#define USE_FLASH_W25N01G
#define W25N01G_SPI_BUS          BUS_SPI2
#define W25N01G_CS_PIN           PB12
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** UART *****************************
#define USE_VCP

#define USE_UART1                   // internal ELRS receiver
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2  // pin labelled as "TX2"
#define UART2_RX_PIN            PA3  // pin labelled as "RX2"

#define USE_UART3
#define UART3_TX_PIN            NONE
#define UART3_RX_PIN            PC11 // pin labelled "RX3" on the "DSM" port

#define USE_UART4
#define UART4_TX_PIN            PA0  // pin labelled "ESC"
#define UART4_RX_PIN            PA1  // pin labelled "RPM"

#define USE_UART5
#define UART5_TX_PIN            PC12 // pin labelled "TX5" on the "GPS" port
#define UART5_RX_PIN            PD2  // pin labelled "RX5" on the "GPS" port

#define SERIAL_PORT_COUNT       6

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART1
#define GPS_UART                SERIAL_PORT_USART5

#define SENSORS_SET (SENSOR_ACC|SENSOR_BARO)

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1
#define ADC1_DMA_STREAM             DMA2_Stream0

#define ADC_CHANNEL_1_PIN           PC2
#define ADC_CHANNEL_2_PIN           PC1
#define ADC_CHANNEL_3_PIN           PC0

#define VBAT_ADC_CHANNEL            ADC_CHN_3 // pin labelled "BAT+" on the "EXT" port
//BEC ADC is ADC_CHN_2
//BUS ADC is ADC_CHN_1

#define VBAT_SCALE_DEFAULT          1898

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PA15 // enable pin for internal ELRS receiver
#define PINIO1_FLAGS				PINIO_FLAGS_INVERTED // turn on by default

#define DEFAULT_FEATURES                (FEATURE_TELEMETRY | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_LED_STRIP )

#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA 0xffff
#define TARGET_IO_PORTB 0xffff
#define TARGET_IO_PORTC 0xffff
#define TARGET_IO_PORTD 0xffff

#define MAX_PWM_OUTPUT_PORTS        9

#define USE_DSHOT
#define USE_SERIALSHOT
#define USE_ESC_SENSOR
#define USE_SMARTPORT_MASTER // no internal current sensor, enable SMARTPORT_MASTER so external ones can be used

#define USE_DSHOT_DMAR
#define TARGET_MOTOR_COUNT          8 // more than 8 DSHOT motors crashes the FC

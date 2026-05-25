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

#define TARGET_BOARD_IDENTIFIER "AEDH"
#define USBD_PRODUCT_STRING     "AEDROXH7"

#define USE_TARGET_CONFIG

// *************** LEDs ***************************
#define LED0                    PE5
#define LED1                    PE2

// *************** Beeper *************************
#define BEEPER                  PA7
#define BEEPER_INVERTED
#define BEEPER_PWM_FREQUENCY    2500

// *************** SPI1 - OSD (MAX7456) ***********
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PB3
#define SPI1_MISO_PIN           PB4
#define SPI1_MOSI_PIN           PB5

#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PE4

// *************** SPI2 - Gyro (ICM42688P) ********
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PB14
#define SPI2_MOSI_PIN           PB15

#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW90_DEG
#define ICM42605_SPI_BUS        BUS_SPI2
#define ICM42605_CS_PIN         PB12
#define ICM42605_EXTI_PIN       PC4

// *************** SPI3 - Flash (W25N01G) *********
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PB2
#define SPI3_NSS_PIN            PA15
// PB2 requires AF7 for SPI3 MOSI — AF6 is the default and incorrect for this pin
#define SPI3_SCK_AF             GPIO_AF6_SPI3
#define SPI3_MISO_AF            GPIO_AF6_SPI3
#define SPI3_MOSI_AF            GPIO_AF7_SPI3

#define USE_BLACKBOX
#define USE_FLASHFS
#define USE_FLASH_W25N01G
#define W25N01G_SPI_BUS         BUS_SPI3
#define W25N01G_CS_PIN          PA15
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// *************** I2C1 - Magnetometer ************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB6
#define I2C1_SDA                PB7

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

// *************** I2C2 - Barometer (DPS310) ******
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C2
#define PITOT_I2C_BUS           BUS_I2C1
#define RANGEFINDER_I2C_BUS     BUS_I2C1
#define USE_RANGEFINDER

// *************** UARTs **************************
// UART5 not wired; UART6 pins PC6/PC7 used as motor outputs
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

// *************** CAN Bus *************************
// TODO: verify CAN syntax for INAV and confirm transceiver standby pin (if any)
#define USE_DRONECAN
#define CAN1_RX                 PD0
#define CAN1_TX                 PD1

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define USE_UART8
#define UART8_TX_PIN            PE1
#define UART8_RX_PIN            PE0

#define SERIAL_PORT_COUNT       6   // VCP + UART1-3 + UART7 + UART8

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART3

// *************** ADC ****************************
#define USE_ADC
#define ADC_INSTANCE            ADC1

#define ADC_CHANNEL_1_PIN       PC0
#define ADC_CHANNEL_2_PIN       PC1
#define ADC_CHANNEL_3_PIN       PC5

#define VBAT_ADC_CHANNEL        ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL ADC_CHN_2
#define RSSI_ADC_CHANNEL        ADC_CHN_3

// *************** LED Strip **********************
#define USE_LED_STRIP
#define WS2811_PIN              PA5

// *************** PINIO **************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PA2
#define PINIO2_PIN              PA3
#define PINIO3_PIN              PB1
#define PINIO3_FLAGS            PINIO_FLAGS_INVERTED
#define PINIO4_PIN              PD15

// *************** Features ***********************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)
#define CURRENT_METER_SCALE     250

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_ESC_SENSOR

// *************** IO Port Masks ******************
#define TARGET_IO_PORTA         (0xffff & ~(BIT(13) | BIT(14)))
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

// *************** PWM Outputs ********************
#define MAX_PWM_OUTPUT_PORTS    8
#define USE_DSHOT

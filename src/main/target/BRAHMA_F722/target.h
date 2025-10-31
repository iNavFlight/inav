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

#define USE_TARGET_CONFIG
#define TARGET_BOARD_IDENTIFIER         "DMF7"
#define USBD_PRODUCT_STRING             "BRAHMA_F722"


// ******** Board LEDs  **********************
#define LED0                            PC14


// ******* Beeper ***********
#define BEEPER                          PB2
#define BEEPER_INVERTED


#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PC13

// *************** Baro **************************
#define USE_I2C

#define USE_I2C_DEVICE_1
#define I2C1_SCL                        PB8
#define I2C1_SDA                        PB9
#define DEFAULT_I2C_BUS                 BUS_I2C1

#define USE_BARO
#define BARO_I2C_BUS                    DEFAULT_I2C_BUS 
#define DPS310_I2C_ADDR                 0x77
#define USE_BARO_DPS310

//*********** Magnetometer / Compass *************
#define USE_MAG
#define MAG_I2C_BUS                     DEFAULT_I2C_BUS
#define USE_MAG_ALL

// ******* SERIAL ********
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN                    PB6
#define UART1_RX_PIN                    PB7

#define USE_UART2
#define UART2_TX_PIN                    PA2
#define UART2_RX_PIN                    PA3

#define USE_UART3
#define UART3_TX_PIN                    PB10
#define UART3_RX_PIN                    PB11

#define USE_UART4
#define UART4_TX_PIN                    PA0
#define UART4_RX_PIN                    PA1

#define USE_UART5
#define UART5_TX_PIN                    PC12
#define UART5_RX_PIN                    PD2

#define USE_UART6
#define UART6_TX_PIN                    PC6
#define UART6_RX_PIN                    PC7

#define SERIAL_PORT_COUNT               7


// ******* SPI ********
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN                    PA5
#define SPI1_MISO_PIN                   PA6
#define SPI1_MOSI_PIN                   PA7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN                    PB13
#define SPI2_MISO_PIN                   PB14
#define SPI2_MOSI_PIN                   PB15

#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN                    PC10
#define SPI3_MISO_PIN                   PC11
#define SPI3_MOSI_PIN                   PB5

// ******* ADC ********
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC1
#define ADC_CHANNEL_2_PIN               PC3
#define ADC_CHANNEL_3_PIN               PC2
#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

#define VBAT_SCALE_DEFAULT              1100
#define CURRENT_METER_SCALE             183

// ******* OSD ********
#define USE_MAX7456
#define MAX7456_SPI_BUS                 BUS_SPI2
#define MAX7456_CS_PIN                  PB12

//******* FLASH ********
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS                  BUS_SPI3
#define M25P16_CS_PIN                   PA13

#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

//************ LEDSTRIP *****************
#define USE_LED_STRIP
#define WS2811_PIN                      PB3

// ******* FEATURES ********
#define SERIALRX_UART                   SERIAL_PORT_USART1
#define SERIALRX_PROVIDER               SERIALRX_SBUS

#define DEFAULT_FEATURES                (FEATURE_TX_PROF_SEL  | FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT  | FEATURE_BLACKBOX | FEATURE_TELEMETRY)

#define TARGET_IO_PORTA                 0xffff
#define TARGET_IO_PORTB                 0xffff
#define TARGET_IO_PORTC                 0xffff
#define TARGET_IO_PORTD                 (BIT(2))

#define MAX_PWM_OUTPUT_PORTS            8
#define TARGET_MOTOR_COUNT              4

// ESC-related features
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

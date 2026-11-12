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

/*
 * HGLRC Specter F722 Lite  (board_name HGLRCF722MINI, manufacturer HGLR)
 * Derived from MAMBAF722_2022A, corrected to the board's real pin map from a
 * Betaflight 4.5.1 factory `resource`/`diff all` dump.
 * Deltas vs Mamba: gyro = ICM42688P (unconditional), beeper PC13, OSD-CS PB2,
 * flash on SPI2/PB12 (SPI3 removed), ADC PC2/PC1/PC0, PINIO PC3/PA15, RX = UART2 CRSF.
 */

#pragma once

#define TARGET_BOARD_IDENTIFIER         "HGLT"
#define USBD_PRODUCT_STRING             "HGLRCF722LITE"

#define USE_TARGET_CONFIG

// ******** Board LEDs  **********************
#define LED0                            PC14
#define LED1                            PC15

// ******* Beeper ***********
#define BEEPER                          PC13
#define BEEPER_INVERTED

// *************** SPI1 Gyro & ACC — ICM42688P ****************
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN              CW0_DEG        // verified against Betaflight orientation on hardware
#define ICM42605_SPI_BUS                BUS_SPI1
#define ICM42605_CS_PIN                 SPI1_NSS_PIN   // PA4
#define ICM42605_EXTI_PIN               PC4

// *************** I2C ****************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                        PB8
#define I2C1_SDA                        PB9
#define DEFAULT_I2C_BUS                 BUS_I2C1

// *************** Baro **************************
#define USE_BARO
#define BARO_I2C_BUS                    DEFAULT_I2C_BUS
#define USE_BARO_DPS310
#define USE_BARO_BMP280
#define USE_BARO_MS5611
#define USE_BARO_SPL06

// *********** Magnetometer / Compass (external, on GPS module) *************
#define USE_MAG
#define MAG_I2C_BUS                     DEFAULT_I2C_BUS
#define USE_MAG_ALL

// *********** Pitot / airspeed, rangefinder, temp (external I2C) ***********
#define USE_PITOT
#define USE_PITOT_MS4525
#define USE_PITOT_DLVR
#define PITOT_I2C_BUS                   DEFAULT_I2C_BUS

#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS             DEFAULT_I2C_BUS
#define TEMPERATURE_I2C_BUS             DEFAULT_I2C_BUS

// ******* SERIAL ********
#define USE_VCP
#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6

#define UART1_TX_PIN                    PB6
#define UART1_RX_PIN                    PB7

#define UART2_TX_PIN                    PA2
#define UART2_RX_PIN                    PA3

#define UART3_TX_PIN                    PB10
#define UART3_RX_PIN                    PB11

#define UART4_TX_PIN                    PA0
#define UART4_RX_PIN                    PA1

#define UART5_TX_PIN                    PC12
#define UART5_RX_PIN                    PD2

#define UART6_TX_PIN                    PC6
#define UART6_RX_PIN                    PC7

#define SERIAL_PORT_COUNT               7

// ******* SPI ********
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_NSS_PIN                    PA4
#define SPI1_SCK_PIN                    PA5
#define SPI1_MISO_PIN                   PA6
#define SPI1_MOSI_PIN                   PA7

#define USE_SPI_DEVICE_2
#define SPI2_NSS_PIN                    PB12
#define SPI2_SCK_PIN                    PB13
#define SPI2_MISO_PIN                   PB14
#define SPI2_MOSI_PIN                   PB15
// NOTE: SPI3 intentionally omitted — Mamba used it for the flash chip on PA15/PB5,
// but on this board PA15 is PINIO2 and the flash lives on SPI2 (see below).

// ******* ADC ********
#define USE_ADC
#define ADC_CHANNEL_1_PIN               PC2   // VBAT
#define ADC_CHANNEL_2_PIN               PC1   // CURRENT
#define ADC_CHANNEL_3_PIN               PC0   // RSSI

#define VBAT_ADC_CHANNEL                ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL       ADC_CHN_2
#define RSSI_ADC_CHANNEL                ADC_CHN_3

#define VBAT_SCALE_DEFAULT              1100  // iNav default; matches the Mamba F722 baseline

// ******* OSD (MAX7456 on SPI2, CS PB2) ********
#define USE_MAX7456
#define MAX7456_SPI_BUS                 BUS_SPI2
#define MAX7456_CS_PIN                  PB2

// ******* FLASH (M25P16 16MB on SPI2, CS PB12) ********
#define USE_FLASHFS
#define USE_FLASH_M25P16
#define M25P16_SPI_BUS                  BUS_SPI2
#define M25P16_CS_PIN                   PB12
#define ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT

// ******* LED STRIP ********
#define USE_LED_STRIP
#define WS2811_PIN                      PB3

// ******* RX — internal ELRS on UART2, CRSF ********
#define SERIALRX_UART                   SERIAL_PORT_USART2
#define SERIALRX_PROVIDER               SERIALRX_CRSF

#define DEFAULT_FEATURES                (FEATURE_OSD | FEATURE_TELEMETRY)

// ******* IO / PWM ********
#define TARGET_IO_PORTA                 0xffff
#define TARGET_IO_PORTB                 0xffff
#define TARGET_IO_PORTC                 0xffff
#define TARGET_IO_PORTD                 (BIT(2))

#define MAX_PWM_OUTPUT_PORTS            8
#define TARGET_MOTOR_COUNT              8

// ESC-related features
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

// *************** PINIO ***************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                      PC3    // VTX power switcher
#define PINIO2_PIN                      PA15   // user / camera switch

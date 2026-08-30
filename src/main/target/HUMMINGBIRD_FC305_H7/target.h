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

#define TARGET_BOARD_IDENTIFIER "HBH7"
#define USBD_PRODUCT_STRING     "HUMMINGBIRD_FC305_H7"

#define USE_TARGET_CONFIG

// *************** LED, beeper ********************
#define LED0                    PE3
#define LED1                    PE4

#define BEEPER                  PA15
#define BEEPER_INVERTED
#define BEEPER_PWM_FREQUENCY    5400

// *************** SPI1 - ICM42688P ***************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7
#define SPI1_NSS_PIN            PA4

// INAV's ICM42605 driver also detects the ICM42688P by WHO_AM_I.
#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         SPI1_NSS_PIN
#define ICM42605_EXTI_PIN       PE10

// INAV does not currently support the Betaflight GYRO CLKIN feature.
// #define GYRO_CLKIN_PIN          PE9  // TIM1_CH1

// *************** SPI3 - spare configurable bus ***********
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PB3
#define SPI3_MISO_PIN           PB4
#define SPI3_MOSI_PIN           PB5
#define SPI3_NSS_PIN            PD10

// *************** SPI4 - MAX7456 OSD *************
#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN            PE12
#define SPI4_MISO_PIN           PE13
#define SPI4_MOSI_PIN           PE14
#define SPI4_NSS_PIN            PE11

#define USE_OSD
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI4
#define MAX7456_CS_PIN          SPI4_NSS_PIN

// *************** I2C1 - external sensors ********
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_ALL

#define TEMPERATURE_I2C_BUS     BUS_I2C1
#define PITOT_I2C_BUS           BUS_I2C1

#define USE_RANGEFINDER
#define USE_RANGEFINDER_MSP
#define RANGEFINDER_I2C_BUS     BUS_I2C1

// *************** I2C2 - onboard barometer *******
#define USE_I2C_DEVICE_2
#define I2C2_SCL                PB10
#define I2C2_SDA                PB11

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C2
#define USE_BARO_DPS310

// *************** SDIO SD card *******************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define USE_SDIO_PULLUP
#define SDCARD_SDIO_DEVICE      SDIODEV_1
#define SDCARD_SDIO_4BIT
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// SDIODEV_1 uses the STM32H743 fixed AF12 pin mapping:
// CK=PC12, CMD=PD2, D0=PC8, D1=PC9, D2=PC10, D3=PC11.

// *************** UARTs **************************
#define USE_VCP

#define USE_UART1
#define UART1_RX_PIN            PB7
#define UART1_TX_PIN            PB6

#define USE_UART2
#define UART2_RX_PIN            PD6
#define UART2_TX_PIN            PD5

#define USE_UART3
#define UART3_RX_PIN            PD9
#define UART3_TX_PIN            PD8

#define USE_UART4
#define UART4_RX_PIN            PD0
#define UART4_TX_PIN            PD1

#define USE_UART5
#define UART5_RX_PIN            PB12
#define UART5_TX_PIN            PB13

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
#define SERIALRX_UART           SERIAL_PORT_USART5

#define USE_ESC_SENSOR

// *************** ADC ****************************
#define USE_ADC
#define ADC_INSTANCE                ADC1

#define ADC_CHANNEL_1_PIN           PC0
#define ADC_CHANNEL_2_PIN           PC1
#define ADC_CHANNEL_3_PIN           PC5
#define ADC_CHANNEL_4_PIN           PC4  // External ADC1 / airspeed

// Secondary battery ADC pads are present but not enabled by default.
// #define ADC_CHANNEL_5_PIN           PB1  // VBAT2 (Betaflight ADC_VBAT2_PIN)
// #define ADC_CHANNEL_6_PIN           PB0  // CURR2 (Betaflight ADC_CURR2_PIN)

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3
#define AIRSPEED_ADC_CHANNEL        ADC_CHN_4

#define DEFAULT_VOLTAGE_METER_SOURCE VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE CURRENT_METER_ADC
#define CURRENT_METER_SCALE          800

// *************** PINIO **************************
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PC15  // 10V enable
#define PINIO2_PIN              PC14
#define PINIO3_PIN              PC13
#define PINIO1_FLAGS            PINIO_FLAGS_INVERTED
#define PINIO2_FLAGS            PINIO_FLAGS_INVERTED
#define PINIO3_FLAGS            PINIO_FLAGS_INVERTED

// *************** LED strip **********************
#define USE_LED_STRIP
#define WS2811_PIN              PE5

// *************** Other features *****************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_DSHOT
#define USE_DSHOT_DMAR

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define MAX_PWM_OUTPUT_PORTS    10

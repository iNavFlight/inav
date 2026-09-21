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

#define TARGET_BOARD_IDENTIFIER "GFH7"
#define USBD_PRODUCT_STRING     "ACCTONGFH7"

#define USE_TARGET_CONFIG

// *************** LEDs ***************************
// Both LEDs are wired 3V3 -> 220R -> LED anode, cathode -> MCU pin (nets
// LED_GREEN_L / LED_BLUE_L), so they are active LOW. INAV's LEDx_INVERTED
// means "drive HIGH to light up", therefore it must NOT be defined here.
#define LED0                    PC13
#define LED1                    PE10

// *************** Beeper *************************
// BUZZER -> 1.5k -> MMBT3904 NPN base, emitter to GND: active HIGH.
// BEEPER_INVERTED selects push-pull + "drive HIGH to sound", which is correct.
#define BEEPER                  PE12
#define BEEPER_INVERTED

// *************** SPI ****************************
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PD7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PD3
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

#define USE_SPI_DEVICE_4
#define SPI4_SCK_PIN            PE2
#define SPI4_MISO_PIN           PE5
#define SPI4_MOSI_PIN           PE6

// *************** IMUs ***************************
#define USE_DUAL_GYRO
#define USE_TARGET_IMU_HARDWARE_DESCRIPTORS

#define USE_IMU_ICM42605
#define GYRO_1_SPI_BUS          BUS_SPI4
#define GYRO_1_CS_PIN           PE4
#define GYRO_1_EXTI_PIN         PD10
#define GYRO_1_ALIGN            CW270_DEG

#define USE_IMU_LSM6DXX
#define GYRO_2_SPI_BUS          BUS_SPI2
#define GYRO_2_CS_PIN           PB9
#define GYRO_2_EXTI_PIN         PD11
#define GYRO_2_ALIGN            CW0_DEG

// *************** OSD ****************************
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI1
#define MAX7456_CS_PIN          PA4

// *************** I2C ****************************
#define USE_I2C

#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB7

#define USE_I2C_DEVICE_4
#define I2C4_SCL                PD12
#define I2C4_SDA                PD13

#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_DPS310
#define DPS310_I2C_ADDR         (0x77)

#define USE_MAG
#define MAG_I2C_BUS             BUS_I2C1
#define USE_MAG_IST8310

#define TEMPERATURE_I2C_BUS     BUS_I2C4
#define PITOT_I2C_BUS           BUS_I2C4
#define RANGEFINDER_I2C_BUS     BUS_I2C4

// *************** Serial ports *******************
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PD5
#define UART2_RX_PIN            PA3

#define USE_UART3
#define UART3_TX_PIN            PD8
#define UART3_RX_PIN            PD9

#define USE_UART4
#define UART4_TX_PIN            PC10
#define UART4_RX_PIN            PC11

#define USE_UART5
#define UART5_TX_PIN            PB6
#define UART5_RX_PIN            PD2

#define USE_UART7
#define UART7_TX_PIN            PE8
#define UART7_RX_PIN            PE7

#define USE_UART8
#define UART8_TX_PIN            PE1
#define UART8_RX_PIN            PE0

#define SERIAL_PORT_COUNT       8

#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL
#define SERIALRX_UART           SERIAL_PORT_USART4
#define SERIALRX_PROVIDER       SERIALRX_CRSF

// *************** CAN ****************************
// TLE9251VLE transceiver is wired to PD0/PD1. INAV has no CAN/DroneCAN stack
// yet - these defines are unreferenced and kept only as hardware documentation.
#define USE_DRONECAN
#define CAN1_RX                 PD0
#define CAN1_TX                 PD1

// *************** SD card ************************
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_SDIO_DEVICE      SDIODEV_2
#define SDCARD_SDIO_4BIT
#define SDCARD_SDIO2_CK_ALT
#define SDCARD_DETECT_PIN       PA15
#define SDCARD_DETECT_INVERTED
#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** ADC ****************************
#define USE_ADC
#define ADC_INSTANCE            ADC1

#define ADC_CHANNEL_1_PIN       PC0
#define ADC_CHANNEL_2_PIN       PB0
#define ADC_CHANNEL_3_PIN       PB1

#define VBAT_ADC_CHANNEL        ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL ADC_CHN_2
#define RSSI_ADC_CHANNEL        ADC_CHN_3

#define VBAT_SCALE_DEFAULT      1360

// *************** LED strip **********************
#define USE_LED_STRIP
#define WS2811_PIN              PB10

// *************** VTX / 12V BEC power ************
// PC12 drives the 12V BEC enable through an N-MOS: LOW = 12V on, HIGH = 12V off.
// pinioInit() drives the pin LOW when PINIO_FLAGS_INVERTED is not set, so the
// 12V rail is powered at boot (required for VTX / telemetry). As a consequence
// activating the USER1 box switches the 12V rail OFF.
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN              PC12

// *************** Features ***********************
#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_TELEMETRY | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

#define USE_SERIAL_4WAY_BLHELI_INTERFACE
#define USE_ESC_SENSOR
#define USE_DSHOT

#define TARGET_IO_PORTA         (0xffff & ~(BIT(13) | BIT(14)))
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         0xffff
#define TARGET_IO_PORTE         0xffff

#define MAX_PWM_OUTPUT_PORTS    8

/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

/*
 * OpenFC Lite 30x30 (OpenDrone): RP2354B = RP2350B (QFN-80, GPIO 0-47) with
 * 2 MB flash in the package, 12 MHz crystal.
 *
 * IO names: GPIO = port * 16 + pin, so PA0..15 = GPIO0..15, PB0..15 =
 * GPIO16..31, PC0..15 = GPIO32..47.
 *
 * Wired but not used by INAV:
 *   GPIO4/5/6    analog OSD (OSD_W, OSD_EN, OSD_SYNC), no driver
 *   GPIO15       IMU CLKIN, only wired through for TDK parts
 */

#define TARGET_BOARD_IDENTIFIER "OPFC"
#define USBD_PRODUCT_STRING     "OPENFC_LITE"

#define TARGET_IO_PORTA         0xFFFF
#define TARGET_IO_PORTB         0xFFFF
#define TARGET_IO_PORTC         0xFFFF

// Settings live in the last 64 KB of the flash, see rp2350_flash.ld
#define CONFIG_IN_FLASH
#define EEPROM_SIZE             32768

#define LED0                    PA12    // GPIO12

// Low-side N-FET: high = on
#define BEEPER                  PA7     // GPIO7
#define BEEPER_INVERTED

// *************** IMU on SPI1 (RP2350 spi0) ***************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PB2     // GPIO18
#define SPI1_MOSI_PIN           PB3     // GPIO19
#define SPI1_MISO_PIN           PB4     // GPIO20

// The footprint takes TDK and Bosch parts, the BMI270 sits on it turned by
// 90 deg against a TDK part.
#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN        CW270_DEG
#define BMI270_SPI_BUS          BUS_SPI1
#define BMI270_CS_PIN           PA14    // GPIO14

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PA14    // GPIO14

#define USE_IMU_ICM45686
#define IMU_ICM45686_ALIGN      CW0_DEG
#define ICM45686_SPI_BUS        BUS_SPI1
#define ICM45686_CS_PIN         PA14    // GPIO14

// *************** microSD on SPI2 (RP2350 spi1) ***************
// The socket has no card detect switch
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PC10    // GPIO42
#define SPI2_MOSI_PIN           PC11    // GPIO43
#define SPI2_MISO_PIN           PC12    // GPIO44

#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI2
#define SDCARD_CS_PIN           PC14    // GPIO46

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** I2C1 (RP2350 i2c0), pads for baro / mag ***************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SDA                PB0     // GPIO16
#define I2C1_SCL                PB1     // GPIO17
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define USE_BARO_ALL
#define BARO_I2C_BUS            DEFAULT_I2C_BUS

#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS             DEFAULT_I2C_BUS

// *************** UARTs ***************
// UART1/2 are the RP2350 uart0/uart1, UART3/4 are PIO1 UARTs
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA0     // GPIO0, digital VTX connector
#define UART1_RX_PIN            PA1     // GPIO1

#define USE_UART2
#define UART2_TX_PIN            PB6     // GPIO22, receiver pads
#define UART2_RX_PIN            PB5     // GPIO21

#define USE_UART3
#define UART3_TX_PIN            PA2     // GPIO2
#define UART3_RX_PIN            PA3     // GPIO3

#define USE_UART4
#define UART4_TX_PIN            PB10    // GPIO26
#define UART4_RX_PIN            PB11    // GPIO27, also the telemetry pin of the ESC connector

#define SERIAL_PORT_COUNT       5       // VCP, UART1-4

#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART2
#define GPS_UART                SERIAL_PORT_USART3

// *************** ADC ***************
// RP2350B: ADC inputs are GPIO40..47
#define USE_ADC
#define ADC_CHANNEL_1_PIN           PC9     // GPIO41, battery over 100k/10k
#define ADC_CHANNEL_2_PIN           PC8     // GPIO40, current signal of the ESC connector
#define ADC_CHANNEL_3_PIN           PC13    // GPIO45, RSSI pad
#define ADC_CHANNEL_4_PIN           PC15    // GPIO47, ADC pad

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2
#define RSSI_ADC_CHANNEL            ADC_CHN_3

// 11:1 divider is the INAV default (VBAT_SCALE_DEFAULT 1100)
// 10 mV/A (OpenESC 30x30), in 0.1 mV/A
#define CURRENT_METER_SCALE         100

// *************** Outputs ***************
// Motors on PIO0, see timerHardware[] in target.c
#define USE_DSHOT
#define USE_ESC_SENSOR
#define MAX_PWM_OUTPUT_PORTS        4

#define USE_LED_STRIP
#define WS2811_PIN                  PB7     // GPIO23, non-inverting level shifter
// On PIO1 with UART3/UART4, keeps PIO2 free. PIO1 has four state machines: the
// strip takes one, a PIO UART one per direction, so both UARTs in RX+TX do not fit.
#define PIO_LEDSTRIP_INDEX          1

// 10 V VTX rail enable, active high. Inverted, so that the rail is on from
// boot and the USER1 box switches it off.
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PA11    // GPIO11
#define PINIO1_FLAGS                PINIO_FLAGS_INVERTED

// ESC passthrough over the motor pins
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define USE_OSD
#define USE_MSP_OSD

#define DEFAULT_FEATURES            (FEATURE_OSD | FEATURE_VBAT | FEATURE_CURRENT_METER | FEATURE_TELEMETRY | FEATURE_TX_PROF_SEL | FEATURE_BLACKBOX)

// *************** Not available on the RP2350 platform ***************
// No driver for the OLED, PPM/PWM receivers, camera control
#undef USE_DASHBOARD
#undef USE_PPM
#undef USE_PWM
#undef USE_RESOURCE_MGMT
#undef USE_CAMERA_CONTROL
#undef USE_BRUSHED_ESC_AUTODETECT
// The UART drivers have no single-wire half-duplex mode
#undef USE_MSP_OVER_TELEMETRY
#undef USE_TELEMETRY_FRSKY_HUB
#undef USE_TELEMETRY_HOTT
#undef USE_TELEMETRY_SMARTPORT
#undef USE_TELEMETRY_IBUS
#undef USE_TELEMETRY_JETIEXBUS
#undef USE_TELEMETRY_SRXL
#undef USE_TELEMETRY_GHST
#undef USE_SMARTPORT_MASTER
#undef USE_VTX_TRAMP
// CMSIS-DSP is not built for the RP2350, see RP2350_PICO/target.h
#undef USE_ADAPTIVE_FILTER
#undef USE_GYRO_KALMAN
#undef USE_DYNAMIC_FILTERS

// cpu_overclock setting, see systemApplyCpuOverclock()
#define USE_CPU_OVERCLOCK

// Hot functions run from RAM instead of the XIP flash, see RP2350_PICO/target.h
#undef FAST_CODE
#define FAST_CODE __attribute__((section(".time_critical.inav")))
#undef RP2350_FAST_CODE
#define RP2350_FAST_CODE FAST_CODE

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
 * OpenFC Lite 20x20 "Mini" (OpenDrone): RP2354A = RP2350A (QFN-60, GPIO 0-29)
 * with 2 MB flash in the package, 12 MHz crystal.
 *
 * IO names: GPIO = port * 16 + pin, so PA0..15 = GPIO0..15, PB0..13 =
 * GPIO16..29.
 *
 * Wired but not used by INAV:
 *   GPIO9        IMU interrupt, the drivers poll
 *   GPIO14..16   analog OSD (OSD_W, OSD_EN, OSD_SYNC), no driver
 */

#define TARGET_BOARD_IDENTIFIER "OPFM"
#define USBD_PRODUCT_STRING     "OPENFC_LITE_MINI"

#define TARGET_IO_PORTA         0xFFFF
#define TARGET_IO_PORTB         0x3FFF

// Settings live in the last 64 KB of the flash, see rp2350_flash.ld
#define CONFIG_IN_FLASH
#define EEPROM_SIZE             32768

#define LED0                    PB10    // GPIO26

// Low-side N-FET: high = on
#define BEEPER                  PB1     // GPIO17
#define BEEPER_INVERTED

// *************** IMU on SPI1 (RP2350 spi1) ***************
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN            PA10    // GPIO10
#define SPI1_MOSI_PIN           PA11    // GPIO11
#define SPI1_MISO_PIN           PA12    // GPIO12

// The footprint takes Bosch and TDK parts, the BMI270 sits on it turned
// by 90 deg against a TDK part.
#define USE_IMU_BMI270
#define IMU_BMI270_ALIGN        CW270_DEG
#define BMI270_SPI_BUS          BUS_SPI1
#define BMI270_CS_PIN           PA13    // GPIO13

#define USE_IMU_ICM42605
#define IMU_ICM42605_ALIGN      CW0_DEG
#define ICM42605_SPI_BUS        BUS_SPI1
#define ICM42605_CS_PIN         PA13

#define USE_IMU_ICM45686
#define IMU_ICM45686_ALIGN      CW0_DEG
#define ICM45686_SPI_BUS        BUS_SPI1
#define ICM45686_CS_PIN         PA13

// *************** microSD on SPI2 (RP2350 spi0) ***************
// The socket has no card detect switch
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB2     // GPIO18
#define SPI2_MOSI_PIN           PB3     // GPIO19
#define SPI2_MISO_PIN           PB4     // GPIO20

#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI2
#define SDCARD_CS_PIN           PB5     // GPIO21

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** I2C1 (RP2350 i2c0), pads for baro / mag ***************
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SDA                PA4     // GPIO4
#define I2C1_SCL                PA5     // GPIO5
#define DEFAULT_I2C_BUS         BUS_I2C1

#define USE_BARO
#define USE_BARO_ALL
#define BARO_I2C_BUS            DEFAULT_I2C_BUS

#define USE_MAG
#define USE_MAG_ALL
#define MAG_I2C_BUS             DEFAULT_I2C_BUS

// *************** UARTs ***************
// UART1/2 are the RP2350 uart0/uart1, UART3 is a PIO1 UART
#define USE_VCP

#define USE_UART1
#define UART1_TX_PIN            PA0     // GPIO0, digital VTX connector
#define UART1_RX_PIN            PA1     // GPIO1

#define USE_UART2
#define UART2_TX_PIN            PA6     // GPIO6, receiver pads
#define UART2_RX_PIN            PA7     // GPIO7

#define USE_UART3
#define UART3_TX_PIN            PA2     // GPIO2
#define UART3_RX_PIN            PA3     // GPIO3, also the telemetry pin of the ESC connector

#define SERIAL_PORT_COUNT       4       // VCP, UART1-3

#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF
#define SERIALRX_UART           SERIAL_PORT_USART2
#define GPS_UART                SERIAL_PORT_USART3

// *************** ADC ***************
// RP2350A: ADC inputs are GPIO26..29, GPIO26/27 are taken by the LED and the
// 10 V enable
#define USE_ADC
#define ADC_CHANNEL_1_PIN           PB13    // GPIO29, battery over 100k/10k
#define ADC_CHANNEL_2_PIN           PB12    // GPIO28, current signal of the ESC connector

#define VBAT_ADC_CHANNEL            ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL   ADC_CHN_2

// 11:1 divider is the INAV default (VBAT_SCALE_DEFAULT 1100)
// 20 mV/A (OpenESC 20x20), in 0.1 mV/A
#define CURRENT_METER_SCALE         200

// *************** Outputs ***************
// Motors on PIO0, see timerHardware[] in target.c
#define USE_DSHOT
#define USE_ESC_SENSOR
#define MAX_PWM_OUTPUT_PORTS        4

// The pad is driven by an N-FET with a pull-up to 5 V, which inverts
#define USE_LED_STRIP
#define WS2811_PIN                  PA8     // GPIO8
#define WS2811_PIN_INVERTED
#define PIO_LEDSTRIP_INDEX          2

// 10 V VTX rail enable, active high. Inverted, so that the rail is on from
// boot and the USER1 box switches it off.
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN                  PB11    // GPIO27
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

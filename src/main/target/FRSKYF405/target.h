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

#define TARGET_BOARD_IDENTIFIER "FRF4"

#define USBD_PRODUCT_STRING  "FrSkyF405"

// *************** LED & BEEPER **********************
// Status LEDs share SWD debug pins - only available in release builds
// In debug builds, these pins must remain in SWD mode for debugging
// Red LED is likely a power indicator (always-on, no GPIO control)
#ifndef DEBUG_BUILD
    #define LED0                PA14    // Blue LED (shares SWCLK) via R34 1K
    #define LED1                PA13    // Green LED (shares SWDIO) via R35 1K
#endif
// Red LED appears to be power indicator (connected to VCC, not GPIO)

#define BEEPER                  PC15
#define BEEPER_INVERTED         // INAV beeper driver writes !onoff; BEEPER_INVERTED makes it write HIGH to activate the N-channel MOSFET (Q2)

// *************** Gyro & ACC **********************
#define USE_SPI
#define USE_SPI_DEVICE_1

#define SPI1_SCK_PIN            PA5
#define SPI1_MISO_PIN           PA6
#define SPI1_MOSI_PIN           PA7

// IIM-42688P Gyro on SPI1
#define USE_IMU_ICM42605       // IIM-42688P is compatible with ICM42605 driver
#define ICM42605_CS_PIN        PA4
#define ICM42605_SPI_BUS       BUS_SPI1
#define IMU_ICM42605_ALIGN     CW0_DEG  // TODO: Verify orientation from board layout


// *************** OSD *****************************
#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN            PB13
#define SPI2_MISO_PIN           PC2
#define SPI2_MOSI_PIN           PC3

// AT7456E OSD on SPI2
#define USE_MAX7456
#define MAX7456_SPI_BUS         BUS_SPI2
#define MAX7456_CS_PIN          PB12

// *************** SD Card *************************
#define USE_SPI_DEVICE_3
#define SPI3_SCK_PIN            PC10
#define SPI3_MISO_PIN           PC11
#define SPI3_MOSI_PIN           PC12

// SD Card on SPI3
// NOTE: PC14 used for CS (Chip Select) - OSC32_IN pin used as GPIO for slow CS signal
#define USE_SDCARD
#define USE_SDCARD_SPI
#define SDCARD_SPI_BUS          BUS_SPI3
#define SDCARD_CS_PIN           PC14

#define ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT

// *************** UART *****************************
#define USE_VCP
// Note: USB VBUS sensing not clearly defined in schematic

#define USE_UART1
#define UART1_TX_PIN            PA9
#define UART1_RX_PIN            PA10

#define USE_UART2
#define UART2_TX_PIN            PA2
#define UART2_RX_PIN            PA3

// USART2 has SBUS inverter circuit
#define SERIALRX_UART           SERIAL_PORT_USART2
#define SERIALRX_PROVIDER       SERIALRX_SBUS
#define DEFAULT_RX_TYPE         RX_TYPE_SERIAL

// UART3 conflicts with I2C2 (shares PB10/PB11)
// User must choose: enable UART3 OR I2C2, not both
#define USE_UART3
#define UART3_TX_PIN            PB10    // Conflicts with I2C2_SCL
#define UART3_RX_PIN            PB11    // Conflicts with I2C2_SDA

#define USE_UART4
#define UART4_TX_PIN            PA0
#define UART4_RX_PIN            PC5     // Shared with RSSI_ADC

// UART5 TX conflicts with SPI3_MOSI (PC12)
// User must choose: enable UART5 OR SD card, not both
#define USE_UART5
#define UART5_TX_PIN            PC12    // Conflicts with SPI3_MOSI
#define UART5_RX_PIN            PD2

#define USE_UART6
#define UART6_TX_PIN            PC6
#define UART6_RX_PIN            PC7

#define SERIAL_PORT_COUNT       7       // VCP + UART1-6

// *************** I2C ****************************
#define USE_I2C

// I2C1 for barometer (SPL06)
#define USE_I2C_DEVICE_1
#define I2C1_SCL                PB8
#define I2C1_SDA                PB9

// I2C2 conflicts with UART3 (shares PB10/PB11)
// Enable one or the other, not both
// #define USE_I2C_DEVICE_2
// #define I2C2_SCL                PB10    // Conflicts with UART3_TX
// #define I2C2_SDA                PB11    // Conflicts with UART3_RX

#define DEFAULT_I2C_BUS         BUS_I2C1

// SPL06 Barometer on I2C1 @ address 0x76
#define USE_BARO
#define BARO_I2C_BUS            BUS_I2C1
#define USE_BARO_SPL06

// Magnetometer on I2C (external via connector)
#define USE_MAG
#define MAG_I2C_BUS             DEFAULT_I2C_BUS
#define USE_MAG_ALL

#define PITOT_I2C_BUS           DEFAULT_I2C_BUS
#define TEMPERATURE_I2C_BUS     DEFAULT_I2C_BUS
#define USE_RANGEFINDER
#define RANGEFINDER_I2C_BUS     DEFAULT_I2C_BUS

// *************** ADC *****************************
#define USE_ADC
#define ADC_INSTANCE            ADC1
#define ADC1_DMA_STREAM         DMA2_Stream0

#define ADC_CHANNEL_1_PIN       PC0     // VBAT_ADC
#define ADC_CHANNEL_2_PIN       PC1     // CURR_ADC
#define ADC_CHANNEL_3_PIN       PC5     // RSSI_IN (shared with UART4_RX)

#define VBAT_ADC_CHANNEL        ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL ADC_CHN_2
#define RSSI_ADC_CHANNEL        ADC_CHN_3

// Current sensor: INA139 with 0.25mΩ shunt
#define CURRENT_METER_SCALE     250     // TODO: Calibrate based on INA139 + shunt values

#define DEFAULT_FEATURES        (FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT | FEATURE_TELEMETRY | FEATURE_BLACKBOX)

// *************** LED STRIP ***********************
// CON23 LED output - PA15 (T2_1 signal on schematic)
#define USE_LED_STRIP
#define WS2811_PIN              PA15    // TIM2_CH1, labeled as T2_1 on schematic

// *************** PWM OUTPUTS *********************
// 9 motor outputs (S1-S9)
// Note: S7/S8 on TIM12 do not support DShot (no DMA on TIM12 for STM32F405)
#define MAX_PWM_OUTPUT_PORTS    9

// *************** Other ***************************
#define USE_DSHOT
#define USE_ESC_SENSOR
#define USE_SERIAL_4WAY_BLHELI_INTERFACE

#define TARGET_IO_PORTA         0xffff
#define TARGET_IO_PORTB         0xffff
#define TARGET_IO_PORTC         0xffff
#define TARGET_IO_PORTD         (BIT(2))  // PD2 used for UART5_RX

// *************** NOTES & ISSUES *******************
//
// DESIGN NOTES:
// 1. PC14 used for SPI3_NSS (SD card CS) - RTC clock domain, should be fine for slow CS signal
//
// 2. Status LEDs share SWD debug pins (PA13/PA14) - HANDLED WITH #ifndef DEBUG_BUILD
//    Release builds: Pins configured as GPIO outputs for status LEDs
//    Debug builds: Pins remain in SWD mode for debugging (LEDs disabled)
//    Red LED appears to be power indicator (always-on, no GPIO control needed)
//
// PIN CONFLICTS (target or user must choose):
// 3. UART3 vs I2C2: PB10/PB11 shared
//    - Enable UART3 for telemetry/GPS, OR
//    - Enable I2C2 for additional sensors
//
// 4. UART5 TX vs SD Card: PC12 shared (SPI3_MOSI)
//    - Enable UART5 for telemetry, OR
//    - Enable SD card for blackbox logging
//
// 5. UART4_RX vs RSSI ADC: PC5 shared
//    - May be intentional if RSSI comes via UART protocol
//
// MISSING INFORMATION:
// 6. Gyro orientation (IMU_ALIGN) unknown - needs physical board inspection
// 7. USB VBUS sensing pin not clearly defined in schematic (PC13 may be used)

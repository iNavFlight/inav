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

#define TARGET_BOARD_IDENTIFIER "RP2P"
#define USBD_PRODUCT_STRING  "RP2350_PICO"

#define LED0 PB9   // GPIO 25 (Port B pin 9)

// GPIO port mapping for RP2350:
//   Port A (gpioid 0) = GPIO 0-15
//   Port B (gpioid 1) = GPIO 16-29
#define TARGET_IO_PORTA 0xFFFF
#define TARGET_IO_PORTB 0x3FFF

// Flash-based config storage in FLASH_CONFIG region (0x103F0000, 64 KB)
// __config_start / __config_end are defined in rp2350_flash.ld.
#define CONFIG_IN_FLASH
#define EEPROM_SIZE     32768

#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4

#define SERIAL_PORT_COUNT 5  // VCP + UART1 + UART2 + UART3 + UART4

// DShot motor output via PIO0 (1 SM per motor, up to 4 motors).
// GP10–13 default to motors 1–4; GP4–7 are reserved for SPI0 (gyro + flash).
// Motor/servo GPIO assignments come from timerHardware[] in target.c.

// Servo PWM output via hardware PWM slices (GP16–GP19 by default).
// 50 Hz, 1000–2000 µs pulse width. GP16–19 are free from other peripherals.

// ADC inputs are hardware-fixed on RP2350A: ADC0–3 map only to GPIO26–29.
// ADC_CHANNEL_N_PIN cannot be changed without a board respin.
#define ADC_CHANNEL_1_PIN          PB10  /* GPIO 26 — battery voltage  */
#define ADC_CHANNEL_2_PIN          PB11  /* GPIO 27 — current sensor   */
#define ADC_CHANNEL_3_PIN          PB12  /* GPIO 28 — RSSI voltage     */

#define VBAT_ADC_CHANNEL           ADC_CHN_1
#define CURRENT_METER_ADC_CHANNEL  ADC_CHN_2
#define RSSI_ADC_CHANNEL           ADC_CHN_3
#define USE_DSHOT

/*
 * Hardware UART pin assignments for Raspberry Pi Pico 2 — Option C layout.
 *
 * UART1 (INAV) → RP2350 uart0: GP0/1
 * UART2 (INAV) → RP2350 uart1: GP2/3
 * UART3 (INAV) → PIO1 SM0(TX)+SM1(RX): GP8/9
 * UART4 (INAV) → PIO1 SM2(TX)+SM3(RX): GP14/15
 *
 * GP4–7 are reserved for SPI0 (gyro + flash, future M5/M6).
 * GP10–13 are reserved for DShot motors on PIO0.
 * PIO2 SM0 is reserved for WS2812 LED strip; SMs 1–3 spare.
 */
#define UART1_TX_PIN  PA0   /* GPIO0  — uart0 TX  (MSP / configurator) */
#define UART1_RX_PIN  PA1   /* GPIO1  — uart0 RX */
#define UART2_TX_PIN  PA2   /* GPIO2  — uart1 TX  (receiver: CRSF/SBUS) */
#define UART2_RX_PIN  PA3   /* GPIO3  — uart1 RX  (HW inversion, no external inverter) */
/* PIO1: UART3 on SM0(TX)+SM1(RX), UART4 on SM2(TX)+SM3(RX) */
/* PIO2 is reserved for RGB LED strip (SM0) and future UART5/6 (SMs 1–3) */
#define UART3_TX_PIN  PA8   /* GPIO8  — PIO1 SM0 TX  (GPS) */
#define UART3_RX_PIN  PA9   /* GPIO9  — PIO1 SM1 RX */
#define UART4_TX_PIN  PA14  /* GPIO14 — PIO1 SM2 TX  (telemetry / extra) */
#define UART4_RX_PIN  PA15  /* GPIO15 — PIO1 SM3 RX */

#define DEFAULT_RX_FEATURE      FEATURE_RX_MSP
#define DEFAULT_FEATURES        (FEATURE_GPS | FEATURE_VBAT)

#define USE_ADC
#define USE_IMU_FAKE
#define USE_BARO
#define USE_BARO_ALL
#define USE_GPS_FAKE
#define USE_PITOT_FAKE
#define USE_RANGEFINDER_FAKE
#define USE_RX_SIM

#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SDA              PB2   /* GPIO18 — i2c1 SDA */
#define I2C1_SCL              PB3   /* GPIO19 — i2c1 SCL */
#define DEFAULT_I2C_BUS       BUS_I2C1
#define MAG_I2C_BUS           DEFAULT_I2C_BUS
#define BARO_I2C_BUS          DEFAULT_I2C_BUS

#define USE_MAG
#define USE_MAG_ALL

#define USE_MSP_OSD
#define USE_OSD

#undef USE_DASHBOARD
#define USE_VCP
#undef USE_PPM
#undef USE_PWM
#define USE_LED_STRIP
#define WS2811_PIN         PB6    /* GP22 — free from flash/I2C/UART/SPI */
#define PIO_LEDSTRIP_INDEX 2      /* PIO2 SM0 — reserved for WS2812       */
#undef USE_MSP_OVER_TELEMETRY
#undef USE_TELEMETRY_FRSKY_HUB
#undef USE_TELEMETRY_HOTT
#undef USE_TELEMETRY_SMARTPORT
#undef USE_RESOURCE_MGMT
#undef USE_TELEMETRY_CRSF
#undef USE_TELEMETRY_IBUS
#undef USE_TELEMETRY_JETIEXBUS
#undef USE_TELEMETRY_SRXL
#undef USE_TELEMETRY_GHST
#undef USE_VTX_TRAMP
#undef USE_CAMERA_CONTROL
#undef USE_BRUSHED_ESC_AUTODETECT
#undef USE_SERIAL_4WAY_BLHELI_BOOTLOADER
#undef USE_SERIAL_4WAY_SK_BOOTLOADER
#undef USE_ADAPTIVE_FILTER
#undef USE_GYRO_KALMAN

// SPI0 — gyro + flash: GP4 (MISO/PA4), GP6 (SCK/PA6), GP7 (MOSI/PA7)
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN          PA6   /* GPIO6  — spi0 SCK  */
#define SPI1_MISO_PIN         PA4   /* GPIO4  — spi0 MISO */
#define SPI1_MOSI_PIN         PA7   /* GPIO7  — spi0 MOSI */

// FAST_CODE: place hot functions in SRAM (copied from flash at boot) to avoid
// XIP cache pressure on the large PID/scheduler/gyro code path.
// common.h defines FAST_CODE without a #ifndef guard, so we must #undef first.
// The linker script (rp2350_flash.ld) places .time_critical* in .data > RAM AT> FLASH
// so these symbols are automatically copied to SRAM by crt0 at boot.
#undef FAST_CODE
#define FAST_CODE __attribute__((section(".time_critical.inav")))

// RP2350_FAST_CODE: like FAST_CODE but only active on RP2350.
// Used for functions too large for F7 ITCM (e.g. imuMahonyAHRSupdate ~2 KB)
// that would overflow F7 ITCM if marked plain FAST_CODE.
#undef RP2350_FAST_CODE
#define RP2350_FAST_CODE FAST_CODE

#define TARGET_FLASH_SIZE 4096

#define LED_STRIP_TIMER 1
#define SOFTSERIAL_1_TIMER 2
#define SOFTSERIAL_2_TIMER 3

/* RP2350 PWM slice "timers" — one TIM_TypeDef per slice, used as group IDs.
 * Analogous to TIM1/TIM3/… on STM32; pins sharing a slice must run at the
 * same update rate.  Defined in drivers/timer_rp2350.c. */
extern TIM_TypeDef rp2350Pwm4;   /* slice 4:  GP8/GP9   — servos (dual-use UART3) */
extern TIM_TypeDef rp2350Pwm5;   /* slice 5:  GP10/GP11 — motors 1-2              */
extern TIM_TypeDef rp2350Pwm6;   /* slice 6:  GP12/GP13 — motors 3-4              */
extern TIM_TypeDef rp2350Pwm7;   /* slice 7:  GP14/GP15 — servos (dual-use UART4) */
extern TIM_TypeDef rp2350Pwm10;  /* slice 10: GP20/GP21 — servos (dedicated)      */

/* On RP2350, TIMn = PWM slice n (slice = gpio / 2; A/B channels share clkdiv/wrap).
 * GP8/9→slice4, GP10/11→slice5, GP12/13→slice6, GP14/15→slice7, GP20/21→slice10. */
#define TIM4  (&rp2350Pwm4)
#define TIM5  (&rp2350Pwm5)
#define TIM6  (&rp2350Pwm6)
#define TIM7  (&rp2350Pwm7)
#define TIM10 (&rp2350Pwm10)

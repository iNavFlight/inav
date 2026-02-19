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

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TARGET_BOARD_IDENTIFIER "RP2P"
#define USBD_PRODUCT_STRING  "RP2350_PICO"

// Onboard LED — GPIO 25 = Port B, pin 9 (25 - 16 = 9)
#define PICO_LED_PIN 25
#define LED0 PB9

// GPIO port mapping for RP2350:
//   Port A (gpioid 0) = GPIO 0-15
//   Port B (gpioid 1) = GPIO 16-29
#define TARGET_IO_PORTA 0xFFFF
#define TARGET_IO_PORTB 0x3FFF

// RAM-based config (will move to flash-based later)
#define CONFIG_IN_RAM
#define EEPROM_SIZE     32768

#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4

#define SERIAL_PORT_COUNT 5  // VCP + UART1 + UART2 + UART3 + UART4

// DShot motor output via PIO0 (1 SM per motor, max 4 motors)
// GP8–11 are dedicated to motors; GP4–7 are reserved for SPI0 (future M5).
#define USE_DSHOT
#define MOTOR1_PIN  8   /* GP8  — PIO0 SM0 DShot */
#define MOTOR2_PIN  9   /* GP9  — PIO0 SM1 DShot */
#define MOTOR3_PIN 10   /* GP10 — PIO0 SM2 DShot */
#define MOTOR4_PIN 11   /* GP11 — PIO0 SM3 DShot */

/*
 * Hardware UART pin assignments for Raspberry Pi Pico 2 — Option C layout.
 *
 * UART1 (INAV) → RP2350 uart0: GP0/1
 * UART2 (INAV) → RP2350 uart1: GP2/3
 * UART3 (INAV) → PIO1 SM0(TX)+SM1(RX): GP12/13
 * UART4 (INAV) → PIO1 SM2(TX)+SM3(RX): GP14/15
 *
 * GP4–7 are reserved for SPI0 (gyro + flash, future M5/M6).
 * GP8–11 are reserved for DShot motors on PIO0 (future M8).
 * PIO2 SM0 is reserved for WS2812 LED strip; SMs 1–3 spare.
 */
#define UART1_TX_PIN  0   /* GPIO0  — uart0 TX  (MSP / configurator) */
#define UART1_RX_PIN  1   /* GPIO1  — uart0 RX */
#define UART2_TX_PIN  2   /* GPIO2  — uart1 TX  (receiver: CRSF/SBUS) */
#define UART2_RX_PIN  3   /* GPIO3  — uart1 RX  (HW inversion, no external inverter) */
/* PIO1: UART3 on SM0(TX)+SM1(RX), UART4 on SM2(TX)+SM3(RX) */
/* PIO2 is reserved for RGB LED strip (SM0) and future UART5/6 (SMs 1–3) */
#define UART3_TX_PIN 12   /* GPIO12 — PIO1 SM0 TX  (GPS) */
#define UART3_RX_PIN 13   /* GPIO13 — PIO1 SM1 RX */
#define UART4_TX_PIN 14   /* GPIO14 — PIO1 SM2 TX  (telemetry / extra) */
#define UART4_RX_PIN 15   /* GPIO15 — PIO1 SM3 RX */

#define DEFAULT_RX_FEATURE      FEATURE_RX_MSP
#define DEFAULT_FEATURES        (FEATURE_GPS)

// Fake sensors for M1 stub build
#define USE_ADC
#define USE_IMU_FAKE
#define USE_FAKE_BARO
#define USE_FAKE_MAG
#define USE_GPS_FAKE
#define USE_PITOT_FAKE
#define USE_RANGEFINDER_FAKE
#define USE_RX_SIM

#define USE_MSP_OSD
#define USE_OSD

#undef USE_DASHBOARD
#define USE_VCP
#undef USE_PPM
#undef USE_PWM
#undef USE_LED_STRIP
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
#undef USE_I2C
#undef USE_SPI

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

extern uint32_t SystemCoreClock;

#define U_ID_0 0
#define U_ID_1 1
#define U_ID_2 2

// Dummy type definitions (same pattern as SITL)
typedef struct
{
    void* dummy;
} TIM_TypeDef;

typedef struct
{
    void* dummy;
} TIM_OCInitTypeDef;

typedef struct {
    void* dummy;
} DMA_TypeDef;

typedef struct {
    void* dummy;
} DMA_Channel_TypeDef;

typedef struct
{
    void* dummy;
} SPI_TypeDef;

typedef struct
{
    void* dummy;
} I2C_TypeDef;

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
typedef enum {TEST_IRQ = 0 } IRQn_Type;
typedef enum {
    EXTI_Trigger_Rising = 0x08,
    EXTI_Trigger_Falling = 0x0C,
    EXTI_Trigger_Rising_Falling = 0x10
} EXTITrigger_TypeDef;

typedef struct
{
  uint32_t IDR;
  uint32_t ODR;
  uint32_t BSRR;
  uint32_t BRR;
} GPIO_TypeDef;

#define GPIOA_BASE ((intptr_t)0x0001)

typedef struct
{
    uint32_t dummy;
} USART_TypeDef;

#define USART1 ((USART_TypeDef *)0x0001)
#define USART2 ((USART_TypeDef *)0x0002)
#define USART3 ((USART_TypeDef *)0x0003)
#define USART4 ((USART_TypeDef *)0x0004)
#define USART5 ((USART_TypeDef *)0x0005)
#define USART6 ((USART_TypeDef *)0x0006)
#define USART7 ((USART_TypeDef *)0x0007)
#define USART8 ((USART_TypeDef *)0x0008)

#define UART4 ((USART_TypeDef *)0x0004)
#define UART5 ((USART_TypeDef *)0x0005)
#define UART7 ((USART_TypeDef *)0x0007)
#define UART8 ((USART_TypeDef *)0x0008)

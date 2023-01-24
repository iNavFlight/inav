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

#include <platform.h>

#define TARGET_BOARD_IDENTIFIER "SITL"
#define USBD_PRODUCT_STRING  "SITL"

#define REQUIRE_PRINTF_LONG_SUPPORT

// file name to save config
#define EEPROM_FILENAME "eeprom.bin"
#define CONFIG_IN_FILE
#define EEPROM_SIZE     32768

#undef SCHEDULER_DELAY_LIMIT
#define SCHEDULER_DELAY_LIMIT           1

#define USE_UART1
#define USE_UART2
#define USE_UART3
#define USE_UART4
#define USE_UART5
#define USE_UART6
#define USE_UART7
#define USE_UART8

#define SERIAL_PORT_COUNT 8

#define DEFAULT_RX_FEATURE      FEATURE_RX_MSP
#define DEFAULT_FEATURES        (FEATURE_GPS |  FEATURE_OSD | FEATURE_CURRENT_METER | FEATURE_VBAT)

#define USE_ADC
#define USE_MAG
#define USE_BARO
#define USE_PITOT_FAKE
#define USE_IMU_FAKE
#define USE_FAKE_BARO
#define USE_FAKE_MAG
#define USE_GPS_FAKE

#undef USE_DASHBOARD
#undef USE_TELEMETRY_LTM

#undef USE_GYRO_KALMAN // Strange behaviour under x86/x64 ?!?
#undef USE_VCP
#undef USE_PPM
#undef USE_PWM
#undef USE_LED_STRIP
#undef USE_TELEMETRY
#undef USE_MSP_OVER_TELEMETRY
#undef USE_TELEMETRY_FRSKY_HUB
#undef USE_TELEMETRY_HOTT
#undef USE_TELEMETRY_SMARTPORT
#undef USE_TELEMETRY_MAVLINK
#undef USE_RESOURCE_MGMT
#undef USE_TELEMETRY_CRSF
#undef USE_TELEMETRY_IBUS
#undef USE_TELEMETRY_JETIEXBUS
#undef USE_TELEMETRY_SRXL
#undef USE_TELEMETRY_GHST
#undef USE_VTX_COMMON
#undef USE_VTX_CONTROL
#undef USE_VTX_SMARTAUDIO
#undef USE_VTX_TRAMP
#undef USE_CAMERA_CONTROL
#undef USE_BRUSHED_ESC_AUTODETECT
#undef USE_SERIAL_4WAY_BLHELI_BOOTLOADER
#undef USE_SERIAL_4WAY_SK_BOOTLOADER

#undef USE_I2C
#undef USE_SPI

// Some dummys
#define TARGET_FLASH_SIZE 2048

#define LED_STRIP_TIMER 1
#define SOFTSERIAL_1_TIMER 2
#define SOFTSERIAL_2_TIMER 3

#define DEFIO_NO_PORTS

extern uint32_t SystemCoreClock;

#define U_ID_0 0
#define U_ID_1 1
#define U_ID_2 2

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

typedef enum 
{
    SITL_SIM_NONE,
    SITL_SIM_REALFLIGHT,
    SITL_SIM_XPLANE,
} SitlSim_e;

bool lockMainPID(void);
void unlockMainPID(void);
void parseArguments(int argc, char *argv[]);
char *strnstr(const char *s, const char *find, size_t slen);

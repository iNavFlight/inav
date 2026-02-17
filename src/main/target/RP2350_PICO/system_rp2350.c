/*
 * RP2350 system functions for INAV — Milestone 2
 *
 * Real implementations of timing, reset, and system init
 * using the Pico SDK hardware abstraction.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <platform.h>
#include "target.h"

#include "common/time.h"
#include "common/utils.h"
#include "drivers/system.h"

#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/structs/m33.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/bootrom.h"

// SystemCoreClock — updated by systemInit from SDK clock tree
uint32_t SystemCoreClock = 150000000;

// DWT cycle counter — Cortex-M33 Data Watchpoint and Trace
#define DWT_CTRL   (m33_hw->dwt_ctrl)
#define DWT_CYCCNT (m33_hw->dwt_cyccnt)
#define DEMCR      (m33_hw->demcr)

#define DWT_CTRL_CYCCNTENA_Msk   (1UL << 0)
#define DEMCR_TRCENA_Msk         (1UL << 24)

// Called from crt0.S as part of SDK runtime init
void SystemInit(void)
{
    // Enable DWT cycle counter
    DEMCR |= DEMCR_TRCENA_Msk;
    DWT_CYCCNT = 0;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void systemInit(void)
{
    // SDK runtime already initialized clocks, GPIO, etc. via crt0 → runtime_init
    // We just need to update our SystemCoreClock variable
    SystemCoreClock = clock_get_hz(clk_sys);

    // Initialize USB stdio (printf over USB CDC)
    stdio_init_all();

    printf("INAV RP2350 booting...\n");
}

// --- Timing functions ---

timeUs_t micros(void)
{
    return (timeUs_t)time_us_32();
}

uint64_t microsISR(void)
{
    return time_us_64();
}

uint32_t millis(void)
{
    return (uint32_t)(time_us_64() / 1000ULL);
}

void delayMicroseconds(timeUs_t us)
{
    busy_wait_us_32((uint32_t)us);
}

void delay(timeMs_t ms)
{
    busy_wait_ms((uint32_t)ms);
}

uint32_t getCycleCounter(void)
{
    return DWT_CYCCNT;
}

// --- Reset functions ---

void systemReset(void)
{
    watchdog_reboot(0, 0, 0);
    while (1) {}
}

void systemResetToBootloader(void)
{
    rom_reset_usb_boot(0, 0);
    while (1) {}
}

// --- Newlib syscall stubs (required by libc_nano) ---

#include <sys/stat.h>

__attribute__((used)) int _fstat(int fd, struct stat *buf)
{
    (void)fd;
    (void)buf;
    return -1;
}

__attribute__((used)) int _isatty(int fd)
{
    (void)fd;
    return 1;
}

// --- Unique ID ---

void getUniqueId(uint8_t *id)
{
    pico_unique_board_id_t board_id;
    pico_get_unique_board_id(&board_id);
    // INAV expects 12 bytes (U_ID_0/1/2 = 3x uint32_t)
    // Pico has 8 bytes — pad with zeros
    for (int i = 0; i < 8; i++) {
        id[i] = board_id.id[i];
    }
    for (int i = 8; i < 12; i++) {
        id[i] = 0;
    }
}

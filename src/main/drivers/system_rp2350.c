/*
 * RP2350 system functions for INAV
 *
 * Processor-level implementations of timing, reset, system init, and
 * CMSIS DSP stubs shared by all RP2350 boards.
 * Analogous to drivers/system_stm32f7xx.c on F7 targets.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <platform.h>

#include "arm_math.h"
#include "common/time.h"
#include "common/utils.h"
#include "drivers/system.h"
#include "drivers/persistent.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/watchdog.h"
#include "hardware/structs/m33.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"
#include "pico/bootrom.h"
#include "pico/runtime_init.h"
#include "tusb.h"

// SystemCoreClock — updated by systemInit from SDK clock tree
uint32_t SystemCoreClock = 192000000;

// ── Pre-init diagnostic blinks (run inside runtime_run_initializers) ──────────
// Enable by defining RP2350_DIAG_BLINK at compile time (e.g. in CMakeLists.txt).
// When active, blinks the onboard LED during early boot so a debugger-less board
// can show how far initialisation reached:
//   1 blink → "00150": after EARLY_RESETS  — GPIO/SIO accessible
//   2 blinks → "00650": after POST_CLOCK_RESETS — clocks, coprocessors, aeabi done
// If neither fires, the crash is in BSS/data copy, bootrom_reset, or early_resets.
#ifdef RP2350_DIAG_BLINK

#define DIAG_LED_PIN 25

static void diagRawBlink(int n)
{
    // gpio_init is safe after EARLY_RESETS ("00100") releases IOBANK0
    gpio_init(DIAG_LED_PIN);
    gpio_set_dir(DIAG_LED_PIN, GPIO_OUT);
    for (int i = 0; i < n; i++) {
        gpio_put(DIAG_LED_PIN, 1);
        busy_wait_us_32(200000);  // 200 ms — no sleep_ms needed (timer always-on)
        gpio_put(DIAG_LED_PIN, 0);
        busy_wait_us_32(200000);
    }
    busy_wait_us_32(600000);  // gap between groups
}

static void diagPreInit_postEarlyResets(void) { diagRawBlink(1); }
PICO_RUNTIME_INIT_FUNC_RUNTIME(diagPreInit_postEarlyResets, "00150");

static void diagPreInit_postClocks(void) { diagRawBlink(2); }
PICO_RUNTIME_INIT_FUNC_RUNTIME(diagPreInit_postClocks, "00650");

#endif // RP2350_DIAG_BLINK

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

static bool usb_task_timer_cb(repeating_timer_t *rt)
{
    (void)rt;
    tud_task();
    return true;
}

static repeating_timer_t usb_task_timer;

void systemInit(void)
{
    persistentObjectInit();

    // Raise sys clock from SDK default (150 MHz) to 192 MHz.
    // The USB PLL is independent (clk_usb stays at 48 MHz from pll_usb) so
    // TinyUSB CDC is unaffected. The hardware timer (clk_timer, derived from
    // clk_ref at 12 MHz) is also unaffected, so micros() stays accurate.
    // Must be called before tusb_init() / stdio_init_all() so those see the
    // final clock rate.
    set_sys_clock_khz(192000, true);

    // SDK runtime already initialized clocks, GPIO, etc. via crt0 → runtime_init
    SystemCoreClock = clock_get_hz(clk_sys);

    // Initialize TinyUSB.  Two complementary mechanisms keep pico_stdio_usb
    // from interfering: LIB_PICO_STDIO_USB=0 (pico_sdk_config.h) means
    // stdio_init_all() below never calls stdio_usb_init() at all, and
    // LIB_TINYUSB_DEVICE=1 (cmake/rp2350.cmake) additionally compiles out the
    // tusb_init()/IRQ/alarm machinery inside stdio_usb.c — so there is no
    // second tusb_init() and no competing tud_task() timer.
    tusb_init();

    // Drive USB CDC from a dedicated 1ms hardware alarm so tud_task() runs even
    // when the cooperative scheduler is busy (e.g. during long tasks or CLI mode).
    // Only ONE caller of tud_task() exists: this timer.  pico_stdio_usb is disabled
    // (LIB_PICO_STDIO_USB=0) so stdio_usb_init() registers no competing IRQ/alarm.
    if (!add_repeating_timer_ms(-1, usb_task_timer_cb, NULL, &usb_task_timer)) {
        // Alarm pool exhausted — USB CDC will not receive background servicing.
        // Nothing useful to do here; continue boot without functional USB.
    }

    // stdio_init_all() is a no-op for USB (LIB_PICO_STDIO_USB=0 skips stdio_usb_init).
    // Kept here in case UART stdio is re-enabled in the future.
    stdio_init_all();
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

void delayNanos(timeDelta_t ns)
{
    if (ns <= 0) return;
    busy_wait_us_32(((uint32_t)ns + 999U) / 1000U);
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

void systemResetRequest(uint32_t requestId)
{
    persistentObjectWrite(PERSISTENT_OBJECT_RESET_REASON, requestId);
    systemReset();
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
    // INAV expects 12 bytes (U_ID_0/1/2 = 3x uint32_t).
    // Pico has 8 bytes — copy and pad with zeros.
    memcpy(id, board_id.id, 8);
    memset(id + 8, 0, 4);
}

// --- CMSIS DSP compatibility ---
//
// The bundled CMSIS DSP library predates ARMv8-M and conflicts with the newer
// CMSIS Core headers required by the Pico SDK, so its sources are not compiled
// for RP2350 (see cmake/rp2350.cmake).  Three elementwise vector functions are
// still referenced unconditionally by INAV's gyro/accel calibration code
// (gyro.c gyroUpdateAndCalibrate, acceleration.c applyAccelerationZero/
// accGetMeasuredAcceleration), so we provide real implementations here — these
// MUST compute, not no-op: a no-op arm_sub_f32/arm_scale_f32 would leave the
// calibrated gyro rate and measured acceleration unwritten (silently zero).
//
// The dynamic-notch FFT entry points (arm_rfft_fast_init_f32,
// arm_cfft_radix8by4_f32, arm_bitreversal_32, stage_rfft_f32,
// arm_cmplx_mag_f32) are NOT provided: USE_DYNAMIC_FILTERS is undefined for
// RP2350 (target.h), so the only caller (flight/gyroanalyse.c) is not compiled
// and no stub is needed — stubbing them would silently run fixed-frequency
// notches instead of the requested adaptive ones.

void arm_sub_f32(float32_t *pSrcA, float32_t *pSrcB,
                  float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++) {
        pDst[i] = pSrcA[i] - pSrcB[i];
    }
}

void arm_scale_f32(float32_t *pSrc, float32_t scale,
                    float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++) {
        pDst[i] = pSrc[i] * scale;
    }
}

void arm_mult_f32(float32_t *pSrcA, float32_t *pSrcB,
                   float32_t *pDst, uint32_t blockSize)
{
    for (uint32_t i = 0; i < blockSize; i++) {
        pDst[i] = pSrcA[i] * pSrcB[i];
    }
}

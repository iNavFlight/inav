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

    // Initialize TinyUSB.  LIB_TINYUSB_DEVICE=1 is defined in cmake/rp2350.cmake so
    // stdio_usb_init() (called via stdio_init_all() below) will skip its own tusb_init()
    // and, critically, will not set up a competing tud_task() timer.
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

// --- CMSIS DSP stubs ---
//
// The bundled CMSIS DSP library predates ARMv8-M and conflicts with the newer
// CMSIS Core headers required by the Pico SDK.  INAV common code that calls
// these functions is compiled for RP2350, so we provide empty stubs here.
// Analogous to SITL's stubs in target/SITL/target.c.

arm_status arm_rfft_fast_init_f32(arm_rfft_fast_instance_f32 *S, uint16_t fftLen)
{
    UNUSED(S);
    UNUSED(fftLen);
    return ARM_MATH_SUCCESS;
}

void arm_cfft_radix8by4_f32(arm_cfft_instance_f32 *S, float32_t *p1)
{
    UNUSED(S);
    UNUSED(p1);
}

void arm_bitreversal_32(uint32_t *pSrc, const uint16_t bitRevLen,
                         const uint16_t *pBitRevTable)
{
    UNUSED(pSrc);
    UNUSED(bitRevLen);
    UNUSED(pBitRevTable);
}

void stage_rfft_f32(arm_rfft_fast_instance_f32 *S, float32_t *p, float32_t *pOut)
{
    UNUSED(S);
    UNUSED(p);
    UNUSED(pOut);
}

void arm_cmplx_mag_f32(float32_t *pSrc, float32_t *pDst, uint32_t numSamples)
{
    UNUSED(pSrc);
    UNUSED(pDst);
    UNUSED(numSamples);
}

void arm_mult_f32(float32_t *pSrcA, float32_t *pSrcB,
                   float32_t *pDst, uint32_t blockSize)
{
    UNUSED(pSrcA);
    UNUSED(pSrcB);
    UNUSED(pDst);
    UNUSED(blockSize);
}

void arm_sub_f32(float32_t *pSrcA, float32_t *pSrcB,
                  float32_t *pDst, uint32_t blockSize)
{
    UNUSED(pSrcA);
    UNUSED(pSrcB);
    UNUSED(pDst);
    UNUSED(blockSize);
}

void arm_scale_f32(float32_t *pSrc, float32_t scale,
                    float32_t *pDst, uint32_t blockSize)
{
    UNUSED(pSrc);
    UNUSED(scale);
    UNUSED(pDst);
    UNUSED(blockSize);
}

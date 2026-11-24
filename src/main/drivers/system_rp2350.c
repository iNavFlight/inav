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
#include "common/printf.h"
#include "common/time.h"
#include "common/utils.h"
#include "drivers/system.h"
#include "drivers/persistent.h"
#include "drivers/usb_msc.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include "hardware/structs/qmi.h"
#include "hardware/structs/m33.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/unique_id.h"
#include "pico/bootrom.h"
#include "pico/runtime_init.h"
#include "tusb.h"

// SystemCoreClock — updated by systemInit from SDK clock tree
uint32_t SystemCoreClock = 150000000;

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

#define RP2350_RATED_CLOCK_MHZ   150

#ifdef USE_CPU_OVERCLOCK

// Multiples of 24 MHz keep the DSHOT PIO dividers integer (DSHOT600: 8, 10).
// Only 150 MHz at 1.10 V is characterised by Raspberry Pi; the voltages are
// chosen with margin and stay below the 1.30 V the regulator allows without
// unlocking its limit.
static const struct {
    uint16_t mhz;
    enum vreg_voltage voltage;
} overclockSteps[] = {
    [CPU_OVERCLOCK_192MHZ] = { 192, VREG_VOLTAGE_1_15 },
    [CPU_OVERCLOCK_240MHZ] = { 240, VREG_VOLTAGE_1_20 },
};

// Not a persistentObject: those are wiped after a watchdog timeout, and this
// marker has to survive exactly that. scratch[4..7] belong to the SDK.
#define OVERCLOCK_SCRATCH_INDEX     3
#define OVERCLOCK_PENDING_MAGIC     0x4F434C4B  // "OCLK"
#define OVERCLOCK_WATCHDOG_MS       500
// The flash read mode and divider are whatever the bootrom found while
// scanning the flash. A W25Q takes the plain 03h read up to 50 MHz only; the
// fast read commands go to 133 MHz, kept well below that here.
#define OVERCLOCK_FLASH_SCK_MAX_03H_HZ   50000000
#define OVERCLOCK_FLASH_SCK_MAX_FAST_HZ  100000000
#define OVERCLOCK_SELFTEST_BYTES    (64 * 1024)

static const char *cpuOverclockFailure;
static char flashFailureText[40];

// NULL when the requested clock is running
const char *systemCpuOverclockFailure(void)
{
    return cpuOverclockFailure;
}

// Reads the code through the XIP path, so it covers flash timing as well
static uint32_t overclockSelfTestSum(void)
{
    const volatile uint32_t *word = (const volatile uint32_t *)XIP_BASE;
    uint32_t sum = 0;
    for (unsigned i = 0; i < OVERCLOCK_SELFTEST_BYTES / sizeof(uint32_t); i++) {
        sum = (sum << 1 | sum >> 31) ^ word[i];
    }
    return sum;
}

static bool overclockFlashClockOk(uint32_t sysHz)
{
    uint32_t clkdiv = (qmi_hw->m[0].timing & QMI_M0_TIMING_CLKDIV_BITS) >> QMI_M0_TIMING_CLKDIV_LSB;
    if (clkdiv == 0) {
        clkdiv = 256;
    }
    const uint32_t readCmd = (qmi_hw->m[0].rcmd & QMI_M0_RCMD_PREFIX_BITS) >> QMI_M0_RCMD_PREFIX_LSB;
    const bool hasCmd = qmi_hw->m[0].rfmt & QMI_M0_RFMT_PREFIX_LEN_BITS;
    const uint32_t maxHz = (hasCmd && readCmd == 0x03) ? OVERCLOCK_FLASH_SCK_MAX_03H_HZ : OVERCLOCK_FLASH_SCK_MAX_FAST_HZ;

    if (sysHz / clkdiv <= maxHz) {
        return true;
    }
    tfp_sprintf(flashFailureText, "flash clkdiv=%d cmd=0x%02x", (int)clkdiv, (int)readCmd);
    return false;
}

/*
 * Has to run before any driver derives a divider from clk_sys (PIO, PWM, I2C).
 *
 * A clock the chip cannot hold would otherwise leave a board that never
 * reaches the CLI to take the setting back, so the switch is made under the
 * watchdog: if it bites, the marker in the scratch register makes the next
 * boot stay at the rated clock. `status` then reports the failure.
 */
void systemApplyCpuOverclock(cpuOverclock_e overclock)
{
    volatile uint32_t *marker = &watchdog_hw->scratch[OVERCLOCK_SCRATCH_INDEX];

    if (overclock == CPU_OVERCLOCK_OFF || overclock >= ARRAYLEN(overclockSteps)) {
        // A warm reboot out of an overclocked session keeps the raised voltage
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
        return;
    }

    if (*marker == OVERCLOCK_PENDING_MAGIC) {
        // The previous boot died in here. A watchdog reset keeps the regulator setting.
        *marker = 0;
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
        cpuOverclockFailure = "watchdog reset during the switch";
        return;
    }

    const uint32_t mhz = overclockSteps[overclock].mhz;

    uint vco, postDiv1, postDiv2;
    if (!check_sys_clock_khz(mhz * 1000, &vco, &postDiv1, &postDiv2)) {
        cpuOverclockFailure = "PLL";
        return;
    }
    if (!overclockFlashClockOk(mhz * 1000000)) {
        cpuOverclockFailure = flashFailureText;
        return;
    }

    const uint32_t expectedSum = overclockSelfTestSum();

    *marker = OVERCLOCK_PENDING_MAGIC;
    watchdog_enable(OVERCLOCK_WATCHDOG_MS, true);

    vreg_set_voltage(overclockSteps[overclock].voltage);
    busy_wait_ms(2);    // let the core supply settle before it is needed
    set_sys_clock_pll(vco, postDiv1, postDiv2);

    if (overclockSelfTestSum() != expectedSum) {
        set_sys_clock_khz(RP2350_RATED_CLOCK_MHZ * 1000, true);
        vreg_set_voltage(VREG_VOLTAGE_DEFAULT);
        cpuOverclockFailure = "self-test";
    }

    watchdog_disable();
    *marker = 0;

    SystemCoreClock = clock_get_hz(clk_sys);
}

#endif // USE_CPU_OVERCLOCK

#define USB_TASK_PERIOD_US  1000

/*
 * Own hardware alarm, not a repeating timer of the SDK alarm pool: the pool
 * loses a timer whose callback returns within the microsecond the alarm fired
 * in. ta_set_timeout() then reads "alarm == now" as an alarm that is still to
 * come and does not arm the next one, and nothing ever does. The callback is
 * that short whenever it only finds the busy flag set, and the CPU fast enough
 * from about 190 MHz: USB then stopped for good as soon as the host read the
 * CLI output slower than it was written (cpu_overclock, slow terminal).
 * Here the next alarm is always written.
 */
static void usbTaskAlarm(uint alarmNum)
{
    // The main loop is inside TinyUSB and may hold a mutex tud_task() needs
    if (!rp2350UsbBusy) {
        tud_task();
    }
    // true = the target is already over, only after a long stall in here
    while (hardware_alarm_set_target(alarmNum, make_timeout_time_us(USB_TASK_PERIOD_US))) {
    }
}

void systemInit(void)
{
    persistentObjectInit();

    // Sys clock at the rated 150 MHz, cpu_overclock raises it later in
    // systemApplyCpuOverclock() once the config is loaded.
    // The USB PLL is independent (clk_usb stays at 48 MHz from pll_usb) so
    // TinyUSB CDC is unaffected. The hardware timer (clk_timer, derived from
    // clk_ref at 12 MHz) is also unaffected, so micros() stays accurate.
    set_sys_clock_khz(RP2350_RATED_CLOCK_MHZ * 1000, true);

    // SDK runtime already initialized clocks, GPIO, etc. via crt0 → runtime_init
    SystemCoreClock = clock_get_hz(clk_sys);

    // Initialize TinyUSB.  Two complementary mechanisms keep pico_stdio_usb
    // from interfering: LIB_PICO_STDIO_USB=0 (pico_sdk_config.h) means
    // stdio_init_all() below never calls stdio_usb_init() at all, and
    // LIB_TINYUSB_DEVICE=1 (cmake/rp2350.cmake) additionally compiles out the
    // tusb_init()/IRQ/alarm machinery inside stdio_usb.c — so there is no
    // second tusb_init() and no competing tud_task() timer.
    tusb_init();

#ifdef USE_USB_MSC
    // Mass storage mode: stay off the bus until mscStart() has the card ready
    if (mscCheckBoot()) {
        tud_disconnect();
    }
#endif

    // Drive USB CDC from a dedicated 1 ms hardware alarm so tud_task() runs even
    // when the cooperative scheduler is busy (e.g. during long tasks or CLI mode).
    // pico_stdio_usb is disabled (LIB_PICO_STDIO_USB=0) so stdio_usb_init()
    // registers no competing IRQ/alarm.
    const int usbAlarm = hardware_alarm_claim_unused(false);
    if (usbAlarm >= 0) {
        hardware_alarm_set_callback((uint)usbAlarm, usbTaskAlarm);
        hardware_alarm_set_target((uint)usbAlarm, make_timeout_time_us(USB_TASK_PERIOD_US));
    }
    // No free alarm: USB CDC gets no background servicing. Nothing useful to
    // do here; continue boot without functional USB.

    // stdio_init_all() is a no-op for USB (LIB_PICO_STDIO_USB=0 skips stdio_usb_init).
    // Kept here in case UART stdio is re-enabled in the future.
    stdio_init_all();
}

// --- Timing functions ---

void failureMode(failureMode_e mode)
{
    UNUSED(mode);
    while (1) {}
}

bool isMPUSoftReset(void)
{
    return false;
}

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

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

/*
 * RP2350 motor output — DShot 600 via PIO0 — Milestone 7
 *
 * Maps INAV's motor output API onto PIO0 state machines using the DShot 600
 * protocol.  One SM per motor; up to 4 motors on a single PIO block.
 *
 * Motor pin assignments (from target.h):
 *   M1 → MOTOR1_PIN (GP8)    M2 → MOTOR2_PIN (GP9)
 *   M3 → MOTOR3_PIN (GP10)   M4 → MOTOR4_PIN (GP11)
 *
 * All SMs are stopped, their TX FIFOs loaded, then restarted together via
 * pio_enable_sm_mask_in_sync() so every motor frame begins on the same
 * system-clock edge — required by multirotor ESC sync expectations.
 *
 * PIO program is pre-assembled from betaflight/src/platform/PICO/dshot.pio
 * (program "dshot_600").  Embedding pre-assembled instructions avoids a
 * pioasm build step and matches the approach used by uart_pio_rp2350.c.
 *
 * DShot packet format (16 bits, MSB first):
 *   [15:5]  11-bit throttle (0 = stop, 1-47 = commands, 48-2047 = throttle)
 *   [4]     telemetry request
 *   [3:0]   4-bit XOR checksum of bits [15:4]
 */

#include "platform.h"

#ifdef RP2350

#include <stdbool.h>
#include <stdint.h>

#include "build/build_config.h"

#include "common/utils.h"

#include "drivers/io_def.h"
#include "drivers/io_types.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/timer.h"

#include "flight/mixer.h"

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"

/* ── Pre-assembled DShot 600 PIO program ─────────────────────────────────────
 *
 * Source: betaflight/src/platform/PICO/dshot.pio, program "dshot_600"
 * Compiled with: pioasm dshot.pio /dev/stdout
 * Copyright (c) 2021-2024 Betaflight contributors.  GPL-3.0.
 *
 * Bit period = DSHOT_BIT_PERIOD (40) PIO cycles.  The SM clock divider is set
 * so one PIO cycle ≈ 1.667 µs / 40 ≈ 41.67 ns at the configured system clock
 * (currently 192 MHz → clkdiv = 8.0 exactly).
 *
 * Signal encoding (non-bidirectional):
 *   '1' bit → pin HIGH for 30 cycles then LOW for 10
 *   '0' bit → pin HIGH for 15 cycles then LOW for 25
 *
 * The SM blocks on a TX-FIFO pull.  It discards the top 16 bits of the
 * 32-bit word ("out y, 16") then shifts out the lower 16, MSB first.
 */
#define DSHOT_BIT_PERIOD   40   /* PIO cycles per DShot 600 bit */
#define DSHOT_600_WRAP_TARGET 0
#define DSHOT_600_WRAP       12

static const uint16_t dshotProgramInstructions[] = {
        //     .wrap_target
    0xff00, //  0: set    pins, 0                [31]  — inter-frame gap (low)
    0xb442, //  1: nop                           [20]  — extend gap to ≥2 µs
    0x80a0, //  2: pull   block                       — wait for next packet
    0x6050, //  3: out    y, 16                       — discard top 16 bits
    0x6041, //  4: out    y, 1                        — shift next bit into y
    0x006a, //  5: jmp    !y, 10                      — branch on bit=0
    0xfd01, //  6: set    pins, 1                [29] — bit=1: high for 30 cyc
    0xe600, //  7: set    pins, 0                [6]  — then low for 10 cyc
    0x00e4, //  8: jmp    !osre, 4                    — more bits? loop
    0x0000, //  9: jmp    0                           — frame done → wrap
    0xee01, // 10: set    pins, 1                [14] — bit=0: high for 15 cyc
    0xf400, // 11: set    pins, 0                [20] — then low for 25 cyc
    0x01e4, // 12: jmp    !osre, 4               [1]  — more bits? loop
        //     .wrap
};

static const struct pio_program dshotProgram = {
    .instructions = dshotProgramInstructions,
    .length       = 13,
    .origin       = -1,
    .pio_version  = 0,
#if PICO_PIO_VERSION > 0
    .used_gpio_ranges = 0x0,
#endif
};

static inline pio_sm_config dshotGetDefaultConfig(uint offset)
{
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + DSHOT_600_WRAP_TARGET,
                           offset + DSHOT_600_WRAP);
    return c;
}

/* ── Module state ────────────────────────────────────────────────────────── */

#define DSHOT_MAX_MOTORS 4

typedef struct {
    uint     pin;
    uint     sm;
    uint16_t value;
    bool     requestTelemetry;
} dshotMotor_t;

static dshotMotor_t dshotMotors[DSHOT_MAX_MOTORS];
static uint         dshotMotorCount  = 0;
static bool         dshotInitialized = false;
static bool         dshotEnabled     = true;
static uint         dshotProgramOffset;

static motorPwmProtocolTypes_e initProtocol = PWM_TYPE_STANDARD;
static pwmInitError_e          rp2350PwmError = PWM_INIT_ERROR_NONE;

/* ── Standard-PWM motor state ────────────────────────────────────────────── */

/*
 * Non-DShot motor protocols (Standard PWM, Oneshot125, Brushed, etc.) use
 * the same RP2350 hardware PWM peripheral as servos, on the motor GPIO pins
 * assigned from timerHardware[] (GP8–GP11 / slices 4–5 by default).
 *
 * Update rate 400 Hz → period = 2.5 ms; with 1 µs/tick: wrap = 2499.
 * pwm_set_gpio_level(pin, us) writes pulse width in µs directly.
 * The INAV motor value passed to pwmWriteMotor() is already in µs (1000–2000).
 */
#define MOTOR_PWM_WRAP   2499u   /* 2.5 ms period = 400 Hz at 1 µs/tick */
#define MOTOR_MIN_US     1000u
#define MOTOR_MAX_US     2000u

static uint8_t motorPwmPins[DSHOT_MAX_MOTORS];
static uint8_t motorPwmCount       = 0;
static bool    motorPwmInitialized = false;
static bool    motorPwmEnabled     = true;

/* ── Servo state ─────────────────────────────────────────────────────────── */

/*
 * Servo PWM via hardware PWM slices (NOT PIO — different from DShot motors).
 * 8 slices × 2 channels = 16 possible servo outputs.
 *
 * Timing: clkdiv = sysHz / 1_000_000 → 1 µs per tick; wrap = 19999 → 50 Hz.
 * pwm_set_gpio_level(pin, us) sets pulse width directly in microseconds.
 */
#define RP2350_MAX_SERVOS    8u
#define SERVO_PWM_WRAP       19999u   /* 20 ms period = 50 Hz at 1 µs/tick */
#define SERVO_MIN_US         750u
#define SERVO_MAX_US         2250u
#define SERVO_DEFAULT_US     1500u    /* center pulse */

static uint8_t servoHwPins[RP2350_MAX_SERVOS];
static uint8_t servoCount     = 0;
static bool    servoInitialized = false;

/* Pending DShot command (e.g. spin direction).  Commands are sent 10×. */
static dshotCommands_e pendingCmd     = 0;
static int             pendingCmdReps = 0;

/* ── DShot packet construction ───────────────────────────────────────────── */

static uint16_t prepareDshotPacket(uint16_t value, bool requestTelemetry)
{
    uint16_t packet = ((uint16_t)(value << 1)) | (requestTelemetry ? 1u : 0u);
    uint16_t csum = 0;
    uint16_t csum_data = packet;
    for (int i = 0; i < 3; i++) {
        csum ^= csum_data;
        csum_data >>= 4;
    }
    csum &= 0xfu;
    return (packet << 4) | csum;
}

/* ── Motor output API ────────────────────────────────────────────────────── */

void pwmWriteMotor(uint8_t index, uint16_t value)
{
    if (dshotInitialized) {
        if (dshotEnabled && index < dshotMotorCount) {
            dshotMotors[index].value = value;
        }
    } else if (motorPwmInitialized && motorPwmEnabled && index < motorPwmCount) {
        /* value=0 means disarmed; output minimum throttle pulse */
        uint16_t us = (value == 0) ? MOTOR_MIN_US
                                   : (uint16_t)constrain(value, MOTOR_MIN_US, MOTOR_MAX_US);
        pwm_set_gpio_level(motorPwmPins[index], us);
    }
}

void pwmShutdownPulsesForAllMotors(uint8_t motorCount)
{
    if (dshotInitialized) {
        for (uint8_t i = 0; i < motorCount && i < dshotMotorCount; i++) {
            dshotMotors[i].value = 0;
        }
        pwmCompleteMotorUpdate();
    } else if (motorPwmInitialized) {
        for (uint8_t i = 0; i < motorCount && i < motorPwmCount; i++) {
            pwm_set_gpio_level(motorPwmPins[i], MOTOR_MIN_US);
        }
    }
}

void pwmCompleteMotorUpdate(void)
{
    /* Standard PWM: pwm_set_gpio_level() takes effect immediately — no work needed */
    if (!dshotInitialized || !dshotEnabled || dshotMotorCount == 0) {
        return;
    }

    uint32_t motorMask = 0;
    for (uint i = 0; i < dshotMotorCount; i++) {
        motorMask |= 1u << dshotMotors[i].sm;
    }

    /* Stop all SMs before loading FIFOs — required for simultaneous restart */
    pio_set_sm_mask_enabled(pio0, motorMask, false);

    for (uint i = 0; i < dshotMotorCount; i++) {
        uint16_t value = dshotMotors[i].value;
        bool telemetry = dshotMotors[i].requestTelemetry;

        /* Inject pending command instead of throttle value */
        if (pendingCmdReps > 0) {
            value    = (uint16_t)pendingCmd;
            telemetry = true;
        }

        uint16_t packet = prepareDshotPacket(value, telemetry);
        /* Discard any stale packet left in the FIFO from a missed cycle */
        pio_sm_drain_tx_fifo(pio0, dshotMotors[i].sm);
        /* PIO discards top 16 bits ("out y, 16") — packet in lower half */
        pio_sm_put(pio0, dshotMotors[i].sm, (uint32_t)packet);
        dshotMotors[i].requestTelemetry = false;
    }

    /* Restart all SMs on the same system-clock cycle */
    pio_enable_sm_mask_in_sync(pio0, motorMask);

    if (pendingCmdReps > 0) {
        pendingCmdReps--;
    }
}

/* ── Motor enable / disable ──────────────────────────────────────────────── */

void pwmDisableMotors(void)
{
    dshotEnabled    = false;
    motorPwmEnabled = false;
    /* Drive all standard-PWM motor pins to minimum throttle immediately */
    for (uint i = 0; i < motorPwmCount; i++) {
        pwm_set_gpio_level(motorPwmPins[i], MOTOR_MIN_US);
    }
}

void pwmEnableMotors(void)
{
    dshotEnabled    = true;
    motorPwmEnabled = true;
}

/* ── Protocol queries ────────────────────────────────────────────────────── */

bool isMotorProtocolDshot(void)
{
    switch (initProtocol) {
        case PWM_TYPE_DSHOT150:
        case PWM_TYPE_DSHOT300:
        case PWM_TYPE_DSHOT600:
            return true;
        default:
            return false;
    }
}

bool isMotorProtocolDigital(void)
{
    return isMotorProtocolDshot();
}

/* ── DShot command queue ─────────────────────────────────────────────────── */

void initDShotCommands(void)
{
    pendingCmd     = 0;
    pendingCmdReps = 0;
}

void sendDShotCommand(dshotCommands_e cmd)
{
    pendingCmd     = cmd;
    pendingCmdReps = 10;  /* DShot spec: send each command 10 times */
}

/* ── Telemetry / pin tag ─────────────────────────────────────────────────── */

void pwmRequestMotorTelemetry(int motorIndex)
{
    if ((uint)motorIndex < dshotMotorCount) {
        dshotMotors[motorIndex].requestTelemetry = true;
    }
}

ioTag_t pwmGetMotorPinTag(int motorIndex)
{
    UNUSED(motorIndex);
    return IOTAG_NONE;
}

/* ── GPIO helpers ────────────────────────────────────────────────────────── */

/*
 * Convert an INAV ioTag_t to the raw RP2350 GPIO number.
 *
 * ioTag_t encoding: ((gpioid + 1) << 4) | pin
 *   gpioid 0 → Port A (GPIO 0–15)
 *   gpioid 1 → Port B (GPIO 16–29)
 *
 * So GPIO number = gpioid * 16 + pin
 *   PA8  (GPIO 8)  → gpioid=0, pin=8  → 0*16+8  = 8
 *   PB0  (GPIO 16) → gpioid=1, pin=0  → 1*16+0  = 16
 */
static inline uint ioTagToGpioNum(ioTag_t tag)
{
    return (uint)(DEFIO_TAG_GPIOID(tag) * 16 + DEFIO_TAG_PIN(tag));
}

/* ── Motor and servo initialisation ─────────────────────────────────────────
 *
 * Iterates timerHardware[] (populated in target.c) to assign each GPIO
 * to motor or servo output.  The assignment for each PWM slice is decided
 * once (when the first pin of that slice is processed) based on:
 *
 *   timerOverrides(sliceId)->outputMode
 *     OUTPUT_MODE_MOTORS  → whole slice is motor output
 *     OUTPUT_MODE_SERVOS  → whole slice is servo output
 *     OUTPUT_MODE_AUTO    → motor if motors still needed, servo otherwise
 *
 * Both channels on a slice always receive the same assignment because they
 * share clkdiv/wrap registers and must run at the same update rate.
 *
 * Called from fc_init.c after the mixer has been configured, so
 * getMotorCount() returns the correct value.
 */

/* Per-slice assignment state (only used inside pwmMotorAndServoInit) */
typedef enum {
    SLICE_UNASSIGNED = 0,
    SLICE_AS_MOTOR,
    SLICE_AS_SERVO,
} sliceAssign_e;

bool pwmMotorAndServoInit(void)
{
    if (dshotInitialized || motorPwmInitialized) {
        return true;  /* already initialized */
    }

    initProtocol = motorConfig()->motorPwmProtocol;
    bool dshot   = isMotorProtocolDshot();
    uint motorCount = (uint)getMotorCount();

    /* ── DShot: load PIO program once ────────────────────────────────── */

    float dshotClkdiv = 0.0f;
    if (dshot) {
        /*
         * RP2350 PIO GPIO base: the only legal values are 0 (GP0–GP31) or 16
         * (GP16–GP47).  RP2350 has GPIOs 0–29 so base=0 always covers all pins.
         */
        if (pio_set_gpio_base(pio0, 0u) != PICO_OK) {
            rp2350PwmError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
            return false;
        }
        int offset = pio_add_program(pio0, &dshotProgram);
        if (offset < 0) {
            rp2350PwmError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
            return false;
        }
        dshotProgramOffset = (uint)offset;

        /*
         * Bit period varies by protocol; the PIO program uses DSHOT_BIT_PERIOD
         * (40) cycles per bit.  Adjust clkdiv so one PIO cycle equals the
         * required nanoseconds for the selected speed:
         *   DShot 150: 6.667 µs / 40 = 166.7 ns/cycle → clkdiv = 32.0 @ 192 MHz
         *   DShot 300: 3.333 µs / 40 =  83.3 ns/cycle → clkdiv = 16.0 @ 192 MHz
         *   DShot 600: 1.667 µs / 40 =  41.7 ns/cycle → clkdiv =  8.0 @ 192 MHz
         */
        float bitPeriodUs;
        switch (initProtocol) {
            case PWM_TYPE_DSHOT150: bitPeriodUs = 6.666667f; break;
            case PWM_TYPE_DSHOT300: bitPeriodUs = 3.333333f; break;
            default:                bitPeriodUs = 1.666667f; break; /* DShot 600 */
        }
        float clocks_per_us = (float)clock_get_hz(clk_sys) / 1000000.0f;
        dshotClkdiv = (bitPeriodUs / (float)DSHOT_BIT_PERIOD) * clocks_per_us;
    }

    /* ── Shared hardware-PWM clock divider (1 µs/tick) ───────────────── */

    float clkdiv = (float)clock_get_hz(clk_sys) / 1000000.0f;

    /* ── Slice assignment tracking ────────────────────────────────────── */

    sliceAssign_e sliceAssign[HARDWARE_TIMER_DEFINITION_COUNT];
    for (int s = 0; s < HARDWARE_TIMER_DEFINITION_COUNT; s++) {
        sliceAssign[s] = SLICE_UNASSIGNED;
    }

    uint32_t motorSliceInitMask = 0;
    uint32_t servoSliceInitMask = 0;
    uint motorIdx = 0;
    uint servoIdx = 0;

    /* ── Main assignment loop ─────────────────────────────────────────── */

    for (int i = 0; i < timerHardwareCount; i++) {
        const timerHardware_t *timHw = &timerHardware[i];
        uint8_t sliceId = timer2id(timHw->tim);
        if (sliceId >= HARDWARE_TIMER_DEFINITION_COUNT) {
            continue;  /* safety: unknown slice */
        }
        uint gpio = ioTagToGpioNum(timHw->tag);

        /* First pin on this slice: determine assignment for the whole slice */
        if (sliceAssign[sliceId] == SLICE_UNASSIGNED) {
            uint8_t mode = timerOverrides(sliceId)->outputMode;
            if (mode == OUTPUT_MODE_MOTORS) {
                sliceAssign[sliceId] = SLICE_AS_MOTOR;
            } else if (mode == OUTPUT_MODE_SERVOS) {
                sliceAssign[sliceId] = SLICE_AS_SERVO;
            } else {
                /* AUTO: motor if we still need motors, servo otherwise */
                sliceAssign[sliceId] = (motorIdx < motorCount) ? SLICE_AS_MOTOR
                                                                : SLICE_AS_SERVO;
            }
        }

        if (sliceAssign[sliceId] == SLICE_AS_MOTOR) {
            if (motorIdx >= DSHOT_MAX_MOTORS) {
                rp2350PwmError = PWM_INIT_ERROR_TOO_MANY_MOTORS;
                return false;
            }
            if (dshot) {
                /* ── DShot motor via PIO ─────────────────────────────── */
                int sm = pio_claim_unused_sm(pio0, false);
                if (sm < 0) {
                    rp2350PwmError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
                    return false;
                }
                pio_sm_config cfg = dshotGetDefaultConfig(dshotProgramOffset);
                sm_config_set_set_pins(&cfg, gpio, 1);
                pio_gpio_init(pio0, gpio);
                pio_sm_set_consecutive_pindirs(pio0, (uint)sm, gpio, 1, true);
                gpio_set_pulls(gpio, false, true);            /* pulldown — idle low */
                sm_config_set_out_shift(&cfg, false, false, 32); /* left shift, no autopull */
                sm_config_set_fifo_join(&cfg, PIO_FIFO_JOIN_TX);
                sm_config_set_clkdiv(&cfg, dshotClkdiv);
                if (pio_sm_init(pio0, (uint)sm, dshotProgramOffset, &cfg) != PICO_OK) {
                    rp2350PwmError = PWM_INIT_ERROR_TIMER_INIT_FAILED;
                    return false;
                }
                pio_sm_set_enabled(pio0, (uint)sm, true);

                dshotMotors[motorIdx].pin              = gpio;
                dshotMotors[motorIdx].sm               = (uint)sm;
                dshotMotors[motorIdx].value            = 0;
                dshotMotors[motorIdx].requestTelemetry = false;
            } else {
                /* ── Standard hardware-PWM motor ─────────────────────── */
                gpio_set_function(gpio, GPIO_FUNC_PWM);
                uint slice = pwm_gpio_to_slice_num(gpio);
                /* Init each slice only once — both channels share clkdiv/wrap */
                if (!(motorSliceInitMask & (1u << slice))) {
                    pwm_config cfg = pwm_get_default_config();
                    pwm_config_set_clkdiv(&cfg, clkdiv);
                    pwm_config_set_wrap(&cfg, MOTOR_PWM_WRAP);
                    pwm_init(slice, &cfg, false);
                    motorSliceInitMask |= (1u << slice);
                }
                pwm_set_gpio_level(gpio, MOTOR_MIN_US); /* minimum throttle at boot */
                pwm_set_enabled(slice, true);
                motorPwmPins[motorIdx] = (uint8_t)gpio;
            }
            motorIdx++;

        } else { /* SLICE_AS_SERVO */
            if (servoIdx >= RP2350_MAX_SERVOS) {
                continue;  /* silently skip: more pins than servo slots */
            }
            gpio_set_function(gpio, GPIO_FUNC_PWM);
            uint slice = pwm_gpio_to_slice_num(gpio);
            /* Init each slice only once — both channels share clkdiv/wrap */
            if (!(servoSliceInitMask & (1u << slice))) {
                pwm_config cfg = pwm_get_default_config();
                pwm_config_set_clkdiv(&cfg, clkdiv);
                pwm_config_set_wrap(&cfg, SERVO_PWM_WRAP);
                pwm_init(slice, &cfg, false);
                servoSliceInitMask |= (1u << slice);
            }
            pwm_set_gpio_level(gpio, SERVO_DEFAULT_US); /* center pulse before enabling */
            pwm_set_enabled(slice, true);
            servoHwPins[servoIdx++] = (uint8_t)gpio;
        }
    }

    /* ── Commit counts ────────────────────────────────────────────────── */

    if (dshot) {
        dshotMotorCount  = motorIdx;
        dshotInitialized = true;
    } else {
        motorPwmCount       = (uint8_t)motorIdx;
        motorPwmInitialized = true;
    }
    servoCount       = (uint8_t)servoIdx;
    servoInitialized = true;

    return true;
}

/* ── ESC update frequency ────────────────────────────────────────────────── */

uint32_t getEscUpdateFrequency(void)
{
    switch (initProtocol) {
        case PWM_TYPE_DSHOT600: return 16000;
        case PWM_TYPE_DSHOT300: return  8000;
        case PWM_TYPE_DSHOT150: return  4000;
        default:                return    400;
    }
}

/* ── Init error reporting ────────────────────────────────────────────────── */

pwmInitError_e getPwmInitError(void)
{
    return rp2350PwmError;
}

const char *getPwmInitErrorMessage(void)
{
    static const char *const msgs[] = {
        [PWM_INIT_ERROR_NONE]                     = "No error",
        [PWM_INIT_ERROR_TOO_MANY_MOTORS]          = "Too many motors",
        [PWM_INIT_ERROR_TOO_MANY_SERVOS]          = "Too many servos",
        [PWM_INIT_ERROR_NOT_ENOUGH_MOTOR_OUTPUTS] = "Not enough motor outputs",
        [PWM_INIT_ERROR_NOT_ENOUGH_SERVO_OUTPUTS] = "Not enough servo outputs",
        [PWM_INIT_ERROR_TIMER_INIT_FAILED]        = "Motor init failed",
    };
    if ((size_t)rp2350PwmError >= ARRAYLEN(msgs) || !msgs[rp2350PwmError]) {
        return "Unknown error";
    }
    return msgs[rp2350PwmError];
}

/* ── Servo output ────────────────────────────────────────────────────────── */

void pwmWriteServo(uint8_t index, uint16_t value)
{
    if (!servoInitialized || index >= servoCount) {
        return;
    }
    /*
     * value=0 means "disarm / center" — output center pulse.
     * Non-zero values are pulse widths in µs, clamped to servo range.
     * At 1 µs/tick, pwm_set_gpio_level(pin, us) sets the pulse directly.
     */
    uint16_t us = (value == 0) ? SERVO_DEFAULT_US
                               : (uint16_t)constrain(value, SERVO_MIN_US, SERVO_MAX_US);
    pwm_set_gpio_level(servoHwPins[index], us);
}

/* ── Beeper stubs (hardware not yet implemented) ─────────────────────────── */

void pwmWriteBeeper(bool onoffBeep)
{
    UNUSED(onoffBeep);
}

void beeperPwmInit(ioTag_t tag, uint16_t frequency)
{
    UNUSED(tag);
    UNUSED(frequency);
}

#endif /* RP2350 */

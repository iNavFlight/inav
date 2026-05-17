/*
 * This file is part of Cleanflight, Betaflight and INAV
 *
 * Cleanflight, Betaflight and INAV are free software. You can 
 * redistribute this software and/or modify this software under 
 * the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, 
 * or (at your option) any later version.
 *
 * Cleanflight, Betaflight and INAV are distributed in the hope that 
 * they will be useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include "platform.h"

#ifdef USE_PINIO

#include "build/debug.h"
#include "common/memory.h"
#include "drivers/io.h"
#include "drivers/pinio.h"
#ifdef USE_LED_STRIP
#include "drivers/light_ws2811strip.h"
#endif

// CCR = duty% directly; 2.4 MHz / 100 = 24 kHz PWM, above audible range
#define PINIO_PWM_PERIOD    100
#define PINIO_PWM_BASE_HZ   2400000

static inline uint8_t pinioEffectiveDuty(uint8_t duty, bool inverted)
{
    return inverted ? (100 - duty) : duty;
}

/*** Hardware definitions ***/
const pinioHardware_t pinioHardware[] = {
#if defined(PINIO1_PIN)
#if !defined(PINIO1_FLAGS)
#define PINIO1_FLAGS 0
#endif
    { .ioTag = IO_TAG(PINIO1_PIN), .ioMode = IOCFG_OUT_PP, .flags = PINIO1_FLAGS },
#endif

#if defined(PINIO2_PIN)
#if !defined(PINIO2_FLAGS)
#define PINIO2_FLAGS 0
#endif
    { .ioTag = IO_TAG(PINIO2_PIN), .ioMode = IOCFG_OUT_PP, .flags = PINIO2_FLAGS },
#endif

#if defined(PINIO3_PIN)
#if !defined(PINIO3_FLAGS)
#define PINIO3_FLAGS 0
#endif
    { .ioTag = IO_TAG(PINIO3_PIN), .ioMode = IOCFG_OUT_PP, .flags = PINIO3_FLAGS },
#endif

#if defined(PINIO4_PIN)
#if !defined(PINIO4_FLAGS)
#define PINIO4_FLAGS 0
#endif
    { .ioTag = IO_TAG(PINIO4_PIN), .ioMode = IOCFG_OUT_PP, .flags = PINIO4_FLAGS },
#endif
};

const int pinioHardwareCount = ARRAYLEN(pinioHardware);

/*** Runtime configuration ***/
typedef struct pinioRuntime_s {
    IO_t io;
    volatile timCCR_t *ccr; // Cached CCR register pointer (NULL for GPIO-only pins)
    bool inverted;
    bool active;        // Mode box state; defaults to true (no RC channel = always active)
    uint8_t duty;       // Timer mode: duty level (0–100) applied by pinioSet(true);
                        // updated by pinioSetDuty(). Defaults to 100 so a mode box
                        // activating with no programming framework condition gives full on.
} pinioRuntime_t;

static pinioRuntime_t pinioRuntime[PINIO_COUNT];
static int pinioRuntimeCount = 0;

// Configure one PINIO runtime slot in PWM mode. Returns false if no TCH available.
static bool pinioInitTimerPWM(int slot, IO_t io, const timerHardware_t *timHw, bool inverted)
{
    TCH_t *tch = timerGetTCH(timHw);
    if (!tch) {
        return false;
    }
    IOInit(io, OWNER_PINIO, RESOURCE_OUTPUT, RESOURCE_INDEX(slot));
    IOConfigGPIOAF(io, IOCFG_AF_PP, timHw->alternateFunction);
    timerConfigBase(tch, PINIO_PWM_PERIOD, PINIO_PWM_BASE_HZ);
    timerPWMConfigChannel(tch, 0);
    timerPWMStart(tch);
    timerEnable(tch);
    pinioRuntime[slot].ccr = timerCCR(tch);
    pinioRuntime[slot].io = io;
    pinioRuntime[slot].inverted = inverted;
    pinioRuntime[slot].active = true;
    pinioRuntime[slot].duty = 100; // default: mode box on = full on
    *pinioRuntime[slot].ccr = pinioEffectiveDuty(0, inverted); // start off
    return true;
}

void pinioInit(void)
{
    int runtimeCount = 0;

    // Pass 1: target-defined PINIO pins (PINIO1_PIN–PINIO4_PIN in target.h).
    // Timer-capable pins are configured as PWM; GPIO-only pins fall back to IOWrite.
    // pwmMotorAndServoInit() runs before pinioInit(), so motor/servo pins are already
    // owned and the OWNER_FREE check correctly skips dual-assigned pads.
    for (int i = 0; i < pinioHardwareCount && runtimeCount < PINIO_COUNT; i++) {
        IO_t io = IOGetByTag(pinioHardware[i].ioTag);
        if (!io) {
            continue;
        }

        bool inverted = (pinioHardware[i].flags & PINIO_FLAGS_INVERTED) != 0;
        const timerHardware_t *timHw = timerGetByTag(pinioHardware[i].ioTag, TIM_USE_ANY);
        if (timHw && IOGetOwner(io) == OWNER_FREE && pinioInitTimerPWM(runtimeCount, io, timHw, inverted)) {
            runtimeCount++;
            continue;
        }

        // GPIO fallback: no timer available or pin already claimed
        IOInit(io, OWNER_PINIO, RESOURCE_OUTPUT, RESOURCE_INDEX(runtimeCount));
        IOConfigGPIO(io, pinioHardware[i].ioMode);
        pinioRuntime[runtimeCount].inverted = inverted;
        pinioRuntime[runtimeCount].io = io;
        inverted ? IOHi(io) : IOLo(io);
        runtimeCount++;
    }

    // Pass 2: timer outputs assigned PINIO mode via the mixer (TIM_USE_PINIO flag).
    // These pins have no PINIO_N_PIN target definition; the user assigns them in the
    // configurator. pwmMotorAndServoInit() left them unclaimed; pick them up here.
    for (int i = 0; i < timerHardwareCount && runtimeCount < PINIO_COUNT; i++) {
        const timerHardware_t *timHw = &timerHardware[i];
        if (!TIM_IS_PINIO(timHw->usageFlags)) {
            continue;
        }
        IO_t io = IOGetByTag(timHw->tag);
        if (!io || IOGetOwner(io) != OWNER_FREE) {
            continue;
        }
        if (pinioInitTimerPWM(runtimeCount, io, timHw, false)) {
            runtimeCount++;
        }
    }

    pinioRuntimeCount = runtimeCount;
}

int pinioGetRuntimeCount(void)
{
    return pinioRuntimeCount;
}

void pinioSetDuty(int index, uint8_t duty)
{
#ifdef USE_LED_STRIP
    if (index == 0) {
        ws2811SetIdleHigh(duty > 0);
        return;
    }
#endif
    index--;  // user-facing 1-4 → runtime 0-3
    if ((unsigned)index >= (unsigned)pinioRuntimeCount) {
        return;
    }
    if (duty > 100) {
        duty = 100;
    }
    if (pinioRuntime[index].ccr) {
        pinioRuntime[index].duty = duty;
        if (pinioRuntime[index].active) {
            *pinioRuntime[index].ccr = pinioEffectiveDuty(duty, pinioRuntime[index].inverted);
        }
    } else {
        IOWrite(pinioRuntime[index].io, (duty > 0) ^ pinioRuntime[index].inverted);
    }
}

// pinioSet is called by PINIOBOX when an RC mode box is assigned to this channel.
// For GPIO channels: drives the pin high or low directly.
// For timer channels: active = output at stored duty (set by pinioSetDuty, default 100%);
// inactive = 0%. The stored duty is NOT modified, so deactivating and reactivating the
// mode box restores the programmed level. Channels with no mode box assigned are never
// called from PINIOBOX, giving the programming framework exclusive uninterrupted control.
void pinioSet(int index, bool newState)
{
    if ((unsigned)index >= (unsigned)pinioRuntimeCount) {
        return;
    }
    if (pinioRuntime[index].ccr) {
        pinioRuntime[index].active = newState;
        uint8_t duty = newState ? pinioRuntime[index].duty : 0;
        *pinioRuntime[index].ccr = pinioEffectiveDuty(duty, pinioRuntime[index].inverted);
    } else {
        IOWrite(pinioRuntime[index].io, newState ^ pinioRuntime[index].inverted);
    }
}
#endif

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
    TCH_t *tch;         // Non-NULL when pin is configured in PWM mode
    bool inverted;
    uint8_t duty;       // Timer mode: duty level (0–100) applied by pinioSet(true);
                        // updated by pinioSetDuty(). Defaults to 100 so a mode box
                        // activating with no programming framework condition gives full on.
} pinioRuntime_t;

static pinioRuntime_t pinioRuntime[PINIO_COUNT];
static int pinioRuntimeCount = 0;

void pinioInit(void)
{
    int runtimeCount = 0;

    // Pass 1: target-defined PINIO pins from pinioHardware[] (PINIO1_PIN–PINIO4_PIN).
    // These may be GPIO-only pads or timer-capable pads; timer is preferred when available.
    for (int i = 0; i < pinioHardwareCount && runtimeCount < PINIO_COUNT; i++) {
        IO_t io = IOGetByTag(pinioHardware[i].ioTag);
        if (!io) {
            continue;
        }

        // If the pin has a timer and is unclaimed, configure it as a PWM output.
        // pwmMotorAndServoInit() runs before pinioInit(), so claimed motor/servo pins
        // are already owned and the OWNER_FREE check correctly skips them.
        const timerHardware_t *timHw = timerGetByTag(pinioHardware[i].ioTag, TIM_USE_ANY);
        if (timHw && IOGetOwner(io) == OWNER_FREE) {
            TCH_t *tch = timerGetTCH(timHw);
            if (tch) {
                IOInit(io, OWNER_PINIO, RESOURCE_OUTPUT, RESOURCE_INDEX(runtimeCount));
                IOConfigGPIOAF(io, IOCFG_AF_PP, timHw->alternateFunction);
                // period=100 means CCR value is directly the duty percentage (0–100);
                // 2.4 MHz / 100 = 24 kHz PWM, above audible range
                timerConfigBase(tch, 100, 2400000);
                timerPWMConfigChannel(tch, 0);
                timerPWMStart(tch);
                timerEnable(tch);
                pinioRuntime[runtimeCount].tch = tch;
                pinioRuntime[runtimeCount].io = io;
                pinioRuntime[runtimeCount].inverted = (pinioHardware[i].flags & PINIO_FLAGS_INVERTED) != 0;
                pinioRuntime[runtimeCount].duty = 100; // default: mode box on = full on
                // Start in the "off" state: HIGH if inverted, LOW if normal
                *timerCCR(tch) = pinioRuntime[runtimeCount].inverted ? 100 : 0;
                runtimeCount++;
                continue;
            }
        }

        // GPIO fallback: no timer available or pin already claimed by another subsystem
        IOInit(io, OWNER_PINIO, RESOURCE_OUTPUT, RESOURCE_INDEX(runtimeCount));
        IOConfigGPIO(io, pinioHardware[i].ioMode);
        if (pinioHardware[i].flags & PINIO_FLAGS_INVERTED) {
            pinioRuntime[runtimeCount].inverted = true;
            IOHi(io);
        } else {
            pinioRuntime[runtimeCount].inverted = false;
            IOLo(io);
        }
        pinioRuntime[runtimeCount].io = io;
        runtimeCount++;
    }

    // Pass 2: timer outputs assigned to PINIO mode via the mixer (TIM_USE_PINIO flag).
    // These pins are NOT pre-defined in target.h; the user assigns them in the configurator.
    // pwmMotorAndServoInit() left them unclaimed; we pick them up here in timerHardware[] order.
    for (int i = 0; i < timerHardwareCount && runtimeCount < PINIO_COUNT; i++) {
        const timerHardware_t *timHw = &timerHardware[i];
        if (!TIM_IS_PINIO(timHw->usageFlags)) {
            continue;
        }

        IO_t io = IOGetByTag(timHw->tag);
        if (!io || IOGetOwner(io) != OWNER_FREE) {
            // Skip invalid pins and pins already claimed by Pass 1
            continue;
        }

        TCH_t *tch = timerGetTCH(timHw);
        if (!tch) {
            continue;
        }

        IOInit(io, OWNER_PINIO, RESOURCE_OUTPUT, RESOURCE_INDEX(runtimeCount));
        IOConfigGPIOAF(io, IOCFG_AF_PP, timHw->alternateFunction);
        timerConfigBase(tch, 100, 2400000);
        timerPWMConfigChannel(tch, 0);
        timerPWMStart(tch);
        timerEnable(tch);
        pinioRuntime[runtimeCount].tch = tch;
        pinioRuntime[runtimeCount].io = io;
        pinioRuntime[runtimeCount].inverted = false;
        pinioRuntime[runtimeCount].duty = 100; // default: mode box on = full on
        *timerCCR(tch) = 0;
        runtimeCount++;
    }

    pinioRuntimeCount = runtimeCount;
}

void pinioSetDuty(int index, uint8_t duty)
{
    if (index < 0 || index >= pinioRuntimeCount) {
        return;
    }

    if (!pinioRuntime[index].io) {
        return;
    }

    // Clamp to valid range
    if (duty > 100) {
        duty = 100;
    }

    if (pinioRuntime[index].tch) {
        pinioRuntime[index].duty = duty;
        *timerCCR(pinioRuntime[index].tch) = pinioRuntime[index].inverted ? (100 - duty) : duty;
    } else {
        // GPIO pin: treat as on/off (0 = off, any non-zero = on)
        IOWrite(pinioRuntime[index].io, (duty > 0) ^ pinioRuntime[index].inverted);
    }
}

// pinioSet is called by PINIOBOX when an RC mode is assigned to this channel.
// For GPIO channels: drives the pin high or low directly.
// For timer channels: active = output at stored duty level (set by pinioSetDuty,
// defaults to 100%); inactive = output at 0%. This integrates mode-box on/off
// with programming-framework duty control: the mode box gates the output, and
// pinioSetDuty() sets the level applied when the gate is open.
// Channels with no mode box assigned are never called from PINIOBOX, so the
// programming framework retains exclusive uninterrupted control in that case.
void pinioSet(int index, bool newState)
{
    if (index < 0 || index >= pinioRuntimeCount || !pinioRuntime[index].io) {
        return;
    }

    if (pinioRuntime[index].tch) {
        uint8_t duty = newState ? pinioRuntime[index].duty : 0;
        *timerCCR(pinioRuntime[index].tch) = pinioRuntime[index].inverted ? (100 - duty) : duty;
    } else {
        IOWrite(pinioRuntime[index].io, newState ^ pinioRuntime[index].inverted);
    }
}
#endif

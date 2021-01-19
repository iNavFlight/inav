/*
 * This file is part of INAV.
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
 *
 */

#include <stdint.h>

#include "platform.h"


#include "build/atomic.h"

#include "drivers/nvic.h"
#include "drivers/time.h"

// cycles per microsecond, this is deliberately uint32_t to avoid type conversions
// This is not static so system.c can set it up for us.
uint32_t usTicks = 0;

// current uptime for 1kHz systick timer. will rollover after 49 days. hopefully we won't care.
STATIC_UNIT_TESTED volatile timeMs_t sysTickUptime = 0;
STATIC_UNIT_TESTED volatile uint32_t sysTickValStamp = 0;

// Return system uptime in milliseconds (rollover in 49 days)
timeMs_t millis(void)
{
    return sysTickUptime;
}

// SysTick

static volatile int sysTickPending = 0;

void SysTick_Handler(void)
{
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        sysTickUptime++;
        sysTickValStamp = SysTick->VAL;
        sysTickPending = 0;
        (void)(SysTick->CTRL);
    }
#ifdef USE_HAL_DRIVER
    // used by the HAL for some timekeeping and timeouts, should always be 1ms
    HAL_IncTick();
#endif
}

uint32_t ticks(void)
{
#ifdef UNIT_TEST
    return 0;
#else
    return DWT->CYCCNT;
#endif
}

void delayNanos(timeDelta_t ns)
{
    const uint32_t startTicks = ticks();
    const uint32_t ticksToWait = (ns * usTicks) / 1000;
    while (ticks() - startTicks <= ticksToWait);
}

// Return system uptime in microseconds
timeUs_t microsISR(void)
{
    register uint32_t ms, pending, cycle_cnt;

    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        cycle_cnt = SysTick->VAL;

        if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
            // Update pending.
            // Record it for multiple calls within the same rollover period
            // (Will be cleared when serviced).
            // Note that multiple rollovers are not considered.

            sysTickPending = 1;

            // Read VAL again to ensure the value is read after the rollover.

            cycle_cnt = SysTick->VAL;
        }

        ms = sysTickUptime;
        pending = sysTickPending;
    }

    // XXX: Be careful to not trigger 64 bit division
    const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
    return ((timeUs_t)(ms + pending) * 1000LL) + ((timeUs_t)partial);
}

timeUs_t micros(void)
{
    register uint32_t ms, cycle_cnt;

    // Call microsISR() in interrupt and elevated (non-zero) BASEPRI context

#ifndef UNIT_TEST
    if ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) || (__get_BASEPRI())) {
        return microsISR();
    }
#endif

    do {
        ms = sysTickUptime;
        cycle_cnt = SysTick->VAL;
    } while (ms != sysTickUptime || cycle_cnt > sysTickValStamp);

    // XXX: Be careful to not trigger 64 bit division
    const uint32_t partial = (usTicks * 1000U - cycle_cnt) / usTicks;
    return ((timeUs_t)ms * 1000LL) + ((timeUs_t)partial);
}

#if 1
void delayMicroseconds(timeUs_t us)
{
    timeUs_t now = micros();
    while (micros() - now < us);
}
#else
void delayMicroseconds(timeUs_t us)
{
    uint32_t elapsed = 0;
    uint32_t lastCount = SysTick->VAL;

    for (;;) {
        register uint32_t current_count = SysTick->VAL;
        timeUs_t elapsed_us;

        // measure the time elapsed since the last time we checked
        elapsed += current_count - lastCount;
        lastCount = current_count;

        // convert to microseconds
        elapsed_us = elapsed / usTicks;
        if (elapsed_us >= us)
            break;

        // reduce the delay by the elapsed time
        us -= elapsed_us;

        // keep fractional microseconds for the next iteration
        elapsed %= usTicks;
    }
}
#endif

void delay(timeMs_t ms)
{
    while (ms--)
        delayMicroseconds(1000);
}

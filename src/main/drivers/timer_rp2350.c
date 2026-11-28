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
 * RP2350 PWM slice group identifiers and timer utilities.
 *
 * Provides the per-slice TIM_TypeDef identity tokens used by timerHardware[]
 * in each board's target.c.  They live in drivers/ rather than a target
 * directory so every RP2350 board can reuse them without duplication.
 *
 * Analogous to the TIM4/TIM5/… peripheral register structs on STM32 (which
 * live in CMSIS headers, not in any target directory).  Here the TIM_TypeDef
 * instances contain no real register content — only their addresses matter,
 * used as unique group identifiers by timer2id() and pwmMotorAndServoInit().
 *
 * The TIMn pointer macros and extern declarations are in rp2350.h, which
 * also documents the GPIO → slice rule.
 */

#include <stdint.h>
#include <stdbool.h>

#include <platform.h>

#include "drivers/timer.h"

/*
 * One TIM_TypeDef instance per RP2350 PWM slice group.
 * Both channels (A = even GPIO, B = odd GPIO) on a slice share clkdiv/wrap,
 * so they must run at the same update rate — same constraint as STM32 timers.
 * GPIO 16-31 reuse slices 0-7, so each slice is reachable from two GPIO pairs;
 * a board must not put both pairs of one slice into different groups.
 */
TIM_TypeDef rp2350Pwm4  = { NULL };  /* slice 4: GP8/GP9   or GP24/GP25 */
TIM_TypeDef rp2350Pwm5  = { NULL };  /* slice 5: GP10/GP11 or GP26/GP27 */
TIM_TypeDef rp2350Pwm6  = { NULL };  /* slice 6: GP12/GP13 or GP28/GP29 */
TIM_TypeDef rp2350Pwm7  = { NULL };  /* slice 7: GP14/GP15 or GP30/GP31 */
TIM_TypeDef rp2350Pwm10 = { NULL };  /* slice 2: GP4/GP5   or GP20/GP21 (historic name) */

void timerInit(void)
{
    /* NOP — RP2350 PWM slices are configured by pwmMotorAndServoInit() */
}

/* Returns the 0-based group index for use as a timerOverrides() array index. */
uint8_t timer2id(const HAL_Timer_t *tim)
{
    if (tim == TIM4)  return 0;
    if (tim == TIM5)  return 1;
    if (tim == TIM6)  return 2;
    if (tim == TIM7)  return 3;
    if (tim == TIM10) return 4;
    return (uint8_t)-1;
}

const timerHardware_t *timerGetByTag(ioTag_t tag, timerUsageFlag_e flag)
{
    if (!tag) {
        return NULL;
    }

    for (int i = 0; i < timerHardwareCount; i++) {
        if (timerHardware[i].tag == tag && (timerHardware[i].usageFlags & flag || flag == 0)) {
            return &timerHardware[i];
        }
    }
    return NULL;
}

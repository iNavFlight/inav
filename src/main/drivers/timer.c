/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "build/atomic.h"

#include "common/utils.h"
#include "common/memory.h"

#include "drivers/io.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/nvic.h"

#include "drivers/timer.h"
#include "drivers/timer_impl.h"

#define TIM_N(n) (1 << (n))

/*
    Groups that allow running different period (ex 50Hz servos + 400Hz throttle + etc):
    TIM1 2 channels
    TIM2 4 channels
    TIM3 4 channels
    TIM4 4 channels
*/

timerConfig_t * timerConfig[HARDWARE_TIMER_DEFINITION_COUNT];

// return index of timer in timer table. Lowest timer has index 0
uint8_t lookupTimerIndex(const TIM_TypeDef *tim)
{
    int i;

    // let gcc do the work, switch should be quite optimized
    for (i = 0; i < HARDWARE_TIMER_DEFINITION_COUNT; i++) {
        if (tim == timerDefinitions[i].tim) {
            return i;
        }
    }

    // make sure final index is out of range
    return ~1;
}

static inline uint8_t lookupChannelIndex(const uint16_t channel)
{
    return channel >> 2;
}

void timerNVICConfigure(uint8_t irq, int irqPriority)
{
    impl_timerNVICConfigure(irq, irqPriority);
}

void timerConfigBase(TIM_TypeDef *tim, uint16_t period, uint8_t mhz)
{
    impl_timerConfigBase(tim, period, mhz);
}

// old interface for PWM inputs. It should be replaced
void timerConfigure(const timerHardware_t *timerHardwarePtr, uint16_t period, uint8_t mhz)
{
    unsigned timer = lookupTimerIndex(timerHardwarePtr->tim);

    impl_timerConfigBase(timerDefinitions[timer].tim, period, mhz);
    impl_enableTimer(timerDefinitions[timer].tim);

    if (timerDefinitions[timer].irq != 0) {
        impl_timerNVICConfigure(timerDefinitions[timer].irq, NVIC_PRIO_TIMER);
    }

    if (timerDefinitions[timer].secondIrq != 0) {
        impl_timerNVICConfigure(timerDefinitions[timer].secondIrq, NVIC_PRIO_TIMER);
    }
}

void timerChCCHandlerInit(timerCCHandlerRec_t *self, timerCCHandlerCallback *fn)
{
    self->fn = fn;
}

void timerChOvrHandlerInit(timerOvrHandlerRec_t *self, timerOvrHandlerCallback *fn)
{
    self->fn = fn;
    self->next = NULL;
}

// update overflow callback list
// some synchronization mechanism is neccesary to avoid disturbing other channels (BASEPRI used now)
static void timerChConfig_UpdateOverflow(timerConfig_t *cfg, TIM_TypeDef *tim) {
    timerOvrHandlerRec_t **chain = &cfg->overflowCallbackActive;
    ATOMIC_BLOCK(NVIC_PRIO_TIMER) {
        for (int i = 0; i < CC_CHANNELS_PER_TIMER; i++)
            if (cfg->overflowCallback[i]) {
                *chain = cfg->overflowCallback[i];
                chain = &cfg->overflowCallback[i]->next;
            }
        *chain = NULL;
    }

    // enable or disable IRQ
    if (cfg->overflowCallbackActive) {
        impl_timerEnableIT(tim, IMPL_TIM_IT_UPDATE_INTERRUPT);
    }
    else {
        impl_timerDisableIT(tim, IMPL_TIM_IT_UPDATE_INTERRUPT);
    }
}

timerConfig_t * timerGetConfigContext(int timerIndex)
{
    // If timer context does not exist - allocate memory
    if (timerConfig[timerIndex] == NULL) {
        timerConfig[timerIndex] = memAllocate(sizeof(timerConfig_t), OWNER_TIMER);
    }

    return timerConfig[timerIndex];
}

// config edge and overflow callback for channel. Try to avoid overflowCallback, it is a bit expensive
void timerChConfigCallbacks(const timerHardware_t *timHw, timerCCHandlerRec_t *edgeCallback, timerOvrHandlerRec_t *overflowCallback)
{
    uint8_t timerIndex = lookupTimerIndex(timHw->tim);

    if (timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
        return;
    }

    uint8_t channelIndex = lookupChannelIndex(timHw->channel);
    if (edgeCallback == NULL)   // disable irq before changing callback to NULL
        impl_timerDisableIT(timHw->tim, TIM_IT_CCx(timHw->channel));

    timerConfig_t * cfg = timerGetConfigContext(timerIndex);

    if (cfg) {
        // setup callback info
        cfg->edgeCallback[channelIndex] = edgeCallback;
        cfg->overflowCallback[channelIndex] = overflowCallback;

        // enable channel IRQ
        if (edgeCallback)
            impl_timerEnableIT(timHw->tim, TIM_IT_CCx(timHw->channel));

        timerChConfig_UpdateOverflow(cfg, timHw->tim);
    }
    else {
        // OOM error, disable IRQs for this timer
        impl_timerDisableIT(timHw->tim, TIM_IT_CCx(timHw->channel));
    }
}

// Configure input captupre
void timerChConfigIC(const timerHardware_t *timHw, bool polarityRising, unsigned inputFilterTicks)
{
    impl_timerChConfigIC(timHw, polarityRising, inputFilterTicks);
}

uint16_t timerGetPeriod(const timerHardware_t *timHw)
{
    return timHw->tim->ARR;
}

void timerInit(void)
{
    memset(timerConfig, 0, sizeof (timerConfig));

    /* enable the timer peripherals */
    for (int i = 0; i < timerHardwareCount; i++) {
        unsigned timer = lookupTimerIndex(timerHardware[i].tim);
        RCC_ClockCmd(timerDefinitions[timer].rcc, ENABLE);
    }

    /* Before 2.0 timer outputs were initialized to IOCFG_AF_PP_PD even if not used */
    /* To keep compatibility make sure all timer output pins are mapped to INPUT with weak pull-down */
    for (int i = 0; i < timerHardwareCount; i++) {
        const timerHardware_t *timerHardwarePtr = &timerHardware[i];
        IOConfigGPIO(IOGetByTag(timerHardwarePtr->tag), IOCFG_IPD);
    }
}

const timerHardware_t *timerGetByTag(ioTag_t tag, timerUsageFlag_e flag)
{
    if (!tag) {
        return NULL;
    }

    for (int i = 0; i < timerHardwareCount; i++) {
        if (timerHardware[i].tag == tag) {
            if (timerHardware[i].usageFlags & flag || flag == 0) {
                return &timerHardware[i];
            }
        }
    }
    return NULL;
}

void timerPWMConfigChannel(TIM_TypeDef * tim, uint8_t channel, bool isNChannel, bool inverted, uint16_t value)
{
    impl_timerPWMConfigChannel(tim, channel, isNChannel, inverted, value);
}

void timerEnable(TIM_TypeDef * tim)
{
    impl_enableTimer(tim);
}

void timerPWMStart(TIM_TypeDef * tim, uint8_t channel, bool isNChannel)
{
    impl_timerPWMStart(tim, channel, isNChannel);
}

uint16_t timerDmaSource(uint8_t channel)
{
    return impl_timerDmaSource(channel);
}

volatile timCCR_t * timerCCR(TIM_TypeDef *tim, uint8_t channel)
{
    return impl_timerCCR(tim, channel);
}

uint16_t timerGetPrescalerByDesiredMhz(TIM_TypeDef *tim, uint16_t mhz)
{
    // protection here for desired MHZ > SystemCoreClock???
    if ((uint32_t)(mhz * 1000000) > (SystemCoreClock / timerClockDivisor(tim))) {
        return 0;
    }
    return (uint16_t)(round((SystemCoreClock / timerClockDivisor(tim) / (mhz * 1000000)) - 1));
}

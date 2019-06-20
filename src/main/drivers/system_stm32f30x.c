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

#include "platform.h"

#include "drivers/nvic.h"
#include "drivers/system.h"

#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)
void SetSysClock(uint8_t underclock);

inline static void NVIC_DisableAllIRQs(void)
{
    // We access CMSIS NVIC registers directly here
    for (int x = 0; x < 8; x++) {
        // Mask all IRQs controlled by a ICERx
        NVIC->ICER[x] = 0xFFFFFFFF;
        // Clear all pending IRQs controlled by a ICPRx
        NVIC->ICPR[x] = 0xFFFFFFFF;
    }
}

void systemReset(void)
{
    // Disable all NVIC interrupts
    __disable_irq();
    NVIC_DisableAllIRQs();

    // Generate system reset
    SCB->AIRCR = AIRCR_VECTKEY_MASK | (uint32_t)0x04;
}

void systemResetToBootloader(void)
{
    __disable_irq();
    NVIC_DisableAllIRQs();

    *((uint32_t *)0x20009FFC) = 0xDEADBEEF; // 40KB SRAM STM32F30X

    // Generate system reset
    SCB->AIRCR = AIRCR_VECTKEY_MASK | (uint32_t)0x04;
}


void enableGPIOPowerUsageAndNoiseReductions(void)
{
    RCC_AHBPeriphClockCmd(
        RCC_AHBPeriph_GPIOA |
        RCC_AHBPeriph_GPIOB |
        RCC_AHBPeriph_GPIOC |
        RCC_AHBPeriph_GPIOD |
        RCC_AHBPeriph_GPIOE |
        RCC_AHBPeriph_GPIOF,
        ENABLE
    );

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_All;

    GPIO_InitStructure.GPIO_Pin &= ~(GPIO_Pin_13 | GPIO_Pin_14); // leave JTAG pins alone
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_All;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

bool isMPUSoftReset(void)
{
    if (cachedRccCsrValue & RCC_CSR_SFTRSTF)
        return true;
    else
        return false;
}

static void systemTimekeepingSetup(void)
{
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);

    cycleCounterInit();
    SysTick_Config(clocks.SYSCLK_Frequency / 1000);
}

void systemClockSetup(uint8_t cpuUnderclock)
{
    // Configure the RCC. Note that this should be called only once per boot
    SetSysClock(cpuUnderclock);

    // Re-initialize system timekeeping - CPU clock changed
    systemTimekeepingSetup();
}

void systemInit(void)
{
    checkForBootLoaderRequest();

    // Enable FPU
    SCB->CPACR = (0x3 << (10 * 2)) | (0x3 << (11 * 2));

    // Configure NVIC preempt/priority groups
    NVIC_PriorityGroupConfig(NVIC_PRIORITY_GROUPING);

    // cache RCC->CSR value to use it in isMPUSoftreset() and others
    cachedRccCsrValue = RCC->CSR;
    RCC_ClearFlag();

    enableGPIOPowerUsageAndNoiseReductions();
    memset(extiHandlerConfigs, 0x00, sizeof(extiHandlerConfigs));

    // Pre-setup SysTick and system time - final setup is done in systemClockSetup
    systemTimekeepingSetup();
}

void checkForBootLoaderRequest(void)
{
}

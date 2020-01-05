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

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/system.h"


#define AIRCR_VECTKEY_MASK    ((uint32_t)0x05FA0000)
void SystemClock_Config(void);

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
    __disable_irq();
    NVIC_DisableAllIRQs();
    NVIC_SystemReset();
}

void systemResetToBootloader(void)
{
    __disable_irq();
    NVIC_DisableAllIRQs();

    (*(__IO uint32_t *) (BKPSRAM_BASE + 4)) = 0xDEADBEEF;   // flag that will be readable after reboot

    NVIC_SystemReset();
}

void enableGPIOPowerUsageAndNoiseReductions(void)
{

    // AHB1
    __HAL_RCC_BKPSRAM_CLK_ENABLE();
    __HAL_RCC_DTCMRAMEN_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
    __HAL_RCC_USB_OTG_HS_ULPI_CLK_ENABLE();

    //APB1
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_UART4_CLK_ENABLE();
    __HAL_RCC_UART5_CLK_ENABLE();
    __HAL_RCC_UART7_CLK_ENABLE();
    __HAL_RCC_UART8_CLK_ENABLE();

    //APB2
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_SDMMC1_CLK_ENABLE();
}

bool isMPUSoftReset(void)
{
    if (RCC->CSR & RCC_CSR_SFTRSTF)
        return true;
    else
        return false;
}

void systemClockSetup(uint8_t cpuUnderclock)
{
    (void)cpuUnderclock;
    // This is a stub
}

void systemInit(void)
{
    checkForBootLoaderRequest();

    //Called by SystemInit from startup_stm32f7xx.s
    //SystemClock_Config();

    // Configure NVIC preempt/priority groups
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITY_GROUPING);

    // cache RCC->CSR value to use it in isMPUSoftreset() and others
    cachedRccCsrValue = RCC->CSR;

    /* Accounts for OP Bootloader, set the Vector Table base address as specified in .ld file */
    //extern void *isr_vector_table_base;
    //NVIC_SetVectorTable((uint32_t)&isr_vector_table_base, 0x0);
    //__HAL_RCC_USB_OTG_FS_CLK_DISABLE;

    //RCC_ClearFlag();

    enableGPIOPowerUsageAndNoiseReductions();

    // Init cycle counter
    cycleCounterInit();

    // SysTick
    //SysTick_Config(SystemCoreClock / 1000);
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

void(*bootJump)(void);
void checkForBootLoaderRequest(void)
{
    uint32_t bt;
    __PWR_CLK_ENABLE();
    __BKPSRAM_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    bt = (*(__IO uint32_t *) (BKPSRAM_BASE + 4)) ;
    if ( bt == 0xDEADBEEF ) {
        (*(__IO uint32_t *) (BKPSRAM_BASE + 4)) =  0xCAFEFEED; // Reset our trigger

        void (*SysMemBootJump)(void);
        __SYSCFG_CLK_ENABLE();
        SYSCFG->MEMRMP |= SYSCFG_MEM_BOOT_ADD0 ;
        uint32_t p =  (*((uint32_t *) 0x1ff00000));
        __set_MSP(p); //Set the main stack pointer to its defualt values
        SysMemBootJump = (void (*)(void)) (*((uint32_t *) 0x1ff00004)); // Point the PC to the System Memory reset vector (+4)
        SysMemBootJump();
        while (1);
    }
}

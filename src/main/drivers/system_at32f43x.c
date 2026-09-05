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

#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/system.h"
#include "target/system.h"
#include "at32f435_437_clock.h"

// See RM_AT32F435_437_EN_V2.05.pdf reference manual table 5-6 for more info.
#if 256 < TARGET_FLASH_SIZE
#define USD_EOPB0_SRAM_CONFIG_MASK 0x7
#else
#define USD_EOPB0_SRAM_CONFIG_MASK 0x3
#endif

static flash_usd_eopb0_type get_sram_config(void)
{
    extern uint32_t _SRAM_SIZE; // Defined in linker file
    switch ((uint32_t)&_SRAM_SIZE) {
#if 256 == TARGET_FLASH_SIZE
    case 448:
        return FLASH_EOPB0_SRAM_448K;
    case 512:
        return FLASH_EOPB0_SRAM_512K;
    case 384:
    default:
        return FLASH_EOPB0_SRAM_384K;
#elif 448 == TARGET_FLASH_SIZE
    case 256:
        return FLASH_EOPB0_SRAM_256K;
    case 320:
        return FLASH_EOPB0_SRAM_320K;
    case 384:
        return FLASH_EOPB0_SRAM_384K;
    case 448:
        return FLASH_EOPB0_SRAM_448K;
    case 512:
        return FLASH_EOPB0_SRAM_512K;
    case 192:
    default:
        return FLASH_EOPB0_SRAM_192K;
#elif 1024 <= TARGET_FLASH_SIZE
    case 128:
        return FLASH_EOPB0_SRAM_128K;
    case 256:
        return FLASH_EOPB0_SRAM_256K;
    case 320:
        return FLASH_EOPB0_SRAM_320K;
    case 384:
        return FLASH_EOPB0_SRAM_384K;
    case 448:
        return FLASH_EOPB0_SRAM_448K;
    case 512:
        return FLASH_EOPB0_SRAM_512K;
    case 192:
    default:
        return FLASH_EOPB0_SRAM_192K;
#endif
    }
}

static void init_sram_config(void)
{
    // Make sure the SRAM config is correct
    const flash_usd_eopb0_type sram_cfg = get_sram_config();
    if (((USD->eopb0) & USD_EOPB0_SRAM_CONFIG_MASK) != sram_cfg) {
        flash_unlock();
        flash_user_system_data_erase();
        flash_eopb0_config(sram_cfg);
        systemReset();
    }
}

// void systemReset(void)
// {
//     __disable_irq();
//     NVIC_SystemReset();
// }



void SetSysClock(void);

void enableGPIOPowerUsageAndNoiseReductions(void)
{
    crm_periph_clock_enable(
        CRM_GPIOA_PERIPH_CLOCK    |
        CRM_GPIOB_PERIPH_CLOCK    |
        CRM_GPIOC_PERIPH_CLOCK    |
        CRM_GPIOD_PERIPH_CLOCK    |
        CRM_GPIOE_PERIPH_CLOCK    |
        CRM_DMA1_PERIPH_CLOCK     |
        CRM_DMA2_PERIPH_CLOCK     |
        0,TRUE);

   	crm_periph_clock_enable(
        CRM_TMR2_PERIPH_CLOCK     |
        CRM_TMR3_PERIPH_CLOCK     |
        CRM_TMR4_PERIPH_CLOCK     |
        CRM_TMR5_PERIPH_CLOCK     |
        CRM_TMR6_PERIPH_CLOCK     |
        CRM_TMR7_PERIPH_CLOCK     |
        CRM_TMR12_PERIPH_CLOCK     |
        CRM_TMR13_PERIPH_CLOCK     |
        CRM_TMR14_PERIPH_CLOCK     |
        CRM_SPI2_PERIPH_CLOCK     |
        CRM_SPI3_PERIPH_CLOCK     |
        CRM_USART2_PERIPH_CLOCK     |
        CRM_USART3_PERIPH_CLOCK     |
        CRM_UART4_PERIPH_CLOCK     |
        CRM_UART5_PERIPH_CLOCK     |
        CRM_I2C1_PERIPH_CLOCK     |
        CRM_I2C2_PERIPH_CLOCK     |
        CRM_I2C3_PERIPH_CLOCK     |
        CRM_UART8_PERIPH_CLOCK     |
        CRM_UART7_PERIPH_CLOCK     |
        CRM_PWC_PERIPH_CLOCK     |
        0,TRUE);    

  	crm_periph_clock_enable(
        CRM_TMR1_PERIPH_CLOCK     |
        CRM_TMR8_PERIPH_CLOCK     |
        CRM_USART1_PERIPH_CLOCK     |
        CRM_USART6_PERIPH_CLOCK     |
        CRM_ADC1_PERIPH_CLOCK     |
        CRM_ADC2_PERIPH_CLOCK     |
        CRM_ADC3_PERIPH_CLOCK     |
        CRM_SPI1_PERIPH_CLOCK     |
        CRM_SPI4_PERIPH_CLOCK     |
        CRM_TMR9_PERIPH_CLOCK     |
        CRM_TMR10_PERIPH_CLOCK     |
        CRM_TMR11_PERIPH_CLOCK     |
        CRM_TMR20_PERIPH_CLOCK     |
        CRM_ACC_PERIPH_CLOCK     |
        0,TRUE);         
  
    gpio_init_type gpio_init_struct;

    gpio_default_para_init(&gpio_init_struct);
    gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
    gpio_init_struct.gpio_pins = GPIO_PINS_ALL;
    gpio_init_struct.gpio_pins &= ~(GPIO_PINS_12|GPIO_PINS_11|GPIO_PINS_9); // leave USB D+/D- alone
    gpio_init_struct.gpio_pins &= ~(GPIO_PINS_14|GPIO_PINS_15|GPIO_PINS_13); // leave JTAG pins alone
    gpio_init(GPIOA, &gpio_init_struct);

    gpio_init_struct.gpio_pins = GPIO_PINS_ALL;
    gpio_init(GPIOB, &gpio_init_struct);
    gpio_init(GPIOC, &gpio_init_struct);
    gpio_init(GPIOD, &gpio_init_struct);
    gpio_init(GPIOE, &gpio_init_struct);
    gpio_init(GPIOF, &gpio_init_struct);
}

bool isMPUSoftReset(void)
{
    if (cachedRccCsrValue & CRM_SW_RESET_FLAG)
        return true;
    else
        return false;
}
//AT32 DIAGRAM2-1 AT32F435/437 DFU BOOTLOADER ADDR
uint32_t systemBootloaderAddress(void)
{
    return 0x1FFF0000;
    //return system_isr_vector_table_base;
}

void systemInit(void)
{
    init_sram_config();
    //config system clock to 288mhz usb 48mhz
    system_clock_config();
    // Configure NVIC preempt/priority groups
	nvic_priority_group_config(NVIC_PRIORITY_GROUPING);

    // cache RCC->CSR value to use it in isMPUSoftReset() and others
    cachedRccCsrValue = CRM->ctrlsts;

    // Although VTOR is already loaded with a possible vector table in RAM,
    // removing the call to NVIC_SetVectorTable causes USB not to become active,
    extern uint8_t isr_vector_table_base;
    nvic_vector_table_set((uint32_t)&isr_vector_table_base, 0x0);

//  disable usb otg fs1/2
    crm_periph_clock_enable(CRM_OTGFS2_PERIPH_CLOCK|CRM_OTGFS1_PERIPH_CLOCK,FALSE);

//  clear all reset flags
    CRM->ctrlsts_bit.rstfc = TRUE;
    enableGPIOPowerUsageAndNoiseReductions();
    // Init cycle counter
    cycleCounterInit();
    // SysTick
    SysTick_Config(system_core_clock / 1000);
}

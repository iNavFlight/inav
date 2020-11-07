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

#include "build/build_config.h"

#include "drivers/light_led.h"
#include "drivers/persistent.h"
#include "drivers/sound_beeper.h"
#include "drivers/system.h"
#include "drivers/time.h"

// cached value of RCC->CSR
uint32_t cachedRccCsrValue;

void cycleCounterInit(void)
{
    extern uint32_t usTicks; // From drivers/time.h
#if defined(USE_HAL_DRIVER)
    // We assume that SystemCoreClock is already set to a correct value by init code
    usTicks = SystemCoreClock / 1000000;
    nsTicks = SystemCoreClock / 1000;
#else
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);
    usTicks = clocks.SYSCLK_Frequency / 1000000;
    nsTicks = clocks.SYSCLK_Frequency / 1000;
#endif

    // Enable DWT for precision time measurement
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

#if defined(DWT_LAR_UNLOCK_VALUE)
#if defined(STM32F7) || defined(STM32H7)
    DWT->LAR = DWT_LAR_UNLOCK_VALUE;
#elif defined(STM32F3) || defined(STM32F4)
    // Note: DWT_Type does not contain LAR member.
#define DWT_LAR
    __O uint32_t *DWTLAR = (uint32_t *)(DWT_BASE + 0x0FB0);
    *(DWTLAR) = DWT_LAR_UNLOCK_VALUE;
#endif
#endif

    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void systemDisableAllIRQs(void)
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
    systemDisableAllIRQs();
    NVIC_SystemReset();
}

void systemResetRequest(uint32_t requestId)
{
    persistentObjectWrite(PERSISTENT_OBJECT_RESET_REASON, requestId);
    systemReset();
}

void systemResetToBootloader(void)
{
    systemResetRequest(RESET_BOOTLOADER_REQUEST_ROM);
}

typedef void resetHandler_t(void);

typedef struct isrVector_s {
    uint32_t    stackEnd;
    resetHandler_t *resetHandler;
} isrVector_t;


void checkForBootLoaderRequest(void)
{
    uint32_t bootloaderRequest = persistentObjectRead(PERSISTENT_OBJECT_RESET_REASON);

    if (bootloaderRequest != RESET_BOOTLOADER_REQUEST_ROM) {
        return;
    }
    persistentObjectWrite(PERSISTENT_OBJECT_RESET_REASON, RESET_NONE);

    volatile isrVector_t *bootloaderVector = (isrVector_t *)systemBootloaderAddress();
    __set_MSP(bootloaderVector->stackEnd);
    bootloaderVector->resetHandler();
    while (1);
}

#define SHORT_FLASH_DURATION 50
#define CODE_FLASH_DURATION 250

void failureMode(failureMode_e mode)
{
#ifdef UNIT_TEST
    (void)mode;
#else
    int codeRepeatsRemaining = 10;
    int codeFlashesRemaining;
    int shortFlashesRemaining;

    while (codeRepeatsRemaining--) {
        LED1_ON;
        LED0_OFF;
        shortFlashesRemaining = 5;
        codeFlashesRemaining = mode + 1;
        uint8_t flashDuration = SHORT_FLASH_DURATION;

        while (shortFlashesRemaining || codeFlashesRemaining) {
            LED1_TOGGLE;
            LED0_TOGGLE;
            BEEP_ON;
            delay(flashDuration);

            LED1_TOGGLE;
            LED0_TOGGLE;
            BEEP_OFF;
            delay(flashDuration);

            if (shortFlashesRemaining) {
                shortFlashesRemaining--;
                if (shortFlashesRemaining == 0) {
                    delay(500);
                    flashDuration = CODE_FLASH_DURATION;
                }
            } else {
                codeFlashesRemaining--;
            }
        }
        delay(1000);
    }

#ifdef DEBUG
    systemReset();
#else
    systemResetToBootloader();
#endif
#endif //UNIT_TEST
}

void initialiseMemorySections(void)
{
#ifdef USE_ITCM_RAM
    /* Load functions into ITCM RAM */
    extern uint8_t tcm_code_start;
    extern uint8_t tcm_code_end;
    extern uint8_t tcm_code;
    memcpy(&tcm_code_start, &tcm_code, (size_t) (&tcm_code_end - &tcm_code_start));
#endif
}

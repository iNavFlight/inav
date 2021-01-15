/*
    ChibiOS - Copyright (C) 2015 RedoX https://github.com/RedoXyde

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include "hal.h"

#if HAL_USE_PAL || defined(__DOXYGEN__)
/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config =
{
    .ports = {
        {
            /*
             * PORTA setup.
             *
             * PTA0 - SWD
             * PTA3 - SWD
             * PTA5 - PIN25
             * PTA12 - PIN3
             * PTA13 - PIN4
             * PTA14 - PIN26
             * PTA15 - PIN27
             * PTA16 - PIN28
             * PTA17 - PIN39
             * PTA18 - Crystal
             * PTA19 - Crystal
             * PTA26 - PIN42
             * PTA28 - PIN40
             * PTA29 - PIN41
             *
             */
            .port = IOPORT1,
            .pads = {
                PAL_MODE_ALTERNATIVE_7,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_ALTERNATIVE_7,       PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_INPUT_ANALOG,      PAL_MODE_INPUT_ANALOG,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
        {
            /*
             * PORTB setup.
             *
             * PTB0 - PIN16
             * PTB1 - PIN17
             * PTB2 - PIN19
             * PTB3 - PIN18
             * PTB4 - PIN49
             * PTB5 - PIN50
             * PTB10 - PIN31
             * PTB11 - PIN32
             * PTB16 - PIN0 - UART0_RX 
             * PTB17 - PIN1 - UART0_TX
             * PTB18 - PIN29
             * PTB19 - PIN30
             * PTB20 - PIN43
             * PTB21 - PIN46
             * PTB22 - PIN44
             * PTB23 - PIN45
             */
            .port = IOPORT2,
            .pads = {
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_ALTERNATIVE_3,     PAL_MODE_ALTERNATIVE_3,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
        {
            /*
             * PORTC setup.
             * PTC0 - PIN15
             * PTC1 - PIN22
             * PTC2 - PIN23
             * PTC3 - PIN9
             * PTC4 - PIN10
             * PTC5 - PIN13
             * PTC6 - PIN11
             * PTC7 - PIN12
             * PTC8 - PIN35
             * PTC9 - PIN36
             * PTC10 - PIN37
             * PTC11 - PIN38
             *
             */
            .port = IOPORT3,
            .pads = {
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
        {
            /*
             * PORTD setup.
             *
             * PTD0 - PIN2
             * PTD1 - PIN14
             * PTD2 - PIN7
             * PTD3 - PIN8
             * PTD4 - PIN6
             * PTD5 - PIN20
             * PTD6 - PIN21
             * PTD7 - PIN5
             * PTD8 - PIN47
             * PTD9 - PIN48
             * PTD11 - PIN55
             * PTD12 - PIN53
             * PTD13 - PIN52
             * PTD14 - PIN51
             * PTD15 - PIN54
             */
            .port = IOPORT4,
            .pads = {
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
        {
            /*
             * PORTE setup.
             *
             * PTE0 - SDHC
             * PTE1 - SDHC
             * PTE2 - SDHC
             * PTE3 - SDHC
             * PTE4 - SDHC
             * PTE5 - SDHC
             * PTE6 - USB OTG power switch
             * PTE10 - PIN56
             * PTE11 - PIN57
             * PTE24 - PIN33
             * PTE25 - PIN34
             * PTE26 - PIN24
             */
            .port = IOPORT5,
            .pads = {
                PAL_MODE_ALTERNATIVE_4,     PAL_MODE_ALTERNATIVE_4,     PAL_MODE_ALTERNATIVE_4,
                PAL_MODE_ALTERNATIVE_4,     PAL_MODE_ALTERNATIVE_4,     PAL_MODE_ALTERNATIVE_4,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
    },
};
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization must be performed just after stack setup
 *          and before any other initialization.
 */
void __early_init(void) {

  MK66F18_clock_init();
}

/**
 * @brief   Board-specific initialization code.
 * @todo    Add your board-specific code, if any.
 */
void boardInit(void) {
}

/*
    ChibiOS - Copyright (C) 2016 flabbergast <s3+flabbergast@sdfeu.org>

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
             * PTA1 - PIN3
             * PTA2 - PIN4
             *
             * PTA18/19 crystal
             * PTA0/3 SWD
             */
            .port = IOPORT1,
            .pads = {
                PAL_MODE_ALTERNATIVE_7,     PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_ALTERNATIVE_7,     PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_INPUT_ANALOG,      PAL_MODE_INPUT_ANALOG,      PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
        {
            /*
             * PORTB setup.
             *
             * PTB0  - PIN16
             * PTB1  - PIN17
             * PTB2  - PIN19
             * PTB3  - PIN18
             * PTB16 - PIN0 - UART0_TX
             * PTB17 - PIN1 - UART0_RX
             */
            .port = IOPORT2,
            .pads = {
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_ALTERNATIVE_3,     PAL_MODE_ALTERNATIVE_3,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
            },
        },
        {
            /*
             * PORTC setup.
             *
             * PTC0  - PIN15
             * PTC1  - PIN22
             * PTC2  - PIN23
             * PTC3  - PIN9
             * PTC4  - PIN10
             * PTC5  - PIN13
             * PTC6  - PIN11
             * PTC7  - PIN12
             */
            .port = IOPORT3,
            .pads = {
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
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
             * PTD0  - PIN2
             * PTD1  - PIN14
             * PTD2  - PIN7
             * PTD3  - PIN8
             * PTD4  - PIN6
             * PTD5  - PIN20
             * PTD6  - PIN21
             * PTD7  - PIN5
             */
            .port = IOPORT4,
            .pads = {
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
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
             * PORTE setup.
             *
             * PTE20 - PIN24
             * PTE21 - PIN25
             * PTE30 - PIN26
             */
            .port = IOPORT5,
            .pads = {
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,
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

  kl2x_clock_init();
}

/**
 * @brief   Board-specific initialization code.
 * @todo    Add your board-specific code, if any.
 */
void boardInit(void) {
}

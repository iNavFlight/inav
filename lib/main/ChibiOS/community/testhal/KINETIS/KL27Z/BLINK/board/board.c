/*
    ChibiOS - (C) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>

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

#include "ch.h"
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
             * on pads: PTA1, PTA2, PTA5, PTA18, PTA19
             *
             * PTA0/3 SWD (default SWD, ALT_7: SWD, ALT_1: PTA0/3)
             * PTA4 NMI button (default NMI_b, ALT_1: PTA4)
             * PTA20 RESET button (default RESET, ALT_7: RESET, ALT_1: PTA20)
             */
            .port = IOPORT1,
            .pads = {
                PAL_MODE_ALTERNATIVE_7,     PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,
                PAL_MODE_ALTERNATIVE_7,     PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_INPUT_ANALOG,      PAL_MODE_INPUT_ANALOG,      PAL_MODE_ALTERNATIVE_7,
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
             * on pads: PTB0, PTB1
             * LED: PTB18
             */
            .port = IOPORT2,
            .pads = {
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_OUTPUT_PUSHPULL,   PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
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
             * on pads: PTC1, PTC2, PTC3, PTC6, PTC7, PTC8, PTC9
             */
            .port = IOPORT3,
            .pads = {
                PAL_MODE_UNCONNECTED,       PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
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
             * on pads: PTD0 - PTD7
             */
            .port = IOPORT4,
            .pads = {
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,
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
             * on pads: PTE0, PTE1, PTE24, PTE25, PTE29, PTE30
             */
            .port = IOPORT5,
            .pads = {
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_OUTPUT_PUSHPULL,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,
                PAL_MODE_UNCONNECTED,       PAL_MODE_UNCONNECTED,       PAL_MODE_INPUT_PULLUP,
                PAL_MODE_INPUT_PULLUP,      PAL_MODE_UNCONNECTED,
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

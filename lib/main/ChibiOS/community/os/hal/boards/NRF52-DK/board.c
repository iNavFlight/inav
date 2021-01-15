/*
    Copyright (C) 2016 Stéphane D'Alu

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
  .pads = {
        PAL_MODE_UNCONNECTED,         /* P0.0 : XTAL (32MHz)   */
        PAL_MODE_UNCONNECTED,         /* P0.1 : XTAL (32MHz)   */
        PAL_MODE_UNCONNECTED,         /* P0.2                  */
        PAL_MODE_UNCONNECTED,         /* P0.3                  */
        PAL_MODE_UNCONNECTED,         /* P0.4                  */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.5 : UART_RTS       */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.6 : UART_TX        */
        PAL_MODE_INPUT_PULLUP,        /* P0.7 : UART_CTS       */
        PAL_MODE_INPUT_PULLUP,        /* P0.8 : UART_RX        */
        PAL_MODE_UNCONNECTED,         /* P0.9                  */
        PAL_MODE_UNCONNECTED,         /* P0.10                 */
        PAL_MODE_UNCONNECTED,         /* P0.11                 */
        PAL_MODE_UNCONNECTED,         /* P0.12                 */
        PAL_MODE_INPUT_PULLUP,        /* P0.13: BTN1           */
        PAL_MODE_INPUT_PULLUP,        /* P0.14: BTN2           */
        PAL_MODE_INPUT_PULLUP,        /* P0.15: BTN3           */
        PAL_MODE_INPUT_PULLUP,        /* P0.16: BTN4           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.17: LED1           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.18: LED2           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.19: LED3           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.20: LED4           */
        PAL_MODE_UNCONNECTED,         /* P0.21                 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.22: SPI_SS         */
        PAL_MODE_INPUT_PULLUP,        /* P0.23: SPI_MISO       */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.24: SPI_MOSI       */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.25: SPI_SCK        */
        PAL_MODE_OUTPUT_OPENDRAIN,    /* P0.26: SDA            */
        PAL_MODE_OUTPUT_OPENDRAIN,    /* P0.27: SCL            */
        PAL_MODE_UNCONNECTED,         /* P0.28                 */
        PAL_MODE_UNCONNECTED,         /* P0.29                 */
        PAL_MODE_UNCONNECTED,         /* P0.30                 */
        PAL_MODE_UNCONNECTED,         /* P0.31                 */
  },
};
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization is performed just after reset before BSS and
 *          DATA segments initialization.
 */
void __early_init(void)
{
}

/**
 * @brief   Late initialization code.
 * @note    This initialization is performed after BSS and DATA segments
 *          initialization and before invoking the main() function.
 */
void boardInit(void)
{
}

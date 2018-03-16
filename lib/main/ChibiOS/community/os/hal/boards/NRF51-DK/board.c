/*
    Copyright (C) 2015 Fabio Utzig
                  2016 Stéphane D'Alu / Bruno Remond

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

/* RAM Banks
 *  (Values are defined in Nordic gcc_startup_nrf51.s)
 */
#define NRF_POWER_RAMON_ADDRESS 0x40000524
#define NRF_POWER_RAMONB_ADDRESS 0x40000554
#define NRF_POWER_RAMONx_RAMxON_ONMODE_Msk 0x3

/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config =
{
  .pads = {
        PAL_MODE_UNCONNECTED,         /* P0.0                  */
        PAL_MODE_UNCONNECTED,         /* P0.1                  */
        PAL_MODE_UNCONNECTED,         /* P0.2                  */
        PAL_MODE_UNCONNECTED,         /* P0.3                  */
        PAL_MODE_UNCONNECTED,         /* P0.4                  */
        PAL_MODE_UNCONNECTED,         /* P0.5                  */
        PAL_MODE_UNCONNECTED,         /* P0.6                  */
        PAL_MODE_OUTPUT_OPENDRAIN,    /* P0.7 : SCL            */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.8 : UART_RTS       */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.9 : UART_TX        */
        PAL_MODE_INPUT_PULLUP,        /* P0.10: UART_CTS       */
        PAL_MODE_INPUT_PULLUP,        /* P0.11: UART_RX        */
        PAL_MODE_UNCONNECTED,         /* P0.12                 */
        PAL_MODE_UNCONNECTED,         /* P0.13                 */
        PAL_MODE_UNCONNECTED,         /* P0.14                 */
        PAL_MODE_UNCONNECTED,         /* P0.15                 */
        PAL_MODE_UNCONNECTED,         /* P0.16                 */
        PAL_MODE_INPUT_PULLUP,        /* P0.17: BTN1           */
        PAL_MODE_INPUT_PULLUP,        /* P0.18: BTN2           */
        PAL_MODE_INPUT_PULLUP,        /* P0.19: BTN3           */
        PAL_MODE_INPUT_PULLUP,        /* P0.20: BTN4           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.21: LED1           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.22: LED2           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.23: LED3           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.24: LED4 | SPI_SEL */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.25: SPI_MOSI       */
        PAL_MODE_UNCONNECTED,         /* P0.26: XTAL (32MHz)   */
        PAL_MODE_UNCONNECTED,         /* P0.27: XTAL (32MHz)   */
        PAL_MODE_INPUT_PULLUP,        /* P0.28: SPI_MISO       */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.29: SPI_SCK        */
        PAL_MODE_OUTPUT_OPENDRAIN,    /* P0.30: SDA            */
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
  /* Make sure ALL RAM banks are powered on */
  *(uint32_t *)NRF_POWER_RAMON_ADDRESS  |= NRF_POWER_RAMONx_RAMxON_ONMODE_Msk;
  *(uint32_t *)NRF_POWER_RAMONB_ADDRESS |= NRF_POWER_RAMONx_RAMxON_ONMODE_Msk;
}

/**
 * @brief   Late initialization code.
 * @note    This initialization is performed after BSS and DATA segments
 *          initialization and before invoking the main() function.
 */
void boardInit(void)
{
}

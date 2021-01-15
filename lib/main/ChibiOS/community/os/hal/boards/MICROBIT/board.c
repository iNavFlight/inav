/*
    Copyright (C) 2017 Stéphane D'Alu

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
        PAL_MODE_OUTPUT_OPENDRAIN,    /* P0.0 : SCL      P19   */
        PAL_MODE_UNCONNECTED,         /* P0.1 : PAD1     P2    */
        PAL_MODE_UNCONNECTED,         /* P0.2 : PAD2     P1    */
        PAL_MODE_UNCONNECTED,         /* P0.3 : PAD3     P0    */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.4 : COL1     P3    */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.5 : COL2     P4    */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.6 : COL3     P10   */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.7 : COL4           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.8 : COL5           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.9 : COL6           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.10: COL7     P9    */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.11: COL8     P7    */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.12: COL9     P6    */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.13: ROW1           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.14: ROW2           */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.15: ROW3           */
        PAL_MODE_UNCONNECTED,         /* P0.16           P16   */
        PAL_MODE_INPUT,               /* P0.17: BTN_A    P5    */
        PAL_MODE_UNCONNECTED,         /* P0.18           P8    */
        PAL_MODE_INPUT,               /* P0.19: BTN_RST        */
        PAL_MODE_UNCONNECTED,         /* P0.20           P12   */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.21: SPI_MOSI P15   */
        PAL_MODE_INPUT_PULLUP,        /* P0.22: SPI_MISO P14   */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.23: SPI_SCK  P13   */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.24: UART_TX        */
        PAL_MODE_INPUT_PULLUP,        /* P0.25: UART_RX        */
        PAL_MODE_INPUT,               /* P0.26: BTN_B    P11   */
        PAL_MODE_INPUT,               /* P0.27: ACC_INT2       */
        PAL_MODE_INPUT,               /* P0.28: ACC_INT1       */
        PAL_MODE_INPUT,               /* P0.29: MAG_INT1       */
        PAL_MODE_OUTPUT_OPENDRAIN,    /* P0.30: SDA      P20   */
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

/*
    Copyright (C) 2016 flabbergast

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
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.0:  PIN11 (AREF0) */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.1:  PIN9  (AIN2)  */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.2:  PIN10 (AIN3)  */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.3:  LED_BLUE      */
        PAL_MODE_UNCONNECTED,         /* P0.4 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.5:  LED_GREEN     */
        PAL_MODE_UNCONNECTED,         /* P0.6 */
        PAL_MODE_UNCONNECTED,         /* P0.7 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.8:  LED_RED       */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.9:  PIN7          */
        PAL_MODE_UNCONNECTED,         /* P0.10 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.11: PIN6          */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.12: PIN5          */
        PAL_MODE_UNCONNECTED,         /* P0.13 */
        PAL_MODE_UNCONNECTED,         /* P0.14 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.15: PIN4          */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.16: PIN3          */
        PAL_MODE_UNCONNECTED,         /* P0.17 */
        PAL_MODE_INPUT_PULLUP,        /* P0.18: PIN2 (RX)     */
        PAL_MODE_UNCONNECTED,         /* P0.19 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.20: PIN1 (TX)     */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.21: PIN15         */
        PAL_MODE_UNCONNECTED,         /* P0.22 */
        PAL_MODE_UNCONNECTED,         /* P0.23 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.24: PIN14         */
        PAL_MODE_UNCONNECTED,         /* P0.25 */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.26: PIN13 (LFXTL) (AIN0) */
        PAL_MODE_OUTPUT_PUSHPULL,     /* P0.27: PIN12 (LFXTL) (AIN1) */
        PAL_MODE_UNCONNECTED,         /* P0.28 */
        PAL_MODE_UNCONNECTED,         /* P0.29 */
        PAL_MODE_UNCONNECTED,         /* P0.30 */
        PAL_MODE_UNCONNECTED,         /* P0.31 */
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

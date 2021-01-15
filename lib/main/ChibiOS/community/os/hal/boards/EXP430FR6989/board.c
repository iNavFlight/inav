/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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

/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
#if HAL_USE_PAL || defined(__DOXYGEN__)
const PALConfig pal_default_config =
{
  {VAL_IOPORT1_OUT, VAL_IOPORT1_DIR, VAL_IOPORT1_REN, VAL_IOPORT1_SEL0, 
    VAL_IOPORT1_SEL1},
  {VAL_IOPORT2_OUT, VAL_IOPORT2_DIR, VAL_IOPORT2_REN, VAL_IOPORT2_SEL0, 
    VAL_IOPORT2_SEL1},
  {VAL_IOPORT3_OUT, VAL_IOPORT3_DIR, VAL_IOPORT3_REN, VAL_IOPORT3_SEL0, 
    VAL_IOPORT3_SEL1},
  {VAL_IOPORT4_OUT, VAL_IOPORT4_DIR, VAL_IOPORT4_REN, VAL_IOPORT4_SEL0, 
    VAL_IOPORT4_SEL1},
  {VAL_IOPORT5_OUT, VAL_IOPORT5_DIR, VAL_IOPORT5_REN, VAL_IOPORT5_SEL0, 
    VAL_IOPORT5_SEL1},
  {VAL_IOPORT0_OUT, VAL_IOPORT0_DIR, VAL_IOPORT0_REN, VAL_IOPORT0_SEL0, 
    VAL_IOPORT0_SEL1}
}; /* Set UART TX pin correctly */
#endif /* HAL_USE_PAL */

/**
 * Board-specific initialization code.
 */
void boardInit(void) {

  /*
   * External interrupts setup, all disabled initially.
   */
  _disable_interrupts();

}

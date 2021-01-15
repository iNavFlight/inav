/*
    Copyright (C) 2014..2017 Marco Veeneman

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
  {VAL_GPIOA_DATA, VAL_GPIOA_DIR, VAL_GPIOA_AFSEL, VAL_GPIOA_DR2R, 
   VAL_GPIOA_DR4R, VAL_GPIOA_DR8R, VAL_GPIOA_ODR, VAL_GPIOA_PUR, 
   VAL_GPIOA_PDR, VAL_GPIOA_SLR, VAL_GPIOA_DEN, VAL_GPIOA_AMSEL, 
   VAL_GPIOA_PCTL},
  {VAL_GPIOB_DATA, VAL_GPIOB_DIR, VAL_GPIOB_AFSEL, VAL_GPIOB_DR2R, 
   VAL_GPIOB_DR4R, VAL_GPIOB_DR8R, VAL_GPIOB_ODR, VAL_GPIOB_PUR, 
   VAL_GPIOB_PDR, VAL_GPIOB_SLR, VAL_GPIOB_DEN, VAL_GPIOB_AMSEL, 
   VAL_GPIOB_PCTL},
  {VAL_GPIOC_DATA, VAL_GPIOC_DIR, VAL_GPIOC_AFSEL, VAL_GPIOC_DR2R, 
   VAL_GPIOC_DR4R, VAL_GPIOC_DR8R, VAL_GPIOC_ODR, VAL_GPIOC_PUR, 
   VAL_GPIOC_PDR, VAL_GPIOC_SLR, VAL_GPIOC_DEN, VAL_GPIOC_AMSEL, 
   VAL_GPIOC_PCTL},
  {VAL_GPIOD_DATA, VAL_GPIOD_DIR, VAL_GPIOD_AFSEL, VAL_GPIOD_DR2R, 
   VAL_GPIOD_DR4R, VAL_GPIOD_DR8R, VAL_GPIOD_ODR, VAL_GPIOD_PUR, 
   VAL_GPIOD_PDR, VAL_GPIOD_SLR, VAL_GPIOD_DEN, VAL_GPIOD_AMSEL, 
   VAL_GPIOD_PCTL},
  {VAL_GPIOE_DATA, VAL_GPIOE_DIR, VAL_GPIOE_AFSEL, VAL_GPIOE_DR2R, 
   VAL_GPIOE_DR4R, VAL_GPIOE_DR8R, VAL_GPIOE_ODR, VAL_GPIOE_PUR, 
   VAL_GPIOE_PDR, VAL_GPIOE_SLR, VAL_GPIOE_DEN, VAL_GPIOE_AMSEL, 
   VAL_GPIOE_PCTL},
  {VAL_GPIOF_DATA, VAL_GPIOF_DIR, VAL_GPIOF_AFSEL, VAL_GPIOF_DR2R, 
   VAL_GPIOF_DR4R, VAL_GPIOF_DR8R, VAL_GPIOF_ODR, VAL_GPIOF_PUR, 
   VAL_GPIOF_PDR, VAL_GPIOF_SLR, VAL_GPIOF_DEN, VAL_GPIOF_AMSEL, 
   VAL_GPIOF_PCTL}
};
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization is performed just after reset before BSS and
 *          DATA segments initialization.
 */
void __early_init(void)
{
  tiva_clock_init();
}

/**
 * @brief   Late initialization code.
 * @note    This initialization is performed after BSS and DATA segments 
 *          initialization and before invoking the main() function.
 */
void boardInit(void)
{
}

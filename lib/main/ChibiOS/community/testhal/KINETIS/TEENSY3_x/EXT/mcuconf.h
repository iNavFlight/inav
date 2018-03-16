/*
    ChibiOS - (C) 2015 RedoX https://github.com/RedoXyde

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

#ifndef _MCUCONF_H_
#define _MCUCONF_H_

#define K20x_MCUCONF

/*
 * HAL driver system settings.
 */
/* PEE mode - 48MHz system clock driven by external crystal. */
#define KINETIS_MCG_MODE            KINETIS_MCG_MODE_PEE
#define KINETIS_PLLCLK_FREQUENCY    96000000UL
#define KINETIS_SYSCLK_FREQUENCY    48000000UL

/*
 * EXT driver system settings.
 */
#define KINETIS_EXTI_NUM_CHANNELS         1
#define KINETIS_EXT_PORTA_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTB_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTC_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTD_IRQ_PRIORITY          12
#define KINETIS_EXT_PORTE_IRQ_PRIORITY          12

/* K20 64pin  */
#define KINETIS_EXT_PORTA_WIDTH                 20
#define KINETIS_EXT_PORTB_WIDTH                 20
#define KINETIS_EXT_PORTC_WIDTH                 12
#define KINETIS_EXT_PORTD_WIDTH                 8
#define KINETIS_EXT_PORTE_WIDTH                 2

#endif /* _MCUCONF_H_ */

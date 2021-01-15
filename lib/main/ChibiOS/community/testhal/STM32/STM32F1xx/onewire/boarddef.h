/*
    ChibiOS/RT - Copyright (C) 2016 Uladzimir Pylinsky aka barthess

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

#ifndef BOARDDEF_H_
#define BOARDDEF_H_

#define ONEWIRE_PORT                  GPIOB
#define ONEWIRE_PIN                   0
#define ONEWIRE_PAD_MODE_IDLE         PAL_MODE_INPUT
#define ONEWIRE_PAD_MODE_ACTIVE       PAL_MODE_STM32_ALTERNATE_OPENDRAIN
#define search_led_on()               (palClearPad(GPIOC, GPIOC_LED))
#define search_led_off()              (palSetPad(GPIOC, GPIOC_LED))
#define ONEWIRE_MASTER_CHANNEL        2
#define ONEWIRE_SAMPLE_CHANNEL        3

#endif /* BOARDDEF_H_ */

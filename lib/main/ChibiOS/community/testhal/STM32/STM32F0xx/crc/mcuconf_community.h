/*
    ChibiOS/RT - Copyright (C) 2015 Michael D. Spradling

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

#ifndef _MCUCONF_COMMUNITY_H_
#define _MCUCONF_COMMUNITY_H_

/*
 * CRC driver system settings.
 */
#define STM32_CRC_USE_CRC1                  TRUE
#define STM32_CRC_CRC1_DMA_IRQ_PRIORITY     1
#define STM32_CRC_CRC1_DMA_PRIORITY         2
#define STM32_CRC_CRC1_DMA_STREAM           STM32_DMA1_STREAM2

#define CRCSW_USE_CRC1                      FALSE
#define CRCSW_CRC32_TABLE                   TRUE
#define CRCSW_CRC16_TABLE                   TRUE
#define CRCSW_PROGRAMMABLE                  TRUE

#endif

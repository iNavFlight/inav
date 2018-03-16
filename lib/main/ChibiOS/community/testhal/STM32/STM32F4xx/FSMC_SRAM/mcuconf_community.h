/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

/*
 * FSMC driver system settings.
 */
#define STM32_FSMC_USE_FSMC1                TRUE
#define STM32_FSMC_FSMC1_IRQ_PRIORITY       10
#define STM32_FSMC_DMA_CHN                  0x03010201

/*
 * FSMC NAND driver system settings.
 */
#define STM32_NAND_USE_FSMC_NAND1           FALSE
#define STM32_NAND_USE_FSMC_NAND2           FALSE
#define STM32_NAND_USE_EXT_INT              FALSE
#define STM32_NAND_DMA_STREAM               STM32_DMA_STREAM_ID(2, 7)
#define STM32_NAND_DMA_PRIORITY             0
#define STM32_NAND_DMA_ERROR_HOOK(nandp)    osalSysHalt("DMA failure")

/*
 * FSMC SRAM driver system settings.
 */
#define STM32_USE_FSMC_SRAM                 TRUE
#define STM32_SRAM_USE_FSMC_SRAM1           FALSE
#define STM32_SRAM_USE_FSMC_SRAM2           FALSE
#define STM32_SRAM_USE_FSMC_SRAM3           FALSE
#define STM32_SRAM_USE_FSMC_SRAM4           TRUE

/*
 * FSMC PC card driver system settings.
 */
#define STM32_USE_FSMC_PCCARD               FALSE

/*
 * FSMC SDRAM driver system settings.
 */
#define STM32_USE_FSMC_SDRAM                FALSE

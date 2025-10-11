/*
******************************************************************************
**

**  File        : LinkerScript.ld
**
**  Author		: STM32CubeMX
**
**  Abstract    : Linker script for STM32H7A3VITx series
**                2097152Kbytes FLASH and 0Kbytes RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**  Distribution: The file is distributed â€œas is,â€ without any warranty
**                of any kind.
**
*****************************************************************************
** @attention
**
** <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**   1. Redistributions of source code must retain the above copyright notice,
**      this list of conditions and the following disclaimer.
**   2. Redistributions in binary form must reproduce the above copyright notice,
**      this list of conditions and the following disclaimer in the documentation
**      and/or other materials provided with the distribution.
**   3. Neither the name of STMicroelectronics nor the names of its contributors
**      may be used to endorse or promote products derived from this software
**      without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x200;      /* required amount of heap  */
_Min_Stack_Size = 0x1800; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
    RAM (rwx)      : ORIGIN = 0x24000000, LENGTH = 1024K
    FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1792K
    DTCMRAM (rwx)      : ORIGIN = 0x20000000, LENGTH = 128K
    ITCMRAM (rwx)      : ORIGIN = 0x00000000, LENGTH = 64K
    FLASH_CONFIG (rwx)      : ORIGIN = 0x081c0000, LENGTH = 256K
    AHB_SRAM1 (rwx)      : ORIGIN = 0x30000000, LENGTH = 128K
    /*AHB_SRAM2 (rwx)      : ORIGIN = 0x30010000, LENGTH = 64K*/
    AHB_SRAM3 (rwx)      : ORIGIN = 0x38000000, LENGTH = 64K
}

REGION_ALIAS("STACKRAM", DTCMRAM)
REGION_ALIAS("FASTRAM", DTCMRAM)
REGION_ALIAS("D2_RAM", AHB_SRAM1)

INCLUDE "stm32_flash_h7a_split.ld"

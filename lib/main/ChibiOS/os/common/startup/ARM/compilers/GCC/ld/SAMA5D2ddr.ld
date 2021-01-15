/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * SAMA5D2 memory setup in non trusted mode.
 */
MEMORY
{
    flash : org = 0x21000000, len = 16M
    ram0  : org = 0x22000000, len = 78M
    ram1  : org = 0x26E00000, len = 1M
    ram2  : org = 0x26F00000, len = 1M
    ram3  : org = 0x00000000, len = 0
    ram4  : org = 0x00000000, len = 0
    ram5  : org = 0x00000000, len = 0
    ram6  : org = 0x00000000, len = 0
    ram7  : org = 0x00000000, len = 0
}

/* RAM region to be used for stacks. This stack accommodates the processing
   of all exceptions and interrupts*/
REGION_ALIAS("STACKS_RAM", ram0);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", ram0);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", ram0);

/* RAM region to be used for lcd frame buffer. */
REGION_ALIAS("FB_RAM", ram1);

/* RAM region to be used for no cache area. */
REGION_ALIAS("NO_CACHE", ram2);

SECTIONS
{
    /* Special section for frame buffer area.*/
    .fbram (NOLOAD) : ALIGN(4)
    {
        __fbram_base__ = .;
        *(.fbram)
        *(.fbram.*)
        *(.bss.__fbram_*)
        . = ALIGN(4);
        __fbram_end__ = .;
    } > FB_RAM
    
    /* Special section for no cache area.*/
    .nocache (NOLOAD) : ALIGN(4)
    {
        __nocache_base__ = .;
        *(.nocache)
        *(.nocache.*)
        *(.bss.__nocache_*)
        . = ALIGN(4);
        __nocache_end__ = .;
    } > NO_CACHE
}

INCLUDE rules.ld

ENTRY(Boot_Handler);


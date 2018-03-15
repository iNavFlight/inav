/*
*****************************************************************************
**
**  File        : stm32_flash_f446.ld
**
**  Abstract    : Linker script for STM32F446 Device with
**                512KByte FLASH, 128KByte RAM
**
*****************************************************************************
*/

/* Stack & Heap sizes */
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x1800;

/* Entry Point */
ENTRY(Reset_Handler)

/*
0x08000000 to 0x08080000 512K full flash,
0x08000000 to 0x0805FFFF 384K firmware,
0x08060000 to 0x08080000 128K config,
*/

/* Specify the memory areas */
MEMORY
{
    FLASH (rx)        : ORIGIN = 0x08000000, LENGTH = 384K
    FLASH_CONFIG (r)  : ORIGIN = 0x08060000, LENGTH = 128K

    RAM (rwx)         : ORIGIN = 0x20000000, LENGTH = 128K
    MEMORY_B1 (rx)    : ORIGIN = 0x60000000, LENGTH = 0K
}

REGION_ALIAS("STACKRAM", RAM)
REGION_ALIAS("FASTRAM", RAM)

INCLUDE "stm32_flash.ld"
/*
*****************************************************************************
**
**  File        : stm32_flash_f446_drbl.ld
**
**  Abstract    : Linker script for STM32F446 Device with
**                512KByte FLASH, 128KByte RAM
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(ResetHandler)

__main_stack_size__     = 0x0F00;
__process_stack_size__  = 0x0400;

/* Specify the memory areas */
MEMORY
{
    START (rx)      : ORIGIN = 0x08000000, LENGTH = 16K
    FLASH_CONFIG (r): ORIGIN = 0x08004000, LENGTH = 16K
    FLASH (rx)      : ORIGIN = 0x08008000, LENGTH = 480K
    RAM (rwx)       : ORIGIN = 0x20000000, LENGTH = 128K
    MEMORY_B1 (rx)  : ORIGIN = 0x60000000, LENGTH = 0K
    ram1            : ORIGIN = 0x00000000, LENGTH = 0
    ram2            : ORIGIN = 0x00000000, LENGTH = 0
    ram3            : ORIGIN = 0x00000000, LENGTH = 0
    ram4            : ORIGIN = 0x00000000, LENGTH = 0
    ram5            : ORIGIN = 0x00000000, LENGTH = 0
    ram6            : ORIGIN = 0x00000000, LENGTH = 0
    ram7            : ORIGIN = 0x00000000, LENGTH = 0
}

/* RAM region to be used for Main stack. This stack accommodates the processing
   of all exceptions and interrupts*/
REGION_ALIAS("MAIN_STACK_RAM", RAM);

/* RAM region to be used for the process stack. This is the stack used by
   the main() function.*/
REGION_ALIAS("PROCESS_STACK_RAM", RAM);

/* RAM region to be used for data segment.*/
REGION_ALIAS("DATA_RAM", RAM);

/* RAM region to be used for BSS segment.*/
REGION_ALIAS("BSS_RAM", RAM);

/* RAM region to be used for the default heap.*/
REGION_ALIAS("HEAP_RAM", RAM);

REGION_ALIAS("STACKRAM", RAM)

INCLUDE "stm32F446xx_chibios_sections.ld"

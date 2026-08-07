/*
*****************************************************************************
**
**  File        : stm32_flash_f765xg_for_bl.ld
**
**  Abstract    : Linker script for STM32F765xGTx Device with
**                1024KByte FLASH, 512KByte RAM (bootloader-aware)
**
*****************************************************************************
*/

/* Stack & Heap sizes */
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x1800;

/* Entry Point */
ENTRY(Reset_Handler)

/*
0x00000000 to 0x00003FFF   16K ITCM RAM,
0x08000000 to 0x080FFFFF 1024K full flash,
0x08000000 to 0x08007FFF   32K isr vector, startup code,
0x08008000 to 0x0800FFFF   32K config,                  // FLASH_Sector_1
0x08010000 to 0x080FFFFF  960K firmware,
*/

/* Specify the memory areas */
MEMORY
{
    ITCM_RAM (rx)               : ORIGIN = 0x00000000, LENGTH = 16K
    ITCM_FLASH (rx)             : ORIGIN = 0x00200000, LENGTH = 32K
    /* config occupies the entire flash sector 1 for the ease of erasure, 32K on F7xx */
    ITCM_FLASH_CONFIG (r)       : ORIGIN = 0x00208000, LENGTH = 32K
    ITCM_FLASH1 (rx)            : ORIGIN = 0x00210000, LENGTH = 960K

    FLASH (rx)                  : ORIGIN = 0x08008000, LENGTH = 32K
    FLASH_CONFIG (r)            : ORIGIN = 0x08010000, LENGTH = 32K
    FLASH1 (rx)                 : ORIGIN = 0x08018000, LENGTH = 928K

    DTCM_RAM (rwx)              : ORIGIN = 0x20000000, LENGTH = 128K
    SRAM1 (rwx)                 : ORIGIN = 0x20020000, LENGTH = 368K
    SRAM2 (rwx)                 : ORIGIN = 0x2007C000, LENGTH = 16K
    MEMORY_B1 (rx)              : ORIGIN = 0x60000000, LENGTH = 0K
}
/* STM32F765 DTCM is 128KB (0x20000000-0x2001FFFF); SRAM1 starts at 0x20020000.
 * DMA cannot access DTCM on STM32F7 - all DMA buffers must be in SRAM1 or SRAM2.
 * NOTE: F745 DTCM is only 64KB, so F745 SRAM1 starts at 0x20010000 (different!). */
REGION_ALIAS("STACKRAM", DTCM_RAM)
REGION_ALIAS("FASTRAM",  DTCM_RAM)
REGION_ALIAS("RAM",      SRAM1)

__firmware_start = ORIGIN(FLASH);

INCLUDE "stm32_flash_f7_split.ld"

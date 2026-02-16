/*
 * Linker script for RP2350 (Raspberry Pi Pico 2)
 *
 * Memory layout:
 *   FLASH:        4032K XIP flash (0x10000000)
 *   FLASH_CONFIG: 64K   settings storage (0x103F0000)
 *   RAM:          512K  SRAM (0x20000000)
 *   SCRATCH_X:    4K    core1 stack (0x20080000)
 *   SCRATCH_Y:    4K    core0 stack (0x20081000)
 */

MEMORY
{
    FLASH(rx)        : ORIGIN = 0x10000000, LENGTH = 4032K
    FLASH_CONFIG(r)  : ORIGIN = 0x103F0000, LENGTH = 64K
    RAM(rwx)         : ORIGIN = 0x20000000, LENGTH = 512K
    SCRATCH_X(rwx)   : ORIGIN = 0x20080000, LENGTH = 4K
    SCRATCH_Y(rwx)   : ORIGIN = 0x20081000, LENGTH = 4K
}

ENTRY(_entry_point)

/* Stack at top of RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Config storage */
__config_start = ORIGIN(FLASH_CONFIG);
__config_end = ORIGIN(FLASH_CONFIG) + LENGTH(FLASH_CONFIG);

SECTIONS
{
    /* Boot2 stage 2 bootloader - must be first 256 bytes */
    .boot2 :
    {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > FLASH

    ASSERT(__boot2_end__ - __boot2_start__ == 256,
        "ERROR: boot2 section must be exactly 256 bytes")

    /* Vector table */
    .isr_vector :
    {
        . = ALIGN(4);
        PROVIDE(isr_vector_table_base = .);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* Program code */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)

        KEEP (*(.init))
        KEEP (*(.fini))

        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM : {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } > FLASH

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } > FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(.fini_array*))
        KEEP (*(SORT(.fini_array.*)))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } > FLASH

    /* INAV parameter group registry */
    .pg_registry :
    {
        PROVIDE_HIDDEN (__pg_registry_start = .);
        KEEP (*(.pg_registry))
        KEEP (*(SORT(.pg_registry.*)))
        PROVIDE_HIDDEN (__pg_registry_end = .);
    } > FLASH

    .pg_resetdata :
    {
        PROVIDE_HIDDEN (__pg_resetdata_start = .);
        KEEP (*(.pg_resetdata))
        PROVIDE_HIDDEN (__pg_resetdata_end = .);
    } > FLASH

    /* INAV bus device registry */
    .busdev_registry :
    {
        PROVIDE_HIDDEN (__busdev_registry_start = .);
        KEEP (*(.busdev_registry))
        KEEP (*(SORT(.busdev_registry.*)))
        PROVIDE_HIDDEN (__busdev_registry_end = .);
    } > FLASH

    _sidata = .;

    /* Initialized data - loaded from flash, runs from RAM */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)

        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    /* Uninitialized data */
    . = ALIGN(4);
    .bss :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(SORT_BY_ALIGNMENT(.bss*))
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > RAM

    /* Heap and stack check */
    _Min_Heap_Size = 0x400;
    _Min_Stack_Size = 0x800;
    _heap_stack_end = ORIGIN(RAM) + LENGTH(RAM) - 8;
    _heap_stack_begin = _heap_stack_end - _Min_Stack_Size - _Min_Heap_Size;
    . = _heap_stack_begin;
    ._user_heap_stack :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(4);
    } > RAM = 0xa5

    /* Scratch memory for multicore */
    .scratch_x :
    {
        *(.scratch_x.*)
    } > SCRATCH_X

    .scratch_y :
    {
        *(.scratch_y.*)
    } > SCRATCH_Y

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}

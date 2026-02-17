/*
 * Linker script for RP2350 (Raspberry Pi Pico 2)
 *
 * Compatible with both Pico SDK crt0.S and INAV sections.
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

/* Stack — SDK uses __StackTop/__StackLimit, INAV uses _estack */
__StackTop = ORIGIN(RAM) + LENGTH(RAM);
_estack = __StackTop;

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

    /* Vector table — SDK crt0.S uses .vectors */
    .vectors :
    {
        . = ALIGN(256);
        PROVIDE(isr_vector_table_base = .);
        KEEP(*(.vectors))
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* SDK reset handler code */
    .reset :
    {
        KEEP(*(.reset))
    } > FLASH

    /* Program code */
    .text :
    {
        . = ALIGN(4);
        *(.time_critical*)
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
        __etext = .;
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

    /* SDK binary info */
    .binary_info :
    {
        __binary_info_start = .;
        KEEP(*(.binary_info.keep.*))
        __binary_info_end = .;
    } > FLASH

    _sidata = LOADADDR(.data);

    /* Initialized data - loaded from flash, runs from RAM */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        __data_start__ = .;
        *(.data)
        *(.data*)
        *(.time_critical*)
        *(.after_data.*)

        . = ALIGN(4);
        _edata = .;
        __data_end__ = .;
    } > RAM AT> FLASH

    /* SDK mutex array */
    .mutex_array (NOLOAD) :
    {
        . = ALIGN(4);
        __mutex_array_start = .;
        KEEP(*(SORT(.mutex_array.*)))
        __mutex_array_end = .;
    } > RAM

    /* Uninitialized data */
    . = ALIGN(4);
    .bss (NOLOAD) :
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

    .heap (NOLOAD) :
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        __end__ = .;
        PROVIDE( __HeapLimit = __StackLimit );
    } > RAM

    /* Stack limit — checked by SDK malloc/sbrk for heap/stack collision */
    __StackLimit = __StackTop - _Min_Stack_Size;

    /* Scratch memory for multicore */
    .scratch_x :
    {
        __scratch_x_start__ = .;
        *(.scratch_x.*)
        . = ALIGN(4);
        __scratch_x_end__ = .;
    } > SCRATCH_X AT> FLASH
    __scratch_x_source__ = LOADADDR(.scratch_x);

    .scratch_y :
    {
        __scratch_y_start__ = .;
        *(.scratch_y.*)
        . = ALIGN(4);
        __scratch_y_end__ = .;
    } > SCRATCH_Y AT> FLASH
    __scratch_y_source__ = LOADADDR(.scratch_y);

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}

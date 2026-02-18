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
    .flash_begin : {
        __flash_binary_start = .;
    } > FLASH

    /* Vector table — SDK crt0.S uses .vectors, followed by the picobin
       IMAGE_DEF block that the RP2350 ROM bootloader requires to execute
       the image. Without .embedded_block, the ROM stays in mass storage. */
    .text_boot :
    {
        __logical_binary_start = .;
        KEEP (*(.vectors))
        KEEP (*(.binary_info_header))
        __binary_info_header_end = .;
        KEEP (*(.embedded_block))
        __embedded_block_end = .;
    } > FLASH

    /* SDK reset handler code */
    .reset :
    {
        KEEP(*(.reset))
    } > FLASH

    /* Optional stage-2 XIP setup function (not needed for normal flash boot
       on RP2350, but must be <= 256 bytes if present) */
    .boot2 :
    {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > FLASH

    ASSERT(__boot2_end__ - __boot2_start__ <= 256,
        "ERROR: boot2 XIP setup function must be no more than 256 bytes")

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

    /* SDK binary info */
    .binary_info :
    {
        __binary_info_start = .;
        KEEP(*(.binary_info.keep.*))
        __binary_info_end = .;
        /* Pad to 4-byte boundary so that .data's LMA (= LOADADDR(.data)) is
           4-byte aligned.  crt0.S copies .data with ldmia which requires
           word-aligned addresses; a misaligned source triggers a HardFault. */
        . = ALIGN(4);
    } > FLASH

    /* Initialized data — loaded from flash, runs from RAM.
       SDK mutex_array goes here so mutex_t structs are copied to RAM at boot
       and can be written by mutex_init(). Placing mutex_array in flash-only
       would prevent mutex_init from storing the spin_lock pointer, leaving
       spin_lock=NULL and causing an infinite spin on ROM address 0x00000000. */
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
        PROVIDE_HIDDEN (__mutex_array_start = .);
        KEEP(*(SORT(.mutex_array.*)))
        KEEP(*(.mutex_array))
        PROVIDE_HIDDEN (__mutex_array_end = .);

        . = ALIGN(4);
        _edata = .;
        __data_end__ = .;
    } > RAM AT> FLASH

    /* __etext is (for backwards compatibility) the name of the .data init source pointer.
       crt0.S uses __etext to find where to copy .data from — it must equal LOADADDR(.data),
       NOT the end of .text (which comes earlier, before pg_registry/busdev_registry sections). */
    __etext = LOADADDR(.data);
    _sidata  = LOADADDR(.data);

    /* RAM interrupt vector table.
       Cortex-M33 VTOR requires alignment to next power-of-2 >= table size.
       RP2350 has 16 + 52 = 68 exceptions × 4 bytes = 272 bytes → 512-byte alignment.
       Without this, the hardware clears VTOR[8:0] (shifts by -0x40), making
       irq_set_exclusive_handler read the wrong entry and hard-assert. */
    .ram_vector_table (NOLOAD) :
    {
        . = ALIGN(512);
        *(.ram_vector_table)
    } > RAM

    /* Uninitialized data */
    . = ALIGN(4);
    .bss (NOLOAD) :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(SORT_BY_ALIGNMENT(.bss*))
        /* INAV "fast RAM" BSS — no separate fast RAM on RP2350; flatten here so
           crt0's __bss_start__/__bss_end__ loop zeros it like any other BSS. */
        *(.fastram_bss*)
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

    /* End picobin block — closes the block loop started by .embedded_block.
       Must be at the end of flash so the ROM can find both ends. */
    .flash_end : {
        KEEP(*(.embedded_end_block*))
        PROVIDE(__flash_binary_end = .);
    } > FLASH =0xaa

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}

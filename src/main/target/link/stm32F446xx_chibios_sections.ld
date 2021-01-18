/*
    ChibiOS/RT - Copyright (C) 2006,2007,2008,2009,2010,
                 2011,2012,2013 Giovanni Di Sirio.

    This file is part of ChibiOS/RT.

    ChibiOS/RT is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS/RT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

                                      ---

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes ChibiOS/RT, without being obliged to provide
    the source code for any proprietary components. See the file exception.txt
    for full details of how and when the exception can be applied.
*/

__ram_start__           = ORIGIN(RAM);
__ram_size__            = LENGTH(RAM);
__ram_end__             = __ram_start__ + __ram_size__;

__ram0_start__          = ORIGIN(RAM);
__ram0_size__           = LENGTH(RAM);
__ram0_end__            = __ram0_start__ + __ram0_size__;
__ram1_start__          = ORIGIN(ram1);
__ram1_size__           = LENGTH(ram1);
__ram1_end__            = __ram1_start__ + __ram1_size__;
__ram2_start__          = ORIGIN(ram2);
__ram2_size__           = LENGTH(ram2);
__ram2_end__            = __ram2_start__ + __ram2_size__;
__ram3_start__          = ORIGIN(ram3);
__ram3_size__           = LENGTH(ram3);
__ram3_end__            = __ram3_start__ + __ram3_size__;
__ram4_start__          = ORIGIN(ram4);
__ram4_size__           = LENGTH(ram4);
__ram4_end__            = __ram4_start__ + __ram4_size__;
__ram5_start__          = ORIGIN(ram5);
__ram5_size__           = LENGTH(ram5);
__ram5_end__            = __ram5_start__ + __ram5_size__;
__ram6_start__          = ORIGIN(ram6);
__ram6_size__           = LENGTH(ram6);
__ram6_end__            = __ram6_start__ + __ram6_size__;
__ram7_start__          = ORIGIN(ram7);
__ram7_size__           = LENGTH(ram7);
__ram7_end__            = __ram7_start__ + __ram7_size__;


ENTRY(ResetHandler)

_estack = ORIGIN(RAM) + LENGTH(RAM);    /* end of RAM */

/* Base address where the config is stored. */
__config_start = ORIGIN(FLASH_CONFIG);
__config_end = ORIGIN(FLASH_CONFIG) + LENGTH(FLASH_CONFIG);

SECTIONS
{
    . = 0;
    _text = .;

    startup : ALIGN(16) SUBALIGN(16)
    {
        PROVIDE (isr_vector_table_base = .);
        KEEP(*(.isr_vector)) /* Startup code */
    } > START

    images : ALIGN(16) SUBALIGN(16)
    {
        KEEP(*(.images_section))
    } > START

    constructors : ALIGN(4) SUBALIGN(4)
    {
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    } > FLASH

    destructors : ALIGN(4) SUBALIGN(4)
    {
        __fini_array_start = .;
        KEEP(*(.fini_array))
        KEEP(*(SORT(.fini_array.*)))
        __fini_array_end = .;
    } > FLASH

    .text : ALIGN(16) SUBALIGN(16)
    {
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        *(.glue_7t)
        *(.glue_7)
        *(.gcc*)
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
     } > FLASH

    .eh_frame_hdr :
    {
        *(.eh_frame_hdr)
    } > FLASH

    .eh_frame : ONLY_IF_RO
    {
        *(.eh_frame)
    } > FLASH

    .textalign : ONLY_IF_RO
    {
        . = ALIGN(8);
    } > FLASH

    .pg_registry :
    {
        PROVIDE_HIDDEN (__pg_registry_start = .);
        KEEP (*(.pg_registry))
        KEEP (*(SORT(.pg_registry.*)))
        PROVIDE_HIDDEN (__pg_registry_end = .);
    } >FLASH

    .pg_resetdata :
    {
        PROVIDE_HIDDEN (__pg_resetdata_start = .);
        KEEP (*(.pg_resetdata))
        PROVIDE_HIDDEN (__pg_resetdata_end = .);
    } >FLASH
 
	.busdev_registry :
    {
    	PROVIDE_HIDDEN (__busdev_registry_start = .);
    	KEEP (*(.busdev_registry))
    	KEEP (*(SORT(.busdev_registry.*)))
    	PROVIDE_HIDDEN (__busdev_registry_end = .);
    } >FLASH
  
    /* Legacy symbol, not used anywhere.*/
    . = ALIGN(4);
    PROVIDE(_etext = .);

    /* Special section for exceptions stack.*/
    .mstack :
    {
        . = ALIGN(8);
        __main_stack_base__ = .;
        . += __main_stack_size__;
        . = ALIGN(8);
        __main_stack_end__ = .;
    } > MAIN_STACK_RAM

    /* Special section for process stack.*/
    .pstack :
    {
        __process_stack_base__ = .;
        __main_thread_stack_base__ = .;
        . += __process_stack_size__;
        . = ALIGN(8);
        __process_stack_end__ = .;
        __main_thread_stack_end__ = .;
    } > PROCESS_STACK_RAM

    .data : ALIGN(4)
    {
        . = ALIGN(4);
        PROVIDE(_textdata = LOADADDR(.data));
        PROVIDE(_data = .);
        _textdata_start = LOADADDR(.data);
        _data_start = .;
        *(.data)
        *(.data.*)
        *(.ramtext)
        . = ALIGN(4);
        PROVIDE(_edata = .);
        _data_end = .;
    } > DATA_RAM AT > FLASH

    .bss (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        _bss_start = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
        PROVIDE(end = .);
    } > BSS_RAM
    
    .fastram_bss (NOLOAD) :
  	{
    	__fastram_bss_start__ = .;
    	*(.fastram_bss)
    	*(SORT_BY_ALIGNMENT(.fastram_bss*))
    	. = ALIGN(4);
    	__fastram_bss_end__ = .;
 	} > FASTRAM

    .ram0_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram0_init_text__ = LOADADDR(.ram0_init);
        __ram0_init__ = .;
        *(.ram0_init)
        *(.ram0_init.*)
        . = ALIGN(4);
    } > RAM AT > FLASH

    .ram0 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram0_clear__ = .;
        *(.ram0_clear)
        *(.ram0_clear.*)
        . = ALIGN(4);
        __ram0_noinit__ = .;
        *(.ram0)
        *(.ram0.*)
        . = ALIGN(4);
        __ram0_free__ = .;
    } > RAM

    .ram1_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram1_init_text__ = LOADADDR(.ram1_init);
        __ram1_init__ = .;
        *(.ram1_init)
        *(.ram1_init.*)
        . = ALIGN(4);
    } > ram1 AT > FLASH

    .ram1 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram1_clear__ = .;
        *(.ram1_clear)
        *(.ram1_clear.*)
        . = ALIGN(4);
        __ram1_noinit__ = .;
        *(.ram1)
        *(.ram1.*)
        . = ALIGN(4);
        __ram1_free__ = .;
    } > ram1

    .ram2_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram2_init_text__ = LOADADDR(.ram2_init);
        __ram2_init__ = .;
        *(.ram2_init)
        *(.ram2_init.*)
        . = ALIGN(4);
    } > ram2 AT > FLASH

    .ram2 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram2_clear__ = .;
        *(.ram2_clear)
        *(.ram2_clear.*)
        . = ALIGN(4);
        __ram2_noinit__ = .;
        *(.ram2)
        *(.ram2.*)
        . = ALIGN(4);
        __ram2_free__ = .;
    } > ram2

    .ram3_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram3_init_text__ = LOADADDR(.ram3_init);
        __ram3_init__ = .;
        *(.ram3_init)
        *(.ram3_init.*)
        . = ALIGN(4);
    } > ram3 AT > FLASH

    .ram3 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram3_clear__ = .;
        *(.ram3_clear)
        *(.ram3_clear.*)
        . = ALIGN(4);
        __ram3_noinit__ = .;
        *(.ram3)
        *(.ram3.*)
        . = ALIGN(4);
        __ram3_free__ = .;
    } > ram3

    .ram4_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram4_init_text__ = LOADADDR(.ram4_init);
        __ram4_init__ = .;
        *(.ram4_init)
        *(.ram4_init.*)
        . = ALIGN(4);
    } > ram4 AT > FLASH

    .ram4 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram4_clear__ = .;
        *(.ram4_clear)
        *(.ram4_clear.*)
        . = ALIGN(4);
        __ram4_noinit__ = .;
        *(.ram4)
        *(.ram4.*)
        . = ALIGN(4);
        __ram4_free__ = .;
    } > ram4

    .ram5_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram5_init_text__ = LOADADDR(.ram5_init);
        __ram5_init__ = .;
        *(.ram5_init)
        *(.ram5_init.*)
        . = ALIGN(4);
    } > ram5 AT > FLASH

    .ram5 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram5_clear__ = .;
        *(.ram5_clear)
        *(.ram5_clear.*)
        . = ALIGN(4);
        __ram5_noinit__ = .;
        *(.ram5)
        *(.ram5.*)
        . = ALIGN(4);
        __ram5_free__ = .;
    } > ram5

    .ram6_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram6_init_text__ = LOADADDR(.ram6_init);
        __ram6_init__ = .;
        *(.ram6_init)
        *(.ram6_init.*)
        . = ALIGN(4);
    } > ram6 AT > FLASH

    .ram6 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram6_clear__ = .;
        *(.ram6_clear)
        *(.ram6_clear.*)
        . = ALIGN(4);
        __ram6_noinit__ = .;
        *(.ram6)
        *(.ram6.*)
        . = ALIGN(4);
        __ram6_free__ = .;
    } > ram6

    .ram7_init : ALIGN(4)
    {
        . = ALIGN(4);
        __ram7_init_text__ = LOADADDR(.ram7_init);
        __ram7_init__ = .;
        *(.ram7_init)
        *(.ram7_init.*)
        . = ALIGN(4);
    } > ram7 AT > FLASH

    .ram7 (NOLOAD) : ALIGN(4)
    {
        . = ALIGN(4);
        __ram7_clear__ = .;
        *(.ram7_clear)
        *(.ram7_clear.*)
        . = ALIGN(4);
        __ram7_noinit__ = .;
        *(.ram7)
        *(.ram7.*)
        . = ALIGN(4);
        __ram7_free__ = .;
    } > ram7

    /* The default heap uses the (statically) unused part of a RAM section.*/
/*
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __heap_base__ = .;
        . = ORIGIN(HEAP_RAM) + LENGTH(HEAP_RAM);
        __heap_end__ = .;
    } > HEAP_RAM
*/
}

PROVIDE(end = .);
_end            = .;

__heap_base__   = _end;
__heap_end__    = __ram_end__;


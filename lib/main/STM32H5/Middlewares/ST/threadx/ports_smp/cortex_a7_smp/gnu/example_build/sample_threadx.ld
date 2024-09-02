/* Linker script to place sections and symbol values.
 * It references following symbols, which must be defined in code:
 *   Vectors : Entry point
 * 
 * It defines following symbols, which code can use without definition:
 *   __code_start
 *   __exidx_start
 *   __exidx_end
 *   __data_start
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   __stack
 *   __irq_stack
 *   __stack
 *   __pagetable_start
 */
ENTRY(Vectors)

SECTIONS
{

    .vectors 0x80008000:
    {
        _exec = .;
        __code_start = .;
        KEEP(*(VECTORS))
    }

    .init :
    {
        KEEP (*(SORT_NONE(.init)))
    }

    .text :
    {
        KEEP(*(ENABLE_CACHES))
        *(.text*)
    }

    .fini :
    {
        KEEP (*(SORT_NONE(.fini)))
    }

    .rodata :
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }

    .eh_frame :
    {
        KEEP (*(.eh_frame))
    }

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    }

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    }

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    }

    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array ))
        PROVIDE_HIDDEN (__init_array_end = .);
    }

    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array ))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }

    .ctors :
    {
        /* gcc uses crtbegin.o to find the start of
           the constructors, so we make sure it is
           first.  Because this is a wildcard, it
           doesn't matter if the user does not
           actually link against crtbegin.o; the
           linker won't look for a file to match a
           wildcard.  The wildcard also means that it
           doesn't matter which directory crtbegin.o
           is in.  */
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        /* We don't want to include the .ctor section from
           the crtend.o file until after the sorted ctors.
           The .ctor section from the crtend file contains the
           end of ctors marker and it must be last */
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    }

    .dtors :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    }

    .jcr :
    {
        KEEP (*(.jcr))
    }

    .data :
    {
        __data_start = . ;
        *(.data .data.* .gnu.linkonce.d.*)
        SORT(CONSTRUCTORS)
    }

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    }

    .heap (NOLOAD):
    {
        . = ALIGN(64);
        __end__ = .;
        PROVIDE(end = .);
        . = . + 0xA0000;
    }

    .stack (NOLOAD):
    {
        . = ALIGN(64);
        . = . + 4 * 0x4000;
        __stack = .;
        _stack_init_usr = .;
    }

    .irq_stacks (NOLOAD):
    {
        . = ALIGN(64);
        . = . + 4 * 1024;
        __irq_stack = .;
        _stack_init_irq = .;
    }
    
    _end = .;

    .pagetable 0x80100000 (NOLOAD):
    {
        _page_table_top = .;
        __pagetable_start = .;
        . = . + 0x4000;
    }
}

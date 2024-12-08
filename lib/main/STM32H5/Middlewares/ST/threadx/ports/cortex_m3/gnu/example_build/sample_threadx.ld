MEMORY
{
  RAM (wx) : ORIGIN = 0x20000000, LENGTH = 0x00800000
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00400000
}

__STACKSIZE__ = 1024;
__STACKSIZE_PROCESS__ = 0;
__HEAPSIZE__ = 128;

SECTIONS
{
  .vectors : 
  {
    KEEP(*(.vectors .vectors.*))
  } > FLASH

	.text :
	{
		*(.text*)

		KEEP(*(.init))
		KEEP(*(.fini))

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    __ctors_start__ = ALIGN(4);
		*(SORT(.ctors.*))
		*(.ctors)
    __ctors_end__ = ALIGN(4);

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    __dtors_start__ = ALIGN(4);
 		*(SORT(.dtors.*))
 		*(.dtors)
    __dtors_end__ = ALIGN(4);

		*(.rodata*)

		KEEP(*(.eh_frame*))
	} > FLASH

	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;

	__data_load_start__ = ALIGN (4);

	.data : AT (__data_load_start__) 
	{
		__data_start__ = .;

		*(vtable)
		*(.data*)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		
    __data_end__ = .;

	} > RAM

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;

	} > RAM
	
	.heap (COPY):
	{
    __heap_start__ = ALIGN(4);
    *(.heap)
    . = ALIGN(. + __HEAPSIZE__, 4);
    __heap_end__ = ALIGN(4);
	} > RAM

  .stack ALIGN(4) (NOLOAD) :
  {
    __stack_start__ = ALIGN(4);
    *(.stack)
    . = ALIGN(. + __STACKSIZE__, 4);
    __stack_end__ = ALIGN(4);
  } > RAM

  __RAM_segment_used_end__ = .;
}

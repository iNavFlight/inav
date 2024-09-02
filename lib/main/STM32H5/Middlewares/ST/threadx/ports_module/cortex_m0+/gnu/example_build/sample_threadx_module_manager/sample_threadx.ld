MEMORY
{
  UNPLACED_SECTIONS (wx) : ORIGIN = 0x100000000, LENGTH = 0
  AHB_Peripherals (wx) : ORIGIN = 0x50000000, LENGTH = 0x00200000
  APB1_Peripherals (wx) : ORIGIN = 0x40080000, LENGTH = 0x00080000
  APB0_Peripherals (wx) : ORIGIN = 0x40000000, LENGTH = 0x00080000
  GPIO (wx) : ORIGIN = 0x2009c000, LENGTH = 0x00004000
  AHBSRAM1 (wx) : ORIGIN = 0x20080000, LENGTH = 0x00004000
  AHBSRAM0 (wx) : ORIGIN = 0x2007c000, LENGTH = 0x00004000
  RAM (wx) : ORIGIN = 0x20000000, LENGTH = 0x00020000
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00030000
}


SECTIONS
{
  __AHB_Peripherals_segment_start__ = 0x50000000;
  __AHB_Peripherals_segment_end__ = 0x50200000;
  __APB1_Peripherals_segment_start__ = 0x40080000;
  __APB1_Peripherals_segment_end__ = 0x40100000;
  __APB0_Peripherals_segment_start__ = 0x40000000;
  __APB0_Peripherals_segment_end__ = 0x40080000;
  __GPIO_segment_start__ = 0x2009c000;
  __GPIO_segment_end__ = 0x200a0000;
  __AHBSRAM1_segment_start__ = 0x20080000;
  __AHBSRAM1_segment_end__ = 0x20084000;
  __AHBSRAM0_segment_start__ = 0x2007c000;
  __AHBSRAM0_segment_end__ = 0x20080000;
  __RAM_segment_start__ = 0x20000000;
  __RAM_segment_end__ = 0x20020000;
  __FLASH_segment_start__ = 0x00000000;
  __FLASH_segment_end__ = 0x00030000;

  __STACKSIZE__ = 1024;
  __STACKSIZE_PROCESS__ = 0;
  __STACKSIZE_IRQ__ = 0;
  __STACKSIZE_FIQ__ = 0;
  __STACKSIZE_SVC__ = 0;
  __STACKSIZE_ABT__ = 0;
  __STACKSIZE_UND__ = 0;
  __HEAPSIZE__ = 128;

  __vectors_load_start__ = __FLASH_segment_start__;
  .vectors __FLASH_segment_start__ : AT(__FLASH_segment_start__)
  {
    __vectors_start__ = .;
    *(.vectors .vectors.*)
  }
  __vectors_end__ = __vectors_start__ + SIZEOF(.vectors);

  . = ASSERT(__vectors_end__ >= __FLASH_segment_start__ && __vectors_end__ <= (__FLASH_segment_start__ + 0x00030000) , "error: .vectors is too large to fit in FLASH memory segment");

  __init_load_start__ = ALIGN(__vectors_end__ , 4);
  .init ALIGN(__vectors_end__ , 4) : AT(ALIGN(__vectors_end__ , 4))
  {
    __init_start__ = .;
    *(.init .init.*)
  }
  __init_end__ = __init_start__ + SIZEOF(.init);

  . = ASSERT(__init_end__ >= __FLASH_segment_start__ && __init_end__ <= (__FLASH_segment_start__ + 0x00030000) , "error: .init is too large to fit in FLASH memory segment");

  __text_load_start__ = ALIGN(__init_end__ , 4);
  .text ALIGN(__init_end__ , 4) : AT(ALIGN(__init_end__ , 4))
  {
    __text_start__ = .;
    *(.text .text.* .glue_7t .glue_7 .gnu.linkonce.t.* .gcc_except_table)
  }
  __text_end__ = __text_start__ + SIZEOF(.text);

  . = ASSERT(__text_end__ >= __FLASH_segment_start__ && __text_end__ <= (__FLASH_segment_start__ + 0x00030000) , "error: .text is too large to fit in FLASH memory segment");

  __dtors_load_start__ = ALIGN(__text_end__ , 4);
  .dtors ALIGN(__text_end__ , 4) : AT(ALIGN(__text_end__ , 4))
  {
    __dtors_start__ = .;
    KEEP (*(SORT(.dtors.*))) KEEP (*(.dtors))
  }
  __dtors_end__ = __dtors_start__ + SIZEOF(.dtors);

  . = ASSERT(__dtors_end__ >= __FLASH_segment_start__ && __dtors_end__ <= (__FLASH_segment_start__ + 0x00030000) , "error: .dtors is too large to fit in FLASH memory segment");

  __ctors_load_start__ = ALIGN(__dtors_end__ , 4);
  .ctors ALIGN(__dtors_end__ , 4) : AT(ALIGN(__dtors_end__ , 4))
  {
    __ctors_start__ = .;
    KEEP (*(SORT(.ctors.*))) KEEP (*(.ctors))
  }
  __ctors_end__ = __ctors_start__ + SIZEOF(.ctors);

  . = ASSERT(__ctors_end__ >= __FLASH_segment_start__ && __ctors_end__ <= (__FLASH_segment_start__ + 0x00030000) , "error: .ctors is too large to fit in FLASH memory segment");

  __rodata_load_start__ = ALIGN(__ctors_end__ , 4);
  .rodata ALIGN(__ctors_end__ , 4) : AT(ALIGN(__ctors_end__ , 4))
  {
    __rodata_start__ = .;
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }
  __rodata_end__ = __rodata_start__ + SIZEOF(.rodata);

  . = ASSERT(__rodata_end__ >= __FLASH_segment_start__ && __rodata_end__ <= (__FLASH_segment_start__ + 0x00030000) , "error: .rodata is too large to fit in FLASH memory segment");

  __fast_load_start__ = ALIGN(__rodata_end__ , 4);
  .fast ALIGN(__RAM_segment_start__ , 4) : AT(ALIGN(__rodata_end__ , 4))
  {
    __fast_start__ = .;
    *(.fast .fast.*)
  }
  __fast_end__ = __fast_start__ + SIZEOF(.fast);

  __fast_load_end__ = __fast_load_start__ + SIZEOF(.fast);

  . = ASSERT((__fast_load_start__ + SIZEOF(.fast)) >= __FLASH_segment_start__ && (__fast_load_start__ + SIZEOF(.fast)) <= (__FLASH_segment_start__ + 0x00030000) , "error: .fast is too large to fit in FLASH memory segment");

  .fast_run ALIGN(__RAM_segment_start__ , 4) (NOLOAD) :
  {
    __fast_run_start__ = .;
    . = MAX(__fast_run_start__ + SIZEOF(.fast), .);
  }
  __fast_run_end__ = __fast_run_start__ + SIZEOF(.fast_run);

  . = ASSERT(__fast_run_end__ >= __RAM_segment_start__ && __fast_run_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .fast_run is too large to fit in RAM memory segment");

  __data_load_start__ = ALIGN(__fast_load_start__ + SIZEOF(.fast) , 4);
  .data ALIGN(__fast_run_end__ , 4) : AT(ALIGN(__fast_load_start__ + SIZEOF(.fast) , 4))
  {
    __data_start__ = .;
    *(.data .data.* .gnu.linkonce.d.*)
  }
  __data_end__ = __data_start__ + SIZEOF(.data);

  __data_load_end__ = __data_load_start__ + SIZEOF(.data);

  __FLASH_segment_used_end__ = ALIGN(__fast_load_start__ + SIZEOF(.fast) , 4) + SIZEOF(.data);

  . = ASSERT((__data_load_start__ + SIZEOF(.data)) >= __FLASH_segment_start__ && (__data_load_start__ + SIZEOF(.data)) <= (__FLASH_segment_start__ + 0x00030000) , "error: .data is too large to fit in FLASH memory segment");

  .data_run ALIGN(__fast_run_end__ , 4) (NOLOAD) :
  {
    __data_run_start__ = .;
    . = MAX(__data_run_start__ + SIZEOF(.data), .);
  }
  __data_run_end__ = __data_run_start__ + SIZEOF(.data_run);

  . = ASSERT(__data_run_end__ >= __RAM_segment_start__ && __data_run_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .data_run is too large to fit in RAM memory segment");

  __bss_load_start__ = ALIGN(__data_run_end__ , 4);
  .bss ALIGN(__data_run_end__ , 4) (NOLOAD) : AT(ALIGN(__data_run_end__ , 4))
  {
    __bss_start__ = .;
    *(.bss .bss.* .gnu.linkonce.b.*) *(COMMON)
  }
  __bss_end__ = __bss_start__ + SIZEOF(.bss);

  . = ASSERT(__bss_end__ >= __RAM_segment_start__ && __bss_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .bss is too large to fit in RAM memory segment");

  __non_init_load_start__ = ALIGN(__bss_end__ , 4);
  .non_init ALIGN(__bss_end__ , 4) (NOLOAD) : AT(ALIGN(__bss_end__ , 4))
  {
    __non_init_start__ = .;
    *(.non_init .non_init.*)
  }
  __non_init_end__ = __non_init_start__ + SIZEOF(.non_init);

  . = ASSERT(__non_init_end__ >= __RAM_segment_start__ && __non_init_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .non_init is too large to fit in RAM memory segment");

  __heap_load_start__ = ALIGN(__non_init_end__ , 4);
  .heap ALIGN(__non_init_end__ , 4) (NOLOAD) : AT(ALIGN(__non_init_end__ , 4))
  {
    __heap_start__ = .;
    *(.heap)
    . = ALIGN(MAX(__heap_start__ + __HEAPSIZE__ , .), 4);
  }
  __heap_end__ = __heap_start__ + SIZEOF(.heap);

  . = ASSERT(__heap_end__ >= __RAM_segment_start__ && __heap_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .heap is too large to fit in RAM memory segment");

  __stack_load_start__ = ALIGN(__heap_end__ , 4);
  .stack ALIGN(__heap_end__ , 4) (NOLOAD) : AT(ALIGN(__heap_end__ , 4))
  {
    __stack_start__ = .;
    *(.stack)
    . = ALIGN(MAX(__stack_start__ + __STACKSIZE__ , .), 4);
  }
  __stack_end__ = __stack_start__ + SIZEOF(.stack);

  . = ASSERT(__stack_end__ >= __RAM_segment_start__ && __stack_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .stack is too large to fit in RAM memory segment");

  __stack_process_load_start__ = ALIGN(__stack_end__ , 4);
  .stack_process ALIGN(__stack_end__ , 4) (NOLOAD) : AT(ALIGN(__stack_end__ , 4))
  {
    __stack_process_start__ = .;
    *(.stack_process)
    . = ALIGN(MAX(__stack_process_start__ + __STACKSIZE_PROCESS__ , .), 4);
  }
  __stack_process_end__ = __stack_process_start__ + SIZEOF(.stack_process);

  __RAM_segment_used_end__ = ALIGN(__stack_end__ , 4) + SIZEOF(.stack_process);

  . = ASSERT(__stack_process_end__ >= __RAM_segment_start__ && __stack_process_end__ <= (__RAM_segment_start__ + 0x00020000) , "error: .stack_process is too large to fit in RAM memory segment");

}


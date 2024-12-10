
/* .text is used instead of .section .text so it works with arm-aout too.  */
    .text
    .code 32
    .align  0

    .global _mainCRTStartup
    .global _start
    .global start
start:
_start:
_mainCRTStartup:

/* Start by setting up a stack */
    /*  Set up the stack pointer to a fixed value */
    ldr r3, .LC0
    mov     sp, r3
    /* Setup a default stack-limit in case the code has been
       compiled with "-mapcs-stack-check".  Hard-wiring this value
       is not ideal, since there is currently no support for
       checking that the heap and stack have not collided, or that
       this default 64k is enough for the program being executed.
       However, it ensures that this simple crt0 world will not
       immediately cause an overflow event:  */
    sub sl, sp, #64 << 10   /* Still assumes 256bytes below sl */
    mov     a2, #0          /* Second arg: fill value */
    mov fp, a2          /* Null frame pointer */
    mov r7, a2          /* Null frame pointer for Thumb */

    ldr a1, .LC1        /* First arg: start of memory block */
    ldr a3, .LC2
    sub a3, a3, a1      /* Third arg: length of block */



        bl      memset
    mov r0, #0      /*  no arguments  */
    mov r1, #0      /*  no argv either */
#ifdef __USES_INITFINI__
    /* Some arm/elf targets use the .init and .fini sections
       to create constructors and destructors, and for these
       targets we need to call the _init function and arrange
       for _fini to be called at program exit.  */
    mov r4, r0
    mov r5, r1
/*        ldr     r0, .Lfini */
        bl      atexit
/*        bl      init */
    mov r0, r4
    mov r1, r5
#endif
        bl      main

        bl      exit         /* Should not return.  */


    /* For Thumb, constants must be after the code since only
       positive offsets are supported for PC relative addresses.  */

    .align 0
.LC0:
.LC1:
    .word   __bss_start__
.LC2:
    .word   __bss_end__
/*
#ifdef __USES_INITFINI__
.Lfini:
    .word   _fini
#endif  */
    /* Return ...  */
#ifdef __APCS_26__
    movs    pc, lr
#else
#ifdef __THUMB_INTERWORK
    bx  lr
#else
    mov pc, lr
#endif
#endif


/*  Workspace for Angel calls.  */
    .data
/*  Data returned by monitor SWI.  */
.global __stack_base__
HeapBase:   .word   0
HeapLimit:  .word   0
__stack_base__: .word   0
StackLimit: .word   0

/**
 * Minimal startup for RP2350 (Cortex-M33)
 *
 * Performs:
 *   - Set initial SP
 *   - Copy .data from flash to RAM
 *   - Zero .bss
 *   - Call SystemInit (if present)
 *   - Call main
 */

    .syntax unified
    .cpu cortex-m33
    .fpu fpv5-sp-d16
    .thumb

.global g_pfnVectors
.global Default_Handler
.global _entry_point

/* Linker script symbols */
.word _sidata
.word _sdata
.word _edata
.word _sbss
.word _ebss

    .section .text.Reset_Handler
    .weak Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* Set stack pointer */
    ldr r0, =_estack
    msr msp, r0

    /* Copy .data section from flash to RAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata
    movs r3, #0
    b LoopCopyDataInit

CopyDataInit:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4

LoopCopyDataInit:
    adds r4, r0, r3
    cmp r4, r1
    bcc CopyDataInit

    /* Zero .bss section */
    ldr r2, =_sbss
    ldr r4, =_ebss
    movs r3, #0
    b LoopFillZerobss

FillZerobss:
    str r3, [r2]
    adds r2, r2, #4

LoopFillZerobss:
    cmp r2, r4
    bcc FillZerobss

    /* Call SystemInit if it exists */
    bl SystemInit

    /* Call main */
    bl main

    /* If main returns, loop forever */
    b .

    .size Reset_Handler, . - Reset_Handler

/**
 * Default handler for unhandled interrupts
 */
    .section .text.Default_Handler, "ax", %progbits
Default_Handler:
Infinite_Loop:
    b Infinite_Loop
    .size Default_Handler, . - Default_Handler

/**
 * Minimal vector table (placed in .isr_vector section)
 */
    .section .isr_vector, "a", %progbits
    .type g_pfnVectors, %object
    .size g_pfnVectors, . - g_pfnVectors

g_pfnVectors:
    .word _estack               /* Top of Stack */
    .word Reset_Handler         /* Reset Handler */
    .word NMI_Handler           /* NMI Handler */
    .word HardFault_Handler     /* Hard Fault Handler */
    .word MemManage_Handler     /* MPU Fault Handler */
    .word BusFault_Handler      /* Bus Fault Handler */
    .word UsageFault_Handler    /* Usage Fault Handler */
    .word SecureFault_Handler   /* Secure Fault Handler (ARMv8-M) */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word SVC_Handler           /* SVCall Handler */
    .word DebugMon_Handler      /* Debug Monitor Handler */
    .word 0                     /* Reserved */
    .word PendSV_Handler        /* PendSV Handler */
    .word SysTick_Handler       /* SysTick Handler */

    /* RP2350 peripheral interrupts - 52 IRQs */
    .word TIMER0_IRQ_0_Handler
    .word TIMER0_IRQ_1_Handler
    .word TIMER0_IRQ_2_Handler
    .word TIMER0_IRQ_3_Handler
    .word TIMER1_IRQ_0_Handler
    .word TIMER1_IRQ_1_Handler
    .word TIMER1_IRQ_2_Handler
    .word TIMER1_IRQ_3_Handler
    .word PWM_IRQ_WRAP_0_Handler
    .word PWM_IRQ_WRAP_1_Handler
    .word DMA_IRQ_0_Handler
    .word DMA_IRQ_1_Handler
    .word DMA_IRQ_2_Handler
    .word DMA_IRQ_3_Handler
    .word USBCTRL_IRQ_Handler
    .word PIO0_IRQ_0_Handler
    .word PIO0_IRQ_1_Handler
    .word PIO1_IRQ_0_Handler
    .word PIO1_IRQ_1_Handler
    .word PIO2_IRQ_0_Handler
    .word PIO2_IRQ_1_Handler
    .word IO_IRQ_BANK0_Handler
    .word IO_IRQ_QSPI_Handler
    .word SIO_IRQ_FIFO_Handler
    .word SPI0_IRQ_Handler
    .word SPI1_IRQ_Handler
    .word UART0_IRQ_Handler
    .word UART1_IRQ_Handler
    .word ADC_IRQ_FIFO_Handler
    .word I2C0_IRQ_Handler
    .word I2C1_IRQ_Handler
    .word OTP_IRQ_Handler
    .word TRNG_IRQ_Handler
    .word 0                     /* Reserved (PROC0_IRQ_CTI) */
    .word 0                     /* Reserved (PROC1_IRQ_CTI) */
    .word PLL_SYS_IRQ_Handler
    .word PLL_USB_IRQ_Handler
    .word POWMAN_IRQ_POW_Handler
    .word POWMAN_IRQ_TIMER_Handler
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word 0                     /* Reserved */
    .word SWI_IRQ_0_Handler
    .word SWI_IRQ_1_Handler
    .word SWI_IRQ_2_Handler
    .word SWI_IRQ_3_Handler
    .word SWI_IRQ_4_Handler
    .word SWI_IRQ_5_Handler

/**
 * Weak aliases for interrupt handlers - default to Default_Handler
 */
    .weak NMI_Handler
    .thumb_set NMI_Handler, Default_Handler
    .weak HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler
    .weak MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler
    .weak BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler
    .weak UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler
    .weak SecureFault_Handler
    .thumb_set SecureFault_Handler, Default_Handler
    .weak SVC_Handler
    .thumb_set SVC_Handler, Default_Handler
    .weak DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler
    .weak PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler
    .weak SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler

    .weak TIMER0_IRQ_0_Handler
    .thumb_set TIMER0_IRQ_0_Handler, Default_Handler
    .weak TIMER0_IRQ_1_Handler
    .thumb_set TIMER0_IRQ_1_Handler, Default_Handler
    .weak TIMER0_IRQ_2_Handler
    .thumb_set TIMER0_IRQ_2_Handler, Default_Handler
    .weak TIMER0_IRQ_3_Handler
    .thumb_set TIMER0_IRQ_3_Handler, Default_Handler
    .weak TIMER1_IRQ_0_Handler
    .thumb_set TIMER1_IRQ_0_Handler, Default_Handler
    .weak TIMER1_IRQ_1_Handler
    .thumb_set TIMER1_IRQ_1_Handler, Default_Handler
    .weak TIMER1_IRQ_2_Handler
    .thumb_set TIMER1_IRQ_2_Handler, Default_Handler
    .weak TIMER1_IRQ_3_Handler
    .thumb_set TIMER1_IRQ_3_Handler, Default_Handler
    .weak PWM_IRQ_WRAP_0_Handler
    .thumb_set PWM_IRQ_WRAP_0_Handler, Default_Handler
    .weak PWM_IRQ_WRAP_1_Handler
    .thumb_set PWM_IRQ_WRAP_1_Handler, Default_Handler
    .weak DMA_IRQ_0_Handler
    .thumb_set DMA_IRQ_0_Handler, Default_Handler
    .weak DMA_IRQ_1_Handler
    .thumb_set DMA_IRQ_1_Handler, Default_Handler
    .weak DMA_IRQ_2_Handler
    .thumb_set DMA_IRQ_2_Handler, Default_Handler
    .weak DMA_IRQ_3_Handler
    .thumb_set DMA_IRQ_3_Handler, Default_Handler
    .weak USBCTRL_IRQ_Handler
    .thumb_set USBCTRL_IRQ_Handler, Default_Handler
    .weak PIO0_IRQ_0_Handler
    .thumb_set PIO0_IRQ_0_Handler, Default_Handler
    .weak PIO0_IRQ_1_Handler
    .thumb_set PIO0_IRQ_1_Handler, Default_Handler
    .weak PIO1_IRQ_0_Handler
    .thumb_set PIO1_IRQ_0_Handler, Default_Handler
    .weak PIO1_IRQ_1_Handler
    .thumb_set PIO1_IRQ_1_Handler, Default_Handler
    .weak PIO2_IRQ_0_Handler
    .thumb_set PIO2_IRQ_0_Handler, Default_Handler
    .weak PIO2_IRQ_1_Handler
    .thumb_set PIO2_IRQ_1_Handler, Default_Handler
    .weak IO_IRQ_BANK0_Handler
    .thumb_set IO_IRQ_BANK0_Handler, Default_Handler
    .weak IO_IRQ_QSPI_Handler
    .thumb_set IO_IRQ_QSPI_Handler, Default_Handler
    .weak SIO_IRQ_FIFO_Handler
    .thumb_set SIO_IRQ_FIFO_Handler, Default_Handler
    .weak SPI0_IRQ_Handler
    .thumb_set SPI0_IRQ_Handler, Default_Handler
    .weak SPI1_IRQ_Handler
    .thumb_set SPI1_IRQ_Handler, Default_Handler
    .weak UART0_IRQ_Handler
    .thumb_set UART0_IRQ_Handler, Default_Handler
    .weak UART1_IRQ_Handler
    .thumb_set UART1_IRQ_Handler, Default_Handler
    .weak ADC_IRQ_FIFO_Handler
    .thumb_set ADC_IRQ_FIFO_Handler, Default_Handler
    .weak I2C0_IRQ_Handler
    .thumb_set I2C0_IRQ_Handler, Default_Handler
    .weak I2C1_IRQ_Handler
    .thumb_set I2C1_IRQ_Handler, Default_Handler
    .weak OTP_IRQ_Handler
    .thumb_set OTP_IRQ_Handler, Default_Handler
    .weak TRNG_IRQ_Handler
    .thumb_set TRNG_IRQ_Handler, Default_Handler
    .weak PLL_SYS_IRQ_Handler
    .thumb_set PLL_SYS_IRQ_Handler, Default_Handler
    .weak PLL_USB_IRQ_Handler
    .thumb_set PLL_USB_IRQ_Handler, Default_Handler
    .weak POWMAN_IRQ_POW_Handler
    .thumb_set POWMAN_IRQ_POW_Handler, Default_Handler
    .weak POWMAN_IRQ_TIMER_Handler
    .thumb_set POWMAN_IRQ_TIMER_Handler, Default_Handler
    .weak SWI_IRQ_0_Handler
    .thumb_set SWI_IRQ_0_Handler, Default_Handler
    .weak SWI_IRQ_1_Handler
    .thumb_set SWI_IRQ_1_Handler, Default_Handler
    .weak SWI_IRQ_2_Handler
    .thumb_set SWI_IRQ_2_Handler, Default_Handler
    .weak SWI_IRQ_3_Handler
    .thumb_set SWI_IRQ_3_Handler, Default_Handler
    .weak SWI_IRQ_4_Handler
    .thumb_set SWI_IRQ_4_Handler, Default_Handler
    .weak SWI_IRQ_5_Handler
    .thumb_set SWI_IRQ_5_Handler, Default_Handler

/**
 * Entry point - jump over boot2 to Reset_Handler
 * The _entry_point is what the linker ENTRY() references.
 * On RP2350, the boot ROM loads boot2 from the first 256 bytes of flash,
 * executes it, then jumps to the vector table that follows.
 */
    .section .text._entry_point
    .type _entry_point, %function
_entry_point:
    b Reset_Handler
    .size _entry_point, . - _entry_point

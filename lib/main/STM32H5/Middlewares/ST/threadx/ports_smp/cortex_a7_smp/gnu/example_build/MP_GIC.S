@;==================================================================
@; Cortex-A MPCore - GIC Code
@;
@; Copyright (c) 2011-2012 ARM Ltd.  All rights reserved.
@;==================================================================

    .arm

@;==================================================================
@; GIC. Generic Interrupt Controller Architecture Specification
@;==================================================================
@
@        ; Interrupt Distributor offset from base of private peripheral space --> 0x1000
@		; CPU Interface offset from base of private peripheral space --> 0x2000
@
@        ; Typical calls to enable interrupt ID X:
@        ; enableIntID(X)                    <-- Enable that ID
@        ; setIntPriority(X, 0)              <-- Set the priority of X to 0 (the max priority)
@        ; setPriorityMask(0x1F)             <-- Set CPU's priority mask to 0x1F (the lowest priority)
@        ; enableGIC()                       <-- Enable the GIC (global)
@        ; enableGICProcessorInterface()     <-- Enable the CPU interface (local to the CPU)
@
@
@        EXPORT  enableGIC
@        ; void enableGIC(void)
@        ; Global enable of the Interrupt Distributor
    .text
    .align 2
    .global enableGIC
    .type   enableGIC,function
enableGIC:
        @ Get base address of private peripheral space
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address
        ADD     r0, r0, #0x1000            @ Add the Distributor offset

        LDR     r1, [r0, #0x000]           @ Read the Distributor Control Register (GICD_CTLR)
        ORR     r1, r1, #0x01              @ Interrupts forwarded
        STR     r1, [r0, #0x000]           @ Write Distributor Control Register (GICD_CTLR)

        BX      lr


@; ------------------------------------------------------------
@
@        EXPORT disableGIC
@        ; void disableGIC(void)
@        ; Global disable of the Interrupt Distributor
    .text
    .align 2
    .global disableGIC
    .type   disableGIC,function
disableGIC:
        @ Get base address of private peripheral space
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address
        ADD     r0, r0, #0x1000            @ Add the Distributor offset

        LDR     r1, [r0, #0x000]           @ Read the Distributor Control Register (GICD_CTLR)
        BIC     r1, r1, #0x01              @ Interrupts not forwarded
        STR     r1, [r0, #0x000]           @ Write the Distributor Control Register (GICD_CTLR)

        BX      lr

@; ------------------------------------------------------------
@
@        EXPORT  enableIntID
@        ; void enableIntID(unsigned int ID)
@        ; Enables the interrupt source number ID
    .text
    .align 2
    .global enableIntID
    .type   enableIntID,function
enableIntID:
        @ Get base address of private peripheral space
        MOV     r1, r0                     @ Back up passed in ID value
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address

        @ Each interrupt source has an enable bit in the GIC.  These
        @ are grouped into registers, with 32 sources per register
        @ First, we need to identify which 32 bit block the interrupt lives in
        MOV     r2, r1                     @ Make working copy of ID in r2
        MOV     r2, r2, LSR #5             @ LSR by 5 places, affective divide by 32
                                           @ r2 now contains the 32 bit block this ID lives in
        MOV     r2, r2, LSL #2             @ Now multiply by 4, to covert offset into an address offset (four bytes per reg)

        @ Now work out which bit within the 32 bit block the ID is
        AND     r1, r1, #0x1F              @ Mask off to give offset within 32bit block
        MOV     r3, #1                     @ Move enable value into r3
        MOV     r3, r3, LSL r1             @ Shift it left to position of ID

        ADD     r2, r2, #0x1100            @ Add the base offset of the Interrupt Set-Enable Registers to the offset for the ID
        STR     r3, [r0, r2]               @ Store out (GICD_ISENABLERn)

        BX      lr

@; ------------------------------------------------------------
@
@        EXPORT  disableIntID
@        ; void disableIntID(unsigned int ID)
@        ; Disables the interrupt source number ID
    .text
    .align 2
    .global disableIntID
    .type   disableIntID,function
disableIntID:
        @ Get base address of private peripheral space
        MOV     r1, r0                     @ Back up passed in ID value
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address

        @ First, we need to identify which 32 bit block the interrupt lives in
        MOV     r2, r1                     @ Make working copy of ID in r2
        MOV     r2, r2, LSR #5             @ LSR by 5 places, affective divide by 32
                                           @ r2 now contains the 32 bit block this ID lives in
        MOV     r2, r2, LSL #2             @ Now multiply by 4, to covert offset into an address offset (four bytes per reg)

        @ Now work out which bit within the 32 bit block the ID is
        AND     r1, r1, #0x1F              @ Mask off to give offset within 32bit block
        MOV     r3, #1                     @ Move enable value into r3
        MOV     r3, r3, LSL r1             @ Shift it left to position of ID in 32 bit block

        ADD     r2, r2, #0x1180            @ Add the base offset of the Interrupt Clear-Enable Registers to the offset for the ID
        STR     r3, [r0, r2]               @ Store out (GICD_ICENABLERn)

        BX      lr

@; ------------------------------------------------------------
@
@        EXPORT setIntPriority
@        ; void setIntPriority(unsigned int ID, unsigned int priority)
@        ; Sets the priority of the specifed ID
@        ; r0 = ID
@        ; r1 = priority
    .text
    .align 2
    .global setIntPriority
    .type   setIntPriority,function
setIntPriority:
        @ Get base address of private peripheral space
        MOV     r2, r0                     @ Back up passed in ID value
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address

        @ r0 = base addr
        @ r1 = priority
        @ r2 = ID

        @ Make sure that priority value is only 5 bits, and convert to expected format
        AND     r1, r1, #0x1F
        MOV     r1, r1, LSL #3

        @ Find which priority register this ID lives in
        BIC     r3, r2, #0x03              @ Make a copy of the ID, clearing off the bottom two bits
                                           @ There are four IDs per reg, by clearing the bottom two bits we get an address offset
        ADD     r3, r3, #0x1400            @ Now add the offset of the Interrupt Priority Registers from the base of the private peripheral space
        ADD     r0, r0, r3                 @ Now add in the base address of the private peripheral space, giving us the absolute address

        @ Now work out which ID in the register it is
        AND     r2, r2, #0x03              @ Clear all but the bottom two bits, leaves which ID in the reg it is (which byte)
        MOV     r2, r2, LSL #3             @ Multiply by 8, this gives a bit offset

        @ Read -> Modify -> Write
        MOV     r12, #0xFF                 @ Mask (8 bits)
        MOV     r12, r12, LSL r2           @ Move mask into correct bit position
        MOV     r1, r1, LSL r2             @ Also, move passed in priority value into correct bit position

        LDR     r3, [r0]                   @ Read current value of the Interrupt Priority Registers (GICD_IPRIORITYRn)
        BIC     r3, r3, r12                @ Clear appropriate field
        ORR     r3, r3, r1                 @ Now OR in the priority value
        STR     r3, [r0]                   @ And store it back again  (GICD_IPRIORITYRn)

        BX      lr

@; ------------------------------------------------------------
@
@        EXPORT enableGICProcessorInterface
@        ; void enableGICProcessorInterface(void)
@        ; Enables the processor interface
@        ; Must be done on each core separately
    .text
    .align 2
    .global enableGICProcessorInterface
    .type   enableGICProcessorInterface,function
enableGICProcessorInterface:
        @ Get base address of private peripheral space
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address
        ADD     r0, r0, #0x2000            @ Add the CPU interface offset

        LDR     r1, [r0]                   @ Read the CPU Interface Control Register (GICC_CTLR)
        ORR     r1, r1, #0x01              @ Set bit 0, the enable bit
        STR     r1, [r0]                   @ Write the CPU Interface Control Register (GICC_CTLR)

        BX      lr


@; ------------------------------------------------------------
@
@        EXPORT disableGICProcessorInterface
@        ; void disableGICProcessorInterface(void)
@        ; Disables the processor interface
@        ; Must be done on each core separately
    .text
    .align 2
    .global disableGICProcessorInterface
    .type   disableGICProcessorInterface,function
disableGICProcessorInterface:
        @ Get base address of private peripheral space
        MRC     p15, 4, r0, c15, c0, 0     @ Read periph base address
        ADD     r0, r0, #0x2000            @ Add the CPU interface offset

        LDR     r1, [r0]                   @ Read the CPU Interface Control Register (GICC_CTLR)
        BIC     r1, r1, #0x01              @ Clear bit 0, the enable bit
        STR     r1, [r0]                   @ Write the CPU Interface Control Register (GICC_CTLR)

        BX      lr


@; ------------------------------------------------------------
@
@        EXPORT setPriorityMask
@        ; void setPriorityMask(unsigned int priority)
@        ; Sets the Priority mask register for the CPU run on
@        ; The reset value masks ALL interrupts!
    .text
    .align 2
    .global setPriorityMask
    .type   setPriorityMask,function
setPriorityMask:

        @ Get base address of private peripheral space
        MRC     p15, 4, r1, c15, c0, 0     @ Read periph base address
        ADD     r1, r1, #0x2000            @ Add the CPU interface offset

        STR     r0, [r1, #0x0004]          @ Write the Interrupt Priority Mask register (GICC_PMR)

        BX      lr

@; ------------------------------------------------------------
@
@       EXPORT  setBinaryPoint
@       ; void setBinaryPoint(unsigned int priority)
@       ; Sets the Binary Point Register for the CPU run on
    .text
    .align 2
    .global setBinaryPoint
    .type   setBinaryPoint,function
setBinaryPoint:

       @ Get base address of private peripheral space
       MRC     p15, 4, r1, c15, c0, 0      @ Read periph base address
       ADD     r1, r1, #0x2000             @ Add the CPU interface offset

       STR     r0, [r1, #0x0008]           @ Write the Binary Point Register (GICC_BPR)

       BX      lr

@; ------------------------------------------------------------
@
@       EXPORT  readIntAck
@       ; unsigned int readIntAck(void)
@       ; Returns the value of the Interrupt Acknowledge Register
    .text
    .align 2
    .global readIntAck
    .type   readIntAck,function
readIntAck:
       MRC     p15, 4, r1, c15, c0, 0      @ Read periph base address
       ADD     r1, r1, #0x2000             @ Add the CPU interface offset

       LDR     r0, [r1, #0x000C]           @ Read the Interrupt Acknowledge Register (GICC_IAR)
       BX      lr

@; ------------------------------------------------------------
@
@       EXPORT  writeEOI
@       ; void writeEOI(unsigned int ID)
@       ; Writes ID to the End Of Interrupt register
    .text
    .align 2
    .global writeEOI
    .type   writeEOI,function
writeEOI:

       @ Get base address of private peripheral space
       MRC     p15, 4, r1, c15, c0, 0      @ Read periph base address
       ADD     r1, r1, #0x2000             @ Add the CPU interface offset

       STR     r0, [r1, #0x0010]           @ Write ID to the End of Interrupt register (GICC_EOIR)

       BX      lr

@;==================================================================
@; SGI
@;==================================================================
@
@       EXPORT sendSGI
@       ; void sendSGI(unsigned int ID, unsigned int target_list, unsigned int filter_list);
@       ; Send a software generate interrupt
    .text
    .align 2
    .global sendSGI
    .type   sendSGI,function
sendSGI:
       AND     r3, r0, #0x0F               @ Mask off unused bits of ID, and move to r3
       AND     r1, r1, #0x0F               @ Mask off unused bits of target_filter
       AND     r2, r2, #0x0F               @ Mask off unused bits of filter_list

       ORR     r3, r3, r1, LSL #16         @ Combine ID and target_filter
       ORR     r3, r3, r2, LSL #24         @ and now the filter list

       @ Get the address of the GIC
       MRC     p15, 4, r0, c15, c0, 0      @ Read periph base address
       ADD     r0, r0, #0x1F00             @ Add offset of the sgi_trigger reg

       STR     r3, [r0]                    @ Write to the Software Generated Interrupt Register  (GICD_SGIR)

       BX      lr

@;==================================================================
@; End of code
@;==================================================================
@
@;==================================================================
@; End of MP_GIC.s
@;==================================================================

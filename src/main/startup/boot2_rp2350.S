/*
 * Boot2 stage 2 bootloader stub for RP2350
 *
 * Minimal placeholder for M1 (build system milestone).
 * For real hardware, replace with a proper boot2 from the Pico SDK.
 *
 * Must be exactly 256 bytes in the .boot2 section.
 */

    .syntax unified
    .cpu cortex-m33
    .thumb

    .section .boot2, "ax"

    .global _boot2_entry
    .type _boot2_entry, %function
_boot2_entry:
    /* Load address of vector table (0x10000100 = flash base + 256) */
    movw r0, #0x0100
    movt r0, #0x1000
    /* Load reset vector from vector table */
    ldr r1, [r0, #4]
    bx r1
    .ltorg
    .size _boot2_entry, . - _boot2_entry

    /* Pad remainder with zeros to exactly 256 bytes */
    .balign 256, 0

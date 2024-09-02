/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Arm Limited
 * Copyright (c) 2023 STMicroelectronics
 */

#include "bootutil/fault_injection_hardening.h"

#ifdef FIH_ENABLE_DOUBLE_VARS
/* Variable that could be (but isn't) changed at runtime to force the compiler
 * not to optimize the double check. Value doesn't matter.
 */
volatile int _fih_mask = _FIH_MASK_VALUE;
fih_int FIH_SUCCESS = {FIH_POSITIVE_VALUE, _FIH_MASK_VALUE ^ FIH_POSITIVE_VALUE};
fih_int FIH_FAILURE = {FIH_NEGATIVE_VALUE, _FIH_MASK_VALUE ^ FIH_NEGATIVE_VALUE};
#else
fih_int FIH_SUCCESS = {FIH_POSITIVE_VALUE};
fih_int FIH_FAILURE = {FIH_NEGATIVE_VALUE};
#endif /* FIH_ENABLE_DOUBLE_VARS */

#ifdef FIH_ENABLE_CFI

#ifdef FIH_ENABLE_DOUBLE_VARS
fih_int _fih_cfi_ctr = {0, 0 ^ _FIH_MASK_VALUE};
#else
fih_int _fih_cfi_ctr = {0};
#endif /* FIH_ENABLE_DOUBLE_VARS */

/* Increment the CFI counter by one, and return the value before the increment.
 */
fih_int fih_cfi_get_and_increment(void)
{
    fih_int saved = _fih_cfi_ctr;
    _fih_cfi_ctr = fih_int_encode(fih_int_decode(saved) + 1);
    return saved;
}

/* Validate that the saved precall value is the same as the value of the global
 * counter. For this to be the case, a fih_ret must have been called between
 * these functions being executed. If the values aren't the same then panic.
 */
void fih_cfi_validate(fih_int saved)
{
    int ret = fih_int_decode(saved);
    if (ret != fih_int_decode(_fih_cfi_ctr)) {
        FIH_PANIC;
    }
}

/* Decrement the global CFI counter by one, so that it has the same value as
 * before the cfi_precall
 */
void fih_cfi_decrement(void)
{
    _fih_cfi_ctr = fih_int_encode(fih_int_decode(_fih_cfi_ctr) - 1);
}

#endif /* FIH_ENABLE_CFI */

#ifdef FIH_ENABLE_GLOBAL_FAIL
/* Global failure loop for bootloader code. Uses attribute used to prevent
 * compiler removing due to non-standard calling procedure. Multiple loop jumps
 * used to make unlooping difficult.
 */
__attribute__((used))
__attribute__((noinline))
void fih_panic_loop(void)
{
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
    __asm volatile ("b fih_panic_loop");
}
#endif /* FIH_ENABLE_GLOBAL_FAIL */

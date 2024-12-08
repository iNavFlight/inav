/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Arm Limited
 * Copyright (c) 2023 STMicroelectronics
 */

#ifndef __FAULT_INJECTION_HARDENING_H__
#define __FAULT_INJECTION_HARDENING_H__

/* Fault injection mitigation library.
 *
 * Has support for different measures, which can either be enabled/disabled
 * separately or by defining one of the MCUBOOT_FIH_PROFILEs.
 *
 * NOTE: These constructs against fault injection attacks are not guaranteed to
 *       be secure for all compilers, but execution is going to be correct and
 *       including them will certainly help to harden the code.
 *
 * FIH_ENABLE_DOUBLE_VARS makes critical variables into a tuple (x, x ^ msk).
 * Then the correctness of x can be checked by XORing the two tuple values
 * together. This also means that comparisons between fih_ints can be verified
 * by doing x == y && x_msk == y_msk.
 *
 * FIH_ENABLE_GLOBAL_FAIL makes all while(1) failure loops redirect to a global
 * failure loop. This loop has mitigations against loop escapes / unlooping.
 * This also means that any unlooping won't immediately continue executing the
 * function that was executing before the failure.
 *
 * FIH_ENABLE_CFI (Control Flow Integrity) creates a global counter that is
 * incremented before every FIH_CALL of vulnerable functions. On the function
 * return the counter is decremented, and after the return it is verified that
 * the counter has the same value as before this process. This can be used to
 * verify that the function has actually been called. This protection is
 * intended to discover that important functions are called in an expected
 * sequence and neither of them is missed due to an instruction skip which could
 * be a result of glitching attack. It does not provide protection against ROP
 * or JOP attacks.
 *
 * FIH_ENABLE_DELAY causes random delays. This makes it hard to cause faults
 * precisely. It requires an RNG. An mbedtls integration is provided in
 * fault_injection_hardening_delay_mbedtls.h, but any RNG that has an entropy
 * source can be used by implementing the fih_delay_random_uchar function.
 *
 * The basic call pattern is:
 *
 * fih_int fih_rc = FIH_FAILURE;
 * FIH_CALL(vulnerable_function, fih_rc, arg1, arg2);
 * if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
 *     FIH_PANIC;
 * }
 *
 * Note that any function called by FIH_CALL must only return using FIH_RETURN,
 * as otherwise the CFI counter will not be decremented and the CFI check will
 * fail causing a panic.
 */

#include "mcuboot_config/mcuboot_config.h"

#if defined(MCUBOOT_FIH_PROFILE_HIGH)

#define FIH_ENABLE_DELAY         /* Requires an entropy source */
#define FIH_ENABLE_DOUBLE_VARS
#define FIH_ENABLE_GLOBAL_FAIL
#define FIH_ENABLE_CFI

#elif defined(MCUBOOT_FIH_PROFILE_MEDIUM)

#define FIH_ENABLE_DOUBLE_VARS
#define FIH_ENABLE_GLOBAL_FAIL
#define FIH_ENABLE_CFI

#elif defined(MCUBOOT_FIH_PROFILE_LOW)

#define FIH_ENABLE_GLOBAL_FAIL
#define FIH_ENABLE_CFI

#elif !defined(MCUBOOT_FIH_PROFILE_OFF)
#define MCUBOOT_FIH_PROFILE_OFF
#endif /* MCUBOOT_FIH_PROFILE */

#ifdef FIH_ENABLE_DELAY
#include "fault_injection_hardening_delay_rng.h"
#endif /* FIH_ENABLE_DELAY */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Non-zero success value to defend against register resets. Zero is the most
 * common value for a corrupted register so complex bit-patterns are used
 */
#ifndef MCUBOOT_FIH_PROFILE_OFF
#define FIH_POSITIVE_VALUE 0x1AAAAAAA
#define FIH_NEGATIVE_VALUE 0x15555555
#else
#define FIH_POSITIVE_VALUE 0
#define FIH_NEGATIVE_VALUE -1
#endif

/* A volatile mask is used to prevent compiler optimization - the mask is xored
 * with the variable to create the backup and the integrity can be checked with
 * another xor. The mask value doesn't _really_ matter that much, as long as
 * it has reasonably high hamming weight.
 */
#define _FIH_MASK_VALUE 0xBEEF

#ifdef FIH_ENABLE_DOUBLE_VARS

/* All ints are replaced with two int - the normal one and a backup which is
 * XORed with the mask.
 */
extern volatile int _fih_mask;
typedef volatile struct {
    volatile int val;
    volatile int msk;
} fih_int;

#else

typedef int fih_int;

#endif /* FIH_ENABLE_DOUBLE_VARS */

extern fih_int FIH_SUCCESS;
extern fih_int FIH_FAILURE;

extern void Error_Handler(void);

#ifdef FIH_ENABLE_GLOBAL_FAIL
/* Global failure handler - more resistant to unlooping. noinline and used are
 * used to prevent optimization
 */
__attribute__((noinline)) __attribute__((used))
void fih_panic_loop(void);
#define FIH_PANIC Error_Handler()
#else
#define FIH_PANIC Error_Handler()
#endif  /* FIH_ENABLE_GLOBAL_FAIL */

/* NOTE: For functions to be inlined outside their compilation unit they have to
 * have the body in the header file. This is required as function calls are easy
 * to skip.
 */
#ifdef FIH_ENABLE_DELAY

/* Delaying logic, with randomness from a CSPRNG */
__attribute__((always_inline)) inline
int fih_delay(void)
{
    unsigned char delay;
    int foo = 0;
    volatile int rc;

    delay = fih_delay_random_uchar();

    for (volatile int i = 0; i < delay; i++) {
        foo++;
    }

    rc = 1;

    /* rc is volatile so if it is the return value then the function cannot be
     * optimized
     */
    return rc;
}

#else

__attribute__((always_inline)) inline
int fih_delay_init(void)
{
    return 1;
}

__attribute__((always_inline)) inline
int fih_delay(void)
{
    return 1;
}
#endif /* FIH_ENABLE_DELAY */

#ifdef FIH_ENABLE_DOUBLE_VARS

__attribute__((always_inline)) inline
void fih_int_validate(fih_int x)
{
    int x_msk = x.msk;
    x_msk = x_msk ^ _fih_mask;
    if (x.val != x_msk) {
        FIH_PANIC;
    }
}

/* Convert a fih_int to an int. Validate for tampering. */
__attribute__((always_inline)) inline
int fih_int_decode(fih_int x)
{
    fih_int_validate(x);
    return x.val;
}

/* Convert an int to a fih_int, can be used to encode specific error codes. */
__attribute__((always_inline)) inline
fih_int fih_int_encode(int x)
{
    fih_int ret = {x, x ^ _fih_mask};
    return ret;
}
#ifdef __ICCARM__
#pragma diag_suppress=Pa082
#endif
/* Standard equality. If A == B then 1, else 0 */
__attribute__((always_inline)) inline
int fih_eq(fih_int x, fih_int y)
{
    int x_msk = x.msk;
    int y_val = y.val;
    fih_int_validate(x);
    fih_int_validate(y);
    return (x.val == y_val) && fih_delay() && (x_msk == y.msk);
}

__attribute__((always_inline)) inline
int fih_not_eq(fih_int x, fih_int y)
{
    int x_msk = x.msk;
    int y_val = y.val;
    fih_int_validate(x);
    fih_int_validate(y);
    return (x.val != y_val) && fih_delay() && (x_msk != y.msk);
}

#else

/* NOOP */
__attribute__((always_inline)) inline
void fih_int_validate(fih_int x)
{
    (void) x;
    return;
}

/* NOOP */
__attribute__((always_inline)) inline
int fih_int_decode(fih_int x)
{
    return x;
}

/* NOOP */
__attribute__((always_inline)) inline
fih_int fih_int_encode(int x)
{
    return x;
}

__attribute__((always_inline)) inline
int fih_eq(fih_int x, fih_int y)
{
    return x == y;
}

__attribute__((always_inline)) inline
int fih_not_eq(fih_int x, fih_int y)
{
    return x != y;
}
#endif /* FIH_ENABLE_DOUBLE_VARS */

/* C has a common return pattern where 0 is a correct value and all others are
 * errors. This function converts 0 to FIH_SUCCESS and any other number to a
 * value that is not FIH_SUCCESS
 */
__attribute__((always_inline)) inline
fih_int fih_int_encode_zero_equality(int x)
{
    if (x) {
        return FIH_FAILURE;
    } else {
        return FIH_SUCCESS;
    }
}

#ifdef FIH_ENABLE_CFI
extern fih_int _fih_cfi_ctr;
#endif /* FIH_ENABLE_CFI */

fih_int fih_cfi_get_and_increment(void);
void fih_cfi_validate(fih_int saved);
void fih_cfi_decrement(void);

/* Label for interacting with FIH testing tool. Can be parsed from the elf file
 * after compilation. Does not require debug symbols.
 */
#if defined(__ICCARM__)
#define FIH_LABEL(str, lin, cnt) __asm volatile ("FIH_LABEL_" str "_" #lin "_" #cnt "::" ::);
#else
#define FIH_LABEL(str) __asm volatile ("FIH_LABEL_" str "_%=:" ::);
#endif

/* Main FIH calling macro. return variable is second argument. Does some setup
 * before and validation afterwards. Inserts labels for use with testing script.
 *
 * First perform the precall step - this gets the current value of the CFI
 * counter and saves it to a local variable, and then increments the counter.
 *
 * Then set the return variable to FIH_FAILURE as a base case.
 *
 * Then perform the function call. As part of the funtion FIH_RET must be called
 * which will decrement the counter.
 *
 * The postcall step gets the value of the counter and compares it to the
 * previously saved value. If this is equal then the function call and all child
 * function calls were performed.
 */
#if defined(__ICCARM__)
#define FIH_CALL(f, ret, ...) FIH_CALL2(f, ret, __LINE__, __COUNTER__, __VA_ARGS__)

#define FIH_CALL2(f, ret, l, c, ...) \
    do { \
        FIH_LABEL("FIH_CALL_START", l, c);        \
        FIH_CFI_PRECALL_BLOCK; \
        ret = FIH_FAILURE; \
        if (fih_delay()) { \
            ret = f(__VA_ARGS__); \
        } \
        FIH_CFI_POSTCALL_BLOCK; \
        fih_int_validate(ret); \
        FIH_LABEL("FIH_CALL_END", l, c);          \
    } while (0)

#else

#define FIH_CALL(f, ret, ...) \
    do { \
        FIH_LABEL("FIH_CALL_START"); \
        FIH_CFI_PRECALL_BLOCK; \
        ret = FIH_FAILURE; \
        if (fih_delay()) { \
            ret = f(__VA_ARGS__); \
        } \
        FIH_CFI_POSTCALL_BLOCK; \
        fih_int_validate(ret); \
        FIH_LABEL("FIH_CALL_END"); \
    } while (0)
#endif

/* FIH return changes the state of the internal state machine. If you do a
 * FIH_CALL then you need to do a FIH_RET else the state machine will detect
 * tampering and panic.
 */
#define FIH_RET(ret) \
    do { \
        FIH_CFI_PRERET; \
        return ret; \
    } while (0)


#ifdef FIH_ENABLE_CFI
/* Macro wrappers for functions - Even when the functions have zero body this
 * saves a few bytes on noop functions as it doesn't generate the call/ret
 *
 * CFI precall function saves the CFI counter and then increments it - the
 * postcall then checks if the counter is equal to the saved value. In order for
 * this to be the case a FIH_RET must have been performed inside the called
 * function in order to decrement the counter, so the function must have been
 * called.
 */
#define FIH_CFI_PRECALL_BLOCK \
    fih_int _fih_cfi_saved_value = fih_cfi_get_and_increment()

#define FIH_CFI_POSTCALL_BLOCK \
        fih_cfi_validate(_fih_cfi_saved_value)

#define FIH_CFI_PRERET \
        fih_cfi_decrement()
#else
#define FIH_CFI_PRECALL_BLOCK
#define FIH_CFI_POSTCALL_BLOCK
#define FIH_CFI_PRERET
#endif  /* FIH_ENABLE_CFI */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FAULT_INJECTION_HARDENING_H__ */

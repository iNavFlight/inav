// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Warnings configuration and common macros for Azure SDK code.
 * Do not include this file directly.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifdef _MSC_VER

// Disable warnings:
// -----------------

// warning C4204: nonstandard extension used: non-constant aggregate initializer
#pragma warning(disable : 4204)

// warning C4221: nonstandard extension used: '...': cannot be initialized using address of
// automatic variable '...'
#pragma warning(disable : 4221)

// warning C28278 : Function appears with no prototype in scope. Only limited analysis can be
// performed. Include the appropriate header or add a prototype. This warning also occurs if
// parameter or return types are omitted in a function definition.
#pragma warning(disable : 28278)

// Treat warnings as errors:
// -------------------------

// warning C4710: '...': function not inlined
#pragma warning(error : 4710)

#endif // _MSC_VER

#ifdef __GNUC__

#pragma GCC diagnostic ignored "-Wmissing-braces"

#endif // __GNUC__

#ifdef __clang__

#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wmissing-braces"

#endif // __clang__

#ifndef _az_CFG_H
#define _az_CFG_H

/**
 * @brief Inline function.
 */
#ifdef _MSC_VER
#define AZ_INLINE static __forceinline
#elif defined(__GNUC__) || defined(__clang__) // !_MSC_VER
#define AZ_INLINE __attribute__((always_inline)) static inline
#else // !_MSC_VER !__GNUC__ !__clang__
#define AZ_INLINE static inline
#endif // _MSC_VER

#if defined(__GNUC__) && __GNUC__ >= 7
#define _az_FALLTHROUGH __attribute__((fallthrough))
#else // !__GNUC__ >= 7
#define _az_FALLTHROUGH \
  do                    \
  {                     \
  } while (0)
#endif // __GNUC__ >= 7

/**
 * @brief Enforce that the return value is handled (only applicable on supported compilers).
 */
#ifdef _MSC_VER
#define AZ_NODISCARD _Check_return_
#elif defined(__GNUC__) || defined(__clang__) // !_MSC_VER
#define AZ_NODISCARD __attribute__((warn_unused_result))
#else // !_MSC_VER !__GNUC__ !__clang__
#define AZ_NODISCARD
#endif // _MSC_VER

// Get the number of elements in an array
#define _az_COUNTOF(array) (sizeof(array) / sizeof((array)[0]))

#endif // _az_CFG_H

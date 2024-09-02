#ifndef PSTDALIGN_H
#define PSTDALIGN_H

/*
 * NOTE: aligned_alloc is defined via paligned_alloc.h
 * and requires aligned_free to be fully portable although
 * free also works on C11 and platforms with posix_memalign.
 *
 * NOTE: C++11 defines alignas as a keyword but then also defines
 * __alignas_is_defined.
 *
 * C++14 does not define __alignas_is_defined, at least sometimes.
 *
 * GCC 8.3 reverts on this and makes C++11 behave the same as C++14
 * preventing a simple __cplusplus version check from working.
 *
 * Clang C++ without std=c++11 or std=c++14 does define alignas
 * but does so incorrectly wrt. C11 and C++11 semantics because
 * `alignas(4) float x;` is not recognized.
 * To fix such issues, either move to a std version, or
 * include a working stdalign.h for the given compiler before
 * this file.
 *
 * newlib defines _Alignas and _Alignof in sys/cdefs but rely on
 * gcc version for <stdaligh.h> which can lead to conflicts if
 * stdalign is not included.
 *
 * newlibs need for <stdalign.h> conflicts with broken C++ stdalign
 * but this can be fixed be using std=C++11 or newer.
 *
 * MSVC does not support <stdalign.h> at least up to MSVC 2015,
 * but does appear to support alignas and alignof keywords in
 * recent standard C++. 
 *
 * If stdalign.h is supported but heuristics in this file are
 * insufficient to detect this, try including <stdaligh.h> manually
 * or define HAVE_STDALIGN_H.
 */

/* https://github.com/dvidelabs/flatcc/issues/130 */
#ifndef __alignas_is_defined
#if defined(__cplusplus)
#if __cplusplus == 201103 && !defined(__clang__) && ((__GNUC__ > 8) || (__GNUC__ == 8 && __GNUC_MINOR__ >= 3))
#define __alignas_is_defined 1
#define __alignof_is_defined 1
#include <stdalign.h>
#endif
#endif
#endif

/* Allow for alternative solution to be included first. */
#ifndef __alignas_is_defined

#ifdef __cplusplus
#if defined(PORTABLE_PATCH_CPLUSPLUS_STDALIGN)
#include <stdalign.h>
#undef alignas
#define alignas(t) __attribute__((__aligned__(t)))
#endif
#endif

#if !defined(PORTABLE_HAS_INCLUDE_STDALIGN)
#if defined(__has_include)
#if __has_include(<stdalign.h>)
#define PORTABLE_HAS_INCLUDE_STDALIGN 1
#else
#define PORTABLE_HAS_INCLUDE_STDALIGN 0
#endif
#endif
#endif

 /* https://lists.gnu.org/archive/html/bug-gnulib/2015-08/msg00003.html */
#if defined(__cplusplus)
#if !defined(_MSC_VER)
#include <stdalign.h>
#endif
#if __cplusplus > 201103
#define __alignas_is_defined 1
#define __alignof_is_defined 1
#endif
#elif PORTABLE_HAS_INCLUDE_STDALIGN
#include <stdalign.h>
#elif !defined(__clang__) && ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#include <stdalign.h>
#elif defined(HAVE_STDALIGN_H)
#include <stdaligh.h>
#endif

#endif /* __alignas_is_defined */

#ifndef __alignas_is_defined

#ifdef __cplusplus
extern "C" {
#endif

#if (!defined(__clang__) && defined(__GNUC__) && \
        ((__GNUC__ < 4) || (__GNUC__ == 4 && __GNUC_MINOR__ < 7)))
#undef PORTABLE_C11_STDALIGN_MISSING
#define PORTABLE_C11_STDALIGN_MISSING
#endif

#if defined(__IBMC__)
#undef PORTABLE_C11_STDALIGN_MISSING
#define PORTABLE_C11_STDALIGN_MISSING
#endif

#if ((defined(__STDC__) && __STDC__ && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) && \
    !defined(PORTABLE_C11_STDALIGN_MISSING))
/* C11 or newer */
#include <stdalign.h>
#else
#if defined(__GNUC__) || defined(__IBM_ALIGNOF__) || defined(__clang__)

#ifndef _Alignas
#define _Alignas(t) __attribute__((__aligned__(t)))
#endif

#ifndef _Alignof
#define _Alignof(t) __alignof__(t)
#endif

#elif defined(_MSC_VER)

#define _Alignas(t) __declspec (align(t))
#define _Alignof(t) __alignof(t)
#elif defined(__CCRX__)
#define alignas(t)
#define alignof(t)
#else
#error please update pstdalign.h with support for current compiler and library
#endif

#endif /* __STDC__ */

#ifndef alignas
#define alignas _Alignas
#endif

#ifndef alignof
#define alignof _Alignof
#endif

#define __alignas_is_defined 1
#define __alignof_is_defined 1

#ifdef __cplusplus
}
#endif

#endif /* __alignas__is_defined */

#include "paligned_alloc.h"

#endif /* PSTDALIGN_H */

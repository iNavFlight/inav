#ifndef PDIAGNOSTIC_PUSH_H
#define PDIAGNOSTIC_PUSH_H

/*
 * See also comment in "pdiagnostic.h"
 *
 * e.g.
 * #define PDIAGNOSTIC_IGNORE_USED_FUNCTION
 * #define PDIAGNOSTIC_IGNORE_USED_VARIABLE
 * #include "pdiagnostic_push"
 * ...
 * #include "pdiagnostic_pop.h"
 * <eof>
 *
 * or if push pop isn't desired:
 * #define PDIAGNOSTIC_IGNORE_USED_FUNCTION
 * #define PDIAGNOSTIC_IGNORE_USED_VARIABLE
 * #include "pdiagnostic.h"
 * ...
 * <eof>
 *
 *
 * Some if these warnings cannot be ignored
 * at the #pragma level, but might in the future.
 * Use compiler switches like -Wno-unused-function
 * to work around this.
 */

#if defined(_MSC_VER)
#pragma warning( push )
#define PDIAGNOSTIC_PUSHED_MSVC 1
#else
#define PDIAGNOSTIC_PUSHED_MSVC 0
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#define PDIAGNOSTIC_PUSHED_CLANG 1
#else
#define PDIAGNOSTIC_PUSHED_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#if ((__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#define PDIAGNOSTIC_PUSHED_GCC 1
#else
#define PDIAGNOSTIC_PUSHED_GCC 0
#endif // GNUC >= 4.6
#else
#define PDIAGNOSTIC_PUSHED_GCC 0
#endif // defined(__GNUC__) && !defined(__clang__)

#endif /* PDIAGNOSTIC_PUSH_H */

/*
 * We cannot handle nested push, but we can add to the parent context
 * so keep this outside the header include guard.
 */
#include "pdiagnostic.h"

#ifndef PSTATIC_ASSERT_H
#define PSTATIC_ASSERT_H

#include <assert.h>

/* Handle clang */
#ifndef __has_feature
  #define __has_feature(x) 0
#endif

#if defined(static_assert)
#ifndef __static_assert_is_defined
#define __static_assert_is_defined 1
#endif
#endif

/* Handle static_assert as a keyword in C++ and compiler specifics. */
#if !defined(__static_assert_is_defined)

#if defined(__cplusplus)

#if __cplusplus >= 201103L
#define __static_assert_is_defined 1
#elif __has_feature(cxx_static_assert)
#define __static_assert_is_defined 1
#elif defined(_MSC_VER) && (_MSC_VER >= 1600)
#define __static_assert_is_defined 1
#endif

#else

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#define __static_assert_is_defined 1
#elif __has_feature(c_static_assert)
#define static_assert(pred, msg) _Static_assert(pred, msg)
#define __static_assert_is_defined 1
#elif defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/* In case the clib headers are not compliant. */
#define static_assert(pred, msg) _Static_assert(pred, msg)
#define __static_assert_is_defined 1
#endif

#endif /* __cplusplus */
#endif /* __static_assert_is_defined */


#if !defined(__static_assert_is_defined)

#define __PSTATIC_ASSERT_CONCAT_(a, b) static_assert_scope_##a##_line_##b
#define __PSTATIC_ASSERT_CONCAT(a, b) __PSTATIC_ASSERT_CONCAT_(a, b)
#ifdef __COUNTER__
#define static_assert(e, msg) enum { __PSTATIC_ASSERT_CONCAT(__COUNTER__, __LINE__) = 1/(!!(e)) }
#else
#include "pstatic_assert_scope.h"
#define static_assert(e, msg) enum { __PSTATIC_ASSERT_CONCAT(__PSTATIC_ASSERT_COUNTER, __LINE__) = 1/(int)(!!(e)) }
#endif

#define __static_assert_is_defined 1

#endif /* __static_assert_is_defined */

#endif /* PSTATIC_ASSERT_H */

/* Update scope counter outside of include guard. */
#ifdef __PSTATIC_ASSERT_COUNTER
#include "pstatic_assert_scope.h"
#endif

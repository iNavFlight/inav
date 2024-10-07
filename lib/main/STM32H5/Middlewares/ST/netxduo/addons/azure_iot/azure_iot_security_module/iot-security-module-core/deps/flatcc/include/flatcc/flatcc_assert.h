#ifndef FLATCC_ASSERT_H
#define FLATCC_ASSERT_H
#include <asc_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
* This assert abstraction is only used for the flatcc runtime library.
* The flatcc compiler uses Posix assert routines regardless of how this
* file is configured.
*
* This header makes it possible to use systems where assert is not
* valid to use. Note that `<assert.h>` may remain a dependency for static
* assertions.
*
* `FLATCC_ASSERT` is designed to handle errors which cannot be ignored
* and could lead to crash. The portable library may use assertions that
* are not affected by this macro.
*
* `FLATCC_ASSERT` defaults to POSIX assert but can be overrided by a
* preprocessor definition.
*
* Runtime assertions can be entirely disabled by defining
* `FLATCC_NO_ASSERT`.
*/

#ifdef FLATCC_NO_ASSERT 
/* NOTE: This will not affect inclusion of <assert.h> for static assertions. */
#undef FLATCC_ASSERT
#define FLATCC_ASSERT(x) ((void)0)
/* Grisu3 is used for floating point conversion in JSON processing. */
#define GRISU3_NO_ASSERT
#define __SET_ASSERT__
#define __ASSERT_VAL__
#define __ASSERT_REASON__
#else
extern int __SET_ASSERT__;
extern int __ASSERT_VAL__;
extern const char *__ASSERT_REASON__;
#endif

#ifndef FLATCC_ASSERT
#include <assert.h>
#define FLATCC_ASSERT assert
#endif

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_ASSERT_H */

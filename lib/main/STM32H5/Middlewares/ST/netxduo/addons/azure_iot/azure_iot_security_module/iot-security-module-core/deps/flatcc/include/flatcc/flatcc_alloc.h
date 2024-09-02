#ifndef FLATCC_ALLOC_H
#define FLATCC_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * These allocation abstractions are __only__ for runtime libraries.
 *
 * The flatcc compiler uses Posix allocation routines regardless
 * of how this file is configured.
 *
 * This header makes it possible to use systems where malloc is not
 * valid to use. In this case the portable library will not help
 * because it implements Posix / C11 abstractions.
 *
 * Systems like FreeRTOS do not work with Posix memory calls and here it
 * can be helpful to override runtime allocation primitives.
 *
 * In general, it is better to customize the allocator and emitter via
 * flatcc_builder_custom_init and to avoid using the default emitter
 * specific high level calls the copy out a buffer that must later be
 * deallocated. This provides full control of allocation withou the need
 * for this file.
 *
 *
 * IMPORTANT
 *
 * If you override malloc, free, etc., make sure your applications
 * use the same allocation methods. For example, samples/monster.c
 * and several test cases are no longer guaranteed to work out of the
 * box.
 *
 * The changes must only affect target runtime compilation including the
 * the runtime library libflatccrt.
 *
 * The host system flatcc compiler and the compiler library libflatcc
 * should NOT be compiled with non-Posix allocation since the compiler
 * has a dependency on the runtime library and the wrong free operation
 * might be callled. The safest way to avoid this problem this is to
 * compile flatcc with the CMake script and the runtime files with a
 * dedicated build system for the target system.
 */

#include <stdlib.h>

#include <asc_config.h>
/* ASC Disable memory allocation usage */
#ifdef ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
#define FLATCC_ALLOC(n) NULL
#define FLATCC_FREE(p) NULL
#define FLATCC_REALLOC(p,n) NULL
#define FLATCC_CALLOC(nm,n) NULL
#define FLATCC_ALIGNED_ALLOC(a,n) NULL
#define FLATCC_ALIGNED_FREE(p) NULL
#endif /* ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR */

#ifndef FLATCC_ALLOC
#define FLATCC_ALLOC(n) malloc(n)
#endif

#ifndef FLATCC_FREE
#define FLATCC_FREE(p) free(p)
#endif

#ifndef FLATCC_REALLOC
#define FLATCC_REALLOC(p, n) realloc(p, n)
#endif

#ifndef FLATCC_CALLOC
#define FLATCC_CALLOC(nm, n) calloc(nm, n)
#endif

/*
 * Implements `aligned_alloc` and `aligned_free`.
 * Even with C11, this implements non-standard aligned_free needed for portable
 * aligned_alloc implementations.
 */
#ifndef FLATCC_USE_GENERIC_ALIGNED_ALLOC

#ifndef FLATCC_NO_PALIGNED_ALLOC
#include "flatcc/portable/paligned_alloc.h"
#else
#if !defined(__aligned_free_is_defined) || !__aligned_free_is_defined
#define aligned_free free
#endif
#endif

#else /* FLATCC_USE_GENERIC_ALIGNED_ALLOC */

#ifndef FLATCC_ALIGNED_ALLOC
static inline void *__flatcc_aligned_alloc(size_t alignment, size_t size)
{
    char *raw;
    void *buf;
    size_t total_size = (size + alignment - 1 + sizeof(void *));

    if (alignment < sizeof(void *)) {
        alignment = sizeof(void *);
    }
    raw = (char *)(size_t)FLATCC_ALLOC(total_size);
    buf = raw + alignment - 1 + sizeof(void *);
    buf = (void *)(((size_t)buf) & ~(alignment - 1));
    ((void **)buf)[-1] = raw;
    return buf;
}
#define FLATCC_ALIGNED_ALLOC(alignment, size) __flatcc_aligned_alloc(alignment, size)
#endif /* FLATCC_USE_GENERIC_ALIGNED_ALLOC */

#ifndef FLATCC_ALIGNED_FREE
static inline void __flatcc_aligned_free(void *p)
{
    char *raw;

    if (!p) return;
    raw = ((void **)p)[-1];

    FLATCC_FREE(raw);
}
#define FLATCC_ALIGNED_FREE(p) __flatcc_aligned_free(p)
#endif

#endif /* FLATCC_USE_GENERIC_ALIGNED_ALLOC */

#ifndef FLATCC_ALIGNED_ALLOC
#define FLATCC_ALIGNED_ALLOC(a, n) aligned_alloc(a, n)
#endif

#ifndef FLATCC_ALIGNED_FREE
#define FLATCC_ALIGNED_FREE(p) aligned_free(p)
#endif

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_ALLOC_H */

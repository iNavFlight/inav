#ifndef PALIGNED_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * NOTE: MSVC in general has no aligned alloc function that is
 * compatible with free and it is not trivial to implement a version
 * which is. Therefore, to remain portable, end user code needs to
 * use `aligned_free` which is not part of C11 but defined in this header.
 *
 * The same issue is present on some Unix systems not providing
 * posix_memalign.
 *
 * Note that clang and gcc with -std=c11 or -std=c99 will not define
 * _POSIX_C_SOURCE and thus posix_memalign cannot be detected but
 * aligned_alloc is not necessarily available either. We assume
 * that clang always has posix_memalign although it is not strictly
 * correct. For gcc, use -std=gnu99 or -std=gnu11 or don't use -std in
 * order to enable posix_memalign, or live with the fallback until using
 * a system where glibc has a version that supports aligned_alloc.
 *
 * For C11 compliant compilers and compilers with posix_memalign,
 * it is valid to use free instead of aligned_free with the above
 * caveats.
 */

#include <stdlib.h>

/*
 * Define this to see which version is used so the fallback is not
 * enganged unnecessarily:
 *
 * #define PORTABLE_DEBUG_ALIGNED_ALLOC
 */

#if 0
#define PORTABLE_DEBUG_ALIGNED_ALLOC
#endif

#if !defined(PORTABLE_C11_ALIGNED_ALLOC)

#if defined (_ISOC11_SOURCE)
/* glibc aligned_alloc detection. */
#define PORTABLE_C11_ALIGNED_ALLOC 1
#elif defined (__GLIBC__)
/* aligned_alloc is not available in glibc just because __STDC_VERSION__ >= 201112L. */
#define PORTABLE_C11_ALIGNED_ALLOC 0
#elif defined (__clang__)
#define PORTABLE_C11_ALIGNED_ALLOC 0
#elif defined(__IBMC__)
#define PORTABLE_C11_ALIGNED_ALLOC 0
#elif (defined(__STDC__) && __STDC__ && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
#define PORTABLE_C11_ALIGNED_ALLOC 1
#else
#define PORTABLE_C11_ALIGNED_ALLOC 0
#endif

#endif /* PORTABLE_C11_ALIGNED_ALLOC */

/* https://linux.die.net/man/3/posix_memalign */
#if !defined(PORTABLE_POSIX_MEMALIGN) && defined(_GNU_SOURCE)
#define PORTABLE_POSIX_MEMALIGN 1
#endif

/* https://forum.kde.org/viewtopic.php?p=66274 */
#if !defined(PORTABLE_POSIX_MEMALIGN) && defined(_XOPEN_SOURCE)
#if _XOPEN_SOURCE >= 600
#define PORTABLE_POSIX_MEMALIGN 1
#endif
#endif

#if !defined(PORTABLE_POSIX_MEMALIGN) && defined(_POSIX_C_SOURCE)
#if _POSIX_C_SOURCE >= 200112L
#define PORTABLE_POSIX_MEMALIGN 1
#endif
#endif

#if !defined(PORTABLE_POSIX_MEMALIGN) && defined(__clang__)
#define PORTABLE_POSIX_MEMALIGN 1
#endif

#if !defined(PORTABLE_POSIX_MEMALIGN)
#define PORTABLE_POSIX_MEMALIGN 0
#endif

/* https://forum.kde.org/viewtopic.php?p=66274 */
#if (defined(__STDC__) && __STDC__ && defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L)
/* C11 or newer */
#include <stdalign.h>
#endif

/* C11 or newer */
#if !defined(aligned_alloc) && !defined(__aligned_alloc_is_defined)

#if PORTABLE_C11_ALIGNED_ALLOC
#ifdef PORTABLE_DEBUG_ALIGNED_ALLOC
#error "DEBUG: C11_ALIGNED_ALLOC configured"
#endif
#elif defined(_MSC_VER)

/* Aligned _aligned_malloc is not compatible with free. */
#define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)
#define aligned_free(p) _aligned_free(p)
#define __aligned_alloc_is_defined 1
#define __aligned_free_is_defined 1

#elif PORTABLE_POSIX_MEMALIGN

#if defined(__GNUC__)
#if !defined(__GNUCC__)
extern int posix_memalign (void **, size_t, size_t);
#elif __GNUCC__ < 5
extern int posix_memalign (void **, size_t, size_t);
#endif
#endif

static inline void *__portable_aligned_alloc(size_t alignment, size_t size)
{
    int err;
    void *p = 0;

    if (alignment < sizeof(void *)) {
        alignment = sizeof(void *);
    }
    err = posix_memalign(&p, alignment, size);
    if (err && p) {
        free(p);
        p = 0;
    }
    return p;
}

#ifdef PORTABLE_DEBUG_ALIGNED_ALLOC
#error "DEBUG: POSIX_MEMALIGN configured"
#endif

#define aligned_alloc(alignment, size) __portable_aligned_alloc(alignment, size)
#define aligned_free(p) free(p)
#define __aligned_alloc_is_defined 1
#define __aligned_free_is_defined 1

#else

static inline void *__portable_aligned_alloc(size_t alignment, size_t size)
{
    char *raw;
    void *buf;
    size_t total_size = (size + alignment - 1 + sizeof(void *));

    if (alignment < sizeof(void *)) {
        alignment = sizeof(void *);
    }
    raw = (char *)(size_t)malloc(total_size);
    buf = raw + alignment - 1 + sizeof(void *);
    buf = (void *)(((size_t)buf) & ~(alignment - 1));
    ((void **)buf)[-1] = raw;
    return buf;
}

static inline void __portable_aligned_free(void *p)
{
    char *raw;
    
    if (p) {
        raw = (char*)((void **)p)[-1];
        free(raw);
    }
}

#define aligned_alloc(alignment, size) __portable_aligned_alloc(alignment, size)
#define aligned_free(p) __portable_aligned_free(p)
#define __aligned_alloc_is_defined 1
#define __aligned_free_is_defined 1

#ifdef PORTABLE_DEBUG_ALIGNED_ALLOC
#error "DEBUG: aligned_alloc malloc fallback configured"
#endif

#endif

#endif /* aligned_alloc */

#if !defined(aligned_free) && !defined(__aligned_free_is_defined)
#define aligned_free(p) free(p)
#define __aligned_free_is_defined 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* PALIGNED_ALLOC_H */

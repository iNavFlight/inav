#ifndef PENDIAN_H
#define PENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Defines platform optimized (as per linux <endian.h>
 *
 *     le16toh, le32to, le64toh, be16toh, be32toh, be64toh
 *     htole16, htole32, htole64, htobe16, htobe32, htobe64
 *
 * Falls back to auto-detect endian conversion which is also fast
 * if fast byteswap operation was detected.
 *
 * Also defines platform optimized:
 *
 *     bswap16, bswap32, bswap64,
 *
 * with fall-back to shift-or implementation.
 *
 * For convenience also defines:
 *
 *     le8to, be8toh, htole8, htobe8
 *     bswap8
 *
 * The convience functions makes is simpler to define conversion macros
 * based on type size.
 *
 * NOTE: this implementation expects arguments with no side-effects and
 * with appropriately sized unsigned arguments. These are expected to be
 * used with typesafe wrappers.
 */

#ifndef UINT8_MAX
#include "pstdint.h"
#endif

#if defined(__linux__)
#include <endian.h>
#elif defined(__OpenBSD__) || defined(__FreeBSD__)
#include <sys/endian.h>
#endif

#include "pendian_detect.h"

#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#include <stdlib.h>
#define bswap16 _byteswap_ushort
#define bswap32 _byteswap_ulong
#define bswap64 _byteswap_uint64
#endif
#elif defined(__clang__)
#if __has_builtin(__builtin_bswap16)
#ifndef bswap16
#define bswap16 __builtin_bswap16
#endif
#endif
#if __has_builtin(__builtin_bswap32)
#ifndef bswap32
#define bswap32 __builtin_bswap32
#endif
#endif
#if __has_builtin(__builtin_bswap64)
#ifndef bswap64
#define bswap64 __builtin_bswap64
#endif
#endif
#elif defined(__OpenBSD__) || defined(__FreeBSD__)
#ifndef bswap16
#define bswap16 swap16
#endif
#ifndef bswap32
#define bswap32 swap32
#endif
#ifndef bswap64
#define bswap64 swap64
#endif
#elif defined(__GNUC__)  /* Supported since at least GCC 4.4 */
#ifndef bswap32
#define bswap32 __builtin_bswap32
#endif
#ifndef bswap64
#define bswap64 __builtin_bswap64
#endif
#endif

#ifndef bswap16
#define bswap16(v)                                                          \
      (((uint16_t)(v) << 8) | ((uint16_t)(v) >> 8))
#endif

#ifndef bswap32
#define bswap32(v)                                                          \
      ((((uint32_t)(v) << 24))                                              \
          | (((uint32_t)(v) << 8) & UINT32_C(0x00FF0000))                   \
          | (((uint32_t)(v) >> 8) & UINT32_C(0x0000FF00))                   \
          | (((uint32_t)(v) >> 24)))
#endif

#ifndef bswap64
#define bswap64(v)                                                          \
      ((((uint64_t)(v) << 56))                                              \
          | (((uint64_t)(v) << 40) & UINT64_C(0x00FF000000000000))          \
          | (((uint64_t)(v) << 24) & UINT64_C(0x0000FF0000000000))          \
          | (((uint64_t)(v) << 8) & UINT64_C(0x000000FF00000000))           \
          | (((uint64_t)(v) >> 8) & UINT64_C(0x00000000FF000000))           \
          | (((uint64_t)(v) >> 24) & UINT64_C(0x0000000000FF0000))          \
          | (((uint64_t)(v) >> 40) & UINT64_C(0x000000000000FF00))          \
          | (((uint64_t)(v) >> 56)))
#endif

#ifndef bswap8
#define bswap8(v) ((uint8_t)(v))
#endif

#if !defined(le16toh) && defined(letoh16)
#define le16toh letoh16
#define le32toh letoh32
#define le64toh letoh64
#endif

#if !defined(be16toh) && defined(betoh16)
#define be16toh betoh16
#define be32toh betoh32
#define be64toh betoh64
#endif

/* Assume it goes for all. */
#if !defined(le16toh)

#if (__LITTLE_ENDIAN__==1)

#define le16toh(v) (v)
#define le32toh(v) (v)
#define le64toh(v) (v)

#define htole16(v) (v)
#define htole32(v) (v)
#define htole64(v) (v)

#define be16toh(v) bswap16(v)
#define be32toh(v) bswap32(v)
#define be64toh(v) bswap64(v)

#define htobe16(v) bswap16(v)
#define htobe32(v) bswap32(v)
#define htobe64(v) bswap64(v)

#elif (__BIG_ENDIAN__==1)

#define le16toh(v) bswap16(v)
#define le32toh(v) bswap32(v)
#define le64toh(v) bswap64(v)

#define htole16(v) bswap16(v)
#define htole32(v) bswap32(v)
#define htole64(v) bswap64(v)

#define be16toh(v) (v)
#define be32toh(v) (v)
#define be64toh(v) (v)

#define htobe16(v) (v)
#define htobe32(v) (v)
#define htobe64(v) (v)

#else

static const int __pendian_test = 1;

#define le16toh(v) (*(char *)&__pendian_test ? (v) : bswap16(v))
#define le32toh(v) (*(char *)&__pendian_test ? (v) : bswap32(v))
#define le64toh(v) (*(char *)&__pendian_test ? (v) : bswap64(v))

#define htole16(v) (*(char *)&__pendian_test ? (v) : bswap16(v))
#define htole32(v) (*(char *)&__pendian_test ? (v) : bswap32(v))
#define htole64(v) (*(char *)&__pendian_test ? (v) : bswap64(v))

#define be16toh(v) (*(char *)&__pendian_test ? bswap16(v) : (v))
#define be32toh(v) (*(char *)&__pendian_test ? bswap32(v) : (v))
#define be64toh(v) (*(char *)&__pendian_test ? bswap64(v) : (v))

#define htobe16(v) (*(char *)&__pendian_test ? bswap16(v) : (v))
#define htobe32(v) (*(char *)&__pendian_test ? bswap32(v) : (v))
#define htobe64(v) (*(char *)&__pendian_test ? bswap64(v) : (v))

#endif

#endif /* le16toh */

/* Helpers not part of Linux <endian.h> */
#if !defined(le8toh)
#define le8toh(n) (n)
#define htole8(n) (n)
#define be8toh(n) (n)
#define htobe8(n) (n)
#endif

#ifdef __cplusplus
}
#endif

#endif /* PENDIAN_H */

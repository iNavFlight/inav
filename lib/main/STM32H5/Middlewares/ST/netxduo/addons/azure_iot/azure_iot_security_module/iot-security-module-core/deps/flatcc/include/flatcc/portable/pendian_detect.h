/*
 * Uses various known flags to decide endianness and defines:
 *
 * __LITTLE_ENDIAN__ or __BIG_ENDIAN__ if not already defined
 *
 * and also defines
 *
 * __BYTE_ORDER__ to either __ORDER_LITTLE_ENDIAN__ or
 * __ORDER_BIG_ENDIAN__ if not already defined
 *
 * If none of these could be set, __UNKNOWN_ENDIAN__ is defined,
 * which is not a known flag. If __BYTE_ORDER__ is defined but
 * not big or little endian, __UNKNOWN_ENDIAN__ is also defined.
 *
 * Note: Some systems define __BYTE_ORDER without __ at the end
 * - this will be mapped to to __BYTE_ORDER__.
 */

#ifndef PENDIAN_DETECT
#define PENDIAN_DETECT

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __ORDER_LITTLE_ENDIAN__
#define __ORDER_LITTLE_ENDIAN__ 1234
#endif

#ifndef __ORDER_BIG_ENDIAN__
#define __ORDER_BIG_ENDIAN__ 4321
#endif

#ifdef __BYTE_ORDER__

#if (__LITTLE_ENDIAN__ == 1) && __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error __LITTLE_ENDIAN__ inconsistent with __BYTE_ORDER__
#endif

#if (__BIG_ENDIAN__ == 1) && __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
#error __BIG_ENDIAN__ inconsistent with __BYTE_ORDER__
#endif

#else /* __BYTE_ORDER__ */


#if                                                                         \
  (__LITTLE_ENDIAN__ == 1) ||                                               \
  (defined(__BYTE_ORDER) && __BYTE_ORDER == __ORDER_LITTLE_ENDIAN) ||       \
  defined(__ARMEL__) || defined(__THUMBEL__) ||                             \
  defined(__AARCH64EL__) ||                                                 \
  (defined(_MSC_VER) && defined(_M_ARM)) ||                                 \
  defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) ||           \
  defined(_M_X64) || defined(_M_IX86) || defined(_M_I86) ||                 \
  defined(__i386__) || defined(__alpha__) ||                                \
  defined(__ia64) || defined(__ia64__) ||                                   \
  defined(_M_IA64) || defined(_M_ALPHA) ||                                  \
  defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) ||            \
  defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) ||            \
  defined(__bfin__) || defined(__LIT)

#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__

#endif

#if                                                                         \
  (__BIG_ENDIAN__ == 1) ||                                                  \
  (defined(__BYTE_ORDER) && __BYTE_ORDER == __ORDER_BIG_ENDIAN) ||          \
  defined(__ARMEB__) || defined(THUMBEB__) || defined (__AARCH64EB__) ||    \
  defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__) ||           \
  defined(__sparc) || defined(__sparc__) ||                                 \
  defined(_POWER) || defined(__powerpc__) || defined(__ppc__) ||            \
  defined(__hpux) || defined(__hppa) || defined(__s390__) || defined(__BIG)

#define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__

#endif

#endif /* __BYTE_ORDER__ */

#ifdef __BYTE_ORDER__

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#ifndef __LITTLE_ENDIAN__
#define __LITTLE_ENDIAN__ 1
#endif

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#ifndef __BIG_ENDIAN__
#define __BIG_ENDIAN__ 1
#endif

#else

/*
 * Custom extension - we only define __BYTE_ORDER__ if known big or little.
 * User code that understands __BYTE_ORDER__ may also assume unkown if
 * it is not defined by now - this will allow other endian formats than
 * big or little when supported by compiler.
 */
#ifndef __UNKNOWN_ENDIAN__
#define __UNKNOWN_ENDIAN__ 1
#endif

#endif
#endif /* __BYTE_ORDER__ */

#if (__LITTLE_ENDIAN__ == 1) && (__BIG_ENDIAN__ == 1)
#error conflicting definitions of __LITTLE_ENDIAN__ and __BIG_ENDIAN__
#endif

#ifdef __cplusplus
}
#endif

#endif /* PENDIAN_DETECT */

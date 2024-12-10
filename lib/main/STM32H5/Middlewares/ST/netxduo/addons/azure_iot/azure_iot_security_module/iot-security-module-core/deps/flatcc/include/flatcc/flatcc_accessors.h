#ifndef FLATCC_ACCESSORS
#define FLATCC_ACCESSORS

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UINT8_MAX
#include <stdint.h>
#endif

#define __flatcc_basic_scalar_accessors_impl(N, T, W, E)                    \
static inline size_t N ## __size(void)                                      \
{ return sizeof(T); }                                                       \
static inline T *N ## __ptr_add(T *p, size_t i)                             \
{ return p + i; }                                                           \
static inline const T *N ## __const_ptr_add(const T *p, size_t i)           \
{ return p + i; }                                                           \
static inline T N ## _read_from_pe(const void *p)                           \
{ return N ## _cast_from_pe(*(T *)p); }                                     \
static inline T N ## _read_to_pe(const void *p)                             \
{ return N ## _cast_to_pe(*(T *)p); }                                       \
static inline T N ## _read(const void *p)                                   \
{ return *(T *)p; }                                                         \
static inline void N ## _write_from_pe(void *p, T v)                        \
{ *(T *)p = N ## _cast_from_pe(v); }                                        \
static inline void N ## _write_to_pe(void *p, T v)                          \
{ *(T *)p = N ## _cast_to_pe(v); }                                          \
static inline void N ## _write(void *p, T v)                                \
{ *(T *)p = v; }                                                            \
static inline T N ## _read_from_le(const void *p)                           \
{ return N ## _cast_from_le(*(T *)p); }                                     \

#define __flatcc_define_integer_accessors_impl(N, T, W, E)                  \
static inline T N ## _cast_from_pe(T v)                                     \
{ return (T) E ## W ## toh((uint ## W ## _t)v); }                           \
static inline T N ## _cast_to_pe(T v)                                       \
{ return (T) hto ## E ## W((uint ## W ## _t)v); }                           \
static inline T N ## _cast_from_le(T v)                                     \
{ return (T) le ## W ## toh((uint ## W ## _t)v); }                          \
static inline T N ## _cast_to_le(T v)                                       \
{ return (T) htole ## W((uint ## W ## _t)v); }                              \
static inline T N ## _cast_from_be(T v)                                     \
{ return (T) be ## W ## toh((uint ## W ## _t)v); }                          \
static inline T N ## _cast_to_be(T v)                                       \
{ return (T) htobe ## W((uint ## W ## _t)v); }                              \
__flatcc_basic_scalar_accessors_impl(N, T, W, E)

#define __flatcc_define_real_accessors_impl(N, T, W, E)                     \
union __ ## N ## _cast { T v; uint ## W ## _t u; };                         \
static inline T N ## _cast_from_pe(T v)                                     \
{ union __ ## N ## _cast x;                                                 \
  x.v = v; x.u = E ## W ## toh(x.u); return x.v; }                          \
static inline T N ## _cast_to_pe(T v)                                       \
{ union __ ## N ## _cast x;                                                 \
  x.v = v; x.u = hto ## E ## W(x.u); return x.v; }                          \
static inline T N ## _cast_from_le(T v)                                     \
{ union __ ## N ## _cast x;                                                 \
  x.v = v; x.u = le ## W ## toh(x.u); return x.v; }                         \
static inline T N ## _cast_to_le(T v)                                       \
{ union __ ## N ## _cast x;                                                 \
  x.v = v; x.u = htole ## W(x.u); return x.v; }                             \
static inline T N ## _cast_from_be(T v)                                     \
{ union __ ## N ## _cast x;                                                 \
  x.v = v; x.u = be ## W ## toh(x.u); return x.v; }                         \
static inline T N ## _cast_to_be(T v)                                       \
{ union __ ## N ## _cast x;                                                 \
  x.v = v; x.u = htobe ## W(x.u); return x.v; }                             \
__flatcc_basic_scalar_accessors_impl(N, T, W, E)

#define __flatcc_define_integer_accessors(N, T, W, E)                       \
__flatcc_define_integer_accessors_impl(N, T, W, E)

#define __flatcc_define_real_accessors(N, T, W, E)                          \
__flatcc_define_real_accessors_impl(N, T, W, E)

#define __flatcc_define_basic_integer_accessors(NS, TN, T, W, E)            \
__flatcc_define_integer_accessors(NS ## TN, T, W, E)

#define __flatcc_define_basic_real_accessors(NS, TN, T, W, E)               \
__flatcc_define_real_accessors(NS ## TN, T, W, E)

#define __flatcc_define_basic_scalar_accessors(NS, E)                       \
__flatcc_define_basic_integer_accessors(NS, char, char, 8, E)               \
__flatcc_define_basic_integer_accessors(NS, uint8, uint8_t, 8, E)           \
__flatcc_define_basic_integer_accessors(NS, uint16, uint16_t, 16, E)        \
__flatcc_define_basic_integer_accessors(NS, uint32, uint32_t, 32, E)        \
__flatcc_define_basic_integer_accessors(NS, uint64, uint64_t, 64, E)        \
__flatcc_define_basic_integer_accessors(NS, int8, int8_t, 8, E)             \
__flatcc_define_basic_integer_accessors(NS, int16, int16_t, 16, E)          \
__flatcc_define_basic_integer_accessors(NS, int32, int32_t, 32, E)          \
__flatcc_define_basic_integer_accessors(NS, int64, int64_t, 64, E)          \
__flatcc_define_basic_real_accessors(NS, float, float, 32, E)               \
__flatcc_define_basic_real_accessors(NS, double, double, 64, E)

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_ACCESSORS */

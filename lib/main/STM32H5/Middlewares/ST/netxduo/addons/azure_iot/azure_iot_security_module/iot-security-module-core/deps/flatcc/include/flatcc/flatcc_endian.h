#ifndef FLATCC_ENDIAN_H
#define FLATCC_ENDIAN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file provides helper macros to define type-specific macros and
 * inline functions that convert between stored data and native data
 * indedpently of both native (host) endianness and protocol endianness
 * (i.e. the serialized endian format).
 *
 * To detect endianness correctly ensure one of the following is defined.
 *
 *     __LITTLE_ENDIAN__
 *     __BIG_ENDIAN__
 *     FLATBUFFERS_LITTLEENDIAN=1
 *     FLATBUFFERS_LITTLEENDIAN=0
 *
 * Note: the Clang compiler likely already does this, but other
 * compilers may have their own way, if at all.
 *
 * It is also necessary to include <endian.h> or a compatible
 * implementation in order to provide:
 *
 *     le16toh, le32to, le64toh, be16toh, be32toh, be64toh,
 *     htole16, htole32, htole64, htobe16, htobe32, htobe64.
 *
 * A simple way to ensure all of the above for most platforms is
 * to include the portable endian support file:
 *
 *     #include "flatcc/portable/pendian.h"
 *
 * It is also necessary to include
 *
 *      #include "flatcc/flatcc_types.h"
 *
 * or an equivalent file. This makes it possible to change the
 * endianness of the serialized data and the sizes of flatbuffer
 * specific types such as `uoffset_t`.
 *
 * Note: the mentioned include files are likely already included
 * by the file including this file, at least for the default
 * configuration.
 */

#ifndef UINT8_t
#include <stdint.h>
#endif

/* These are needed to simplify accessor macros and are not found in <endian.h>. */
#ifndef le8toh
#define le8toh(n) (n)
#endif

#ifndef be8toh
#define be8toh(n) (n)
#endif

#ifndef htole8
#define htole8(n) (n)
#endif

#ifndef htobe8
#define htobe8(n) (n)
#endif

#include "flatcc/flatcc_accessors.h"

/* This is the binary encoding endianness, usually LE for flatbuffers. */
#if FLATBUFFERS_PROTOCOL_IS_LE
#define flatbuffers_endian le
#elif FLATBUFFERS_PROTOCOL_IS_BE
#define flatbuffers_endian be
#else
#error "flatbuffers has no defined endiannesss"
#endif

 __flatcc_define_basic_scalar_accessors(flatbuffers_, flatbuffers_endian)

 __flatcc_define_integer_accessors(flatbuffers_bool, flatbuffers_bool_t,
         FLATBUFFERS_BOOL_WIDTH, flatbuffers_endian)
 __flatcc_define_integer_accessors(flatbuffers_union_type, flatbuffers_union_type_t,
         FLATBUFFERS_UTYPE_WIDTH, flatbuffers_endian)

 __flatcc_define_integer_accessors(__flatbuffers_uoffset, flatbuffers_uoffset_t,
         FLATBUFFERS_UOFFSET_WIDTH, flatbuffers_endian)
 __flatcc_define_integer_accessors(__flatbuffers_soffset, flatbuffers_soffset_t,
         FLATBUFFERS_SOFFSET_WIDTH, flatbuffers_endian)
 __flatcc_define_integer_accessors(__flatbuffers_voffset, flatbuffers_voffset_t,
         FLATBUFFERS_VOFFSET_WIDTH, flatbuffers_endian)
 __flatcc_define_integer_accessors(__flatbuffers_utype, flatbuffers_utype_t,
         FLATBUFFERS_UTYPE_WIDTH, flatbuffers_endian)
 __flatcc_define_integer_accessors(__flatbuffers_thash, flatbuffers_thash_t,
         FLATBUFFERS_THASH_WIDTH, flatbuffers_endian)

/* flatcc/portable/pendian.h sets LITTLE/BIG flags if possible, and always defines le16toh. */
#ifndef flatbuffers_is_native_pe
#if (__LITTLE_ENDIAN__==1) || FLATBUFFERS_LITTLEENDIAN
#undef FLATBUFFERS_LITTLEENDIAN
#define FLATBUFFERS_LITTLEENDIAN 1
#define flatbuffers_is_native_pe() (FLATBUFFERS_PROTOCOL_IS_LE)
#elif (__BIG_ENDIAN__==1) || (defined(FLATBUFFERS_LITTLEENDIAN) && !FLATBUFFERS_LITTLEENDIAN)
#undef FLATBUFFERS_LITTLEENDIAN
#define FLATBUFFERS_LITTLEENDIAN 0
#define flatbuffers_is_native_pe() (FLATBUFFERS_PROTOCOL_IS_BE)
#else
#define flatbuffers_is_native_pe() (__FLATBUFFERS_CONCAT(flatbuffers_endian, 16toh)(1) == 1)
#endif
#endif

#ifndef flatbuffers_is_native_le
#define flatbuffers_is_native_le() flatbuffers_is_native_pe()
#endif

#ifndef flatbuffers_is_native_be
#define flatbuffers_is_native_be() (!flatbuffers_is_native_pe())
#endif

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_ENDIAN_H */

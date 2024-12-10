#ifndef FLATCC_TYPES_H
#define FLATCC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#ifndef UINT8_MAX
#include <stdint.h>
#endif

/*
 * This should match generated type declaratios in
 * `flatbuffers_common_reader.h` (might have different name prefix).
 * Read only generated code does not depend on library code,
 * hence the duplication.
 */
#ifndef flatbuffers_types_defined
#define flatbuffers_types_defined

/*
 * uoffset_t and soffset_t must be same integer type, except for sign.
 * They can be (u)int16_t, (u)int32_t, or (u)int64_t.
 * The default is (u)int32_t.
 *
 * voffset_t is expected to be uint16_t, but can experimentally be
 * compiled from uint8_t up to uint32_t.
 *
 * ID_MAX is the largest value that can index a vtable. The table size
 * is given as voffset value. Each id represents a voffset value index
 * from 0 to max inclusive. Space is required for two header voffset
 * fields and the unaddressible highest index (due to the table size
 * representation). For 16-bit voffsets this yields a max of 2^15 - 4,
 * or (2^16 - 1) / 2 - 3.
 */

#define flatbuffers_uoffset_t_defined
#define flatbuffers_soffset_t_defined
#define flatbuffers_voffset_t_defined
#define flatbuffers_utype_t_defined
#define flatbuffers_bool_t_defined
#define flatbuffers_thash_t_defined
#define flatbuffers_fid_t_defined

/* uoffset_t is also used for vector and string headers. */
#define FLATBUFFERS_UOFFSET_MAX UINT32_MAX
#define FLATBUFFERS_SOFFSET_MAX INT32_MAX
#define FLATBUFFERS_SOFFSET_MIN INT32_MIN
#define FLATBUFFERS_VOFFSET_MAX UINT16_MAX
#define FLATBUFFERS_UTYPE_MAX UINT8_MAX
/* Well - the max of the underlying type. */
#define FLATBUFFERS_BOOL_MAX UINT8_MAX
#define FLATBUFFERS_THASH_MAX UINT32_MAX

#define FLATBUFFERS_ID_MAX (FLATBUFFERS_VOFFSET_MAX / sizeof(flatbuffers_voffset_t) - 3)
/* Vectors of empty structs can yield div by zero, so we must guard against this. */
#define FLATBUFFERS_COUNT_MAX(elem_size) (FLATBUFFERS_UOFFSET_MAX/((elem_size) == 0 ? 1 : (elem_size)))

#define FLATBUFFERS_UOFFSET_WIDTH 32
#define FLATBUFFERS_COUNT_WIDTH 32
#define FLATBUFFERS_SOFFSET_WIDTH 32
#define FLATBUFFERS_VOFFSET_WIDTH 16
#define FLATBUFFERS_UTYPE_WIDTH 8
#define FLATBUFFERS_BOOL_WIDTH 8
#define FLATBUFFERS_THASH_WIDTH 32

#define FLATBUFFERS_TRUE 1
#define FLATBUFFERS_FALSE 0

#define FLATBUFFERS_PROTOCOL_IS_LE 1
#define FLATBUFFERS_PROTOCOL_IS_BE 0

typedef uint32_t flatbuffers_uoffset_t;
typedef int32_t flatbuffers_soffset_t;
typedef uint16_t flatbuffers_voffset_t;
typedef uint8_t flatbuffers_utype_t;
typedef uint8_t flatbuffers_bool_t;
typedef uint32_t flatbuffers_thash_t;
/* Public facing type operations. */
typedef flatbuffers_utype_t flatbuffers_union_type_t;

static const flatbuffers_bool_t flatbuffers_true = FLATBUFFERS_TRUE;
static const flatbuffers_bool_t flatbuffers_false = FLATBUFFERS_FALSE;

#define FLATBUFFERS_IDENTIFIER_SIZE (FLATBUFFERS_THASH_WIDTH / 8)

typedef char flatbuffers_fid_t[FLATBUFFERS_IDENTIFIER_SIZE];

#endif /* flatbuffers_types_defined */

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_TYPES_H */

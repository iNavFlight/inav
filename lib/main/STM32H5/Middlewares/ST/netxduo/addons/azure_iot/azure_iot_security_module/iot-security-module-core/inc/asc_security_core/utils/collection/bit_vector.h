/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/

#ifndef __BIT_VECTOR_H__
#define __BIT_VECTOR_H__

#include <stdbool.h>

#include <asc_config.h>

#define __BV_VEC_IND(index) (index>>3)
#define __BV_VEC_SHIFT(index) (index & 0x07)
#define __BV_CEIL_DIV8(x) ((x>>3)+(!!(x & 0x07)))

#define BIT_VECTOR_DECLARATIONS(type, size) \
typedef struct {\
    char data[__BV_CEIL_DIV8(size)]; \
} bit_vector_##type;

/**
 * @brief Set/Clear bit at index place.
 *
 * @param bit_vector The pointer on @c bit_vector_##type object.
 * @param index The bit index.
 * @param bit   Value to set: true/false.
 * @param size  Bit vector size.
 *
 * @return false on overflow, otherwise true.
 */
bool __bit_vector_set(char bit_vector[], int index, bool bit, int size);

/**
 * @brief Get bit at index place.
 *
 * @param bit_vector The pointer on @c bit_vector_##type object.
 * @param index The bit index.
 * @param size  Bit vector size.
 *
 * @return false on overflow, otherwise the actual value.
 */
bool __bit_vector_get(char bit_vector[], int index, int size);

/**
 * @brief Check if the vector is zero.
 *
 * @param bit_vector The pointer on @c bit_vector_##type object.
 * @param size  Bit vector size.
 *
 * @return true if all bits in vector are zero, otherwise false.
 */
bool __is_bit_vector_zero(char bit_vector[], int size);

/**
 * @brief Set the vector to zero.
 *
 * @param bit_vector The pointer on @c bit_vector_##type object.
 * @param size  Bit vector size.
 *
 * @return none.
 */
void __bit_vector_clean(char bit_vector[], int size);

/**
 * @brief Takes vector size in bits by type.
 *
 * @param bit_vector type
 * 
 * @return vector size in bits.
 */
#define bit_vector_size(type) ((int)(sizeof(bit_vector_##type)<<3))

#define bit_vector_set(type, bit_vector_ptr, index, bit) \
__bit_vector_set((bit_vector_ptr)->data, index, bit, bit_vector_size(type))

#define bit_vector_get(type, bit_vector_ptr, index) \
__bit_vector_get((bit_vector_ptr)->data, index, bit_vector_size(type))

#define is_bit_vector_zero(type, bit_vector_ptr) \
__is_bit_vector_zero((bit_vector_ptr)->data, bit_vector_size(type))

#define bit_vector_clean(type, bit_vector_ptr) \
__bit_vector_clean((bit_vector_ptr)->data, bit_vector_size(type))

#endif /* __BIT_VECTOR_H__ */
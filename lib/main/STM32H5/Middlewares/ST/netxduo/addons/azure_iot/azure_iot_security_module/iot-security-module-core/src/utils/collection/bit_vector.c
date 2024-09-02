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
#include <stdbool.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/bit_vector.h"

bool __bit_vector_set(char bit_vector[], int index, bool bit, int size)
{
    if (index >= 0 && index < size) {
        if (bit) {
            bit_vector[__BV_VEC_IND(index)] |= (char)(0x01 << __BV_VEC_SHIFT(index));
        } else {
            bit_vector[__BV_VEC_IND(index)] &= (char)~(0x01 << __BV_VEC_SHIFT(index));
        }
        return true;
    }
    return false;
}

bool __bit_vector_get(char bit_vector[], int index, int size)
{
    if (index >= 0 && index < size) {
        return (!!(bit_vector[__BV_VEC_IND(index)] & (0x01 << __BV_VEC_SHIFT(index))));
    }
    return false;
}

bool __is_bit_vector_zero(char bit_vector[], int size)
{
    int vector_size_in_bytes = size>>3;

    for(int i = 0; i < vector_size_in_bytes; i++) {
        if (bit_vector[i] != 0x00) {
            return false;
        }
    }
    return true;
}

void __bit_vector_clean(char bit_vector[], int size)
{
    memset(bit_vector, 0, (size_t)size>>3);
}

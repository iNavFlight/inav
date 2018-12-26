/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

/*
 * These functions expect array to be 4-byte aligned since they alias array
 * to an uint32_t pointer in order to work fast. Use the BITARRAY_DECLARE()
 * macro to declare a bit array that can be safely used by them.
 */

typedef uint32_t bitarrayElement_t;

#define BITARRAY_DECLARE(name, bits) bitarrayElement_t name[(bits + 31) / 32]

bool bitArrayGet(const bitarrayElement_t *array, unsigned bit);
void bitArraySet(bitarrayElement_t *array, unsigned bit);
void bitArrayClr(bitarrayElement_t *array, unsigned bit);

void bitArraySetAll(bitarrayElement_t *array, size_t size);
void bitArrayClrAll(bitarrayElement_t *array, size_t size);

#define BITARRAY_SET_ALL(array) bitArraySetAll(array, sizeof(array))
#define BITARRAY_CLR_ALL(array) bitArrayClrAll(array, sizeof(array))

// Returns the first set bit with pos >= start_bit, or -1 if all bits
// are zero. Note that size must indicate the size of array in bytes.
// In most cases, you should use the BITARRAY_FIND_FIRST_SET() macro
// to call this function.
int bitArrayFindFirstSet(const bitarrayElement_t *array, unsigned start_bit, size_t size);

#define BITARRAY_FIND_FIRST_SET(array, start_bit) bitArrayFindFirstSet(array, start_bit, sizeof(array))

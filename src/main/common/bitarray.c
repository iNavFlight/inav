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

#include <stdint.h>
#include <stdbool.h>

#include "bitarray.h"

// bit[0] in an element must be the MSB to allow using clz
// to find set bits faster.
#define BITARRAY_BIT_OP(array, bit, op) ((array)[(bit) / (sizeof((array)[0]) * 8)] op ((1u<<31) >> ((bit) % (sizeof((array)[0]) * 8))))

bool bitArrayGet(const void *array, unsigned bit)
{
    return BITARRAY_BIT_OP((uint32_t*)array, bit, &);
}

void bitArraySet(void *array, unsigned bit)
{
    BITARRAY_BIT_OP((uint32_t*)array, bit, |=);
}

void bitArrayClr(void *array, unsigned bit)
{
    BITARRAY_BIT_OP((uint32_t*)array, bit, &=~);
}

__attribute__((always_inline)) static inline uint8_t __CLZ(uint32_t val)
{
#ifdef __arm__
    uint8_t lz;
    __asm__ volatile ("clz %0, %1" : "=r" (lz) : "r" (val) );
    return lz;
#else
    // __builtin_clz is not defined for zero, since it's ARCH
    // dependant. Make it return 32 like ARM's CLZ.
    return val ? __builtin_clz(val) : 32;
#endif
}

int bitArrayFindFirstSet(const void *array, unsigned start, size_t size)
{
    const uint32_t *ptr = (uint32_t*)array;
    const uint32_t *end = ptr + (size / 4);
    const uint32_t *p = ptr + start / (8 * 4);
    int ret;
    // First iteration might need to mask some bits
    uint32_t mask = 0xFFFFFFFF >> (start % (8 * 4));
    if ((ret = __CLZ(*p & mask)) != 32) {
        return ret;
    }
    p++;
    while (p < end) {
        if ((ret = __CLZ(*p)) != 32) {
            return (((char *)p) - ((char *)ptr)) * 8 + ret;
        }
        p++;
    }
    return -1;
}

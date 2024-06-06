/*
 * This file is part of INAV.
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
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdint.h>

#include "gtest/gtest.h"
#include "unittest_macros.h"

#include "io/gimbal_serial.h"

void dumpMemory(uint8_t *mem, int size)
{
    for(int i =0; i < size; ++i) {
        printf("%02x ", mem[i]);
    }
    printf("\n");
}

TEST(GimbalSerialTest, TestGimbalSerialScale)
{
//uint8_t gimbal_scale8(int8_t inputMin, int8_t inputMax, int8_t outputMin, int8_t outputMax, int8_t value);
    uint8_t res8 = gimbal_scale8(-16, 15, 0, 32, 0);
    printf("res8: %i\n", res8);
    EXPECT_TRUE(res8 == 15);
//uint16_t gimbal_scale16(int16_t inputMin, int16_t inputMax, int16_t outputMin, int16_t outputMax, int16_t value);
    uint16_t res16 = gimbal_scale16(1000, 2000, -2048, 2047, 0);
    printf("res16: %i\n", res16);
    EXPECT_TRUE(res16 == 1499);
    //EXPECT_TRUE(valuesAdded == 12);
    //EXPECT_TRUE(1 == valuesAdded);
    //EXPECT_FALSE(memcmp((void *)expected, (void *)&cfg, 17));
    //EXPECT_FALSE(strcmp(buf, " 123.45"));
}
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
#include "drivers/headtracker_common.h"

void dumpMemory(uint8_t *mem, int size)
{
    for(int i =0; i < size; ++i) {
        printf("%02x ", mem[i]);
    }
    printf("\n");
}

extern "C" {
timeUs_t micros(void) {
    return 10;
}
}

TEST(GimbalSerialTest, TestGimbalSerialScale)
{
    int16_t res16 = gimbal_scale12(1000, 2000, 2000);
    EXPECT_TRUE(res16 == 2047);
    res16 = gimbal_scale12(1000, 2000, 1000);
    printf("res16: %i\n", res16);
    EXPECT_TRUE(res16 == -2048);
    res16 = gimbal_scale12(1000, 2000, 1500);
    printf("res16: %i\n", res16);
    EXPECT_TRUE(res16 == -1);
    res16 = gimbal_scale12(1000, 2000, 1501);
    printf("res16: %i\n", res16);
    EXPECT_TRUE(res16 == 3);
}
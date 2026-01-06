/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

//Run only this test:
// cmake -DTOOLCHAIN= .. && make terrain_unittest && ./src/test/unit/terrain_unittest --gtest_filter=Terrain*

#include <stdint.h>
#include <stdbool.h>

extern "C" {
    #include "terrain/terrain_utils.h"
    #include "common/time.h"
    #include "drivers/time.h"
    extern timeUs_t usTicks;
    extern volatile timeMs_t sysTickUptime;
    extern volatile timeMs_t sysTickValStamp;
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

SysTick_Type SysTickValue;
SysTick_Type *SysTick = &SysTickValue;


TEST(TerrainTest, calculateGridInfo)
{
    /////////////////////////////////////////////////////////////////////
    gpsLocation_t loc1 = { .lat = 374221234, .lon = -1220845678, .alt = 0 };
    gridInfo_t info1;

    calculateGridInfo(&loc1, &info1);

    EXPECT_EQ(info1.latDegrees, 37);
    EXPECT_EQ(info1.lonDegrees, -123);

    EXPECT_EQ(info1.latDegrees, 37);
    EXPECT_EQ(info1.lonDegrees, -123);

    EXPECT_EQ(info1.grid_idx_x, 65);
    EXPECT_EQ(info1.grid_idx_y, 96);

    EXPECT_EQ(info1.gridLat, 374204140);
    EXPECT_EQ(info1.gridLon, -1220904251);

    ////////////////////////////////////////////////////////////////////////
    gpsLocation_t loc2 = { .lat = 491304520, .lon = 165976430, .alt = 0 };
    gridInfo_t info2;

    calculateGridInfo(&loc2, &info2);

    EXPECT_EQ(info2.latDegrees, 49);
    EXPECT_EQ(info2.lonDegrees, 16);

    EXPECT_EQ(info2.grid_idx_x, 20);
    EXPECT_EQ(info2.grid_idx_y, 51);

    EXPECT_EQ(info2.idx_x, 4);
    EXPECT_EQ(info2.idx_y, 23);

    EXPECT_EQ(info2.gridLat, 491293581);
    EXPECT_EQ(info2.gridLon, 165873573);

    /////////////////////////////////////////////////////////////////////
}
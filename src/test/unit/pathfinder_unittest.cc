/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

// Written by Alberto G. Hierro <alberto@garciahierro.com>

#include <stdint.h>
#include <stdbool.h>

#include <limits.h>

#include <math.h>

#define PATHFINDER_DECLARE(name, cap) \
    pathfinder_t name; \
    pathfinderPoint_t name_##storage[cap]; \
    pathfinderInit(&name, name_##storage, cap); \

extern "C" {
    #include "flight/pathfinder.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

static void pathfinderCountHelper(pathfinder_t *pf, pathfinder_index_t pos, pathfinderPoint_t *p, void *userData)
{
    UNUSED(pf);
    UNUSED(pos);
    UNUSED(p);

    int *count = static_cast<int*>(userData);
    (*count)++;
}

static int pathfinderEnsureCount(pathfinder_t *pf)
{
    // We count the nodes explicitely to check for consistency of the internal state
    int count = 0;
    pathFinderForEach(pf, pathfinderCountHelper, &count);
    EXPECT_EQ(pathfinderCount(pf), count);
    return pathfinderCount(pf);
}

TEST(PathfinderUnittest, TestAddReset)
{
    PATHFINDER_DECLARE(pf, 2);

    pathfinderAdd(&pf, 0, 0, 0);
    EXPECT_EQ(1, pathfinderEnsureCount(&pf));

    pathfinderReset(&pf);

    EXPECT_EQ(0, pathfinderEnsureCount(&pf));
}

TEST(PathfinderUnittest, TestPruneInsignificant)
{
    PATHFINDER_DECLARE(pf, 4);

    pathfinderAdd(&pf, 1, 0, 0);
    EXPECT_EQ(1, pathfinderEnsureCount(&pf));

    pathfinderAdd(&pf, 2, 0, 0);
    EXPECT_EQ(2, pathfinderEnsureCount(&pf));

    pathfinderAdd(&pf, 3, 0, 0);
    EXPECT_EQ(2, pathfinderEnsureCount(&pf));
}

TEST(PathfinderUnittest, TestIntersect)
{
    PATHFINDER_DECLARE(pf, 8);
    pathfinderSetInitialEpsilon(&pf, 1e-10);

    pathfinderAdd(&pf, 0, 0, 0);
    EXPECT_EQ(1, pathfinderEnsureCount(&pf));

    pathfinderAdd(&pf, 0, 10, 0);
    EXPECT_EQ(2, pathfinderEnsureCount(&pf));

    pathfinderAdd(&pf, 5, 5, 0);
    EXPECT_EQ(3, pathfinderEnsureCount(&pf));

    // This should intersect at (0, 5). Note that
    // we're left with 2 points afterwards because the
    // assumption is that the next Add() will take care
    // of joining (0, 5) to (-1, 5), allowing for the
    // pruning algorithm to overwrite (0, 5) if needed.
    pathfinderAdd(&pf, -1, 5, 0);
    EXPECT_EQ(2, pathfinderEnsureCount(&pf));

    int32_t x, y, z;
    pathfinderPointAt(&pf, 0, &x, &y, &z);
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    pathfinderPointAt(&pf, 1, &x, &y, &z);
    EXPECT_EQ(0, x);
    EXPECT_EQ(5, y);
}

TEST(PathfinderUnittest, TestPop)
{
    PATHFINDER_DECLARE(pf, 8);
    pathfinderSetInitialEpsilon(&pf, 1e-10);

    pathfinderAdd(&pf, 0, 0, 0);
    EXPECT_EQ(1, pathfinderEnsureCount(&pf));

    pathfinderAdd(&pf, 1, 1, 0);
    EXPECT_EQ(2, pathfinderEnsureCount(&pf));

    pathfinderAdd(&pf, 100, 0, 0);
    EXPECT_EQ(3, pathfinderEnsureCount(&pf));

    int32_t x, y, z;

    EXPECT_EQ(true, pathfinderPop(&pf, &x, &y, &z));
    EXPECT_EQ(100, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(2, pathfinderEnsureCount(&pf));

    EXPECT_EQ(true, pathfinderPop(&pf, &x, &y, &z));
    EXPECT_EQ(1, x);
    EXPECT_EQ(1, y);
    EXPECT_EQ(1, pathfinderEnsureCount(&pf));

    EXPECT_EQ(true, pathfinderPop(&pf, &x, &y, &z));
    EXPECT_EQ(0, x);
    EXPECT_EQ(0, y);
    EXPECT_EQ(0, pathfinderEnsureCount(&pf));

    EXPECT_EQ(false, pathfinderPop(&pf, &x, &y, &z));
}

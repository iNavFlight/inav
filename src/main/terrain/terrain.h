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
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdint.h>

#include "navigation/navigation.h"

#define TERRAIN_TASK_RATE_HZ 50

#define TERRAIN_STATUS_FAILURE (-1)
#define TERRAIN_STATUS_WRONG_BLOC_SIZE (-2)
#define TERRAIN_STATUS_WRONG_BITMAP (-3)
#define TERRAIN_STATUS_NO_DATA (-4)
#define TERRAIN_STATUS_NO_CARD (-5)

// MAVLink sends 4x4 grids
#define TERRAIN_GRID_MAVLINK_SIZE 4

// a 2k grid_block on disk contains 8x7 of the mavlink grids.  Each
// grid block overlaps by one with its neighbour. This ensures that
// the altitude at any point can be calculated from a single grid
// block
#define TERRAIN_GRID_BLOCK_MUL_X 7
#define TERRAIN_GRID_BLOCK_MUL_Y 8

// this is the spacing between 32x28 grid blocks, in grid_spacing units
#define TERRAIN_GRID_BLOCK_SPACING_X ((TERRAIN_GRID_BLOCK_MUL_X-1)*TERRAIN_GRID_MAVLINK_SIZE)
#define TERRAIN_GRID_BLOCK_SPACING_Y ((TERRAIN_GRID_BLOCK_MUL_Y-1)*TERRAIN_GRID_MAVLINK_SIZE)

// giving a total grid size of a disk grid_block of 32x28
#define TERRAIN_GRID_BLOCK_SIZE_X (TERRAIN_GRID_MAVLINK_SIZE*TERRAIN_GRID_BLOCK_MUL_X)
#define TERRAIN_GRID_BLOCK_SIZE_Y (TERRAIN_GRID_MAVLINK_SIZE*TERRAIN_GRID_BLOCK_MUL_Y)


#define TASK_TERRAIN_RATE_MS   10
#define TERRAIN_MAX_DISTANCE_CM INT16_MAX

enum GridCacheState {
    GRID_CACHE_INVALID      = 0,    // when first initialised
    GRID_CACHE_DISKWAIT     = 1,    // when waiting for disk read
    GRID_CACHE_VALID        = 2,    // when at least partially valid
};

typedef struct __attribute__((packed)){
        // bitmap of 4x4 grids filled in from GCS (56 bits are used)
        uint64_t bitmap;

        // south west corner of block in degrees*10^7
        int32_t lat;
        int32_t lon;

        // crc of whole block, taken with crc=0
        uint16_t crc;

        // format version number
        uint16_t version;

        // grid spacing in meters
        uint16_t spacing;

        // heights in meters over a 32*28 grid
        int16_t height[TERRAIN_GRID_BLOCK_SIZE_X][TERRAIN_GRID_BLOCK_SIZE_Y];

        // indices info 32x28 grids for this degree reference
        uint16_t grid_idx_x;
        uint16_t grid_idx_y;

        // rounded latitude/longitude in degrees.
        int16_t lonDegrees;
        int8_t latDegrees;

} gridBlock_t;


typedef union {
    gridBlock_t block;
    uint8_t buffer[2048];
} gridIoBlock_t;

/*
  grid_info is a broken down representation of a Location, giving
  the index terms for finding the right grid
 */
typedef struct {
    // rounded latitude/longitude in degrees.
    int8_t latDegrees;
    int16_t lonDegrees;

    // lat and lon of SW corner of this 32*28 grid, *10^7 degrees
    int32_t gridLat;
    int32_t gridLon;

    // indices info 32x28 grids for this degree reference
    uint16_t grid_idx_x;
    uint16_t grid_idx_y;

    // indexes into 32x28 grid
    uint8_t idx_x; // north (0..27)
    uint8_t idx_y; // east  (0..31)

    // fraction within the grid square
    float frac_x; // north (0..1)
    float frac_y; // east  (0..1)

    // file offset of this grid
    uint32_t file_offset;
} gridInfo_t;

typedef struct {
    gridBlock_t gridBlock;
    enum GridCacheState state;
    // the last time access was requested to this block, used for LRU
    timeMs_t lastAccessMs;
} gridCache_t;


typedef struct terrainConfig_s {
    bool terrainEnabled;
} terrainConfig_t;

PG_DECLARE(terrainConfig_t, terrainConfig);

void terrainInit(void);
void terrainUpdateTask(timeUs_t currentTimeUs);
int32_t terrainGetLastDistanceCm(void);

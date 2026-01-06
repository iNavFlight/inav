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

#include <string.h>
#include "platform.h"

#ifdef USE_TERRAIN

#include "terrain.h"
#include "terrain_utils.h"
#include "terrain_location.h"

#include "navigation/navigation.h"

#include "drivers/time.h"


static uint8_t grid_spacing = 30;
static gridCache_t cache[TERRAIN_GRID_BLOCK_CACHE_SIZE];

/**
  given an idx_x and idx_y within a 32x28 grid block, return
  the bit number (0..55) corresponding to the 4x4 subgrid
 */
static bool gridBitnum(uint8_t idx_x, uint8_t idx_y, uint8_t *bitnum)
{
    if (idx_x > 27 || idx_y > 31) {
        return false;
    }

    uint8_t subgrid_x = idx_x / TERRAIN_GRID_MAVLINK_SIZE;
    uint8_t subgrid_y = idx_y / TERRAIN_GRID_MAVLINK_SIZE;

    if (subgrid_x >= TERRAIN_GRID_BLOCK_MUL_X ||
        subgrid_y >= TERRAIN_GRID_BLOCK_MUL_Y) {
        return false;
    }

    *bitnum = subgrid_y + TERRAIN_GRID_BLOCK_MUL_Y * subgrid_x;
    return true;
}

/**
  find a grid block that needs to be read from disk
 */
gridBlock_t* getGridBlockToRead(void)
{
    for (uint16_t i = 0; i < TERRAIN_GRID_BLOCK_CACHE_SIZE; i++) {
        if (cache[i].state == GRID_CACHE_DISKWAIT) {
            return &(cache[i].gridBlock);
        }
    }

    return NULL;
}

/**
  mark a grid block as needing to be read from disk
 */
void markGridBlockNeedRead(gridBlock_t *gridBlock)
{
    for (uint16_t i = 0; i < TERRAIN_GRID_BLOCK_CACHE_SIZE; i++) {
        if (&(cache[i].gridBlock) == gridBlock) {
            cache[i].state = GRID_CACHE_DISKWAIT;
            return;
        }
    }
}

/**
  mark a grid block as having been read from disk
 */
void markGridBlockAsRead(gridBlock_t *gridBlock)
{
    for (uint16_t i = 0; i < TERRAIN_GRID_BLOCK_CACHE_SIZE; i++) {
        if (&(cache[i].gridBlock) == gridBlock) {
            cache[i].state = GRID_CACHE_VALID;
            return;
        }
    }
}

/**
  mark a grid block as having been read from disk
 */
void markGridBlockInvalid(gridBlock_t *gridBlock)
{
    for (uint16_t i = 0; i < TERRAIN_GRID_BLOCK_CACHE_SIZE; i++) {
        if (&(cache[i].gridBlock) == gridBlock) {
            cache[i].state = GRID_CACHE_INVALID;
            return;
        }
    }
}

/**
  given a gps location, calculate the grid info
 */
bool calculateGridInfo(const gpsLocation_t *loc, gridInfo_t *info)
{
    info->latDegrees = (loc->lat < 0 ? (loc->lat - 9999999L) : loc->lat) / 10000000L; // 10 * 1000 * 1000L
    info->lonDegrees = (loc->lon < 0 ? (loc->lon - 9999999L) : loc->lon) / 10000000L; // 10 * 1000 * 1000L

    gpsLocation_t ref;
    ref.lat = info->latDegrees * 10000000L;
    ref.lon = info->lonDegrees * 10000000L;

    neVector_t offset = gpsGetDistanceNE(&ref, loc);

    uint32_t idx_x = offset.north / grid_spacing;
    uint32_t idx_y = offset.east  / grid_spacing;

    info->grid_idx_x = idx_x / TERRAIN_GRID_BLOCK_SPACING_X;
    info->grid_idx_y = idx_y / TERRAIN_GRID_BLOCK_SPACING_Y;

    info->idx_x = idx_x % TERRAIN_GRID_BLOCK_SPACING_X;
    info->idx_y = idx_y % TERRAIN_GRID_BLOCK_SPACING_Y;

    info->frac_x = (offset.north - idx_x * grid_spacing) / grid_spacing;
    info->frac_y = (offset.east  - idx_y * grid_spacing) / grid_spacing;

    gpsLocation_t gridRef = ref;
    offsetLatlng(&gridRef,info->grid_idx_x * TERRAIN_GRID_BLOCK_SPACING_X * grid_spacing,info->grid_idx_y * TERRAIN_GRID_BLOCK_SPACING_Y * grid_spacing);

    info->gridLat = gridRef.lat;
    info->gridLon = gridRef.lon;

    return true;
}

/**
  calculate how many blocks east are in a grid block
 */
uint32_t eastBlocks(gridBlock_t *gridBlock)
{
    gpsLocation_t loc1, loc2;

    loc1.lat = gridBlock->latDegrees * 10 * 1000 * 1000L;
    loc1.lon = gridBlock->lonDegrees * 10 * 1000 * 1000L;

    loc2.lat = loc1.lat;
    loc2.lon = (gridBlock->lonDegrees + 1) * 10 * 1000 * 1000L;

    float east_m = 2.0f * gridBlock->spacing * TERRAIN_GRID_BLOCK_SIZE_Y;
    float lat_rad = (loc2.lat * 1e-7f) * DEG2RAD;
    float dLon_deg = east_m / (111319.5f * cosf(lat_rad));
    loc2.lon += (int32_t)(dLon_deg * 1e7f);

    neVector_t offset = gpsGetDistanceNE(&loc1, &loc2);

    return offset.east / (gridBlock->spacing * TERRAIN_GRID_BLOCK_SPACING_Y);
}

/**
  find or allocate a grid cache entry for a given grid info
 */
gridCache_t* findGridCache(gridInfo_t *info)
{
    uint16_t oldest_i = 0;

    // see if we have that grid
    const timeMs_t nowMs = millis();

    for (uint16_t i = 0; i < TERRAIN_GRID_BLOCK_CACHE_SIZE; i++) {
        gridBlock_t *grid = & cache[i].gridBlock;
        if (TERRAIN_LATLON_EQUAL(grid->lat, info->gridLat) &&
            TERRAIN_LATLON_EQUAL(grid->lon , info->gridLon) &&
            cache[i].gridBlock.spacing == grid_spacing) {
            cache[i].lastAccessMs = nowMs;
            return &cache[i];
        }
        if (cache[i].lastAccessMs < cache[oldest_i].lastAccessMs) {
            oldest_i = i;
        }
    }

    // Not found. Use the oldest grid and make it this grid,
    // initially unpopulated
    gridCache_t *gridCache = &cache[oldest_i];
    memset(gridCache, 0, sizeof(gridCache_t));

    gridCache->gridBlock.lat = info->gridLat;
    gridCache->gridBlock.lon = info->gridLon;
    gridCache->gridBlock.spacing = grid_spacing;
    gridCache->gridBlock.grid_idx_x = info->grid_idx_x;
    gridCache->gridBlock.grid_idx_y = info->grid_idx_y;
    gridCache->gridBlock.latDegrees = info->latDegrees;
    gridCache->gridBlock.lonDegrees = info->lonDegrees;
    gridCache->gridBlock.version = 1;
    gridCache->lastAccessMs = nowMs;

    // mark as waiting for disk read
    gridCache->state = GRID_CACHE_DISKWAIT;

    return gridCache;
}

/*
  given a grid_info check that a given idx_x/idx_y is available (set
  in the bitmap)
 */
bool checkBitmap(gridBlock_t *grid, uint8_t idx_x, uint8_t idx_y)
{
    uint8_t bitnum;
    if(!gridBitnum(idx_x, idx_y, &bitnum)){
        return false;
    }

    return (grid->bitmap & (((uint64_t)1U) << bitnum)) != 0;
}

#endif
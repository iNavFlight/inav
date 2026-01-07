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

#include "platform.h"

#ifdef USE_TERRAIN

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "terrain.h"
#include "terrain_utils.h"
#include "terrain_io.h"

#include "common/utils.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "drivers/sdcard/sdcard.h"
#include "drivers/time.h"


PG_REGISTER_WITH_RESET_TEMPLATE(terrainConfig_t, terrainConfig, PG_TERRAIN_CONFIG, 1);

PG_RESET_TEMPLATE(terrainConfig_t, terrainConfig,
                  .terrainEnabled = false,
);

static bool terrainSubsystemSdCardOk = false;

static struct {
    timeMs_t lastUpdate;
    int32_t terrainAGL;
} terrainHeight;

static struct {
    bool homeAltitudeFound;
    float homeAltitudeM;
    gpsLocation_t homeLocation;
} terrainHomePos = {
    .homeAltitudeFound = false,
    .homeAltitudeM = 0.0f,
};

/**
 * @brief Get the height above mean sea level (AMSL) in meters for a given GPS location.
 */
static float getGeigHtAmslMeters(const gpsLocation_t *loc)
{
    if(isTerrainIoFailure()){
        return TERRAIN_STATUS_FAILURE;
    }

    gridInfo_t info;
    calculateGridInfo(loc, &info);

    if (info.idx_x > TERRAIN_GRID_BLOCK_SIZE_X - 2) {
        return TERRAIN_STATUS_WRONG_BLOC_SIZE;
    }
    if (info.idx_y > TERRAIN_GRID_BLOCK_SIZE_Y - 2) {
        return TERRAIN_STATUS_WRONG_BLOC_SIZE;
    }

    // find the grid
    gridBlock_t *grid = &(findGridCache(&info)->gridBlock);

    // check we have all 4 required heights, it's check if grid is loaded from SD card
    if (!checkBitmap(grid, info.idx_x,   info.idx_y) || !checkBitmap(grid, info.idx_x, info.idx_y + 1) || !checkBitmap(grid, info.idx_x + 1, info.idx_y) || !checkBitmap(grid, info.idx_x + 1, info.idx_y + 1)) {
        markGridBlockNeedRead(grid);
        return TERRAIN_STATUS_WRONG_BITMAP;
    }

    // hXY are the heights of the 4 surrounding grid points
    const int16_t h00 = grid->height[info.idx_x+0][info.idx_y+0];
    const int16_t h01 = grid->height[info.idx_x+0][info.idx_y+1];
    const int16_t h10 = grid->height[info.idx_x+1][info.idx_y+0];
    const int16_t h11 = grid->height[info.idx_x+1][info.idx_y+1];

    // do a simple dual linear interpolation. We could do something
    // fancier, but it probably isn't worth it as long as the
    // grid_spacing is kept small enough
    const float avg1 = (1.0f-info.frac_x) * h00  + info.frac_x * h10;
    const float avg2 = (1.0f-info.frac_x) * h01  + info.frac_x * h11;
    const float avg  = (1.0f-info.frac_y) * avg1 + info.frac_y * avg2;

    return avg;
}

void terrainInit(void)
{
}

/**
 * @brief Update the terrain subsystem.
 *
 * This function checks the status of the SD card and attempts to load terrain data into the cache.
 * It also tries to determine the home altitude based on the GPS origin if it hasn't been found yet.
 */
void terrainUpdateTask(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    //terrain must be enabled
    if(terrainConfig()->terrainEnabled == false){
        return;
    }

    //must be GPS fix
    if(STATE(GPS_FIX) == false){
        return;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////

    if(terrainSubsystemSdCardOk == false){
        terrainSubsystemSdCardOk = sdcard_isInserted() && sdcard_isFunctional() && afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_READY;
    }

    if(terrainSubsystemSdCardOk == true) {
        //this call IO to load data from SD card
        loadGridToCacheTask();
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    //try to find home altitude to safe offset
    //save reference only once when gps origin is valid and different than last saved
    if(terrainHomePos.homeAltitudeFound == false && posControl.gpsOrigin.valid && (posControl.gpsOrigin.lat != terrainHomePos.homeLocation.lat || posControl.gpsOrigin.lon != terrainHomePos.homeLocation.lon)){

        gpsLocation_t homeLoc = {
                .lat = posControl.gpsOrigin.lat,
                .lon = posControl.gpsOrigin.lon,
        };

        float heightASLHome = getGeigHtAmslMeters(&homeLoc);

        if(heightASLHome >= 0){
            terrainHomePos.homeAltitudeFound = true;
            terrainHomePos.homeAltitudeM = heightASLHome;
            terrainHomePos.homeLocation.lat = posControl.gpsOrigin.lat;
            terrainHomePos.homeLocation.lon = posControl.gpsOrigin.lon;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////////////////////////////////////
    float heightASL = getGeigHtAmslMeters(&gpsSol.llh);
    if(heightASL >= 0){
        if(terrainHomePos.homeAltitudeFound){
            terrainHeight.terrainAGL =  MAX(0, ((int32_t)getEstimatedActualPosition(Z) + (int32_t)(terrainHomePos.homeAltitudeM * 100.0f)) - (int32_t)(heightASL * 100.0f));
        }else{
            terrainHeight.terrainAGL = MAX(0, ((int32_t)getEstimatedActualPosition(Z) + GPS_home.alt) - (int32_t)(heightASL * 100.0f));
        }

        terrainHeight.lastUpdate = millis();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////
}

int32_t terrainGetLastDistanceCm(void)
{
    return terrainHeight.lastUpdate + 500 < millis() ? TERRAIN_STATUS_NO_DATA : terrainHeight.terrainAGL;
}

#endif
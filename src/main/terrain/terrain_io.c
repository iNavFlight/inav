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

#include <stdio.h>
#include <string.h>
#include "platform.h"

#ifdef USE_TERRAIN

#include "terrain_io.h"
#include "common/log.h"
#include "drivers/time.h"
#include "terrain_utils.h"
#include "drivers/sdcard/sdcard.h"

#include "io/asyncfatfs/asyncfatfs.h"

static terrainIoState_t terrainIoState;

/**
 * @brief Marks the terrain IO state machine as failed, disabling further operations.
 */
static void hardFailure(void)
{
    terrainIoState.status = TERRAIN_IO_FAILURE;
    terrainIoState.gridBlock = NULL;
    LOG_DEBUG(SYSTEM, "TERRAIN RELEASE FAILURE");
}

/**
 * @brief Resets the terrain IO state machine to its idle state.
 */
static void resetStateMachine(void)
{
    terrainIoState.status = TERRAIN_IO_IDLE;
    terrainIoState.datFile = NULL;
    terrainIoState.gridBlock = NULL;

    LOG_DEBUG(SYSTEM, "TERRAIN RELEASE RESET STATE");
}

/**
 * @brief Retrieves or initializes the file open status for a given latitude and longitude.
 */
static terrainIoFileOpenStatus_t* getFileOpenStatusIndex(int8_t latDegrees, int16_t lonDegrees)
{
    uint16_t oldest_i = 0;
    const timeMs_t nowMs = millis();

    for(uint8_t i = 0; i < TERRAIN_IO_MAX_FILE_OPEN_STATUS; i++){
        if(terrainIoState.fileOpenStatus[i].latDegrees == latDegrees && terrainIoState.fileOpenStatus[i].lonDegrees == lonDegrees){
            terrainIoState.fileOpenStatus[i].lastAccessTimeMs = nowMs;
            return &(terrainIoState.fileOpenStatus[i]);
        }

        if(terrainIoState.fileOpenStatus[i].lastAccessTimeMs < terrainIoState.fileOpenStatus[oldest_i].lastAccessTimeMs){
            oldest_i = i;
        }
    }

    //not found, return oldest
    terrainIoState.fileOpenStatus[oldest_i].latDegrees = latDegrees;
    terrainIoState.fileOpenStatus[oldest_i].lonDegrees = lonDegrees;
    terrainIoState.fileOpenStatus[oldest_i].errorOpenCount = 0;
    terrainIoState.fileOpenStatus[oldest_i].lastAccessTimeMs = nowMs;

    return &(terrainIoState.fileOpenStatus[oldest_i]);
}

/**
 * @brief Callback function invoked when a terrain file is opened.
 */
void terrainIoOpenedFileCallback(afatfsFilePtr_t file)
{
    if(terrainIoState.status != TERRAIN_IO_OPEN_FILE_PENDING){
        hardFailure();
        return;
    }

    if(file == NULL){
        LOG_DEBUG(SYSTEM, "TERRAIN OPEN FILE NULL, RESET STATE");
        markGridBlockInvalid(terrainIoState.gridBlock);
        terrainIoFileOpenStatus_t* fileOpenStatus = getFileOpenStatusIndex(terrainIoState.gridBlock->latDegrees, terrainIoState.gridBlock->lonDegrees);
        fileOpenStatus->errorOpenCount++;
        resetStateMachine();
        return;
    }

    terrainIoState.status = TERRAIN_IO_SEEK;
    terrainIoState.datFile = file;
}

/**
 * @brief Callback function invoked when a terrain file is closed.
 */
void terrainIoClosedFileCallback(void)
{
    if(terrainIoState.status != TERRAIN_IO_CLOSE_PENDING){
        hardFailure();
        return;
    }

    LOG_DEBUG(SYSTEM, "TERRAIN CLOSE FILE OK");
    markGridBlockAsRead(terrainIoState.gridBlock);
    resetStateMachine();
}

/**
 * @brief Main task function for loading terrain grid data into cache.
 */
void loadGridToCacheTask(void)
{
    if(terrainIoState.status == TERRAIN_IO_FAILURE){
        LOG_DEBUG(SYSTEM, "TERRAIN IO FAILURE");
        return;
    }

    ///////////////////////////////////////////////////////////////////
    /////// IDLE          /////////////////////////////////////////////
    // check if in cache is some block which is waiting to read from disk
    if(terrainIoState.status == TERRAIN_IO_IDLE){
        gridBlock_t* gridBlock = getGridBlockToRead();

        //nothing to read
        if(gridBlock == NULL){
            return;
        }

        //check if we had too many errors opening this file
        terrainIoFileOpenStatus_t* fileOpenStatus = getFileOpenStatusIndex(gridBlock->latDegrees, gridBlock->lonDegrees);
        if(fileOpenStatus->errorOpenCount >= 3){
            LOG_DEBUG(SYSTEM, "TERRAIN IO FAILURE TOO MANY OPEN ERRORS");
            markGridBlockInvalid(gridBlock);
            return;
        }

        //set grid block for process. open file -> seek -> read -> close
        terrainIoState.gridBlock = gridBlock;
        terrainIoState.status = TERRAIN_IO_CHANGE_DIR;

        return;
    }

    ///////////////////////////////////////////////////////////////////
    /////// CHANGE DIR   /////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_CHANGE_DIR){
        LOG_DEBUG(SYSTEM, "TERRAIN CHANGING DIR");
        terrainIoState.status = TERRAIN_IO_CHANGE_DIR_PENDING;

        if(!afatfs_chdir(NULL)){
            LOG_DEBUG(SYSTEM, "TERRAIN CHANGE DIR ERROR");
            resetStateMachine();
            return;
        }

        terrainIoState.status = TERRAIN_IO_OPEN_FILE;
        return;
    }
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /////// OPEN FILE     /////////////////////////////////////////////
    // wait to call callback terrainIoOpenedFileCallback
    if(terrainIoState.status == TERRAIN_IO_OPEN_FILE){
        LOG_DEBUG(SYSTEM, "TERRAIN OPENING FILE");
        if(terrainIoState.gridBlock == NULL){
            LOG_DEBUG(SYSTEM, "TERRAIN OPENING FILE GRID BLOCK NULL");
            resetStateMachine();
            return;
        }

        terrainIoState.status = TERRAIN_IO_OPEN_FILE_PENDING;
        char filename[20];
        snprintf(filename, sizeof(filename),
                 "%c%02ld%c%03ld.DAT",
                 terrainIoState.gridBlock->latDegrees < 0 ? 'S' : 'N',
                 labs(terrainIoState.gridBlock->latDegrees),
                 terrainIoState.gridBlock->lonDegrees < 0 ? 'W' : 'E',
                 labs(terrainIoState.gridBlock->lonDegrees));

        if(!afatfs_fopen(filename, "r", terrainIoOpenedFileCallback)){
            LOG_DEBUG(SYSTEM, "TERRAIN OPEN FILE ERROR");
            resetStateMachine();
            return;
        }

        return;
    }
    ///////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    /////// SEEK TO BLOCK ///////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_SEEK){
        if(terrainIoState.datFile == NULL || terrainIoState.gridBlock == NULL){
            resetStateMachine();
            return;
        }

        LOG_DEBUG(SYSTEM, "TERRAIN SEEK TO POSITION %d,%d", terrainIoState.gridBlock->grid_idx_x, terrainIoState.gridBlock->grid_idx_y);

        //calculate file offset
        uint32_t blocknum = (eastBlocks(terrainIoState.gridBlock) * terrainIoState.gridBlock->grid_idx_x) + terrainIoState.gridBlock->grid_idx_y;
        uint32_t fileOffset = blocknum * sizeof(terrainIoState.ioBlock);

        afatfsOperationStatus_e seekState = afatfs_fseek(terrainIoState.datFile, (int32_t)fileOffset, AFATFS_SEEK_SET);
        if(seekState != AFATFS_OPERATION_FAILURE){
            LOG_DEBUG(SYSTEM, "TERRAIN SEEK OK");
            terrainIoState.status = TERRAIN_IO_READ; // we don't wait to end of seek, after seek done, reading will be available in next task call
            return;
        }

        LOG_DEBUG(SYSTEM, "TERRAIN SEEK ERROR");

        resetStateMachine();
        return;
    }
    //////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////// READING BLOCK DATA /////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_READ){
        static uint32_t bytesRead = 0;

        uint32_t readNow = afatfs_fread(terrainIoState.datFile, (uint8_t*)&terrainIoState.ioBlock + bytesRead, sizeof(terrainIoState.ioBlock) - bytesRead);
        bytesRead += readNow;

        LOG_DEBUG(SYSTEM, "TERRAIN READING DATA %d/%d", (int)bytesRead, sizeof(terrainIoState.ioBlock));
        if(bytesRead == 0 && !(sdcard_isInserted() && sdcard_isFunctional() && afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_READY)){
            LOG_DEBUG(SYSTEM, "TERRAIN SD CARD FAILURE");
            hardFailure();
            return;
        }

        if(bytesRead < sizeof(terrainIoState.ioBlock)){

            if (afatfs_feof(terrainIoState.datFile)) {
                markGridBlockInvalid(terrainIoState.gridBlock);
                terrainIoState.status = TERRAIN_IO_CLOSE;
                bytesRead = 0;
                return;
            }

        }else{
            bytesRead = 0;
            //check if idx and idy is same for terrainIoState.ioBlock and terrainIoState.gridBlock, to be sure we loaded from file correct block
            if(terrainIoState.ioBlock.block.grid_idx_x != terrainIoState.gridBlock->grid_idx_x || terrainIoState.ioBlock.block.grid_idx_y != terrainIoState.gridBlock->grid_idx_y) {
                //wrong block loaded
                terrainIoState.status = TERRAIN_IO_CLOSE;
                return;
            }

            terrainIoState.status = TERRAIN_IO_CLOSE;
            memcpy(terrainIoState.gridBlock, &terrainIoState.ioBlock.block, sizeof(gridBlock_t));
            return;
        }
    }
    //////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////// CLOSING BLOCK DATA /////////////////////////////////////////////
    // wait to call callback terrainIoClosedFileCallback
    if(terrainIoState.status == TERRAIN_IO_CLOSE){
        terrainIoState.status = TERRAIN_IO_CLOSE_PENDING;

        afatfs_fclose(terrainIoState.datFile, terrainIoClosedFileCallback);
        return;
    }
    //////////////////////////////////////////////////////////////////////
}

/**
 * @brief Checks if the terrain IO system is in a failure state.
 *
 * @return true if the terrain IO system has failed, false otherwise.
 */
bool isTerrainIoFailure(void)
{
    return terrainIoState.status == TERRAIN_IO_FAILURE;
}

#endif


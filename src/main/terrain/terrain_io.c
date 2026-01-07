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
    terrainIoState.datFile = NULL;
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
    terrainIoState.bytesRead = 0;
    terrainIoState.readsZeroBytesCount = 0;
    terrainIoState.openFileStartTimeMs = 0;

    LOG_DEBUG(SYSTEM, "TERRAIN RELEASE RESET STATE");
}

/**
 * @brief we need to close file and reset state
 */
static void cleanUp(void)
{
    if(terrainIoState.status == TERRAIN_IO_CLOSE || terrainIoState.status == TERRAIN_IO_CLOSE_PENDING){
        return;
    }

    LOG_DEBUG(SYSTEM, "TERRAIN RELEASE RESET STATE");
    if(terrainIoState.datFile != NULL){
        LOG_DEBUG(SYSTEM, "TERRAIN CLOSE CLEAN UP FILE");
        terrainIoState.status = TERRAIN_IO_CLOSE;
    } else {
        LOG_DEBUG(SYSTEM, "TERRAIN CLOSE CLEAN RESET NOW");
        resetStateMachine();
    }
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

static void increaseFileStatusErrorCount(int8_t latDegrees, int16_t lonDegrees)
{
    terrainIoFileOpenStatus_t* fileOpenStatus = getFileOpenStatusIndex(latDegrees, lonDegrees);
    fileOpenStatus->errorOpenCount++;
}

/**
 * @brief Callback function invoked when a terrain file is opened.
 */
void terrainIoOpenedFileCallback(afatfsFilePtr_t file)
{
    if (terrainIoState.status == TERRAIN_IO_FAILURE) {
        return;
    }

    if(terrainIoState.status != TERRAIN_IO_OPEN_FILE_PENDING){
        hardFailure();
        return;
    }

    if(file == NULL){
        LOG_DEBUG(SYSTEM, "TERRAIN OPEN FILE NULL, RESET STATE");
        markGridBlockInvalid(terrainIoState.gridBlock);
        increaseFileStatusErrorCount(terrainIoState.gridBlock->latDegrees, terrainIoState.gridBlock->lonDegrees);
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
    if (terrainIoState.status == TERRAIN_IO_FAILURE) {
        return;
    }

    if(terrainIoState.status != TERRAIN_IO_CLOSE_PENDING){
        hardFailure();
        return;
    }

    LOG_DEBUG(SYSTEM, "TERRAIN CLOSE FILE OK");
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

        //check if we had too many errors opening of this file
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
        //change dir to ROOT  (null)
        if(!afatfs_chdir(NULL)){
            LOG_DEBUG(SYSTEM, "TERRAIN CHANGE DIR ERROR");
            markGridBlockInvalid(terrainIoState.gridBlock);
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
            markGridBlockInvalid(terrainIoState.gridBlock);
            resetStateMachine();
            return;
        }

        terrainIoState.status = TERRAIN_IO_OPEN_FILE_PENDING;
        terrainIoState.openFileStartTimeMs = millis();

        char filename[20];
        snprintf(filename, sizeof(filename),
                 "%c%02ld%c%03ld.DAT",
                 terrainIoState.gridBlock->latDegrees < 0 ? 'S' : 'N',
                 labs(terrainIoState.gridBlock->latDegrees),
                 terrainIoState.gridBlock->lonDegrees < 0 ? 'W' : 'E',
                 labs(terrainIoState.gridBlock->lonDegrees));

        //of most of the time is callback terrainIoOpenedFileCallback called immediately, not in next cycle
        if(!afatfs_fopen(filename, "r", terrainIoOpenedFileCallback)){
            LOG_DEBUG(SYSTEM, "TERRAIN OPEN FILE ERROR");
            markGridBlockInvalid(terrainIoState.gridBlock);
            resetStateMachine();
            return;
        }

        //don't add code here, put it to terrainIoOpenedFileCallback

        return;
    }
    ///////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    /////// OPEN FILE PENDING WATCHDOG ///////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_OPEN_FILE_PENDING){
        if(millis() - terrainIoState.openFileStartTimeMs > 3000){
            LOG_DEBUG(SYSTEM, "TERRAIN OPEN FILE TIMEOUT");
            markGridBlockInvalid(terrainIoState.gridBlock);
            increaseFileStatusErrorCount(terrainIoState.gridBlock->latDegrees, terrainIoState.gridBlock->lonDegrees);
            resetStateMachine();
            return;
        }
    }
    ///////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    /////// SEEK TO BLOCK ///////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_SEEK){
        if(terrainIoState.datFile == NULL || terrainIoState.gridBlock == NULL){
            markGridBlockInvalid(terrainIoState.gridBlock);
            cleanUp();
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
        markGridBlockInvalid(terrainIoState.gridBlock);
        cleanUp();
        return;
    }
    //////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////// READING BLOCK DATA /////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_READ){
        if(terrainIoState.datFile == NULL || terrainIoState.gridBlock == NULL){
            markGridBlockInvalid(terrainIoState.gridBlock);
            cleanUp();
            return;
        }

        uint32_t readNow = afatfs_fread(terrainIoState.datFile, (uint8_t*)&terrainIoState.ioBlock + terrainIoState.bytesRead, sizeof(terrainIoState.ioBlock) - terrainIoState.bytesRead);
        terrainIoState.bytesRead += readNow;

        LOG_DEBUG(SYSTEM, "TERRAIN READING DATA %d/%d", (int)terrainIoState.bytesRead, sizeof(terrainIoState.ioBlock));
        if(terrainIoState.bytesRead == 0 && !(sdcard_isInserted() && sdcard_isFunctional() && afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_READY)){
            LOG_DEBUG(SYSTEM, "TERRAIN SD CARD FAILURE");
            hardFailure();
            return;
        }

        if(terrainIoState.bytesRead == 0){
            terrainIoState.readsZeroBytesCount++;
        }

        // if readNow is zero, it could mean something bad happen, broken file, or any other problem with SD card
        // block is 2048, asyncfatfs reads 512. so we accept (2048 / 512) * 2 errors for a single reading.
        if(terrainIoState.readsZeroBytesCount > (sizeof(terrainIoState.ioBlock) / 512) * 2){
            markGridBlockInvalid(terrainIoState.gridBlock);

            //we have to increase error for file, for case if error for file reach threshold, and mark file as invalid
            increaseFileStatusErrorCount(terrainIoState.gridBlock->latDegrees, terrainIoState.gridBlock->lonDegrees);
            cleanUp();
            return;
        }

        if(terrainIoState.bytesRead < sizeof(terrainIoState.ioBlock)){
            //file should be divided by 2048, reading up to end of file and not have all data should never happen
            if (afatfs_feof(terrainIoState.datFile)) {
                //if it happens we have to close file and increase error count for file
                increaseFileStatusErrorCount(terrainIoState.gridBlock->latDegrees, terrainIoState.gridBlock->lonDegrees);
                markGridBlockInvalid(terrainIoState.gridBlock);
                cleanUp();
                return;
            }

        } else {
            //check if idx and idy is same for terrainIoState.ioBlock and terrainIoState.gridBlock, to be sure we loaded from file correct block
            if(terrainIoState.ioBlock.block.grid_idx_x != terrainIoState.gridBlock->grid_idx_x || terrainIoState.ioBlock.block.grid_idx_y != terrainIoState.gridBlock->grid_idx_y) {
                markGridBlockInvalid(terrainIoState.gridBlock);
                cleanUp();
                return;
            }

            memcpy(terrainIoState.gridBlock, &terrainIoState.ioBlock.block, sizeof(gridBlock_t));
            markGridBlockAsRead(terrainIoState.gridBlock);
            cleanUp();
            return;
        }
    }
    //////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////// CLOSING BLOCK DATA /////////////////////////////////////////////
    // wait to call callback terrainIoClosedFileCallback
    if(terrainIoState.status == TERRAIN_IO_CLOSE){
        terrainIoState.status = TERRAIN_IO_CLOSE_PENDING;

        if(terrainIoState.datFile == NULL) {
            resetStateMachine();
        }

        if(!afatfs_fclose(terrainIoState.datFile, terrainIoClosedFileCallback)){
            resetStateMachine();
        }
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


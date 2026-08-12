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
#include "platform.h"

#ifdef USE_TERRAIN

#include "terrain_io.h"
#include "terrain_utils.h"

#include "common/log.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/sdcard/sdcard.h"

#include "blackbox/blackbox.h"


#include "io/asyncfatfs/asyncfatfs.h"

static terrainIoState_t terrainIoState;

/**
 * @brief Acquires exclusive SD card access from blackbox for a single operation.
 *
 * Access is held only around individual SD operations (open, seek, sector reads) and released
 * in between, so blackbox can keep logging while a grid block load is in progress.
 *
 * @return true if access is held and the operation may proceed, false to retry on the next task call.
 */
static bool acquireSdAccess(void)
{
    if(terrainIoState.sdAccessHeld){
        return true;
    }

    //request access from blackbox, returns false while the request is still pending
    if(requestToSdCardAccess() == false){
        if(!terrainIoState.sdAccessRequested){
            LOG_DEBUG(TERRAIN, "TERRAIN SD LOCK REQUESTED, PENDING");
        }
        terrainIoState.sdAccessRequested = true;
        return false;
    }

    //for terrain SD operations the card must be in idle
    if(!afatfs_isIdle()){
        LOG_DEBUG(TERRAIN, "TERRAIN SD LOCK RELEASED, FS NOT IDLE");
        releaseSdCardAccess();
        terrainIoState.sdAccessRequested = false;
        return false;
    }

    LOG_DEBUG(TERRAIN, "TERRAIN SD LOCK ACQUIRED");
    terrainIoState.sdAccessRequested = false;
    terrainIoState.sdAccessHeld = true;
    return true;
}

/**
 * @brief Releases SD card access back to blackbox, also cancels a pending request.
 */
static void releaseSdAccess(void)
{
    if(terrainIoState.sdAccessHeld || terrainIoState.sdAccessRequested){
        LOG_DEBUG(TERRAIN, "TERRAIN SD LOCK RELEASED (held=%d requested=%d)", terrainIoState.sdAccessHeld, terrainIoState.sdAccessRequested);
        releaseSdCardAccess();
    }
    terrainIoState.sdAccessHeld = false;
    terrainIoState.sdAccessRequested = false;
}

/**
 * @brief Marks the terrain IO state machine as failed, disabling further operations.
 */
static void hardFailure(void)
{
    terrainIoState.datFile = NULL;
    terrainIoState.status = TERRAIN_IO_FAILURE;
    terrainIoState.gridBlock = NULL;
    LOG_DEBUG(TERRAIN, "TERRAIN HARD FAILURE, STATE MACHINE DISABLED");
    releaseSdAccess();
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

    LOG_DEBUG(TERRAIN, "TERRAIN RESET STATE -> IDLE");
    releaseSdAccess();
}

/**
 * @brief Finishes a successful block read, keeping the .DAT file open for subsequent reads.
 *
 * Unlike resetStateMachine() the file handle stays open, so the next block from the same
 * tile skips the expensive directory scan in afatfs_fopen.
 */
static void finishGridBlockRead(void)
{
    terrainIoState.status = TERRAIN_IO_IDLE;
    terrainIoState.gridBlock = NULL;
    terrainIoState.bytesRead = 0;
    terrainIoState.readsZeroBytesCount = 0;
    terrainIoState.lastReadActivityMs = millis();

    LOG_DEBUG(TERRAIN, "TERRAIN READ FINISHED -> IDLE (file kept open)");
    releaseSdAccess();
}

/**
 * @brief we need to close file and reset state
 */
static void cleanUp(void)
{
    if(terrainIoState.status == TERRAIN_IO_CLOSE || terrainIoState.status == TERRAIN_IO_CLOSE_PENDING){
        return;
    }

    if(terrainIoState.datFile != NULL){
        LOG_DEBUG(TERRAIN, "TERRAIN CLEANUP -> CLOSE");
        terrainIoState.status = TERRAIN_IO_CLOSE;
    } else {
        LOG_DEBUG(TERRAIN, "TERRAIN CLEANUP, NO FILE, RESET NOW");
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
        LOG_DEBUG(TERRAIN, "TERRAIN OPEN CALLBACK, FILE NULL, RESET STATE");
        setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
        increaseFileStatusErrorCount(terrainIoState.gridBlock->latDegrees, terrainIoState.gridBlock->lonDegrees);
        resetStateMachine();
        return;
    }

    terrainIoState.status = TERRAIN_IO_SEEK;
    terrainIoState.datFile = file;
    terrainIoState.openFileLatDegrees = terrainIoState.gridBlock->latDegrees;
    terrainIoState.openFileLonDegrees = terrainIoState.gridBlock->lonDegrees;
    terrainIoState.lastReadActivityMs = millis();

    LOG_DEBUG(TERRAIN, "TERRAIN OPEN CALLBACK, FILE OK -> SEEK");

    //the open (directory scan) is the longest SD operation, give the card back to blackbox before the seek starts
    releaseSdAccess();
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

    LOG_DEBUG(TERRAIN, "TERRAIN CLOSE CALLBACK, FILE CLOSED");
    resetStateMachine();
}

/**
 * @brief Main task function for loading terrain grid data into cache.
 */
void loadGridToCacheTask(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    if(terrainIoState.status == TERRAIN_IO_FAILURE){
        //LOG_DEBUG(TERRAIN, "TERRAIN IO FAILURE");
        return;
    }

    //SD card or file system is not prepared or it's not ready yet
    if(!sdcard_isInserted() || !sdcard_isFunctional() || afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY){
        //SD card is not ready but IO is in IDLE, probably something heppend with SD card during reading
        //a kept-open file handle is also invalid once the card went away
        if(terrainIoState.status != TERRAIN_IO_IDLE || terrainIoState.datFile != NULL){
            hardFailure();
            return;
        }else{
            return;
        }
    }

    ///////////////////////////////////////////////////////////////////
    /////// IDLE          /////////////////////////////////////////////
    // check if in the cache is some block which is waiting to read from disk
    if(terrainIoState.status == TERRAIN_IO_IDLE){
        gridBlock_t* gridBlock = getGridBlockToRead();

        //nothing to read
        if(gridBlock == NULL){
            //cancel a pending access request so blackbox is not stalled waiting for us
            releaseSdAccess();

            //close the kept-open file when it was not used for a while, frees the file handle
            if(terrainIoState.datFile != NULL && millis() - terrainIoState.lastReadActivityMs > TERRAIN_IO_KEEP_OPEN_IDLE_MS){
                LOG_DEBUG(TERRAIN, "TERRAIN IDLE TIMEOUT -> CLOSE");
                terrainIoState.status = TERRAIN_IO_CLOSE;
            }
            return;
        }

        //check if we had too many errors opening of this file
        terrainIoFileOpenStatus_t* fileOpenStatus = getFileOpenStatusIndex(gridBlock->latDegrees, gridBlock->lonDegrees);
        if(fileOpenStatus->errorOpenCount >= 3){
            LOG_DEBUG(TERRAIN, "TERRAIN TOO MANY OPEN ERRORS, BLOCK INVALID");
            setGridStatus(gridBlock, GRID_CACHE_INVALID);
            return;
        }

        if(terrainIoState.datFile != NULL){
            //the file for this tile is still open from the previous block, skip the expensive open and go straight to seek
            if(terrainIoState.openFileLatDegrees == gridBlock->latDegrees && terrainIoState.openFileLonDegrees == gridBlock->lonDegrees){
                ///////////////////////////////////////////////////////////////////
                //request access from blackbox, SD card must be in idle
                if(!acquireSdAccess()){
                    return;
                }
                ////////////////////////////////////////////////////////////////

                LOG_DEBUG(TERRAIN, "TERRAIN IDLE, FILE ALREADY OPEN -> SEEK");
                terrainIoState.gridBlock = gridBlock;
                terrainIoState.bytesRead = 0;
                terrainIoState.readsZeroBytesCount = 0;
                terrainIoState.status = TERRAIN_IO_SEEK;
                return;
            }

            //block is from a different tile, close the old file first; the block stays pending and is picked up again next call
            LOG_DEBUG(TERRAIN, "TERRAIN IDLE, DIFFERENT TILE -> CLOSE");
            terrainIoState.status = TERRAIN_IO_CLOSE;
            return;
        }

        ///////////////////////////////////////////////////////////////////
        //request access from blackbox, SD card must be in idle
        //access is held for the whole open (directory scan cannot be split), released again in terrainIoOpenedFileCallback
        if(!acquireSdAccess()){
            return;
        }
        ////////////////////////////////////////////////////////////////

        //set grid block for process. open file -> seek -> read
        LOG_DEBUG(TERRAIN, "TERRAIN IDLE -> CHANGE_DIR");
        terrainIoState.gridBlock = gridBlock;
        terrainIoState.status = TERRAIN_IO_CHANGE_DIR;

        return;
    }

    ///////////////////////////////////////////////////////////////////
    /////// CHANGE DIR   /////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_CHANGE_DIR){
        //already in the root directory, no need to change the directory
        if(afatfs_isCurrentDirRoot()){
            LOG_DEBUG(TERRAIN, "TERRAIN CHANGE_DIR, ALREADY ROOT -> OPEN_FILE");
            terrainIoState.status = TERRAIN_IO_OPEN_FILE;
            return;
        }

        LOG_DEBUG(TERRAIN, "TERRAIN CHANGE_DIR, CHANGING TO ROOT");
        //change dir to ROOT (null)
        if(!afatfs_chdir(NULL)){
            LOG_DEBUG(TERRAIN, "TERRAIN CHANGE_DIR ERROR");
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
            resetStateMachine();
            return;
        }

        LOG_DEBUG(TERRAIN, "TERRAIN CHANGE_DIR OK -> OPEN_FILE");
        terrainIoState.status = TERRAIN_IO_OPEN_FILE;
        return;
    }
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /////// OPEN FILE     /////////////////////////////////////////////
    // wait to call callback terrainIoOpenedFileCallback
    if(terrainIoState.status == TERRAIN_IO_OPEN_FILE){
        if(terrainIoState.gridBlock == NULL){
            LOG_DEBUG(TERRAIN, "TERRAIN OPEN_FILE, GRID BLOCK NULL, RESET STATE");
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
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

        LOG_DEBUG(TERRAIN, "TERRAIN OPEN_FILE %s -> OPEN_FILE_PENDING", filename);

        //of most of the time is callback terrainIoOpenedFileCallback called immediately, not in next cycle
        if(!afatfs_fopen(filename, "r", terrainIoOpenedFileCallback)){
            LOG_DEBUG(TERRAIN, "TERRAIN OPEN_FILE ERROR");
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
            resetStateMachine();
            return;
        }

        //don't add code here, put it to terrainIoOpenedFileCallback
        return;
    }
    ///////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////
    /////// OPEN FILE PENDING WATCHDOG ///////////////////////////////////
    // asynfatfs can be in infinite state "opening" if the card failures during opening. So after a while reset state machine
    if(terrainIoState.status == TERRAIN_IO_OPEN_FILE_PENDING){
        if(millis() - terrainIoState.openFileStartTimeMs > 3000){
            LOG_DEBUG(TERRAIN, "TERRAIN OPEN_FILE_PENDING TIMEOUT, RESET STATE");
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
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
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
            cleanUp();
            return;
        }

        ///////////////////////////////////////////////////////////////////
        //access was released after the open, request it again for the seek + read
        if(!acquireSdAccess()){
            return;
        }
        ////////////////////////////////////////////////////////////////

        LOG_DEBUG(TERRAIN, "TERRAIN SEEK TO POSITION %d,%d", (int)terrainIoState.gridBlock->grid_idx_x, (int)terrainIoState.gridBlock->grid_idx_y);

        //calculate file offset
        uint32_t blocknum = (eastBlocks(terrainIoState.gridBlock) * terrainIoState.gridBlock->grid_idx_x) + terrainIoState.gridBlock->grid_idx_y;
        uint64_t fileOffset64 = (uint64_t)blocknum * TERRAIN_GRID_BLOCK_SIZE_SD_CARD;

        if(fileOffset64 > UINT32_MAX) {
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
            cleanUp();
            return;
        }

        afatfsOperationStatus_e seekState = afatfs_fseek(terrainIoState.datFile, (int32_t)fileOffset64, AFATFS_SEEK_SET);
        if(seekState != AFATFS_OPERATION_FAILURE){
            LOG_DEBUG(TERRAIN, "TERRAIN SEEK OK -> READ");
            //the read overwrites the destination block, stash the expected idx for the completion check
            terrainIoState.expectedIdxX = terrainIoState.gridBlock->grid_idx_x;
            terrainIoState.expectedIdxY = terrainIoState.gridBlock->grid_idx_y;
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_READING);
            terrainIoState.status = TERRAIN_IO_READ; // we don't wait to end of seek, after seek done, reading will be available in next task call
            return;
        }

        LOG_DEBUG(TERRAIN, "TERRAIN SEEK ERROR");
        setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
        cleanUp();
        return;
    }
    //////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////// READING BLOCK DATA /////////////////////////////////////////////
    if(terrainIoState.status == TERRAIN_IO_READ){
        if(terrainIoState.datFile == NULL || terrainIoState.gridBlock == NULL){
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
            cleanUp();
            return;
        }

        ///////////////////////////////////////////////////////////////////
        //access is released after every completed sector read, request it again for the next one
        if(!acquireSdAccess()){
            return;
        }
        ////////////////////////////////////////////////////////////////

        uint32_t readNow = afatfs_fread(terrainIoState.datFile, (uint8_t*)terrainIoState.gridBlock + terrainIoState.bytesRead, sizeof(gridBlock_t) - terrainIoState.bytesRead);
        terrainIoState.bytesRead += readNow;

        LOG_DEBUG(TERRAIN, "TERRAIN READING DATA %d/%d", (int)terrainIoState.bytesRead, (int)sizeof(gridBlock_t));
        if(terrainIoState.bytesRead == 0 && !(sdcard_isInserted() && sdcard_isFunctional() && afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_READY)){
            LOG_DEBUG(TERRAIN, "TERRAIN READ, SD CARD FAILURE");
            hardFailure();
            return;
        }

        if(terrainIoState.bytesRead == 0){
            terrainIoState.readsZeroBytesCount++;
        }

        // if readNow is zero, it could mean something bad happen, broken file, or any other problem with SD card
        // a block needs ceil(sizeof(gridBlock_t) / 512) sector reads, asyncfatfs reads 512. so we accept 2 errors per sector for a single reading.
        if(terrainIoState.readsZeroBytesCount > ((sizeof(gridBlock_t) + 511) / 512) * 2){
            LOG_DEBUG(TERRAIN, "TERRAIN READ, TOO MANY ZERO READS");
            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);

            //we have to increase error for file, for case if error for file reach threshold, and mark file as invalid
            //use the open-file tile, the block's own degrees may already be overwritten by the partial read
            increaseFileStatusErrorCount(terrainIoState.openFileLatDegrees, terrainIoState.openFileLonDegrees);
            cleanUp();
            return;
        }

        if(terrainIoState.bytesRead < sizeof(gridBlock_t)){
            //file should be divided by 2048, reading up to end of file and not have all data should never happen
            if (afatfs_feof(terrainIoState.datFile)) {
                LOG_DEBUG(TERRAIN, "TERRAIN READ, UNEXPECTED EOF");
                //if it happens we have to close file and increase error count for file
                //use the open-file tile, the block's own degrees may already be overwritten by the partial read
                increaseFileStatusErrorCount(terrainIoState.openFileLatDegrees, terrainIoState.openFileLonDegrees);
                setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
                cleanUp();
                return;
            }

            //a sector was read, give the card back to blackbox so it can flush before the next sector read
            if(readNow > 0){
                releaseSdAccess();
            }

        } else {
            if(
                    terrainIoState.gridBlock->grid_idx_x != terrainIoState.expectedIdxX
                    || terrainIoState.gridBlock->grid_idx_y != terrainIoState.expectedIdxY
                    || terrainIoState.gridBlock->crc != getBlockCrc(terrainIoState.gridBlock)

            ) {
                LOG_DEBUG(TERRAIN, "TERRAIN READ, BLOCK IDX MISMATCH");
                setGridStatus(terrainIoState.gridBlock, GRID_CACHE_INVALID);
                cleanUp();
                return;
            }

            setGridStatus(terrainIoState.gridBlock, GRID_CACHE_VALID);

            //keep the file open, next block from the same tile skips the expensive open
            finishGridBlockRead();
            return;
        }
    }
    //////////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////////////
    /////// CLOSING BLOCK DATA /////////////////////////////////////////////
    // wait to call callback terrainIoClosedFileCallback
    if(terrainIoState.status == TERRAIN_IO_CLOSE){
        if(terrainIoState.datFile == NULL) {
            resetStateMachine();
            return;
        }
        LOG_DEBUG(TERRAIN, "TERRAIN CLOSE -> CLOSE_PENDING");
        terrainIoState.status = TERRAIN_IO_CLOSE_PENDING;
        if(!afatfs_fclose(terrainIoState.datFile, terrainIoClosedFileCallback)){
            LOG_DEBUG(TERRAIN, "TERRAIN CLOSE ERROR, RESET STATE");
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


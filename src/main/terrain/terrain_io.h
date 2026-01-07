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

#include "terrain.h"
#include "io/asyncfatfs/asyncfatfs.h"

#define TERRAIN_IO_MAX_FILE_OPEN_STATUS 4 //supposed max 4 files opened for a single flight

/**
 * @brief Enumeration of terrain IO status states.
 */
typedef enum {
    TERRAIN_IO_IDLE,
    TERRAIN_IO_CHANGE_DIR,
    TERRAIN_IO_OPEN_FILE,
    TERRAIN_IO_OPEN_FILE_PENDING, //wait state in async, waiting to call callback
    TERRAIN_IO_SEEK,
    TERRAIN_IO_READ,
    TERRAIN_IO_CLOSE,
    TERRAIN_IO_CLOSE_PENDING, //wait state in async, waiting to call callback
    TERRAIN_IO_FAILURE, //something bad happened, disable SD card
} terrainIoStatus_e;

/**
 * @brief Structure to track the open status of terrain data files.
 */
typedef struct {
    int8_t latDegrees;
    int16_t lonDegrees;
    uint8_t errorOpenCount;
    timeMs_t lastAccessTimeMs;
} terrainIoFileOpenStatus_t;

/**
 * @brief Structure representing the state of the terrain IO system.
 */
typedef struct {
    terrainIoStatus_e status;
    gridBlock_t *gridBlock;
    gridIoBlock_t ioBlock; // 2048 bytes because block in disk is exactly 2048 bytes
    afatfsFilePtr_t datFile;
    terrainIoFileOpenStatus_t fileOpenStatus[TERRAIN_IO_MAX_FILE_OPEN_STATUS];
    uint32_t bytesRead;
    uint32_t readsZeroBytesCount;
    timeMs_t openFileStartTimeMs;
} terrainIoState_t;



void loadGridToCacheTask(void);
void terrainIoOpenedDirCallback(afatfsFilePtr_t directory);
void terrainIoOpenedFileCallback(afatfsFilePtr_t file);
void terrainIoClosedFileCallback(void);
void terrainIoClosedDirCallback(void);
bool isTerrainIoFailure(void);

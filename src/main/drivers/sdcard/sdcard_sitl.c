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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"

#if defined(USE_SDCARD) && defined(USE_SDCARD_SITL)

#include "common/utils.h"

#include "drivers/sdcard/sdcard.h"
#include "drivers/sdcard/sdcard_impl.h"
#include "drivers/sdcard/sdcard_standard.h"
#include "drivers/sdcard/sdcard_sitl.h"

#define SDCARD_SITL_PATH_MAX 260

static FILE *imageFile = NULL;
static char imagePath[SDCARD_SITL_PATH_MAX];

bool sdcardSitlSetPath(const char *path)
{
    if (path == NULL || strlen(path) == 0 || strlen(path) >= SDCARD_SITL_PATH_MAX) {
        return false;
    }
    strcpy(imagePath, path);
    return true;
}

static void sdcardSitl_init(void)
{
    sdcard.state = SDCARD_STATE_NOT_PRESENT;

    if (imagePath[0] == '\0') {
        fprintf(stderr, "[SDCARD] No image file specified (--sdcard=<image>), no card simulated\n");
        return;
    }

    imageFile = fopen(imagePath, "r+b");
    if (imageFile == NULL) {
        fprintf(stderr, "[SDCARD] Cannot open image file %s, no card simulated\n", imagePath);
        return;
    }

    long imageSize = -1;
    if (fseek(imageFile, 0, SEEK_END) == 0) {
        imageSize = ftell(imageFile);
    }

    if (imageSize < SDCARD_BLOCK_SIZE) {
        fprintf(stderr, "[SDCARD] Image file %s is unusable, no card simulated\n", imagePath);
        fclose(imageFile);
        imageFile = NULL;
        return;
    }

    memset(&sdcard.metadata, 0, sizeof(sdcard.metadata));
    sdcard.metadata.numBlocks = imageSize / SDCARD_BLOCK_SIZE;
    memcpy(sdcard.metadata.productName, "SITL", 4);

    sdcard.state = SDCARD_STATE_READY;
    fprintf(stderr, "[SDCARD] Simulating card from %s (%u blocks)\n", imagePath, (unsigned)sdcard.metadata.numBlocks);
}

static bool sdcardSitl_readBlock(uint32_t blockIndex, uint8_t *buffer, sdcard_operationCompleteCallback_c callback, uint32_t callbackData)
{
    if (sdcard.state != SDCARD_STATE_READY) {
        return false;
    }

    sdcard.pendingOperation.buffer = buffer;
    sdcard.pendingOperation.blockIndex = blockIndex;
    sdcard.pendingOperation.callback = callback;
    sdcard.pendingOperation.callbackData = callbackData;

    sdcard.state = SDCARD_STATE_READING;

    return true;
}

static sdcardOperationStatus_e sdcardSitl_beginWriteBlocks(uint32_t blockIndex, uint32_t blockCount)
{
    UNUSED(blockIndex);
    UNUSED(blockCount);

    // Writes are addressed per block, no setup is needed for the backing file
    if (sdcard.state != SDCARD_STATE_READY) {
        return SDCARD_OPERATION_BUSY;
    }

    return SDCARD_OPERATION_SUCCESS;
}

static sdcardOperationStatus_e sdcardSitl_writeBlock(uint32_t blockIndex, uint8_t *buffer, sdcard_operationCompleteCallback_c callback, uint32_t callbackData)
{
    if (sdcard.state != SDCARD_STATE_READY) {
        return SDCARD_OPERATION_BUSY;
    }

    sdcard.pendingOperation.buffer = buffer;
    sdcard.pendingOperation.blockIndex = blockIndex;
    sdcard.pendingOperation.callback = callback;
    sdcard.pendingOperation.callbackData = callbackData;

    sdcard.state = SDCARD_STATE_SENDING_WRITE;

    return SDCARD_OPERATION_IN_PROGRESS;
}

static bool sdcardSitl_transferBlock(bool isWrite)
{
    if (imageFile == NULL || sdcard.pendingOperation.blockIndex >= sdcard.metadata.numBlocks) {
        return false;
    }

    if (fseek(imageFile, (long)sdcard.pendingOperation.blockIndex * SDCARD_BLOCK_SIZE, SEEK_SET) != 0) {
        return false;
    }

    if (isWrite) {
        if (fwrite(sdcard.pendingOperation.buffer, 1, SDCARD_BLOCK_SIZE, imageFile) != SDCARD_BLOCK_SIZE) {
            return false;
        }
        fflush(imageFile);
        return true;
    }

    return fread(sdcard.pendingOperation.buffer, 1, SDCARD_BLOCK_SIZE, imageFile) == SDCARD_BLOCK_SIZE;
}

static bool sdcardSitl_poll(void)
{
    switch (sdcard.state) {
        case SDCARD_STATE_READING: {
            bool success = sdcardSitl_transferBlock(false);

            sdcard.state = SDCARD_STATE_READY;

            if (sdcard.pendingOperation.callback) {
                sdcard.pendingOperation.callback(
                    SDCARD_BLOCK_OPERATION_READ,
                    sdcard.pendingOperation.blockIndex,
                    success ? sdcard.pendingOperation.buffer : NULL,
                    sdcard.pendingOperation.callbackData
                );
            }
            break;
        }
        case SDCARD_STATE_SENDING_WRITE: {
            bool success = sdcardSitl_transferBlock(true);

            sdcard.state = SDCARD_STATE_READY;

            if (sdcard.pendingOperation.callback) {
                sdcard.pendingOperation.callback(
                    SDCARD_BLOCK_OPERATION_WRITE,
                    sdcard.pendingOperation.blockIndex,
                    success ? sdcard.pendingOperation.buffer : NULL,
                    sdcard.pendingOperation.callbackData
                );
            }
            break;
        }
        default:
            break;
    }

    return sdcard.state == SDCARD_STATE_READY;
}

static bool sdcardSitl_isFunctional(void)
{
    return sdcard.state != SDCARD_STATE_NOT_PRESENT;
}

static bool sdcardSitl_isInitialized(void)
{
    return sdcard.state >= SDCARD_STATE_READY;
}

static const sdcardMetadata_t* sdcardSitl_getMetadata(void)
{
    return &sdcard.metadata;
}

sdcardVTable_t sdcardSitlVTable = {
    .init = &sdcardSitl_init,
    .readBlock = &sdcardSitl_readBlock,
    .beginWriteBlocks = &sdcardSitl_beginWriteBlocks,
    .writeBlock = &sdcardSitl_writeBlock,
    .poll = &sdcardSitl_poll,
    .isFunctional = &sdcardSitl_isFunctional,
    .isInitialized = &sdcardSitl_isInitialized,
    .getMetadata = &sdcardSitl_getMetadata,
};

#endif

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

#include "platform.h"

#if defined(USE_SDCARD)

#include "build/debug.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/io.h"
#include "drivers/bus.h"

#include "drivers/sdcard/sdcard.h"
#include "drivers/sdcard/sdcard_impl.h"
#include "drivers/sdcard/sdcard_standard.h"

#include "scheduler/protothreads.h"

#ifndef SDCARD_DETECT_PIN
#define SDCARD_DETECT_PIN           NONE
#endif

sdcard_t sdcard;

void sdcardInsertionDetectDeinit(void)
{
    sdcard.cardDetectPin = IOGetByTag(IO_TAG(SDCARD_DETECT_PIN));

    if (sdcard.cardDetectPin) {
        IOInit(sdcard.cardDetectPin, OWNER_FREE, RESOURCE_NONE, 0);
        IOConfigGPIO(sdcard.cardDetectPin, IOCFG_IN_FLOATING);
    }
}

void sdcardInsertionDetectInit(void)
{
    sdcard.cardDetectPin = IOGetByTag(IO_TAG(SDCARD_DETECT_PIN));

    if (sdcard.cardDetectPin) {
        IOInit(sdcard.cardDetectPin, OWNER_SDCARD, RESOURCE_INPUT, 0);
        IOConfigGPIO(sdcard.cardDetectPin, IOCFG_IPU);
    }
}

/**
 * Returns true if the card is physically inserted into the slot
 */
bool sdcard_isInserted(void)
{
    if (sdcard.cardDetectPin) {
        bool result = (IORead(sdcard.cardDetectPin) != 0);
#if defined(SDCARD_DETECT_INVERTED)
        return !result;
#else
        return result;
#endif
    }
    else {
        return true;
    }
}

/**
 * Dispatch
 */
sdcardVTable_t *sdcardVTable = NULL;

void sdcard_init(void)
{
#if defined(USE_SDCARD_SPI)
    sdcardVTable = &sdcardSpiVTable;
#elif defined(USE_SDCARD_SDIO)
    sdcardVTable = &sdcardSdioVTable;
#endif

    if (sdcardVTable) {
        sdcardVTable->init();
    }
}

bool sdcard_readBlock(uint32_t blockIndex, uint8_t *buffer, sdcard_operationCompleteCallback_c callback, uint32_t callbackData)
{
    if (sdcardVTable) {
        return sdcardVTable->readBlock(blockIndex, buffer, callback, callbackData);
    } else {
        return false;
    }
}

sdcardOperationStatus_e sdcard_beginWriteBlocks(uint32_t blockIndex, uint32_t blockCount)
{
    if (sdcardVTable) {
        return sdcardVTable->beginWriteBlocks(blockIndex, blockCount);
    } else {
        return false;
    }
}

sdcardOperationStatus_e sdcard_writeBlock(uint32_t blockIndex, uint8_t *buffer, sdcard_operationCompleteCallback_c callback, uint32_t callbackData)
{
    if (sdcardVTable) {
        return sdcardVTable->writeBlock(blockIndex, buffer, callback, callbackData);
    } else {
        return false;
    }
}

bool sdcard_poll(void)
{
    if (sdcardVTable) {
        return sdcardVTable->poll();
    } else {
        return false;
    }
}

bool sdcard_isFunctional(void)
{
    if (sdcardVTable) {
        return sdcardVTable->isFunctional();
    } else {
        return false;
    }
}

bool sdcard_isInitialized(void)
{
    if (sdcardVTable) {
        return sdcardVTable->isInitialized();
    } else {
        return false;
    }
}

const sdcardMetadata_t* sdcard_getMetadata(void)
{
    if (sdcardVTable) {
        return sdcardVTable->getMetadata();
    }
    else {
        return NULL;
    }
}

#endif

/*
 * This file is part of iNav.
 *
 * iNav is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

/*#include "common/log.h"*/
#include "common/maths.h"
/*#include "common/printf.h"*/

#include "drivers/bus.h"
#include "drivers/flash.h"
#include "drivers/persistent.h"
#include "drivers/io.h"
#include "drivers/light_led.h"
#include "drivers/sdcard/sdcard.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "fc/firmware_update_common.h"

#include "io/asyncfatfs/asyncfatfs.h"


#if !(defined(USE_FLASHFS) || defined(USE_SDCARD))
#error No storage backend available
#endif


typedef struct {
    uint16_t size;
    uint8_t count;
} flashSectorDef_t;

#if defined(STM32F405xx)
#define SECTOR_COUNT 12
flashSectorDef_t flashSectors[] = { { 16, 4 }, { 64, 1 }, { 128, 7 }, { 0, 0 } };

#elif defined(STM32F722xx)
#define SECTOR_COUNT 8
flashSectorDef_t flashSectors[] = { { 16, 4 }, { 64, 1 }, { 128, 3 }, { 0, 0 } };

#elif defined(STM32F745xG) || defined(STM32F765xG)
#define SECTOR_COUNT 8
flashSectorDef_t flashSectors[] = { { 32, 4 }, { 128, 1 }, { 256, 3 }, { 0, 0 } };

#elif defined(STM32F765xI)
#define SECTOR_COUNT 8
flashSectorDef_t flashSectors[] = { { 32, 4 }, { 128, 1 }, { 256, 7 }, { 0, 0 } };

#else
#error Unsupported MCU
#endif

#if defined(STM32F4)
    #define flashLock() FLASH_Lock()
    #define flashUnlock() FLASH_Unlock()
#elif defined(STM32F7)
    #define flashLock() HAL_FLASH_Lock()
    #define flashUnlock() HAL_FLASH_Unlock()
#endif

static bool dataBackEndInitialized = false;

#ifdef USE_SDCARD
static afatfsFilePtr_t flashDataFile = NULL;

static void flashDataFileOpenCallback(afatfsFilePtr_t file)
{
    flashDataFile = file;
}
#endif

static void init(void)
{
#ifdef USE_HAL_DRIVER
    HAL_Init();
#endif

    /*printfSupportInit();*/

    systemInit();

    __enable_irq();

    // initialize IO (needed for all IO operations)
    IOInitGlobal();

    ledInit(false);

    LED0_OFF;
    LED1_OFF;

    for(int x = 0; x < 10; ++x) {
        LED0_TOGGLE;
        LED1_TOGGLE;
        delay(200);
    }

}

static bool dataBackendInit(void)
{
    if (dataBackEndInitialized) return true;

    busInit();

#if defined(USE_SDCARD)
    sdcardInsertionDetectInit();
    sdcard_init();
    afatfs_init();

    afatfsError_e sdError = afatfs_getLastError();
    while ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY) && ((sdError = afatfs_getLastError()) == AFATFS_ERROR_NONE)) {
        afatfs_poll();
    }

    if (sdError != AFATFS_ERROR_NONE) {
        return false;
    }

#elif defined(USE_FLASHFS) && defined(USE_FLASH_M25P16)
    if (!flashInit()) {
        return false;
    }

#endif

    dataBackEndInitialized = true;
    return true;
}

typedef void resetHandler_t(void);

typedef struct isrVector_s {
    uint32_t    stackEnd;
    resetHandler_t *resetHandler;
} isrVector_t;

static void do_jump(uint32_t address)
{
#ifdef STM32F7
    __DSB();
    __DMB();
    __ISB();
#endif

    volatile isrVector_t *bootloaderVector = (isrVector_t *)address;
    __set_MSP(bootloaderVector->stackEnd);
    bootloaderVector->resetHandler();
}

void bootloader_jump_to_app(void)
{
    FLASH->ACR &= (~FLASH_ACR_PRFTEN);

#if defined(STM32F4)
    RCC_APB1PeriphResetCmd(~0, DISABLE);
    RCC_APB2PeriphResetCmd(~0, DISABLE);
#elif defined(STM32F7)
    RCC->APB1ENR = 0;
    RCC->APB1LPENR = 0;
    RCC->APB2ENR = 0;
    RCC->APB2LPENR = 0;
#endif

    __disable_irq();

    do_jump(FIRMWARE_START_ADDRESS);
}

// find sector specified address is in (assume than the config section doesn't span more than 1 sector)
// returns -1 if not found
int8_t mcuFlashAddressSectorIndex(uint32_t address)
{
    uint32_t sectorStartAddress = FLASH_START_ADDRESS;
    uint8_t sector = 0;
    flashSectorDef_t *sectorDef = flashSectors;

    do {
        for (unsigned j = 0; j < sectorDef->count; ++j) {
            uint32_t sectorEndAddress = sectorStartAddress + sectorDef->size * 1024;
            /*if ((CONFIG_START_ADDRESS >= sectorStartAddress) && (CONFIG_START_ADDRESS < sectorEndAddress) && (CONFIG_END_ADDRESS <= sectorEndAddress)) {*/
            if ((address >= sectorStartAddress) && (address < sectorEndAddress)) {
                return sector;
            }
            sectorStartAddress = sectorEndAddress;
            sector += 1;
        }
        sectorDef += 1;
    } while (sectorDef->count);

    return -1;
}

uint32_t mcuFlashSectorID(uint8_t sectorIndex)
{
#if defined(STM32F4)
    if (sectorIndex < 12) {
        return sectorIndex * 8;
    } else {
        return 0x80 + (sectorIndex - 12) * 8;
    }
#elif defined(STM32F7)
    return sectorIndex;
#endif
}

bool mcuFlashSectorErase(uint8_t sectorIndex)
{
#if defined(STM32F4)
    return (FLASH_EraseSector(mcuFlashSectorID(sectorIndex), VoltageRange_3) == FLASH_COMPLETE);
#elif defined(STM32F7)
    FLASH_EraseInitTypeDef EraseInitStruct = {
        .TypeErase     = FLASH_TYPEERASE_SECTORS,
        .VoltageRange  = FLASH_VOLTAGE_RANGE_3, // 2.7-3.6V
        .NbSectors     = 1
    };
    EraseInitStruct.Sector = mcuFlashSectorID(sectorIndex);
    uint32_t SECTORError;
    const HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
    return (status == HAL_OK);
#else
#error Unsupported MCU
#endif
}

bool mcuFirmwareFlashErase(bool includeConfig)
{
    int8_t firmwareSectorIndex = mcuFlashAddressSectorIndex(FIRMWARE_START_ADDRESS);
    int8_t configSectorIndex = mcuFlashAddressSectorIndex(CONFIG_START_ADDRESS);

    if ((firmwareSectorIndex == -1) || (configSectorIndex == -1)) {
        return false;
    }

    LED0_OFF;
    LED1_ON;
    for (unsigned i = firmwareSectorIndex; i < SECTOR_COUNT; ++i) {
        if (includeConfig || (!includeConfig && (i != (uint8_t)configSectorIndex))) {
            if (!mcuFlashSectorErase(i)) {
                LED1_OFF;
                return false;
            }
            LED0_TOGGLE;
        }
    }

    LED1_OFF;
    return true;
}

bool mcuFlashWriteWord(uint32_t address, uint32_t data)
{
#if defined(STM32F4)
    const FLASH_Status status = FLASH_ProgramWord(address, data);
    return (status == FLASH_COMPLETE);
#elif defined(STM32F7)
    const HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, (uint64_t)data);
    return (status == HAL_OK);
#else
#error Unsupported MCU
#endif
}

typedef enum {
    FLASH_OPERATION_UPDATE,
    FLASH_OPERATION_ROLLBACK
} flashOperation_e;

#if defined(USE_SDCARD)
bool afatfs_fseekWorkAround(afatfsFilePtr_t file, uint32_t forward)
{
    uint8_t buffer[256];
    while (forward > 0) {
        uint32_t bytesRead = afatfs_freadSync(file, buffer, MIN(forward, (uint16_t)256));
        if (bytesRead < 256) {
            return false;
        }
        forward -= bytesRead;
    }
    return true;
}
#endif

bool flash(flashOperation_e flashOperation)
{
    if (!dataBackendInit()) {
        return false;
    }

    uint32_t buffer;
    uint32_t flashDstAddress = FIRMWARE_START_ADDRESS + sizeof(buffer); // Write the first bytes last so that we can check that the firmware has been written fully

#if defined(USE_SDCARD)
    const char * const flashDataFileName = (flashOperation == FLASH_OPERATION_UPDATE ? FIRMWARE_UPDATE_FIRMWARE_FILENAME : FIRMWARE_UPDATE_BACKUP_FILENAME);
    if ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY)
            || !afatfs_fopen(flashDataFileName, "r", flashDataFileOpenCallback)
            || (afatfs_fileSize(flashDataFile) > AVAILABLE_FIRMWARE_SPACE)) {
        return false;
    }

#elif defined(USE_FLASHFS)
    flashPartitionType_e srcFlashPartitionType = (flashOperation == FLASH_OPERATION_UPDATE ? FLASH_PARTITION_TYPE_UPDATE_FIRMWARE : FLASH_PARTITION_TYPE_FULL_BACKUP);
    flashPartition_t *flashDataPartition = flashPartitionFindByType(srcFlashPartitionType);
    const flashGeometry_t *flashGeometry = flashGetGeometry();
    uint32_t flashDataPartitionSize = (flashDataPartition->endSector - flashDataPartition->startSector + 1) * (flashGeometry->sectorSize * flashGeometry->pageSize);
    firmwareUpdateMetadata_t updateMetadata;

    if (!flashDataPartition || !firmwareUpdateMetadataRead(&updateMetadata)
            || (updateMetadata.firmwareSize > flashDataPartitionSize)
            || (updateMetadata.firmwareSize > AVAILABLE_FIRMWARE_SPACE)) {
        return false;
    }

#endif

    flashUnlock();
    bool flashSucceeded = false;

    if (!mcuFirmwareFlashErase(flashOperation != FLASH_OPERATION_UPDATE)) goto flashFailed;

    LED0_OFF;
    LED1_OFF;

    uint32_t counter = 0;

#if defined(USE_SDCARD)

    if (afatfs_fseekSync(flashDataFile, sizeof(buffer), AFATFS_SEEK_SET) == AFATFS_OPERATION_FAILURE) {
        goto flashFailed;
    }

    while (!afatfs_feof(flashDataFile)) {

        if ((flashOperation == FLASH_OPERATION_UPDATE) && (flashDstAddress == CONFIG_START_ADDRESS)) {
            // skip config region
            const uint32_t configSize = CONFIG_END_ADDRESS - CONFIG_START_ADDRESS;
            /*if (afatfs_fseekSync(flashDataFile, configSize, AFATFS_SEEK_CUR) == AFATFS_OPERATION_FAILURE) {*/
            if (!afatfs_fseekWorkAround(flashDataFile, configSize)) { // workaround fseek bug, should be ^^^^^^^^^
                goto flashFailed;
            }
            flashDstAddress += configSize;
        }

        afatfs_freadSync(flashDataFile, (uint8_t *)&buffer, sizeof(buffer));

        if (!mcuFlashWriteWord(flashDstAddress, buffer)) {
            goto flashFailed;
        }

        flashDstAddress += sizeof(buffer);

        if (++counter % (10*1024/4) == 0) {
            LED0_TOGGLE;
            LED1_TOGGLE;
        }

    }

    if ((afatfs_fseekSync(flashDataFile, 0, AFATFS_SEEK_SET) == AFATFS_OPERATION_FAILURE)
            || (afatfs_freadSync(flashDataFile, (uint8_t *)&buffer, sizeof(buffer)) != sizeof(buffer))) {
        goto flashFailed;
    }

#elif defined(USE_FLASHFS)
    const uint32_t flashSrcStartAddress = flashDataPartition->startSector * flashGeometry->sectorSize;
    uint32_t flashSrcAddress = flashSrcStartAddress + sizeof(buffer);
    const uint32_t flashDstEndAddress = (flashOperation == FLASH_OPERATION_UPDATE ? FIRMWARE_START_ADDRESS + updateMetadata.firmwareSize : FLASH_END);

    while (flashDstAddress < flashDstEndAddress) {

        if ((flashOperation == FLASH_OPERATION_UPDATE) && (flashDstAddress == CONFIG_START_ADDRESS)) {
            // skip config region
            const uint32_t configSize = CONFIG_END_ADDRESS - CONFIG_START_ADDRESS;
            flashSrcAddress += configSize;
            flashDstAddress += configSize;

            if (flashDstAddress >= flashDstEndAddress) {
                goto flashFailed;
            }
        }

        flashReadBytes(flashSrcAddress, (uint8_t*)&buffer, sizeof(buffer));
        if (!mcuFlashWriteWord(flashDstAddress, buffer)) {
            goto flashFailed;
        }

        flashSrcAddress += sizeof(buffer);
        flashDstAddress += sizeof(buffer);

        if (++counter % (10*1024/4) == 0) {
            LED0_TOGGLE;
            LED1_TOGGLE;
        }

    }

    flashReadBytes(flashSrcStartAddress, (uint8_t*)&buffer, sizeof(buffer));

#endif

    if (!mcuFlashWriteWord(FIRMWARE_START_ADDRESS, buffer)) {
        goto flashFailed;
    }

    flashSucceeded = true;

flashFailed:

    flashLock();

    LED0_OFF;
    LED1_OFF;

    return flashSucceeded;
}

#if defined(USE_FLASHFS)
bool dataflashChipEraseUpdatePartition(void)
{
    flashPartition_t *flashDataPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_UPDATE_FIRMWARE);
    if (!flashDataPartition) return false;

    const flashGeometry_t *flashGeometry = flashGetGeometry();

    LED0_OFF;

    for (unsigned i = flashDataPartition->startSector; i <= flashDataPartition->endSector; i++) {
        uint32_t flashAddress = flashGeometry->sectorSize * i;
        flashEraseSector(flashAddress);
        flashWaitForReady(1000);
        LED0_TOGGLE;
    }

    LED0_OFF;

    return true;
}
#endif

int main(void)
{
    init();

    uint32_t bootloaderRequest = persistentObjectRead(PERSISTENT_OBJECT_RESET_REASON);
    if ((bootloaderRequest == RESET_BOOTLOADER_FIRMWARE_UPDATE) || (bootloaderRequest == RESET_BOOTLOADER_FIRMWARE_ROLLBACK)) {
        flashOperation_e flashOperation = (bootloaderRequest == RESET_BOOTLOADER_FIRMWARE_UPDATE ? FLASH_OPERATION_UPDATE : FLASH_OPERATION_ROLLBACK);
        const bool success = flash(flashOperation);
        persistentObjectWrite(PERSISTENT_OBJECT_RESET_REASON, success ? RESET_BOOTLOADER_FIRMWARE_UPDATE_SUCCESS : RESET_BOOTLOADER_FIRMWARE_UPDATE_FAILED);
    } else if (*(uint32_t*)FIRMWARE_START_ADDRESS == 0xFFFFFFFF) {
        if (!flash(FLASH_OPERATION_ROLLBACK)) {
            LED0_OFF;
            LED1_OFF;

            while (true) {
                LED0_TOGGLE;
                LED1_TOGGLE;
                delay(2000);
            }
        }
    }

    bootloader_jump_to_app();

    return 0;
}

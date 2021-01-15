
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "common/log.h"

#include "drivers/flash.h"

#include "fc/firmware_update_common.h"

#include "io/asyncfatfs/asyncfatfs.h"

#ifdef MSP_FIRMWARE_UPDATE

#if !(defined(USE_FLASHFS) || defined(USE_SDCARD))
#error No storage backend available
#endif

#ifdef USE_SDCARD
static afatfsFilePtr_t metaFile = NULL;

static void metaFileOpenCallback(afatfsFilePtr_t file)
{
    metaFile = file;
}
#endif

bool firmwareUpdateMetadataRead(firmwareUpdateMetadata_t *updateMetadata)
{
#if defined(USE_SDCARD)
    if ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY)
            || !afatfs_fopen(FIRMWARE_UPDATE_META_FILENAME, "r", metaFileOpenCallback)
            || (afatfs_freadSync(metaFile, (uint8_t *)updateMetadata, sizeof(*updateMetadata)) != sizeof(*updateMetadata))) {
        return false;
    }

#elif defined(USE_FLASHFS)
    flashPartition_t *flashPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_FIRMWARE_UPDATE_META);
    if (!flashPartition) return false;

    const flashGeometry_t *flashGeometry = flashGetGeometry();
    uint32_t flashAddress = flashPartition->startSector * flashGeometry->sectorSize;

    if (!flashReadBytes(flashAddress, (uint8_t *)updateMetadata, sizeof(*updateMetadata))) {
        return false;
    }

#endif

    return true;
}

bool firmwareUpdateMetadataWrite(firmwareUpdateMetadata_t *updateMetadata)
{
    updateMetadata->magic = FIRMWARE_UPDATE_METADATA_MAGIC;

#if defined(USE_SDCARD)
    if ((afatfs_getFilesystemState() != AFATFS_FILESYSTEM_STATE_READY)
            || !afatfs_fopen(FIRMWARE_UPDATE_META_FILENAME, "w+", metaFileOpenCallback)
            || (afatfs_fwriteSync(metaFile, (uint8_t *)updateMetadata, sizeof(*updateMetadata)) != sizeof(*updateMetadata))) {
        return false;
    }

    afatfs_fcloseSync(metaFile);

#elif defined(USE_FLASHFS)
    flashPartition_t *flashPartition = flashPartitionFindByType(FLASH_PARTITION_TYPE_FIRMWARE_UPDATE_META);
    if (!flashPartition) return false;

    const flashGeometry_t *flashGeometry = flashGetGeometry();
    const uint32_t flashPartitionSize = (flashPartition->endSector - flashPartition->startSector + 1) * flashGeometry->sectorSize;
    uint32_t flashAddress = flashPartition->startSector * flashGeometry->sectorSize;

    if (flashPartitionSize < sizeof(*updateMetadata)) return false;

    flashEraseSector(flashAddress);
    flashWaitForReady(1000);
    flashPageProgram(flashAddress, (uint8_t *)updateMetadata, sizeof(*updateMetadata));

#endif

    return true;
}

#endif

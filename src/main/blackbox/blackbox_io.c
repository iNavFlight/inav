/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#if defined(SITL_BUILD)
#include <stdio.h>
#include <time.h>
#endif

#include "platform.h"

#ifdef USE_BLACKBOX

#include "blackbox.h"
#include "blackbox_io.h"

#include "common/axis.h"
#include "common/encoding.h"
#include "common/maths.h"
#include "common/printf.h"
#include "common/typeconversion.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#ifdef USE_SDCARD
#include "drivers/sdcard/sdcard.h"
#endif

#include "io/asyncfatfs/asyncfatfs.h"
#include "io/flashfs.h"
#include "io/serial.h"

#include "msp/msp_serial.h"

#include "sensors/gyro.h"
#include "fc/config.h"

#define BLACKBOX_SERIAL_PORT_MODE MODE_TX

// How many bytes can we transmit per loop iteration when writing headers?
static uint8_t blackboxMaxHeaderBytesPerIteration;

// How many bytes can we write *this* iteration without overflowing transmit buffers or overstressing the OpenLog?
int32_t blackboxHeaderBudget;

STATIC_UNIT_TESTED serialPort_t *blackboxPort = NULL;
#ifndef UNIT_TEST
static portSharing_e blackboxPortSharing;
#endif // UNIT_TEST

#ifdef USE_SDCARD

static struct {
    afatfsFilePtr_t logFile;
    afatfsFilePtr_t logDirectory;
    afatfsFinder_t logDirectoryFinder;
    uint32_t largestLogFileNumber;

    // Wraparound scan state. The directory is scanned once for the log to evict: the one starting at or after
    // scanBoundaryCluster (the freefile's start when the card filled), as it is what blocks the freefile. If none,
    // the lowest-addressed log overall (the freefile reached the end of the region and wraps).
    // scratch* tracks the two lowest first-clusters >= scanBoundaryCluster; oldestLog* the two lowest overall.
    // The "second" value is the next surviving log's start, which bounds the reclaimed region.
    // UINT32_MAX in the first/second cluster fields means "none found yet".
    uint32_t scanBoundaryCluster;
    afatfsDirEntryPointer_t scratchEntryPos;
    uint32_t scratchFirstCluster;
    uint32_t scratchSecondCluster;
    uint32_t scratchFileSize;
    afatfsDirEntryPointer_t oldestLogEntryPos;
    uint32_t oldestLogFirstCluster;
    uint32_t oldestLogSecondCluster;
    uint32_t oldestLogFileSize;
    // Set when a scan finds nothing usable to reclaim; stops retries until logging is next started
    bool wraparoundGaveUp;

    enum {
        BLACKBOX_SDCARD_INITIAL,
        BLACKBOX_SDCARD_WAITING,
        BLACKBOX_SDCARD_ENUMERATE_FILES,
        BLACKBOX_SDCARD_CHANGE_INTO_LOG_DIRECTORY,
        BLACKBOX_SDCARD_READY_TO_CREATE_LOG,
        BLACKBOX_SDCARD_SCAN_FOR_OLDEST_LOG,
        BLACKBOX_SDCARD_DELETE_OLDEST_LOG,
        BLACKBOX_SDCARD_RELOCATE_FREEFILE,
        BLACKBOX_SDCARD_READY_TO_LOG
    } state;
} blackboxSDCard;

#endif

#if defined(SITL_BUILD)
static struct {
    FILE *file_handler;
} blackboxFile;
#endif

#ifndef UNIT_TEST
void blackboxOpen(void)
{
    serialPort_t *sharedBlackboxAndMspPort = findSharedSerialPort(FUNCTION_BLACKBOX, FUNCTION_MSP);
    if (sharedBlackboxAndMspPort) {
        mspSerialReleasePortIfAllocated(sharedBlackboxAndMspPort);
    }
}
#endif // UNIT_TEST

void blackboxWrite(uint8_t value)
{
    switch (blackboxConfig()->device) {
#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        flashfsWriteByte(value); // Write byte asynchronously
        break;
#endif
#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        afatfs_fputc(blackboxSDCard.logFile, value);
        break;
#endif
#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        fputc(value, blackboxFile.file_handler);
        break;
#endif
    case BLACKBOX_DEVICE_SERIAL:
    default:
        serialWrite(blackboxPort, value);
        break;
    }
}

// Print the null-terminated string 's' to the blackbox device and return the number of bytes written
int blackboxPrint(const char *s)
{
    int length;
    const uint8_t *pos;

    switch (blackboxConfig()->device) {

#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        length = strlen(s);
        flashfsWrite((const uint8_t*) s, length, false); // Write asynchronously
        break;
#endif

#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        length = strlen(s);
        afatfs_fwrite(blackboxSDCard.logFile, (const uint8_t*) s, length); // Ignore failures due to buffers filling up
        break;
#endif

#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        length = strlen(s);
        fputs(s, blackboxFile.file_handler);
        break;
#endif

    case BLACKBOX_DEVICE_SERIAL:
    default:
        pos = (uint8_t*) s;
        while (*pos) {
            serialWrite(blackboxPort, *pos);
            pos++;
        }

        length = pos - (uint8_t*) s;
        break;
    }

    return length;
}

/**
 * If there is data waiting to be written to the blackbox device, attempt to write (a portion of) that now.
 *
 * Intended to be called regularly for the blackbox device to perform housekeeping.
 */
void blackboxDeviceFlush(void)
{
    switch (blackboxConfig()->device) {
#ifdef USE_FLASHFS
        /*
         * This is our only output device which requires us to call flush() in order for it to write anything. The other
         * devices will progressively write in the background without Blackbox calling anything.
         */
    case BLACKBOX_DEVICE_FLASH:
        flashfsFlushAsync();
        break;
#endif

    default:
        ;
    }
}

/**
 * If there is data waiting to be written to the blackbox device, attempt to write (a portion of) that now.
 *
 * Returns true if all data has been written to the device.
 */
bool blackboxDeviceFlushForce(void)
{
    switch (blackboxConfig()->device) {
    case BLACKBOX_DEVICE_SERIAL:
        // Nothing to speed up flushing on serial, as serial is continuously being drained out of its buffer
        return isSerialTransmitBufferEmpty(blackboxPort);

#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        return flashfsFlushAsync();
#endif

#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        /* SD card will flush itself without us calling it, but we need to call flush manually in order to check
         * if it's done yet or not!
         */
        return afatfs_flush();
#endif

#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        fflush(blackboxFile.file_handler);
        return true;
#endif

    default:
        return false;
    }
}

/**
 * Attempt to open the logging device. Returns true if successful.
 */
#ifndef UNIT_TEST
bool blackboxDeviceOpen(void)
{
    switch (blackboxConfig()->device) {
    case BLACKBOX_DEVICE_SERIAL:
        {
            serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_BLACKBOX);
            baudRate_e baudRateIndex;
            portOptions_t portOptions = SERIAL_PARITY_NO | SERIAL_NOT_INVERTED;

            if (!portConfig) {
                return false;
            }

            blackboxPortSharing = determinePortSharing(portConfig, FUNCTION_BLACKBOX);
            baudRateIndex = portConfig->peripheral_baudrateIndex;

            if (baudRates[baudRateIndex] == 230400) {
                /*
                 * OpenLog's 230400 baud rate is very inaccurate, so it requires a larger inter-character gap in
                 * order to maintain synchronization.
                 */
                portOptions |= SERIAL_STOPBITS_2;
            } else {
                portOptions |= SERIAL_STOPBITS_1;
            }

            blackboxPort = openSerialPort(portConfig->identifier, FUNCTION_BLACKBOX, NULL, NULL, baudRates[baudRateIndex],
                BLACKBOX_SERIAL_PORT_MODE, portOptions);

            /*
             * The slowest MicroSD cards have a write latency approaching 150ms. The OpenLog's buffer is about 900
             * bytes. In order for its buffer to be able to absorb this latency we must write slower than 6000 B/s.
             *
             * So:
             *     Bytes per loop iteration = floor((looptime_ns / 1000000.0) * 6000)
             *                              = floor((looptime_ns * 6000) / 1000000.0)
             *                              = floor((looptime_ns * 3) / 500.0)
             *                              = (looptime_ns * 3) / 500
             */
            blackboxMaxHeaderBytesPerIteration = constrain((getLooptime() * 3) / 500, 1, BLACKBOX_TARGET_HEADER_BUDGET_PER_ITERATION);

            return blackboxPort != NULL;
        }
        break;
#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        if (flashfsGetSize() == 0 || isBlackboxDeviceFull()) {
            return false;
        }

        blackboxMaxHeaderBytesPerIteration = BLACKBOX_TARGET_HEADER_BUDGET_PER_ITERATION;

        return true;
        break;
#endif
#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        if (afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_FATAL || afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_UNKNOWN || afatfs_isFull()) {
            return false;
        }

        blackboxMaxHeaderBytesPerIteration = BLACKBOX_TARGET_HEADER_BUDGET_PER_ITERATION;

        return true;
        break;
#endif
#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        {
            const time_t now = time(NULL);
            const struct tm *t = localtime(&now);
            char filename[32];
            strftime(filename, sizeof(filename), "%Y_%m_%d_%H%M%S.TXT", t);

            blackboxFile.file_handler = fopen(filename, "wb");
            if (blackboxFile.file_handler == NULL) {
                fprintf(stderr, "[BlackBox] Failed to create log file\n");
                return false;
            }
            fprintf(stderr, "[BlackBox] Created %s\n", filename);
        }

        blackboxMaxHeaderBytesPerIteration = BLACKBOX_TARGET_HEADER_BUDGET_PER_ITERATION;
        return true;
        break;
#endif
    default:
        return false;
    }
}
#endif // UNIT_TEST

/**
 * Close the Blackbox logging device immediately without attempting to flush any remaining data.
 */
#ifndef UNIT_TEST
void blackboxDeviceClose(void)
{
    switch (blackboxConfig()->device) {
    case BLACKBOX_DEVICE_SERIAL:
        // Since the serial port could be shared with other processes, we have to give it back here
        closeSerialPort(blackboxPort);
        blackboxPort = NULL;

        /*
         * Normally this would be handled by mw.c, but since we take an unknown amount
         * of time to shut down asynchronously, we're the only ones that know when to call it.
         */
        if (blackboxPortSharing == PORTSHARING_SHARED) {
            mspSerialAllocatePorts();
        }
        break;
#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        // Some flash device, e.g., NAND devices, require explicit close to flush internally buffered data.
        flashfsClose();
        break;
#endif
#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        fclose(blackboxFile.file_handler);
        blackboxFile.file_handler = NULL;
        break;
#endif
    default:
        ;
    }
}
#endif // UNIT_TEST

#ifdef USE_SDCARD

static void blackboxOldestLogDirOpened(afatfsFilePtr_t directory)
{
    if (directory) {
        blackboxSDCard.logDirectory = directory;
        blackboxSDCard.scanBoundaryCluster = afatfs_freeFileFirstCluster();
        blackboxSDCard.scratchFirstCluster = UINT32_MAX; // Sentinel: nothing found yet
        blackboxSDCard.scratchSecondCluster = UINT32_MAX;
        blackboxSDCard.oldestLogFirstCluster = UINT32_MAX;
        blackboxSDCard.oldestLogSecondCluster = UINT32_MAX;

        afatfs_findFirst(blackboxSDCard.logDirectory, &blackboxSDCard.logDirectoryFinder);

        blackboxSDCard.state = BLACKBOX_SDCARD_SCAN_FOR_OLDEST_LOG;
    } else {
        // Retry
        blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
    }
}

static void blackboxLogDirCreated(afatfsFilePtr_t directory)
{
    if (directory) {
        blackboxSDCard.logDirectory = directory;

        afatfs_findFirst(blackboxSDCard.logDirectory, &blackboxSDCard.logDirectoryFinder);

        blackboxSDCard.state = BLACKBOX_SDCARD_ENUMERATE_FILES;
    } else {
        // Retry
        blackboxSDCard.state = BLACKBOX_SDCARD_INITIAL;
    }
}

static void blackboxLogFileCreated(afatfsFilePtr_t file)
{
    if (file) {
        blackboxSDCard.logFile = file;

        blackboxSDCard.largestLogFileNumber++;

        blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_LOG;
    } else {
        // Retry
        blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
    }
}

static void blackboxCreateLogFile(void)
{
    uint32_t remainder = blackboxSDCard.largestLogFileNumber + 1;

    char filename[13];

    filename[0] = 'L';
    filename[1] = 'O';
    filename[2] = 'G';

    for (int i = 7; i >= 3; i--) {
        filename[i] = (remainder % 10) + '0';
        remainder /= 10;
    }

    filename[8] = '.';
    filename[9] = 'T';
    filename[10] = 'X';
    filename[11] = 'T';
    filename[12] = 0;

    blackboxSDCard.state = BLACKBOX_SDCARD_WAITING;

    afatfs_fopen(filename, "as", blackboxLogFileCreated);
}

/**
 * Begin a new log on the SDCard.
 *
 * Keep calling until the function returns true (open is complete).
 */
static bool blackboxSDCardBeginLog(void)
{
    fatDirectoryEntry_t *directoryEntry;

    doMore:
    switch (blackboxSDCard.state) {
    case BLACKBOX_SDCARD_INITIAL:
        if (afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_READY) {
            blackboxSDCard.state = BLACKBOX_SDCARD_WAITING;
            blackboxSDCard.wraparoundGaveUp = false;

            if(afatfs_isCurrentDirRoot()){
                //we are in root of SD card, we have to create or move to log directory
                afatfs_mkdir("logs", blackboxLogDirCreated);
            }
            else
            {
                //we are already in log directory
                blackboxSDCard.logDirectory = NULL;
                blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
            }
        }
        break;

    case BLACKBOX_SDCARD_WAITING:
        // Waiting for directory entry to be created
        break;

    case BLACKBOX_SDCARD_ENUMERATE_FILES:

        if (blackboxSDCard.logDirectory == NULL) {
            blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
            break;
        }

        while (afatfs_findNext(blackboxSDCard.logDirectory, &blackboxSDCard.logDirectoryFinder, &directoryEntry) == AFATFS_OPERATION_SUCCESS) {
            if (directoryEntry && !fat_isDirectoryEntryTerminator(directoryEntry)) {
                // If this is a log file, parse the log number from the filename
                if (
                    directoryEntry->filename[0] == 'L' && directoryEntry->filename[1] == 'O' && directoryEntry->filename[2] == 'G'
                    && directoryEntry->filename[8] == 'T' && directoryEntry->filename[9] == 'X' && directoryEntry->filename[10] == 'T'
                ) {
                    char logSequenceNumberString[6];

                    memcpy(logSequenceNumberString, directoryEntry->filename + 3, 5);
                    logSequenceNumberString[5] = '\0';

                    blackboxSDCard.largestLogFileNumber = MAX((uint32_t) fastA2I(logSequenceNumberString), blackboxSDCard.largestLogFileNumber);
                }
            } else {
                // We're done checking all the files on the card, now we can create a new log file
                afatfs_findLast(blackboxSDCard.logDirectory);

                blackboxSDCard.state = BLACKBOX_SDCARD_CHANGE_INTO_LOG_DIRECTORY;
                goto doMore;
            }
        }
        break;

    case BLACKBOX_SDCARD_CHANGE_INTO_LOG_DIRECTORY:
        //if logDirectory is NULL, it would mean change directory to ROOT, we don't want to do that
        if (blackboxSDCard.logDirectory == NULL) {
            blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
            break;
        }

        // Change into the log directory:
        if (afatfs_chdir(blackboxSDCard.logDirectory)) {
            // We no longer need our open handle on the log directory
            afatfs_fclose(blackboxSDCard.logDirectory, NULL);
            blackboxSDCard.logDirectory = NULL;

            blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
            goto doMore;
        }
        break;

    case BLACKBOX_SDCARD_READY_TO_CREATE_LOG:
        if (blackboxConfig()->sdcardWraparound && afatfs_isFull() && !blackboxSDCard.wraparoundGaveUp) {
            blackboxSDCard.state = BLACKBOX_SDCARD_WAITING;
            afatfs_fopen(".", "r", blackboxOldestLogDirOpened);
        } else {
            blackboxCreateLogFile();
        }
        break;

    case BLACKBOX_SDCARD_SCAN_FOR_OLDEST_LOG:
        while (afatfs_findNext(blackboxSDCard.logDirectory, &blackboxSDCard.logDirectoryFinder, &directoryEntry) == AFATFS_OPERATION_SUCCESS) {
            if (directoryEntry && !fat_isDirectoryEntryTerminator(directoryEntry)) {
                // Match deleted entries too (first byte replaced by the deleted marker): their clusters are free
                // but still delimit the region.
                if (
                    (directoryEntry->filename[0] == 'L' || directoryEntry->filename[0] == FAT_DELETED_FILE_MARKER)
                    && directoryEntry->filename[1] == 'O' && directoryEntry->filename[2] == 'G'
                    && directoryEntry->filename[8] == 'T' && directoryEntry->filename[9] == 'X' && directoryEntry->filename[10] == 'T'
                ) {
                    uint32_t entryFirstCluster = ((uint32_t) directoryEntry->firstClusterHigh << 16) | directoryEntry->firstClusterLow;
                    // Cluster 0 means no clusters were ever allocated (e.g. a log discarded by
                    // blackboxDeviceEndLog(false)); it must not become a candidate or it would always win.
                    if (entryFirstCluster != 0) {
                        if (entryFirstCluster >= blackboxSDCard.scanBoundaryCluster) {
                            if (entryFirstCluster < blackboxSDCard.scratchFirstCluster) {
                                blackboxSDCard.scratchSecondCluster = blackboxSDCard.scratchFirstCluster;
                                blackboxSDCard.scratchEntryPos = blackboxSDCard.logDirectoryFinder;
                                blackboxSDCard.scratchFirstCluster = entryFirstCluster;
                                blackboxSDCard.scratchFileSize = directoryEntry->fileSize;
                            } else if (entryFirstCluster < blackboxSDCard.scratchSecondCluster) {
                                blackboxSDCard.scratchSecondCluster = entryFirstCluster;
                            }
                        }
                        if (entryFirstCluster < blackboxSDCard.oldestLogFirstCluster) {
                            blackboxSDCard.oldestLogSecondCluster = blackboxSDCard.oldestLogFirstCluster;
                            blackboxSDCard.oldestLogEntryPos = blackboxSDCard.logDirectoryFinder;
                            blackboxSDCard.oldestLogFirstCluster = entryFirstCluster;
                            blackboxSDCard.oldestLogFileSize = directoryEntry->fileSize;
                        } else if (entryFirstCluster < blackboxSDCard.oldestLogSecondCluster) {
                            blackboxSDCard.oldestLogSecondCluster = entryFirstCluster;
                        }
                    }
                }
            } else {
                afatfs_findLast(blackboxSDCard.logDirectory);
                afatfs_fclose(blackboxSDCard.logDirectory, NULL);
                blackboxSDCard.logDirectory = NULL;

                uint32_t nextBoundaryCluster;
                if (blackboxSDCard.scratchFirstCluster != UINT32_MAX) {
                    blackboxSDCard.oldestLogEntryPos = blackboxSDCard.scratchEntryPos;
                    blackboxSDCard.oldestLogFirstCluster = blackboxSDCard.scratchFirstCluster;
                    blackboxSDCard.oldestLogFileSize = blackboxSDCard.scratchFileSize;
                    nextBoundaryCluster = blackboxSDCard.scratchSecondCluster;
                } else {
                    nextBoundaryCluster = blackboxSDCard.oldestLogSecondCluster;
                }

                if (nextBoundaryCluster != UINT32_MAX) {
                    // Reclaim up to the next log's start: it is supercluster-aligned, so the next lap's boundary search matches again.
                    // Size is in bytes.
                    blackboxSDCard.oldestLogFileSize =
                        (nextBoundaryCluster - blackboxSDCard.oldestLogFirstCluster) * afatfs_freeFileClusterSize();
                }
                // Else nothing lies beyond the evicted log; its own fileSize is used as a conservative size.

                if (
                    blackboxSDCard.oldestLogFirstCluster == UINT32_MAX || blackboxSDCard.oldestLogFirstCluster == 0
                    || blackboxSDCard.oldestLogFileSize < afatfs_freeFileSuperClusterSize()
                ) {
                    // Nothing usable: no logs, no allocated clusters, or less than one supercluster
                    blackboxSDCard.wraparoundGaveUp = true;
                    blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
                } else {
                    blackboxSDCard.state = BLACKBOX_SDCARD_DELETE_OLDEST_LOG;
                }
                goto doMore;
            }
        }
        break;

    case BLACKBOX_SDCARD_DELETE_OLDEST_LOG:
        // Drain the cache first so the delete is flushed before the freefile relocation; see afatfs_hasPendingWrites()
        if (afatfs_hasPendingWrites()) {
            break;
        }

        if (afatfs_deleteDirectoryEntryAt(&blackboxSDCard.oldestLogEntryPos) == AFATFS_OPERATION_SUCCESS) {
            blackboxSDCard.state = BLACKBOX_SDCARD_RELOCATE_FREEFILE;
            goto doMore;
        }
        break;

    case BLACKBOX_SDCARD_RELOCATE_FREEFILE:
        if (afatfs_freeFileRelocate(blackboxSDCard.oldestLogFirstCluster, blackboxSDCard.oldestLogFileSize) == AFATFS_OPERATION_SUCCESS) {
            blackboxSDCard.state = BLACKBOX_SDCARD_READY_TO_CREATE_LOG;
            goto doMore;
        }
        break;

    case BLACKBOX_SDCARD_READY_TO_LOG:
        return true; // Log has been created!
    }

    // Not finished init yet
    return false;
}

#endif

/**
 * Begin a new log (for devices which support separations between the logs of multiple flights).
 *
 * Keep calling until the function returns true (open is complete).
 */
bool blackboxDeviceBeginLog(void)
{
    switch (blackboxConfig()->device) {
#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        return blackboxSDCardBeginLog();
#endif
    default:
        return true;
    }

}

/**
 * Terminate the current log (for devices which support separations between the logs of multiple flights).
 *
 * retainLog - Pass true if the log should be kept, or false if the log should be discarded (if supported).
 *
 * Keep calling until this returns true
 */
bool blackboxDeviceEndLog(bool retainLog)
{
#ifndef USE_SDCARD
    (void) retainLog;
#endif

    switch (blackboxConfig()->device) {
#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        // Keep retrying until the close operation queues
        if (
            (retainLog && afatfs_fclose(blackboxSDCard.logFile, NULL))
            || (!retainLog && afatfs_funlink(blackboxSDCard.logFile, NULL))
        ) {
            // Don't bother waiting the for the close to complete, it's queued now and will complete eventually
            blackboxSDCard.logFile = NULL;
            blackboxSDCard.state = BLACKBOX_SDCARD_INITIAL;
            return true;
        }
        return false;
#endif
    default:
        return true;
    }
}

bool isBlackboxDeviceFull(void)
{
    switch (blackboxConfig()->device) {
    case BLACKBOX_DEVICE_SERIAL:
        return false;

#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        return flashfsIsEOF();
#endif

#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        // With wraparound enabled, a full card is only "full" once reclaiming has failed
        return afatfs_isFull() && (!blackboxConfig()->sdcardWraparound || blackboxSDCard.wraparoundGaveUp);
#endif

#if defined (SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        return false;
#endif

    default:
        return false;
    }
}

#ifdef USE_SDCARD
/**
 * True if the card is full but wraparound may still reclaim space: close this log and start a new one.
 */
bool blackboxDeviceNeedsNewLog(void)
{
    return blackboxConfig()->device == BLACKBOX_DEVICE_SDCARD
        && blackboxConfig()->sdcardWraparound
        && afatfs_isFull()
        && !blackboxSDCard.wraparoundGaveUp;
}
#endif

bool isBlackboxDeviceWorking(void)
{
    switch (blackboxConfig()->device) {
        case BLACKBOX_DEVICE_SERIAL:
            return blackboxPort != NULL;
#ifdef USE_SDCARD
        case BLACKBOX_DEVICE_SDCARD:
            return sdcard_isInserted() && sdcard_isFunctional() && (afatfs_getFilesystemState() == AFATFS_FILESYSTEM_STATE_READY);
#endif
#ifdef USE_FLASHFS
        case BLACKBOX_DEVICE_FLASH:
            return flashfsIsReady();
#endif
#if defined(SITL_BUILD)
        case BLACKBOX_DEVICE_FILE:
            return blackboxFile.file_handler != NULL;
#endif
    default:
        return false;
    }
}

int32_t blackboxGetLogNumber(void)
{
    switch (blackboxConfig()->device) {
#ifdef USE_SDCARD
        case BLACKBOX_DEVICE_SDCARD:
            return blackboxSDCard.largestLogFileNumber;
#endif
        default:
            return -1;
    }
}

/**
 * Call once every loop iteration in order to maintain the global blackboxHeaderBudget with the number of bytes we can
 * transmit this iteration.
 */
void blackboxReplenishHeaderBudget(void)
{
    int32_t freeSpace;

    switch (blackboxConfig()->device) {
    case BLACKBOX_DEVICE_SERIAL:
        freeSpace = serialTxBytesFree(blackboxPort);
        break;
#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        freeSpace = flashfsGetWriteBufferFreeSpace();
        break;
#endif
#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        freeSpace = afatfs_getFreeBufferSpace();
        break;
#endif
#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        freeSpace = BLACKBOX_MAX_ACCUMULATED_HEADER_BUDGET;
        break;
#endif
    default:
        freeSpace = 0;
    }

    blackboxHeaderBudget = MIN(MIN(freeSpace, blackboxHeaderBudget + blackboxMaxHeaderBytesPerIteration), BLACKBOX_MAX_ACCUMULATED_HEADER_BUDGET);
}

/**
 * You must call this function before attempting to write Blackbox header bytes to ensure that the write will not
 * cause buffers to overflow. The number of bytes you can write is capped by the blackboxHeaderBudget. Calling this
 * reservation function doesn't decrease blackboxHeaderBudget, so you must manually decrement that variable by the
 * number of bytes you actually wrote.
 *
 * When the Blackbox device is FlashFS, a successful return code guarantees that no data will be lost if you write that
 * many bytes to the device (i.e. FlashFS's buffers won't overflow).
 *
 * When the device is a serial port, a successful return code guarantees that Cleanflight's serial Tx buffer will not
 * overflow, and the outgoing bandwidth is likely to be small enough to give the OpenLog time to absorb MicroSD card
 * latency. However the OpenLog could still end up silently dropping data.
 *
 * Returns:
 *  BLACKBOX_RESERVE_SUCCESS - Upon success
 *  BLACKBOX_RESERVE_TEMPORARY_FAILURE - The buffer is currently too full to service the request, try again later
 *  BLACKBOX_RESERVE_PERMANENT_FAILURE - The buffer is too small to ever service this request
 */
blackboxBufferReserveStatus_e blackboxDeviceReserveBufferSpace(int32_t bytes)
{
    if (bytes <= blackboxHeaderBudget) {
        return BLACKBOX_RESERVE_SUCCESS;
    }

    // Handle failure:
    switch (blackboxConfig()->device) {
    case BLACKBOX_DEVICE_SERIAL:
        /*
         * One byte of the tx buffer isn't available for user data (due to its circular list implementation),
         * hence the -1. Note that the USB VCP implementation doesn't use a buffer and has txBufferSize set to zero.
         */
        if (blackboxPort->txBufferSize && bytes > (int32_t) blackboxPort->txBufferSize - 1) {
            return BLACKBOX_RESERVE_PERMANENT_FAILURE;
        }

        return BLACKBOX_RESERVE_TEMPORARY_FAILURE;

#ifdef USE_FLASHFS
    case BLACKBOX_DEVICE_FLASH:
        if (bytes > (int32_t) flashfsGetWriteBufferSize()) {
            return BLACKBOX_RESERVE_PERMANENT_FAILURE;
        }

        if (bytes > (int32_t) flashfsGetWriteBufferFreeSpace()) {
            /*
             * The write doesn't currently fit in the buffer, so try to make room for it. Our flushing here means
             * that the Blackbox header writing code doesn't have to guess about the best time to ask flashfs to
             * flush, and doesn't stall waiting for a flush that would otherwise not automatically be called.
             */
            flashfsFlushAsync();
        }

        return BLACKBOX_RESERVE_TEMPORARY_FAILURE;
#endif

#ifdef USE_SDCARD
    case BLACKBOX_DEVICE_SDCARD:
        // Assume that all writes will fit in the SDCard's buffers
        return BLACKBOX_RESERVE_TEMPORARY_FAILURE;
#endif

#if defined(SITL_BUILD)
    case BLACKBOX_DEVICE_FILE:
        // Assume that all writes will fit in the file's buffers
        return BLACKBOX_RESERVE_TEMPORARY_FAILURE;
#endif

    default:
        return BLACKBOX_RESERVE_PERMANENT_FAILURE;
    }
}

#endif

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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#ifdef USE_FLASH_M25P16

#include "flash_m25p16.h"
#include "io.h"
#include "bus_spi.h"
#include "system.h"

#define M25P16_INSTRUCTION_RDID             0x9F
#define M25P16_INSTRUCTION_READ_BYTES       0x03
#define M25P16_INSTRUCTION_READ_STATUS_REG  0x05
#define M25P16_INSTRUCTION_WRITE_STATUS_REG 0x01
#define M25P16_INSTRUCTION_WRITE_ENABLE     0x06
#define M25P16_INSTRUCTION_WRITE_DISABLE    0x04
#define M25P16_INSTRUCTION_PAGE_PROGRAM     0x02
#define M25P16_INSTRUCTION_SECTOR_ERASE     0xD8
#define M25P16_INSTRUCTION_BULK_ERASE       0xC7

#define M25P16_STATUS_FLAG_WRITE_IN_PROGRESS 0x01
#define M25P16_STATUS_FLAG_WRITE_ENABLED     0x02

// Format is manufacturer, memory type, then capacity
#define JEDEC_ID_MICRON_M25P16         0x202015
#define JEDEC_ID_MICRON_N25Q064        0x20BA17
#define JEDEC_ID_WINBOND_W25Q64        0xEF4017
#define JEDEC_ID_MACRONIX_MX25L3206E   0xC22016
#define JEDEC_ID_MACRONIX_MX25L6406E   0xC22017
#define JEDEC_ID_MICRON_N25Q128        0x20ba18
#define JEDEC_ID_WINBOND_W25Q128       0xEF4018

// The timeout we expect between being able to issue page program instructions
#define DEFAULT_TIMEOUT_MILLIS       6

// These take sooooo long:
#define SECTOR_ERASE_TIMEOUT_MILLIS  5000
#define BULK_ERASE_TIMEOUT_MILLIS    21000

static flashGeometry_t geometry = {.pageSize = M25P16_PAGESIZE};

static IO_t m25p16CsPin = IO_NONE;

/*
 * Whether we've performed an action that could have made the device busy for writes.
 *
 * This allows us to avoid polling for writable status when it is definitely ready already.
 */
static bool couldBeBusy = false;

/**
 * Send the given command byte to the device.
 */
static void m25p16_performOneByteCommand(uint8_t command)
{
    uint8_t buffer[] = { command };
    busWrite(M25P16_BUS, (uint32_t)m25p16CsPin, buffer, 1, NULL, 0);
}

/**
 * The flash requires this write enable command to be sent before commands that would cause
 * a write like program and erase.
 */
static void m25p16_writeEnable()
{
    m25p16_performOneByteCommand(M25P16_INSTRUCTION_WRITE_ENABLE);

    // Assume that we're about to do some writing, so the device is just about to become busy
    couldBeBusy = true;
}

static uint8_t m25p16_readStatus()
{
    uint8_t command[1] = { M25P16_INSTRUCTION_READ_STATUS_REG };
    uint8_t data[1];
    busRead(M25P16_BUS, (uint32_t)m25p16CsPin, command, 1, data, 1);
    return data[0];
}

bool m25p16_isReady()
{
    // If couldBeBusy is false, don't bother to poll the flash chip for its status
    couldBeBusy = couldBeBusy && ((m25p16_readStatus() & M25P16_STATUS_FLAG_WRITE_IN_PROGRESS) != 0);

    return !couldBeBusy;
}

bool m25p16_waitForReady(uint32_t timeoutMillis)
{
    uint32_t time = millis();
    while (!m25p16_isReady()) {
        if (millis() - time > timeoutMillis) {
            return false;
        }
    }

    return true;
}

/**
 * Read chip identification and geometry information (into global `geometry`).
 *
 * Returns true if we get valid ident, false if something bad happened like there is no M25P16.
 */
static bool m25p16_readIdentification()
{
    uint8_t command[] = { M25P16_INSTRUCTION_RDID };
    uint8_t data[3];
    uint32_t chipID;

    delay(50); // short delay required after initialisation of SPI device instance.

    /* Just in case transfer fails and writes nothing, so we don't try to verify the ID against random garbage
     * from the stack:
     */
    data[0] = 0;

    busRead(M25P16_BUS, (uint32_t)m25p16CsPin, command, 1, data, 3);

    // Manufacturer, memory type, and capacity
    chipID = (data[0] << 16) | (data[1] << 8) | (data[2]);

    // All supported chips use the same pagesize of 256 bytes

    switch (chipID) {
        case JEDEC_ID_MICRON_M25P16:
            geometry.sectors = 32;
            geometry.pagesPerSector = 256;
        break;
        case JEDEC_ID_MACRONIX_MX25L3206E:
            geometry.sectors = 64;
            geometry.pagesPerSector = 256;
        break;
        case JEDEC_ID_MICRON_N25Q064:
        case JEDEC_ID_WINBOND_W25Q64:
        case JEDEC_ID_MACRONIX_MX25L6406E:
            geometry.sectors = 128;
            geometry.pagesPerSector = 256;
        break;
        case JEDEC_ID_MICRON_N25Q128:
        case JEDEC_ID_WINBOND_W25Q128:
            geometry.sectors = 256;
            geometry.pagesPerSector = 256;
        break;
        default:
            // Unsupported chip or not an SPI NOR flash
            geometry.sectors = 0;
            geometry.pagesPerSector = 0;

            geometry.sectorSize = 0;
            geometry.totalSize = 0;
            return false;
    }

    geometry.sectorSize = geometry.pagesPerSector * geometry.pageSize;
    geometry.totalSize = geometry.sectorSize * geometry.sectors;

    couldBeBusy = true; // Just for luck we'll assume the chip could be busy even though it isn't specced to be

    return true;
}

/**
 * Initialize the driver, must be called before any other routines.
 *
 * Attempts to detect a connected m25p16. If found, true is returned and device capacity can be fetched with
 * m25p16_getGeometry().
 */
bool m25p16_init(ioTag_t csTag)
{
    /*
        if we have already detected a flash device we can simply exit

        TODO: change the init param in favour of flash CFG when ParamGroups work is done
        then cs pin can be specified in hardware_revision.c or config.c (dependent on revision).
    */
    if (geometry.sectors) {
        return true;
    }

    if (csTag) {
        m25p16CsPin = IOGetByTag(csTag);
    } else {
#ifdef M25P16_CS_PIN
        m25p16CsPin = IOGetByTag(IO_TAG(M25P16_CS_PIN));
#else
        return false;
#endif
    }
    IOInit(m25p16CsPin, OWNER_FLASH, RESOURCE_SPI_CS, 0);
    IOConfigGPIO(m25p16CsPin, SPI_IO_CS_CFG);
    IOHi(m25p16CsPin);

#ifndef M25P16_SPI_SHARED
    //Maximum speed for standard READ command is 20mHz, other commands tolerate 25mHz
    busSetSpeed(M25P16_BUS, BUS_SPEED_FAST);
#endif

    return m25p16_readIdentification();
}

/**
 * Erase a sector full of bytes to all 1's at the given byte offset in the flash chip.
 */
void m25p16_eraseSector(uint32_t address)
{
    uint8_t out[] = { M25P16_INSTRUCTION_SECTOR_ERASE, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};

    m25p16_waitForReady(SECTOR_ERASE_TIMEOUT_MILLIS);

    m25p16_writeEnable();

    busWrite(M25P16_BUS, (uint32_t)m25p16CsPin, out, sizeof(out), NULL, 0);
}

void m25p16_eraseCompletely()
{
    m25p16_waitForReady(BULK_ERASE_TIMEOUT_MILLIS);

    m25p16_writeEnable();

    m25p16_performOneByteCommand(M25P16_INSTRUCTION_BULK_ERASE);
}

/**
 * Write bytes to a flash page. Address must not cross a page boundary.
 *
 * Bits can only be set to zero, not from zero back to one again. In order to set bits to 1, use the erase command.
 *
 * Length must be smaller than the page size.
 *
 * This will wait for the flash to become ready before writing begins.
 *
 * Datasheet indicates typical programming time is 0.8ms for 256 bytes, 0.2ms for 64 bytes, 0.05ms for 16 bytes.
 * (Although the maximum possible write time is noted as 5ms).
 */
void m25p16_pageProgram(uint32_t address, uint8_t *data, int length)
{
    uint8_t command[] = { M25P16_INSTRUCTION_PAGE_PROGRAM, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};

    m25p16_waitForReady(DEFAULT_TIMEOUT_MILLIS);

    m25p16_writeEnable();

    busWrite(M25P16_BUS, (uint32_t)m25p16CsPin, command, sizeof(command), data, length);
}

/**
 * Read `length` bytes into the provided `buffer` from the flash starting from the given `address` (which need not lie
 * on a page boundary).
 *
 * Waits up to DEFAULT_TIMEOUT_MILLIS milliseconds for the flash to become ready before reading.
 *
 * The number of bytes actually read is returned, which can be zero if an error or timeout occurred.
 */
int m25p16_readBytes(uint32_t address, uint8_t *buffer, int length)
{
    uint8_t command[] = { M25P16_INSTRUCTION_READ_BYTES, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF};

    if (!m25p16_waitForReady(DEFAULT_TIMEOUT_MILLIS)) {
        return 0;
    }

    busRead(M25P16_BUS, (uint32_t)m25p16CsPin, command, sizeof(command), buffer, length);

    return length;
}

/**
 * Fetch information about the detected flash chip layout.
 *
 * Can be called before calling m25p16_init() (the result would have totalSize = 0).
 */
const flashGeometry_t* m25p16_getGeometry()
{
    return &geometry;
}

#endif

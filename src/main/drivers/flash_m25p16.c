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
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/time.h"

#if defined(USE_QUADSPI)
#include "drivers/bus_quadspi.h"
#include "drivers/bus_quadspi_impl.h"
#endif

#define M25P16_INSTRUCTION_RDID             0x9F
#define M25P16_INSTRUCTION_READ_BYTES       0x03
#define M25P16_INSTRUCTION_QUAD_READ        0x6B
#define M25P16_INSTRUCTION_READ_STATUS_REG  0x05
#define M25P16_INSTRUCTION_WRITE_STATUS_REG 0x01
#define M25P16_INSTRUCTION_WRITE_ENABLE     0x06
#define M25P16_INSTRUCTION_WRITE_DISABLE    0x04
#define M25P16_INSTRUCTION_PAGE_PROGRAM     0x02
#define M25P16_INSTRUCTION_QPAGE_PROGRAM    0x32
#define M25P16_INSTRUCTION_SECTOR_ERASE     0xD8
#define M25P16_INSTRUCTION_BULK_ERASE       0xC7

#define M25P16_STATUS_FLAG_WRITE_IN_PROGRESS 0x01
#define M25P16_STATUS_FLAG_WRITE_ENABLED     0x02

#define W25Q256_INSTRUCTION_ENTER_4BYTE_ADDRESS_MODE 0xB7

#define M25P16_FAST_READ_DUMMY_CYCLES       8

struct {
    uint32_t        jedecID;
    flashSector_t   sectors;
    uint16_t        pagesPerSector;
} m25p16FlashConfig[] = {
    // Macronix MX25L3206E
    // Datasheet: https://docs.rs-online.com/5c85/0900766b814ac6f9.pdf
    {0xC22016, 64, 256},
    // Macronix MX25L6406E
    // Datasheet: https://www.macronix.com/Lists/Datasheet/Attachments/7370/MX25L6406E,%203V,%2064Mb,%20v1.9.pdf
    {0xC22017, 128, 256},
    // Macronix MX25L25635E
    // Datasheet: https://www.macronix.com/Lists/Datasheet/Attachments/7331/MX25L25635E,%203V,%20256Mb,%20v1.3.pdf
    {0xC22019, 512, 256},
    // Micron M25P16
    // Datasheet: https://www.micron.com/-/media/client/global/documents/products/data-sheet/nor-flash/serial-nor/m25p/m25p16.pdf
    {0x202015, 32, 256},
    // Micron N25Q064
    // Datasheet: https://www.micron.com/-/media/client/global/documents/products/data-sheet/nor-flash/serial-nor/n25q/n25q_64a_3v_65nm.pdf
    {0x20BA17, 128, 256},
    // Micron N25Q128
    // Datasheet: https://www.micron.com/-/media/client/global/documents/products/data-sheet/nor-flash/serial-nor/n25q/n25q_128mb_1_8v_65nm.pdf
    {0x20ba18, 256, 256},
    // Winbond W25Q80
    // Datasheet: https://www.winbond.com/resource-files/w25q80dv%20dl_revh_10022015.pdf
    {0xEF4014, 16, 256},
    // Winbond W25Q16
    // Datasheet: https://www.winbond.com/resource-files/w25q16dv_revi_nov1714_web.pdf
    {0xEF4015, 32, 256},
    // Winbond W25X32
    // Datasheet: https://www.winbond.com/resource-files/w25x32a_revb_080709.pdf
    {0xEF3016, 64, 256},
    // Winbond W25Q32
    // Datasheet: https://www.winbond.com/resource-files/w25q32jv%20dtr%20revf%2002242017.pdf?__locale=zh_TW
    {0xEF4016, 64, 256},
    // Winbond W25Q64
    // Datasheet: https://www.winbond.com/resource-files/w25q64jv%20spi%20%20%20revc%2006032016%20kms.pdf
    {0xEF4017, 128, 256}, // W25Q64JV-IQ/JQ
    {0xEF7017, 128, 256}, // W25Q64JV-IM/JM*
    // Winbond W25Q128
    // Datasheet: https://www.winbond.com/resource-files/w25q128fv%20rev.l%2008242015.pdf
    {0xEF4018, 256, 256},
    // Zbit ZB25VQ128
    // Datasheet: http://zbitsemi.com/upload/file/20201010/20201010174048_82182.pdf
    {0x5E4018, 256, 256},
    // Winbond W25Q128_DTR
    // Datasheet: https://www.winbond.com/resource-files/w25q128jv%20dtr%20revb%2011042016.pdf
    {0xEF7018, 256, 256},
    // Winbond W25Q256
    // Datasheet: https://www.winbond.com/resource-files/w25q256jv%20spi%20revb%2009202016.pdf
    {0xEF4019, 512, 256},
    // Cypress S25FL064L
    // Datasheet: https://www.cypress.com/file/316661/download
    {0x016017, 128, 256},
    // Cypress S25FL128L
    // Datasheet: https://www.cypress.com/file/316171/download
    {0x016018, 256, 256},
    // BergMicro W25Q32
    // Datasheet: https://www.winbond.com/resource-files/w25q32jv%20dtr%20revf%2002242017.pdf?__locale=zh_TW
    {0xE04016, 1024, 16},
    // JEDEC_ID_EON_W25Q64
    {0x1C3017, 128, 256},
    // JEDEC_ID_SPANSION_S25FL116
    {0x014015, 32, 256 },
    // End of list
    {0x000000, 0, 0}};

// The timeout we expect between being able to issue page program instructions
#define DEFAULT_TIMEOUT_MILLIS       6

// These take sooooo long:
#define SECTOR_ERASE_TIMEOUT_MILLIS  5000
#define BULK_ERASE_TIMEOUT_MILLIS    21000

static flashGeometry_t geometry = {.pageSize = M25P16_PAGESIZE};

#if !defined(M25P16_QUADSPI_DEVICE)
static busDevice_t * busDev = NULL;
#else
static QUADSPI_TypeDef * qspi = NULL;
#endif /* !defined(M25P16_QUADSPI_DEVICE) */

static bool isLargeFlash = false;
static uint32_t timeoutAt = 0;

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
#if !defined(M25P16_QUADSPI_DEVICE)
    busTransfer(busDev, NULL, &command, 1);
#else
    quadSpiTransmit1LINE(qspi, command, 0, NULL, 0);
#endif /* !defined(M25P16_QUADSPI_DEVICE) */
}

/**
 * The flash requires this write enable command to be sent before commands that would cause
 * a write like program and erase.
 */
static void m25p16_writeEnable(void)
{
    m25p16_performOneByteCommand(M25P16_INSTRUCTION_WRITE_ENABLE);

    // Assume that we're about to do some writing, so the device is just about to become busy
    couldBeBusy = true;
}

static uint8_t m25p16_readStatus(void)
{
    uint8_t status;
#if !defined(M25P16_QUADSPI_DEVICE)
    uint8_t command[2] = { M25P16_INSTRUCTION_READ_STATUS_REG, 0 };
    uint8_t in[2];

    busTransfer(busDev, in, command, sizeof(command));
    status = in[1];
#else
    quadSpiReceive1LINE(qspi, M25P16_INSTRUCTION_READ_STATUS_REG, 0, &status, 1);
#endif /* !defined(M25P16_QUADSPI_DEVICE) */

    return status;
}

bool m25p16_isReady(void)
{
    // If couldBeBusy is false, don't bother to poll the flash chip for its status
    couldBeBusy = couldBeBusy && ((m25p16_readStatus() & M25P16_STATUS_FLAG_WRITE_IN_PROGRESS) != 0);

    return !couldBeBusy;
}

static void m25p16_setTimeout(uint32_t timeoutMillis)
{
    uint32_t now = millis();
    timeoutAt = now + timeoutMillis;
}

bool m25p16_waitForReady(uint32_t timeoutMillis)
{
    uint32_t timeoutAtUse;
    if (timeoutMillis == 0) {
        timeoutAtUse = timeoutAt;
    }
    else {
        timeoutAtUse = millis() + timeoutMillis;
    }

    while (!m25p16_isReady()) {
        uint32_t now = millis();
        if (now >= timeoutAtUse) {
            return false;
        }
    }

    timeoutAt = 0;
    return true;
}

/**
 * Read chip identification and geometry information (into global `geometry`).
 *
 * Returns true if we get valid ident, false if something bad happened like there is no M25P16.
 */
static bool m25p16_readIdentification(void)
{
    uint8_t in[4];
    uint32_t chipID;

    delay(50); // short delay required after initialisation of SPI device instance.

    /* Just in case transfer fails and writes nothing, so we don't try to verify the ID against random garbage
     * from the stack:
     */
    in[1] = 0;

#if !defined(M25P16_QUADSPI_DEVICE)
    uint8_t out[] = { M25P16_INSTRUCTION_RDID, 0, 0, 0 };

    busTransfer(busDev, in, out, sizeof(out));
#else
    bool status = quadSpiReceive1LINE(qspi, M25P16_INSTRUCTION_RDID, 0, &in[1], sizeof(in) - 1);
    if (!status) {
        return false;
    }
#endif /* !defined(M25P16_QUADSPI_DEVICE) */

    // Manufacturer, memory type, and capacity
    chipID = (in[1] << 16) | (in[2] << 8) | (in[3]);

    geometry.sectors = 0;
    geometry.pagesPerSector = 0;
    geometry.sectorSize = 0;
    geometry.totalSize = 0;

    for(int i = 0; m25p16FlashConfig[i].jedecID != 0; ++i) {
        if(m25p16FlashConfig[i].jedecID == chipID) {
            geometry.sectors = m25p16FlashConfig[i].sectors;
            geometry.pagesPerSector = m25p16FlashConfig[i].pagesPerSector;
        }
    }

    if(geometry.sectors == 0) {
        return false;
    }

    geometry.flashType = FLASH_TYPE_NOR;
    geometry.pageSize = M25P16_PAGESIZE;
    geometry.sectorSize = geometry.pagesPerSector * geometry.pageSize;
    geometry.totalSize = geometry.sectorSize * geometry.sectors;

    if (geometry.totalSize > 16 * 1024 * 1024) {
        isLargeFlash = true;
        m25p16_performOneByteCommand(W25Q256_INSTRUCTION_ENTER_4BYTE_ADDRESS_MODE);
    }

    couldBeBusy = true; // Just for luck we'll assume the chip could be busy even though it isn't specced to be

    return true;
}

/**
 * Initialize the driver, must be called before any other routines.
 *
 * Attempts to detect a connected m25p16. If found, true is returned and device capacity can be fetched with
 * m25p16_getGeometry().
 */
bool m25p16_init(int flashNumToUse)
{
    bool detected;
#if !defined(M25P16_QUADSPI_DEVICE)
    // SPI Mode
    busDev = busDeviceInit(BUSTYPE_SPI, DEVHW_M25P16, flashNumToUse, OWNER_FLASH);
    if (busDev == NULL) {
        return false;
    }

#ifndef M25P16_SPI_SHARED
    busSetSpeed(busDev, BUS_SPEED_FAST);
#endif
#else
    // QUADSPI Mode
    UNUSED(flashNumToUse);
    quadSpiPinConfigure(M25P16_QUADSPI_DEVICE);
    quadSpiInitDevice(M25P16_QUADSPI_DEVICE);

    qspi = quadSpiInstanceByDevice(M25P16_QUADSPI_DEVICE);
    quadSpiSetDivisor(qspi, QUADSPI_CLOCK_INITIALISATION);
#endif /* M25P16_QUADSPI_DEVICE */

    detected = m25p16_readIdentification();

#if defined(M25P16_QUADSPI_DEVICE)
    if (detected) {
        quadSpiSetDivisor(qspi, QUADSPI_CLOCK_ULTRAFAST);
    }
#endif

    return detected;
}

void m25p16_setCommandAddress(uint8_t *buf, uint32_t address, bool useLongAddress)
{
    if (useLongAddress) {
        *buf++ = (address >> 24) & 0xff;
    }

    *buf++ = (address >> 16) & 0xff;
    *buf++ = (address >> 8) & 0xff;
    *buf = address & 0xff;
}

/**
 * Erase a sector full of bytes to all 1's at the given byte offset in the flash chip.
 */
void m25p16_eraseSector(uint32_t address)
{
    if (!m25p16_waitForReady(0)) {
        return;
    }

    m25p16_writeEnable();

#if !defined(M25P16_QUADSPI_DEVICE)
    uint8_t out[5] = { M25P16_INSTRUCTION_SECTOR_ERASE };
    m25p16_setCommandAddress(&out[1], address, isLargeFlash);

    busTransfer(busDev, NULL, out, isLargeFlash ? 5 : 4);
#else
    quadSpiInstructionWithAddress1LINE(qspi, M25P16_INSTRUCTION_SECTOR_ERASE, 0, address, isLargeFlash ? 32 : 24);
#endif /* !defined(M25P16_QUADSPI_DEVICE) */

    m25p16_setTimeout(SECTOR_ERASE_TIMEOUT_MILLIS);
}

void m25p16_eraseCompletely(void)
{
    if (!m25p16_waitForReady(0)) {
        return;
    }

    m25p16_writeEnable();

    m25p16_performOneByteCommand(M25P16_INSTRUCTION_BULK_ERASE);

    m25p16_setTimeout(BULK_ERASE_TIMEOUT_MILLIS);
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
 *
 * If you want to write multiple buffers (whose sum of sizes is still not more than the page size) then you can
 * break this operation up into one beginProgram call, one or more continueProgram calls, and one finishProgram call.
 */
uint32_t m25p16_pageProgram(uint32_t address, const uint8_t *data, int length)
{
    if (!m25p16_waitForReady(0)) {
        // return same address to indicate timeout
        return address;
    }

    m25p16_writeEnable();

#if !defined(M25P16_QUADSPI_DEVICE)
    uint8_t command[5] = { M25P16_INSTRUCTION_PAGE_PROGRAM };

    busTransferDescriptor_t txn[2] = {
        { NULL, command, isLargeFlash ? 5 : 4 },
        { NULL, data, length }
    };

    m25p16_setCommandAddress(&command[1], address, isLargeFlash);

    busTransferMultiple(busDev, txn, 2);
#else
    quadSpiTransmitWithAddress4LINES(qspi, M25P16_INSTRUCTION_QPAGE_PROGRAM, 0, address, isLargeFlash ? 32 : 24, data, length);
#endif /* !defined(M25P16_QUADSPI_DEVICE) */

    m25p16_setTimeout(DEFAULT_TIMEOUT_MILLIS);

    return address + length;
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
    if (!m25p16_waitForReady(0)) {
        return 0;
    }

#if !defined(M25P16_QUADSPI_DEVICE)
    uint8_t command[5] = { M25P16_INSTRUCTION_READ_BYTES };

    busTransferDescriptor_t txn[2] = {
        { NULL, command, isLargeFlash ? 5 : 4 },
        { buffer, NULL, length }
    };

    m25p16_setCommandAddress(&command[1], address, isLargeFlash);

    busTransferMultiple(busDev, txn, 2);
#else
    quadSpiReceiveWithAddress4LINES(qspi, M25P16_INSTRUCTION_QUAD_READ, M25P16_FAST_READ_DUMMY_CYCLES,
                                    address, isLargeFlash ? 32 : 24, buffer, length);
#endif /* !defined(M25P16_QUADSPI_DEVICE) */

    return length;
}

/**
 * Fetch information about the detected flash chip layout.
 *
 * Can be called before calling m25p16_init() (the result would have totalSize = 0).
 */
const flashGeometry_t* m25p16_getGeometry(void)
{
    return &geometry;
}

#endif

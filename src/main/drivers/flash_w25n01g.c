/*
 * This file is part of iNav.
 *
 * iNav are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * iNav are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
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

#include "platform.h"

#ifdef USE_FLASH_W25N01G

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/time.h"

#include "flash_w25n01g.h"
     
// Device size parameters
#define W25N01G_PAGE_SIZE       2048
#define W25N01G_PAGES_PER_BLOCK 64
#define W25N01G_BLOCKS_PER_DIE  1024

// BB replacement area
#define W25N01G_BB_MARKER_BLOCKS      1
#define W25N01G_BB_REPLACEMENT_BLOCKS 20
#define W25N01G_BB_MANAGEMENT_BLOCKS  (W25N01G_BB_REPLACEMENT_BLOCKS + W25N01G_BB_MARKER_BLOCKS)

// blocks are zero-based index
#define W25N01G_BB_REPLACEMENT_START_BLOCK (W25N01G_BLOCKS_PER_DIE - W25N01G_BB_REPLACEMENT_BLOCKS)
#define W25N01G_BB_MANAGEMENT_START_BLOCK  (W25N01G_BLOCKS_PER_DIE - W25N01G_BB_MANAGEMENT_BLOCKS)
#define W25N01G_BB_MARKER_BLOCK            (W25N01G_BB_REPLACEMENT_START_BLOCK - W25N01G_BB_MARKER_BLOCKS)

// Instructions
#define W25N01G_INSTRUCTION_RDID                       0x9F
#define W25N01G_INSTRUCTION_DEVICE_RESET               0xFF
#define W25N01G_INSTRUCTION_READ_STATUS_REG            0x05
#define W25N01G_INSTRUCTION_READ_STATUS_ALTERNATE_REG  0x0F
#define W25N01G_INSTRUCTION_WRITE_STATUS_REG           0x01
#define W25N01G_INSTRUCTION_WRITE_STATUS_ALTERNATE_REG 0x1F
#define W25N01G_INSTRUCTION_WRITE_ENABLE               0x06
#define W25N01G_INSTRUCTION_DIE_SELECT                 0xC2
#define W25N01G_INSTRUCTION_BLOCK_ERASE                0xD8
#define W25N01G_INSTRUCTION_READ_BBM_LUT               0xA5
#define W25N01G_INSTRUCTION_BB_MANAGEMENT              0xA1
#define W25N01G_INSTRUCTION_PROGRAM_DATA_LOAD          0x02
#define W25N01G_INSTRUCTION_RANDOM_PROGRAM_DATA_LOAD   0x84
#define W25N01G_INSTRUCTION_PROGRAM_EXECUTE            0x10
#define W25N01G_INSTRUCTION_PAGE_DATA_READ             0x13
#define W25N01G_INSTRUCTION_READ_DATA                  0x03
#define W25N01G_INSTRUCTION_FAST_READ                  0x1B
#define W25N01G_INSTRUCTION_FAST_READ_QUAD_OUTPUT      0x6B

// Config/status register addresses
#define W25N01G_PROT_REG 0xA0
#define W25N01G_CONF_REG 0xB0
#define W25N01G_STAT_REG 0xC0

// Bits in config/status register 1 (W25N01G_PROT_REG)
#define W25N01G_PROT_CLEAR                (0)
#define W25N01G_PROT_SRP1_ENABLE          (1 << 0)
#define W25N01G_PROT_WP_E_ENABLE          (1 << 1)
#define W25N01G_PROT_TB_ENABLE            (1 << 2)
#define W25N01G_PROT_PB0_ENABLE           (1 << 3)
#define W25N01G_PROT_PB1_ENABLE           (1 << 4)
#define W25N01G_PROT_PB2_ENABLE           (1 << 5)
#define W25N01G_PROT_PB3_ENABLE           (1 << 6)
#define W25N01G_PROT_SRP2_ENABLE          (1 << 7)

// Bits in config/status register 2 (W25N01G_CONF_REG)
#define W25N01G_CONFIG_ECC_ENABLE         (1 << 4)
#define W25N01G_CONFIG_BUFFER_READ_MODE   (1 << 3)

// Bits in config/status register 3 (W25N01G_STATREG)
#define W25N01G_STATUS_BBM_LUT_FULL       (1 << 6)
#define W25N01G_STATUS_FLAG_ECC_POS       4
#define W25N01G_STATUS_FLAG_ECC_MASK      ((1 << 5)|(1 << 4))
#define W25N01G_STATUS_FLAG_ECC(status)   (((status) & W25N01G_STATUS_FLAG_ECC_MASK) >> 4)
#define W25N01G_STATUS_PROGRAM_FAIL       (1 << 3)
#define W25N01G_STATUS_ERASE_FAIL         (1 << 2)
#define W25N01G_STATUS_FLAG_WRITE_ENABLED (1 << 1)
#define W25N01G_STATUS_FLAG_BUSY          (1 << 0)
#define W25N01G_BBLUT_TABLE_ENTRY_COUNT    20
#define W25N01G_BBLUT_TABLE_ENTRY_SIZE     4  // in bytes

// Bits in LBA for BB LUT
#define W25N01G_BBLUT_STATUS_ENABLED (1 << 15)
#define W25N01G_BBLUT_STATUS_INVALID (1 << 14)
#define W25N01G_BBLUT_STATUS_MASK    (W25N01G_BBLUT_STATUS_ENABLED | W25N01G_BBLUT_STATUS_INVALID)

// Some useful defs and macros
#define W25N01G_LINEAR_TO_COLUMN(laddr) ((laddr) % W25N01G_PAGE_SIZE)
#define W25N01G_LINEAR_TO_PAGE(laddr) ((laddr) / W25N01G_PAGE_SIZE)
#define W25N01G_LINEAR_TO_BLOCK(laddr) (W25N01G_LINEAR_TO_PAGE(laddr) / W25N01G_PAGES_PER_BLOCK)
#define W25N01G_BLOCK_TO_PAGE(block) ((block) * W25N01G_PAGES_PER_BLOCK)
#define W25N01G_BLOCK_TO_LINEAR(block) (W25N01G_BLOCK_TO_PAGE(block) * W25N01G_PAGE_SIZE)

// IMPORTANT: Timeout values are currently required to be set to the highest value required by any of the supported flash chips by this driver
// The timeout values (2ms minimum to avoid 1 tick advance in consecutive calls to millis).
#define W25N01G_TIMEOUT_PAGE_READ_MS        2   // tREmax = 60us (ECC enabled)
#define W25N01G_TIMEOUT_PAGE_PROGRAM_MS     2   // tPPmax = 700us
#define W25N01G_TIMEOUT_BLOCK_ERASE_MS      15  // tBEmax = 10ms
#define W25N01G_TIMEOUT_RESET_MS            500 // tRSTmax = 500ms

// Sizes (in bits)
#define W28N01G_STATUS_REGISTER_SIZE        8
#define W28N01G_STATUS_PAGE_ADDRESS_SIZE    16
#define W28N01G_STATUS_COLUMN_ADDRESS_SIZE  16

// JEDEC ID
#define JEDEC_ID_WINBOND_W25N01GV 0xEFAA21

static busDevice_t *busDev = NULL;
static flashGeometry_t geometry;

/*
 * Whether we've performed an action that could have made the device busy for writes.
 *
 * This allows us to avoid polling for writable status when it is definitely ready already.
 */
static bool couldBeBusy = false;

static timeMs_t timeoutAt = 0;

static bool w25n01g_waitForReadyInternal(void);

static void w25n01g_setTimeout(timeMs_t timeoutMillis)
{
    timeMs_t now = millis();
    timeoutAt = now + timeoutMillis;
    couldBeBusy = true;
}

/**
 * Send the given command byte to the device.
 */
static void w25n01g_performOneByteCommand(uint8_t command)
{
    busTransfer(busDev, NULL, &command, 1);
}

static void w25n01g_performCommandWithPageAddress(uint8_t command, uint32_t pageAddress)
{
    uint8_t cmd[4] = { command, 0, (pageAddress >> 8) & 0xff, (pageAddress >> 0) & 0xff};
    busTransfer(busDev, NULL, cmd, sizeof(cmd));
}

static uint8_t w25n01g_readRegister(uint8_t reg)
{
    uint8_t command[3] = { W25N01G_INSTRUCTION_READ_STATUS_REG, reg, 0 };
    uint8_t in[3];

    busTransfer(busDev, in, command, sizeof(command));

    return in[2];
}

static void w25n01g_writeRegister(uint8_t reg, uint8_t data)
{
    uint8_t cmd[3] = { W25N01G_INSTRUCTION_WRITE_STATUS_REG, reg, data };
    busTransfer(busDev, NULL, cmd, sizeof(cmd));
}

static void w25n01g_deviceReset(void)
{
    w25n01g_performOneByteCommand(W25N01G_INSTRUCTION_DEVICE_RESET);
    w25n01g_setTimeout(W25N01G_TIMEOUT_RESET_MS);
    w25n01g_waitForReadyInternal();
    // Protection for upper 1/32 (BP[3:0] = 0101, TB=0), WP-E on; to protect bad block replacement area
    // DON'T DO THIS. This will prevent writes through the bblut as well.
    // w25n01g_writeRegister(busdev, W25N01G_PROT_REG, W25N01G_PROT_PB0_ENABLE|W25N01G_PROT_PB2_ENABLE|W25N01G_PROT_WP_E_ENABLE);
    // No protection, WP-E off, WP-E prevents use of IO2
    w25n01g_writeRegister(W25N01G_PROT_REG, W25N01G_PROT_CLEAR);
    // Buffered read mode (BUF = 1), ECC enabled (ECC = 1)
    w25n01g_writeRegister(W25N01G_CONF_REG, W25N01G_CONFIG_ECC_ENABLE | W25N01G_CONFIG_BUFFER_READ_MODE);
}

bool w25n01g_isReady(void)
{
    uint8_t status = w25n01g_readRegister(W25N01G_STAT_REG);

    // If couldBeBusy is false, don't bother to poll the flash chip for its status
    couldBeBusy = couldBeBusy && ((status & W25N01G_STATUS_FLAG_BUSY) != 0);

    return !couldBeBusy;
}

static bool w25n01g_waitForReadyInternal(void)
{
    while (!w25n01g_isReady()) {
        timeMs_t now = millis();
        if (cmp32(now, timeoutAt) >= 0) {
            return false;
        }
    }
    timeoutAt = 0;
    return true;
}

bool w25n01g_waitForReady(timeMs_t timeoutMillis)
{
    w25n01g_setTimeout(timeoutMillis);
    return w25n01g_waitForReadyInternal();
}

/**
 * The flash requires this write enable command to be sent before commands that would cause
 * a write like program and erase.
 */
static void w25n01g_writeEnable(void)
{
    w25n01g_performOneByteCommand(W25N01G_INSTRUCTION_WRITE_ENABLE);

    // Assume that we're about to do some writing, so the device is just about to become busy
    couldBeBusy = true;
}

bool w25n01g_detect(uint32_t chipID)
{
    switch (chipID) {
    case JEDEC_ID_WINBOND_W25N01GV:
        geometry.sectors = 1024;      // Blocks
        geometry.pagesPerSector = 64; // Pages/Blocks
        geometry.pageSize = 2048;
        break;

    default:
        // Unsupported chip
        geometry.sectors = 0;
        geometry.pagesPerSector = 0;
        geometry.sectorSize = 0;
        geometry.totalSize = 0;
        return false;
    }

    geometry.flashType = FLASH_TYPE_NAND;
    geometry.sectorSize = geometry.pagesPerSector * geometry.pageSize;
    geometry.totalSize = geometry.sectorSize * geometry.sectors;
    
    /*flashPartitionSet(FLASH_PARTITION_TYPE_BADBLOCK_MANAGEMENT,
            W25N01G_BB_MANAGEMENT_START_BLOCK,
            W25N01G_BB_MANAGEMENT_START_BLOCK + W25N01G_BB_MANAGEMENT_BLOCKS - 1);*/

    couldBeBusy = true; // Just for luck we'll assume the chip could be busy even though it isn't specced to be

    w25n01g_deviceReset();

    // Upper 4MB (32 blocks * 128KB/block) will be used for bad block replacement area.
    // Blocks in this area are only written through bad block LUT,
    // and factory written bad block marker in unused blocks are retained.
    // When a replacement block is required,
    // (1) "Read BB LUT" command is used to obtain the last block mapped,
    // (2) blocks after the last block is scanned for a good block,
    // (3) the first good block is used for replacement, and the BB LUT is updated.
    // There are only 20 BB LUT entries, and there are 32 replacement blocks.
    // There will be a least chance of running out of replacement blocks.
    // If it ever run out, the device becomes unusable.

    return true;
}

/**
 * Erase a sector full of bytes to all 1's at the given byte offset in the flash chip.
 */
void w25n01g_eraseSector(uint32_t address)
{
    w25n01g_waitForReadyInternal();
    w25n01g_writeEnable();
    w25n01g_performCommandWithPageAddress(W25N01G_INSTRUCTION_BLOCK_ERASE, W25N01G_LINEAR_TO_PAGE(address));
    w25n01g_setTimeout(W25N01G_TIMEOUT_BLOCK_ERASE_MS);
}

// W25N01G does not support full chip erase.
// Call eraseSector repeatedly.
void w25n01g_eraseCompletely(void)
{
    for (uint32_t block = 0; block < geometry.sectors; block++) {
        w25n01g_eraseSector(W25N01G_BLOCK_TO_LINEAR(block));
    }
}

static void w25n01g_programDataLoad(uint16_t columnAddress, const uint8_t *data, int length)
{
    w25n01g_waitForReadyInternal();

    uint8_t cmd[3] = {W25N01G_INSTRUCTION_PROGRAM_DATA_LOAD, columnAddress >> 8, columnAddress & 0xff};

    busTransferDescriptor_t transferDescr[] = {{.length = sizeof(cmd), .rxBuf = NULL, .txBuf = cmd}, {.length = length, .rxBuf = NULL, .txBuf = (uint8_t *)data}};
    busTransferMultiple(busDev, transferDescr, ARRAYLEN(transferDescr);

    w25n01g_setTimeout(W25N01G_TIMEOUT_PAGE_PROGRAM_MS);
}

static void w25n01g_randomProgramDataLoad(uint16_t columnAddress, const uint8_t *data, int length)
{
    w25n01g_waitForReadyInternal();

    uint8_t cmd[3] = {W25N01G_INSTRUCTION_RANDOM_PROGRAM_DATA_LOAD, columnAddress >> 8, columnAddress & 0xff};

    busTransferDescriptor_t transferDescr[] = {{.length = sizeof(cmd), .rxBuf = NULL, .txBuf = cmd}, {.length = length, .rxBuf = NULL, .txBuf = (uint8_t *)data}};
    busTransferMultiple(busDev, transferDescr, ARRAYLEN(transferDescr);

    w25n01g_setTimeout(W25N01G_TIMEOUT_PAGE_PROGRAM_MS);
}

static void w25n01g_programExecute(uint32_t pageAddress)
{
    w25n01g_waitForReadyInternal();
    w25n01g_performCommandWithPageAddress(W25N01G_INSTRUCTION_PROGRAM_EXECUTE, pageAddress);
    w25n01g_setTimeout(W25N01G_TIMEOUT_PAGE_PROGRAM_MS);
}

// Writes are done in three steps:
// (1) Load internal data buffer with data to write
//     - We use "Random Load Program Data", as "Load Program Data" resets unused data bytes in the buffer to 0xff.
//     - Each "Random Load Program Data" instruction must be accompanied by at least a single data.
//     - Each "Random Load Program Data" instruction terminates at the rising of CS.
// (2) Enable write
// (3) Issue "Execute Program"
//
/*
flashfs page program behavior
- Single program never crosses page boundary.
- Except for this characteristic, it program arbitral size.
- Write address is, naturally, not a page boundary.
To cope with this behavior.
pageProgramBegin:
If buffer is dirty and programLoadAddress != address, then the last page is a partial write;
issue PAGE_PROGRAM_EXECUTE to flash buffer contents, clear dirty and record the address as programLoadAddress and programStartAddress.
Else do nothing.
pageProgramContinue:
Mark buffer as dirty.
If programLoadAddress is on page boundary, then issue PROGRAM_LOAD_DATA, else issue RANDOM_PROGRAM_LOAD_DATA.
Update programLoadAddress.
Optionally observe the programLoadAddress, and if it's on page boundary, issue PAGE_PROGRAM_EXECUTE.
pageProgramFinish:
Observe programLoadAddress. If it's on page boundary, issue PAGE_PROGRAM_EXECUTE and clear dirty, else just return.
If pageProgramContinue observes the page boundary, then do nothing(?).
*/
bool bufferDirty = false;
bool isProgramming = false;
static uint32_t programStartAddress;
static uint32_t programLoadAddress;
static uint32_t currentPage = UINT32_MAX;

void w25n01g_pageProgramBegin(uint32_t address)
{
    if (bufferDirty) {
        if (address != programLoadAddress) {
            w25n01g_waitForReadyInternal();
            isProgramming = false;
            w25n01g_writeEnable();
            w25n01g_programExecute(W25N01G_LINEAR_TO_PAGE(programStartAddress));
            bufferDirty = false;
            isProgramming = true;
        }
    } else {
        programStartAddress = programLoadAddress = address;
    }
}

void w25n01g_pageProgramContinue(const uint8_t *data, int length)
{
    // Check for page boundary overrun
    w25n01g_waitForReadyInternal();
    w25n01g_writeEnable();
    isProgramming = false;
    if (!bufferDirty) {
        w25n01g_programDataLoad(W25N01G_LINEAR_TO_COLUMN(programLoadAddress), (uint8_t *)data, length);
    } else {
        w25n01g_randomProgramDataLoad(W25N01G_LINEAR_TO_COLUMN(programLoadAddress), (uint8_t *)data, length);
    }
    // XXX Test if write enable is reset after each data loading.
    bufferDirty = true;
    programLoadAddress += length;
}

void w25n01g_pageProgramFinish(void)
{
    if (bufferDirty && W25N01G_LINEAR_TO_COLUMN(programLoadAddress) == 0) {
        currentPage = W25N01G_LINEAR_TO_PAGE(programStartAddress); // reset page to the page being written
        w25n01g_programExecute(W25N01G_LINEAR_TO_PAGE(programStartAddress));
        bufferDirty = false;
        isProgramming = true;
        programStartAddress = programLoadAddress;
    }
}

/*
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
uint32_t w25n01g_pageProgram(uint32_t address, const uint8_t *data, int length)
{
    w25n01g_pageProgramBegin(address);
    w25n01g_pageProgramContinue((uint8_t *)data, length);
    w25n01g_pageProgramFinish();

    return address + length;
}

void w25n01g_flush(void)
{
    if (bufferDirty) {
        currentPage = W25N01G_LINEAR_TO_PAGE(programStartAddress); // reset page to the page being written
        w25n01g_programExecute(W25N01G_LINEAR_TO_PAGE(programStartAddress));
        bufferDirty = false;
        isProgramming = true;
    } else {
        isProgramming = false;
    }
}

void w25n01g_addError(uint32_t address, uint8_t code)
{
    UNUSED(address);
    UNUSED(code);
}

/*
 * Read `length` bytes into the provided `buffer` from the flash starting from the given `address` (which need not lie
 * on a page boundary).
 *
 * Waits up to W25N01G_TIMEOUT_PAGE_READ_MS milliseconds for the flash to become ready before reading.
 *
 * The number of bytes actually read is returned, which can be zero if an error or timeout occurred.
 */
// Continuous read mode (BUF = 0):
// (1) "Page Data Read" command is executed for the page pointed by address
// (2) "Read Data" command is executed for bytes not requested and data are discarded
// (3) "Read Data" command is executed and data are stored directly into caller's buffer
//
// Buffered read mode (BUF = 1), non-read ahead
// (1) If currentBufferPage != requested page, then issue PAGE_DATA_READ on requested page.
// (2) Compute transferLength as smaller of remaining length and requested length.
// (3) Issue READ_DATA on column address.
// (4) Return transferLength.
int w25n01g_readBytes(uint32_t address, uint8_t *buffer, int length)
{
    uint32_t targetPage = W25N01G_LINEAR_TO_PAGE(address);

    if (currentPage != targetPage) {
        if (!w25n01g_waitForReadyInternal()) {
            return 0;
        }
        currentPage = UINT32_MAX;
        w25n01g_performCommandWithPageAddress(W25N01G_INSTRUCTION_PAGE_DATA_READ, targetPage);
        if (!w25n01g_waitForReady(W25N01G_TIMEOUT_PAGE_READ_MS)) {
            return 0;
        }
        currentPage = targetPage;
    }

    uint16_t column = W25N01G_LINEAR_TO_COLUMN(address);
    uint16_t transferLength;

    if (length > W25N01G_PAGE_SIZE - column) {
        transferLength = W25N01G_PAGE_SIZE - column;
    } else {
        transferLength = length;
    }

    const uint8_t cmd[4] = {W25N01G_INSTRUCTION_READ_DATA, (column >> 8) & 0xff, (column >> 0) & 0xff, 0};

    busTransferDescriptor_t readDescr[] = {{.length = sizeof(cmd), .rxBuf = NULL, .txBuf = cmd}, {.length = transferLength, .rxBuf = buffer, .txBuf = NULL}};
    busTransferMultiple(busDev, readDescr, ARRAYLEN(readDescr);

    if (!w25n01g_waitForReady(W25N01G_TIMEOUT_PAGE_READ_MS)) {
        return 0;
    }

    // Check ECC
    uint8_t statReg = w25n01g_readRegister(W25N01G_STAT_REG);
    uint8_t eccCode = W25N01G_STATUS_FLAG_ECC(statReg);
    switch (eccCode) {
    case 0: // Successful read, no ECC correction
        break;

    case 1: // Successful read with ECC correction
    case 2: // Uncorrectable ECC in a single page
    case 3: // Uncorrectable ECC in multiple pages
        w25n01g_addError(address, eccCode);
        w25n01g_deviceReset();
        break;
    }

    return transferLength;
}

int w25n01g_readExtensionBytes(uint32_t address, uint8_t *buffer, int length)
{
    if (!w25n01g_waitForReadyInternal()) {
        return 0;
    }

    w25n01g_performCommandWithPageAddress(W25N01G_INSTRUCTION_PAGE_DATA_READ, W25N01G_LINEAR_TO_PAGE(address));

    uint32_t column = 2048;

    uint8_t cmd[4];
    cmd[0] = W25N01G_INSTRUCTION_READ_DATA;
    cmd[1] = (column >> 8) & 0xff;
    cmd[2] = (column >> 0) & 0xff;
    cmd[3] = 0;

    busTransferDescriptor_t readDescr[] = {{.length = sizeof(cmd), .rxBuf = NULL, .txBuf = cmd}, {.length = length, .rxBuf = buffer, .txBuf = NULL}};
    busTransferMultiple(busDev, readDescr, ARRAYLEN(readDescr);

    w25n01g_setTimeout(W25N01G_TIMEOUT_PAGE_READ_MS);

    return length;
}

/**
 * Fetch information about the detected flash chip layout.
 *
 * Can be called before calling w25n01g_init() (the result would have totalSize = 0).
 */
const flashGeometry_t* w25n01g_getGeometry(void)
{
    return &geometry;
}

bool w25n01g_init(int flashNumToUse) 
{
    busDev = busDeviceInit(BUSTYPE_SPI, DEVHW_W25N01G, flashNumToUse, OWNER_FLASH);
    if (busDev == NULL) {
        return false;
    }
    
    uint8_t in[4] = { 0 };
    uint32_t chipID;

    delay(50); // short delay required after initialisation of SPI device instance.

    busReadBuf(busDev, W25N01G_INSTRUCTION_RDID, in, sizeof(in));

    chipID = (in[1] << 16) | (in[2] << 8) | (in[3]);

    return w25n01g_detect(chipID);
}

#endif
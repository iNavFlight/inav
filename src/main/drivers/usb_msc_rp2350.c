/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * RP2350 USB mass storage mode: the SD card as a disk, on the TinyUSB MSC class.
 *
 * The board boots into this mode after the CLI command `msc`. It then shows up
 * as a mass storage device only (see the descriptors in
 * serial_usb_vcp_rp2350.c) and stays in mscWaitForButton() until power is
 * removed, the scheduler never starts.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "platform.h"

#if defined(USE_USB_MSC)

#include "common/utils.h"

#include "drivers/light_led.h"
#include "drivers/sdcard/sdcard.h"
#include "drivers/time.h"
#include "drivers/usb_msc.h"

#include "tusb.h"

#define MSC_BLOCK_SIZE          512
#define MSC_CARD_INIT_MS        2000
#define MSC_CARD_REINIT_MS      500
#define MSC_BLOCK_TIMEOUT_MS    1000
// The card driver restarts the card after a failed block, which makes another attempt worth it
#define MSC_BLOCK_ATTEMPTS      3

static bool mscWaitForCard(uint32_t timeoutMs)
{
    const uint32_t start = millis();
    while (!sdcard_poll()) {
        if (millis() - start > timeoutMs) {
            return false;
        }
    }
    return true;
}

void mscInit(void)
{
}

bool mscCheckButton(void)
{
    return false;
}

uint8_t mscStart(void)
{
    LED0_ON;

    sdcardInsertionDetectInit();
    sdcard_init();

    // Bring the card up before the host starts asking. A missing card is not
    // an error, the host is then told there is no medium.
    mscWaitForCard(MSC_CARD_INIT_MS);

    // The MSC callbacks block on the card for milliseconds, too long for the
    // 1 ms timer interrupt: from now on tud_task() runs in mscWaitForButton()
    rp2350UsbBusy = 1;

    // systemInit() kept the device off the bus until now
    tud_connect();

    return 0;
}

void mscWaitForButton(void)
{
    uint32_t lastInitMs = millis();

    while (true) {
        tud_task();

        // The driver gives a card up after a few failed attempts, so start it
        // again: this also picks up a card inserted later.
        const uint32_t now = millis();
        if (!sdcard_poll() && !sdcard_isFunctional() && now - lastInitMs > MSC_CARD_REINIT_MS) {
            lastInitMs = now;
            sdcard_init();
        }
    }
}

// ── TinyUSB MSC callbacks, called from tud_task() ────────────────────────────

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    UNUSED(lun);
    // Fixed width fields, padded with spaces and not terminated
    memcpy(vendor_id, "INAV    ", 8);
    memcpy(product_id, "SD card         ", 16);
    memcpy(product_rev, "1.0 ", 4);
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    UNUSED(lun);
    return sdcard_poll();
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    UNUSED(lun);
    // Zero blocks without a card, TinyUSB then answers "medium not present"
    *block_count = sdcard_poll() ? sdcard_getMetadata()->numBlocks : 0;
    *block_size = MSC_BLOCK_SIZE;
}

// The card driver reports the outcome of a block only to a callback: the
// buffer comes back on success, NULL on failure. No callback at all, the card
// was restarted while the block was in progress, counts as a failure.
static volatile bool mscBlockFailed;

static void mscBlockDone(sdcardBlockOperation_e operation, uint32_t blockIndex, uint8_t *buffer, uint32_t callbackData)
{
    UNUSED(operation);
    UNUSED(blockIndex);
    UNUSED(callbackData);
    mscBlockFailed = (buffer == NULL);
}

static bool mscReadBlock(uint32_t lba, uint8_t *data)
{
    for (int attempt = 0; attempt < MSC_BLOCK_ATTEMPTS; attempt++) {
        const uint32_t start = millis();
        bool started;
        mscBlockFailed = true;
        while (!(started = sdcard_readBlock(lba, data, mscBlockDone, 0)) && millis() - start < MSC_BLOCK_TIMEOUT_MS) {
            sdcard_poll();
        }
        if (started && mscWaitForCard(MSC_BLOCK_TIMEOUT_MS) && !mscBlockFailed) {
            return true;
        }
    }
    return false;
}

static bool mscWriteBlock(uint32_t lba, uint8_t *data)
{
    for (int attempt = 0; attempt < MSC_BLOCK_ATTEMPTS; attempt++) {
        const uint32_t start = millis();
        bool started;
        mscBlockFailed = true;
        while (!(started = (sdcard_writeBlock(lba, data, mscBlockDone, 0) == SDCARD_OPERATION_IN_PROGRESS))
                && millis() - start < MSC_BLOCK_TIMEOUT_MS) {
            sdcard_poll();
        }
        if (started && mscWaitForCard(MSC_BLOCK_TIMEOUT_MS) && !mscBlockFailed) {
            return true;
        }
    }
    return false;
}

// CFG_TUD_MSC_EP_BUFSIZE is a multiple of the block size, so offset is always
// zero and bufsize a whole number of blocks
int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    UNUSED(lun);
    UNUSED(offset);

    uint8_t *data = buffer;
    for (uint32_t i = 0; i < bufsize / MSC_BLOCK_SIZE; i++) {
        if (!mscReadBlock(lba + i, data + i * MSC_BLOCK_SIZE)) {
            return -1;
        }
    }
    return bufsize;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    UNUSED(lun);
    UNUSED(offset);

    for (uint32_t i = 0; i < bufsize / MSC_BLOCK_SIZE; i++) {
        if (!mscWriteBlock(lba + i, buffer + i * MSC_BLOCK_SIZE)) {
            return -1;
        }
    }
    return bufsize;
}

// Every SCSI command the TinyUSB MSC class does not answer by itself
int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    UNUSED(scsi_cmd);
    UNUSED(buffer);
    UNUSED(bufsize);

    // Invalid command operation code
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
    return -1;
}

#endif

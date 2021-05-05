/*
 * This file is part of INAV, Cleanflight and Betaflight.
 *
 * INAV, Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
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

#include "build/debug.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/bus_spi.h"

#include "drivers/sdcard/sdcard.h"
#include "drivers/sdcard/sdcard_impl.h"
#include "drivers/sdcard/sdcard_standard.h"

#include "scheduler/protothreads.h"

#ifdef USE_SDCARD_SPI

#define SDCARD_INIT_NUM_DUMMY_BYTES                 10
#define SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY     8
// Chosen so that CMD8 will have the same CRC as CMD0:
#define SDCARD_IF_COND_CHECK_PATTERN                0xAB

/* Break up 512-byte SD card sectors into chunks of this size to reduce the peak overhead per call to sdcard_poll(). */
#define SDCARD_BLOCK_CHUNK_SIZE     128

#ifndef SDCARD_BUS_SPEED
#define SDCARD_BUS_SPEED            BUS_SPEED_STANDARD
#endif

static void sdcardSpi_select(void)
{
    busSelectDevice(sdcard.dev);
}

static void sdcardSpi_deselect(void)
{
    // As per the SD-card spec, give the card 8 dummy clocks so it can finish its operation
    //spiTransferByte(SDCARD_SPI_INSTANCE, 0xFF);

    while (busIsBusy(sdcard.dev)) { __NOP(); }

    busDeselectDevice(sdcard.dev);
}


/**
 * Returns true if the card has already been, or is currently, initializing and hasn't encountered enough errors to
 * trip our error threshold and be disabled (i.e. our card is in and working!)
 */
bool sdcardSpi_isFunctional(void)
{
    return sdcard.state != SDCARD_STATE_NOT_PRESENT;
}

/**
 * Handle a failure of an SD card operation by resetting the card back to its initialization phase.
 *
 * Increments the failure counter, and when the failure threshold is reached, disables the card until
 * the next call to sdcard_init().
 */
static void sdcardSpi_reset(void)
{
    if (!sdcard_isInserted()) {
        sdcard.state = SDCARD_STATE_NOT_PRESENT;
        return;
    }

    if (sdcard.state >= SDCARD_STATE_READY) {
        busSetSpeed(sdcard.dev, BUS_SPEED_INITIALIZATION);
    }

    sdcard.failureCount++;
    if (sdcard.failureCount >= SDCARD_MAX_CONSECUTIVE_FAILURES) {
        sdcard.state = SDCARD_STATE_NOT_PRESENT;
    } else {
        sdcard.operationStartTime = millis();
        sdcard.state = SDCARD_STATE_RESET;
    }
}

/**
 * The SD card spec requires 8 clock cycles to be sent by us on the bus after most commands so it can finish its
 * processing of that command. The easiest way for us to do this is to just wait for the bus to become idle before
 * we transmit a command, sending at least 8-bits onto the bus when we do so.
 */
static bool sdcardSpi_waitForIdle(int maxBytesToWait)
{
    while (maxBytesToWait > 0) {
        uint8_t response;

        busTransfer(sdcard.dev, &response, NULL, 1);

        if (response == 0xFF) {
            return true;
        }

        maxBytesToWait--;
    }

    return false;
}

/**
 * Wait for up to maxDelay 0xFF idle bytes to arrive from the card, returning the first non-idle byte found.
 *
 * Returns 0xFF on failure.
 */
static uint8_t sdcardSpi_waitForNonIdleByte(int maxDelay)
{
    for (int i = 0; i <= maxDelay; i++) {   // <= so we can wait for maxDelay '0xFF' bytes before reading a response byte afterwards
        uint8_t response;

        busTransfer(sdcard.dev, &response, NULL, 1);

        if (response != 0xFF) {
            return response;
        }
    }

    return 0xFF;
}

/**
 * Waits up to SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY bytes for the card to become ready, send a command to the card
 * with the given argument, waits up to SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY bytes for a reply, and returns the
 * first non-0xFF byte of the reply.
 *
 * You must select the card first with sdcardSpi_select() and deselect it afterwards with sdcardSpi_deselect().
 *
 * Upon failure, 0xFF is returned.
 */
static uint8_t sdcardSpi_sendCommand(uint8_t commandCode, uint32_t commandArgument)
{
    uint8_t command[6] = {
        0x40 | commandCode,
        commandArgument >> 24,
        commandArgument >> 16,
        commandArgument >> 8,
        commandArgument,
        0x95 /* Static CRC. This CRC is valid for CMD0 with a 0 argument, and CMD8 with 0x1AB argument, which are the only
        commands that require a CRC */
    };

    // Go ahead and send the command even if the card isn't idle if this is the reset command
    if (!sdcardSpi_waitForIdle(SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY) && commandCode != SDCARD_COMMAND_GO_IDLE_STATE)
        return 0xFF;

    busTransfer(sdcard.dev, NULL, command, sizeof(command));

    /*
     * The card can take up to SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY bytes to send the response, in the meantime
     * it'll transmit 0xFF filler bytes.
     */
    return sdcardSpi_waitForNonIdleByte(SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY);
}

static uint8_t sdcardSpi_sendAppCommand(uint8_t commandCode, uint32_t commandArgument)
{
    sdcardSpi_sendCommand(SDCARD_COMMAND_APP_CMD, 0);
    return sdcardSpi_sendCommand(commandCode, commandArgument);
}

/**
 * Sends an IF_COND message to the card to check its version and validate its voltage requirements. Sets the global
 * sdCardVersion with the detected version (0, 1, or 2) and returns true if the card is compatible.
 */
static bool sdcardSpi_validateInterfaceCondition(void)
{
    uint8_t ifCondReply[4];

    sdcard.version = 0;

    sdcardSpi_select();

    uint8_t status = sdcardSpi_sendCommand(SDCARD_COMMAND_SEND_IF_COND, (SDCARD_VOLTAGE_ACCEPTED_2_7_to_3_6 << 8) | SDCARD_IF_COND_CHECK_PATTERN);

    // Don't deselect the card right away, because we'll want to read the rest of its reply if it's a V2 card

    if (status == (SDCARD_R1_STATUS_BIT_ILLEGAL_COMMAND | SDCARD_R1_STATUS_BIT_IDLE)) {
        // V1 cards don't support this command
        sdcard.version = 1;
    } else if (status == SDCARD_R1_STATUS_BIT_IDLE) {
        busTransfer(sdcard.dev, ifCondReply, NULL, sizeof(ifCondReply));

        /*
         * We don't bother to validate the SDCard's operating voltage range since the spec requires it to accept our
         * 3.3V, but do check that it echoed back our check pattern properly.
         */
        if (ifCondReply[3] == SDCARD_IF_COND_CHECK_PATTERN) {
            sdcard.version = 2;
        }
    }

    sdcardSpi_deselect();

    return sdcard.version > 0;
}

static bool sdcardSpi_readOCRRegister(uint32_t *result)
{
    uint8_t response[4];

    sdcardSpi_select();

    uint8_t status = sdcardSpi_sendCommand(SDCARD_COMMAND_READ_OCR, 0);

    busTransfer(sdcard.dev, response, NULL, sizeof(response));

    sdcardSpi_deselect();

    if (status == 0) {
        *result = (response[0] << 24) | (response[1] << 16) | (response[2] << 8) | response[3];
        return true;
    } else {
        return false;
    }
}

typedef enum {
    SDCARD_RECEIVE_SUCCESS,
    SDCARD_RECEIVE_BLOCK_IN_PROGRESS,
    SDCARD_RECEIVE_ERROR,
} sdcardReceiveBlockStatus_e;

/**
 * Attempt to receive a data block from the SD card.
 *
 * Return true on success, otherwise the card has not responded yet and you should retry later.
 */
static sdcardReceiveBlockStatus_e sdcardSpi_receiveDataBlock(uint8_t *buffer, int count)
{
    uint8_t dataToken = sdcardSpi_waitForNonIdleByte(8);

    if (dataToken == 0xFF) {
        return SDCARD_RECEIVE_BLOCK_IN_PROGRESS;
    }

    if (dataToken != SDCARD_SINGLE_BLOCK_READ_START_TOKEN) {
        return SDCARD_RECEIVE_ERROR;
    }

    busTransfer(sdcard.dev, buffer, NULL, count);

    // Discard trailing CRC, we don't care
    busTransfer(sdcard.dev, NULL, NULL, 2);

    return SDCARD_RECEIVE_SUCCESS;
}

static bool sdcardSpi_sendDataBlockFinish(void)
{
    static const uint8_t dummyCRC[2] = { 0x00, 0x00 };

    // Send a dummy CRC
    busTransfer(sdcard.dev, NULL, dummyCRC, 2);

    uint8_t dataResponseToken;
    busTransfer(sdcard.dev, &dataResponseToken, NULL, 1);

    /*
     * Check if the card accepted the write (no CRC error / no address error)
     *
     * The lower 5 bits are structured as follows:
     * | 0 | Status  | 1 |
     * | 0 | x  x  x | 1 |
     *
     * Statuses:
     * 010 - Data accepted
     * 101 - CRC error
     * 110 - Write error
     */
    return (dataResponseToken & 0x1F) == 0x05;
}

/**
 * Begin sending a buffer of SDCARD_BLOCK_SIZE bytes to the SD card.
 */
static void sdcardSpi_sendDataBlockBegin(uint8_t *buffer, bool multiBlockWrite)
{
    uint8_t blockStartToken[2] = { 0xFF, multiBlockWrite ? SDCARD_MULTIPLE_BLOCK_WRITE_START_TOKEN : SDCARD_SINGLE_BLOCK_WRITE_START_TOKEN };

    // Card wants 8 dummy clock cycles between the write command's response and a data block beginning:
    busTransfer(sdcard.dev, NULL, blockStartToken, 2);

    // Send the first chunk now
    busTransfer(sdcard.dev, NULL, buffer, SDCARD_BLOCK_CHUNK_SIZE);
}

static bool sdcardSpi_receiveCID(void)
{
    uint8_t cid[16];

    if (sdcardSpi_receiveDataBlock(cid, sizeof(cid)) != SDCARD_RECEIVE_SUCCESS) {
        return false;
    }

    sdcard.metadata.manufacturerID = cid[0];
    sdcard.metadata.oemID = (cid[1] << 8) | cid[2];
    sdcard.metadata.productName[0] = cid[3];
    sdcard.metadata.productName[1] = cid[4];
    sdcard.metadata.productName[2] = cid[5];
    sdcard.metadata.productName[3] = cid[6];
    sdcard.metadata.productName[4] = cid[7];
    sdcard.metadata.productRevisionMajor = cid[8] >> 4;
    sdcard.metadata.productRevisionMinor = cid[8] & 0x0F;
    sdcard.metadata.productSerial = (cid[9] << 24) | (cid[10] << 16) | (cid[11] << 8) | cid[12];
    sdcard.metadata.productionYear = (((cid[13] & 0x0F) << 4) | (cid[14] >> 4)) + 2000;
    sdcard.metadata.productionMonth = cid[14] & 0x0F;

    return true;
}

static bool sdcardSpi_fetchCSD(void)
{
    uint32_t readBlockLen, blockCount, blockCountMult;
    uint64_t capacityBytes;

    sdcardSpi_select();

    /* The CSD command's data block should always arrive within 8 idle clock cycles (SD card spec). This is because
     * the information about card latency is stored in the CSD register itself, so we can't use that yet!
     */
    bool success =
        sdcardSpi_sendCommand(SDCARD_COMMAND_SEND_CSD, 0) == 0
        && sdcardSpi_receiveDataBlock((uint8_t*) &sdcard.csd, sizeof(sdcard.csd)) == SDCARD_RECEIVE_SUCCESS
        && SDCARD_GET_CSD_FIELD(sdcard.csd, 1, TRAILER) == 1;

    if (success) {
        switch (SDCARD_GET_CSD_FIELD(sdcard.csd, 1, CSD_STRUCTURE_VER)) {
            case SDCARD_CSD_STRUCTURE_VERSION_1:
                // Block size in bytes (doesn't have to be 512)
                readBlockLen = 1 << SDCARD_GET_CSD_FIELD(sdcard.csd, 1, READ_BLOCK_LEN);
                blockCountMult = 1 << (SDCARD_GET_CSD_FIELD(sdcard.csd, 1, CSIZE_MULT) + 2);
                blockCount = (SDCARD_GET_CSD_FIELD(sdcard.csd, 1, CSIZE) + 1) * blockCountMult;

                // We could do this in 32 bits but it makes the 2GB case awkward
                capacityBytes = (uint64_t) blockCount * readBlockLen;

                // Re-express that capacity (max 2GB) in our standard 512-byte block size
                sdcard.metadata.numBlocks = capacityBytes / SDCARD_BLOCK_SIZE;
            break;
            case SDCARD_CSD_STRUCTURE_VERSION_2:
                sdcard.metadata.numBlocks = (SDCARD_GET_CSD_FIELD(sdcard.csd, 2, CSIZE) + 1) * 1024;
            break;
            default:
                success = false;
        }
    }

    sdcardSpi_deselect();

    return success;
}

/**
 * Check if the SD Card has completed its startup sequence. Must be called with sdcard.state == SDCARD_STATE_INITIALIZATION.
 *
 * Returns true if the card has finished its init process.
 */
static bool sdcardSpi_checkInitDone(void)
{
    sdcardSpi_select();
    uint8_t status = sdcardSpi_sendAppCommand(SDCARD_ACOMMAND_SEND_OP_COND, sdcard.version == 2 ? 1 << 30 /* We support high capacity cards */ : 0);
    sdcardSpi_deselect();

    // When card init is complete, the idle bit in the response becomes zero.
    return status == 0x00;
}

static bool sdcardSpi_setBlockLength(uint32_t blockLen)
{
    sdcardSpi_select();
    uint8_t status = sdcardSpi_sendCommand(SDCARD_COMMAND_SET_BLOCKLEN, blockLen);
    sdcardSpi_deselect();

    return status == 0;
}

/*
 * Returns true if the card is ready to accept read/write commands.
 */
static bool sdcardSpi_isReady(void)
{
    return sdcard.state == SDCARD_STATE_READY || sdcard.state == SDCARD_STATE_WRITING_MULTIPLE_BLOCKS;
}

/**
 * Send the stop-transmission token to complete a multi-block write.
 *
 * Returns:
 *     SDCARD_OPERATION_IN_PROGRESS - We're now waiting for that stop to complete, the card will enter
 *                                    the SDCARD_STATE_STOPPING_MULTIPLE_BLOCK_WRITE state.
 *     SDCARD_OPERATION_SUCCESS     - The multi-block write finished immediately, the card will enter
 *                                    the SDCARD_READY state.
 *
 */
static sdcardOperationStatus_e sdcardSpi_endWriteBlocks(void)
{
    uint8_t writeEndToken[2] = { 0xFF, SDCARD_MULTIPLE_BLOCK_WRITE_STOP_TOKEN };

    sdcard.multiWriteBlocksRemain = 0;

    // 8 dummy clocks to guarantee N_WR clocks between the last card response and this token
    busTransfer(sdcard.dev, NULL, writeEndToken, 2);

    // Card may choose to raise a busy (non-0xFF) signal after at most N_BR (1 byte) delay
    if (sdcardSpi_waitForNonIdleByte(1) == 0xFF) {
        sdcard.state = SDCARD_STATE_READY;
        return SDCARD_OPERATION_SUCCESS;
    } else {
        sdcard.state = SDCARD_STATE_STOPPING_MULTIPLE_BLOCK_WRITE;
        sdcard.operationStartTime = millis();

        return SDCARD_OPERATION_IN_PROGRESS;
    }
}

/**
 * Call periodically for the SD card to perform in-progress transfers.
 *
 * Returns true if the card is ready to accept commands.
 */
static bool sdcardSpi_poll(void)
{
    if (sdcard.dev == NULL) {
        return false;
    }

    uint8_t initStatus;
    bool sendComplete;

    doMore:
    switch (sdcard.state) {
        case SDCARD_STATE_RESET:
            sdcardSpi_select();

            initStatus = sdcardSpi_sendCommand(SDCARD_COMMAND_GO_IDLE_STATE, 0);

            sdcardSpi_deselect();

            if (initStatus == SDCARD_R1_STATUS_BIT_IDLE) {
                // Check card voltage and version
                if (sdcardSpi_validateInterfaceCondition()) {

                    sdcard.state = SDCARD_STATE_CARD_INIT_IN_PROGRESS;
                    goto doMore;
                } else {
                    // Bad reply/voltage, we ought to refrain from accessing the card.
                    sdcard.state = SDCARD_STATE_NOT_PRESENT;
                }
            }
        break;

        case SDCARD_STATE_CARD_INIT_IN_PROGRESS:
            if (sdcardSpi_checkInitDone()) {
                if (sdcard.version == 2) {
                    // Check for high capacity card
                    uint32_t ocr;

                    if (!sdcardSpi_readOCRRegister(&ocr)) {
                        sdcardSpi_reset();
                        goto doMore;
                    }

                    sdcard.highCapacity = (ocr & (1 << 30)) != 0;
                } else {
                    // Version 1 cards are always low-capacity
                    sdcard.highCapacity = false;
                }

                // Now fetch the CSD and CID registers
                if (sdcardSpi_fetchCSD()) {
                    sdcardSpi_select();

                    uint8_t status = sdcardSpi_sendCommand(SDCARD_COMMAND_SEND_CID, 0);

                    if (status == 0) {
                        // Keep the card selected to receive the response block
                        sdcard.state = SDCARD_STATE_INITIALIZATION_RECEIVE_CID;
                        goto doMore;
                    } else {
                        sdcardSpi_deselect();

                        sdcardSpi_reset();
                        goto doMore;
                    }
                }
            }
        break;
        case SDCARD_STATE_INITIALIZATION_RECEIVE_CID:
            if (sdcardSpi_receiveCID()) {
                sdcardSpi_deselect();

                /* The spec is a little iffy on what the default block size is for Standard Size cards (it can be changed on
                 * standard size cards) so let's just set it to 512 explicitly so we don't have a problem.
                 */
                if (!sdcard.highCapacity && !sdcardSpi_setBlockLength(SDCARD_BLOCK_SIZE)) {
                    sdcardSpi_reset();
                    goto doMore;
                }

                // Now we're done with init and we can switch to the full speed clock (<25MHz)
                busSetSpeed(sdcard.dev, SDCARD_BUS_SPEED);

                sdcard.multiWriteBlocksRemain = 0;

                sdcard.state = SDCARD_STATE_READY;
                goto doMore;
            } // else keep waiting for the CID to arrive
        break;
        case SDCARD_STATE_SENDING_WRITE:
            // Have we finished sending the write yet?
            sendComplete = false;

            // Send another chunk
            busTransfer(sdcard.dev, NULL, sdcard.pendingOperation.buffer + SDCARD_BLOCK_CHUNK_SIZE * sdcard.pendingOperation.chunkIndex, SDCARD_BLOCK_CHUNK_SIZE);
            sdcard.pendingOperation.chunkIndex++;
            sendComplete = sdcard.pendingOperation.chunkIndex == SDCARD_BLOCK_SIZE / SDCARD_BLOCK_CHUNK_SIZE;

            if (sendComplete) {
                // Finish up by sending the CRC and checking the SD-card's acceptance/rejectance
                if (sdcardSpi_sendDataBlockFinish()) {
                    // The SD card is now busy committing that write to the card
                    sdcard.state = SDCARD_STATE_WAITING_FOR_WRITE;
                    sdcard.operationStartTime = millis();

                    // Since we've transmitted the buffer we can go ahead and tell the caller their operation is complete
                    if (sdcard.pendingOperation.callback) {
                        sdcard.pendingOperation.callback(SDCARD_BLOCK_OPERATION_WRITE, sdcard.pendingOperation.blockIndex, sdcard.pendingOperation.buffer, sdcard.pendingOperation.callbackData);
                    }
                } else {
                    /* Our write was rejected! This could be due to a bad address but we hope not to attempt that, so assume
                     * the card is broken and needs reset.
                     */
                    sdcardSpi_reset();

                    // Announce write failure:
                    if (sdcard.pendingOperation.callback) {
                        sdcard.pendingOperation.callback(SDCARD_BLOCK_OPERATION_WRITE, sdcard.pendingOperation.blockIndex, NULL, sdcard.pendingOperation.callbackData);
                    }

                    goto doMore;
                }
            }
        break;
        case SDCARD_STATE_WAITING_FOR_WRITE:
            if (sdcardSpi_waitForIdle(SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY)) {
                sdcard.failureCount = 0; // Assume the card is good if it can complete a write

                // Still more blocks left to write in a multi-block chain?
                if (sdcard.multiWriteBlocksRemain > 1) {
                    sdcard.multiWriteBlocksRemain--;
                    sdcard.multiWriteNextBlock++;
                    sdcard.state = SDCARD_STATE_WRITING_MULTIPLE_BLOCKS;
                } else if (sdcard.multiWriteBlocksRemain == 1) {
                    // This function changes the sd card state for us whether immediately succesful or delayed:
                    if (sdcardSpi_endWriteBlocks() == SDCARD_OPERATION_SUCCESS) {
                        sdcardSpi_deselect();
                    }
                } else {
                    sdcard.state = SDCARD_STATE_READY;
                    sdcardSpi_deselect();
                }
            } else if (millis() > sdcard.operationStartTime + SDCARD_TIMEOUT_WRITE_MSEC) {
                /*
                 * The caller has already been told that their write has completed, so they will have discarded
                 * their buffer and have no hope of retrying the operation. But this should be very rare and it allows
                 * them to reuse their buffer milliseconds faster than they otherwise would.
                 */
                sdcardSpi_reset();
                goto doMore;
            }
        break;
        case SDCARD_STATE_READING:
            switch (sdcardSpi_receiveDataBlock(sdcard.pendingOperation.buffer, SDCARD_BLOCK_SIZE)) {
                case SDCARD_RECEIVE_SUCCESS:
                    sdcardSpi_deselect();

                    sdcard.state = SDCARD_STATE_READY;
                    sdcard.failureCount = 0; // Assume the card is good if it can complete a read

                    if (sdcard.pendingOperation.callback) {
                        sdcard.pendingOperation.callback(
                            SDCARD_BLOCK_OPERATION_READ,
                            sdcard.pendingOperation.blockIndex,
                            sdcard.pendingOperation.buffer,
                            sdcard.pendingOperation.callbackData
                        );
                    }
                break;
                case SDCARD_RECEIVE_BLOCK_IN_PROGRESS:
                    if (millis() <= sdcard.operationStartTime + SDCARD_TIMEOUT_READ_MSEC) {
                        break; // Timeout not reached yet so keep waiting
                    }
                    // Timeout has expired, so fall through to convert to a fatal error
                    FALLTHROUGH;

                case SDCARD_RECEIVE_ERROR:
                    sdcardSpi_deselect();

                    sdcardSpi_reset();

                    if (sdcard.pendingOperation.callback) {
                        sdcard.pendingOperation.callback(
                            SDCARD_BLOCK_OPERATION_READ,
                            sdcard.pendingOperation.blockIndex,
                            NULL,
                            sdcard.pendingOperation.callbackData
                        );
                    }

                    goto doMore;
                break;
            }
        break;
        case SDCARD_STATE_STOPPING_MULTIPLE_BLOCK_WRITE:
            if (sdcardSpi_waitForIdle(SDCARD_MAXIMUM_BYTE_DELAY_FOR_CMD_REPLY)) {
                sdcardSpi_deselect();

                sdcard.state = SDCARD_STATE_READY;
            } else if (millis() > sdcard.operationStartTime + SDCARD_TIMEOUT_WRITE_MSEC) {
                sdcardSpi_reset();
                goto doMore;
            }
        break;
        case SDCARD_STATE_NOT_PRESENT:
        default:
            ;
    }

    // Is the card's initialization taking too long?
    if (sdcard.state >= SDCARD_STATE_RESET && sdcard.state < SDCARD_STATE_READY
            && millis() - sdcard.operationStartTime > SDCARD_TIMEOUT_INIT_MILLIS) {
        sdcardSpi_reset();
    }

    return sdcardSpi_isReady();
}

/**
 * Write the 512-byte block from the given buffer into the block with the given index.
 *
 * If the write does not complete immediately, your callback will be called later. If the write was successful, the
 * buffer pointer will be the same buffer you originally passed in, otherwise the buffer will be set to NULL.
 *
 * Returns:
 *     SDCARD_OPERATION_IN_PROGRESS - Your buffer is currently being transmitted to the card and your callback will be
 *                                    called later to report the completion. The buffer pointer must remain valid until
 *                                    that time.
 *     SDCARD_OPERATION_SUCCESS     - Your buffer has been transmitted to the card now.
 *     SDCARD_OPERATION_BUSY        - The card is already busy and cannot accept your write
 *     SDCARD_OPERATION_FAILURE     - Your write was rejected by the card, card will be reset
 */
static sdcardOperationStatus_e sdcardSpi_writeBlock(uint32_t blockIndex, uint8_t *buffer, sdcard_operationCompleteCallback_c callback, uint32_t callbackData)
{
    uint8_t status;

    doMore:
    switch (sdcard.state) {
        case SDCARD_STATE_WRITING_MULTIPLE_BLOCKS:
            // Do we need to cancel the previous multi-block write?
            if (blockIndex != sdcard.multiWriteNextBlock) {
                if (sdcardSpi_endWriteBlocks() == SDCARD_OPERATION_SUCCESS) {
                    // Now we've entered the ready state, we can try again
                    goto doMore;
                } else {
                    return SDCARD_OPERATION_BUSY;
                }
            }

            // We're continuing a multi-block write
        break;
        case SDCARD_STATE_READY:
            // We're not continuing a multi-block write so we need to send a single-block write command
            sdcardSpi_select();

            // Standard size cards use byte addressing, high capacity cards use block addressing
            status = sdcardSpi_sendCommand(SDCARD_COMMAND_WRITE_BLOCK, sdcard.highCapacity ? blockIndex : blockIndex * SDCARD_BLOCK_SIZE);

            if (status != 0) {
                sdcardSpi_deselect();

                sdcardSpi_reset();

                return SDCARD_OPERATION_FAILURE;
            }
        break;
        default:
            return SDCARD_OPERATION_BUSY;
    }

    sdcardSpi_sendDataBlockBegin(buffer, sdcard.state == SDCARD_STATE_WRITING_MULTIPLE_BLOCKS);

    sdcard.pendingOperation.buffer = buffer;
    sdcard.pendingOperation.blockIndex = blockIndex;
    sdcard.pendingOperation.callback = callback;
    sdcard.pendingOperation.callbackData = callbackData;
    sdcard.pendingOperation.chunkIndex = 1;
    sdcard.state = SDCARD_STATE_SENDING_WRITE;

    return SDCARD_OPERATION_IN_PROGRESS;
}

/**
 * Begin writing a series of consecutive blocks beginning at the given block index. This will allow (but not require)
 * the SD card to pre-erase the number of blocks you specifiy, which can allow the writes to complete faster.
 *
 * Afterwards, just call sdcard_writeBlock() as normal to write those blocks consecutively.
 *
 * It's okay to abort the multi-block write at any time by writing to a non-consecutive address, or by performing a read.
 *
 * Returns:
 *     SDCARD_OPERATION_SUCCESS     - Multi-block write has been queued
 *     SDCARD_OPERATION_BUSY        - The card is already busy and cannot accept your write
 *     SDCARD_OPERATION_FAILURE     - A fatal error occured, card will be reset
 */
static sdcardOperationStatus_e sdcardSpi_beginWriteBlocks(uint32_t blockIndex, uint32_t blockCount)
{
    if (sdcard.state != SDCARD_STATE_READY) {
        if (sdcard.state == SDCARD_STATE_WRITING_MULTIPLE_BLOCKS) {
            if (blockIndex == sdcard.multiWriteNextBlock) {
                // Assume that the caller wants to continue the multi-block write they already have in progress!
                return SDCARD_OPERATION_SUCCESS;
            } else if (sdcardSpi_endWriteBlocks() != SDCARD_OPERATION_SUCCESS) {
                return SDCARD_OPERATION_BUSY;
            } // Else we've completed the previous multi-block write and can fall through to start the new one
        } else {
            return SDCARD_OPERATION_BUSY;
        }
    }

    sdcardSpi_select();

    if (
        sdcardSpi_sendAppCommand(SDCARD_ACOMMAND_SET_WR_BLOCK_ERASE_COUNT, blockCount) == 0
        && sdcardSpi_sendCommand(SDCARD_COMMAND_WRITE_MULTIPLE_BLOCK, sdcard.highCapacity ? blockIndex : blockIndex * SDCARD_BLOCK_SIZE) == 0
    ) {
        sdcard.state = SDCARD_STATE_WRITING_MULTIPLE_BLOCKS;
        sdcard.multiWriteBlocksRemain = blockCount;
        sdcard.multiWriteNextBlock = blockIndex;

        // Leave the card selected
        return SDCARD_OPERATION_SUCCESS;
    } else {
        sdcardSpi_deselect();

        sdcardSpi_reset();

        return SDCARD_OPERATION_FAILURE;
    }
}

/**
 * Read the 512-byte block with the given index into the given 512-byte buffer.
 *
 * When the read completes, your callback will be called. If the read was successful, the buffer pointer will be the
 * same buffer you originally passed in, otherwise the buffer will be set to NULL.
 *
 * You must keep the pointer to the buffer valid until the operation completes!
 *
 * Returns:
 *     true - The operation was successfully queued for later completion, your callback will be called later
 *     false - The operation could not be started due to the card being busy (try again later).
 */
static bool sdcardSpi_readBlock(uint32_t blockIndex, uint8_t *buffer, sdcard_operationCompleteCallback_c callback, uint32_t callbackData)
{
    if (sdcard.state != SDCARD_STATE_READY) {
        if (sdcard.state == SDCARD_STATE_WRITING_MULTIPLE_BLOCKS) {
            if (sdcardSpi_endWriteBlocks() != SDCARD_OPERATION_SUCCESS) {
                return false;
            }
        } else {
            return false;
        }
    }

    sdcardSpi_select();

    // Standard size cards use byte addressing, high capacity cards use block addressing
    uint8_t status = sdcardSpi_sendCommand(SDCARD_COMMAND_READ_SINGLE_BLOCK, sdcard.highCapacity ? blockIndex : blockIndex * SDCARD_BLOCK_SIZE);

    if (status == 0) {
        sdcard.pendingOperation.buffer = buffer;
        sdcard.pendingOperation.blockIndex = blockIndex;
        sdcard.pendingOperation.callback = callback;
        sdcard.pendingOperation.callbackData = callbackData;

        sdcard.state = SDCARD_STATE_READING;

        sdcard.operationStartTime = millis();

        // Leave the card selected for the whole transaction

        return true;
    } else {
        sdcardSpi_deselect();
        return false;
    }
}

/**
 * Returns true if the SD card has successfully completed its startup procedures.
 */
static bool sdcardSpi_isInitialized(void)
{
    return sdcard.state >= SDCARD_STATE_READY;
}

static const sdcardMetadata_t* sdcardSpi_getMetadata(void)
{
    return &sdcard.metadata;
}

/**
 * Begin the initialization process for the SD card. This must be called first before any other sdcard_ routine.
 */
void sdcardSpi_init(void)
{
    sdcard.dev = busDeviceInit(BUSTYPE_SPI, DEVHW_SDCARD, 0, OWNER_SDCARD);
    if (!sdcard.dev) {
        sdcard.state = SDCARD_STATE_NOT_PRESENT;
        return;
    }

    // Max frequency is initially 400kHz
    busSetSpeed(sdcard.dev, BUS_SPEED_INITIALIZATION);

    // SDCard wants 1ms minimum delay after power is applied to it
    delay(1000);

    // Transmit at least 74 dummy clock cycles with CS high so the SD card can start up
    busDeselectDevice(sdcard.dev);
    busTransfer(sdcard.dev, NULL, NULL, SDCARD_INIT_NUM_DUMMY_BYTES);

    // Wait for that transmission to finish before we enable the SDCard, so it receives the required number of cycles:
    int time = 100000;
    while (busIsBusy(sdcard.dev)) {
        if (time-- == 0) {
            busDeviceDeInit(sdcard.dev);
            sdcard.dev = NULL;
            sdcard.state = SDCARD_STATE_NOT_PRESENT;
            sdcard.failureCount++;
            return;
        }
    }

    sdcard.operationStartTime = millis();
    sdcard.state = SDCARD_STATE_RESET;
    sdcard.failureCount = 0;
}

sdcardVTable_t sdcardSpiVTable = {
    .init = &sdcardSpi_init,
    .readBlock = &sdcardSpi_readBlock,
    .beginWriteBlocks = &sdcardSpi_beginWriteBlocks,
    .writeBlock = &sdcardSpi_writeBlock,
    .poll = &sdcardSpi_poll,
    .isFunctional = &sdcardSpi_isFunctional,
    .isInitialized = &sdcardSpi_isInitialized,
    .getMetadata = &sdcardSpi_getMetadata,
};

#endif

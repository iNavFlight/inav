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
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "platform.h"

#include "build/debug.h"

#include "common/streambuf.h"
#include "common/utils.h"
#include "common/maths.h"
#include "common/crc.h"

#include "drivers/system.h"
#include "drivers/serial.h"

#include "io/serial.h"
#include "fc/cli.h"

#include "msp/msp.h"
#include "msp/msp_serial.h"

static mspPort_t mspPorts[MAX_MSP_PORT_COUNT];


void resetMspPort(mspPort_t *mspPortToReset, serialPort_t *serialPort)
{
    memset(mspPortToReset, 0, sizeof(mspPort_t));

    mspPortToReset->port = serialPort;
}

void mspSerialAllocatePorts(void)
{
    uint8_t portIndex = 0;
    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP);
    while (portConfig && portIndex < MAX_MSP_PORT_COUNT) {
        mspPort_t *mspPort = &mspPorts[portIndex];
        if (mspPort->port) {
            portIndex++;
            continue;
        }

        serialPort_t *serialPort = openSerialPort(portConfig->identifier, FUNCTION_MSP, NULL, NULL, baudRates[portConfig->msp_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);
        if (serialPort) {
            resetMspPort(mspPort, serialPort);
            portIndex++;
        }

        portConfig = findNextSerialPortConfig(FUNCTION_MSP);
    }
}

void mspSerialReleasePortIfAllocated(serialPort_t *serialPort)
{
    for (uint8_t portIndex = 0; portIndex < MAX_MSP_PORT_COUNT; portIndex++) {
        mspPort_t *candidateMspPort = &mspPorts[portIndex];
        if (candidateMspPort->port == serialPort) {
            closeSerialPort(serialPort);
            memset(candidateMspPort, 0, sizeof(mspPort_t));
        }
    }
}

static bool mspSerialProcessReceivedData(mspPort_t *mspPort, uint8_t c)
{
    switch (mspPort->c_state) {
        default:
        case MSP_IDLE:      // Waiting for '$' character
            if (c == '$') {
                mspPort->mspVersion = MSP_V1;
                mspPort->c_state = MSP_HEADER_START;
            }
            else {
                return false;
            }
            break;

        case MSP_HEADER_START:  // Waiting for 'M' (MSPv1 / MSPv2_over_v1) or 'X' (MSPv2 native)
            switch (c) {
                case 'M':
                    mspPort->c_state = MSP_HEADER_M;
                    break;
                case 'X':
                    mspPort->c_state = MSP_HEADER_X;
                    break;
                default:
                    mspPort->c_state = MSP_IDLE;
                    break;
            }
            break;

        case MSP_HEADER_M:      // Waiting for '<'
            if (c == '<') {
                mspPort->offset = 0;
                mspPort->checksum1 = 0;
                mspPort->checksum2 = 0;
                mspPort->c_state = MSP_HEADER_V1;
            }
            else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_X:
            if (c == '<') {
                mspPort->offset = 0;
                mspPort->checksum2 = 0;
                mspPort->mspVersion = MSP_V2_NATIVE;
                mspPort->c_state = MSP_HEADER_V2_NATIVE;
            }
            else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V1:     // Now receive v1 header (size/cmd), this is already checksummable
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum1 ^= c;
            if (mspPort->offset == sizeof(mspHeaderV1_t)) {
                mspHeaderV1_t * hdr = (mspHeaderV1_t *)&mspPort->inBuf[0];
                // Check incoming buffer size limit
                if (hdr->size > MSP_PORT_INBUF_SIZE) {
                    mspPort->c_state = MSP_IDLE;
                }
                else if (hdr->cmd == MSP_V2_FRAME_ID) {
                    // MSPv1 payload must be big enough to hold V2 header + extra checksum
                    if (hdr->size >= sizeof(mspHeaderV2_t) + 1) {
                        mspPort->mspVersion = MSP_V2_OVER_V1;
                        mspPort->c_state = MSP_HEADER_V2_OVER_V1;
                    }
                    else {
                        mspPort->c_state = MSP_IDLE;
                    }
                }
                else {
                    mspPort->dataSize = hdr->size;
                    mspPort->cmdMSP = hdr->cmd;
                    mspPort->cmdFlags = 0;
                    mspPort->offset = 0;                // re-use buffer
                    mspPort->c_state = mspPort->dataSize > 0 ? MSP_PAYLOAD_V1 : MSP_CHECKSUM_V1;    // If no payload - jump to checksum byte
                }
            }
            break;

        case MSP_PAYLOAD_V1:
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum1 ^= c;
            if (mspPort->offset == mspPort->dataSize) {
                mspPort->c_state = MSP_CHECKSUM_V1;
            }
            break;

        case MSP_CHECKSUM_V1:
            if (mspPort->checksum1 == c) {
                mspPort->c_state = MSP_COMMAND_RECEIVED;
            } else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V2_OVER_V1:     // V2 header is part of V1 payload - we need to calculate both checksums now
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum1 ^= c;
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            if (mspPort->offset == (sizeof(mspHeaderV2_t) + sizeof(mspHeaderV1_t))) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&mspPort->inBuf[sizeof(mspHeaderV1_t)];
                mspPort->dataSize = hdrv2->size;

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_PORT_INBUF_SIZE) {
                    mspPort->c_state = MSP_IDLE;
                }
                else {
                    mspPort->cmdMSP = hdrv2->cmd;
                    mspPort->cmdFlags = hdrv2->flags;
                    mspPort->offset = 0;                // re-use buffer
                    mspPort->c_state = mspPort->dataSize > 0 ? MSP_PAYLOAD_V2_OVER_V1 : MSP_CHECKSUM_V2_OVER_V1;
                }
            }
            break;

        case MSP_PAYLOAD_V2_OVER_V1:
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            mspPort->checksum1 ^= c;
            mspPort->inBuf[mspPort->offset++] = c;

            if (mspPort->offset == mspPort->dataSize) {
                mspPort->c_state = MSP_CHECKSUM_V2_OVER_V1;
            }
            break;

        case MSP_CHECKSUM_V2_OVER_V1:
            mspPort->checksum1 ^= c;
            if (mspPort->checksum2 == c) {
                mspPort->c_state = MSP_CHECKSUM_V1; // Checksum 2 correct - verify v1 checksum
            } else {
                mspPort->c_state = MSP_IDLE;
            }
            break;

        case MSP_HEADER_V2_NATIVE:
            mspPort->inBuf[mspPort->offset++] = c;
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            if (mspPort->offset == sizeof(mspHeaderV2_t)) {
                mspHeaderV2_t * hdrv2 = (mspHeaderV2_t *)&mspPort->inBuf[0];

                // Check for potential buffer overflow
                if (hdrv2->size > MSP_PORT_INBUF_SIZE) {
                    mspPort->c_state = MSP_IDLE;
                }
                else {
                    mspPort->dataSize = hdrv2->size;
                    mspPort->cmdMSP = hdrv2->cmd;
                    mspPort->cmdFlags = hdrv2->flags;
                    mspPort->offset = 0;                // re-use buffer
                    mspPort->c_state = mspPort->dataSize > 0 ? MSP_PAYLOAD_V2_NATIVE : MSP_CHECKSUM_V2_NATIVE;
                }
            }
            break;

        case MSP_PAYLOAD_V2_NATIVE:
            mspPort->checksum2 = crc8_dvb_s2(mspPort->checksum2, c);
            mspPort->inBuf[mspPort->offset++] = c;

            if (mspPort->offset == mspPort->dataSize) {
                mspPort->c_state = MSP_CHECKSUM_V2_NATIVE;
            }
            break;

        case MSP_CHECKSUM_V2_NATIVE:
            if (mspPort->checksum2 == c) {
                mspPort->c_state = MSP_COMMAND_RECEIVED;
            } else {
                mspPort->c_state = MSP_IDLE;
            }
            break;
    }

    return true;
}

static uint8_t mspSerialChecksumBuf(uint8_t checksum, const uint8_t *data, int len)
{
    while (len-- > 0) {
        checksum ^= *data++;
    }
    return checksum;
}

#define JUMBO_FRAME_SIZE_LIMIT 255
static int mspSerialSendFrame(mspPort_t *msp, const uint8_t * hdr, int hdrLen, const uint8_t * data, int dataLen, const uint8_t * crc, int crcLen)
{
    // MSP port might be turned into a CLI port, which will make
    // msp->port become NULL.
    serialPort_t *port = msp->port;
    if (!port) {
        return 0;
    }
    // VSP MSP port might be unconnected. To prevent blocking - check if it's connected first
    if (!serialIsConnected(port)) {
        return 0;
    }

    // We are allowed to send out the response if
    //  a) TX buffer is completely empty (we are talking to well-behaving party that follows request-response scheduling;
    //     this allows us to transmit jumbo frames bigger than TX buffer (serialWriteBuf will block, but for jumbo frames we don't care)
    //  b) Response fits into TX buffer
    const int totalFrameLength = hdrLen + dataLen + crcLen;
    if (!isSerialTransmitBufferEmpty(port) && ((int)serialTxBytesFree(port) < totalFrameLength))
        return 0;

    // Transmit frame
    serialBeginWrite(port);
    serialWriteBuf(port, hdr, hdrLen);
    serialWriteBuf(port, data, dataLen);
    serialWriteBuf(port, crc, crcLen);
    serialEndWrite(port);

    return totalFrameLength;
}

static int mspSerialEncode(mspPort_t *msp, mspPacket_t *packet, mspVersion_e mspVersion)
{
    static const uint8_t mspMagic[MSP_VERSION_COUNT] = MSP_VERSION_MAGIC_INITIALIZER;
    const int dataLen = sbufBytesRemaining(&packet->buf);
    uint8_t hdrBuf[16] = { '$', mspMagic[mspVersion], packet->result == MSP_RESULT_ERROR ? '!' : '>'};
    uint8_t crcBuf[2];
    int hdrLen = 3;
    int crcLen = 0;

    #define V1_CHECKSUM_STARTPOS 3
    if (mspVersion == MSP_V1) {
        mspHeaderV1_t * hdrV1 = (mspHeaderV1_t *)&hdrBuf[hdrLen];
        hdrLen += sizeof(mspHeaderV1_t);
        hdrV1->cmd = packet->cmd;

        // Add JUMBO-frame header if necessary
        if (dataLen >= JUMBO_FRAME_SIZE_LIMIT) {
            mspHeaderJUMBO_t * hdrJUMBO = (mspHeaderJUMBO_t *)&hdrBuf[hdrLen];
            hdrLen += sizeof(mspHeaderJUMBO_t);

            hdrV1->size = JUMBO_FRAME_SIZE_LIMIT;
            hdrJUMBO->size = dataLen;
        }
        else {
            hdrV1->size = dataLen;
        }

        // Pre-calculate CRC
        crcBuf[crcLen] = mspSerialChecksumBuf(0, hdrBuf + V1_CHECKSUM_STARTPOS, hdrLen - V1_CHECKSUM_STARTPOS);
        crcBuf[crcLen] = mspSerialChecksumBuf(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcLen++;
    }
    else if (mspVersion == MSP_V2_OVER_V1) {
        mspHeaderV1_t * hdrV1 = (mspHeaderV1_t *)&hdrBuf[hdrLen];

        hdrLen += sizeof(mspHeaderV1_t);

        mspHeaderV2_t * hdrV2 = (mspHeaderV2_t *)&hdrBuf[hdrLen];
        hdrLen += sizeof(mspHeaderV2_t);

        const int v1PayloadSize = sizeof(mspHeaderV2_t) + dataLen + 1;  // MSPv2 header + data payload + MSPv2 checksum
        hdrV1->cmd = MSP_V2_FRAME_ID;

        // Add JUMBO-frame header if necessary
        if (v1PayloadSize >= JUMBO_FRAME_SIZE_LIMIT) {
            mspHeaderJUMBO_t * hdrJUMBO = (mspHeaderJUMBO_t *)&hdrBuf[hdrLen];
            hdrLen += sizeof(mspHeaderJUMBO_t);

            hdrV1->size = JUMBO_FRAME_SIZE_LIMIT;
            hdrJUMBO->size = v1PayloadSize;
        }
        else {
            hdrV1->size = v1PayloadSize;
        }

        // Fill V2 header
        hdrV2->flags = packet->flags;
        hdrV2->cmd = packet->cmd;
        hdrV2->size = dataLen;

        // V2 CRC: only V2 header + data payload
        crcBuf[crcLen] = crc8_dvb_s2_update(0, (uint8_t *)hdrV2, sizeof(mspHeaderV2_t));
        crcBuf[crcLen] = crc8_dvb_s2_update(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcLen++;

        // V1 CRC: All headers + data payload + V2 CRC byte
        crcBuf[crcLen] = mspSerialChecksumBuf(0, hdrBuf + V1_CHECKSUM_STARTPOS, hdrLen - V1_CHECKSUM_STARTPOS);
        crcBuf[crcLen] = mspSerialChecksumBuf(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcBuf[crcLen] = mspSerialChecksumBuf(crcBuf[crcLen], crcBuf, crcLen);
        crcLen++;
    }
    else if (mspVersion == MSP_V2_NATIVE) {
        mspHeaderV2_t * hdrV2 = (mspHeaderV2_t *)&hdrBuf[hdrLen];
        hdrLen += sizeof(mspHeaderV2_t);

        hdrV2->flags = packet->flags;
        hdrV2->cmd = packet->cmd;
        hdrV2->size = dataLen;

        crcBuf[crcLen] = crc8_dvb_s2_update(0, (uint8_t *)hdrV2, sizeof(mspHeaderV2_t));
        crcBuf[crcLen] = crc8_dvb_s2_update(crcBuf[crcLen], sbufPtr(&packet->buf), dataLen);
        crcLen++;
    }
    else {
        // Shouldn't get here
        return 0;
    }

    // Send the frame
    return mspSerialSendFrame(msp, hdrBuf, hdrLen, sbufPtr(&packet->buf), dataLen, crcBuf, crcLen);
}

//uint8_t mspSerialOutBuf[MSP_PORT_OUTBUF_SIZE]; // this buffer also used in msp_shared.c

static mspPostProcessFnPtr mspSerialProcessReceivedCommand(mspPort_t *msp, mspProcessCommandFnPtr mspProcessCommandFn)
{
    static uint8_t mspSerialOutBuf[MSP_PORT_OUTBUF_SIZE];

    mspPacket_t reply = {
        .buf = { .ptr = mspSerialOutBuf, .end = ARRAYEND(mspSerialOutBuf), },
        .cmd = -1,
        .flags = 0,
        .result = 0,
    };
    uint8_t *outBufHead = reply.buf.ptr;

    mspPacket_t command = {
        .buf = { .ptr = msp->inBuf, .end = msp->inBuf + msp->dataSize, },
        .cmd = msp->cmdMSP,
        .flags = msp->cmdFlags,
        .result = 0,
    };

    mspPostProcessFnPtr mspPostProcessFn = NULL;
    const mspResult_e status = mspProcessCommandFn(&command, &reply, &mspPostProcessFn);

    if (status != MSP_RESULT_NO_REPLY) {
        sbufSwitchToReader(&reply.buf, outBufHead); // change streambuf direction
        mspSerialEncode(msp, &reply, msp->mspVersion);
    }

    msp->c_state = MSP_IDLE;
    return mspPostProcessFn;
}

static void mspEvaluateNonMspData(mspPort_t * mspPort, uint8_t receivedChar)
{
    if (receivedChar == '#') {
        mspPort->pendingRequest = MSP_PENDING_CLI;
        return;
    }

    if (receivedChar == serialConfig()->reboot_character) {
        mspPort->pendingRequest = MSP_PENDING_BOOTLOADER;
        return;
    }
}

static void mspProcessPendingRequest(mspPort_t * mspPort)
{
    // If no request is pending or 100ms guard time has not elapsed - do nothing
    if ((mspPort->pendingRequest == MSP_PENDING_NONE) || (millis() - mspPort->lastActivityMs < 100)) {
        return;
    }

    switch(mspPort->pendingRequest) {
        case MSP_PENDING_BOOTLOADER:
            systemResetToBootloader();
            break;

        case MSP_PENDING_CLI:
            if (!cliMode) {
                // When we enter CLI mode - disable this MSP port. Don't care about preserving the port since CLI can only be exited via reboot
                cliEnter(mspPort->port);
                mspPort->port = NULL;
            }
            break;

        default:
            break;
    }
}

void mspSerialProcessOnePort(mspPort_t * const mspPort, mspEvaluateNonMspData_e evaluateNonMspData, mspProcessCommandFnPtr mspProcessCommandFn)
{
    mspPostProcessFnPtr mspPostProcessFn = NULL;

    if (serialRxBytesWaiting(mspPort->port)) {
        // There are bytes incoming - abort pending request
        mspPort->lastActivityMs = millis();
        mspPort->pendingRequest = MSP_PENDING_NONE;

        // Process incoming bytes
        while (serialRxBytesWaiting(mspPort->port)) {
            const uint8_t c = serialRead(mspPort->port);
            const bool consumed = mspSerialProcessReceivedData(mspPort, c);

            //SD(fprintf(stderr, "[MSP]: received char: %02x (%c) state: %i\n", c, isprint(c) ? c : '.', mspPort->c_state));
            if (!consumed && evaluateNonMspData == MSP_EVALUATE_NON_MSP_DATA) {
                mspEvaluateNonMspData(mspPort, c);
            }

            if (mspPort->c_state == MSP_COMMAND_RECEIVED) {
                mspPostProcessFn = mspSerialProcessReceivedCommand(mspPort, mspProcessCommandFn);
                break; // process one command at a time so as not to block.
            }
        }

        if (mspPostProcessFn) {
            waitForSerialPortToFinishTransmitting(mspPort->port);
            mspPostProcessFn(mspPort->port);
        }
    }
    else {
        mspProcessPendingRequest(mspPort);
    }
}

/*
 * Process MSP commands from serial ports configured as MSP ports.
 *
 * Called periodically by the scheduler.
 */
void mspSerialProcess(mspEvaluateNonMspData_e evaluateNonMspData, mspProcessCommandFnPtr mspProcessCommandFn)
{
    for (uint8_t portIndex = 0; portIndex < MAX_MSP_PORT_COUNT; portIndex++) {
        mspPort_t * const mspPort = &mspPorts[portIndex];
        if (mspPort->port) {
            mspSerialProcessOnePort(mspPort, evaluateNonMspData, mspProcessCommandFn);
        }
    }
}

void mspSerialInit(void)
{
    memset(mspPorts, 0, sizeof(mspPorts));
    mspSerialAllocatePorts();
}

int mspSerialPushPort(uint16_t cmd, const uint8_t *data, int datalen, mspPort_t *mspPort, mspVersion_e version)
{
    uint8_t pushBuf[MSP_PORT_OUTBUF_SIZE];

    mspPacket_t push = {
        .buf = { .ptr = pushBuf, .end = ARRAYEND(pushBuf), },
        .cmd = cmd,
        .result = 0,
    };

    sbufWriteData(&push.buf, data, datalen);

    sbufSwitchToReader(&push.buf, pushBuf);

    return mspSerialEncode(mspPort, &push, version);
}

int mspSerialPushVersion(uint8_t cmd, const uint8_t *data, int datalen, mspVersion_e version)
{
    int ret = 0;

    for (int portIndex = 0; portIndex < MAX_MSP_PORT_COUNT; portIndex++) {
        mspPort_t * const mspPort = &mspPorts[portIndex];
        if (!mspPort->port) {
            continue;
        }

        // Avoid unconnected ports (only VCP for now)
        if (!serialIsConnected(mspPort->port)) {
            continue;
        }

        ret = mspSerialPushPort(cmd, data, datalen, mspPort, version);
    }
    return ret; // return the number of bytes written
}

int mspSerialPush(uint8_t cmd, const uint8_t *data, int datalen)
{
    return mspSerialPushVersion(cmd, data, datalen, MSP_V1);
}

uint32_t mspSerialTxBytesFree(serialPort_t *port)
{
   return serialTxBytesFree(port);
}

mspPort_t * mspSerialPortFind(const serialPort_t *serialPort)
{
    for (int portIndex = 0; portIndex < MAX_MSP_PORT_COUNT; portIndex++) {
        mspPort_t * mspPort = &mspPorts[portIndex];
        if (mspPort->port == serialPort) {
            return mspPort;
        }
    }
    return NULL;
}

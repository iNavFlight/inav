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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#ifndef SKIP_MSP

#include "build/build_config.h"

#include "common/streambuf.h"
#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/serial.h"
#include "drivers/buf_writer.h"

#include "fc/runtime_config.h"

#include "io/serial.h"

#include "msp/msp_protocol.h"
#include "msp/msp.h"

#include "flight/mixer.h"

#include "config/config.h"

#include "serial_msp.h"

mspPostProcessFuncPtr mspPostProcessFn = NULL;

serialPort_t *mspSerialPort;
mspPort_t mspPorts[MAX_MSP_PORT_COUNT];
mspPort_t *currentPort;
bufWriter_t *writer;
// cause reboot after MSP processing complete

static void resetMspPort(mspPort_t *mspPortToReset, serialPort_t *serialPort)
{
    memset(mspPortToReset, 0, sizeof(mspPort_t));

    mspPortToReset->port = serialPort;
}

void mspSerialAllocatePorts(void)
{
    serialPort_t *serialPort;

    uint8_t portIndex = 0;

    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_MSP);

    while (portConfig && portIndex < MAX_MSP_PORT_COUNT) {
        mspPort_t *mspPort = &mspPorts[portIndex];
        if (mspPort->port) {
            portIndex++;
            continue;
        }

        serialPort = openSerialPort(portConfig->identifier, FUNCTION_MSP, NULL, baudRates[portConfig->msp_baudrateIndex], MODE_RXTX, SERIAL_NOT_INVERTED);
        if (serialPort) {
            resetMspPort(mspPort, serialPort);
            portIndex++;
        }

        portConfig = findNextSerialPortConfig(FUNCTION_MSP);
    }
}

void mspSerialReleasePortIfAllocated(serialPort_t *serialPort)
{
    uint8_t portIndex;
    for (portIndex = 0; portIndex < MAX_MSP_PORT_COUNT; portIndex++) {
        mspPort_t *candidateMspPort = &mspPorts[portIndex];
        if (candidateMspPort->port == serialPort) {
            closeSerialPort(serialPort);
            memset(candidateMspPort, 0, sizeof(mspPort_t));
        }
    }
}

void mspSerialInit(void)
{
    for(int i = 0; i < MAX_MSP_PORT_COUNT; i++) {
        resetMspPort(&mspPorts[i], NULL);
    }

    mspSerialAllocatePorts();
}

static uint8_t mspSerialChecksum(uint8_t checksum, uint8_t byte)
{
    return checksum ^ byte;
}

static uint8_t mspSerialChecksumBuf(uint8_t checksum, uint8_t *data, int len)
{
    while(len-- > 0) {
        checksum = mspSerialChecksum(checksum, *data++);
    }

    return checksum;
}

void mspSerialEncode(mspPort_t *msp, mspPacket_t *packet)
{
    serialBeginWrite(msp->port);
    int len = sbufBytesRemaining(&packet->buf);
    uint8_t hdr[] = {'$', 'M', packet->result < 0 ? '!' : (msp->mode == MSP_MODE_SERVER ? '>' : '<'), len, packet->cmd};
    uint8_t csum = 0;                                       // initial checksum value
    serialWriteBuf(msp->port, hdr, sizeof(hdr));
    csum = mspSerialChecksumBuf(csum, hdr + 3, 2);          // checksum starts from len field
    if(len > 0) {
        serialWriteBuf(msp->port, sbufPtr(&packet->buf), len);
        csum = mspSerialChecksumBuf(csum, sbufPtr(&packet->buf), len);
    }
    serialWrite(msp->port, csum);
    serialEndWrite(msp->port);
}

STATIC_UNIT_TESTED void mspSerialProcessReceivedCommand(mspPort_t *msp)
{
    uint8_t outBuf[MSP_PORT_OUTBUF_SIZE];

    mspPacket_t message = {
        .buf = {
            .ptr = outBuf,
            .end = ARRAYEND(outBuf),
        },
        .cmd = -1,
        .result = 0,
    };

    mspPacket_t command = {
        .buf = {
            .ptr = msp->inBuf,
            .end = msp->inBuf + msp->dataSize,
        },
        .cmd = msp->cmdMSP,
        .result = 0,
    };

    mspPacket_t *reply = &message;

    uint8_t *outBufHead = reply->buf.ptr;

    int status = mspProcessCommand(&command, reply);

    if (status) {
        // reply should be sent back
        sbufSwitchToReader(&reply->buf, outBufHead); // change streambuf direction
        mspSerialEncode(msp, reply);
    }

    msp->c_state = IDLE;
}

#ifdef USE_MSP_CLIENT
static void mspSerialProcessReceivedReply(mspPort_t *msp)
{
    mspPacket_t reply = {
        .buf = {
            .ptr = msp->inBuf,
            .end = msp->inBuf + msp->dataSize,
        },
        .cmd = msp->cmdMSP,
        .result = 0,
    };

    mspProcessReply(&reply);

    msp->c_state = IDLE;
}
#endif


static bool mspSerialProcessReceivedByte(mspPort_t *msp, uint8_t c)
{
    switch(msp->c_state) {
        default:                 // be conservative with unexpected state
        case IDLE:
            if (c != '$')        // wait for '$' to start MSP message
                return false;
            msp->c_state = HEADER_M;
            break;
        case HEADER_M:
            msp->c_state = (c == 'M') ? HEADER_ARROW : IDLE;
            break;
        case HEADER_ARROW:
            msp->c_state = IDLE;
            switch(c) {
                case '<': // COMMAND
                    if (msp->mode == MSP_MODE_SERVER) {
                        msp->c_state = HEADER_SIZE;
                    }
                    break;
                case '>': // REPLY
                    if (msp->mode == MSP_MODE_CLIENT) {
                        msp->c_state = HEADER_SIZE;
                    }
                    break;
                default:
                    break;
            }
            break;
        case HEADER_SIZE:
            if (c > MSP_PORT_INBUF_SIZE) {
                msp->c_state = IDLE;
            } else {
                msp->dataSize = c;
                msp->offset = 0;
                msp->c_state = HEADER_CMD;
            }
            break;
        case HEADER_CMD:
            msp->cmdMSP = c;
            msp->c_state = HEADER_DATA;
            break;
        case HEADER_DATA:
            if(msp->offset < msp->dataSize) {
                msp->inBuf[msp->offset++] = c;
            } else {
                uint8_t checksum = 0;
                checksum = mspSerialChecksum(checksum, msp->dataSize);
                checksum = mspSerialChecksum(checksum, msp->cmdMSP);
                checksum = mspSerialChecksumBuf(checksum, msp->inBuf, msp->dataSize);
                if(c == checksum)
                    msp->c_state = MESSAGE_RECEIVED;
                else
                    msp->c_state = IDLE;
            }
            break;
    }
    return true;
}

void mspSerialProcess(void)
{
    for (int i = 0; i < MAX_MSP_PORT_COUNT; i++) {
        mspPort_t *msp = &mspPorts[i];
        if (!msp->port) {
            continue;
        }

        uint8_t bytesWaiting;
        while ((bytesWaiting = serialRxBytesWaiting(msp->port))) {
            uint8_t c = serialRead(msp->port);
            bool consumed = mspSerialProcessReceivedByte(msp, c);

            if (!consumed) {
                evaluateOtherData(msp->port, c);
            }

            if (msp->c_state == MESSAGE_RECEIVED) {
                if (msp->mode == MSP_MODE_SERVER) {
                    mspSerialProcessReceivedCommand(msp);
                }
#ifdef USE_MSP_CLIENT
                if (msp->mode == MSP_MODE_CLIENT) {
                    mspSerialProcessReceivedReply(msp);
                }
#endif
                break; // process one command at a time so as not to block and handle modal command immediately
            }
        }
        if (mspPostProcessFn) {
            mspPostProcessFn(msp);
            mspPostProcessFn = NULL;
        }

        // TODO consider extracting this outside the loop and create a new loop in mspClientProcess and rename mspProcess to mspServerProcess
        if (msp->c_state == IDLE && msp->commandSenderFn && !bytesWaiting) {

            uint8_t outBuf[MSP_PORT_OUTBUF_SIZE];
            mspPacket_t message = {
                .buf = {
                    .ptr = outBuf,
                    .end = ARRAYEND(outBuf),
                },
                .cmd = -1,
                .result = 0,
            };

            mspPacket_t *command = &message;

            uint8_t *outBufHead = command->buf.ptr;

            bool shouldSend = msp->commandSenderFn(command); // FIXME rename to request builder

            if (shouldSend) {
                sbufSwitchToReader(&command->buf, outBufHead); // change streambuf direction

                mspSerialEncode(msp, command);
            }

            msp->commandSenderFn = NULL;
        }
    }
}
#endif

/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */


#include "serial_proxy.h"

#if defined(SITL_BUILD)

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "platform.h"


#include <sys/types.h>
#include <fcntl.h>
#include <errno.h> 
#include <termios.h> 
#include <unistd.h>
#include <unistd.h>

#include "drivers/time.h"
#include "msp/msp_serial.h"
#include "msp/msp_protocol.h"
#include "common/crc.h"
#include "rx/sim.h"

#include "drivers/serial_tcp.h"

#define SYM_BEGIN       '$'
#define SYM_PROTO_V1    'M'
#define SYM_PROTO_V2    'X'
#define SYM_FROM_MWC    '>'
#define SYM_TO_MWC      '<'
#define SYM_UNSUPPORTED '!'

#define JUMBO_FRAME_MIN_SIZE  255
#define MAX_MSP_MESSAGE 1024
#define RX_CONFIG_SIZE 24

typedef enum
{
    DS_IDLE,
    DS_PROTO_IDENTIFIER,
    DS_DIRECTION_V1,
    DS_DIRECTION_V2,
    DS_FLAG_V2,
    DS_PAYLOAD_LENGTH_V1,
    DS_PAYLOAD_LENGTH_JUMBO_LOW,
    DS_PAYLOAD_LENGTH_JUMBO_HIGH,
    DS_PAYLOAD_LENGTH_V2_LOW,
    DS_PAYLOAD_LENGTH_V2_HIGH,
    DS_CODE_V1,
    DS_CODE_JUMBO_V1,
    DS_CODE_V2_LOW,
    DS_CODE_V2_HIGH,
    DS_PAYLOAD_V1,
    DS_PAYLOAD_V2,
    DS_CHECKSUM_V1,
    DS_CHECKSUM_V2,
} TDecoderState;

static TDecoderState decoderState = DS_IDLE;

typedef enum
{
    RXC_IDLE = 0,    
    RXC_RQ   = 1,
    RXC_DONE = 2
} TRXConfigState;

static TRXConfigState rxConfigState = RXC_IDLE;


static int message_length_expected;
static unsigned char message_buffer[MAX_MSP_MESSAGE];
static int message_length_received;
static int unsupported;
static int code;
static int message_direction;
static uint8_t message_checksum;
static int reqCount = 0;
static uint16_t rssi = 0;
static uint8_t rxConfigBuffer[RX_CONFIG_SIZE];

static timeMs_t lastWarning = 0;

int serialUartIndex = -1;
char serialPort[64] = "";
int serialBaudRate = 115200;
OptSerialStopBits_e serialStopBits = OPT_SERIAL_STOP_BITS_ONE;  //0:None|1:One|2:OnePointFive|3:Two 
OptSerialParity_e serialParity = OPT_SERIAL_PARITY_NONE;
bool serialFCProxy = false;

#define FC_PROXY_REQUEST_PERIOD_MIN_MS 20  //inav can handle 100 msp messages per second max
#define FC_PROXY_REQUEST_PERIOD_TIMEOUT_MS 200
#define FC_PROXY_REQUEST_PERIOD_RSSI_MS 300
#define SERIAL_BUFFER_SIZE 256

#if defined(__CYGWIN__)
#include <windows.h>
static HANDLE hSerial;
#else
static int fd;
#endif

static bool connected = false;
static bool started = false;

static timeMs_t nextProxyRequestTimeout = 0;
static timeMs_t nextProxyRequestMin = 0;
static timeMs_t nextProxyRequestRssi = 0;

static timeMs_t lastVisit = 0;

void serialProxyInit(void) {
    if ( strlen(serialPort) < 1) {
        return;
    }
    connected = false;

    char portName[64+20];
#if defined(__CYGWIN__)
    sprintf(portName, "\\\\.\\%s", serialPort);

    hSerial = CreateFile(portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            fprintf(stderr, "[SERIALPROXY] ERROR: Sserial port was not attached. Reason: %s not available.\n", portName);
        } else {
            fprintf(stderr, "[SERIALPROXY] ERROR: Can not connect to serial port, unknown error.\n");
        }
        return;
    } else {
        DCB dcbSerialParams = { 0 };
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            fprintf(stderr, "[SERIALPROXY] ALERT: failed to get current serial parameters!\n");
        } else {
            dcbSerialParams.BaudRate = serialBaudRate;
            dcbSerialParams.ByteSize = 8;

            switch (serialStopBits) {
                case OPT_SERIAL_STOP_BITS_ONE:
                    dcbSerialParams.StopBits = ONESTOPBIT;
                    break;
                case OPT_SERIAL_STOP_BITS_TWO:
                    dcbSerialParams.StopBits = TWOSTOPBITS;
                    break;
                case OPT_SERIAL_STOP_BITS_INVALID:
                    break;
            }

            switch (serialParity) {
                case OPT_SERIAL_PARITY_EVEN:
                    dcbSerialParams.Parity = EVENPARITY;
                    break;
                case OPT_SERIAL_PARITY_NONE:
                    dcbSerialParams.Parity = NOPARITY;
                    break;
                case OPT_SERIAL_PARITY_ODD:
                    dcbSerialParams.Parity = ODDPARITY;
                    break;
                case OPT_SERIAL_PARITY_INVALID:
                    break;
            }    

            if (!SetCommState(hSerial, &dcbSerialParams)) {
                fprintf(stderr, "[SERIALPROXY] ALERT: Could not set Serial Port parameters\n");
            } else {
                COMMTIMEOUTS comTimeOut;
                comTimeOut.ReadIntervalTimeout = MAXDWORD;
                comTimeOut.ReadTotalTimeoutMultiplier = 0;
                comTimeOut.ReadTotalTimeoutConstant = 0;
                comTimeOut.WriteTotalTimeoutMultiplier = 0;
                comTimeOut.WriteTotalTimeoutConstant = 0;
                SetCommTimeouts(hSerial, &comTimeOut);
            }
        }
    }
#else
    if ( serialPort[0] != '/') {
        sprintf(portName, "/%s", serialPort);
    } else {
        sprintf(portName, "%s", serialPort);
    }

    fd = open(portName, O_RDWR);
    if (fd == -1)
    {
        fprintf(stderr, "[SERIALPROXY] ERROR: Can not connect to serial port %s\n", portName);
        return;
    }

    struct termios terminalOptions;
    memset(&terminalOptions, 0, sizeof(struct termios));
    tcgetattr(fd, &terminalOptions);

    cfmakeraw(&terminalOptions);

    cfsetispeed(&terminalOptions, serialBaudRate);
    cfsetospeed(&terminalOptions, serialBaudRate);

    terminalOptions.c_cflag = CREAD | CLOCAL;
    terminalOptions.c_cflag |= CS8;
    terminalOptions.c_cflag &= ~HUPCL;

    terminalOptions.c_lflag &= ~ICANON;
    terminalOptions.c_lflag &= ~ECHO; // Disable echo
    terminalOptions.c_lflag &= ~ECHOE; // Disable erasure
    terminalOptions.c_lflag &= ~ECHONL; // Disable new-line echo
    terminalOptions.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    terminalOptions.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    terminalOptions.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    terminalOptions.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    terminalOptions.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    terminalOptions.c_cc[VMIN] = 0;
    terminalOptions.c_cc[VTIME] = 0;

    switch (serialStopBits) {
        case OPT_SERIAL_STOP_BITS_ONE:
            terminalOptions.c_cflag &= ~CSTOPB;  
            break;
        case OPT_SERIAL_STOP_BITS_TWO:
            terminalOptions.c_cflag |= CSTOPB;
            break;
        case OPT_SERIAL_STOP_BITS_INVALID:
            break;
    }

    switch (serialParity) {
        case OPT_SERIAL_PARITY_EVEN:
            terminalOptions.c_cflag |= PARENB;
            terminalOptions.c_cflag &= ~PARODD;
            break;
        case OPT_SERIAL_PARITY_NONE:
            terminalOptions.c_cflag &= ~PARENB;
            terminalOptions.c_cflag &= ~PARODD;
            break;
        case OPT_SERIAL_PARITY_ODD:
            terminalOptions.c_cflag |= PARENB;
            terminalOptions.c_cflag |= PARODD;
            break;
        case OPT_SERIAL_PARITY_INVALID:
            break;
    }    

    int ret = tcsetattr(fd, TCSANOW, &terminalOptions); 
    if (ret == -1)
    {
        fprintf(stderr, "[SERIALPROXY] ALERT: Failed to configure device: %s\n", portName);
        perror("tcsetattr");
        return;
    }
#endif
    connected = true;

    if ( !serialFCProxy ) {
        fprintf(stderr, "[SERIALPROXY] Connected %s to UART%d\n", portName, serialUartIndex);
    } else {
        fprintf(stderr, "[SERIALPROXY] Using proxy flight controller on %s\n", portName);
    }
}

void serialProxyStart(void) {
    started = true;
}

void serialProxyClose(void) {
    if (connected) {
        connected = false;
#if defined(__CYGWIN__)
        CloseHandle(hSerial);
#else
        close(fd);
#endif
        started = false;
        nextProxyRequestTimeout = 0;
        nextProxyRequestMin = 0;
        nextProxyRequestRssi = 0;
        lastWarning = 0;
        lastVisit = 0;
    }
}

static bool canOutputWarning(void) {
    if ( millis() > lastWarning ) {
        lastWarning = millis() + 5000;
        return true;
    }
    return false;
}

int serialProxyReadData(unsigned char *buffer, unsigned int nbChar) {
    if (!connected) return 0;

#if defined(__CYGWIN__)
    COMSTAT status;
    DWORD errors;
    DWORD bytesRead;

    ClearCommError(hSerial, &errors, &status);
    if (status.cbInQue>0) {
        unsigned int toRead = (status.cbInQue>nbChar) ? nbChar : status.cbInQue;
        if (ReadFile(hSerial, buffer, toRead, &bytesRead, NULL) && (bytesRead != 0)) {
            return bytesRead;
        }
    }
    return 0;
#else
    if (nbChar == 0) return 0;
    int bytesRead = read(fd, buffer, nbChar);
    return bytesRead;
#endif    
}

bool serialProxyWriteData(unsigned char *buffer, unsigned int nbChar) {
  if (!connected) return false;

#if defined(__CYGWIN__)
        COMSTAT status;
        DWORD errors;
        DWORD bytesSent;
        if (!WriteFile(hSerial, (void *)buffer, nbChar, &bytesSent, 0)) {
            ClearCommError(hSerial, &errors, &status);
            if ( canOutputWarning() ) {
                fprintf(stderr, "[SERIALPROXY] ERROR: Can not write to serial port\n");
            }
            return false;
        }
        if ( bytesSent != nbChar ) {
            if ( canOutputWarning() ) {
                fprintf(stderr, "[SERIALPROXY] ERROR: Can not write to serial port\n");
            }
        }
#else
        ssize_t l = write(fd, buffer, nbChar);
        if ( l!= nbChar ) {
            if ( canOutputWarning() ) {
                fprintf(stderr, "[SERIALPROXY] ERROR: Can not write to serial port\n");
            }
            return false;
        }
#endif    
    return true;
}

bool serialProxyIsConnected(void) {
    return connected;
}

static void mspSendCommand(int cmd, unsigned char* data, int dataLen) {
    uint8_t msgBuf[MAX_MSP_MESSAGE] = { '$', 'X', '<' };
    int msgLen = 3;

    mspHeaderV2_t* hdrV2 = (mspHeaderV2_t *)&msgBuf[msgLen];
    hdrV2->flags = 0;
    hdrV2->cmd = cmd;
    hdrV2->size = dataLen;
    msgLen += sizeof(mspHeaderV2_t);

    for ( int i =0; i < dataLen; i++ ) {
        msgBuf[msgLen++] = data[i];    
    }

    uint8_t crc = crc8_dvb_s2_update(0, (uint8_t *)hdrV2, sizeof(mspHeaderV2_t));
    crc = crc8_dvb_s2_update(crc, data,dataLen);
    msgBuf[msgLen] = crc;
    msgLen++;

    serialProxyWriteData((unsigned char *)&msgBuf, msgLen);
}


static void mspRequestChannels(void) {
    mspSendCommand(MSP_RC, NULL, 0);
}

static void mspRequestRssi(void){
    mspSendCommand(MSP_ANALOG, NULL, 0);
}

static void requestRXConfigState(void) {
    mspSendCommand(MSP_RX_CONFIG, NULL, 0);
    rxConfigState = RXC_RQ;
    fprintf(stderr, "[SERIALPROXY] Requesting RX config from proxy FC...\n");
}

static void processMessage(void) {
    if ( code == MSP_RC ) {
        if ( reqCount > 0 ) reqCount--;
        int count = message_length_received / 2;
        if ( count <= MAX_SUPPORTED_RC_CHANNEL_COUNT) {
            uint16_t* channels = (uint16_t*)(&message_buffer[0]);
            //AETR => AERT
            uint v = channels[2];
            channels[2] = channels[3];
            channels[3] = v;
            if ( rssi > 0 ) {
                rxSimSetChannelValue(channels, count);
            }
        }
    } else if ( code == MSP_ANALOG ) {
        if ( reqCount > 0 ) reqCount--;
        if ( message_length_received >=7 ) {
            rssi = *((uint16_t*)(&message_buffer[3]));
            rxSimSetRssi( rssi );
        }
    } else if ( code == MSP_RX_CONFIG ) {
        memcpy( &(rxConfigBuffer[0]), &(message_buffer[0]), RX_CONFIG_SIZE );
        *((uint16_t*)&(rxConfigBuffer[1])) = 2500; //maxcheck
        *((uint16_t*)&(rxConfigBuffer[5])) = 500; //mincheck

        mspSendCommand( MSP_SET_RX_CONFIG, rxConfigBuffer, RX_CONFIG_SIZE );
        rxConfigState = RXC_DONE;
        fprintf(stderr, "[SERIALPROXY] Setting RX config on proxy FC...\n");
    }
}

static void decodeProxyMessages(unsigned char* data, int len) {
  for (int i = 0; i < len; i++) {
    switch (decoderState) {
        case DS_IDLE: // sync char 1
        if (data[i] == SYM_BEGIN) {
            decoderState = DS_PROTO_IDENTIFIER;
        }
        break;

        case DS_PROTO_IDENTIFIER: // sync char 2
        switch (data[i]) {
            case SYM_PROTO_V1:
                decoderState = DS_DIRECTION_V1;
                break;
            case SYM_PROTO_V2:
                decoderState = DS_DIRECTION_V2;
                break;
            default:
                //unknown protocol
                decoderState = DS_IDLE;
        }
        break;

        case DS_DIRECTION_V1: // direction (should be >)

        case DS_DIRECTION_V2:
            unsupported = 0;
            switch (data[i]) {
                case SYM_FROM_MWC:
                    message_direction = 1;
                    break;
                case SYM_TO_MWC:
                    message_direction = 0;
                    break;
                case SYM_UNSUPPORTED:
                    unsupported = 1;
                    break;
            }
            decoderState = decoderState == DS_DIRECTION_V1 ? DS_PAYLOAD_LENGTH_V1 : DS_FLAG_V2;
        break;

        case DS_FLAG_V2:
            // Ignored for now
            decoderState = DS_CODE_V2_LOW;
            break;

        case DS_PAYLOAD_LENGTH_V1:
            message_length_expected = data[i];

            if (message_length_expected == JUMBO_FRAME_MIN_SIZE) {
                decoderState = DS_CODE_JUMBO_V1;
            } else {
                message_length_received = 0;
                decoderState = DS_CODE_V1;
            }
            break;

        case DS_PAYLOAD_LENGTH_V2_LOW:
            message_length_expected = data[i];
            decoderState = DS_PAYLOAD_LENGTH_V2_HIGH;
            break;

        case DS_PAYLOAD_LENGTH_V2_HIGH:
            message_length_expected |= data[i] << 8;
            message_length_received = 0;
            if (message_length_expected <= MAX_MSP_MESSAGE) {
                decoderState = message_length_expected > 0 ? DS_PAYLOAD_V2 : DS_CHECKSUM_V2;
            } else {
                //too large payload
                decoderState = DS_IDLE;
            }
            break;

        case DS_CODE_V1:
        case DS_CODE_JUMBO_V1:
            code = data[i];
            if (message_length_expected > 0) {
                // process payload
                if (decoderState == DS_CODE_JUMBO_V1) {
                    decoderState = DS_PAYLOAD_LENGTH_JUMBO_LOW;
                } else {
                    decoderState = DS_PAYLOAD_V1;
                }
            } else {
                // no payload
                decoderState = DS_CHECKSUM_V1;
            }
            break;

        case DS_CODE_V2_LOW:
            code = data[i];
            decoderState = DS_CODE_V2_HIGH;
            break;

        case DS_CODE_V2_HIGH:
            code |= data[i] << 8;
            decoderState = DS_PAYLOAD_LENGTH_V2_LOW;
            break;

        case DS_PAYLOAD_LENGTH_JUMBO_LOW:
            message_length_expected = data[i];
            decoderState = DS_PAYLOAD_LENGTH_JUMBO_HIGH;
            break;

        case DS_PAYLOAD_LENGTH_JUMBO_HIGH:
            message_length_expected |= data[i] << 8;
            message_length_received = 0;
            decoderState = DS_PAYLOAD_V1;
            break;

        case DS_PAYLOAD_V1:
        case DS_PAYLOAD_V2:
            message_buffer[message_length_received] = data[i];
            message_length_received++;

            if (message_length_received >= message_length_expected) {
                decoderState = decoderState == DS_PAYLOAD_V1 ? DS_CHECKSUM_V1 : DS_CHECKSUM_V2;
            }
            break;

        case DS_CHECKSUM_V1:
            if (message_length_expected >= JUMBO_FRAME_MIN_SIZE) {
                message_checksum = JUMBO_FRAME_MIN_SIZE;
            } else {
                message_checksum = message_length_expected;
            }
            message_checksum ^= code;
            if (message_length_expected >= JUMBO_FRAME_MIN_SIZE) {
                message_checksum ^= message_length_expected & 0xFF;
                message_checksum ^= (message_length_expected & 0xFF00) >> 8;
            }
            for (int ii = 0; ii < message_length_received; ii++) {
                message_checksum ^= message_buffer[ii];
            }
            if (message_checksum == data[i]) {
                processMessage();
            }
            decoderState = DS_IDLE;
            break;

        case DS_CHECKSUM_V2:
            message_checksum = 0;
            message_checksum = crc8_dvb_s2(message_checksum, 0); // flag
            message_checksum = crc8_dvb_s2(message_checksum, code & 0xFF);
            message_checksum = crc8_dvb_s2(message_checksum, (code & 0xFF00) >> 8);
            message_checksum = crc8_dvb_s2(message_checksum, message_length_expected & 0xFF);
            message_checksum = crc8_dvb_s2(message_checksum, (message_length_expected & 0xFF00) >> 8);
            for (int ii = 0; ii < message_length_received; ii++) {
                message_checksum = crc8_dvb_s2(message_checksum, message_buffer[ii]);
            }
            if (message_checksum == data[i]) {
                processMessage();
            }
            decoderState = DS_IDLE;
            break;

        default:
        break;
    }
  }
}

void serialProxyProcess(void) {

    if (!started) return;
    if (!connected) return;

    if ((millis() - lastVisit) > 500) {
        if ( lastVisit > 0 ) {
            fprintf(stderr, "timeout=%dms\n", millis() - lastVisit);
        }
    }
    lastVisit = millis();

    if ( serialFCProxy ) {
        //first we have to change FC min_check and max_check thresholds to avoid activating stick commands on proxy FC
        if ( rxConfigState == RXC_IDLE ) { 
            requestRXConfigState();
        } else if ( rxConfigState == RXC_DONE ) {
            if ( (nextProxyRequestTimeout <= millis()) || ((reqCount == 0) && (nextProxyRequestMin <= millis()))) {
                nextProxyRequestTimeout = millis() + FC_PROXY_REQUEST_PERIOD_TIMEOUT_MS;
                nextProxyRequestMin = millis() + FC_PROXY_REQUEST_PERIOD_MIN_MS;
                mspRequestChannels();
                reqCount++;
            }
            
            if ( nextProxyRequestRssi <= millis()) {
                nextProxyRequestRssi = millis() + FC_PROXY_REQUEST_PERIOD_RSSI_MS;
                mspRequestRssi();
                reqCount++;
            }
        }

        unsigned char buf[SERIAL_BUFFER_SIZE];
        int count = serialProxyReadData(buf, SERIAL_BUFFER_SIZE);
        if (count == 0) return;
        decodeProxyMessages(buf, count);

    } else {

        if ( serialUartIndex == -1 )  return;
        unsigned char buf[SERIAL_BUFFER_SIZE];
        uint32_t avail = tcpRXBytesFree(serialUartIndex-1);
        if ( avail == 0 ) return;
        if (avail > SERIAL_BUFFER_SIZE) avail = SERIAL_BUFFER_SIZE;

        int count = serialProxyReadData(buf, avail);
        if (count == 0) return;

        tcpReceiveBytesEx( serialUartIndex-1, buf, count);
    }
    
}

#endif
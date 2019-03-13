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
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#if defined(USE_GPS) && (defined(USE_GPS_PROTO_NMEA) || defined(USE_GPS_PROTO_MTK))

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/gps_conversion.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/serial.h"
#include "io/gps.h"
#include "io/gps_private.h"

#include "scheduler/protothreads.h"

/* This is a light implementation of a GPS frame decoding
   This should work with most of modern GPS devices configured to output 5 frames.
   It assumes there are some NMEA GGA frames to decode on the serial bus
   Now verifies checksum correctly before applying data

   Here we use only the following data :
     - latitude
     - longitude
     - GPS fix is/is not ok
     - GPS num sat (4 is enough to be +/- reliable)
     // added by Mis
     - GPS altitude (for OSD displaying)
     - GPS speed (for OSD displaying)
*/

#define NO_FRAME   0
#define FRAME_GGA  1
#define FRAME_RMC  2

static uint32_t grab_fields(char *src, uint8_t mult)
{                               // convert string to uint32
    uint32_t i;
    uint32_t tmp = 0;
    for (i = 0; src[i] != 0; i++) {
        if (src[i] == '.') {
            i++;
            if (mult == 0)
                break;
            else
                src[i + mult] = 0;
        }
        tmp *= 10;
        if (src[i] >= '0' && src[i] <= '9')
            tmp += src[i] - '0';
        if (i >= 15)
            return 0; // out of bounds
    }
    return tmp;
}

typedef struct gpsDataNmea_s {
    bool fix;
    int32_t latitude;
    int32_t longitude;
    uint8_t numSat;
    int32_t altitude;
    uint16_t speed;
    uint16_t ground_course;
    uint16_t hdop;
    uint32_t time;
    uint32_t date;
} gpsDataNmea_t;

#define NMEA_BUFFER_SIZE        16

static bool gpsNewFrameNMEA(char c)
{
    static gpsDataNmea_t gps_Msg;

    uint8_t frameOK = 0;
    static uint8_t param = 0, offset = 0, parity = 0;
    static char string[NMEA_BUFFER_SIZE];
    static uint8_t checksum_param, gps_frame = NO_FRAME;

    switch (c) {
        case '$':
            param = 0;
            offset = 0;
            parity = 0;
            break;
        case ',':
        case '*':
            string[offset] = 0;
            if (param == 0) {       //frame identification
                gps_frame = NO_FRAME;
                if (strcmp(string, "GPGGA") == 0 || strcmp(string, "GNGGA") == 0) {
                    gps_frame = FRAME_GGA;
                }
                else if (strcmp(string, "GPRMC") == 0 || strcmp(string, "GNRMC") == 0) {
                    gps_frame = FRAME_RMC;
                }
            }

            switch (gps_frame) {
                case FRAME_GGA:        //************* GPGGA FRAME parsing
                    switch (param) {
            //          case 1:             // Time information
            //              break;
                        case 2:
                            gps_Msg.latitude = GPS_coord_to_degrees(string);
                            break;
                        case 3:
                            if (string[0] == 'S')
                                gps_Msg.latitude *= -1;
                            break;
                        case 4:
                            gps_Msg.longitude = GPS_coord_to_degrees(string);
                            break;
                        case 5:
                            if (string[0] == 'W')
                                gps_Msg.longitude *= -1;
                            break;
                        case 6:
                            if (string[0] > '0') {
                                gps_Msg.fix = true;
                            } else {
                                gps_Msg.fix = false;
                            }
                            break;
                        case 7:
                            gps_Msg.numSat = grab_fields(string, 0);
                            break;
                        case 8:
                            gps_Msg.hdop = grab_fields(string, 1) * 10;          // hdop
                            break;
                        case 9:
                            gps_Msg.altitude = grab_fields(string, 1) * 10;     // altitude in cm
                            break;
                    }
                    break;
                case FRAME_RMC:        //************* GPRMC FRAME parsing
                                       // $GNRMC,130059.00,V,,,,,,,110917,,,N*62
                    switch (param) {
                        case 1:
                            gps_Msg.time = grab_fields(string, 2);
                            break;
                        case 7:
                            gps_Msg.speed = ((grab_fields(string, 1) * 5144L) / 1000L);    // speed in cm/s added by Mis
                            break;
                        case 8:
                            gps_Msg.ground_course = (grab_fields(string, 1));      // ground course deg * 10
                            break;
                        case 9:
                            gps_Msg.date = grab_fields(string, 0);
                            break;
                    }
                    break;
            }

            param++;
            offset = 0;
            if (c == '*')
                checksum_param = 1;
            else
                parity ^= c;
            break;
        case '\r':
        case '\n':
            if (checksum_param) {   //parity checksum
                uint8_t checksum = 16 * ((string[0] >= 'A') ? string[0] - 'A' + 10 : string[0] - '0') + ((string[1] >= 'A') ? string[1] - 'A' + 10 : string[1] - '0');
                if (checksum == parity) {
                    gpsStats.packetCount++;
                    switch (gps_frame) {
                    case FRAME_GGA:
                        frameOK = 1;
                        gpsSol.numSat = gps_Msg.numSat;
                        if (gps_Msg.fix) {
                            gpsSol.fixType = GPS_FIX_3D;    // NMEA doesn't report fix type, assume 3D

                            gpsSol.llh.lat = gps_Msg.latitude;
                            gpsSol.llh.lon = gps_Msg.longitude;
                            gpsSol.llh.alt = gps_Msg.altitude;

                            // EPH/EPV are unreliable for NMEA as they are not real accuracy
                            gpsSol.hdop = gpsConstrainHDOP(gps_Msg.hdop);
                            gpsSol.eph = gpsConstrainEPE(gps_Msg.hdop * GPS_HDOP_TO_EPH_MULTIPLIER);
                            gpsSol.epv = gpsConstrainEPE(gps_Msg.hdop * GPS_HDOP_TO_EPH_MULTIPLIER);
                            gpsSol.flags.validEPE = 0;
                        }
                        else {
                            gpsSol.fixType = GPS_NO_FIX;
                        }

                        // NMEA does not report VELNED
                        gpsSol.flags.validVelNE = 0;
                        gpsSol.flags.validVelD = 0;
                        break;
                    case FRAME_RMC:
                        gpsSol.groundSpeed = gps_Msg.speed;
                        gpsSol.groundCourse = gps_Msg.ground_course;

                        // This check will miss 00:00:00.00, but we shouldn't care - next report will be valid
                        if (gps_Msg.date != 0 && gps_Msg.time != 0) {
                            gpsSol.time.year = (gps_Msg.date % 100) + 2000;
                            gpsSol.time.month = (gps_Msg.date / 100) % 100;
                            gpsSol.time.day = (gps_Msg.date / 10000) % 100;
                            gpsSol.time.hours = (gps_Msg.time / 1000000) % 100;
                            gpsSol.time.minutes = (gps_Msg.time / 10000) % 100;
                            gpsSol.time.seconds = (gps_Msg.time / 100) % 100;
                            gpsSol.time.millis = (gps_Msg.time & 100) * 10;
                            gpsSol.flags.validTime = 1;
                        }
                        else {
                            gpsSol.flags.validTime = 0;
                        }

                        break;
                    } // end switch
                }
                else {
                    gpsStats.errors++;
                }
            }
            checksum_param = 0;
            break;
        default:
            if (offset < (NMEA_BUFFER_SIZE-1)) {    // leave 1 byte to trailing zero
                string[offset++] = c;

                // only checksum if character is recorded and used (will cause checksum failure on dropped characters)
                if (!checksum_param)
                    parity ^= c;
            }
    }
    return frameOK;
}

static ptSemaphore_t semNewDataReady;

STATIC_PROTOTHREAD(gpsProtocolReceiverThread)
{
    ptBegin(gpsProtocolReceiverThread);

    while (1) {
        // Wait until there are bytes to consume
        ptWait(serialRxBytesWaiting(gpsState.gpsPort));

        // Consume bytes until buffer empty of until we have full message received
        while (serialRxBytesWaiting(gpsState.gpsPort)) {
            uint8_t newChar = serialRead(gpsState.gpsPort);
            if (gpsNewFrameNMEA(newChar)) {
                gpsSol.flags.validVelNE = 0;
                gpsSol.flags.validVelD = 0;
                ptSemaphoreSignal(semNewDataReady);
                break;
            }
        }
    }

    ptEnd(0);
}

#ifdef USE_GPS_PROTO_MTK
static const char * mtkBaudInitData[GPS_BAUDRATE_COUNT] = {
    "$PMTK251,115200*1F\r\n",     // GPS_BAUDRATE_115200
    "$PMTK251,57600*2C\r\n",      // GPS_BAUDRATE_57600
    "$PMTK251,38400*27\r\n",      // GPS_BAUDRATE_38400
    "$PMTK251,19200*22\r\n",      // GPS_BAUDRATE_19200
    "$PMTK251,9600*17\r\n"        // GPS_BAUDRATE_9600
};

STATIC_PROTOTHREAD(gpsProtocolStateThreadMTK)
{
    ptBegin(gpsProtocolStateThreadMTK);

    // Change baud rate
    ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));
    if (gpsState.gpsConfig->autoBaud != GPS_AUTOBAUD_OFF) {
        // Autobaud logic:
        //  0. Wait for TX buffer to be empty
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        //  1. Set serial port to baud rate specified by [autoBaudrateIndex]
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.autoBaudrateIndex]]);
        gpsState.autoBaudrateIndex = (gpsState.autoBaudrateIndex + 1) % GPS_BAUDRATE_COUNT;

        //  2. Send an $UBX command to switch the baud rate specified by portConfig [baudrateIndex]
        serialPrint(gpsState.gpsPort, mtkBaudInitData[gpsState.baudrateIndex]);

        //  3. Wait for command to be received and processed by GPS
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        //  4. Switch to [baudrateIndex]
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.baudrateIndex]]);

        //  5. Attempt to configure the GPS
        ptDelayMs(GPS_BAUD_CHANGE_DELAY);
    }
    else {
        // Set baud rate
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.baudrateIndex]]);
    }

    // Send configuration commands
    if (gpsState.gpsConfig->autoConfig) {
        // Disable all messages except GGA and RMC
        serialPrint(gpsState.gpsPort, "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28\r\n");
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        // Set Nav Threshold (the minimum speed the GPS must be moving to update the position) to 0 m/s
        serialPrint(gpsState.gpsPort, "$PMTK397,0*23\r\n");
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        // SBAS/WAAS
        if (gpsState.gpsConfig->sbasMode != SBAS_NONE) {
            serialPrint(gpsState.gpsPort, "$PMTK313,1*2E\r\n");     // SBAS ON
            ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

            serialPrint(gpsState.gpsPort, "$PMTK301,2*2E\r\n");     // WAAS ON
            ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));
        }
        else {
            serialPrint(gpsState.gpsPort, "$PMTK313,0*2F\r\n");     // SBAS OFF
            ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

            serialPrint(gpsState.gpsPort, "$PMTK301,0*2C\r\n");     // WAAS OFF
            ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));
        }

        // 5Hz update, should works for most modules
        serialPrint(gpsState.gpsPort, "$PMTK220,200*2C\r\n");
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        // Try set 10Hz update rate. Module will ignore it if can't support
        serialPrint(gpsState.gpsPort, "$PMTK220,100*2F\r\n");
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));
    }

    // Reset protocol timeout
    gpsSetProtocolTimeout(GPS_TIMEOUT);

    // GPS is ready - execute the gpsProcessNewSolutionData() based on gpsProtocolReceiverThread semaphore
    while (1) {
        ptSemaphoreWait(semNewDataReady);
        gpsProcessNewSolutionData();
    }

    ptEnd(0);
}

void gpsHandleMTK(void)
{
    // Run the protocol threads
    gpsProtocolReceiverThread();
    gpsProtocolStateThreadMTK();

    // If thread stopped - signal communication loss and restart
    if (ptIsStopped(ptGetHandle(gpsProtocolReceiverThread)) || ptIsStopped(ptGetHandle(gpsProtocolStateThreadMTK))) {
        gpsSetState(GPS_LOST_COMMUNICATION);
    }
}
#endif

STATIC_PROTOTHREAD(gpsProtocolStateThreadNMEA)
{
    ptBegin(gpsProtocolStateThreadNMEA);

    // Change baud rate
    ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));
    if (gpsState.gpsConfig->autoBaud != GPS_AUTOBAUD_OFF) {
        // Cycle through available baud rates and hope that we will match GPS
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.autoBaudrateIndex]]);
        gpsState.autoBaudrateIndex = (gpsState.autoBaudrateIndex + 1) % GPS_BAUDRATE_COUNT;
        ptDelayMs(GPS_BAUD_CHANGE_DELAY);
    }
    else {
        // Set baud rate
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.baudrateIndex]]);
    }

    // No configuration is done for pure NMEA modules

    // GPS setup done, reset timeout
    gpsSetProtocolTimeout(GPS_TIMEOUT);

    // GPS is ready - execute the gpsProcessNewSolutionData() based on gpsProtocolReceiverThread semaphore
    while (1) {
        ptSemaphoreWait(semNewDataReady);
        gpsProcessNewSolutionData();
    }

    ptEnd(0);
}

void gpsHandleNMEA(void)
{
    // Run the protocol threads
    gpsProtocolReceiverThread();
    gpsProtocolStateThreadNMEA();

    // If thread stopped - signal communication loss and restart
    if (ptIsStopped(ptGetHandle(gpsProtocolReceiverThread)) || ptIsStopped(ptGetHandle(gpsProtocolStateThreadNMEA))) {
        gpsSetState(GPS_LOST_COMMUNICATION);
    }
}

void gpsRestartNMEA_MTK(void)
{
    ptSemaphoreInit(semNewDataReady);
    ptRestart(ptGetHandle(gpsProtocolReceiverThread));
    ptRestart(ptGetHandle(gpsProtocolStateThreadNMEA));
#ifdef USE_GPS_PROTO_MTK
    ptRestart(ptGetHandle(gpsProtocolStateThreadMTK));
#endif
}

#endif

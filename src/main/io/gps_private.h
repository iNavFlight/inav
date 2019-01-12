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

#pragma once

#ifdef USE_GPS

#include "io/serial.h"

#define GPS_HDOP_TO_EPH_MULTIPLIER      2   // empirical value

// GPS timeout for wrong baud rate/disconnection/etc in milliseconds (default 2000 ms)
#define GPS_TIMEOUT             (1000)
#define GPS_SHORT_TIMEOUT       (500)
#define GPS_BAUD_CHANGE_DELAY   (100)
#define GPS_INIT_DELAY          (500)
#define GPS_BOOT_DELAY          (3000)

typedef enum {
    GPS_UNKNOWN,                // 0
    GPS_INITIALIZING,           // 1
    GPS_RUNNING,                // 2
    GPS_LOST_COMMUNICATION,     // 3
} gpsState_e;

typedef struct {
    const gpsConfig_t *   gpsConfig;
    const serialConfig_t * serialConfig;
    serialPort_t *  gpsPort;                // Serial GPS only

    uint32_t        hwVersion;

    gpsState_e      state;
    gpsBaudRate_e   baudrateIndex;
    gpsBaudRate_e   autoBaudrateIndex;      // Driver internal use (for autoBaud)
    uint8_t         autoConfigStep;         // Driver internal use (for autoConfig)

    timeMs_t        lastStateSwitchMs;
    timeMs_t        lastLastMessageMs;
    timeMs_t        lastMessageMs;
    timeMs_t        timeoutMs;
} gpsReceiverData_t;

extern gpsReceiverData_t gpsState;

extern baudRate_e gpsToSerialBaudRate[GPS_BAUDRATE_COUNT];

extern void gpsSetState(gpsState_e state);
extern void gpsFinalizeChangeBaud(void);

extern uint16_t gpsConstrainEPE(uint32_t epe);
extern uint16_t gpsConstrainHDOP(uint32_t hdop);

void gpsProcessNewSolutionData(void);
void gpsSetProtocolTimeout(timeMs_t timeoutMs);

extern void gpsRestartUBLOX(void);
extern void gpsHandleUBLOX(void);

extern void gpsRestartNMEA_MTK(void);
extern void gpsHandleNMEA(void);
extern void gpsHandleMTK(void);

extern void gpsRestartNAZA(void);
extern void gpsHandleNAZA(void);

#endif

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
#include <math.h>

#include "platform.h"
#include "build/build_config.h"


#ifdef USE_GPS

#include "build/debug.h"

#include "common/maths.h"
#include "common/axis.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/compass/compass.h"
#include "drivers/light_led.h"
#include "drivers/serial.h"
#include "drivers/system.h"
#include "drivers/time.h"

#if defined(USE_FAKE_GPS)
#include "fc/runtime_config.h"
#endif

#include "sensors/sensors.h"
#include "sensors/compass.h"

#include "io/serial.h"
#include "io/gps.h"
#include "io/gps_private.h"

#include "navigation/navigation.h"

#include "config/feature.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

typedef struct {
    bool                isDriverBased;
    portMode_t          portMode;           // Port mode RX/TX (only for serial based)
    void                (*restart)(void);   // Restart protocol driver thread
    void                (*protocol)(void);  // Process protocol driver thread
} gpsProviderDescriptor_t;

// GPS public data
gpsReceiverData_t gpsState;
gpsStatistics_t   gpsStats;
gpsSolutionData_t gpsSol;

// Map gpsBaudRate_e index to baudRate_e
baudRate_e gpsToSerialBaudRate[GPS_BAUDRATE_COUNT] = { BAUD_115200, BAUD_57600, BAUD_38400, BAUD_19200, BAUD_9600, BAUD_230400 };

static gpsProviderDescriptor_t gpsProviders[GPS_PROVIDER_COUNT] = {
    /* NMEA GPS */
#ifdef USE_GPS_PROTO_NMEA
    { false, MODE_RX, gpsRestartNMEA, &gpsHandleNMEA },
#else
    { false, 0, NULL, NULL },
#endif

    /* UBLOX binary */
#ifdef USE_GPS_PROTO_UBLOX
    { false, MODE_RXTX, &gpsRestartUBLOX, &gpsHandleUBLOX },
#else
    { false, 0, NULL, NULL },
#endif

    /* UBLOX7PLUS binary */
#ifdef USE_GPS_PROTO_UBLOX
    { false, MODE_RXTX, &gpsRestartUBLOX, &gpsHandleUBLOX },
#else
    { false, 0,  NULL, NULL },
#endif

    /* MSP GPS */
#ifdef USE_GPS_PROTO_MSP
    { true, 0, &gpsRestartMSP, &gpsHandleMSP },
#else
    { false, 0, NULL, NULL },
#endif

#ifdef USE_GPS_FAKE
    {true, 0, &gpsFakeRestart, &gpsFakeHandle},
#else
    { false, 0, NULL, NULL },
#endif

};

PG_REGISTER_WITH_RESET_TEMPLATE(gpsConfig_t, gpsConfig, PG_GPS_CONFIG, 2);

PG_RESET_TEMPLATE(gpsConfig_t, gpsConfig,
    .provider = SETTING_GPS_PROVIDER_DEFAULT,
    .sbasMode = SETTING_GPS_SBAS_MODE_DEFAULT,
    .autoConfig = SETTING_GPS_AUTO_CONFIG_DEFAULT,
    .autoBaud = SETTING_GPS_AUTO_BAUD_DEFAULT,
    .dynModel = SETTING_GPS_DYN_MODEL_DEFAULT,
    .gpsMinSats = SETTING_GPS_MIN_SATS_DEFAULT,
    .ubloxUseGalileo = SETTING_GPS_UBLOX_USE_GALILEO_DEFAULT,
    .ubloxNavHz = SETTING_GPS_UBLOX7_NAV_HZ_DEFAULT
);

void gpsSetState(gpsState_e state)
{
    gpsState.state = state;
    gpsState.lastStateSwitchMs = millis();
}

static void gpsUpdateTime(void)
{
    if (!rtcHasTime() && gpsSol.flags.validTime && gpsSol.time.year != 0) {
        rtcSetDateTime(&gpsSol.time);
    }
}

void gpsSetProtocolTimeout(timeMs_t timeoutMs)
{
    gpsState.lastLastMessageMs = gpsState.lastMessageMs;
    gpsState.lastMessageMs = millis();
    gpsState.timeoutMs = timeoutMs;
}

void gpsProcessNewSolutionData(void)
{
    // Set GPS fix flag only if we have 3D fix
    if (gpsSol.fixType == GPS_FIX_3D && gpsSol.numSat >= gpsConfig()->gpsMinSats) {
        ENABLE_STATE(GPS_FIX);
    }
    else {
        /* When no fix available - reset flags as well */
        gpsSol.flags.validVelNE = false;
        gpsSol.flags.validVelD = false;
        gpsSol.flags.validEPE = false;
        DISABLE_STATE(GPS_FIX);
    }

    // Set sensor as ready and available
    sensorsSet(SENSOR_GPS);

    // Pass on GPS update to NAV and IMU
    onNewGPSData();

    // Update time
    gpsUpdateTime();

    // Update timeout
    gpsSetProtocolTimeout(gpsState.baseTimeoutMs);

    // Update statistics
    gpsStats.lastMessageDt = gpsState.lastMessageMs - gpsState.lastLastMessageMs;
    gpsSol.flags.hasNewData = true;

    // Toggle heartbeat
    gpsSol.flags.gpsHeartbeat = !gpsSol.flags.gpsHeartbeat;
}

static void gpsResetSolution(void)
{
    gpsSol.eph = 9999;
    gpsSol.epv = 9999;
    gpsSol.numSat = 0;
    gpsSol.hdop = 9999;

    gpsSol.fixType = GPS_NO_FIX;

    gpsSol.flags.validVelNE = false;
    gpsSol.flags.validVelD = false;
    gpsSol.flags.validMag = false;
    gpsSol.flags.validEPE = false;
    gpsSol.flags.validTime = false;
}

void gpsPreInit(void)
{
    // Make sure gpsProvider is known when gpsMagDetect is called
    gpsState.gpsConfig = gpsConfig();
    gpsState.baseTimeoutMs = (gpsState.gpsConfig->provider == GPS_NMEA) ? GPS_TIMEOUT*2 : GPS_TIMEOUT;
}

void gpsInit(void)
{
    gpsState.serialConfig = serialConfig();
    gpsState.gpsConfig = gpsConfig();

    gpsStats.errors = 0;
    gpsStats.timeouts = 0;

    // Reset solution, timeout and prepare to start
    gpsResetSolution();
    gpsSetProtocolTimeout(gpsState.baseTimeoutMs);
    gpsSetState(GPS_UNKNOWN);

    // If given GPS provider has protocol() function not defined - we can't use it
    if (!gpsProviders[gpsState.gpsConfig->provider].protocol) {
        featureClear(FEATURE_GPS);
        return;
    }

    // Shortcut for driver-based GPS (MSP)
    if (gpsProviders[gpsState.gpsConfig->provider].isDriverBased) {
        gpsSetState(GPS_INITIALIZING);
        return;
    }

    serialPortConfig_t * gpsPortConfig = findSerialPortConfig(FUNCTION_GPS);
    if (!gpsPortConfig) {
        featureClear(FEATURE_GPS);
        return;
    }

    // Start with baud rate index as configured for serial port
    int baudrateIndex;
    gpsState.baudrateIndex = GPS_BAUDRATE_115200;
    for (baudrateIndex = 0, gpsState.baudrateIndex = 0; baudrateIndex < GPS_BAUDRATE_COUNT; baudrateIndex++) {
        if (gpsToSerialBaudRate[baudrateIndex] == gpsPortConfig->gps_baudrateIndex) {
            gpsState.baudrateIndex = baudrateIndex;
            break;
        }
    }

    // Start with the same baud for autodetection
    gpsState.autoBaudrateIndex = gpsState.baudrateIndex;

    // Open serial port
    portMode_t mode = gpsProviders[gpsState.gpsConfig->provider].portMode;
    gpsState.gpsPort = openSerialPort(gpsPortConfig->identifier, FUNCTION_GPS, NULL, NULL, baudRates[gpsToSerialBaudRate[gpsState.baudrateIndex]], mode, SERIAL_NOT_INVERTED);

    // Check if we have a serial port opened
    if (!gpsState.gpsPort) {
        featureClear(FEATURE_GPS);
        return;
    }

    gpsSetState(GPS_INITIALIZING);
}

uint16_t gpsConstrainEPE(uint32_t epe)
{
    return (epe > 9999) ? 9999 : epe; // max 99.99m error
}

uint16_t gpsConstrainHDOP(uint32_t hdop)
{
    return (hdop > 9999) ? 9999 : hdop; // max 99.99m error
}

bool gpsUpdate(void)
{
    // Sanity check
    if (!feature(FEATURE_GPS)) {
        sensorsClear(SENSOR_GPS);
        DISABLE_STATE(GPS_FIX);
        return false;
    }

    /* Extra delay for at least 2 seconds after booting to give GPS time to initialise */
    if (!isMPUSoftReset() && (millis() < GPS_BOOT_DELAY)) {
        sensorsClear(SENSOR_GPS);
        DISABLE_STATE(GPS_FIX);
        return false;
    }

#ifdef USE_SIMULATOR
    if (ARMING_FLAG(SIMULATOR_MODE_HITL)) {
        gpsUpdateTime();
        gpsSetState(GPS_RUNNING);
        sensorsSet(SENSOR_GPS);
        return gpsSol.flags.hasNewData;
    }
#endif

    // Assume that we don't have new data this run
    gpsSol.flags.hasNewData = false;

    switch (gpsState.state) {
    default:
    case GPS_INITIALIZING:
        // Wait for GPS_INIT_DELAY before starting the GPS protocol thread
        if ((millis() - gpsState.lastStateSwitchMs) >= GPS_INIT_DELAY) {
            // Reset internals
            DISABLE_STATE(GPS_FIX);
            gpsSol.fixType = GPS_NO_FIX;

            // Reset solution
            gpsResetSolution();

            // Call GPS protocol reset handler
            gpsProviders[gpsState.gpsConfig->provider].restart();

            // Switch to GPS_RUNNING state (mind the timeout)
            gpsSetProtocolTimeout(gpsState.baseTimeoutMs);
            gpsSetState(GPS_RUNNING);
        }
        break;

    case GPS_RUNNING:
        // Call GPS protocol thread
        gpsProviders[gpsState.gpsConfig->provider].protocol();

        // Check for GPS timeout
        if ((millis() - gpsState.lastMessageMs) > gpsState.baseTimeoutMs) {
            sensorsClear(SENSOR_GPS);
            DISABLE_STATE(GPS_FIX);
            gpsSol.fixType = GPS_NO_FIX;
            gpsSetState(GPS_LOST_COMMUNICATION);
        }
        break;

    case GPS_LOST_COMMUNICATION:
        gpsStats.timeouts++;
        gpsSetState(GPS_INITIALIZING);
        break;
    }

    return gpsSol.flags.hasNewData;
}

void gpsEnablePassthrough(serialPort_t *gpsPassthroughPort)
{
    waitForSerialPortToFinishTransmitting(gpsState.gpsPort);
    waitForSerialPortToFinishTransmitting(gpsPassthroughPort);

    if (!(gpsState.gpsPort->mode & MODE_TX))
    serialSetMode(gpsState.gpsPort, gpsState.gpsPort->mode | MODE_TX);

    LED0_OFF;
    LED1_OFF;

    char c;
    while (1) {
        if (serialRxBytesWaiting(gpsState.gpsPort)) {
            LED0_ON;
            c = serialRead(gpsState.gpsPort);
            serialWrite(gpsPassthroughPort, c);
            LED0_OFF;
        }
        if (serialRxBytesWaiting(gpsPassthroughPort)) {
            LED1_ON;
            c = serialRead(gpsPassthroughPort);
            serialWrite(gpsState.gpsPort, c);
            LED1_OFF;
        }
    }
}

void updateGpsIndicator(timeUs_t currentTimeUs)
{
    static timeUs_t GPSLEDTime;
    if ((int32_t)(currentTimeUs - GPSLEDTime) >= 0 && (gpsSol.numSat>= 5)) {
        GPSLEDTime = currentTimeUs + 150000;
        LED1_TOGGLE;
    }
}

bool isGPSHealthy(void)
{
    return true;
}

bool isGPSHeadingValid(void)
{
    return sensors(SENSOR_GPS) && STATE(GPS_FIX) && gpsSol.numSat >= 6 && gpsSol.groundSpeed >= 300;
}

#endif

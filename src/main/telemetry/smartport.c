/*
 * SmartPort Telemetry implementation by frank26080115
 * see https://github.com/frank26080115/cleanflight/wiki/Using-Smart-Port
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_SMARTPORT)

#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "sensors/battery.h"

#include "io/gps.h"
#include "io/serial.h"

#include "rx/frsky_crc.h"

#include "telemetry/telemetry.h"
#include "telemetry/smartport.h"
#include "telemetry/smartport_legacy.h"
#include "telemetry/msp_shared.h"


#define __USE_C99_MATH // for roundf()
#define SMARTPORT_BAUD 57600
#define SMARTPORT_UART_MODE MODE_RXTX
#define SMARTPORT_SERVICE_TIMEOUT_MS 1 // max allowed time to find a value to send

static serialPort_t *smartPortSerialPort = NULL; // The 'SmartPort'(tm) Port.
static serialPortConfig_t *portConfig;

static portSharing_e smartPortPortSharing;

enum
{
    TELEMETRY_STATE_UNINITIALIZED,
    TELEMETRY_STATE_INITIALIZED_SERIAL,
    TELEMETRY_STATE_INITIALIZED_EXTERNAL,
};

static uint8_t telemetryState = TELEMETRY_STATE_UNINITIALIZED;

#define SMARTPORT_MSP_PAYLOAD_SIZE (sizeof(smartPortPayload_t) - sizeof(uint8_t))

static smartPortWriteFrameFn *smartPortWriteFrame;

#if defined(USE_MSP_OVER_TELEMETRY)
static bool smartPortMspReplyPending = false;
#endif

smartPortPayload_t *smartPortDataReceive(uint16_t c, bool *clearToSend, smartPortCheckQueueEmptyFn *checkQueueEmpty, bool useChecksum)
{
    static uint8_t rxBuffer[sizeof(smartPortPayload_t)];
    static uint8_t smartPortRxBytes = 0;
    static bool skipUntilStart = true;
    static bool awaitingSensorId = false;
    static bool byteStuffing = false;
    static uint16_t checksum = 0;

    if (c == FSSP_START_STOP) {
        *clearToSend = false;
        smartPortRxBytes = 0;
        awaitingSensorId = true;
        skipUntilStart = false;

        return NULL;
    } else if (skipUntilStart) {
        return NULL;
    }

    if (awaitingSensorId) {
        awaitingSensorId = false;
        if ((c == FSSP_SENSOR_ID1) && checkQueueEmpty()) {
            // our slot is starting, no need to decode more
            *clearToSend = true;
            skipUntilStart = true;
        } else if (c == FSSP_SENSOR_ID2) {
            checksum = 0;
        } else {
            skipUntilStart = true;
        }
    } else {
        if (c == FSSP_DLE) {
            byteStuffing = true;

            return NULL;
        } else if (byteStuffing) {
            c ^= FSSP_DLE_XOR;
            byteStuffing = false;
        }

        if (smartPortRxBytes < sizeof(smartPortPayload_t)) {
            rxBuffer[smartPortRxBytes++] = (uint8_t)c;
            checksum += c;

            if (!useChecksum && (smartPortRxBytes == sizeof(smartPortPayload_t))) {
                skipUntilStart = true;

                return (smartPortPayload_t *)&rxBuffer;
            }
        } else {
            skipUntilStart = true;

            checksum += c;
            checksum = (checksum & 0xFF) + (checksum >> 8);
            if (checksum == 0xFF) {
                return (smartPortPayload_t *)&rxBuffer;
            }
        }
    }

    return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_CUSTOM_TELEMETRY
static void smartPortSensorEncodeINT(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload) {
    payload->data = sensor->value;
}

static void smartPortSensorEncodeAttitude(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    const uint32_t roll = (uint32_t)(telemetrySensorValue(TELEM_ATTITUDE_ROLL) * 10) & 0xFFFF;
    const uint32_t pitch = (uint32_t)(telemetrySensorValue(TELEM_ATTITUDE_PITCH) * 10) & 0xFFFF;

    payload->data = roll | pitch << 16;
}

static void smartPortSensorEncodeFuel(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    if (telemetryConfig()->smartportFuelUnit == SMARTPORT_FUEL_UNIT_PERCENT) {
        payload->data = calculateBatteryPercentage();
    } else if (isAmperageConfigured()) {
        payload->data = telemetryConfig()->smartportFuelUnit == SMARTPORT_FUEL_UNIT_MAH ? getMAhDrawn() : getMWhDrawn();
    }
}

static void smartPortSensorEncodeKnots(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    const uint32_t mknots = gpsSol.groundSpeed * 19438UL / 1000;

    payload->data = mknots;
}

static void smartPortSensorEncodeLat(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    uint32_t lat = (uint32_t)(abs(gpsSol.llh.lat) * 6) / 100; //danger zone, may overflow for latitudes > 3579139.2 degrees, so convert int from abs function to uint32_t

    if (gpsSol.llh.lat < 0) {
        lat |= BIT(30);
    }

    payload->data = lat;
}

static void smartPortSensorEncodeLon(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    uint32_t lon = (uint32_t)(abs(gpsSol.llh.lon) * 6) / 100; //danger zone, may overflow for longitudes > 3579139.2 degrees , so convert int from abs function to uint32_t

    if (gpsSol.llh.lon < 0) {
        lon |= BIT(30);
    }

    payload->data = lon | BIT(31);
}

static void smartPortSensorEncodeHeading(__unused telemetrySensor_t *sensor, smartPortPayload_t *payload)
{
    const uint32_t cdeg = gpsSol.groundCourse * 10;

    payload->data = cdeg;
}

#endif
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_CUSTOM_TELEMETRY
#define TLM_SENSOR(NAME, APPID, FAST, SLOW, WF, WS, DENOM, ENC) \
{ \
    .sensor_id = TELEM_##NAME, \
    .app_id = (APPID), \
    .fast_interval = (FAST), \
    .slow_interval = (SLOW), \
    .fast_weight = (WF), \
    .slow_weight = (WS), \
    .ratio_num = 1, \
    .ratio_den = (DENOM), \
    .value = 0, \
    .bucket = 0, \
    .update = 0, \
    .active = false, \
    .encode = (telemetryEncode_f)smartPortSensorEncode##ENC, \
}

static telemetrySensor_t smartportTelemetrySensors[] =
{
        TLM_SENSOR(HEARTBEAT,               0x5100,  1000,  1000,   0,   0,   0,    INT),

        TLM_SENSOR(BATTERY_VOLTAGE,         0x0210,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(BATTERY_CURRENT,         0x0200,   100,  3000,   1,  10,   10,   INT),
        TLM_SENSOR(BATTERY_CONSUMPTION,     0x5250,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(BATTERY_FUEL,            0x0600,   100,  3000,   1,  10,   0,    Fuel),
        TLM_SENSOR(BATTERY_CHARGE_LEVEL,    0x0600,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(BATTERY_CELL_COUNT,      0x5260,   200,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(BATTERY_CELL_VOLTAGE,    0x0910,   200,  3000,   1,  10,   0,    INT),

        TLM_SENSOR(HEADING,                 0x5210,   100,  3000,   1,  10,   0,    INT),
#ifdef USE_BARO
        TLM_SENSOR(ALTITUDE,                0x0100,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(VARIOMETER,              0x0110,   100,  3000,   1,  10,   0,    INT),
#endif
        TLM_SENSOR(ATTITUDE,                0x0730,   100,  3000,   1,  10,   0,    Attitude),

        TLM_SENSOR(ACCEL_X,                 0x0700,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ACCEL_Y,                 0x0710,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ACCEL_Z,                 0x0720,   100,  3000,   1,  10,   0,    INT),
#ifdef USE_GPS
        TLM_SENSOR(GPS_SATS,                0x0860,   500,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(GPS_COORD,               0x0800,   100,  3000,   1,  10,   0,    Lat),
        TLM_SENSOR(GPS_COORD,               0x0800,   100,  3000,   1,  10,   0,    Lon),
        TLM_SENSOR(GPS_ALTITUDE,            0x0820,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(GPS_HEADING,             0x0840,   100,  3000,   1,  10,   0,    Heading),
        TLM_SENSOR(GPS_GROUNDSPEED,         0x0830,   100,  3000,   1,  10,   0,    Knots),
        TLM_SENSOR(GPS_HOME_DISTANCE,       0x5269,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(GPS_HOME_DIRECTION,      0x5268,   100,  3000,   1,  10,   0,   INT),
        TLM_SENSOR(GPS_AZIMUTH,             0x526A,   100,  3000,   1,  10,   0,   INT),
        TLM_SENSOR(GPS_HDOP,                0x526B,   100,  3000,   1,  10,   0,   INT),
#endif
        TLM_SENSOR(CPU_LOAD,                0x51D0,   200,  3000,   1,  10,   10,   INT),
        TLM_SENSOR(FLIGHT_MODE,             0x5121,   100,  3000,   1,   1,   0,    INT),
        TLM_SENSOR(ARMING_FLAGS,            0x5122,   100,  3000,   1,   1,   0,    INT),
        TLM_SENSOR(PROFILES,                0x5123,   100,  3000,   1,   1,   0,    INT),

#ifdef USE_ESC_SENSOR
        TLM_SENSOR(ESC1_RPM,                0x5130,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC2_RPM,                0x5131,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC3_RPM,                0x5132,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC4_RPM,                0x5133,   100,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC1_TEMPERATURE,        0x5140,   200,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC2_TEMPERATURE,        0x5141,   200,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC3_TEMPERATURE,        0x5142,   200,  3000,   1,  10,   0,    INT),
        TLM_SENSOR(ESC4_TEMPERATURE,        0x5143,   200,  3000,   1,  10,   0,    INT),
#endif
};
#endif
void smartPortSendByte(uint8_t c, uint16_t *checksum, serialPort_t *port)
{
    // smart port escape sequence
    if (c == FSSP_DLE || c == FSSP_START_STOP) {
        serialWrite(port, FSSP_DLE);
        serialWrite(port, c ^ FSSP_DLE_XOR);
    } else {
        serialWrite(port, c);
    }

    if (checksum != NULL) {
        frskyCheckSumStep(checksum, c);
    }
}

void smartPortWriteFrameSerial(const smartPortPayload_t *payload, serialPort_t *port, uint16_t checksum)
{
    uint8_t *data = (uint8_t *)payload;
    for (unsigned i = 0; i < sizeof(smartPortPayload_t); i++) {
        smartPortSendByte(*data++, &checksum, port);
    }
    frskyCheckSumFini(&checksum);
    smartPortSendByte(checksum, NULL, port);
}

static void smartPortWriteFrameInternal(const smartPortPayload_t *payload)
{
    smartPortWriteFrameSerial(payload, smartPortSerialPort, 0);
}

static void initSmartPortSensors(void)
{
#ifdef USE_CUSTOM_TELEMETRY
    if(telemetryConfig()->smartport_telemetry_mode == SMARTPORT_TELEMETRY_STATE_LEGACY) {
        initSmartPortSensorsLegacy();
    } else {
        telemetryScheduleInit(smartportTelemetrySensors, ARRAYLEN(smartportTelemetrySensors));

        for(size_t i = 0; i < ARRAYLEN(smartportTelemetrySensors); i++) {
            if(telemetrySensorActive(smartportTelemetrySensors[i].sensor_id) && telemetrySensorAllowed(smartportTelemetrySensors[i].sensor_id)) {
                telemetryScheduleAdd(&smartportTelemetrySensors[i]);
            }
        }
    }
#else
    initSmartPortSensorsLegacy();
#endif
}

bool initSmartPortTelemetry(void)
{
    if (telemetryState == TELEMETRY_STATE_UNINITIALIZED) {
        portConfig = findSerialPortConfig(FUNCTION_TELEMETRY_SMARTPORT);
        if (portConfig) {
            smartPortPortSharing = determinePortSharing(portConfig, FUNCTION_TELEMETRY_SMARTPORT);
            smartPortWriteFrame = smartPortWriteFrameInternal;
            initSmartPortSensors();
            telemetryState = TELEMETRY_STATE_INITIALIZED_SERIAL;
        }

        return true;
    }

    return false;
}

bool initSmartPortTelemetryExternal(smartPortWriteFrameFn *smartPortWriteFrameExternal)
{
    if (telemetryState == TELEMETRY_STATE_UNINITIALIZED) {
        smartPortWriteFrame = smartPortWriteFrameExternal;
        initSmartPortSensors();
        telemetryState = TELEMETRY_STATE_INITIALIZED_EXTERNAL;
        return true;
    }

    return false;
}

static void freeSmartPortTelemetryPort(void)
{
    closeSerialPort(smartPortSerialPort);
    smartPortSerialPort = NULL;
}

static void configureSmartPortTelemetryPort(void)
{
    if (portConfig) {
        portOptions_t portOptions = (telemetryConfig()->halfDuplex ? SERIAL_BIDIR : SERIAL_UNIDIR) | (telemetryConfig()->telemetry_inverted ? SERIAL_NOT_INVERTED : SERIAL_INVERTED);

        smartPortSerialPort = openSerialPort(portConfig->identifier, FUNCTION_TELEMETRY_SMARTPORT, NULL, NULL, SMARTPORT_BAUD, SMARTPORT_UART_MODE, portOptions);
    }
}

void checkSmartPortTelemetryState(void)
{
    if (telemetryState == TELEMETRY_STATE_INITIALIZED_SERIAL) {
        bool enableSerialTelemetry = telemetryDetermineEnabledState(smartPortPortSharing);

        if (enableSerialTelemetry && !smartPortSerialPort) {
            configureSmartPortTelemetryPort();
        } else if (!enableSerialTelemetry && smartPortSerialPort) {
            freeSmartPortTelemetryPort();
        }
    }
}

#if defined(USE_MSP_OVER_TELEMETRY)
static void smartPortSendMspResponse(uint8_t *data, const uint8_t dataSize) {
    smartPortPayload_t payload;
    payload.frameId = FSSP_MSPS_FRAME;
    memcpy(&payload.valueId, data, MIN(dataSize,SMARTPORT_MSP_PAYLOAD_SIZE));

    smartPortWriteFrame(&payload);
}

bool smartPortPayloadContainsMSP(const smartPortPayload_t *payload)
{
    return payload && (payload->frameId == FSSP_MSPC_FRAME_SMARTPORT || payload->frameId == FSSP_MSPC_FRAME_FPORT);
}
#endif


void processSmartPortTelemetry(smartPortPayload_t *payload, volatile bool *clearToSend, const uint32_t *requestTimeout)
{
    if (payload) {
        // do not check the physical ID here again
        // unless we start receiving other sensors' packets

#if defined(USE_MSP_OVER_TELEMETRY)
        if (smartPortPayloadContainsMSP(payload)) {
            // Pass only the payload: skip frameId
            uint8_t *frameStart = (uint8_t *)&payload->valueId;
            smartPortMspReplyPending = handleMspFrame(frameStart, SMARTPORT_MSP_PAYLOAD_SIZE);
        }
#endif
    }

    bool doRun = true;
    while (doRun && *clearToSend) {
        // Ensure we won't get stuck in the loop if there happens to be nothing available to send in a timely manner - dump the slot if we loop in there for too long.
        if (requestTimeout) {
            if (millis() >= *requestTimeout) {
                *clearToSend = false;

                return;
            }
        } else {
            doRun = false;
        }

#if defined(USE_MSP_OVER_TELEMETRY)
        if (smartPortMspReplyPending) {
            smartPortMspReplyPending = sendMspReply(SMARTPORT_MSP_PAYLOAD_SIZE, &smartPortSendMspResponse);
            *clearToSend = false;

            return;
        }
#endif
        telemetrySensor_t *sensor = telemetryScheduleNext();
        if (sensor) {
            smartPortPayload_t frame;
            frame.frameId = FSSP_DATA_FRAME;
            frame.valueId = sensor->app_id;
            sensor->encode(sensor, &frame);

            telemetryScheduleCommit(sensor);
            smartPortWriteFrame(&frame);
            *clearToSend = false;
        }

    }
}

static bool serialCheckQueueEmpty(void)
{
    return (serialRxBytesWaiting(smartPortSerialPort) == 0);
}

void handleSmartPortTelemetry(timeUs_t currentTimeUs)
{
    if (telemetryState != TELEMETRY_STATE_UNINITIALIZED) {

        telemetryScheduleUpdate(currentTimeUs);

        if (telemetryState == TELEMETRY_STATE_INITIALIZED_SERIAL && smartPortSerialPort) {
            const uint32_t requestTimeout = millis() + SMARTPORT_SERVICE_TIMEOUT_MS;

            bool clearToSend = false;
            smartPortPayload_t *payload = NULL;

            while (serialRxBytesWaiting(smartPortSerialPort) > 0 && !payload) {
                uint8_t c = serialRead(smartPortSerialPort);
                payload = smartPortDataReceive(c, &clearToSend, serialCheckQueueEmpty, true);
            }

            processSmartPortTelemetry(payload, &clearToSend, &requestTimeout);

            if (clearToSend) {
                const smartPortPayload_t emptySmartPortFrame = { 0, };
                smartPortWriteFrame(&emptySmartPortFrame);
            }
        }
    }
}
#endif

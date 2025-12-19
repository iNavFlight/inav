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
#include <string.h>

#include "platform.h"

#if defined(USE_TELEMETRY) && defined(USE_SERIALRX_CRSF) && defined(USE_TELEMETRY_CRSF)

#include "build/atomic.h"
#include "build/version.h"

#include "common/axis.h"
#include "common/crc.h"
#include "common/streambuf.h"
#include "common/time.h"
#include "common/utils.h"
#include "common/printf.h"

#include "config/feature.h"

#include "drivers/nvic.h"

#include "fc/config.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"
#include "flight/mixer.h"

#include "io/gps.h"

#include "navigation/navigation.h"

#include "rx/crsf.h"

#include "sensors/battery.h"
#include "sensors/esc_sensor.h"
#include "sensors/temperature.h"

#include "telemetry/crsf.h"
#include "telemetry/telemetry.h"
#include "telemetry/msp_shared.h"


#define CRSF_DEVICEINFO_VERSION             0x01
// According to TBS: "CRSF over serial should always use a sync byte at the beginning of each frame.
// To get better performance it's recommended to use the sync byte 0xC8 to get better performance"
//
// Digitalentity: Using frame address byte as a sync field looks somewhat hacky to me, but seems it's needed to get CRSF working properly
#define CRSF_DEVICEINFO_PARAMETER_COUNT     0

#define CRSF_MSP_BUFFER_SIZE 96
#define CRSF_MSP_LENGTH_OFFSET 1

static uint8_t crsfCrc;
static bool deviceInfoReplyPending;
static sbuf_t crsfSbuf;
static uint8_t crsfFrame[CRSF_FRAME_SIZE_MAX];

/////////////////////////////////////////////////////
#define CRSF_CUSTOM_TELEMETRY_MIN_SPACE     32

static uint8_t crsfTelemetryState;
static float crsfTelemetryRateBucket;
static float crsfTelemetryRateQuanta;

static uint8_t crsfCustomTelemetryFrameId;
static timeUs_t crsfTelemetryUpdateTime;
/////////////////////////////////////////////////////

#if defined(USE_MSP_OVER_TELEMETRY)
/////////////////////////////////////////////////////////
///////////////// MSP       /////////////////////////////
typedef struct mspBuffer_s {
    uint8_t bytes[CRSF_MSP_BUFFER_SIZE];
    int len;
} mspBuffer_t;

static mspBuffer_t mspRxBuffer;

bool bufferCrsfMspFrame(uint8_t *frameStart, int frameLength)
{
    if (mspRxBuffer.len + CRSF_MSP_LENGTH_OFFSET + frameLength > CRSF_MSP_BUFFER_SIZE) {
        return false;
    } else {
        uint8_t *p = mspRxBuffer.bytes + mspRxBuffer.len;
        *p++ = frameLength;
        memcpy(p, frameStart, frameLength);
        mspRxBuffer.len += CRSF_MSP_LENGTH_OFFSET + frameLength;
        return true;
    }
}

bool handleCrsfMspFrameBuffer(uint8_t payloadSize, mspResponseFnPtr responseFn)
{
    static bool replyPending = false;
    if (replyPending) {
        if (crsfRxIsTelemetryBufEmpty()) {
            replyPending = sendMspReply(payloadSize, responseFn);
        }
        return replyPending;
    }

    if (!mspRxBuffer.len) {
        return false;
    }
    int pos = 0;
    while (true) {
        const int mspFrameLength = mspRxBuffer.bytes[pos];
        if (handleMspFrame(&mspRxBuffer.bytes[CRSF_MSP_LENGTH_OFFSET + pos], mspFrameLength)) {
            if (crsfRxIsTelemetryBufEmpty()) {
                replyPending = sendMspReply(payloadSize, responseFn);
            } else {
                replyPending = true;
            }
        }
        pos += CRSF_MSP_LENGTH_OFFSET + mspFrameLength;
        ATOMIC_BLOCK(NVIC_PRIO_SERIALUART) {
            if (pos >= mspRxBuffer.len) {
                mspRxBuffer.len = 0;
                return replyPending ;
            }
        }
    }
    return replyPending;
}
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
#endif

/////////////////////////////////////////////////////////
static inline size_t crsfLinkFrameSlots(size_t bytes)
{
    // Telemetry data is send in 5 byte slots, with 1 slot overhead
    return (bytes + 9) / 5;
}

static inline void crsfTelemetryRateConsume(size_t slots)
{
    crsfTelemetryRateBucket -= slots;
}

static void crsfSerialize8(sbuf_t *dst, uint8_t v)
{
    sbufWriteU8(dst, v);
    crsfCrc = crc8_dvb_s2(crsfCrc, v);
}

static void crsfSerialize16BE(sbuf_t *dst, uint16_t v)
{
    // Use BigEndian format
    crsfSerialize8(dst,  (v >> 8));
    crsfSerialize8(dst, (uint8_t)v);
}

#ifdef USE_CUSTOM_TELEMETRY
static void crsfSerialize24BE(sbuf_t *dst, uint32_t v)
{
    // Use BigEndian format
    crsfSerialize8(dst, (v >> 16));
    crsfSerialize8(dst, (v >> 8));
    crsfSerialize8(dst, (uint8_t)v);
}
#endif

static void crsfSerialize32BE(sbuf_t *dst, uint32_t v)
{
    // Use BigEndian format
    crsfSerialize8(dst, (v >> 24));
    crsfSerialize8(dst, (v >> 16));
    crsfSerialize8(dst, (v >> 8));
    crsfSerialize8(dst, (uint8_t)v);
}

static void crsfSerializeData(sbuf_t *dst, const uint8_t *data, int len)
{
    for (int ii = 0; ii< len; ++ii) {
        crsfSerialize8(dst, data[ii]);
    }
}

static void crsfFinalize(sbuf_t *dst)
{
    sbufWriteU8(dst, crsfCrc);

    // Consume telemetry rate
    crsfTelemetryRateConsume(crsfLinkFrameSlots(dst->ptr - crsfFrame));

    sbufSwitchToReader(dst, crsfFrame);
    // write the telemetry frame to the receiver.
    crsfRxWriteTelemetryData(sbufPtr(dst), sbufBytesRemaining(dst));
}

#ifdef USE_CUSTOM_TELEMETRY
static void crsfFrameCustomTelemetrySensor(sbuf_t *dst, telemetrySensor_t * sensor)
{
    crsfSerialize16BE(dst, sensor->app_id);
    sensor->encode(sensor, dst);
}

static void crsfFillCustomSensorHeader(sbuf_t *dst)
{
    sbufWriteU8(dst, 0); // placeholder for [SIZE]
    crsfSerialize8(dst, CRSF_FRAMETYPE_CUSTOM_TELEM);
    crsfSerialize8(dst, CRSF_ADDRESS_RADIO_TRANSMITTER);
    crsfSerialize8(dst, CRSF_ADDRESS_FLIGHT_CONTROLLER);
    crsfSerialize8(dst, crsfCustomTelemetryFrameId);
}
#endif

//create start of CRSF header without length and command
static void crsfInitializeFrame(sbuf_t *dst)
{
    crsfCrc = 0;
    sbufWriteU8(dst, CRSF_TELEMETRY_SYNC_BYTE);
}

//sensor encoder NIL
void crsfSensorEncodeNil(__unused telemetrySensor_t *sensor, __unused sbuf_t *buf)
{
}

#ifdef USE_CUSTOM_TELEMETRY
void crsfSensorEncodeU8(telemetrySensor_t *sensor, sbuf_t *buf)
{
    crsfSerialize8(buf, constrain((uint8_t)sensor->value, 0, 0xFF));
}

void crsfSensorEncodeU16(telemetrySensor_t *sensor, sbuf_t *buf)
{
    crsfSerialize16BE(buf, constrain((uint16_t)sensor->value, 0, 0xFFFF));
}

void crsfSensorEncodeU24(telemetrySensor_t *sensor, sbuf_t *buf)
{
    crsfSerialize24BE(buf, constrain((uint32_t)sensor->value, 0, 0xFFFFFF));
}

void crsfSensorEncodeU32(telemetrySensor_t *sensor, sbuf_t *buf)
{
    crsfSerialize32BE(buf, sensor->value);
}

void crsfSensorEncodeCellVolt(telemetrySensor_t *sensor, sbuf_t *buf)
{
    const int volt = constrain(sensor->value, 200, 455) - 200;
    crsfSerialize8(buf, volt);
}

void crsfSensorEncodeCells(__unused telemetrySensor_t *sensor, sbuf_t *buf)
{
    const int cells = MIN(getBatteryCellCount(), 16);
    crsfSerialize8(buf, cells);
    for (int i = 0; i < cells; i++) {
        int volt = constrain(getBatteryAverageCellVoltage(), 200, 455) - 200;
        crsfSerialize8(buf, volt);
    }
}

void crsfSensorEncodeAttitude(__unused telemetrySensor_t *sensor, sbuf_t *buf)
{
    crsfSerialize16BE(buf, attitude.values.pitch);
    crsfSerialize16BE(buf, attitude.values.roll);
    crsfSerialize16BE(buf, attitude.values.yaw);
}


void crsfSensorEncodeLatLong(telemetrySensor_t *sensor, sbuf_t *buf)
{
    UNUSED(sensor);
    crsfSerialize32BE(buf, gpsSol.llh.lat);
    crsfSerialize32BE(buf, gpsSol.llh.lon);
}

void crsfSensorEncodeEscRpm(telemetrySensor_t *sensor, sbuf_t *buf)
{
    UNUSED(sensor);
    uint8_t motorCount = MAX(getMotorCount(), 1); //must send at least one motor, to avoid CRSF frame shifting
    motorCount = MIN(getMotorCount(), CRSF_PAYLOAD_SIZE_MAX / 3); // 3 bytes per RPM value
    motorCount = MIN(motorCount, MAX_SUPPORTED_MOTORS); // ensure we don't exceed available ESC telemetry data

    for (uint8_t i = 0; i < motorCount; i++) {
        const escSensorData_t *escState = getEscTelemetry(i);
        crsfSerialize24BE(buf, escState->rpm & 0xFFFFFF);
    }
}

void crsfSensorEncodeEscTemp(telemetrySensor_t *sensor, sbuf_t *buf) {
    UNUSED(sensor);
    uint8_t motorCount = MAX(getMotorCount(), 1); //must send at least one motor, to avoid CRSF frame shifting
    motorCount = MIN(getMotorCount(), CRSF_PAYLOAD_SIZE_MAX / 3); // 3 bytes per RPM value
    motorCount = MIN(motorCount, MAX_SUPPORTED_MOTORS); // ensure we don't exceed available ESC telemetry data

    for (uint8_t i = 0; i < motorCount; i++) {
        const escSensorData_t *escState = getEscTelemetry(i);
        uint32_t temp = (escState) ? (escState->temperature * 10) & 0xFFFFFF : TEMPERATURE_INVALID_VALUE;
        crsfSerialize24BE(buf, temp);
    }
}
#endif // USE_CUSTOM_TELEMETRY

///////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

static bool crsfCanTransmitTelemetry(void)
{
    return (crsfTelemetryRateBucket >= 0) && !(isArmingDisabledReason() & ARMING_DISABLED_BOOT_GRACE_TIME);
}

static void crsfTelemetryRateUpdate(timeUs_t currentTimeUs)
{
    timeDelta_t delta = cmpTimeUs(currentTimeUs, crsfTelemetryUpdateTime);

    crsfTelemetryRateBucket += crsfTelemetryRateQuanta * delta;
    crsfTelemetryRateBucket = constrainf(crsfTelemetryRateBucket, -25, 1);

    crsfTelemetryUpdateTime = currentTimeUs;

    telemetryScheduleUpdate(currentTimeUs);
}

//initialize buffer for sending telemetry
static sbuf_t * crsfInitializeSbuf(void)
{
    sbuf_t * dst = &crsfSbuf;

    dst->ptr = crsfFrame;
    dst->end = crsfFrame + CRSF_FRAME_SIZE_MAX;

    return dst;
}

#define TLM_SENSOR(NAME, APPID, FAST, SLOW, DENOM, ENCODER) \
{ \
    .sensor_id = TELEM_##NAME, \
    .app_id = (APPID), \
    .fast_interval = (FAST), \
    .slow_interval = (SLOW), \
    .fast_weight = 0, \
    .slow_weight = 0, \
    .ratio_num = 1, \
    .ratio_den = (DENOM), \
    .value = 0, \
    .bucket = 0, \
    .update = 0, \
    .active = false, \
    .encode = (telemetryEncode_f)crsfSensorEncode##ENCODER, \
}

///////////////////////////////////////////////////////////////////////
////// LEGACY sensors, don't add new sensors here /////////////////////
static telemetrySensor_t crsfNativeTelemetrySensors[] =
{
    TLM_SENSOR(FLIGHT_MODE,             0,  100,  100,  0,  Nil),
    TLM_SENSOR(BATTERY,                 0,  100,  100,  0,  Nil),
    TLM_SENSOR(ATTITUDE,                0,  100,  100,  0,  Nil),
#ifdef USE_BARO
    TLM_SENSOR(ALTITUDE,                0,  100,  100,  0,  Nil),
#endif
#if defined(USE_BARO) || defined(USE_GPS)
    TLM_SENSOR(VARIOMETER,              0,  100,  100,  0,  Nil),
#endif
#ifdef USE_GPS
    TLM_SENSOR(GPS,                     0,  100,  100,  0,  Nil),
#endif
};
///////////////////////////////////////////////////////////////////////
#ifdef USE_CUSTOM_TELEMETRY
static telemetrySensor_t crsfCustomTelemetrySensors[] =
{
    TLM_SENSOR(NONE,                    0x1000,  1000,  1000,    0,     Nil),
    TLM_SENSOR(HEARTBEAT,               0x1001,  1000,  1000,    0,     U16),

    TLM_SENSOR(BATTERY_VOLTAGE,         0x1011,   200,  3000,    0,     U16),
    TLM_SENSOR(BATTERY_CURRENT,         0x1012,   200,  3000,    0,     U16),
    TLM_SENSOR(BATTERY_CONSUMPTION,     0x1013,   200,  3000,    0,     U16),
    TLM_SENSOR(BATTERY_CHARGE_LEVEL,    0x1014,   200,  3000,    0,     U8),
    TLM_SENSOR(BATTERY_CELL_COUNT,      0x1020,   200,  3000,    0,     U8),
    TLM_SENSOR(BATTERY_CELL_VOLTAGE,    0x1021,   200,  3000,    0,     CellVolt),
    TLM_SENSOR(BATTERY_CELL_VOLTAGES,   0x102F,   200,  3000,    0,     Cells),

#ifdef USE_BARO
    TLM_SENSOR(ALTITUDE,                0x10B2,   200,  3000,    0,     U24),
    TLM_SENSOR(VARIOMETER,              0x10B3,   200,  3000,    0,     U16),
#endif
    TLM_SENSOR(HEADING,                 0x10B1,   200,  3000,    0,     U16),

    TLM_SENSOR(ATTITUDE,                0x1100,   100,  3000,    0,     Attitude),
    TLM_SENSOR(ATTITUDE_PITCH,          0x1101,   200,  3000,    10,    U16),
    TLM_SENSOR(ATTITUDE_ROLL,           0x1102,   200,  3000,    10,    U16),
    TLM_SENSOR(ATTITUDE_YAW,            0x1103,   200,  3000,    10,    U16),

    TLM_SENSOR(ACCEL_X,                 0x1111,   200,  3000,    100,   U16),
    TLM_SENSOR(ACCEL_Y,                 0x1112,   200,  3000,    100,   U16),
    TLM_SENSOR(ACCEL_Z,                 0x1113,   200,  3000,    100,   U16),

#ifdef USE_GPS
    TLM_SENSOR(GPS_SATS,                0x1121,   500,  3000,    0,     U8),
    TLM_SENSOR(GPS_HDOP,                0x1123,   500,  3000,    0,     U8),
    TLM_SENSOR(GPS_COORD,               0x1125,   200,  3000,    0,     LatLong),
    TLM_SENSOR(GPS_ALTITUDE,            0x1126,   200,  3000,    0,     U16),
    TLM_SENSOR(GPS_HEADING,             0x1127,   200,  3000,    0,     U16),
    TLM_SENSOR(GPS_GROUNDSPEED,         0x1128,   200,  3000,    0,     U16),
    TLM_SENSOR(GPS_HOME_DISTANCE,       0x1129,   200,  3000,    0,     U16),
    TLM_SENSOR(GPS_HOME_DIRECTION,      0x112A,   200,  3000,    0,     U16),
    TLM_SENSOR(GPS_AZIMUTH,             0x112B,   200,  3000,    0,     U16),
#endif

#ifdef USE_ESC_SENSOR
    TLM_SENSOR(ESC_RPM,                 0x1131,   200,  3000,    0,     EscRpm),
    TLM_SENSOR(ESC1_RPM,                0x1132,   200,  3000,    0,     U16),
    TLM_SENSOR(ESC2_RPM,                0x1133,   200,  3000,    0,     U16),
    TLM_SENSOR(ESC3_RPM,                0x1134,   200,  3000,    0,     U16),
    TLM_SENSOR(ESC4_RPM,                0x1135,   200,  3000,    0,     U16),

    TLM_SENSOR(ESC_TEMPERATURE,         0x1136,   200,  3000,    0,     EscTemp),
    TLM_SENSOR(ESC1_TEMPERATURE,        0x1137,   200,  3000,    0,     U8),
    TLM_SENSOR(ESC2_TEMPERATURE,        0x1138,   200,  3000,    0,     U8),
    TLM_SENSOR(ESC3_TEMPERATURE,        0x1139,   200,  3000,    0,     U8),
    TLM_SENSOR(ESC4_TEMPERATURE,        0x113A,   200,  3000,    0,     U8),
#endif

    TLM_SENSOR(CPU_LOAD,                0x1150,   500,  3000,    10,    U8),
    TLM_SENSOR(FLIGHT_MODE,             0x1251,   200,  3000,    0,     U16),
    TLM_SENSOR(ARMING_FLAGS,            0x1252,   200,  3000,    0,     U8),
    TLM_SENSOR(PROFILES,                0x1253,   200,  3000,    0,     U16),
};

telemetrySensor_t * crsfGetCustomSensor(sensor_id_e id)
{
    for (size_t i = 0; i < ARRAYLEN(crsfCustomTelemetrySensors); i++) {
        telemetrySensor_t * sensor = &crsfCustomTelemetrySensors[i];
        if (sensor->sensor_id == id)
            return sensor;
    }

    return NULL;
}
#endif // USE_CUSTOM_TELEMETRY

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
/*
CRSF frame has the structure:
<Device address> <Frame length> <Type> <Payload> <CRC>
Device address: (uint8_t)
Frame length:   length in  bytes including Type (uint8_t)
Type:           (uint8_t)
CRC:            (uint8_t), crc of <Type> and <Payload>
*/

/*
0x02 GPS
Payload:
int32_t     Latitude ( degree / 10`000`000 )
int32_t     Longitude (degree / 10`000`000 )
uint16_t    Groundspeed ( km/h / 10 )
uint16_t    GPS heading ( degree / 100 )
uint16      Altitude ( meter ­1000m offset )
uint8_t     Satellites in use ( counter )
*/
static void crsfFrameGps(sbuf_t *dst)
{
    // use sbufWrite since CRC does not include frame length
    sbufWriteU8(dst, CRSF_FRAME_GPS_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC);
    crsfSerialize8(dst, CRSF_FRAMETYPE_GPS);
    crsfSerialize32BE(dst, gpsSol.llh.lat); // CRSF and betaflight use same units for degrees
    crsfSerialize32BE(dst, gpsSol.llh.lon);
    crsfSerialize16BE(dst, (gpsSol.groundSpeed * 36 + 50) / 100); // gpsSol.groundSpeed is in cm/s
    crsfSerialize16BE(dst, DECIDEGREES_TO_CENTIDEGREES(gpsSol.groundCourse)); // gpsSol.groundCourse is 0.1 degrees, need 0.01 deg
    const uint16_t altitude = (getEstimatedActualPosition(Z) / 100) + 1000;
    crsfSerialize16BE(dst, altitude);
    crsfSerialize8(dst, gpsSol.numSat);
}

/*
0x07 Vario sensor
Payload:
int16      Vertical speed ( cm/s )
*/
static void crsfFrameVarioSensor(sbuf_t *dst)
{
    // use sbufWrite since CRC does not include frame length
    sbufWriteU8(dst, CRSF_FRAME_VARIO_SENSOR_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC);
    crsfSerialize8(dst, CRSF_FRAMETYPE_VARIO_SENSOR);
    crsfSerialize16BE(dst, lrintf(getEstimatedActualVelocity(Z)));
}

/*
0x08 Battery sensor
Payload:
uint16_t    Voltage ( mV * 100 )
uint16_t    Current ( mA * 100 )
uint24_t    Capacity ( mAh )
uint8_t     Battery remaining ( percent )
*/
static void crsfFrameBatterySensor(sbuf_t *dst)
{
    // use sbufWrite since CRC does not include frame length
    sbufWriteU8(dst, CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC);
    crsfSerialize8(dst, CRSF_FRAMETYPE_BATTERY_SENSOR);
    if (telemetryConfig()->report_cell_voltage) {
        crsfSerialize16BE(dst, getBatteryAverageCellVoltage() / 10);
    } else {
        crsfSerialize16BE(dst, getBatteryVoltage() / 10); // vbat is in units of 0.01V
    }
    crsfSerialize16BE(dst, getAmperage() / 10);
    const uint8_t batteryRemainingPercentage = calculateBatteryPercentage();
    crsfSerialize8(dst, (getMAhDrawn() >> 16));
    crsfSerialize8(dst, (getMAhDrawn() >> 8));
    crsfSerialize8(dst, (uint8_t)getMAhDrawn());
    crsfSerialize8(dst, batteryRemainingPercentage);
}

const int32_t ALT_MIN_DM = 10000;
const int32_t ALT_THRESHOLD_DM = 0x8000 - ALT_MIN_DM;
const int32_t ALT_MAX_DM = 0x7ffe * 10 - 5;

/*
0x09 Barometer altitude and vertical speed
Payload:
uint16_t    altitude_packed ( dm - 10000 )
*/
static void crsfBarometerAltitude(sbuf_t *dst)
{
    int32_t altitude_dm = lrintf(getEstimatedActualPosition(Z) / 10);
    uint16_t altitude_packed;
    if (altitude_dm < -ALT_MIN_DM) {
        altitude_packed = 0;
    } else if (altitude_dm > ALT_MAX_DM) {
        altitude_packed = 0xfffe;
    } else if (altitude_dm < ALT_THRESHOLD_DM) {
        altitude_packed = altitude_dm + ALT_MIN_DM;
    } else {
        altitude_packed = ((altitude_dm + 5) / 10) | 0x8000;
    }
    sbufWriteU8(dst, CRSF_FRAME_BAROMETER_ALTITUDE_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC);
    crsfSerialize8(dst, CRSF_FRAMETYPE_BAROMETER_ALTITUDE);
    crsfSerialize16BE(dst, altitude_packed);
}

/*
0x1E Attitude
Payload:
int16_t     Pitch angle ( rad / 10000 )
int16_t     Roll angle ( rad / 10000 )
int16_t     Yaw angle ( rad / 10000 )
*/

// convert andgle in decidegree to radians/10000 with reducing angle to +/-180 degree range
static int16_t decidegrees2Radians10000(int16_t angle_decidegree)
{
    while (angle_decidegree > 1800) {
        angle_decidegree -= 3600;
    }
    while (angle_decidegree < -1800) {
        angle_decidegree += 3600;
    }
    return (int16_t)(RAD * 1000.0f * angle_decidegree);
}

static void crsfFrameAttitude(sbuf_t *dst)
{
     sbufWriteU8(dst, CRSF_FRAME_ATTITUDE_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC);
     crsfSerialize8(dst, CRSF_FRAMETYPE_ATTITUDE);
    crsfSerialize16BE(dst, decidegrees2Radians10000(attitude.values.pitch));
    crsfSerialize16BE(dst, decidegrees2Radians10000(attitude.values.roll));
    crsfSerialize16BE(dst, decidegrees2Radians10000(attitude.values.yaw));
}

/*
0x21 Flight mode text based
Payload:
char[]      Flight mode ( Null­terminated string )
*/
static void crsfFrameFlightMode(sbuf_t *dst)
{
    // just do "OK" for the moment as a placeholder
    // write zero for frame length, since we don't know it yet
    uint8_t *lengthPtr = sbufPtr(dst);
    sbufWriteU8(dst, 0);
    crsfSerialize8(dst, CRSF_FRAMETYPE_FLIGHT_MODE);

    static uint8_t hrstSent = 0;

    // use same logic as OSD, so telemetry displays same flight text as OSD when armed
    const char *flightMode = "OK";
    if (ARMING_FLAG(ARMED)) {
        flightMode = "ACRO";
#ifdef USE_FW_AUTOLAND
        if (FLIGHT_MODE(NAV_FW_AUTOLAND)) {
            flightMode = "LAND";
        } else
#endif
        if (FLIGHT_MODE(FAILSAFE_MODE)) {
            flightMode = "!FS!";
        } else if (IS_RC_MODE_ACTIVE(BOXHOMERESET) && hrstSent < 4 && !FLIGHT_MODE(NAV_RTH_MODE) && !FLIGHT_MODE(NAV_WP_MODE)) {
            flightMode = "HRST";
            hrstSent++;
        } else if (FLIGHT_MODE(MANUAL_MODE)) {
            flightMode = "MANU";
#ifdef USE_GEOZONE
        } else if (FLIGHT_MODE(NAV_SEND_TO) && !FLIGHT_MODE(NAV_WP_MODE)) {
            flightMode = "GEO";
#endif  
        } else if (FLIGHT_MODE(TURTLE_MODE)) {
            flightMode = "TURT";
        } else if (FLIGHT_MODE(NAV_RTH_MODE)) {
            flightMode = isWaypointMissionRTHActive() ? "WRTH" : "RTH";
        } else if (FLIGHT_MODE(NAV_POSHOLD_MODE) && STATE(AIRPLANE)) {
            flightMode = "LOTR";
        } else if (FLIGHT_MODE(NAV_POSHOLD_MODE)) {
            flightMode = "HOLD";
        } else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && FLIGHT_MODE(NAV_ALTHOLD_MODE)) {
            flightMode = "CRUZ";
        } else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) {
            flightMode = "CRSH";
        } else if (FLIGHT_MODE(NAV_WP_MODE)) {
            flightMode = "WP";
        } else if (FLIGHT_MODE(NAV_ALTHOLD_MODE) && navigationRequiresAngleMode()) {
            flightMode = "AH";
        } else if (FLIGHT_MODE(ANGLE_MODE)) {
            flightMode = "ANGL";
        } else if (FLIGHT_MODE(HORIZON_MODE)) {
            flightMode = "HOR";
        } else if (FLIGHT_MODE(ANGLEHOLD_MODE)) {
            flightMode = "ANGH";
        }
#ifdef USE_GPS
    } else if (feature(FEATURE_GPS) && navConfig()->general.flags.extra_arming_safety && (!STATE(GPS_FIX) || !STATE(GPS_FIX_HOME))) {
        flightMode = "WAIT"; // Waiting for GPS lock
#endif
    } else if (isArmingDisabled()) {
        flightMode = "!ERR";
    }

    if (!IS_RC_MODE_ACTIVE(BOXHOMERESET) && hrstSent > 0)
        hrstSent = 0;

    crsfSerializeData(dst, (const uint8_t*)flightMode, strlen(flightMode));
    crsfSerialize8(dst, 0); // zero terminator for string
    // write in the length
    *lengthPtr = sbufPtr(dst) - lengthPtr;
}

/*
0x29 Device Info
Payload:
uint8_t     Destination
uint8_t     Origin
char[]      Device Name ( Null terminated string )
uint32_t    Null Bytes
uint32_t    Null Bytes
uint32_t    Null Bytes
uint8_t     255 (Max MSP Parameter)
uint8_t     0x01 (Parameter version 1)
*/
static void crsfFrameDeviceInfo(sbuf_t *dst)
{
    char buff[30];
    tfp_sprintf(buff, "%s %s: %s", FC_FIRMWARE_NAME, FC_VERSION_STRING, TARGET_BOARD_IDENTIFIER);

    uint8_t *lengthPtr = sbufPtr(dst);
    sbufWriteU8(dst, 0);
    crsfSerialize8(dst, CRSF_FRAMETYPE_DEVICE_INFO);
    crsfSerialize8(dst, CRSF_ADDRESS_RADIO_TRANSMITTER);
    crsfSerialize8(dst, CRSF_ADDRESS_FLIGHT_CONTROLLER);
    crsfSerializeData(dst, (const uint8_t*)buff, strlen(buff));
    crsfSerialize8(dst, 0); // zero terminator for string
	for (unsigned int ii=0; ii<12; ii++) {
        crsfSerialize8(dst, 0x00);
    }
    crsfSerialize8(dst, CRSF_DEVICEINFO_PARAMETER_COUNT);
    crsfSerialize8(dst, CRSF_DEVICEINFO_VERSION);
    *lengthPtr = sbufPtr(dst) - lengthPtr;
}

/*
 * 0x0B Heartbeat
 * Payload:
 * int16_t    Origin Device address
*/
static void crsfFrameHeartbeat(sbuf_t *dst)
{
    sbufWriteU8(dst, CRSF_FRAMETYPE_HEARTBEAT);
    sbufWriteU16(dst, CRSF_ADDRESS_FLIGHT_CONTROLLER);
}

#define BV(x)  (1 << (x)) // bit value
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#if defined(USE_MSP_OVER_TELEMETRY)

static bool mspReplyPending;

//Id of the last receiver MSP frame over CRSF. Needed to send response with correct frame ID
static uint8_t mspRequestOriginID = 0;

void crsfScheduleMspResponse(uint8_t requestOriginID)
{
    mspReplyPending = true;
    mspRequestOriginID = requestOriginID;
}

void crsfSendMspResponse(uint8_t *payload, const uint8_t payloadSize)
{
    sbuf_t *dst = crsfInitializeSbuf();

    crsfInitializeFrame(dst);  // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
    sbufWriteU8(dst, payloadSize + CRSF_FRAME_LENGTH_EXT_TYPE_CRC); // [LENGTH]
    crsfSerialize8(dst, CRSF_FRAMETYPE_MSP_RESP);
    crsfSerialize8(dst, mspRequestOriginID);
    crsfSerialize8(dst, CRSF_ADDRESS_FLIGHT_CONTROLLER);
    crsfSerializeData(dst, (const uint8_t*)payload, payloadSize);
    crsfFinalize(dst);
}
#endif

static bool crsfSendNativeTelemetry(void)
{
    if (crsfTelemetryState != CRSFR_TELEMETRY_STATE_NATIVE)
    {
        return false;
    }

    telemetrySensor_t *sensor = telemetryScheduleNext();

    if (sensor) {
        sbuf_t *dst = crsfInitializeSbuf();
        switch (sensor->sensor_id) {
            case TELEM_ATTITUDE:
                crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
                crsfFrameAttitude(dst);  // create whole frame without SYNC and CRC
                crsfFinalize(dst);
                break;
#if defined(USE_BARO) || defined(USE_GPS)
            case TELEM_VARIOMETER:
                crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
                crsfFrameVarioSensor(dst); // create whole frame without SYNC and CRC
                crsfFinalize(dst);
                break;
            case TELEM_ALTITUDE:
                crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
                crsfBarometerAltitude(dst); // create whole frame without SYNC and CRC
                crsfFinalize(dst);
                break;
#endif
            case TELEM_BATTERY:
                crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
                crsfFrameBatterySensor(dst); // create whole frame without SYNC and CRC
                crsfFinalize(dst);
                break;
            case TELEM_FLIGHT_MODE:
                crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
                crsfFrameFlightMode(dst); // create whole frame without SYNC and CRC
                crsfFinalize(dst);
                break;
#ifdef USE_GPS
            case TELEM_GPS:
                crsfInitializeFrame(dst);
                crsfFrameGps(dst);
                crsfFinalize(dst);
                break;
#endif
            default:
                crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
                crsfFrameHeartbeat(dst);  // create whole frame without SYNC and CRC
                crsfFinalize(dst);
                break;
        }
        telemetryScheduleCommit(sensor);
        return true;
    }

    return false;
}

#ifdef USE_CUSTOM_TELEMETRY
static bool crsfSendCustomTelemetry(void)
{
    if (crsfTelemetryState == CRSFR_TELEMETRY_STATE_CUSTOM)
    {
        size_t sensor_count = 0;
        sbuf_t *dst = crsfInitializeSbuf(); // prepare buffer

        crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
        uint8_t *lengthPtr = sbufPtr(dst); // [LENGTH] take position of length, because we don't know length
        crsfFillCustomSensorHeader(dst); // fill rest of header [CRSF_FRAMETYPE_CUSTOM_TELEM, CRSF_ADDRESS_RADIO_TRANSMITTER, CRSF_ADDRESS_FLIGHT_CONTROLLER, frameId]

        while (sbufBytesRemaining(dst) > CRSF_CUSTOM_TELEMETRY_MIN_SPACE) {
            telemetrySensor_t *sensor = telemetryScheduleNext();
            if (sensor) {
                crsfFrameCustomTelemetrySensor(dst, sensor);
                if (sbufBytesRemaining(dst) < 1) {
                    break;
                }
                telemetryScheduleCommit(sensor);
                sensor_count++;
            }
            else {
                break;
            }
        }

        if (sensor_count) {
            *lengthPtr = sbufPtr(dst) - lengthPtr; // write length,[LENGTH]
            // here should frame looks like:
            // [CRSF_TELEMETRY_SYNC_BYTE] [LENGTH] [CRSF_FRAMETYPE_CUSTOM_TELEM, CRSF_ADDRESS_RADIO_TRANSMITTER, CRSF_ADDRESS_FLIGHT_CONTROLLER, frameId] [DATA]
            // CRC is filled by crsfFinalize function
            crsfFinalize(dst);
            crsfCustomTelemetryFrameId++;
            return true;
        }
    }

    return false;
}
#endif

#ifdef USE_CUSTOM_TELEMETRY
static bool crsfPopulateCustomTelemetry(void)
{
    if (crsfTelemetryState == CRSFR_TELEMETRY_STATE_POPULATE)
    {
        static int slot = -10;

        if (slot < 0) {
            telemetrySensor_t * sensor = crsfGetCustomSensor(TELEM_NONE);
            slot++;

            if (sensor) {
                sbuf_t *dst = crsfInitializeSbuf();
                crsfInitializeFrame(dst);
                uint8_t *lengthPtr = sbufPtr(dst);
                crsfFillCustomSensorHeader(dst);
                crsfFrameCustomTelemetrySensor(dst, sensor);
                *lengthPtr = sbufPtr(dst) - lengthPtr;
                crsfFinalize(dst);
                return true;
            }
            return false;
        }

        // send list of active sensors to LUA telemetry
        while (slot < TELEM_SENSOR_SLOT_COUNT) {
            sensor_id_e id = telemetryConfig()->telemetry_sensors[slot];
            slot++;

            if (telemetrySensorActive(id)) {
                telemetrySensor_t * sensor = crsfGetCustomSensor(id);
                if (sensor) {
                    sbuf_t *dst = crsfInitializeSbuf();
                    crsfInitializeFrame(dst);
                    uint8_t *lengthPtr = sbufPtr(dst);
                    crsfFillCustomSensorHeader(dst);
                    crsfFrameCustomTelemetrySensor(dst, sensor);
                    *lengthPtr = sbufPtr(dst) - lengthPtr;
                    crsfFinalize(dst);
                    crsfCustomTelemetryFrameId++;
                    return true;
                }
            }
        }

        crsfTelemetryState = CRSFR_TELEMETRY_STATE_CUSTOM;
    }

    return false;
}
#endif

static bool crsfSendDeviceInfoData(void)
{
    if (deviceInfoReplyPending) {
        deviceInfoReplyPending = false;
        sbuf_t *dst = crsfInitializeSbuf();
        crsfInitializeFrame(dst); // write sync byte [CRSF_TELEMETRY_SYNC_BYTE]
        crsfFrameDeviceInfo(dst); // create whole frame without SYNC and CRC
        crsfFinalize(dst);
        return true;
    }
    return false;
}

void crsfScheduleDeviceInfoResponse(void)
{
    deviceInfoReplyPending = true;
}

bool checkCrsfTelemetryState(void)
{
    return crsfTelemetryState;
}

/*
 * Called periodically by the scheduler
 */
void handleCrsfTelemetry(timeUs_t currentTimeUs)
{
    if (!crsfTelemetryState) {
        return;
    }

    crsfTelemetryRateUpdate(currentTimeUs);

    if (crsfCanTransmitTelemetry()) {
        // Give the receiver a chance to send any outstanding telemetry data.
        // This needs to be done at high frequency, to enable the RX to send the telemetry frame
        // in between the RX frames.
        crsfRxSendTelemetryData();

        // Send ad-hoc response frames as soon as possible
#if defined(USE_MSP_OVER_TELEMETRY)
        if (mspReplyPending) {
            mspReplyPending = handleCrsfMspFrameBuffer(CRSF_FRAME_TX_MSP_FRAME_SIZE, &crsfSendMspResponse);
            return;
        }
#endif

        if (!crsfRxIsTelemetryBufEmpty()) {
            return; // do nothing if telemetry ouptut buffer is not empty yet.
        }

        bool __unused sent =
                crsfSendDeviceInfoData() ||
                crsfSendNativeTelemetry()
#ifdef USE_CUSTOM_TELEMETRY
                ||
                crsfSendCustomTelemetry() ||
                crsfPopulateCustomTelemetry()
#endif
        ;
    }
}
#ifdef USE_CUSTOM_TELEMETRY
static void initCrsfCustomSensors(void)
{
    telemetryScheduleInit(crsfCustomTelemetrySensors, ARRAYLEN(crsfCustomTelemetrySensors));

    for(size_t i = 0; i < ARRAYLEN(crsfCustomTelemetrySensors); i++) {
        if(telemetrySensorActive(crsfCustomTelemetrySensors[i].sensor_id) && telemetrySensorAllowed(crsfCustomTelemetrySensors[i].sensor_id)) {
            telemetryScheduleAdd(&crsfCustomTelemetrySensors[i]);
        }

    }
}
#endif

static void initCrsfNativeSensors(void)
{
    telemetryScheduleInit(crsfNativeTelemetrySensors, ARRAYLEN(crsfNativeTelemetrySensors));

    for(size_t i = 0; i < ARRAYLEN(crsfNativeTelemetrySensors); i++) {
        if(telemetrySensorAllowed(crsfNativeTelemetrySensors[i].sensor_id)) {
            telemetryScheduleAdd(&crsfNativeTelemetrySensors[i]);
        }
    }
}


void initCrsfTelemetry(void)
{
    // check if there is a serial port open for CRSF telemetry (ie opened by the CRSF RX)
    // and feature is enabled, if so, set CRSF telemetry enabled

#ifdef USE_CUSTOM_TELEMETRY
    crsfTelemetryState = !crsfRxIsActive() ? CRSFR_TELEMETRY_STATE_OFF : (telemetryConfig()->crsf_telemetry_mode == CRSFR_TELEMETRY_STATE_NATIVE ? CRSFR_TELEMETRY_STATE_NATIVE : CRSFR_TELEMETRY_STATE_POPULATE);
#else
    crsfTelemetryState = !crsfRxIsActive() ? CRSFR_TELEMETRY_STATE_OFF : CRSFR_TELEMETRY_STATE_NATIVE;
#endif

    if(crsfTelemetryState) {
        deviceInfoReplyPending = false;

        const float rate = telemetryConfig()->crsf_telemetry_link_rate;
        const float ratio = telemetryConfig()->crsf_telemetry_link_ratio;

        crsfTelemetryRateQuanta = rate / (ratio * 1000000);
        crsfTelemetryRateBucket = 0;
        crsfCustomTelemetryFrameId = 0;

#if defined(USE_MSP_OVER_TELEMETRY)
        mspReplyPending = false;
#endif
#ifdef USE_CUSTOM_TELEMETRY
        if (crsfTelemetryState == CRSFR_TELEMETRY_STATE_NATIVE) {
            initCrsfNativeSensors();
        } else {
            initCrsfCustomSensors();
        }
#else
        initCrsfNativeSensors();
#endif
    }
}

#endif

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

/*
 Created by Marcin Baliniak
 some functions based on MinimOSD

 OSD-CMS separation by jflyper
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <inttypes.h>

#include "platform.h"

#ifdef USE_OSD

#include "build/debug.h"
#include "build/version.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_osd.h"

#include "common/axis.h"
#include "common/constants.h"
#include "common/filter.h"
#include "common/log.h"
#include "common/olc.h"
#include "common/printf.h"
#include "common/string_light.h"
#include "common/time.h"
#include "common/typeconversion.h"
#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/display.h"
#include "drivers/display_canvas.h"
#include "drivers/display_font_metadata.h"
#include "drivers/osd_symbols.h"
#include "drivers/time.h"
#include "drivers/vtx_common.h"
#include "drivers/gimbal_common.h"

#include "io/adsb.h"
#include "io/flashfs.h"
#include "io/gps.h"
#include "io/osd.h"
#include "io/osd_common.h"
#include "io/osd_hud.h"
#include "io/osd_utils.h"
#include "io/displayport_msp_dji_compat.h"
#include "io/vtx.h"
#include "io/vtx_string.h"

#include "io/osd/custom_elements.h"

#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/fc_core.h"
#include "fc/fc_tasks.h"
#include "fc/multifunction.h"
#include "fc/rc_adjustments.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/power_limits.h"
#include "flight/rth_estimator.h"
#include "flight/servos.h"
#include "flight/wind_estimator.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "rx/rx.h"
#include "rx/msp_override.h"

#include "sensors/acceleration.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/compass.h"
#include "sensors/diagnostics.h"
#include "sensors/sensors.h"
#include "sensors/pitotmeter.h"
#include "sensors/temperature.h"
#include "sensors/esc_sensor.h"
#include "sensors/rangefinder.h"

#include "programming/logic_condition.h"
#include "programming/global_variables.h"

#ifdef USE_BLACKBOX
#include "blackbox/blackbox_io.h"
#endif

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

#define VIDEO_BUFFER_CHARS_PAL    480
#define VIDEO_BUFFER_CHARS_HDZERO 900
#define VIDEO_BUFFER_CHARS_DJIWTF 1320

#define GFORCE_FILTER_TC 0.2

#define OSD_STATS_SINGLE_PAGE_MIN_ROWS 18
#define IS_HI(X)  (rxGetChannelValue(X) > 1750)
#define IS_LO(X)  (rxGetChannelValue(X) < 1250)
#define IS_MID(X) (rxGetChannelValue(X) > 1250 && rxGetChannelValue(X) < 1750)

#define OSD_RESUME_UPDATES_STICK_COMMAND (checkStickPosition(THR_HI) || checkStickPosition(PIT_HI))
#define STATS_PAGE2 (checkStickPosition(ROL_HI))
#define STATS_PAGE1 (checkStickPosition(ROL_LO))

#define SPLASH_SCREEN_DISPLAY_TIME 4000 // ms
#define STATS_SCREEN_DISPLAY_TIME 60000 // ms

#define EFFICIENCY_UPDATE_INTERVAL (5 * 1000)

// Adjust OSD_MESSAGE's default position when
// changing OSD_MESSAGE_LENGTH
#define OSD_MESSAGE_LENGTH 28
#define OSD_ALTERNATING_CHOICES(ms, num_choices) ((millis() / ms) % num_choices)
#define _CONST_STR_SIZE(s) ((sizeof(s)/sizeof(s[0]))-1) // -1 to avoid counting final '\0'
// Wrap all string constants intenteded for display as messages with
// this macro to ensure compile time length validation.
#define OSD_MESSAGE_STR(x) ({ \
    STATIC_ASSERT(_CONST_STR_SIZE(x) <= OSD_MESSAGE_LENGTH, message_string_ ## __COUNTER__ ## _too_long); \
    x; \
})

#define OSD_CHR_IS_NUM(c) (c >= '0' && c <= '9')

#define OSD_CENTER_LEN(x) ((osdDisplayPort->cols - x) / 2)
#define OSD_CENTER_S(s) OSD_CENTER_LEN(strlen(s))

#define OSD_MIN_FONT_VERSION 3

static timeMs_t linearDescentMessageMs  = 0;
static timeMs_t notify_settings_saved   = 0;
static bool     savingSettings          = false;

static unsigned currentLayout = 0;
static int layoutOverride = -1;
static bool hasExtendedFont = false; // Wether the font supports characters > 256
static timeMs_t layoutOverrideUntil = 0;
static pt1Filter_t GForceFilter, GForceFilterAxis[XYZ_AXIS_COUNT];
static float GForce, GForceAxis[XYZ_AXIS_COUNT];

typedef struct statistic_s {
    uint16_t max_speed;
    uint16_t max_3D_speed;
    uint16_t max_air_speed;
    uint16_t min_voltage; // /100
    int16_t max_current;
    int32_t max_power;
    uint8_t min_rssi;
    int16_t min_lq; // for CRSF
    int16_t min_rssi_dbm; // for CRSF
    int32_t max_altitude;
    uint32_t max_distance;
    uint8_t min_sats;
    uint8_t max_sats;
    int16_t max_esc_temp;
    int16_t min_esc_temp;
    int32_t flightStartMAh;
    int32_t flightStartMWh;
} statistic_t;

static statistic_t stats;

static timeUs_t resumeRefreshAt = 0;
static bool refreshWaitForResumeCmdRelease;

static bool fullRedraw = false;

static uint8_t armState;

static textAttributes_t osdGetMultiFunctionMessage(char *buff);
static uint8_t osdWarningsFlags = 0;

typedef struct osdMapData_s {
    uint32_t scale;
    char referenceSymbol;
} osdMapData_t;

static osdMapData_t osdMapData;

static displayPort_t *osdDisplayPort;
static bool osdDisplayIsReady = false;
#if defined(USE_CANVAS)
static displayCanvas_t osdCanvas;
static bool osdDisplayHasCanvas;
#else
#define osdDisplayHasCanvas false
#endif

#define AH_MAX_PITCH_DEFAULT 20 // Specify default maximum AHI pitch value displayed (degrees)

PG_REGISTER_WITH_RESET_TEMPLATE(osdConfig_t, osdConfig, PG_OSD_CONFIG, 13);
PG_REGISTER_WITH_RESET_FN(osdLayoutsConfig_t, osdLayoutsConfig, PG_OSD_LAYOUTS_CONFIG, 2);

void osdStartedSaveProcess(void) {
    savingSettings = true;
}

void osdShowEEPROMSavedNotification(void) {
    savingSettings = false;
    notify_settings_saved = millis() + 5000;
}

bool osdDisplayIsPAL(void)
{
    return displayScreenSize(osdDisplayPort) == VIDEO_BUFFER_CHARS_PAL;
}

bool osdDisplayIsHD(void)
{
    if (displayScreenSize(osdDisplayPort) >= VIDEO_BUFFER_CHARS_HDZERO)
    {
        return true;
    }
    return false;
}

bool osdIsNotMetric(void) {
    return !(osdConfig()->units == OSD_UNIT_METRIC || osdConfig()->units == OSD_UNIT_METRIC_MPH);
}

/**
 * Converts distance into a string based on the current unit system
 * prefixed by a a symbol to indicate the unit used.
 * @param dist Distance in centimeters
 */
static void osdFormatDistanceSymbol(char *buff, int32_t dist, uint8_t decimals, uint8_t digits)
{
    if (digits == 0)    // Total number of digits (including decimal point)
        digits = 3U;
    uint8_t sym_index = digits; // Position (index) at buffer of units symbol
    uint8_t symbol_m = SYM_DIST_M;
    uint8_t symbol_km = SYM_DIST_KM;
    uint8_t symbol_ft = SYM_DIST_FT;
    uint8_t symbol_mi = SYM_DIST_MI;
    uint8_t symbol_nm = SYM_DIST_NM;

#ifndef DISABLE_MSP_DJI_COMPAT   // IF DJICOMPAT is not supported, there's no need to check for it and change the values
    if (isDJICompatibleVideoSystem(osdConfig())) {
        // Add one digit so up no switch to scaled decimal occurs above 99
        digits = 4U;
        sym_index = 4U;
        // Use altitude symbols on purpose, as it seems distance symbols are not defined in DJICOMPAT mode
        symbol_m = SYM_ALT_M;
        symbol_km = SYM_ALT_KM;
        symbol_ft = SYM_ALT_FT;
        symbol_mi = SYM_MI;
        symbol_nm = SYM_MI;
    }
#endif

    switch ((osd_unit_e)osdConfig()->units) {
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_IMPERIAL:
        if (osdFormatCentiNumber(buff, CENTIMETERS_TO_CENTIFEET(dist), FEET_PER_MILE, decimals, 3, digits, false)) {
            buff[sym_index] = symbol_mi;
        } else {
            buff[sym_index] = symbol_ft;
        }
        buff[sym_index + 1] = '\0';
        break;
    case OSD_UNIT_METRIC_MPH:
        FALLTHROUGH;
    case OSD_UNIT_METRIC:
        if (osdFormatCentiNumber(buff, dist, METERS_PER_KILOMETER, decimals, 3, digits, false)) {
            buff[sym_index] = symbol_km;
        } else {
            buff[sym_index] = symbol_m;
        }
        buff[sym_index + 1] = '\0';
        break;
    case OSD_UNIT_GA:
        if (osdFormatCentiNumber(buff, CENTIMETERS_TO_CENTIFEET(dist), (uint32_t)FEET_PER_NAUTICALMILE, decimals, 3, digits, false)) {
            buff[sym_index] = symbol_nm;
        } else {
            buff[sym_index] = symbol_ft;
        }
        buff[sym_index + 1] = '\0';
        break;
    }
}

/**
 * Converts distance into a string based on the current unit system.
 * @param dist Distance in centimeters
 */
static void osdFormatDistanceStr(char *buff, int32_t dist)
{
    int32_t centifeet;
    switch ((osd_unit_e)osdConfig()->units) {
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_IMPERIAL:
        centifeet = CENTIMETERS_TO_CENTIFEET(dist);
        if (abs(centifeet) < FEET_PER_MILE * 100 / 2) {
            // Show feet when dist < 0.5mi
            tfp_sprintf(buff, "%d%c", (int)(centifeet / 100), SYM_FT);
        } else {
            // Show miles when dist >= 0.5mi
            tfp_sprintf(buff, "%d.%02d%c", (int)(centifeet / (100*FEET_PER_MILE)),
                (abs(centifeet) % (100 * FEET_PER_MILE)) / FEET_PER_MILE, SYM_MI);
        }
        break;
    case OSD_UNIT_METRIC_MPH:
        FALLTHROUGH;
    case OSD_UNIT_METRIC:
        if (abs(dist) < METERS_PER_KILOMETER * 100) {
            // Show meters when dist < 1km
            tfp_sprintf(buff, "%d%c", (int)(dist / 100), SYM_M);
        } else {
            // Show kilometers when dist >= 1km
            tfp_sprintf(buff, "%d.%02d%c", (int)(dist / (100*METERS_PER_KILOMETER)),
                (abs(dist) % (100 * METERS_PER_KILOMETER)) / METERS_PER_KILOMETER, SYM_KM);
        }
        break;
    case OSD_UNIT_GA:
         centifeet = CENTIMETERS_TO_CENTIFEET(dist);
        if (abs(centifeet) < 100000) {
            // Show feet when dist < 1000ft
            tfp_sprintf(buff, "%d%c", (int)(centifeet / 100), SYM_FT);
        } else {
            // Show nautical miles when dist >= 1000ft
            tfp_sprintf(buff, "%d.%02d%c", (int)(centifeet / (100 * FEET_PER_NAUTICALMILE)),
                (int)((abs(centifeet) % (int)(100 * FEET_PER_NAUTICALMILE)) / FEET_PER_NAUTICALMILE), SYM_NM);
        }
        break;
    }
}

/**
 * Converts velocity based on the current unit system (kmh or mph).
 * @param alt Raw velocity (i.e. as taken from gpsSol.groundSpeed in centimeters/second)
 */
static int32_t osdConvertVelocityToUnit(int32_t vel)
{
    switch ((osd_unit_e)osdConfig()->units) {
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_METRIC_MPH:
        FALLTHROUGH;
    case OSD_UNIT_IMPERIAL:
        return CMSEC_TO_CENTIMPH(vel) / 100; // Convert to mph
    case OSD_UNIT_METRIC:
        return CMSEC_TO_CENTIKPH(vel) / 100;   // Convert to kmh
    case OSD_UNIT_GA:
        return CMSEC_TO_CENTIKNOTS(vel) / 100; // Convert to Knots
    }
    // Unreachable
    return -1;
}

/**
 * Converts velocity into a string based on the current unit system.
 * @param vel Raw velocity (i.e. as taken from gpsSol.groundSpeed in centimeters/seconds)
 * @param _3D is a 3D velocity
 * @param _max is a maximum velocity
 */
void osdFormatVelocityStr(char* buff, int32_t vel, bool _3D, bool _max)
{
    switch ((osd_unit_e)osdConfig()->units) {
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_METRIC_MPH:
        FALLTHROUGH;
    case OSD_UNIT_IMPERIAL:
        if (_max) {
            tfp_sprintf(buff, "%c%3d%c", SYM_MAX, (int)osdConvertVelocityToUnit(vel), (_3D ? SYM_3D_MPH : SYM_MPH));
        } else {
            tfp_sprintf(buff, "%3d%c", (int)osdConvertVelocityToUnit(vel), (_3D ? SYM_3D_MPH : SYM_MPH));
        }
        break;
    case OSD_UNIT_METRIC:
        if (_max) {
            tfp_sprintf(buff, "%c%3d%c", SYM_MAX, (int)osdConvertVelocityToUnit(vel), (_3D ? SYM_3D_KMH : SYM_KMH));
        } else {
            tfp_sprintf(buff, "%3d%c", (int)osdConvertVelocityToUnit(vel), (_3D ? SYM_3D_KMH : SYM_KMH));
        }
        break;
    case OSD_UNIT_GA:
        if (_max) {
            tfp_sprintf(buff, "%c%3d%c", SYM_MAX, (int)osdConvertVelocityToUnit(vel), (_3D ? SYM_3D_KT : SYM_KT));
        } else {
            tfp_sprintf(buff, "%3d%c", (int)osdConvertVelocityToUnit(vel), (_3D ? SYM_3D_KT : SYM_KT));
        }
        break;
    }
}

/**
 * Returns the average velocity. This always uses stats, so can be called as an OSD element later if wanted, to show a real time average
 */
static void osdGenerateAverageVelocityStr(char* buff) {
    uint32_t cmPerSec = getTotalTravelDistance() / getFlightTime();
    osdFormatVelocityStr(buff, cmPerSec, false, false);
}

/**
 * Converts wind speed into a string based on the current unit system, using
 * always 3 digits and an additional character for the unit at the right. buff
 * is null terminated.
 * @param ws Raw wind speed in cm/s
 */
#ifdef USE_WIND_ESTIMATOR
static void osdFormatWindSpeedStr(char *buff, int32_t ws, bool isValid)
{
    int32_t centivalue;
    char suffix;
    switch (osdConfig()->units) {
        case OSD_UNIT_UK:
            FALLTHROUGH;
        case OSD_UNIT_METRIC_MPH:
            FALLTHROUGH;
        case OSD_UNIT_IMPERIAL:
            centivalue = CMSEC_TO_CENTIMPH(ws);
            suffix = SYM_MPH;
            break;
        case OSD_UNIT_GA:
            centivalue = CMSEC_TO_CENTIKNOTS(ws);
            suffix = SYM_KT;
            break;
        default:
        case OSD_UNIT_METRIC:
            if (osdConfig()->estimations_wind_mps)
            {
                centivalue = ws;
                suffix = SYM_MS;
            }
            else
            {
                centivalue = CMSEC_TO_CENTIKPH(ws);
                suffix = SYM_KMH;
            }
            break;
    }

    osdFormatCentiNumber(buff, centivalue, 0, 2, 0, 3, false);

    if (!isValid && ((millis() / 1000) % 4 < 2))
        suffix = '*';

    buff[3] = suffix;
    buff[4] = '\0';
}
#endif

/**
* Converts altitude into a string based on the current unit system
* prefixed by a a symbol to indicate the unit used.
* @param alt Raw altitude/distance (i.e. as taken from baro.BaroAlt in centimeters)
*/
void osdFormatAltitudeSymbol(char *buff, int32_t alt)
{
    uint8_t digits = osdConfig()->decimals_altitude;
    uint8_t totalDigits = digits + 1;
    uint8_t symbolIndex = digits + 1;
    uint8_t symbolKFt = SYM_ALT_KFT;

    if (alt >= 0) {
        buff[0] = ' ';
    }

#ifndef DISABLE_MSP_DJI_COMPAT   // IF DJICOMPAT is not supported, there's no need to check for it and change the values
    if (isDJICompatibleVideoSystem(osdConfig())) {
        totalDigits++;
        digits++;
        symbolIndex++;
        symbolKFt = SYM_ALT_FT;
    }
#endif

    switch ((osd_unit_e)osdConfig()->units) {
        case OSD_UNIT_UK:
            FALLTHROUGH;
        case OSD_UNIT_GA:
            FALLTHROUGH;
        case OSD_UNIT_IMPERIAL:
            if (osdFormatCentiNumber(buff + totalDigits - digits, CENTIMETERS_TO_CENTIFEET(alt), 1000, 0, 2, digits, false)) {
                // Scaled to kft
                buff[symbolIndex++] = symbolKFt;
            } else {
                // Formatted in feet
                buff[symbolIndex++] = SYM_ALT_FT;
            }
            buff[symbolIndex] = '\0';
            break;
        case OSD_UNIT_METRIC_MPH:
            FALLTHROUGH;
        case OSD_UNIT_METRIC:
            // alt is alredy in cm
            if (osdFormatCentiNumber(buff + totalDigits - digits, alt, 1000, 0, 2, digits, false)) {
                // Scaled to km
                buff[symbolIndex++] = SYM_ALT_KM;
            } else {
                // Formatted in m
                buff[symbolIndex++] = SYM_ALT_M;
            }
            buff[symbolIndex] = '\0';
            break;
    }
}

/**
* Converts altitude into a string based on the current unit system.
* @param alt Raw altitude/distance (i.e. as taken from baro.BaroAlt in centimeters)
*/
static void osdFormatAltitudeStr(char *buff, int32_t alt)
{
    int32_t value;
    switch ((osd_unit_e)osdConfig()->units) {
        case OSD_UNIT_UK:
            FALLTHROUGH;
        case OSD_UNIT_GA:
            FALLTHROUGH;
        case OSD_UNIT_IMPERIAL:
            value = CENTIMETERS_TO_FEET(alt);
            tfp_sprintf(buff, "%d%c", (int)value, SYM_FT);
            break;
        case OSD_UNIT_METRIC_MPH:
            FALLTHROUGH;
        case OSD_UNIT_METRIC:
            value = CENTIMETERS_TO_METERS(alt);
            tfp_sprintf(buff, "%d%c", (int)value, SYM_M);
            break;
    }
}

static void osdFormatTime(char *buff, uint32_t seconds, char sym_m, char sym_h)
{
    uint32_t value = seconds;
    char sym = sym_m;
    // Maximum value we can show in minutes is 99 minutes and 59 seconds
    if (seconds > (99 * 60) + 59) {
        sym = sym_h;
        value = seconds / 60;
    }
    buff[0] = sym;
    tfp_sprintf(buff + 1, "%02d:%02d", (int)(value / 60), (int)(value % 60));
}

static inline void osdFormatOnTime(char *buff)
{
    osdFormatTime(buff, micros() / 1000000, SYM_ON_M, SYM_ON_H);
}

static inline void osdFormatFlyTime(char *buff, textAttributes_t *attr)
{
    uint32_t seconds = getFlightTime();
    osdFormatTime(buff, seconds, SYM_FLY_M, SYM_FLY_H);
    if (attr && osdConfig()->time_alarm > 0) {
       if (seconds / 60 >= osdConfig()->time_alarm && ARMING_FLAG(ARMED)) {
            TEXT_ATTRIBUTES_ADD_BLINK(*attr);
        }
    }
}

/**
 * Trim whitespace from string.
 * Used in Stats screen on lines with multiple values.
*/
char *osdFormatTrimWhiteSpace(char *buff)
{
    char *end;

    // Trim leading spaces
    while(isspace((unsigned char)*buff)) buff++;

    // All spaces?
    if(*buff == 0)
    return buff;

    // Trim trailing spaces
    end = buff + strlen(buff) - 1;
    while(end > buff && isspace((unsigned char)*end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return buff;
}

/**
 * Converts RSSI into a % value used by the OSD.
 * Range is [0, 100]
 */
static uint8_t osdConvertRSSI(void)
{
    return constrain(getRSSI() * 100 / RSSI_MAX_VALUE, 0, 100);
}

static uint16_t osdGetCrsfLQ(void)
{
    int16_t statsLQ = rxLinkStatistics.uplinkLQ;
    int16_t scaledLQ = scaleRange(constrain(statsLQ, 0, 100), 0, 100, 170, 300);
    int16_t displayedLQ = 0;
    switch (osdConfig()->crsf_lq_format) {
        case OSD_CRSF_LQ_TYPE1:
            displayedLQ = statsLQ;
            break;
        case OSD_CRSF_LQ_TYPE2:
            displayedLQ = statsLQ;
            break;
        case OSD_CRSF_LQ_TYPE3:
            displayedLQ = rxLinkStatistics.rfMode >= 2 ? scaledLQ : statsLQ;
            break;
    }
    return displayedLQ;
}

static int16_t osdGetCrsfdBm(void)
{
    return rxLinkStatistics.uplinkRSSI;
}
/**
* Displays a temperature postfixed with a symbol depending on the current unit system
* @param label to display
* @param valid true if measurement is valid
* @param temperature in deciDegrees Celcius
*/
static void osdDisplayTemperature(uint8_t elemPosX, uint8_t elemPosY, uint16_t symbol, const char *label, bool valid, int16_t temperature, int16_t alarm_min, int16_t alarm_max)
{
    char buff[TEMPERATURE_LABEL_LEN + 2 < 6 ? 6 : TEMPERATURE_LABEL_LEN + 2];
    textAttributes_t elemAttr = valid ? TEXT_ATTRIBUTES_NONE : _TEXT_ATTRIBUTES_BLINK_BIT;
    uint8_t valueXOffset = 0;

    if (symbol) {
        buff[0] = symbol;
        buff[1] = '\0';
        displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, buff, elemAttr);
        valueXOffset = 1;
    }
#ifdef USE_TEMPERATURE_SENSOR
    else if (label[0] != '\0') {
        uint8_t label_len = strnlen(label, TEMPERATURE_LABEL_LEN);
        memcpy(buff, label, label_len);
        memset(buff + label_len, ' ', TEMPERATURE_LABEL_LEN + 1 - label_len);
        buff[5] = '\0';
        displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, buff, elemAttr);
        valueXOffset = osdConfig()->temp_label_align == OSD_ALIGN_LEFT ? 5 : label_len + 1;
    }
#else
    UNUSED(label);
#endif

    if (valid) {

        if ((temperature <= alarm_min) || (temperature >= alarm_max)) TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        if (osdConfig()->units == OSD_UNIT_IMPERIAL) temperature = temperature * 9 / 5.0f + 320;
        tfp_sprintf(buff, "%3d", temperature / 10);

    } else
        strcpy(buff, "---");

    buff[3] = osdConfig()->units == OSD_UNIT_IMPERIAL ? SYM_TEMP_F : SYM_TEMP_C;
    buff[4] = '\0';

    displayWriteWithAttr(osdDisplayPort, elemPosX + valueXOffset, elemPosY, buff, elemAttr);
}

#ifdef USE_TEMPERATURE_SENSOR
static void osdDisplayTemperatureSensor(uint8_t elemPosX, uint8_t elemPosY, uint8_t sensorIndex)
{
    int16_t temperature;
    const bool valid = getSensorTemperature(sensorIndex, &temperature);
    const tempSensorConfig_t *sensorConfig = tempSensorConfig(sensorIndex);
    uint16_t symbol = sensorConfig->osdSymbol ? SYM_TEMP_SENSOR_FIRST + sensorConfig->osdSymbol - 1 : 0;
    osdDisplayTemperature(elemPosX, elemPosY, symbol, sensorConfig->label, valid, temperature, sensorConfig->alarm_min, sensorConfig->alarm_max);
}
#endif

static void osdFormatCoordinate(char *buff, char sym, int32_t val)
{
    // up to 4 for number + 1 for the symbol + null terminator + fill the rest with decimals
    const int coordinateLength = osdConfig()->coordinate_digits + 1;

    buff[0] = sym;
    int32_t integerPart = val / GPS_DEGREES_DIVIDER;
    // Latitude maximum integer width is 3 (-90) while
    // longitude maximum integer width is 4 (-180).
    int integerDigits = tfp_sprintf(buff + 1, (integerPart == 0 && val < 0) ? "-%d" : "%d", (int)integerPart);
    // We can show up to 7 digits in decimalPart.
    int32_t decimalPart = abs(val % (int)GPS_DEGREES_DIVIDER);
    STATIC_ASSERT(GPS_DEGREES_DIVIDER == 1e7, adjust_max_decimal_digits);
    int decimalDigits;
    bool djiCompat = false;  // Assume DJICOMPAT mode is no enabled

#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
        if(isDJICompatibleVideoSystem(osdConfig())) {
            djiCompat = true;
        }
#endif

    if (!djiCompat) {
        decimalDigits = tfp_sprintf(buff + 1 + integerDigits, "%07d", (int)decimalPart);
        // Embbed the decimal separator
        buff[1 + integerDigits - 1] += SYM_ZERO_HALF_TRAILING_DOT - '0';
        buff[1 + integerDigits] += SYM_ZERO_HALF_LEADING_DOT - '0';
    } else {
        // DJICOMPAT mode enabled
        decimalDigits = tfp_sprintf(buff + 1 + integerDigits, ".%06d", (int)decimalPart);
    }
    // Fill up to coordinateLength with zeros
    int total = 1 + integerDigits + decimalDigits;
    while(total < coordinateLength) {
        buff[total] = '0';
        total++;
    }
    buff[coordinateLength] = '\0';
}

static void osdFormatCraftName(char *buff)
{
    if (strlen(systemConfig()->craftName) == 0)
            strcpy(buff, "CRAFT_NAME");
    else {
        for (int i = 0; i < MAX_NAME_LENGTH; i++) {
            buff[i] = sl_toupper((unsigned char)systemConfig()->craftName[i]);
            if (systemConfig()->craftName[i] == 0)
                break;
        }
    }
}

void osdFormatPilotName(char *buff)
{
    if (strlen(systemConfig()->pilotName) == 0)
            strcpy(buff, "PILOT_NAME");
    else {
        for (int i = 0; i < MAX_NAME_LENGTH; i++) {
            buff[i] = sl_toupper((unsigned char)systemConfig()->pilotName[i]);
            if (systemConfig()->pilotName[i] == 0)
                break;
        }
    }
}

static const char * osdArmingDisabledReasonMessage(void)
{
    const char *message = NULL;
    static char messageBuf[OSD_MESSAGE_LENGTH+1];

    switch (isArmingDisabledReason()) {
        case ARMING_DISABLED_FAILSAFE_SYSTEM:
            // See handling of FAILSAFE_RX_LOSS_MONITORING in failsafe.c
            if (failsafePhase() == FAILSAFE_RX_LOSS_MONITORING) {
                if (failsafeIsReceivingRxData()) {
                    // reminder to disarm to exit FAILSAFE_RX_LOSS_MONITORING once timeout period ends
                    if (IS_RC_MODE_ACTIVE(BOXARM)) {
                        return OSD_MESSAGE_STR(OSD_MSG_TURN_ARM_SW_OFF);
                    }
                } else {
                    // Not receiving RX data
                    return OSD_MESSAGE_STR(OSD_MSG_RC_RX_LINK_LOST);
                }
            }
            return OSD_MESSAGE_STR(OSD_MSG_DISABLED_BY_FS);
        case ARMING_DISABLED_NOT_LEVEL:
            return OSD_MESSAGE_STR(OSD_MSG_AIRCRAFT_UNLEVEL);
        case ARMING_DISABLED_SENSORS_CALIBRATING:
            return OSD_MESSAGE_STR(OSD_MSG_SENSORS_CAL);
        case ARMING_DISABLED_SYSTEM_OVERLOADED:
            return OSD_MESSAGE_STR(OSD_MSG_SYS_OVERLOADED);
        case ARMING_DISABLED_NAVIGATION_UNSAFE:
            // Check the exact reason
            switch (navigationIsBlockingArming(NULL)) {
                char buf[6];
                case NAV_ARMING_BLOCKER_NONE:
                    break;
                case NAV_ARMING_BLOCKER_MISSING_GPS_FIX:
                    return OSD_MESSAGE_STR(OSD_MSG_WAITING_GPS_FIX);
                case NAV_ARMING_BLOCKER_NAV_IS_ALREADY_ACTIVE:
                    return OSD_MESSAGE_STR(OSD_MSG_DISABLE_NAV_FIRST);
                case NAV_ARMING_BLOCKER_FIRST_WAYPOINT_TOO_FAR:
                    osdFormatDistanceSymbol(buf, distanceToFirstWP(), 0, 3);
                    tfp_sprintf(messageBuf, "FIRST WP TOO FAR (%s)", buf);
                    return message = messageBuf;
                case NAV_ARMING_BLOCKER_JUMP_WAYPOINT_ERROR:
                    return OSD_MESSAGE_STR(OSD_MSG_JUMP_WP_MISCONFIG);
            }
            break;
        case ARMING_DISABLED_COMPASS_NOT_CALIBRATED:
            return OSD_MESSAGE_STR(OSD_MSG_MAG_NOT_CAL);
        case ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED:
            return OSD_MESSAGE_STR(OSD_MSG_ACC_NOT_CAL);
        case ARMING_DISABLED_ARM_SWITCH:
            return OSD_MESSAGE_STR(OSD_MSG_DISARM_1ST);
        case ARMING_DISABLED_HARDWARE_FAILURE:
            {
                if (!HW_SENSOR_IS_HEALTHY(getHwGyroStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_GYRO_FAILURE);
                }
                if (!HW_SENSOR_IS_HEALTHY(getHwAccelerometerStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_ACC_FAIL);
                }
                if (!HW_SENSOR_IS_HEALTHY(getHwCompassStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_MAG_FAIL);
                }
                if (!HW_SENSOR_IS_HEALTHY(getHwBarometerStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_BARO_FAIL);
                }
                if (!HW_SENSOR_IS_HEALTHY(getHwGPSStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_GPS_FAIL);
                }
                if (!HW_SENSOR_IS_HEALTHY(getHwRangefinderStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_RANGEFINDER_FAIL);
                }
                if (!HW_SENSOR_IS_HEALTHY(getHwPitotmeterStatus())) {
                    return OSD_MESSAGE_STR(OSD_MSG_PITOT_FAIL);
                }
            }
            return OSD_MESSAGE_STR(OSD_MSG_HW_FAIL);
        case ARMING_DISABLED_BOXFAILSAFE:
            return OSD_MESSAGE_STR(OSD_MSG_FS_EN);
        case ARMING_DISABLED_RC_LINK:
            return OSD_MESSAGE_STR(OSD_MSG_NO_RC_LINK);
        case ARMING_DISABLED_THROTTLE:
            return OSD_MESSAGE_STR(OSD_MSG_THROTTLE_NOT_LOW);
        case ARMING_DISABLED_ROLLPITCH_NOT_CENTERED:
            return OSD_MESSAGE_STR(OSD_MSG_ROLLPITCH_OFFCENTER);
        case ARMING_DISABLED_SERVO_AUTOTRIM:
            return OSD_MESSAGE_STR(OSD_MSG_AUTOTRIM_ACTIVE);
        case ARMING_DISABLED_OOM:
            return OSD_MESSAGE_STR(OSD_MSG_NOT_ENOUGH_MEMORY);
        case ARMING_DISABLED_INVALID_SETTING:
            return OSD_MESSAGE_STR(OSD_MSG_INVALID_SETTING);
        case ARMING_DISABLED_CLI:
            return OSD_MESSAGE_STR(OSD_MSG_CLI_ACTIVE);
        case ARMING_DISABLED_PWM_OUTPUT_ERROR:
            return OSD_MESSAGE_STR(OSD_MSG_PWM_INIT_ERROR);
        case ARMING_DISABLED_NO_PREARM:
            return OSD_MESSAGE_STR(OSD_MSG_NO_PREARM);
        case ARMING_DISABLED_DSHOT_BEEPER:
            return OSD_MESSAGE_STR(OSD_MSG_DSHOT_BEEPER);
            // Cases without message
        case ARMING_DISABLED_LANDING_DETECTED:
            FALLTHROUGH;
        case ARMING_DISABLED_CMS_MENU:
            FALLTHROUGH;
        case ARMING_DISABLED_OSD_MENU:
            FALLTHROUGH;
        case ARMING_DISABLED_ALL_FLAGS:
            FALLTHROUGH;
        case ARMED:
            FALLTHROUGH;
        case SIMULATOR_MODE_HITL:
            FALLTHROUGH;
        case SIMULATOR_MODE_SITL:
            FALLTHROUGH;
        case WAS_EVER_ARMED:
            break;
    }
    return NULL;
}

static const char * osdFailsafePhaseMessage(void)
{
    // See failsafe.h for each phase explanation
    switch (failsafePhase()) {
        case FAILSAFE_RETURN_TO_HOME:
            // XXX: Keep this in sync with OSD_FLYMODE.
            return OSD_MESSAGE_STR(OSD_MSG_RTH_FS);
        case FAILSAFE_LANDING:
            // This should be considered an emergengy landing
            return OSD_MESSAGE_STR(OSD_MSG_EMERG_LANDING_FS);
        case FAILSAFE_RX_LOSS_MONITORING:
            // Only reachable from FAILSAFE_LANDED, which performs
            // a disarm. Since aircraft has been disarmed, we no
            // longer show failsafe details.
            FALLTHROUGH;
        case FAILSAFE_LANDED:
            // Very brief, disarms and transitions into
            // FAILSAFE_RX_LOSS_MONITORING. Note that it prevents
            // further rearming via ARMING_DISABLED_FAILSAFE_SYSTEM,
            // so we'll show the user how to re-arm in when
            // that flag is the reason to prevent arming.
            FALLTHROUGH;
        case FAILSAFE_RX_LOSS_IDLE:
            // This only happens when user has chosen NONE as FS
            // procedure. The recovery messages should be enough.
            FALLTHROUGH;
        case FAILSAFE_IDLE:
            // Failsafe not active
            FALLTHROUGH;
        case FAILSAFE_RX_LOSS_DETECTED:
            // Very brief, changes to FAILSAFE_RX_LOSS_RECOVERED
            // or the FS procedure immediately.
            FALLTHROUGH;
        case FAILSAFE_RX_LOSS_RECOVERED:
            // Exiting failsafe
            break;
    }
    return NULL;
}

static const char * osdFailsafeInfoMessage(void)
{
    if (failsafeIsReceivingRxData() && !FLIGHT_MODE(NAV_FW_AUTOLAND)) {
        // User must move sticks to exit FS mode
        return OSD_MESSAGE_STR(OSD_MSG_MOVE_EXIT_FS);
    }
    return OSD_MESSAGE_STR(OSD_MSG_RC_RX_LINK_LOST);
}

#if defined(USE_SAFE_HOME)
static const char * divertingToSafehomeMessage(void)
{
#ifdef USE_FW_AUTOLAND
	if (!posControl.fwLandState.landWp && (NAV_Status.state != MW_NAV_STATE_HOVER_ABOVE_HOME && posControl.safehomeState.isApplied)) {
#else
    if (NAV_Status.state != MW_NAV_STATE_HOVER_ABOVE_HOME && posControl.safehomeState.isApplied) {
#endif
	    return OSD_MESSAGE_STR(OSD_MSG_DIVERT_SAFEHOME);
	}
#endif
	return NULL;
}


static const char * navigationStateMessage(void)
{
    if (!posControl.rthState.rthLinearDescentActive && linearDescentMessageMs != 0)
        linearDescentMessageMs = 0;

    switch (NAV_Status.state) {
        case MW_NAV_STATE_NONE:
            break;
        case MW_NAV_STATE_RTH_START:
            return OSD_MESSAGE_STR(OSD_MSG_STARTING_RTH);
        case MW_NAV_STATE_RTH_CLIMB:
            return OSD_MESSAGE_STR(OSD_MSG_RTH_CLIMB);
        case MW_NAV_STATE_RTH_ENROUTE:
            if (posControl.flags.rthTrackbackActive) {
                return OSD_MESSAGE_STR(OSD_MSG_RTH_TRACKBACK);
            } else {
                if (posControl.rthState.rthLinearDescentActive && (linearDescentMessageMs == 0 || linearDescentMessageMs > millis())) {
                    if (linearDescentMessageMs == 0)
                        linearDescentMessageMs = millis() + 5000; // Show message for 5 seconds.

                    return OSD_MESSAGE_STR(OSD_MSG_RTH_LINEAR_DESCENT);
                } else
                    return OSD_MESSAGE_STR(OSD_MSG_HEADING_HOME);
            }
        case MW_NAV_STATE_HOLD_INFINIT:
            // Used by HOLD flight modes. No information to add.
            break;
        case MW_NAV_STATE_HOLD_TIMED:
            // "HOLDING WP FOR xx S" Countdown added in osdGetSystemMessage
            break;
        case MW_NAV_STATE_WP_ENROUTE:
            // "TO WP" + WP countdown added in osdGetSystemMessage
            break;
        case MW_NAV_STATE_PROCESS_NEXT:
            return OSD_MESSAGE_STR(OSD_MSG_PREPARE_NEXT_WP);
        case MW_NAV_STATE_DO_JUMP:
            // Not used
            break;
        case MW_NAV_STATE_LAND_START:
            // Not used
            break;
        case MW_NAV_STATE_EMERGENCY_LANDING:
            return OSD_MESSAGE_STR(OSD_MSG_EMERG_LANDING);
        case MW_NAV_STATE_LAND_IN_PROGRESS:
            return OSD_MESSAGE_STR(OSD_MSG_LANDING);
        case MW_NAV_STATE_HOVER_ABOVE_HOME:
            if (STATE(FIXED_WING_LEGACY)) {
#if defined(USE_SAFE_HOME)
                if (posControl.safehomeState.isApplied) {
                    return OSD_MESSAGE_STR(OSD_MSG_LOITERING_SAFEHOME);
                }
#endif
                return OSD_MESSAGE_STR(OSD_MSG_LOITERING_HOME);
            }
            return OSD_MESSAGE_STR(OSD_MSG_HOVERING);
        case MW_NAV_STATE_LANDED:
            return OSD_MESSAGE_STR(OSD_MSG_LANDED);
        case MW_NAV_STATE_LAND_SETTLE:
        {
            // If there is a FS landing delay occurring. That is handled by the calling function.
            if (posControl.landingDelay > 0)
                break;

            return OSD_MESSAGE_STR(OSD_MSG_PREPARING_LAND);
        }
        case MW_NAV_STATE_LAND_START_DESCENT:
            // Not used
            break;
    }

    return NULL;
}

static void osdFormatMessage(char *buff, size_t size, const char *message, bool isCenteredText)
{
    // String is always filled with Blanks
    memset(buff, SYM_BLANK, size);
    if (message) {
        size_t messageLength = strlen(message);
        int rem = isCenteredText ? MAX(0, (int)size - (int)messageLength) : 0;
        strncpy(buff + rem / 2, message, MIN((int)size - rem / 2, (int)messageLength));
    }
    // Ensure buff is zero terminated
    buff[size] = '\0';
}

/**
 * Draws the battery symbol filled in accordingly to the
 * battery voltage to buff[0].
 **/
static void osdFormatBatteryChargeSymbol(char *buff)
{
    uint8_t p = calculateBatteryPercentage();
    p = (100 - p) / 16.6;
    buff[0] = SYM_BATT_FULL + p;
}

static void osdUpdateBatteryCapacityOrVoltageTextAttributes(textAttributes_t *attr)
{
    const batteryState_e batteryState = getBatteryState();

    if (batteryState == BATTERY_WARNING || batteryState == BATTERY_CRITICAL) {
        TEXT_ATTRIBUTES_ADD_BLINK(*attr);
    }
}

void osdCrosshairPosition(uint8_t *x, uint8_t *y)
{
    *x = osdDisplayPort->cols / 2;
    *y = osdDisplayPort->rows / 2;
    *y -= osdConfig()->horizon_offset; // positive horizon_offset moves the HUD up, negative moves down
}

/**
 * Check if this OSD layout is using scaled or unscaled throttle.
 * If both are used, it will default to scaled.
 */
bool osdUsingScaledThrottle(void)
{
    bool usingScaledThrottle = OSD_VISIBLE(osdLayoutsConfig()->item_pos[currentLayout][OSD_SCALED_THROTTLE_POS]);
    bool usingRCThrottle = OSD_VISIBLE(osdLayoutsConfig()->item_pos[currentLayout][OSD_THROTTLE_POS]);

    if (!usingScaledThrottle && !usingRCThrottle)
        usingScaledThrottle = true;

    return usingScaledThrottle;
}

/**
 * Formats throttle position prefixed by its symbol.
 * Shows unscaled or scaled (output to motor) throttle percentage
 **/
static void osdFormatThrottlePosition(char *buff, bool useScaled, textAttributes_t *elemAttr)
{
    buff[0] = SYM_BLANK;
    buff[1] = SYM_THR;
    if (navigationIsControllingThrottle()) {
        buff[0] = SYM_AUTO_THR0;
        buff[1] = SYM_AUTO_THR1;
        if (isFixedWingAutoThrottleManuallyIncreased()) {
            TEXT_ATTRIBUTES_ADD_BLINK(*elemAttr);
        }
        useScaled = true;
    }
#ifdef USE_POWER_LIMITS
    if (powerLimiterIsLimiting()) {
        TEXT_ATTRIBUTES_ADD_BLINK(*elemAttr);
    }
#endif
    int8_t throttlePercent = getThrottlePercent(useScaled);
    if ((useScaled && throttlePercent <= 0) || !ARMING_FLAG(ARMED)) {
        const char* message = ARMING_FLAG(ARMED) ? (throttlePercent == 0 && !ifMotorstopFeatureEnabled()) ? "IDLE" : "STOP" : "DARM";
        buff[0] = SYM_THR;
        strcpy(buff + 1, message);
        return;
    }
    tfp_sprintf(buff + 2, "%3d", throttlePercent);
}

/**
 * Formats gvars prefixed by its number (0-indexed). If autoThr
 **/
static void osdFormatGVar(char *buff, uint8_t index)
{
    buff[0] = 'G';
    buff[1] = '0'+index;
    buff[2] = ':';
    #ifdef USE_PROGRAMMING_FRAMEWORK
    osdFormatCentiNumber(buff + 3, (int32_t)gvGet(index)*(int32_t)100, 1, 0, 0, 5, false);
    #endif
}

#if defined(USE_ESC_SENSOR)
static void osdFormatRpm(char *buff, uint32_t rpm)
{
    buff[0] = SYM_RPM;
    if (rpm) {
        if ( digitCount(rpm) > osdConfig()->esc_rpm_precision) {
            uint8_t rpmMaxDecimals = (osdConfig()->esc_rpm_precision - 3);
            osdFormatCentiNumber(buff + 1, rpm / 10, 0, rpmMaxDecimals, rpmMaxDecimals, osdConfig()->esc_rpm_precision-1, false);
            buff[osdConfig()->esc_rpm_precision] = 'K';
            buff[osdConfig()->esc_rpm_precision+1] = '\0';
        }
        else {
            switch(osdConfig()->esc_rpm_precision) {
                case 6:
                    tfp_sprintf(buff + 1, "%6lu", rpm);
                    break;
                case 5:
                    tfp_sprintf(buff + 1, "%5lu", rpm);
                    break;
                case 4:
                    tfp_sprintf(buff + 1, "%4lu", rpm);
                    break;
                case 3:
                default:
                    tfp_sprintf(buff + 1, "%3lu", rpm);
                    break;
            }


        }
    }
    else {
        uint8_t buffPos = 1;
        while (buffPos <=( osdConfig()->esc_rpm_precision)) {
            strcpy(buff + buffPos++, "-");
        }
    }
}
#endif

int32_t osdGetAltitude(void)
{
    return getEstimatedActualPosition(Z);
}

static inline int32_t osdGetAltitudeMsl(void)
{
    return getEstimatedActualPosition(Z) + posControl.gpsOrigin.alt;
}

uint16_t osdGetRemainingGlideTime(void) {
    float value = getEstimatedActualVelocity(Z);
    static pt1Filter_t glideTimeFilterState;
    const  timeMs_t curTimeMs = millis();
    static timeMs_t glideTimeUpdatedMs;

    value = pt1FilterApply4(&glideTimeFilterState, isnormal(value) ? value : 0, 0.5, MS2S(curTimeMs - glideTimeUpdatedMs));
    glideTimeUpdatedMs = curTimeMs;

    if (value < 0) {
        value = osdGetAltitude() / abs((int)value);
    } else {
        value = 0;
    }

    return (uint16_t)roundf(value);
}

static bool osdIsHeadingValid(void)
{
    return isImuHeadingValid();
}

int16_t osdGetHeading(void)
{
    return attitude.values.yaw;
}

int16_t osdGetPanServoOffset(void)
{
    int8_t servoIndex = osdConfig()->pan_servo_index;
    int16_t servoMiddle = servoParams(servoIndex)->middle;
    int16_t servoPosition = servo[servoIndex];

    gimbalDevice_t *dev = gimbalCommonDevice();
    if (dev && gimbalCommonIsReady(dev)) {
        servoPosition = gimbalCommonGetPanPwm(dev);
        servoMiddle = PWM_RANGE_MIDDLE + gimbalConfig()->panTrim;
    }

    return (int16_t)CENTIDEGREES_TO_DEGREES((servoPosition - servoMiddle) * osdConfig()->pan_servo_pwm2centideg);
}

// Returns a heading angle in degrees normalized to [0, 360).
int osdGetHeadingAngle(int angle)
{
    while (angle < 0) {
        angle += 360;
    }
    while (angle >= 360) {
        angle -= 360;
    }
    return angle;
}

#if defined(USE_GPS)

/* Draws a map with the given symbol in the center and given point of interest
 * defined by its distance in meters and direction in degrees.
 * referenceHeading indicates the up direction in the map, in degrees, while
 * referenceSym (if non-zero) is drawn at the upper right corner below a small
 * arrow to indicate the map reference to the user. The drawn argument is an
 * in-out used to store the last position where the craft was drawn to avoid
 * erasing all screen on each redraw.
 */
static void osdDrawMap(int referenceHeading, uint16_t referenceSym, uint16_t centerSym,
                       uint32_t poiDistance, int16_t poiDirection, uint16_t poiSymbol,
                       uint16_t *drawn, uint32_t *usedScale)
{
    // TODO: These need to be tested with several setups. We might
    // need to make them configurable.
    const int hMargin = 5;
    const int vMargin = 3;

    // TODO: Get this from the display driver?
    const int charWidth = 12;
    const int charHeight = 18;

    uint8_t minX = hMargin;
    uint8_t maxX = osdDisplayPort->cols - 1 - hMargin;
    uint8_t minY = vMargin;
    uint8_t maxY = osdDisplayPort->rows - 1 - vMargin;
    uint8_t midX = osdDisplayPort->cols / 2;
    uint8_t midY = osdDisplayPort->rows / 2;

    // Fixed marks
    displayWriteChar(osdDisplayPort, midX, midY, centerSym);

    // First, erase the previous drawing.
    if (OSD_VISIBLE(*drawn)) {
        displayWriteChar(osdDisplayPort, OSD_X(*drawn), OSD_Y(*drawn), SYM_BLANK);
        *drawn = 0;
    }

    uint32_t initialScale;
    const unsigned scaleMultiplier = 2;
    // We try to reduce the scale when the POI will be around half the distance
    // between the center and the closers map edge, to avoid too much jumping
    const int scaleReductionMultiplier = MIN(midX - hMargin, midY - vMargin) / 2;

    switch (osdConfig()->units) {
        case OSD_UNIT_UK:
            FALLTHROUGH;
        case OSD_UNIT_IMPERIAL:
            initialScale = 16; // 16m ~= 0.01miles
            break;
        case OSD_UNIT_GA:
            initialScale = 18; // 18m ~= 0.01 nautical miles
            break;
        default:
        case OSD_UNIT_METRIC_MPH:
            FALLTHROUGH;
        case OSD_UNIT_METRIC:
            initialScale = 10; // 10m as initial scale
            break;
    }

    // Try to keep the same scale when getting closer until we draw over the center point
    uint32_t scale = initialScale;
    if (*usedScale) {
        scale = *usedScale;
        if (scale > initialScale && poiDistance < *usedScale * scaleReductionMultiplier) {
            scale /= scaleMultiplier;
        }
    }

    if (STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
        ) {

        int directionToPoi = osdGetHeadingAngle(poiDirection - referenceHeading);
        float poiAngle = DEGREES_TO_RADIANS(directionToPoi);
        float poiSin = sin_approx(poiAngle);
        float poiCos = cos_approx(poiAngle);

        // Now start looking for a valid scale that lets us draw everything
        int ii;
        for (ii = 0; ii < 50; ii++) {
            // Calculate location of the aircraft in map
            int points = poiDistance / ((float)scale / charHeight);

            float pointsX = points * poiSin;
            int poiX = midX - roundf(pointsX / charWidth);
            if (poiX < minX || poiX > maxX) {
                scale *= scaleMultiplier;
                continue;
            }

            float pointsY = points * poiCos;
            int poiY = midY + roundf(pointsY / charHeight);
            if (poiY < minY || poiY > maxY) {
                scale *= scaleMultiplier;
                continue;
            }

            if (poiX == midX && poiY == midY) {
                // We're over the map center symbol, so we would be drawing
                // over it even if we increased the scale. Alternate between
                // drawing the center symbol or drawing the POI.
                if (centerSym != SYM_BLANK && OSD_ALTERNATING_CHOICES(1000, 2) == 0) {
                    break;
                }
            } else {

                uint16_t c;
                if (displayReadCharWithAttr(osdDisplayPort, poiX, poiY, &c, NULL) && c != SYM_BLANK) {
                    // Something else written here, increase scale. If the display doesn't support reading
                    // back characters, we assume there's nothing.
                    //
                    // If we're close to the center, decrease scale. Otherwise increase it.
                    uint8_t centerDeltaX = (maxX - minX) / (scaleMultiplier * 2);
                    uint8_t centerDeltaY = (maxY - minY) / (scaleMultiplier * 2);
                    if (poiX >= midX - centerDeltaX && poiX <= midX + centerDeltaX &&
                        poiY >= midY - centerDeltaY && poiY <= midY + centerDeltaY &&
                        scale > scaleMultiplier) {

                        scale /= scaleMultiplier;
                    } else {
                        scale *= scaleMultiplier;
                    }
                    continue;
                }
            }

            // Draw the point on the map
            if (poiSymbol == SYM_ARROW_UP) {
                // Drawing aircraft, rotate
                int mapHeading = osdGetHeadingAngle(DECIDEGREES_TO_DEGREES(osdGetHeading()) - referenceHeading);
                poiSymbol += mapHeading * 2 / 45;
            }
            displayWriteChar(osdDisplayPort, poiX, poiY, poiSymbol);

            // Update saved location
            *drawn = OSD_POS(poiX, poiY) | OSD_VISIBLE_FLAG;
            break;
        }
    }

    *usedScale = scale;

    // Update global map data for scale and reference
    osdMapData.scale = scale;
    osdMapData.referenceSymbol = referenceSym;
}

/* Draws a map with the home in the center and the craft moving around.
 * See osdDrawMap() for reference.
 */
static void osdDrawHomeMap(int referenceHeading, uint8_t referenceSym, uint16_t *drawn, uint32_t *usedScale)
{
    osdDrawMap(referenceHeading, referenceSym, SYM_HOME, GPS_distanceToHome, GPS_directionToHome, SYM_ARROW_UP, drawn, usedScale);
}

/* Draws a map with the aircraft in the center and the home moving around.
 * See osdDrawMap() for reference.
 */
static void osdDrawRadar(uint16_t *drawn, uint32_t *usedScale)
{
    int16_t reference = DECIDEGREES_TO_DEGREES(osdGetHeading());
    int16_t poiDirection = osdGetHeadingAngle(GPS_directionToHome + 180);
    osdDrawMap(reference, 0, SYM_ARROW_UP, GPS_distanceToHome, poiDirection, SYM_HOME, drawn, usedScale);
}

static uint16_t crc_accumulate(uint8_t data, uint16_t crcAccum)
{
    uint8_t tmp;
    tmp = data ^ (uint8_t)(crcAccum & 0xff);
    tmp ^= (tmp << 4);
    crcAccum = (crcAccum >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);
    return crcAccum;
}


static void osdDisplayTelemetry(void)
{
    uint32_t          trk_data;
    uint16_t          trk_crc = 0;
    char              trk_buffer[31];
    static int16_t    trk_elevation = 127;
    static uint16_t   trk_bearing   = 0;

    if (ARMING_FLAG(ARMED)) {
      if (STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
                || STATE(GPS_ESTIMATED_FIX)
#endif
            ){
        if (GPS_distanceToHome > 5) {
          trk_bearing = GPS_directionToHome;
          trk_bearing += 360 + 180;
          trk_bearing %= 360;
          int32_t alt = CENTIMETERS_TO_METERS(osdGetAltitude());
          float at = atan2(alt, GPS_distanceToHome);
          trk_elevation = at * 57.2957795f; // 57.2957795 = 1 rad
          trk_elevation += 37; // because elevation in telemetry should be from -37 to 90
          if (trk_elevation < 0) {
            trk_elevation = 0;
          }
        }
      }
    }
    else{
      trk_elevation = 127;
      trk_bearing   = 0;
    }

    trk_data = 0;                                                // bit  0    - packet type 0 = bearing/elevation, 1 = 2 byte data packet
    trk_data = trk_data | (uint32_t)(0x7F & trk_elevation) << 1; // bits 1-7  - elevation angle to target. NOTE number is abused. constrained value of -37 to 90 sent as 0 to 127.
    trk_data = trk_data | (uint32_t)trk_bearing << 8;            // bits 8-17 - bearing angle to target. 0 = true north. 0 to 360
    trk_crc = crc_accumulate(0xFF & trk_data, trk_crc);          // CRC First Byte  bits 0-7
    trk_crc = crc_accumulate(0xFF & trk_bearing, trk_crc);       // CRC Second Byte bits 8-15
    trk_crc = crc_accumulate(trk_bearing >> 8, trk_crc);         // CRC Third Byte  bits  16-17
    trk_data = trk_data | (uint32_t)trk_crc << 17;               // bits 18-29 CRC & 0x3FFFF

    for (uint8_t t_ctr = 0; t_ctr < 30; t_ctr++) {               // Prepare screen buffer and write data line.
      if (trk_data & (uint32_t)1 << t_ctr){
        trk_buffer[29 - t_ctr] = SYM_TELEMETRY_0;
      }
      else{
        trk_buffer[29 - t_ctr] = SYM_TELEMETRY_1;
      }
    }
    trk_buffer[30] = 0;
    displayWrite(osdDisplayPort, 0, 0, trk_buffer);
    if (osdConfig()->telemetry>1){
      displayWrite(osdDisplayPort, 0, 3, trk_buffer);               // Test display because normal telemetry line is not visible
    }
}
#endif

static void osdFormatPidControllerOutput(char *buff, const char *label, const pidController_t *pidController, uint8_t scale, bool showDecimal) {
    strcpy(buff, label);
    for (uint8_t i = strlen(label); i < 5; ++i) buff[i] = ' ';
    uint8_t decimals = showDecimal ? 1 : 0;
    osdFormatCentiNumber(buff + 5, pidController->proportional * scale, 0, decimals, 0, 4, false);
    buff[9] = ' ';
    osdFormatCentiNumber(buff + 10, pidController->integrator * scale, 0, decimals, 0, 4, false);
    buff[14] = ' ';
    osdFormatCentiNumber(buff + 15, pidController->derivative * scale, 0, decimals, 0, 4, false);
    buff[19] = ' ';
    osdFormatCentiNumber(buff + 20, pidController->output_constrained * scale, 0, decimals, 0, 4, false);
    buff[24] = '\0';
}

static void osdDisplayBatteryVoltage(uint8_t elemPosX, uint8_t elemPosY, uint16_t voltage, uint8_t digits, uint8_t decimals)
{
    char buff[7];
    textAttributes_t elemAttr = TEXT_ATTRIBUTES_NONE;

    osdFormatBatteryChargeSymbol(buff);
    buff[1] = '\0';
    osdUpdateBatteryCapacityOrVoltageTextAttributes(&elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, buff, elemAttr);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    digits = MIN(digits, 5);
    osdFormatCentiNumber(buff, voltage, 0, decimals, 0, digits, false);
    buff[digits] = SYM_VOLT;
    buff[digits+1] = '\0';
    const batteryState_e batteryVoltageState = checkBatteryVoltageState();
    if (batteryVoltageState == BATTERY_CRITICAL || batteryVoltageState == BATTERY_WARNING) {
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    }
    displayWriteWithAttr(osdDisplayPort, elemPosX + 1, elemPosY, buff, elemAttr);
}

static void osdDisplayFlightPIDValues(uint8_t elemPosX, uint8_t elemPosY, const char *str, pidIndex_e pidIndex, adjustmentFunction_e adjFuncP, adjustmentFunction_e adjFuncI, adjustmentFunction_e adjFuncD, adjustmentFunction_e adjFuncFF)
{
    textAttributes_t elemAttr;
    char buff[4];

    const pid8_t *pid = &pidBank()->pid[pidIndex];
    pidType_e pidType = pidIndexGetType(pidIndex);

    displayWrite(osdDisplayPort, elemPosX, elemPosY, str);

    if (pidType == PID_TYPE_NONE) {
        // PID is not used in this configuration. Draw dashes.
        // XXX: Keep this in sync with the %3d format and spacing used below
        displayWrite(osdDisplayPort, elemPosX + 6, elemPosY, "-   -   -   -");
        return;
    }

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pid->P);
    if ((isAdjustmentFunctionSelected(adjFuncP)) || (((adjFuncP == ADJUSTMENT_ROLL_P) || (adjFuncP == ADJUSTMENT_PITCH_P)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_P))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY, buff, elemAttr);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pid->I);
    if ((isAdjustmentFunctionSelected(adjFuncI)) || (((adjFuncI == ADJUSTMENT_ROLL_I) || (adjFuncI == ADJUSTMENT_PITCH_I)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_I))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 8, elemPosY, buff, elemAttr);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pid->D);
    if ((isAdjustmentFunctionSelected(adjFuncD)) || (((adjFuncD == ADJUSTMENT_ROLL_D) || (adjFuncD == ADJUSTMENT_PITCH_D)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_D))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 12, elemPosY, buff, elemAttr);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pid->FF);
    if ((isAdjustmentFunctionSelected(adjFuncFF)) || (((adjFuncFF == ADJUSTMENT_ROLL_FF) || (adjFuncFF == ADJUSTMENT_PITCH_FF)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_FF))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 16, elemPosY, buff, elemAttr);
}

static void osdDisplayNavPIDValues(uint8_t elemPosX, uint8_t elemPosY, const char *str, pidIndex_e pidIndex, adjustmentFunction_e adjFuncP, adjustmentFunction_e adjFuncI, adjustmentFunction_e adjFuncD)
{
    textAttributes_t elemAttr;
    char buff[4];

    const pid8_t *pid = &pidBank()->pid[pidIndex];
    pidType_e pidType = pidIndexGetType(pidIndex);

    displayWrite(osdDisplayPort, elemPosX, elemPosY, str);

    if (pidType == PID_TYPE_NONE) {
        // PID is not used in this configuration. Draw dashes.
        // XXX: Keep this in sync with the %3d format and spacing used below
        displayWrite(osdDisplayPort, elemPosX + 6, elemPosY, "-   -   -");
        return;
    }

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pid->P);
    if ((isAdjustmentFunctionSelected(adjFuncP)) || (((adjFuncP == ADJUSTMENT_ROLL_P) || (adjFuncP == ADJUSTMENT_PITCH_P)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_P))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY, buff, elemAttr);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pid->I);
    if ((isAdjustmentFunctionSelected(adjFuncI)) || (((adjFuncI == ADJUSTMENT_ROLL_I) || (adjFuncI == ADJUSTMENT_PITCH_I)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_I))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 8, elemPosY, buff, elemAttr);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    tfp_sprintf(buff, "%3d", pidType == PID_TYPE_PIFF ? pid->FF : pid->D);
    if ((isAdjustmentFunctionSelected(adjFuncD)) || (((adjFuncD == ADJUSTMENT_ROLL_D) || (adjFuncD == ADJUSTMENT_PITCH_D)) && (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_D))))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + 12, elemPosY, buff, elemAttr);
}

static void osdDisplayAdjustableDecimalValue(uint8_t elemPosX, uint8_t elemPosY, const char *str, const uint8_t valueOffset, const float value, const uint8_t valueLength, const uint8_t maxDecimals, adjustmentFunction_e adjFunc) {
    char buff[8];
    textAttributes_t elemAttr;
    displayWrite(osdDisplayPort, elemPosX, elemPosY, str);

    elemAttr = TEXT_ATTRIBUTES_NONE;
    osdFormatCentiNumber(buff, value * 100, 0, maxDecimals, 0, MIN(valueLength, 8), false);
    if (isAdjustmentFunctionSelected(adjFunc))
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    displayWriteWithAttr(osdDisplayPort, elemPosX + strlen(str) + 1 + valueOffset, elemPosY, buff, elemAttr);
}

int8_t getGeoWaypointNumber(int8_t waypointIndex)
{
    static int8_t lastWaypointIndex = 1;
    static int8_t geoWaypointIndex;

    if (waypointIndex != lastWaypointIndex) {
        lastWaypointIndex = geoWaypointIndex = waypointIndex;
        for (uint8_t i = posControl.startWpIndex; i <= waypointIndex; i++) {
            if (posControl.waypointList[i].action == NAV_WP_ACTION_SET_POI ||
                posControl.waypointList[i].action == NAV_WP_ACTION_SET_HEAD ||
                posControl.waypointList[i].action == NAV_WP_ACTION_JUMP) {
                    geoWaypointIndex -= 1;
            }
        }
    }

    return geoWaypointIndex - posControl.startWpIndex + 1;
}

void osdDisplaySwitchIndicator(const char *swName, int rcValue, char *buff) {
    int8_t ptr = 0;

    if (osdConfig()->osd_switch_indicators_align_left) {
        for (ptr = 0; ptr < constrain(strlen(swName), 0, OSD_SWITCH_INDICATOR_NAME_LENGTH); ptr++) {
            buff[ptr] = swName[ptr];
        }

        if ( rcValue < 1333) {
            buff[ptr++] = SYM_SWITCH_INDICATOR_LOW;
        } else if ( rcValue > 1666) {
            buff[ptr++] = SYM_SWITCH_INDICATOR_HIGH;
        } else {
            buff[ptr++] = SYM_SWITCH_INDICATOR_MID;
        }
    } else {
        if ( rcValue < 1333) {
            buff[ptr++] = SYM_SWITCH_INDICATOR_LOW;
        } else if ( rcValue > 1666) {
            buff[ptr++] = SYM_SWITCH_INDICATOR_HIGH;
        } else {
            buff[ptr++] = SYM_SWITCH_INDICATOR_MID;
        }

        for (ptr = 1; ptr < constrain(strlen(swName), 0, OSD_SWITCH_INDICATOR_NAME_LENGTH) + 1; ptr++) {
            buff[ptr] = swName[ptr-1];
        }

        ptr++;
    }

    buff[ptr] = '\0';
}

static bool osdElementEnabled(uint8_t elementID, bool onlyCurrentLayout) {
    bool elementEnabled = false;

    if (onlyCurrentLayout) {
        elementEnabled = OSD_VISIBLE(osdLayoutsConfig()->item_pos[currentLayout][elementID]);
    } else {
        for (uint8_t layout = 0; layout < 4 && !elementEnabled; layout++) {
            elementEnabled = OSD_VISIBLE(osdLayoutsConfig()->item_pos[layout][elementID]);
        }
    }

    return elementEnabled;
}

static bool osdDrawSingleElement(uint8_t item)
{
    uint16_t pos = osdLayoutsConfig()->item_pos[currentLayout][item];
    if (!OSD_VISIBLE(pos)) {
        return false;
    }
    uint8_t elemPosX = OSD_X(pos);
    uint8_t elemPosY = OSD_Y(pos);
    textAttributes_t elemAttr = TEXT_ATTRIBUTES_NONE;
    char buff[32] = {0};

    switch (item) {
    case OSD_CUSTOM_ELEMENT_1:
    {
        customElementDrawElement(buff, 0);
        break;
    }
    case OSD_CUSTOM_ELEMENT_2:
    {
        customElementDrawElement(buff, 1);
        break;
    }
    case OSD_CUSTOM_ELEMENT_3:
    {
        customElementDrawElement(buff, 2);
        break;
    }
    case OSD_CUSTOM_ELEMENT_4:
    {
        customElementDrawElement(buff, 3);
        break;
    }
    case OSD_CUSTOM_ELEMENT_5:
    {
        customElementDrawElement(buff, 4);
        break;
    }
    case OSD_CUSTOM_ELEMENT_6:
    {
        customElementDrawElement(buff, 5);
        break;
    }
    case OSD_CUSTOM_ELEMENT_7:
    {
        customElementDrawElement(buff, 6);
        break;
    }
    case OSD_CUSTOM_ELEMENT_8:
    {
        customElementDrawElement(buff, 7);
        break;
    }
    case OSD_RSSI_VALUE:
        {
            uint8_t osdRssi = osdConvertRSSI();
            buff[0] = SYM_RSSI;
            if (osdRssi < 100)
                tfp_sprintf(buff + 1, "%2d", osdRssi);
            else
                tfp_sprintf(buff + 1, " %c", SYM_MAX);
            
            if (osdRssi < osdConfig()->rssi_alarm) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            break;
        }

    case OSD_MAIN_BATT_VOLTAGE: {
        uint8_t base_digits = 2U;
#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
        if(isDJICompatibleVideoSystem(osdConfig())) {
            base_digits = 3U;   // Add extra digit to account for decimal point taking an extra character space
        }
#endif
        osdDisplayBatteryVoltage(elemPosX, elemPosY, getBatteryRawVoltage(), base_digits + osdConfig()->main_voltage_decimals, osdConfig()->main_voltage_decimals);
        return true;
    }

    case OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE: {
        uint8_t base_digits = 2U;
#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
        if(isDJICompatibleVideoSystem(osdConfig())) {
            base_digits = 3U;   // Add extra digit to account for decimal point taking an extra character space
        }
#endif
        osdDisplayBatteryVoltage(elemPosX, elemPosY, getBatterySagCompensatedVoltage(), base_digits + osdConfig()->main_voltage_decimals, osdConfig()->main_voltage_decimals);
        return true;
    }

    case OSD_CURRENT_DRAW: {
        osdFormatCentiNumber(buff, getAmperage(), 0, 2, 0, 3, false);
        buff[3] = SYM_AMP;
        buff[4] = '\0';

        uint8_t current_alarm = osdConfig()->current_alarm;
        if ((current_alarm > 0) && ((getAmperage() / 100.0f) > current_alarm)) {
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        }
        break;
    }

    case OSD_MAH_DRAWN: {
        uint8_t mah_digits = osdConfig()->mAh_precision; // Initialize to config value

#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
        if (isDJICompatibleVideoSystem(osdConfig())) {
            //DJIcompat is unable to work with scaled values and it only has mAh symbol to work with
            tfp_sprintf(buff, "%5d", (int)getMAhDrawn());   // Use 5 digits to allow packs below 100Ah
            buff[5] = SYM_MAH;
            buff[6] = '\0';
        } else
#endif
        {
            if (osdFormatCentiNumber(buff, getMAhDrawn() * 100, 1000, 0, 2, mah_digits, false)) {
                // Shown in Ah
                buff[mah_digits] = SYM_AH;
            } else {
                // Shown in mAh
                buff[mah_digits] = SYM_MAH;
            }
            buff[mah_digits + 1] = '\0';
        }

        osdUpdateBatteryCapacityOrVoltageTextAttributes(&elemAttr);
        break;
    }

    case OSD_WH_DRAWN:
        osdFormatCentiNumber(buff, getMWhDrawn() / 10, 0, 2, 0, 3, false);
        osdUpdateBatteryCapacityOrVoltageTextAttributes(&elemAttr);
        buff[3] = SYM_WH;
        buff[4] = '\0';
        break;

    case OSD_BATTERY_REMAINING_CAPACITY:
    {
        bool unitsDrawn = false;

        if (currentBatteryProfile->capacity.value == 0)
            tfp_sprintf(buff, "  NA");
        else if (!batteryWasFullWhenPluggedIn())
            tfp_sprintf(buff, "  NF");
        else if (batteryMetersConfig()->capacity_unit == BAT_CAPACITY_UNIT_MAH) {
            uint8_t mah_digits = osdConfig()->mAh_precision; // Initialize to config value

#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
            if (isDJICompatibleVideoSystem(osdConfig())) {
                //DJIcompat is unable to work with scaled values and it only has mAh symbol to work with
                tfp_sprintf(buff, "%5d", (int)getBatteryRemainingCapacity());   // Use 5 digits to allow packs below 100Ah
                buff[5] = SYM_MAH;
                buff[6] = '\0';
                unitsDrawn = true;
            } else
#endif
            {
                if (osdFormatCentiNumber(buff, getBatteryRemainingCapacity() * 100, 1000, 0, 2, mah_digits, false)) {
                    // Shown in Ah
                    buff[mah_digits] = SYM_AH;
                } else {
                    // Shown in mAh
                    buff[mah_digits] = SYM_MAH;
                }
                buff[mah_digits + 1] = '\0';
                unitsDrawn = true;
            }
        } else // batteryMetersConfig()->capacityUnit == BAT_CAPACITY_UNIT_MWH
            osdFormatCentiNumber(buff + 1, getBatteryRemainingCapacity() / 10, 0, 2, 0, 3, false);

        if (!unitsDrawn) {
        buff[4] = batteryMetersConfig()->capacity_unit == BAT_CAPACITY_UNIT_MAH ? SYM_MAH : SYM_WH;
        buff[5] = '\0';
        }

        if (batteryUsesCapacityThresholds()) {
            osdUpdateBatteryCapacityOrVoltageTextAttributes(&elemAttr);
        }

        break;
    }
    case OSD_BATTERY_REMAINING_PERCENT:
        osdFormatBatteryChargeSymbol(buff);
        tfp_sprintf(buff + 1, "%3d%%", calculateBatteryPercentage());
        osdUpdateBatteryCapacityOrVoltageTextAttributes(&elemAttr);
        break;

    case OSD_POWER_SUPPLY_IMPEDANCE:
        if (isPowerSupplyImpedanceValid())
            tfp_sprintf(buff, "%3d", getPowerSupplyImpedance());
        else
            strcpy(buff, "---");
        buff[3] = SYM_MILLIOHM;
        buff[4] = '\0';
        break;

#ifdef USE_GPS
    case OSD_GPS_SATS:
        buff[0] = SYM_SAT_L;
        buff[1] = SYM_SAT_R;
        tfp_sprintf(buff + 2, "%2d", gpsSol.numSat);
#ifdef USE_GPS_FIX_ESTIMATION
        if (STATE(GPS_ESTIMATED_FIX)) {
            strcpy(buff + 2, "ES");
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        } else
#endif
        if (!STATE(GPS_FIX)) {
            hardwareSensorStatus_e sensorStatus = getHwGPSStatus();
            if (sensorStatus == HW_SENSOR_UNAVAILABLE || sensorStatus == HW_SENSOR_UNHEALTHY) {
                buff[2] = SYM_ALERT;
                buff[3] = '\0';
            }
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        }
        break;

    case OSD_GPS_SPEED:
        osdFormatVelocityStr(buff, gpsSol.groundSpeed, false, false);
        break;

    case OSD_GPS_MAX_SPEED:
        osdFormatVelocityStr(buff, stats.max_speed, false, true);
        break;

    case OSD_3D_SPEED:
        osdFormatVelocityStr(buff, osdGet3DSpeed(), true, false);
        break;

    case OSD_3D_MAX_SPEED:
        osdFormatVelocityStr(buff, stats.max_3D_speed, true, true);
        break;

    case OSD_GLIDESLOPE:
        {
            float horizontalSpeed = gpsSol.groundSpeed;
            float sinkRate = -getEstimatedActualVelocity(Z);
            static pt1Filter_t gsFilterState;
            const timeMs_t currentTimeMs = millis();
            static timeMs_t gsUpdatedTimeMs;
            float glideSlope = horizontalSpeed / sinkRate;
            glideSlope = pt1FilterApply4(&gsFilterState, isnormal(glideSlope) ? glideSlope : 200, 0.5, MS2S(currentTimeMs - gsUpdatedTimeMs));
            gsUpdatedTimeMs = currentTimeMs;

            buff[0] = SYM_GLIDESLOPE;
            if (glideSlope > 0.0f && glideSlope < 100.0f) {
                osdFormatCentiNumber(buff + 1, glideSlope * 100.0f, 0, 2, 0, 3, false);
            } else {
                buff[1] = buff[2] = buff[3] = '-';
            }
            buff[4] = '\0';
            break;
        }

    case OSD_GPS_LAT:
        osdFormatCoordinate(buff, SYM_LAT, gpsSol.llh.lat);
        break;

    case OSD_GPS_LON:
        osdFormatCoordinate(buff, SYM_LON, gpsSol.llh.lon);
        break;

    case OSD_HOME_DIR:
        {
            if ((STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
                    || STATE(GPS_ESTIMATED_FIX)
#endif
                    ) && STATE(GPS_FIX_HOME) && isImuHeadingValid()) {
                if (GPS_distanceToHome < (navConfig()->general.min_rth_distance / 100) ) {
                    displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_HOME_NEAR);
                }
                else
                {
                    int16_t panHomeDirOffset = 0;
                    if (!(osdConfig()->pan_servo_pwm2centideg == 0)){
                        panHomeDirOffset = osdGetPanServoOffset();
                    }
                    int16_t flightDirection = STATE(AIRPLANE) ? CENTIDEGREES_TO_DEGREES(posControl.actualState.cog) : DECIDEGREES_TO_DEGREES(osdGetHeading());
                    int homeDirection = GPS_directionToHome - flightDirection + panHomeDirOffset;
                    osdDrawDirArrow(osdDisplayPort, osdGetDisplayPortCanvas(), OSD_DRAW_POINT_GRID(elemPosX, elemPosY), homeDirection);
                }
            } else {
                // No home or no fix or unknown heading, blink.
                // If we're unarmed, show the arrow pointing up so users can see the arrow
                // while configuring the OSD. If we're armed, show a '-' indicating that
                // we don't know the direction to home.
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
                displayWriteCharWithAttr(osdDisplayPort, elemPosX, elemPosY, ARMING_FLAG(ARMED) ? '-' : SYM_ARROW_UP, elemAttr);
            }
            return true;
        }

    case OSD_HOME_HEADING_ERROR:
        {
            buff[0] = SYM_HOME;
            buff[1] = SYM_HEADING;

            if (isImuHeadingValid() && navigationPositionEstimateIsHealthy()) {
                int16_t h = lrintf(CENTIDEGREES_TO_DEGREES((float)wrap_18000(DEGREES_TO_CENTIDEGREES((int32_t)GPS_directionToHome) - (STATE(AIRPLANE) ? posControl.actualState.cog : DECIDEGREES_TO_CENTIDEGREES((int32_t)osdGetHeading())))));
                tfp_sprintf(buff + 2, "%4d", h);
            } else {
                strcpy(buff + 2, "----");
            }

            buff[6] = SYM_DEGREES;
            buff[7] = '\0';
            break;
        }

    case OSD_HOME_DIST:
        {
            buff[0] = SYM_HOME;
            uint32_t distance_to_home_cm = GPS_distanceToHome * 100;
            osdFormatDistanceSymbol(&buff[1], distance_to_home_cm, 0, osdConfig()->decimals_distance);

            uint16_t dist_alarm = osdConfig()->dist_alarm;
            if (dist_alarm > 0 && GPS_distanceToHome > dist_alarm) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
        }
        break;

    case OSD_TRIP_DIST:
        buff[0] = SYM_TOTAL;
        osdFormatDistanceSymbol(buff + 1, getTotalTravelDistance(), 0, osdConfig()->decimals_distance);
        break;

    case OSD_BLACKBOX:
        {
#ifdef USE_BLACKBOX
            if (IS_RC_MODE_ACTIVE(BOXBLACKBOX)) {
                if (!isBlackboxDeviceWorking()) {
                    tfp_sprintf(buff, "%c%c", SYM_BLACKBOX, SYM_ALERT);
                } else if (isBlackboxDeviceFull()) {
                    tfp_sprintf(buff, "%cFULL", SYM_BLACKBOX);
                } else {
                    int32_t logNumber = blackboxGetLogNumber();
                    if (logNumber >= 0) {
                        tfp_sprintf(buff, "%c%05" PRId32, SYM_BLACKBOX, logNumber);
                    } else {
                        tfp_sprintf(buff, "%c", SYM_BLACKBOX);
                    }
                }
            }
#endif // USE_BLACKBOX
        }
        break;

    case OSD_ODOMETER:
        {
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_ODOMETER);
            float_t odometerDist = CENTIMETERS_TO_METERS(getTotalTravelDistance());
#ifdef USE_STATS
            odometerDist+= statsConfig()->stats_total_dist;
#endif

            switch (osdConfig()->units) {
                case OSD_UNIT_UK:
                    FALLTHROUGH;
                case OSD_UNIT_IMPERIAL:
                    osdFormatCentiNumber(buff, METERS_TO_MILES(odometerDist) * 100, 1, 1, 1, 6, true);
                    buff[6] = SYM_MI;
                    break;
                default:
                case OSD_UNIT_GA:
                    osdFormatCentiNumber(buff, METERS_TO_NAUTICALMILES(odometerDist) * 100, 1, 1, 1, 6, true);
                    buff[6] = SYM_NM;
                    break;
                case OSD_UNIT_METRIC_MPH:
                    FALLTHROUGH;
                case OSD_UNIT_METRIC:
                    osdFormatCentiNumber(buff, METERS_TO_KILOMETERS(odometerDist) * 100, 1, 1, 1, 6, true);
                    buff[6] = SYM_KM;
                    break;
            }
            buff[7] = '\0';
            elemPosX++;
        }
        break;

    case OSD_GROUND_COURSE:
        {
            buff[0] = SYM_GROUND_COURSE;
            if (osdIsHeadingValid()) {
                tfp_sprintf(&buff[1], "%3d", (int16_t)CENTIDEGREES_TO_DEGREES(posControl.actualState.cog));
            } else {
                buff[1] = buff[2] = buff[3] = '-';
            }
            buff[4] = SYM_DEGREES;
            buff[5] = '\0';
            break;
        }

    case OSD_COURSE_HOLD_ERROR:
        {
            if (ARMING_FLAG(ARMED) && !FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) {
                displayWrite(osdDisplayPort, elemPosX, elemPosY, "     ");
                return true;
            }

            buff[0] = SYM_HEADING;

            if ((!ARMING_FLAG(ARMED)) || (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && isAdjustingPosition())) {
                buff[1] = buff[2] = buff[3] = '-';
            } else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) {
                int16_t herr = lrintf(CENTIDEGREES_TO_DEGREES((float)navigationGetHeadingError()));
                if (ABS(herr) > 99)
                    strcpy(buff + 1, ">99");
                else
                    tfp_sprintf(buff + 1, "%3d", herr);
            }

            buff[4] = SYM_DEGREES;
            buff[5] = '\0';
            break;
        }

    case OSD_COURSE_HOLD_ADJUSTMENT:
        {
            int16_t heading_adjust = lrintf(CENTIDEGREES_TO_DEGREES((float)getCruiseHeadingAdjustment()));

            if (ARMING_FLAG(ARMED) && ((!FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) || !(isAdjustingPosition() || isAdjustingHeading() || (heading_adjust != 0)))) {
                displayWrite(osdDisplayPort, elemPosX, elemPosY, "      ");
                return true;
            }

            buff[0] = SYM_HEADING;

            if (!ARMING_FLAG(ARMED)) {
                buff[1] = buff[2] = buff[3] = buff[4] = '-';
            } else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) {
                tfp_sprintf(buff + 1, "%4d", heading_adjust);
            }

            buff[5] = SYM_DEGREES;
            buff[6] = '\0';
            break;
        }

    case OSD_CROSS_TRACK_ERROR:
        {
            if (isWaypointNavTrackingActive()) {
                buff[0] = SYM_CROSS_TRACK_ERROR;
                osdFormatDistanceSymbol(buff + 1, navigationGetCrossTrackError(), 0, 3);
            } else {
                displayWrite(osdDisplayPort, elemPosX, elemPosY, "     ");
                return true;
            }
            break;
        }

    case OSD_GPS_HDOP:
        {
            buff[0] = SYM_HDP_L;
            buff[1] = SYM_HDP_R;
            int32_t centiHDOP = 100 * gpsSol.hdop / HDOP_SCALE;
            uint8_t digits = 2U;
#ifndef DISABLE_MSP_DJI_COMPAT   // IF DJICOMPAT is not supported, there's no need to check for it and change the values
            if (isDJICompatibleVideoSystem(osdConfig())) {
                digits = 3U;
            }
#endif
            osdFormatCentiNumber(&buff[2], centiHDOP, 0, 1, 0, digits, false);
            break;
        }
#ifdef USE_ADSB
        case OSD_ADSB_WARNING:
        {
            static uint8_t adsblen = 1;
            uint8_t arrowPositionX = 0;

            for (int i = 0; i <= adsblen; i++) {
                buff[i] = SYM_BLANK;
            }

            buff[adsblen]='\0';
            displayWrite(osdDisplayPort, elemPosX, elemPosY, buff); // clear any previous chars because variable element size
            adsblen=1;
            adsbVehicle_t *vehicle = findVehicleClosest();

            if(vehicle != NULL){
                recalculateVehicle(vehicle);
            }

            if (
                    vehicle != NULL &&
                    (vehicle->calculatedVehicleValues.dist > 0) &&
                    vehicle->calculatedVehicleValues.dist < METERS_TO_CENTIMETERS(osdConfig()->adsb_distance_warning) &&
                    (osdConfig()->adsb_ignore_plane_above_me_limit == 0 || METERS_TO_CENTIMETERS(osdConfig()->adsb_ignore_plane_above_me_limit) > vehicle->calculatedVehicleValues.verticalDistance)
            ){
                buff[0] = SYM_ADSB;
                osdFormatDistanceStr(&buff[1], (int32_t)vehicle->calculatedVehicleValues.dist);
                adsblen = strlen(buff);

                buff[adsblen-1] = SYM_BLANK;

                arrowPositionX = adsblen-1;
                osdFormatDistanceStr(&buff[adsblen], vehicle->calculatedVehicleValues.verticalDistance);
                adsblen = strlen(buff)-1;

                if (vehicle->calculatedVehicleValues.dist < METERS_TO_CENTIMETERS(osdConfig()->adsb_distance_alert)) {
                    TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
                }
            }

            buff[adsblen]='\0';
            displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, buff, elemAttr);

            if (arrowPositionX > 0){
                int16_t panHomeDirOffset = 0;
                if (osdConfig()->pan_servo_pwm2centideg != 0){
                    panHomeDirOffset = osdGetPanServoOffset();
                }
                int16_t flightDirection = STATE(AIRPLANE) ? CENTIDEGREES_TO_DEGREES(posControl.actualState.cog) : DECIDEGREES_TO_DEGREES(osdGetHeading());
                osdDrawDirArrow(osdDisplayPort, osdGetDisplayPortCanvas(), OSD_DRAW_POINT_GRID(elemPosX + arrowPositionX, elemPosY), CENTIDEGREES_TO_DEGREES(vehicle->calculatedVehicleValues.dir) - flightDirection + panHomeDirOffset);
            }

            return true;
        }
        case OSD_ADSB_INFO:
        {
            buff[0] = SYM_ADSB;
            if(getAdsbStatus()->vehiclesMessagesTotal > 0 || getAdsbStatus()->heartbeatMessagesTotal > 0){
                tfp_sprintf(buff + 1, "%2d", getActiveVehiclesCount());
            }else{
                buff[1] = '-';
            }

            break;
        }

#endif
    case OSD_MAP_NORTH:
        {
            static uint16_t drawn = 0;
            static uint32_t scale = 0;
            osdDrawHomeMap(0, 'N', &drawn, &scale);
            return true;
        }
    case OSD_MAP_TAKEOFF:
        {
            static uint16_t drawn = 0;
            static uint32_t scale = 0;
            osdDrawHomeMap(CENTIDEGREES_TO_DEGREES(navigationGetHomeHeading()), 'T', &drawn, &scale);
            return true;
        }
    case OSD_RADAR:
        {
            static uint16_t drawn = 0;
            static uint32_t scale = 0;
            osdDrawRadar(&drawn, &scale);
            return true;
        }
#endif // GPS

    case OSD_ALTITUDE:
        {
            int32_t alt = osdGetAltitude();
            osdFormatAltitudeSymbol(buff, alt);

            uint16_t alt_alarm = osdConfig()->alt_alarm;
            uint16_t neg_alt_alarm = osdConfig()->neg_alt_alarm;
            if ((alt_alarm > 0 && CENTIMETERS_TO_METERS(alt) > alt_alarm) ||
                (neg_alt_alarm > 0 && alt < 0 && -CENTIMETERS_TO_METERS(alt) > neg_alt_alarm)) {

                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, buff, elemAttr);

            if (STATE(MULTIROTOR) && posControl.flags.isAdjustingAltitude) {
                /* Indicate MR altitude adjustment active with constant symbol at first blank position.
                 * Alternate symbol on/off with 600ms cycle if first position not blank (to maintain visibility of -ve sign) */
                int8_t blankPos;
                for (blankPos = 2; blankPos >= 0; blankPos--) {
                    if (buff[blankPos] == SYM_BLANK) {
            break;
        }
                }
                if (blankPos >= 0 || OSD_ALTERNATING_CHOICES(600, 2) == 0) {
                    blankPos = blankPos < 0 ? 0 : blankPos;
                    displayWriteChar(osdDisplayPort, elemPosX + blankPos, elemPosY, SYM_TERRAIN_FOLLOWING);
                }
            }
            return true;
        }

    case OSD_ALTITUDE_MSL:
        {
            int32_t alt = osdGetAltitudeMsl();
            osdFormatAltitudeSymbol(buff, alt);
            break;
        }

#ifdef USE_RANGEFINDER
    case OSD_RANGEFINDER:
        {
            int32_t range = rangefinderGetLatestRawAltitude();
            if (range < 0) {
                buff[0] = '-';
                buff[1] = '-';
                buff[2] = '-';
            } else {
                osdFormatDistanceSymbol(buff, range, 1, 3);
            }
        }
        break;
#endif

    case OSD_ONTIME:
        {
            osdFormatOnTime(buff);
            break;
        }

    case OSD_FLYTIME:
        {
            osdFormatFlyTime(buff, &elemAttr);
            break;
        }

    case OSD_ONTIME_FLYTIME:
        {
            if (ARMING_FLAG(ARMED)) {
                osdFormatFlyTime(buff, &elemAttr);
            } else {
                osdFormatOnTime(buff);
            }
            break;
        }

    case OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH:
        {
            /*static int32_t updatedTimeSeconds = 0;*/
            static int32_t timeSeconds = -1;
#if defined(USE_ADC) && defined(USE_GPS)
            static timeUs_t updatedTimestamp = 0;
            timeUs_t currentTimeUs = micros();
            if (cmpTimeUs(currentTimeUs, updatedTimestamp) >= MS2US(1000)) {
#ifdef USE_WIND_ESTIMATOR
                timeSeconds = calculateRemainingFlightTimeBeforeRTH(osdConfig()->estimations_wind_compensation);
#else
                timeSeconds = calculateRemainingFlightTimeBeforeRTH(false);
#endif
                updatedTimestamp = currentTimeUs;
            }
#endif
            if ((!ARMING_FLAG(ARMED)) || (timeSeconds == -1)) {
                buff[0] = SYM_FLIGHT_MINS_REMAINING;
                strcpy(buff + 1, "--:--");
#if defined(USE_ADC) && defined(USE_GPS)
                updatedTimestamp = 0;
#endif
            } else if (timeSeconds == -2) {
                // Wind is too strong to come back with cruise throttle
                buff[0] = SYM_FLIGHT_MINS_REMAINING;
                buff[1] = buff[2] = buff[4] = buff[5] = SYM_WIND_HORIZONTAL;
                buff[3] = ':';
                buff[6] = '\0';
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            } else {
                osdFormatTime(buff, timeSeconds, SYM_FLIGHT_MINS_REMAINING, SYM_FLIGHT_HOURS_REMAINING);
                if (timeSeconds == 0)
                    TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
        }
        break;

    case OSD_REMAINING_DISTANCE_BEFORE_RTH:;
        static int32_t distanceMeters = -1;
#if defined(USE_ADC) && defined(USE_GPS)
        static timeUs_t updatedTimestamp = 0;
        timeUs_t currentTimeUs = micros();
        if (cmpTimeUs(currentTimeUs, updatedTimestamp) >= MS2US(1000)) {
#ifdef USE_WIND_ESTIMATOR
            distanceMeters = calculateRemainingDistanceBeforeRTH(osdConfig()->estimations_wind_compensation);
#else
            distanceMeters = calculateRemainingDistanceBeforeRTH(false);
#endif
            updatedTimestamp = currentTimeUs;
        }
#endif
        displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_FLIGHT_DIST_REMAINING);

        if ((!ARMING_FLAG(ARMED)) || (distanceMeters == -1)) {
            buff[osdConfig()->decimals_distance] = SYM_BLANK;
            buff[osdConfig()->decimals_distance + 1] = '\0';
            strcpy(buff, "---");
        } else if (distanceMeters == -2) {
            // Wind is too strong to come back with cruise throttle
            for (uint8_t i = 0; i < osdConfig()->decimals_distance; i++) {
                buff[i] = SYM_WIND_HORIZONTAL;
            }
            switch ((osd_unit_e)osdConfig()->units){
                case OSD_UNIT_UK:
                    FALLTHROUGH;
                case OSD_UNIT_IMPERIAL:
                    buff[osdConfig()->decimals_distance] = SYM_DIST_MI;
                    break;
                case OSD_UNIT_METRIC_MPH:
                    FALLTHROUGH;
                case OSD_UNIT_METRIC:
                    buff[osdConfig()->decimals_distance] = SYM_DIST_KM;
                    break;
                case OSD_UNIT_GA:
                    buff[osdConfig()->decimals_distance] = SYM_DIST_NM;
                    break;
            }
            buff[osdConfig()->decimals_distance+1] = '\0';
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        } else {
            osdFormatDistanceSymbol(buff, distanceMeters * 100, 0, osdConfig()->decimals_distance);
            if (distanceMeters == 0)
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        }
        elemPosX++;
        break;

    case OSD_FLYMODE:
        {
            char *p = "ACRO";
#ifdef USE_FW_AUTOLAND
            if (FLIGHT_MODE(NAV_FW_AUTOLAND))
                p = "LAND";
            else
#endif
            if (FLIGHT_MODE(FAILSAFE_MODE))
                p = "!FS!";
            else if (FLIGHT_MODE(MANUAL_MODE))
                p = "MANU";
            else if (FLIGHT_MODE(TURTLE_MODE))
                p = "TURT";
            else if (FLIGHT_MODE(NAV_RTH_MODE))
                p = isWaypointMissionRTHActive() ? "WRTH" : "RTH ";
            else if (FLIGHT_MODE(NAV_POSHOLD_MODE) && STATE(AIRPLANE))
                p = "LOTR";
            else if (FLIGHT_MODE(NAV_POSHOLD_MODE))
                p = "HOLD";
            else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && FLIGHT_MODE(NAV_ALTHOLD_MODE))
                p = "CRUZ";
            else if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE))
                p = "CRSH";
            else if (FLIGHT_MODE(NAV_WP_MODE))
                p = " WP ";
            else if (FLIGHT_MODE(NAV_ALTHOLD_MODE) && navigationRequiresAngleMode()) {
                // If navigationRequiresAngleMode() returns false when ALTHOLD is active,
                // it means it can be combined with ANGLE, HORIZON, ACRO, etc...
                // and its display is handled by OSD_MESSAGES rather than OSD_FLYMODE.
                // (Currently only applies to multirotor).
                p = " AH ";
            }
            else if (FLIGHT_MODE(ANGLE_MODE))
                p = "ANGL";
            else if (FLIGHT_MODE(HORIZON_MODE))
                p = "HOR ";
            else if (FLIGHT_MODE(ANGLEHOLD_MODE))
                p = "ANGH";

            displayWrite(osdDisplayPort, elemPosX, elemPosY, p);
            return true;
        }

    case OSD_CRAFT_NAME:
        osdFormatCraftName(buff);
        break;

    case OSD_PILOT_NAME:
        osdFormatPilotName(buff);
        break;

    case OSD_PILOT_LOGO:
        displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_PILOT_LOGO_SML_L);
        displayWriteChar(osdDisplayPort, elemPosX+1, elemPosY, SYM_PILOT_LOGO_SML_C);
        displayWriteChar(osdDisplayPort, elemPosX+2, elemPosY, SYM_PILOT_LOGO_SML_R);
        break;

    case OSD_THROTTLE_POS:
    {
        osdFormatThrottlePosition(buff, false, &elemAttr);
        break;
    }

    case OSD_VTX_CHANNEL:
        {
            vtxDeviceOsdInfo_t osdInfo;
            vtxCommonGetOsdInfo(vtxCommonDevice(), &osdInfo);

            tfp_sprintf(buff, "CH:%c%s:", osdInfo.bandLetter, osdInfo.channelName);
            displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);

            tfp_sprintf(buff, "%c", osdInfo.powerIndexLetter);
            if (isAdjustmentFunctionSelected(ADJUSTMENT_VTX_POWER_LEVEL)) TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            displayWriteWithAttr(osdDisplayPort, elemPosX + 6, elemPosY, buff, elemAttr);
            return true;
        }
        break;

    case OSD_VTX_POWER:
        {
            vtxDeviceOsdInfo_t osdInfo;
            vtxCommonGetOsdInfo(vtxCommonDevice(), &osdInfo);

            tfp_sprintf(buff, "%c", SYM_VTX_POWER);
            displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);

            tfp_sprintf(buff, "%c", osdInfo.powerIndexLetter);
            if (isAdjustmentFunctionSelected(ADJUSTMENT_VTX_POWER_LEVEL)) TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            displayWriteWithAttr(osdDisplayPort, elemPosX+1, elemPosY, buff, elemAttr);
            return true;
        }

#if defined(USE_SERIALRX_CRSF)
    case OSD_CRSF_RSSI_DBM:
        {
            int16_t rssi = rxLinkStatistics.uplinkRSSI;
            buff[0] = (rxLinkStatistics.activeAntenna == 0) ? SYM_RSSI : SYM_2RSS; // Separate symbols for each antenna
            if (rssi <= -100) {
                tfp_sprintf(buff + 1, "%4d%c", rssi, SYM_DBM);
            } else {
                tfp_sprintf(buff + 1, "%3d%c%c", rssi, SYM_DBM, ' ');
            }
            if (!failsafeIsReceivingRxData()){
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            } else if (osdConfig()->rssi_dbm_alarm && rssi < osdConfig()->rssi_dbm_alarm) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            break;
        }
    case OSD_CRSF_LQ:
        {
            buff[0] = SYM_LQ;
            int16_t statsLQ = rxLinkStatistics.uplinkLQ;
            int16_t scaledLQ = scaleRange(constrain(statsLQ, 0, 100), 0, 100, 170, 300);
            switch (osdConfig()->crsf_lq_format) {
                case OSD_CRSF_LQ_TYPE1:
                    if (!failsafeIsReceivingRxData()) {
                        tfp_sprintf(buff+1, "%3d", 0);
                    } else {
                        tfp_sprintf(buff+1, "%3d", rxLinkStatistics.uplinkLQ);
                    }
                    break;
                case OSD_CRSF_LQ_TYPE2:
                    if (!failsafeIsReceivingRxData()) {
                        tfp_sprintf(buff+1, "%s:%3d", " ", 0);
                    } else {
                        tfp_sprintf(buff+1, "%d:%3d", rxLinkStatistics.rfMode, rxLinkStatistics.uplinkLQ);
                    }
                    break;
                case OSD_CRSF_LQ_TYPE3:
                    if (!failsafeIsReceivingRxData()) {
                        tfp_sprintf(buff+1, "%3d", 0);
                    } else {
                        tfp_sprintf(buff+1, "%3d", rxLinkStatistics.rfMode >= 2 ? scaledLQ : rxLinkStatistics.uplinkLQ);
                    }
                    break;
            }
            if (!failsafeIsReceivingRxData()) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            } else if (rxLinkStatistics.uplinkLQ < osdConfig()->link_quality_alarm) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            break;
        }

    case OSD_CRSF_SNR_DB:
        {
            static pt1Filter_t snrFilterState;
            static timeMs_t snrUpdated = 0;
            int8_t snrFiltered = pt1FilterApply4(&snrFilterState, rxLinkStatistics.uplinkSNR, 0.5f, MS2S(millis() - snrUpdated));
            snrUpdated = millis();

            const char* showsnr = "-20";
            const char* hidesnr = "   ";
            if (snrFiltered > osdConfig()->snr_alarm) {
                if (cmsInMenu) {
                    buff[0] = SYM_SNR;
                    tfp_sprintf(buff + 1, "%s%c", showsnr, SYM_DB);
                } else {
                    buff[0] = SYM_BLANK;
                    tfp_sprintf(buff + 1, "%s%c", hidesnr, SYM_BLANK);
                }
            } else if (snrFiltered <= osdConfig()->snr_alarm) {
                buff[0] = SYM_SNR;
                if (snrFiltered <= -10) {
                    tfp_sprintf(buff + 1, "%3d%c", snrFiltered, SYM_DB);
                } else {
                    tfp_sprintf(buff + 1, "%2d%c%c", snrFiltered, SYM_DB, ' ');
                }
            }
            break;
        }

    case OSD_CRSF_TX_POWER:
        {
            if (!failsafeIsReceivingRxData())
                tfp_sprintf(buff, "%s%c", "    ", SYM_BLANK);
            else
                tfp_sprintf(buff, "%4d%c", rxLinkStatistics.uplinkTXPower, SYM_MW);
            break;
        }
#endif

    case OSD_FORMATION_FLIGHT:
    {
        static uint8_t currentPeerIndex = 0;
        static timeMs_t lastPeerSwitch;

        if ((STATE(GPS_FIX) && isImuHeadingValid())) {
            if ((radar_pois[currentPeerIndex].gps.lat == 0 || radar_pois[currentPeerIndex].gps.lon == 0 || radar_pois[currentPeerIndex].state >= 2) || (millis() > (osdConfig()->radar_peers_display_time * 1000) + lastPeerSwitch)) {
                lastPeerSwitch = millis();

                for(uint8_t i = 1; i < RADAR_MAX_POIS - 1; i++) {
                    uint8_t nextPeerIndex = (currentPeerIndex + i) % (RADAR_MAX_POIS - 1);
                    if (radar_pois[nextPeerIndex].gps.lat != 0 && radar_pois[nextPeerIndex].gps.lon != 0 && radar_pois[nextPeerIndex].state < 2) {
                        currentPeerIndex = nextPeerIndex;
                        break;
                    }
                }
            }

            radar_pois_t *currentPeer = &(radar_pois[currentPeerIndex]);
            if (currentPeer->gps.lat != 0 && currentPeer->gps.lon != 0 && currentPeer->state < 2) {
                fpVector3_t poi;
                geoConvertGeodeticToLocal(&poi, &posControl.gpsOrigin, &currentPeer->gps, GEO_ALT_RELATIVE);

                currentPeer->distance = calculateDistanceToDestination(&poi) / 100; // In m
                currentPeer->altitude = (int16_t )((currentPeer->gps.alt - osdGetAltitudeMsl()) / 100);
                currentPeer->direction = (int16_t )(calculateBearingToDestination(&poi) / 100); // In °

                int16_t panServoDirOffset = 0;
                if (osdConfig()->pan_servo_pwm2centideg != 0){
                    panServoDirOffset = osdGetPanServoOffset();
                }

                //line 1
                //[peer heading][peer ID][LQ][direction to peer]

                //[peer heading]
                int relativePeerHeading = osdGetHeadingAngle(currentPeer->heading - (int)DECIDEGREES_TO_DEGREES(osdGetHeading()));
                displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_DECORATION + ((relativePeerHeading + 22) / 45) % 8);

                //[peer ID]
                displayWriteChar(osdDisplayPort, elemPosX + 1, elemPosY, 65 + currentPeerIndex);

                //[LQ]
                displayWriteChar(osdDisplayPort, elemPosX + 2, elemPosY, SYM_HUD_SIGNAL_0 + currentPeer->lq);

                //[direction to peer]
                int directionToPeerError = wrap_180(osdGetHeadingAngle(currentPeer->direction) + panServoDirOffset - (int)DECIDEGREES_TO_DEGREES(osdGetHeading()));
                uint16_t iconIndexOffset = constrain(((directionToPeerError + 180) / 30), 0, 12);
                if (iconIndexOffset == 12) {
                    iconIndexOffset = 0; // Directly behind
                }
                displayWriteChar(osdDisplayPort, elemPosX + 3, elemPosY, SYM_HUD_CARDINAL + iconIndexOffset);


                //line 2
                switch ((osd_unit_e)osdConfig()->units) {
                    case OSD_UNIT_UK:
                                FALLTHROUGH;
                    case OSD_UNIT_IMPERIAL:
                        osdFormatCentiNumber(buff, CENTIMETERS_TO_CENTIFEET(currentPeer->distance * 100), FEET_PER_MILE, 0, 4, 4, false);
                        break;
                    case OSD_UNIT_GA:
                        osdFormatCentiNumber(buff, CENTIMETERS_TO_CENTIFEET(currentPeer->distance * 100), (uint32_t)FEET_PER_NAUTICALMILE, 0, 4, 4, false);
                        break;
                    default:
                                FALLTHROUGH;
                    case OSD_UNIT_METRIC_MPH:
                                FALLTHROUGH;
                    case OSD_UNIT_METRIC:
                        osdFormatCentiNumber(buff, currentPeer->distance * 100, METERS_PER_KILOMETER, 0, 4, 4, false);
                        break;
                }
                displayWrite(osdDisplayPort, elemPosX, elemPosY + 1, buff);


                //line 3
                displayWriteChar(osdDisplayPort, elemPosX, elemPosY + 2, (currentPeer->altitude >= 0) ? SYM_AH_DECORATION_UP : SYM_AH_DECORATION_DOWN);

                int altc = currentPeer->altitude;
                switch ((osd_unit_e)osdConfig()->units) {
                    case OSD_UNIT_UK:
                                FALLTHROUGH;
                    case OSD_UNIT_GA:
                                FALLTHROUGH;
                    case OSD_UNIT_IMPERIAL:
                        // Convert to feet
                        altc = CENTIMETERS_TO_FEET(altc * 100);
                        break;
                    default:
                                FALLTHROUGH;
                    case OSD_UNIT_METRIC_MPH:
                                FALLTHROUGH;
                    case OSD_UNIT_METRIC:
                        // Already in metres
                        break;
                }

                altc = ABS(constrain(altc, -999, 999));
                tfp_sprintf(buff, "%3d", altc);
                displayWrite(osdDisplayPort, elemPosX + 1, elemPosY + 2, buff);

                return true;
            }
        }

        //clear screen
        for(uint8_t i = 0; i < 4; i++){
            displayWriteChar(osdDisplayPort, elemPosX + i, elemPosY, SYM_BLANK);
            displayWriteChar(osdDisplayPort, elemPosX + i, elemPosY + 1, SYM_BLANK);
            displayWriteChar(osdDisplayPort, elemPosX + i, elemPosY + 2, SYM_BLANK);
        }

        return true;
    }

    case OSD_CROSSHAIRS: // Hud is a sub-element of the crosshair

        osdCrosshairPosition(&elemPosX, &elemPosY);
        osdHudDrawCrosshair(osdGetDisplayPortCanvas(), elemPosX, elemPosY);

        if (osdConfig()->hud_homing && (STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
            ) && STATE(GPS_FIX_HOME) && isImuHeadingValid()) {
            osdHudDrawHoming(elemPosX, elemPosY);
        }

        if ((STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
            || STATE(GPS_ESTIMATED_FIX)
#endif
            ) && isImuHeadingValid()) {

            if (osdConfig()->hud_homepoint || osdConfig()->hud_radar_disp > 0 || osdConfig()->hud_wp_disp > 0) {
                    osdHudClear();
            }

            // -------- POI : Home point

            if (osdConfig()->hud_homepoint) { // Display the home point (H)
                osdHudDrawPoi(GPS_distanceToHome, GPS_directionToHome, -osdGetAltitude() / 100, 0, SYM_HOME, 0 , 0);
            }

            // -------- POI : Nearby aircrafts from ESP32 radar

            if (osdConfig()->hud_radar_disp > 0) { // Display the POI from the radar
                for (uint8_t i = 0; i < osdConfig()->hud_radar_disp; i++) {
                    if (radar_pois[i].gps.lat != 0 && radar_pois[i].gps.lon != 0 && radar_pois[i].state < 2) { // state 2 means POI has been lost and must be skipped
                        fpVector3_t poi;
                        geoConvertGeodeticToLocal(&poi, &posControl.gpsOrigin, &radar_pois[i].gps, GEO_ALT_RELATIVE);
                        radar_pois[i].distance = calculateDistanceToDestination(&poi) / 100; // In meters

                        if (radar_pois[i].distance >= osdConfig()->hud_radar_range_min && radar_pois[i].distance <= osdConfig()->hud_radar_range_max) {
                            radar_pois[i].direction = calculateBearingToDestination(&poi) / 100; // In °
                            radar_pois[i].altitude = (radar_pois[i].gps.alt - osdGetAltitudeMsl()) / 100;
                            osdHudDrawPoi(radar_pois[i].distance, osdGetHeadingAngle(radar_pois[i].direction), radar_pois[i].altitude, 1, 65 + i, radar_pois[i].heading, radar_pois[i].lq);
                        }
                    }
                }
            }

            // -------- POI : Next waypoints from navigation

            if (osdConfig()->hud_wp_disp > 0 && posControl.waypointListValid && posControl.waypointCount > 0) { // Display the next waypoints
                gpsLocation_t wp2;
                int j;

                for (int i = osdConfig()->hud_wp_disp - 1; i >= 0 ; i--) { // Display in reverse order so the next WP is always written on top
                    j = posControl.activeWaypointIndex + i;
                    if (j > posControl.startWpIndex + posControl.waypointCount - 1) { // limit to max WP index for mission
                        break;
                    }
                    if (posControl.waypointList[j].lat != 0 && posControl.waypointList[j].lon != 0) {
                        wp2.lat = posControl.waypointList[j].lat;
                        wp2.lon = posControl.waypointList[j].lon;
                        wp2.alt = posControl.waypointList[j].alt;
                        fpVector3_t poi;
                        geoConvertGeodeticToLocal(&poi, &posControl.gpsOrigin, &wp2, waypointMissionAltConvMode(posControl.waypointList[j].p3));
                        int32_t altConvModeAltitude = waypointMissionAltConvMode(posControl.waypointList[j].p3) == GEO_ALT_ABSOLUTE ? osdGetAltitudeMsl() : osdGetAltitude();
                        j = getGeoWaypointNumber(j);
                        while (j > 9) j -= 10; // Only the last digit displayed if WP>=10, no room for more (48 = ascii 0)
                        osdHudDrawPoi(calculateDistanceToDestination(&poi) / 100, osdGetHeadingAngle(calculateBearingToDestination(&poi) / 100), (posControl.waypointList[j].alt - altConvModeAltitude)/ 100, 2, SYM_WAYPOINT, 48 + j, i);
                    }
                }
            }
        }

        return true;
        break;

    case OSD_ATTITUDE_ROLL:
        buff[0] = SYM_ROLL_LEVEL;
        if (ABS(attitude.values.roll) >= 1)
            buff[0] += (attitude.values.roll < 0 ? -1 : 1);
        osdFormatCentiNumber(buff + 1, DECIDEGREES_TO_CENTIDEGREES(ABS(attitude.values.roll)), 0, 1, 0, 3, false);
        break;

    case OSD_ATTITUDE_PITCH:
        if (ABS(attitude.values.pitch) < 1)
            buff[0] = 'P';
        else if (attitude.values.pitch > 0)
            buff[0] = SYM_PITCH_DOWN;
        else if (attitude.values.pitch < 0)
            buff[0] = SYM_PITCH_UP;
        osdFormatCentiNumber(buff + 1, DECIDEGREES_TO_CENTIDEGREES(ABS(attitude.values.pitch)), 0, 1, 0, 3, false);
        break;

    case OSD_ARTIFICIAL_HORIZON:
        {
            float rollAngle = DECIDEGREES_TO_RADIANS(attitude.values.roll);
            float pitchAngle = DECIDEGREES_TO_RADIANS(attitude.values.pitch);

            pitchAngle -= osdConfig()->ahi_camera_uptilt_comp ? DEGREES_TO_RADIANS(osdConfig()->camera_uptilt) : 0;
            pitchAngle += DEGREES_TO_RADIANS(getFixedWingLevelTrim());
            if (osdConfig()->ahi_reverse_roll) {
                rollAngle = -rollAngle;
            }
            osdDrawArtificialHorizon(osdDisplayPort, osdGetDisplayPortCanvas(),
                 OSD_DRAW_POINT_GRID(elemPosX, elemPosY), rollAngle, pitchAngle);
            osdDrawSingleElement(OSD_HORIZON_SIDEBARS);
            osdDrawSingleElement(OSD_CROSSHAIRS);

            return true;
        }

    case OSD_HORIZON_SIDEBARS:
        {
            osdDrawSidebars(osdDisplayPort, osdGetDisplayPortCanvas());
            return true;
        }

#if defined(USE_BARO) || defined(USE_GPS)
    case OSD_VARIO:
        {
            float zvel = getEstimatedActualVelocity(Z);
            osdDrawVario(osdDisplayPort, osdGetDisplayPortCanvas(), OSD_DRAW_POINT_GRID(elemPosX, elemPosY), zvel);
            return true;
        }

    case OSD_VARIO_NUM:
        {
            int16_t value = getEstimatedActualVelocity(Z);
            char sym;
            switch ((osd_unit_e)osdConfig()->units) {
                case OSD_UNIT_UK:
                    FALLTHROUGH;
                case OSD_UNIT_IMPERIAL:
                    // Convert to centifeet/s
                    value = CENTIMETERS_TO_CENTIFEET(value);
                    sym = SYM_FTS;
                    break;
                case OSD_UNIT_GA:
                    // Convert to centi-100feet/min
                    value = CENTIMETERS_TO_FEET(value * 60);
                    sym = SYM_100FTM;
                    break;
                default:
                case OSD_UNIT_METRIC_MPH:
                    FALLTHROUGH;
                case OSD_UNIT_METRIC:
                    // Already in cm/s
                    sym = SYM_MS;
                    break;
            }

            osdFormatCentiNumber(buff, value, 0, 1, 0, 3, false);
            buff[3] = sym;
            buff[4] = '\0';
            break;
        }
    case OSD_CLIMB_EFFICIENCY:
        {
            // amperage is in centi amps (10mA), vertical speed is in cms/s. We want
            // Ah/dist only to show when vertical speed > 1m/s.
            static pt1Filter_t veFilterState;
            static timeUs_t vEfficiencyUpdated = 0;
            int32_t value = 0;
            timeUs_t currentTimeUs = micros();
            timeDelta_t vEfficiencyTimeDelta = cmpTimeUs(currentTimeUs, vEfficiencyUpdated);
            if (getEstimatedActualVelocity(Z) > 0) {
                if (vEfficiencyTimeDelta >= EFFICIENCY_UPDATE_INTERVAL) {
                                                            // Centiamps (kept for osdFormatCentiNumber) / m/s - Will appear as A / m/s in OSD
                    value = pt1FilterApply4(&veFilterState, (float)getAmperage() / (getEstimatedActualVelocity(Z) / 100.0f), 1, US2S(vEfficiencyTimeDelta));

                    vEfficiencyUpdated = currentTimeUs;
                } else {
                    value = veFilterState.state;
                }
            }
            bool efficiencyValid = (value > 0) && (getEstimatedActualVelocity(Z) > 100);
            switch (osdConfig()->units) {
                case OSD_UNIT_UK:
                    FALLTHROUGH;
                case OSD_UNIT_GA:
                    FALLTHROUGH;
                case OSD_UNIT_IMPERIAL:
                    // mAh/foot
                    if (efficiencyValid) {
                        osdFormatCentiNumber(buff, (value * METERS_PER_FOOT), 1, 2, 2, 3, false);
                        tfp_sprintf(buff + strlen(buff), "%c%c", SYM_AH_V_FT_0, SYM_AH_V_FT_1);
                    } else {
                        buff[0] = buff[1] = buff[2] = '-';
                        buff[3] = SYM_AH_V_FT_0;
                        buff[4] = SYM_AH_V_FT_1;
                        buff[5] = '\0';
                    }
                    break;
                case OSD_UNIT_METRIC_MPH:
                    FALLTHROUGH;
                case OSD_UNIT_METRIC:
                    // mAh/metre
                    if (efficiencyValid) {
                        osdFormatCentiNumber(buff, value, 1, 2, 2, 3, false);
                        tfp_sprintf(buff + strlen(buff), "%c%c", SYM_AH_V_M_0, SYM_AH_V_M_1);
                    } else {
                        buff[0] = buff[1] = buff[2] = '-';
                        buff[3] = SYM_AH_V_M_0;
                        buff[4] = SYM_AH_V_M_1;
                        buff[5] = '\0';
                    }
                    break;
            }
            break;
        }
    case OSD_GLIDE_TIME_REMAINING:
        {
            uint16_t glideTime = osdGetRemainingGlideTime();
            buff[0] = SYM_GLIDE_MINS;
            if (glideTime > 0) {
                // Maximum value we can show in minutes is 99 minutes and 59 seconds. It is extremely unlikely that glide
                // time will be longer than 99 minutes. If it is, it will show 99:^^
                if (glideTime > (99 * 60) + 59) {
                    tfp_sprintf(buff + 1, "%02d:", (int)(glideTime / 60));
                    buff[4] = SYM_DECORATION;
                    buff[5] = SYM_DECORATION;
                } else {
                    tfp_sprintf(buff + 1, "%02d:%02d", (int)(glideTime / 60), (int)(glideTime % 60));
                }
            } else {
               tfp_sprintf(buff + 1, "%s", "--:--");
            }
            buff[6] = '\0';
            break;
        }
    case OSD_GLIDE_RANGE:
        {
            uint16_t glideSeconds = osdGetRemainingGlideTime();
            buff[0] = SYM_GLIDE_DIST;
            if (glideSeconds > 0) {
                uint32_t glideRangeCM = glideSeconds * gpsSol.groundSpeed;
                osdFormatDistanceSymbol(buff + 1, glideRangeCM, 0, 3);
            } else {
                tfp_sprintf(buff + 1, "%s%c", "---", SYM_BLANK);
                buff[5] = '\0';
            }
            break;
        }
#endif
    case OSD_SWITCH_INDICATOR_0:
        osdDisplaySwitchIndicator(osdConfig()->osd_switch_indicator0_name, rxGetChannelValue(osdConfig()->osd_switch_indicator0_channel - 1), buff);
        break;

    case OSD_SWITCH_INDICATOR_1:
        osdDisplaySwitchIndicator(osdConfig()->osd_switch_indicator1_name, rxGetChannelValue(osdConfig()->osd_switch_indicator1_channel - 1), buff);
        break;

    case OSD_SWITCH_INDICATOR_2:
        osdDisplaySwitchIndicator(osdConfig()->osd_switch_indicator2_name, rxGetChannelValue(osdConfig()->osd_switch_indicator2_channel - 1), buff);
        break;

    case OSD_SWITCH_INDICATOR_3:
        osdDisplaySwitchIndicator(osdConfig()->osd_switch_indicator3_name, rxGetChannelValue(osdConfig()->osd_switch_indicator3_channel - 1), buff);
        break;

    case OSD_PAN_SERVO_CENTRED:
        {
            int16_t panOffset = osdGetPanServoOffset();
            const timeMs_t panServoTimeNow = millis();
            static timeMs_t panServoTimeOffCentre = 0;

            if (panOffset < 0) {
                if (osdConfig()->pan_servo_offcentre_warning != 0 && panOffset >= -osdConfig()->pan_servo_offcentre_warning) {
                    if (panServoTimeOffCentre == 0) {
                        panServoTimeOffCentre = panServoTimeNow;
                    } else if (panServoTimeNow >= (panServoTimeOffCentre + 10000 )) {
                        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
                    }
                } else {
                    panServoTimeOffCentre = 0;
                }

                if (osdConfig()->pan_servo_indicator_show_degrees) {
                    tfp_sprintf(buff, "%3d%c", -panOffset, SYM_DEGREES);
                    displayWriteWithAttr(osdDisplayPort, elemPosX+1, elemPosY, buff, elemAttr);
                }
                displayWriteCharWithAttr(osdDisplayPort, elemPosX, elemPosY, SYM_SERVO_PAN_IS_OFFSET_R, elemAttr);
            } else if (panOffset > 0) {
                if (osdConfig()->pan_servo_offcentre_warning != 0 && panOffset <= osdConfig()->pan_servo_offcentre_warning) {
                    if (panServoTimeOffCentre == 0) {
                        panServoTimeOffCentre = panServoTimeNow;
                    } else if (panServoTimeNow >= (panServoTimeOffCentre + 10000 )) {
                        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
                    }
                } else {
                    panServoTimeOffCentre = 0;
                }

                if (osdConfig()->pan_servo_indicator_show_degrees) {
                    tfp_sprintf(buff, "%3d%c", panOffset, SYM_DEGREES);
                    displayWriteWithAttr(osdDisplayPort, elemPosX+1, elemPosY, buff, elemAttr);
                }
                displayWriteCharWithAttr(osdDisplayPort, elemPosX, elemPosY, SYM_SERVO_PAN_IS_OFFSET_L, elemAttr);
            } else {
                panServoTimeOffCentre = 0;

                if (osdConfig()->pan_servo_indicator_show_degrees) {
                    tfp_sprintf(buff, "%3d%c", panOffset, SYM_DEGREES);
                    displayWriteWithAttr(osdDisplayPort, elemPosX+1, elemPosY, buff, elemAttr);
                }
                displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_SERVO_PAN_IS_CENTRED);
            }

            return true;
        }
        break;

    case OSD_ACTIVE_PROFILE:
        tfp_sprintf(buff, "%c%u", SYM_PROFILE, (getConfigProfile() + 1));
        displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);
        break;

    case OSD_ROLL_PIDS:
        osdDisplayFlightPIDValues(elemPosX, elemPosY, "ROL", PID_ROLL, ADJUSTMENT_ROLL_P, ADJUSTMENT_ROLL_I, ADJUSTMENT_ROLL_D, ADJUSTMENT_ROLL_FF);
        return true;

    case OSD_PITCH_PIDS:
        osdDisplayFlightPIDValues(elemPosX, elemPosY, "PIT", PID_PITCH, ADJUSTMENT_PITCH_P, ADJUSTMENT_PITCH_I, ADJUSTMENT_PITCH_D, ADJUSTMENT_PITCH_FF);
        return true;

    case OSD_YAW_PIDS:
        osdDisplayFlightPIDValues(elemPosX, elemPosY, "YAW", PID_YAW, ADJUSTMENT_YAW_P, ADJUSTMENT_YAW_I, ADJUSTMENT_YAW_D, ADJUSTMENT_YAW_FF);
        return true;

    case OSD_LEVEL_PIDS:
        osdDisplayNavPIDValues(elemPosX, elemPosY, "LEV", PID_LEVEL, ADJUSTMENT_LEVEL_P, ADJUSTMENT_LEVEL_I, ADJUSTMENT_LEVEL_D);
        return true;

    case OSD_POS_XY_PIDS:
        osdDisplayNavPIDValues(elemPosX, elemPosY, "PXY", PID_POS_XY, ADJUSTMENT_POS_XY_P, ADJUSTMENT_POS_XY_I, ADJUSTMENT_POS_XY_D);
        return true;

    case OSD_POS_Z_PIDS:
        osdDisplayNavPIDValues(elemPosX, elemPosY, "PZ", PID_POS_Z, ADJUSTMENT_POS_Z_P, ADJUSTMENT_POS_Z_I, ADJUSTMENT_POS_Z_D);
        return true;

    case OSD_VEL_XY_PIDS:
        osdDisplayNavPIDValues(elemPosX, elemPosY, "VXY", PID_VEL_XY, ADJUSTMENT_VEL_XY_P, ADJUSTMENT_VEL_XY_I, ADJUSTMENT_VEL_XY_D);
        return true;

    case OSD_VEL_Z_PIDS:
        osdDisplayNavPIDValues(elemPosX, elemPosY, "VZ", PID_VEL_Z, ADJUSTMENT_VEL_Z_P, ADJUSTMENT_VEL_Z_I, ADJUSTMENT_VEL_Z_D);
        return true;

    case OSD_HEADING_P:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "HP", 0, pidBank()->pid[PID_HEADING].P, 3, 0, ADJUSTMENT_HEADING_P);
        return true;

    case OSD_BOARD_ALIGN_ROLL:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "AR", 0, DECIDEGREES_TO_DEGREES((float)boardAlignment()->rollDeciDegrees), 4, 1, ADJUSTMENT_ROLL_BOARD_ALIGNMENT);
        return true;

    case OSD_BOARD_ALIGN_PITCH:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "AP", 0, DECIDEGREES_TO_DEGREES((float)boardAlignment()->pitchDeciDegrees), 4, 1, ADJUSTMENT_PITCH_BOARD_ALIGNMENT);
        return true;

    case OSD_RC_EXPO:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "EXP", 0, currentControlRateProfile->stabilized.rcExpo8, 3, 0, ADJUSTMENT_RC_EXPO);
        return true;

    case OSD_RC_YAW_EXPO:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "YEX", 0, currentControlRateProfile->stabilized.rcYawExpo8, 3, 0, ADJUSTMENT_RC_YAW_EXPO);
        return true;

    case OSD_THROTTLE_EXPO:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "TEX", 0, currentControlRateProfile->throttle.rcExpo8, 3, 0, ADJUSTMENT_THROTTLE_EXPO);
        return true;

    case OSD_PITCH_RATE:
        displayWrite(osdDisplayPort, elemPosX, elemPosY, "SPR");

        elemAttr = TEXT_ATTRIBUTES_NONE;
        tfp_sprintf(buff, "%3d", currentControlRateProfile->stabilized.rates[FD_PITCH]);
        if (isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_RATE) || isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_RATE))
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY, buff, elemAttr);
        return true;

    case OSD_ROLL_RATE:
        displayWrite(osdDisplayPort, elemPosX, elemPosY, "SRR");

        elemAttr = TEXT_ATTRIBUTES_NONE;
        tfp_sprintf(buff, "%3d", currentControlRateProfile->stabilized.rates[FD_ROLL]);
        if (isAdjustmentFunctionSelected(ADJUSTMENT_ROLL_RATE) || isAdjustmentFunctionSelected(ADJUSTMENT_PITCH_ROLL_RATE))
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY, buff, elemAttr);
        return true;

    case OSD_YAW_RATE:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "SYR", 0, currentControlRateProfile->stabilized.rates[FD_YAW], 3, 0, ADJUSTMENT_YAW_RATE);
        return true;

    case OSD_MANUAL_RC_EXPO:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "MEX", 0, currentControlRateProfile->manual.rcExpo8, 3, 0, ADJUSTMENT_MANUAL_RC_EXPO);
        return true;

    case OSD_MANUAL_RC_YAW_EXPO:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "MYX", 0, currentControlRateProfile->manual.rcYawExpo8, 3, 0, ADJUSTMENT_MANUAL_RC_YAW_EXPO);
        return true;

    case OSD_MANUAL_PITCH_RATE:
        displayWrite(osdDisplayPort, elemPosX, elemPosY, "MPR");

        elemAttr = TEXT_ATTRIBUTES_NONE;
        tfp_sprintf(buff, "%3d", currentControlRateProfile->manual.rates[FD_PITCH]);
        if (isAdjustmentFunctionSelected(ADJUSTMENT_MANUAL_PITCH_RATE) || isAdjustmentFunctionSelected(ADJUSTMENT_MANUAL_PITCH_ROLL_RATE))
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY, buff, elemAttr);
        return true;

    case OSD_MANUAL_ROLL_RATE:
        displayWrite(osdDisplayPort, elemPosX, elemPosY, "MRR");

        elemAttr = TEXT_ATTRIBUTES_NONE;
        tfp_sprintf(buff, "%3d", currentControlRateProfile->manual.rates[FD_ROLL]);
        if (isAdjustmentFunctionSelected(ADJUSTMENT_MANUAL_ROLL_RATE) || isAdjustmentFunctionSelected(ADJUSTMENT_MANUAL_PITCH_ROLL_RATE))
            TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
        displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY, buff, elemAttr);
        return true;

    case OSD_MANUAL_YAW_RATE:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "MYR", 0, currentControlRateProfile->stabilized.rates[FD_YAW], 3, 0, ADJUSTMENT_YAW_RATE);
        return true;

    case OSD_NAV_FW_CRUISE_THR:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "CRZ", 0, currentBatteryProfile->nav.fw.cruise_throttle, 4, 0, ADJUSTMENT_NAV_FW_CRUISE_THR);
        return true;

    case OSD_NAV_FW_PITCH2THR:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "P2T", 0, currentBatteryProfile->nav.fw.pitch_to_throttle, 3, 0, ADJUSTMENT_NAV_FW_PITCH2THR);
        return true;

    case OSD_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE:
        osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "0TP", 0, (float)navConfig()->fw.minThrottleDownPitchAngle / 10, 3, 1, ADJUSTMENT_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE);
        return true;

    case OSD_FW_ALT_PID_OUTPUTS:
        {
            const navigationPIDControllers_t *nav_pids = getNavigationPIDControllers();
            osdFormatPidControllerOutput(buff, "PZO", &nav_pids->fw_alt, 10, true); // display requested pitch degrees
            break;
        }

    case OSD_FW_POS_PID_OUTPUTS:
        {
            const navigationPIDControllers_t *nav_pids = getNavigationPIDControllers(); // display requested roll degrees
            osdFormatPidControllerOutput(buff, "PXYO", &nav_pids->fw_nav, 1, true);
            break;
        }

    case OSD_MC_VEL_Z_PID_OUTPUTS:
        {
            const navigationPIDControllers_t *nav_pids = getNavigationPIDControllers();
            osdFormatPidControllerOutput(buff, "VZO", &nav_pids->vel[Z], 100, false); // display throttle adjustment µs
            break;
        }

    case OSD_MC_VEL_X_PID_OUTPUTS:
        {
            const navigationPIDControllers_t *nav_pids = getNavigationPIDControllers();
            osdFormatPidControllerOutput(buff, "VXO", &nav_pids->vel[X], 100, false); // display requested acceleration cm/s^2
            break;
        }

    case OSD_MC_VEL_Y_PID_OUTPUTS:
        {
            const navigationPIDControllers_t *nav_pids = getNavigationPIDControllers();
            osdFormatPidControllerOutput(buff, "VYO", &nav_pids->vel[Y], 100, false); // display requested acceleration cm/s^2
            break;
        }

    case OSD_MC_POS_XYZ_P_OUTPUTS:
        {
            const navigationPIDControllers_t *nav_pids = getNavigationPIDControllers();
            strcpy(buff, "POSO ");
            // display requested velocity cm/s
            tfp_sprintf(buff + 5, "%4d", (int)lrintf(nav_pids->pos[X].output_constrained * 100));
            buff[9] = ' ';
            tfp_sprintf(buff + 10, "%4d", (int)lrintf(nav_pids->pos[Y].output_constrained * 100));
            buff[14] = ' ';
            tfp_sprintf(buff + 15, "%4d", (int)lrintf(nav_pids->pos[Z].output_constrained * 100));
            buff[19] = '\0';
            break;
        }

    case OSD_POWER:
        {
            bool kiloWatt = osdFormatCentiNumber(buff, getPower(), 1000, 2, 2, 3, false);
            buff[3] = kiloWatt ? SYM_KILOWATT : SYM_WATT;
            buff[4] = '\0';

            uint8_t current_alarm = osdConfig()->current_alarm;
            if ((current_alarm > 0) && ((getAmperage() / 100.0f) > current_alarm)) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            break;
        }

    case OSD_AIR_SPEED:
        {
        #ifdef USE_PITOT
            buff[0] = SYM_AIR;

            if (pitotIsHealthy())
            {
                const float airspeed_estimate = getAirspeedEstimate();
                osdFormatVelocityStr(buff + 1, airspeed_estimate, false, false);
                if ((osdConfig()->airspeed_alarm_min != 0 && airspeed_estimate < osdConfig()->airspeed_alarm_min) ||
                    (osdConfig()->airspeed_alarm_max != 0 && airspeed_estimate > osdConfig()->airspeed_alarm_max)) {
                        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
                }
            }
            else
            {
                strcpy(buff + 1, "  X!");
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
        #else
            return false;
        #endif
            break;
        }

    case OSD_AIR_MAX_SPEED:
        {
        #ifdef USE_PITOT
            buff[0] = SYM_MAX;
            buff[1] = SYM_AIR;
            osdFormatVelocityStr(buff + 2, stats.max_air_speed, false, false);
        #else
            return false;
        #endif
            break;
        }

    case OSD_RTC_TIME:
        {
            // RTC not configured will show 00:00
            dateTime_t dateTime;
            rtcGetDateTimeLocal(&dateTime);
            buff[0] = SYM_CLOCK;
            tfp_sprintf(buff + 1, "%02u:%02u:%02u", dateTime.hours, dateTime.minutes, dateTime.seconds);
            break;
        }

    case OSD_MESSAGES:
        {
            elemAttr = osdGetSystemMessage(buff, OSD_MESSAGE_LENGTH, true);
            break;
        }

    case OSD_VERSION:
        {
            tfp_sprintf(buff, "INAV %s", FC_VERSION_STRING);
            displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);
            break;
        }

    case OSD_MAIN_BATT_CELL_VOLTAGE:
        {
            uint8_t base_digits = 3U;
#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
            if(isDJICompatibleVideoSystem(osdConfig())) {
                base_digits = 4U;   // Add extra digit to account for decimal point taking an extra character space
            }
#endif
            osdDisplayBatteryVoltage(elemPosX, elemPosY, getBatteryRawAverageCellVoltage(), base_digits, 2);
            return true;
        }

    case OSD_MAIN_BATT_SAG_COMPENSATED_CELL_VOLTAGE:
        {
            uint8_t base_digits = 3U;
#ifndef DISABLE_MSP_DJI_COMPAT // IF DJICOMPAT is not supported, there's no need to check for it
            if(isDJICompatibleVideoSystem(osdConfig())) {
                base_digits = 4U;   // Add extra digit to account for decimal point taking an extra character space
            }
#endif
            osdDisplayBatteryVoltage(elemPosX, elemPosY, getBatterySagCompensatedAverageCellVoltage(), base_digits, 2);
            return true;
        }

    case OSD_SCALED_THROTTLE_POS:
        {
            osdFormatThrottlePosition(buff, true, &elemAttr);
            break;
        }

    case OSD_HEADING:
        {
            buff[0] = SYM_HEADING;
            if (osdIsHeadingValid()) {
                int16_t h = DECIDEGREES_TO_DEGREES(osdGetHeading());
                if (h < 0) {
                    h += 360;
                }
                tfp_sprintf(&buff[1], "%3d", h);
            } else {
                buff[1] = buff[2] = buff[3] = '-';
            }
            buff[4] = SYM_DEGREES;
            buff[5] = '\0';
            break;
        }

    case OSD_HEADING_GRAPH:
        {
            if (osdIsHeadingValid()) {
                osdDrawHeadingGraph(osdDisplayPort, osdGetDisplayPortCanvas(), OSD_DRAW_POINT_GRID(elemPosX, elemPosY), osdGetHeading());
                return true;
            } else {
                buff[0] = buff[2] = buff[4] = buff[6] = buff[8] = SYM_HEADING_LINE;
                buff[1] = buff[3] = buff[5] = buff[7] = SYM_HEADING_DIVIDED_LINE;
                buff[OSD_HEADING_GRAPH_WIDTH] = '\0';
            }
            break;
        }

    case OSD_EFFICIENCY_MAH_PER_KM:
        {
            // amperage is in centi amps, speed is in cms/s. We want
            // mah/km. Only show when ground speed > 1m/s.
            static pt1Filter_t eFilterState;
            static timeUs_t efficiencyUpdated = 0;
            int32_t value = 0;
            bool moreThanAh = false;
            timeUs_t currentTimeUs = micros();
            timeDelta_t efficiencyTimeDelta = cmpTimeUs(currentTimeUs, efficiencyUpdated);
            uint8_t digits = 3U;
#ifndef DISABLE_MSP_DJI_COMPAT   // IF DJICOMPAT is not supported, there's no need to check for it and change the values
            if (isDJICompatibleVideoSystem(osdConfig())) {
                // Increase number of digits so values above 99 don't get scaled by osdFormatCentiNumber
                digits = 4U;
            }
#endif
            if ((STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
                || STATE(GPS_ESTIMATED_FIX)
#endif
                ) && gpsSol.groundSpeed > 0) {
                if (efficiencyTimeDelta >= EFFICIENCY_UPDATE_INTERVAL) {
                    value = pt1FilterApply4(&eFilterState, ((float)getAmperage() / gpsSol.groundSpeed) / 0.0036f,
                        1, US2S(efficiencyTimeDelta));

                    efficiencyUpdated = currentTimeUs;
                } else {
                    value = eFilterState.state;
                }
            }
            bool efficiencyValid = (value > 0) && (gpsSol.groundSpeed > 100);
            switch (osdConfig()->units) {
                case OSD_UNIT_UK:
                    FALLTHROUGH;
                case OSD_UNIT_IMPERIAL:
                    moreThanAh = osdFormatCentiNumber(buff, value * METERS_PER_MILE / 10, 1000, 0, 2, digits, false);
                    if (!moreThanAh) {
                        tfp_sprintf(buff + strlen(buff), "%c%c", SYM_MAH_MI_0, SYM_MAH_MI_1);
                    } else {
                        tfp_sprintf(buff + strlen(buff), "%c", SYM_AH_MI);
                    }
                    if (!efficiencyValid) {
                        buff[0] = buff[1] = buff[2] = buff[3] = '-';
                        buff[digits] = SYM_MAH_MI_0;        // This will overwrite the "-" at buff[3] if not in DJICOMPAT mode
                        buff[digits + 1] = SYM_MAH_MI_1;
                        buff[digits + 2] = '\0';
                    }
                    break;
                case OSD_UNIT_GA:
                     moreThanAh = osdFormatCentiNumber(buff, value * METERS_PER_NAUTICALMILE / 10, 1000, 0, 2, digits, false);
                    if (!moreThanAh) {
                        tfp_sprintf(buff + strlen(buff), "%c%c", SYM_MAH_NM_0, SYM_MAH_NM_1);
                    } else {
                        tfp_sprintf(buff + strlen(buff), "%c", SYM_AH_NM);
                    }
                    if (!efficiencyValid) {
                        buff[0] = buff[1] = buff[2] = buff[3] = '-';
                        buff[digits] = SYM_MAH_NM_0;
                        buff[digits + 1] = SYM_MAH_NM_1;
                        buff[digits + 2] = '\0';
                    }
                    break;
                case OSD_UNIT_METRIC_MPH:
                    FALLTHROUGH;
                case OSD_UNIT_METRIC:
                    moreThanAh = osdFormatCentiNumber(buff, value * 100, 1000, 0, 2, digits, false);
                    if (!moreThanAh) {
                        tfp_sprintf(buff + strlen(buff), "%c%c", SYM_MAH_KM_0, SYM_MAH_KM_1);
                    } else {
                        tfp_sprintf(buff + strlen(buff), "%c", SYM_AH_KM);
                    }
                    if (!efficiencyValid) {
                        buff[0] = buff[1] = buff[2] = buff[3] = '-';
                        buff[digits] = SYM_MAH_KM_0;
                        buff[digits + 1] = SYM_MAH_KM_1;
                        buff[digits + 2] = '\0';
                    }
                    break;
            }
            break;
        }

    case OSD_EFFICIENCY_WH_PER_KM:
        {
            // amperage is in centi amps, speed is in cms/s. We want
            // mWh/km. Only show when ground speed > 1m/s.
            static pt1Filter_t eFilterState;
            static timeUs_t efficiencyUpdated = 0;
            int32_t value = 0;
            timeUs_t currentTimeUs = micros();
            timeDelta_t efficiencyTimeDelta = cmpTimeUs(currentTimeUs, efficiencyUpdated);
            if ((STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
                    || STATE(GPS_ESTIMATED_FIX)
#endif
                ) && gpsSol.groundSpeed > 0) {
                if (efficiencyTimeDelta >= EFFICIENCY_UPDATE_INTERVAL) {
                    value = pt1FilterApply4(&eFilterState, ((float)getPower() / gpsSol.groundSpeed) / 0.0036f,
                        1, US2S(efficiencyTimeDelta));

                    efficiencyUpdated = currentTimeUs;
                } else {
                    value = eFilterState.state;
                }
            }
            bool efficiencyValid = (value > 0) && (gpsSol.groundSpeed > 100);
            switch (osdConfig()->units) {
                case OSD_UNIT_UK:
                    FALLTHROUGH;
                case OSD_UNIT_IMPERIAL:
                    osdFormatCentiNumber(buff, value * METERS_PER_MILE / 10000, 0, 2, 0, 3, false);
                    buff[3] = SYM_WH_MI;
                    break;
                case OSD_UNIT_GA:
                    osdFormatCentiNumber(buff, value * METERS_PER_NAUTICALMILE / 10000, 0, 2, 0, 3, false);
                    buff[3] = SYM_WH_NM;
                    break;
                case OSD_UNIT_METRIC_MPH:
                    FALLTHROUGH;
                case OSD_UNIT_METRIC:
                    osdFormatCentiNumber(buff, value / 10, 0, 2, 0, 3, false);
                    buff[3] = SYM_WH_KM;
                    break;
            }
            buff[4] = '\0';
            if (!efficiencyValid) {
                buff[0] = buff[1] = buff[2] = '-';
            }
            break;
        }

    case OSD_GFORCE:
        {
            buff[0] = SYM_GFORCE;
            osdFormatCentiNumber(buff + 1, GForce, 0, 2, 0, 3, false);
            if (GForce > osdConfig()->gforce_alarm * 100) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            break;
        }

    case OSD_GFORCE_X:
    case OSD_GFORCE_Y:
    case OSD_GFORCE_Z:
        {
            float GForceValue = GForceAxis[item - OSD_GFORCE_X];
            buff[0] = SYM_GFORCE_X + item - OSD_GFORCE_X;
            osdFormatCentiNumber(buff + 1, GForceValue, 0, 2, 0, 4, false);
            if ((GForceValue < osdConfig()->gforce_axis_alarm_min * 100) || (GForceValue > osdConfig()->gforce_axis_alarm_max * 100)) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
            break;
        }
    case OSD_DEBUG:
        {
            /*
             * Longest representable string is -2147483648 does not fit in the screen.
             * Only 7 digits for negative and 8 digits for positive values allowed
             */
            for (uint8_t bufferIndex = 0; bufferIndex < DEBUG32_VALUE_COUNT; ++elemPosY, bufferIndex += 2) {
                tfp_sprintf(
                    buff,
                    "[%u]=%8ld [%u]=%8ld",
                    bufferIndex,
                    (long)constrain(debug[bufferIndex], -9999999, 99999999),
                    bufferIndex+1,
                    (long)constrain(debug[bufferIndex+1], -9999999, 99999999)
                );
                displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);
            }
            break;
        }

    case OSD_IMU_TEMPERATURE:
        {
            int16_t temperature;
            const bool valid = getIMUTemperature(&temperature);
            osdDisplayTemperature(elemPosX, elemPosY, SYM_IMU_TEMP, NULL, valid, temperature, osdConfig()->imu_temp_alarm_min, osdConfig()->imu_temp_alarm_max);
            return true;
        }

    case OSD_BARO_TEMPERATURE:
        {
            int16_t temperature;
            const bool valid = getBaroTemperature(&temperature);
            osdDisplayTemperature(elemPosX, elemPosY, SYM_BARO_TEMP, NULL, valid, temperature, osdConfig()->imu_temp_alarm_min, osdConfig()->imu_temp_alarm_max);
            return true;
        }

#ifdef USE_TEMPERATURE_SENSOR
    case OSD_TEMP_SENSOR_0_TEMPERATURE:
    case OSD_TEMP_SENSOR_1_TEMPERATURE:
    case OSD_TEMP_SENSOR_2_TEMPERATURE:
    case OSD_TEMP_SENSOR_3_TEMPERATURE:
    case OSD_TEMP_SENSOR_4_TEMPERATURE:
    case OSD_TEMP_SENSOR_5_TEMPERATURE:
    case OSD_TEMP_SENSOR_6_TEMPERATURE:
    case OSD_TEMP_SENSOR_7_TEMPERATURE:
        {
            osdDisplayTemperatureSensor(elemPosX, elemPosY, item - OSD_TEMP_SENSOR_0_TEMPERATURE);
            return true;
        }
#endif /* ifdef USE_TEMPERATURE_SENSOR */

    case OSD_WIND_SPEED_HORIZONTAL:
#ifdef USE_WIND_ESTIMATOR
        {
            bool valid = isEstimatedWindSpeedValid();
            float horizontalWindSpeed;
            uint16_t angle;
            horizontalWindSpeed = getEstimatedHorizontalWindSpeed(&angle);
            int16_t windDirection = osdGetHeadingAngle( CENTIDEGREES_TO_DEGREES((int)angle) - DECIDEGREES_TO_DEGREES(attitude.values.yaw) + 22);
            buff[0] = SYM_WIND_HORIZONTAL;
            buff[1] = SYM_DECORATION + (windDirection*2 / 90);
            osdFormatWindSpeedStr(buff + 2, horizontalWindSpeed, valid);
            break;
        }
#else
        return false;
#endif

    case OSD_WIND_SPEED_VERTICAL:
#ifdef USE_WIND_ESTIMATOR
        {
            buff[0] = SYM_WIND_VERTICAL;
            buff[1] = SYM_BLANK;
            bool valid = isEstimatedWindSpeedValid();
            float verticalWindSpeed;
            verticalWindSpeed = -getEstimatedWindSpeed(Z);  //from NED to NEU
            if (verticalWindSpeed < 0) {
                buff[1] = SYM_AH_DECORATION_DOWN;
                verticalWindSpeed = -verticalWindSpeed;
            } else {
                buff[1] = SYM_AH_DECORATION_UP;
            }
            osdFormatWindSpeedStr(buff + 2, verticalWindSpeed, valid);
            break;
        }
#else
        return false;
#endif

    case OSD_PLUS_CODE:
        {
            STATIC_ASSERT(GPS_DEGREES_DIVIDER == OLC_DEG_MULTIPLIER, invalid_olc_deg_multiplier);
            int digits = osdConfig()->plus_code_digits;
            int digitsRemoved = osdConfig()->plus_code_short * 2;
            if ((STATE(GPS_FIX)
#ifdef USE_GPS_FIX_ESTIMATION
                    || STATE(GPS_ESTIMATED_FIX)
#endif
            )) {
                olc_encode(gpsSol.llh.lat, gpsSol.llh.lon, digits, buff, sizeof(buff));
            } else {
                // +codes with > 8 digits have a + at the 9th digit
                // and we only support 10 and up.
                memset(buff, '-', digits + 1);
                buff[8] = '+';
                buff[digits + 1] = '\0';
            }
            // Optionally trim digits from the left
            memmove(buff, buff+digitsRemoved, strlen(buff) + digitsRemoved);
            buff[digits + 1 - digitsRemoved] = '\0';
            break;
        }

    case OSD_AZIMUTH:
        {

            buff[0] = SYM_AZIMUTH;
            if (osdIsHeadingValid()) {
                int16_t h = GPS_directionToHome;
                if (h < 0) {
                    h += 360;
                }
                if (h >= 180)
                    h = h - 180;
                else
                    h = h + 180;

                tfp_sprintf(&buff[1], "%3d", h);
            } else {
                buff[1] = buff[2] = buff[3] = '-';
            }
            buff[4] = SYM_DEGREES;
            buff[5] = '\0';
            break;
        }

    case OSD_MAP_SCALE:
        {
            float scaleToUnit;
            int scaleUnitDivisor;
            char symUnscaled;
            char symScaled;
            int maxDecimals;

            switch (osdConfig()->units) {
            case OSD_UNIT_UK:
                FALLTHROUGH;
            case OSD_UNIT_IMPERIAL:
                scaleToUnit = 100 / 1609.3440f; // scale to 0.01mi for osdFormatCentiNumber()
                scaleUnitDivisor = 0;
                symUnscaled = SYM_MI;
                symScaled = SYM_MI;
                maxDecimals = 2;
                break;
            case OSD_UNIT_GA:
                scaleToUnit = 100 / 1852.0010f; // scale to 0.01mi for osdFormatCentiNumber()
                scaleUnitDivisor = 0;
                symUnscaled = SYM_NM;
                symScaled = SYM_NM;
                maxDecimals = 2;
                break;
            default:
            case OSD_UNIT_METRIC_MPH:
                FALLTHROUGH;
            case OSD_UNIT_METRIC:
                scaleToUnit = 100; // scale to cm for osdFormatCentiNumber()
                scaleUnitDivisor = 1000; // Convert to km when scale gets bigger than 999m
                symUnscaled = SYM_M;
                symScaled = SYM_KM;
                maxDecimals = 0;
                break;
            }
            buff[0] = SYM_SCALE;
            if (osdMapData.scale > 0) {
                bool scaled = osdFormatCentiNumber(&buff[1], osdMapData.scale * scaleToUnit, scaleUnitDivisor, maxDecimals, 2, 3, false);
                buff[4] = scaled ? symScaled : symUnscaled;
                // Make sure this is cleared if the map stops being drawn
                osdMapData.scale = 0;
            } else {
                memset(&buff[1], '-', 4);
            }
            buff[5] = '\0';
            break;
        }
    case OSD_MAP_REFERENCE:
        {
            char referenceSymbol;
            if (osdMapData.referenceSymbol) {
                referenceSymbol = osdMapData.referenceSymbol;
                // Make sure this is cleared if the map stops being drawn
                osdMapData.referenceSymbol = 0;
            } else {
                referenceSymbol = '-';
            }
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY, SYM_DECORATION);
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY + 1, referenceSymbol);
            return true;
        }

    case OSD_GVAR_0:
    {
        osdFormatGVar(buff, 0);
        break;
    }
    case OSD_GVAR_1:
    {
        osdFormatGVar(buff, 1);
        break;
    }
    case OSD_GVAR_2:
    {
        osdFormatGVar(buff, 2);
        break;
    }
    case OSD_GVAR_3:
    {
        osdFormatGVar(buff, 3);
        break;
    }

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    case OSD_RC_SOURCE:
        {
            const char *source_text = IS_RC_MODE_ACTIVE(BOXMSPRCOVERRIDE) && !mspOverrideIsInFailsafe() ? "MSP" : "STD";
            if (IS_RC_MODE_ACTIVE(BOXMSPRCOVERRIDE) && mspOverrideIsInFailsafe()) TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, source_text, elemAttr);
            return true;
        }
#endif

#if defined(USE_ESC_SENSOR)
    case OSD_ESC_RPM:
        {
            escSensorData_t * escSensor = escSensorGetData();
            if (escSensor && escSensor->dataAge <= ESC_DATA_MAX_AGE) {
                osdFormatRpm(buff, escSensor->rpm);
            }
            else {
                osdFormatRpm(buff, 0);
            }
            break;
        }
    case OSD_ESC_TEMPERATURE:
        {
            escSensorData_t * escSensor = escSensorGetData();
            bool escTemperatureValid = escSensor && escSensor->dataAge <= ESC_DATA_MAX_AGE;
            osdDisplayTemperature(elemPosX, elemPosY, SYM_ESC_TEMP, NULL, escTemperatureValid, (escSensor->temperature)*10, osdConfig()->esc_temp_alarm_min, osdConfig()->esc_temp_alarm_max);
            return true;
        }
#endif
    case OSD_TPA:
        {
            char buff[4];
            textAttributes_t attr;

            displayWrite(osdDisplayPort, elemPosX, elemPosY, "TPA");
            attr = TEXT_ATTRIBUTES_NONE;
            tfp_sprintf(buff, "%3d", currentControlRateProfile->throttle.dynPID);
            if (isAdjustmentFunctionSelected(ADJUSTMENT_TPA)) {
                TEXT_ATTRIBUTES_ADD_BLINK(attr);
            }
            displayWriteWithAttr(osdDisplayPort, elemPosX + 5, elemPosY, buff, attr);

            displayWrite(osdDisplayPort, elemPosX, elemPosY + 1, "BP");
            attr = TEXT_ATTRIBUTES_NONE;
            tfp_sprintf(buff, "%4d", currentControlRateProfile->throttle.pa_breakpoint);
            if (isAdjustmentFunctionSelected(ADJUSTMENT_TPA_BREAKPOINT)) {
                TEXT_ATTRIBUTES_ADD_BLINK(attr);
            }
            displayWriteWithAttr(osdDisplayPort, elemPosX + 4, elemPosY + 1, buff, attr);

            return true;
        }
    case OSD_TPA_TIME_CONSTANT:
        {
            osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "TPA TC", 0, currentControlRateProfile->throttle.fixedWingTauMs, 4, 0, ADJUSTMENT_FW_TPA_TIME_CONSTANT);
            return true;
        }
    case OSD_FW_LEVEL_TRIM:
        {
            osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "LEVEL", 0, getFixedWingLevelTrim(), 3, 1, ADJUSTMENT_FW_LEVEL_TRIM);
            return true;
        }

    case OSD_NAV_FW_CONTROL_SMOOTHNESS:
        {
            osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "CTL S", 0, navConfig()->fw.control_smoothness, 1, 0, ADJUSTMENT_NAV_FW_CONTROL_SMOOTHNESS);
            return true;
        }
#ifdef USE_MULTI_MISSION
    case OSD_NAV_WP_MULTI_MISSION_INDEX:
        {
            osdDisplayAdjustableDecimalValue(elemPosX, elemPosY, "WP NO", 0, navConfig()->general.waypoint_multi_mission_index, 1, 0, ADJUSTMENT_NAV_WP_MULTI_MISSION_INDEX);
            return true;
        }
#endif
    case OSD_MISSION:
        {
            if (IS_RC_MODE_ACTIVE(BOXPLANWPMISSION)) {
                char buf[5];
                switch (posControl.wpMissionPlannerStatus) {
                case WP_PLAN_WAIT:
                    strcpy(buf, "WAIT");
                    break;
                case WP_PLAN_SAVE:
                    strcpy(buf, "SAVE");
                    break;
                case WP_PLAN_OK:
                    strcpy(buf, " OK ");
                    break;
                case WP_PLAN_FULL:
                    strcpy(buf, "FULL");
                }
                tfp_sprintf(buff, "%s>%2uWP", buf, posControl.wpPlannerActiveWPIndex);
            } else if (posControl.wpPlannerActiveWPIndex){
                tfp_sprintf(buff, "PLAN>%2uWP", posControl.waypointCount);  // mission planner mision active
            }
#ifdef USE_MULTI_MISSION
            else {
                if (ARMING_FLAG(ARMED) && !(IS_RC_MODE_ACTIVE(BOXCHANGEMISSION) && posControl.multiMissionCount > 1)){
                    // Limit field size when Armed, only show selected mission
                    tfp_sprintf(buff, "M%u       ", posControl.loadedMultiMissionIndex);
                } else if (posControl.multiMissionCount) {
                    if (navConfig()->general.waypoint_multi_mission_index != posControl.loadedMultiMissionIndex) {
                        tfp_sprintf(buff, "M%u/%u>LOAD", navConfig()->general.waypoint_multi_mission_index, posControl.multiMissionCount);
                    } else {
                        if (posControl.waypointListValid && posControl.waypointCount > 0) {
                            tfp_sprintf(buff, "M%u/%u>%2uWP", posControl.loadedMultiMissionIndex, posControl.multiMissionCount, posControl.waypointCount);
                        } else {
                            tfp_sprintf(buff, "M0/%u> 0WP", posControl.multiMissionCount);
                        }
                    }
                } else {    // no multi mission loaded - show active WP count from other source
                    tfp_sprintf(buff, "WP CNT>%2u", posControl.waypointCount);
                }
            }
#endif
            displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);
            return true;
        }

#ifdef USE_POWER_LIMITS
    case OSD_PLIMIT_REMAINING_BURST_TIME:
        osdFormatCentiNumber(buff, powerLimiterGetRemainingBurstTime() * 100, 0, 1, 0, 3, false);
        buff[3] = 'S';
        buff[4] = '\0';
        break;

    case OSD_PLIMIT_ACTIVE_CURRENT_LIMIT:
        if (currentBatteryProfile->powerLimits.continuousCurrent) {
            osdFormatCentiNumber(buff, powerLimiterGetActiveCurrentLimit(), 0, 2, 0, 3, false);
            buff[3] = SYM_AMP;
            buff[4] = '\0';

            if (powerLimiterIsLimitingCurrent()) {
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            }
        }
        break;

#ifdef USE_ADC
    case OSD_PLIMIT_ACTIVE_POWER_LIMIT:
        {
            if (currentBatteryProfile->powerLimits.continuousPower) {
                bool kiloWatt = osdFormatCentiNumber(buff, powerLimiterGetActivePowerLimit(), 1000, 2, 2, 3, false);
                buff[3] = kiloWatt ? SYM_KILOWATT : SYM_WATT;
                buff[4] = '\0';

                if (powerLimiterIsLimitingPower()) {
                    TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
                }
            }
            break;
        }
#endif // USE_ADC
#endif // USE_POWER_LIMITS
    case OSD_MULTI_FUNCTION:
        {
            // message shown infrequently so only write when needed
            static bool clearMultiFunction = true;
            elemAttr = osdGetMultiFunctionMessage(buff);
            if (buff[0] == 0) {
                if (clearMultiFunction) {
                    displayWrite(osdDisplayPort, elemPosX, elemPosY, "          ");
                    clearMultiFunction = false;
                }
                return true;
            }
            clearMultiFunction = true;
            break;
        }

    default:
        return false;
    }

    displayWriteWithAttr(osdDisplayPort, elemPosX, elemPosY, buff, elemAttr);
    return true;
}

uint8_t osdIncElementIndex(uint8_t elementIndex)
{
    ++elementIndex;

    if (elementIndex == OSD_ARTIFICIAL_HORIZON) {   // always drawn last so skip
        elementIndex++;
    }

#ifndef USE_TEMPERATURE_SENSOR
    if (elementIndex == OSD_TEMP_SENSOR_0_TEMPERATURE) {
        elementIndex = OSD_ALTITUDE_MSL;
    }
#endif

    if (!(feature(FEATURE_VBAT) && feature(FEATURE_CURRENT_METER))) {
        if (elementIndex == OSD_POWER) {
            elementIndex = OSD_GPS_LON;
        }
        if (elementIndex == OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE) {
            elementIndex = OSD_LEVEL_PIDS;
        }
#ifdef USE_POWER_LIMITS
        if (elementIndex == OSD_PLIMIT_REMAINING_BURST_TIME) {
            elementIndex = OSD_GLIDESLOPE;
        }
#endif
    }

#ifndef USE_POWER_LIMITS
    if (elementIndex == OSD_PLIMIT_REMAINING_BURST_TIME) {
        elementIndex = OSD_GLIDESLOPE;
    }
#endif

    if (!feature(FEATURE_CURRENT_METER)) {
        if (elementIndex == OSD_CURRENT_DRAW) {
            elementIndex = OSD_GPS_SPEED;
        }
        if (elementIndex == OSD_EFFICIENCY_MAH_PER_KM) {
            elementIndex = OSD_BATTERY_REMAINING_PERCENT;
        }
        if (elementIndex == OSD_EFFICIENCY_WH_PER_KM) {
            elementIndex = OSD_TRIP_DIST;
        }
        if (elementIndex == OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH) {
            elementIndex = OSD_HOME_HEADING_ERROR;
        }
        if (elementIndex == OSD_CLIMB_EFFICIENCY) {
            elementIndex = OSD_NAV_WP_MULTI_MISSION_INDEX;
        }
    }

    if (!STATE(ESC_SENSOR_ENABLED)) {
        if (elementIndex == OSD_ESC_RPM) {
            elementIndex = OSD_AZIMUTH;
        }
    }

    if (!feature(FEATURE_GPS)) {
        if (elementIndex == OSD_GPS_HDOP || elementIndex == OSD_TRIP_DIST || elementIndex == OSD_3D_SPEED || elementIndex == OSD_MISSION ||
            elementIndex == OSD_AZIMUTH || elementIndex == OSD_BATTERY_REMAINING_CAPACITY || elementIndex == OSD_EFFICIENCY_MAH_PER_KM) {
            elementIndex++;
        }
        if (elementIndex == OSD_HEADING_GRAPH && !sensors(SENSOR_MAG)) {
            elementIndex = feature(FEATURE_CURRENT_METER) ? OSD_WH_DRAWN : OSD_BATTERY_REMAINING_PERCENT;
        }
        if (elementIndex == OSD_EFFICIENCY_WH_PER_KM) {
            elementIndex = OSD_ATTITUDE_PITCH;
        }
        if (elementIndex == OSD_GPS_SPEED) {
            elementIndex = OSD_ALTITUDE;
        }
        if (elementIndex == OSD_GPS_LON) {
            elementIndex = sensors(SENSOR_MAG) ? OSD_HEADING : OSD_VARIO;
        }
        if (elementIndex == OSD_MAP_NORTH) {
            elementIndex = feature(FEATURE_CURRENT_METER) ? OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE : OSD_LEVEL_PIDS;
        }
        if (elementIndex == OSD_PLUS_CODE) {
            elementIndex = OSD_GFORCE;
        }
        if (elementIndex == OSD_GLIDESLOPE) {
            elementIndex = OSD_AIR_MAX_SPEED;
        }
        if (elementIndex == OSD_GLIDE_RANGE) {
            elementIndex = feature(FEATURE_CURRENT_METER) ? OSD_CLIMB_EFFICIENCY : OSD_PILOT_NAME;
        }
        if (elementIndex == OSD_NAV_WP_MULTI_MISSION_INDEX) {
            elementIndex = OSD_PILOT_NAME;
        }
    }

    if (!sensors(SENSOR_ACC)) {
        if (elementIndex == OSD_CROSSHAIRS) {
            elementIndex = OSD_ONTIME;
        }
        if (elementIndex == OSD_GFORCE) {
            elementIndex = OSD_RC_SOURCE;
        }
    }

    if (elementIndex == OSD_ITEM_COUNT) {
        elementIndex = 0;
    }
    return elementIndex;
}

void osdDrawNextElement(void)
{
    static uint8_t elementIndex = 0;
    // Flag for end of loop, also prevents infinite loop when no elements are enabled
    uint8_t index = elementIndex;
    do {
        elementIndex = osdIncElementIndex(elementIndex);
    } while (!osdDrawSingleElement(elementIndex) && index != elementIndex);

    // Draw artificial horizon + tracking telemetry last
    osdDrawSingleElement(OSD_ARTIFICIAL_HORIZON);
    if (osdConfig()->telemetry>0){
        osdDisplayTelemetry();
    }
}

PG_RESET_TEMPLATE(osdConfig_t, osdConfig,
    .rssi_alarm = SETTING_OSD_RSSI_ALARM_DEFAULT,
    .time_alarm = SETTING_OSD_TIME_ALARM_DEFAULT,
    .alt_alarm = SETTING_OSD_ALT_ALARM_DEFAULT,
    .dist_alarm = SETTING_OSD_DIST_ALARM_DEFAULT,
    .neg_alt_alarm = SETTING_OSD_NEG_ALT_ALARM_DEFAULT,
    .current_alarm = SETTING_OSD_CURRENT_ALARM_DEFAULT,
    .imu_temp_alarm_min = SETTING_OSD_IMU_TEMP_ALARM_MIN_DEFAULT,
    .imu_temp_alarm_max = SETTING_OSD_IMU_TEMP_ALARM_MAX_DEFAULT,
    .esc_temp_alarm_min = SETTING_OSD_ESC_TEMP_ALARM_MIN_DEFAULT,
    .esc_temp_alarm_max = SETTING_OSD_ESC_TEMP_ALARM_MAX_DEFAULT,
    .gforce_alarm = SETTING_OSD_GFORCE_ALARM_DEFAULT,
    .gforce_axis_alarm_min = SETTING_OSD_GFORCE_AXIS_ALARM_MIN_DEFAULT,
    .gforce_axis_alarm_max = SETTING_OSD_GFORCE_AXIS_ALARM_MAX_DEFAULT,
#ifdef USE_BARO
    .baro_temp_alarm_min = SETTING_OSD_BARO_TEMP_ALARM_MIN_DEFAULT,
    .baro_temp_alarm_max = SETTING_OSD_BARO_TEMP_ALARM_MAX_DEFAULT,
#endif
#ifdef USE_ADSB
    .adsb_distance_warning = SETTING_OSD_ADSB_DISTANCE_WARNING_DEFAULT,
    .adsb_distance_alert = SETTING_OSD_ADSB_DISTANCE_ALERT_DEFAULT,
    .adsb_ignore_plane_above_me_limit = SETTING_OSD_ADSB_IGNORE_PLANE_ABOVE_ME_LIMIT_DEFAULT,
#endif
#ifdef USE_SERIALRX_CRSF
    .snr_alarm = SETTING_OSD_SNR_ALARM_DEFAULT,
    .crsf_lq_format = SETTING_OSD_CRSF_LQ_FORMAT_DEFAULT,
    .link_quality_alarm = SETTING_OSD_LINK_QUALITY_ALARM_DEFAULT,
    .rssi_dbm_alarm = SETTING_OSD_RSSI_DBM_ALARM_DEFAULT,
    .rssi_dbm_max = SETTING_OSD_RSSI_DBM_MAX_DEFAULT,
    .rssi_dbm_min = SETTING_OSD_RSSI_DBM_MIN_DEFAULT,
#endif
#ifdef USE_TEMPERATURE_SENSOR
    .temp_label_align = SETTING_OSD_TEMP_LABEL_ALIGN_DEFAULT,
#endif
#ifdef USE_PITOT
    .airspeed_alarm_min = SETTING_OSD_AIRSPEED_ALARM_MIN_DEFAULT,
    .airspeed_alarm_max = SETTING_OSD_AIRSPEED_ALARM_MAX_DEFAULT,
#endif
#ifndef DISABLE_MSP_DJI_COMPAT
    .highlight_djis_missing_characters = SETTING_OSD_HIGHLIGHT_DJIS_MISSING_FONT_SYMBOLS_DEFAULT,
#endif

    .video_system = SETTING_OSD_VIDEO_SYSTEM_DEFAULT,
    .row_shiftdown = SETTING_OSD_ROW_SHIFTDOWN_DEFAULT,
    .msp_displayport_fullframe_interval = SETTING_OSD_MSP_DISPLAYPORT_FULLFRAME_INTERVAL_DEFAULT,

    .ahi_reverse_roll = SETTING_OSD_AHI_REVERSE_ROLL_DEFAULT,
    .ahi_max_pitch = SETTING_OSD_AHI_MAX_PITCH_DEFAULT,
    .crosshairs_style = SETTING_OSD_CROSSHAIRS_STYLE_DEFAULT,
    .horizon_offset = SETTING_OSD_HORIZON_OFFSET_DEFAULT,
    .camera_uptilt = SETTING_OSD_CAMERA_UPTILT_DEFAULT,
    .ahi_camera_uptilt_comp = SETTING_OSD_AHI_CAMERA_UPTILT_COMP_DEFAULT,
    .camera_fov_h = SETTING_OSD_CAMERA_FOV_H_DEFAULT,
    .camera_fov_v = SETTING_OSD_CAMERA_FOV_V_DEFAULT,
    .hud_margin_h = SETTING_OSD_HUD_MARGIN_H_DEFAULT,
    .hud_margin_v = SETTING_OSD_HUD_MARGIN_V_DEFAULT,
    .hud_homing = SETTING_OSD_HUD_HOMING_DEFAULT,
    .hud_homepoint = SETTING_OSD_HUD_HOMEPOINT_DEFAULT,
    .hud_radar_disp = SETTING_OSD_HUD_RADAR_DISP_DEFAULT,
    .hud_radar_range_min = SETTING_OSD_HUD_RADAR_RANGE_MIN_DEFAULT,
    .hud_radar_range_max = SETTING_OSD_HUD_RADAR_RANGE_MAX_DEFAULT,
    .hud_radar_alt_difference_display_time = SETTING_OSD_HUD_RADAR_ALT_DIFFERENCE_DISPLAY_TIME_DEFAULT,
    .hud_radar_distance_display_time = SETTING_OSD_HUD_RADAR_DISTANCE_DISPLAY_TIME_DEFAULT,
    .hud_wp_disp = SETTING_OSD_HUD_WP_DISP_DEFAULT,
    .left_sidebar_scroll = SETTING_OSD_LEFT_SIDEBAR_SCROLL_DEFAULT,
    .right_sidebar_scroll = SETTING_OSD_RIGHT_SIDEBAR_SCROLL_DEFAULT,
    .sidebar_scroll_arrows = SETTING_OSD_SIDEBAR_SCROLL_ARROWS_DEFAULT,
    .sidebar_horizontal_offset = SETTING_OSD_SIDEBAR_HORIZONTAL_OFFSET_DEFAULT,
    .left_sidebar_scroll_step = SETTING_OSD_LEFT_SIDEBAR_SCROLL_STEP_DEFAULT,
    .right_sidebar_scroll_step = SETTING_OSD_RIGHT_SIDEBAR_SCROLL_STEP_DEFAULT,
    .sidebar_height = SETTING_OSD_SIDEBAR_HEIGHT_DEFAULT,
    .ahi_pitch_interval = SETTING_OSD_AHI_PITCH_INTERVAL_DEFAULT,
    .osd_home_position_arm_screen = SETTING_OSD_HOME_POSITION_ARM_SCREEN_DEFAULT,
    .pan_servo_index = SETTING_OSD_PAN_SERVO_INDEX_DEFAULT,
    .pan_servo_pwm2centideg = SETTING_OSD_PAN_SERVO_PWM2CENTIDEG_DEFAULT,
    .pan_servo_offcentre_warning = SETTING_OSD_PAN_SERVO_OFFCENTRE_WARNING_DEFAULT,
    .pan_servo_indicator_show_degrees = SETTING_OSD_PAN_SERVO_INDICATOR_SHOW_DEGREES_DEFAULT,
    .esc_rpm_precision = SETTING_OSD_ESC_RPM_PRECISION_DEFAULT,
    .mAh_precision = SETTING_OSD_MAH_PRECISION_DEFAULT,
    .osd_switch_indicator0_name = SETTING_OSD_SWITCH_INDICATOR_ZERO_NAME_DEFAULT,
    .osd_switch_indicator0_channel = SETTING_OSD_SWITCH_INDICATOR_ZERO_CHANNEL_DEFAULT,
    .osd_switch_indicator1_name = SETTING_OSD_SWITCH_INDICATOR_ONE_NAME_DEFAULT,
    .osd_switch_indicator1_channel = SETTING_OSD_SWITCH_INDICATOR_ONE_CHANNEL_DEFAULT,
    .osd_switch_indicator2_name = SETTING_OSD_SWITCH_INDICATOR_TWO_NAME_DEFAULT,
    .osd_switch_indicator2_channel = SETTING_OSD_SWITCH_INDICATOR_TWO_CHANNEL_DEFAULT,
    .osd_switch_indicator3_name = SETTING_OSD_SWITCH_INDICATOR_THREE_NAME_DEFAULT,
    .osd_switch_indicator3_channel = SETTING_OSD_SWITCH_INDICATOR_THREE_CHANNEL_DEFAULT,
    .osd_switch_indicators_align_left = SETTING_OSD_SWITCH_INDICATORS_ALIGN_LEFT_DEFAULT,
    .system_msg_display_time = SETTING_OSD_SYSTEM_MSG_DISPLAY_TIME_DEFAULT,
    .units = SETTING_OSD_UNITS_DEFAULT,
    .main_voltage_decimals = SETTING_OSD_MAIN_VOLTAGE_DECIMALS_DEFAULT,
    .decimals_altitude = SETTING_OSD_DECIMALS_ALTITUDE_DEFAULT,
    .decimals_distance = SETTING_OSD_DECIMALS_DISTANCE_DEFAULT,
    .use_pilot_logo = SETTING_OSD_USE_PILOT_LOGO_DEFAULT,
    .inav_to_pilot_logo_spacing = SETTING_OSD_INAV_TO_PILOT_LOGO_SPACING_DEFAULT,
    .arm_screen_display_time = SETTING_OSD_ARM_SCREEN_DISPLAY_TIME_DEFAULT,

#ifdef USE_WIND_ESTIMATOR
    .estimations_wind_compensation = SETTING_OSD_ESTIMATIONS_WIND_COMPENSATION_DEFAULT,
#endif

    .coordinate_digits = SETTING_OSD_COORDINATE_DIGITS_DEFAULT,

    .osd_failsafe_switch_layout = SETTING_OSD_FAILSAFE_SWITCH_LAYOUT_DEFAULT,

    .plus_code_digits = SETTING_OSD_PLUS_CODE_DIGITS_DEFAULT,
    .plus_code_short = SETTING_OSD_PLUS_CODE_SHORT_DEFAULT,

    .ahi_width = SETTING_OSD_AHI_WIDTH_DEFAULT,
    .ahi_height = SETTING_OSD_AHI_HEIGHT_DEFAULT,
    .ahi_vertical_offset = SETTING_OSD_AHI_VERTICAL_OFFSET_DEFAULT,
    .ahi_bordered = SETTING_OSD_AHI_BORDERED_DEFAULT,
    .ahi_style = SETTING_OSD_AHI_STYLE_DEFAULT,

    .force_grid = SETTING_OSD_FORCE_GRID_DEFAULT,

    .stats_energy_unit = SETTING_OSD_STATS_ENERGY_UNIT_DEFAULT,
    .stats_page_auto_swap_time = SETTING_OSD_STATS_PAGE_AUTO_SWAP_TIME_DEFAULT,
    .stats_show_metric_efficiency = SETTING_OSD_STATS_SHOW_METRIC_EFFICIENCY_DEFAULT,

    .radar_peers_display_time = SETTING_OSD_RADAR_PEERS_DISPLAY_TIME_DEFAULT
);

void pgResetFn_osdLayoutsConfig(osdLayoutsConfig_t *osdLayoutsConfig)
{
    osdLayoutsConfig->item_pos[0][OSD_ALTITUDE] = OSD_POS(1, 0) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_MAIN_BATT_VOLTAGE] = OSD_POS(12, 0) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE] = OSD_POS(12, 1);

    osdLayoutsConfig->item_pos[0][OSD_RSSI_VALUE] = OSD_POS(23, 0) | OSD_VISIBLE_FLAG;
    //line 2
    osdLayoutsConfig->item_pos[0][OSD_HOME_DIST] = OSD_POS(1, 1);
    osdLayoutsConfig->item_pos[0][OSD_TRIP_DIST] = OSD_POS(1, 2);
    osdLayoutsConfig->item_pos[0][OSD_ODOMETER] = OSD_POS(1, 3);
    osdLayoutsConfig->item_pos[0][OSD_MAIN_BATT_CELL_VOLTAGE] = OSD_POS(12, 1);
    osdLayoutsConfig->item_pos[0][OSD_MAIN_BATT_SAG_COMPENSATED_CELL_VOLTAGE] = OSD_POS(12, 1);
    osdLayoutsConfig->item_pos[0][OSD_GPS_SPEED] = OSD_POS(23, 1);
    osdLayoutsConfig->item_pos[0][OSD_3D_SPEED] = OSD_POS(23, 1);
    osdLayoutsConfig->item_pos[0][OSD_GLIDESLOPE] = OSD_POS(23, 2);

    osdLayoutsConfig->item_pos[0][OSD_THROTTLE_POS] = OSD_POS(1, 2) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_SCALED_THROTTLE_POS] = OSD_POS(6, 2);
    osdLayoutsConfig->item_pos[0][OSD_HEADING] = OSD_POS(12, 2);
    osdLayoutsConfig->item_pos[0][OSD_GROUND_COURSE] = OSD_POS(12, 3);
    osdLayoutsConfig->item_pos[0][OSD_COURSE_HOLD_ERROR] = OSD_POS(12, 2);
    osdLayoutsConfig->item_pos[0][OSD_COURSE_HOLD_ADJUSTMENT] = OSD_POS(12, 2);
    osdLayoutsConfig->item_pos[0][OSD_CROSS_TRACK_ERROR] = OSD_POS(12, 3);
    osdLayoutsConfig->item_pos[0][OSD_HEADING_GRAPH] = OSD_POS(18, 2);
    osdLayoutsConfig->item_pos[0][OSD_CURRENT_DRAW] = OSD_POS(2, 3) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_MAH_DRAWN] = OSD_POS(1, 4) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_WH_DRAWN] = OSD_POS(1, 5);
    osdLayoutsConfig->item_pos[0][OSD_BATTERY_REMAINING_CAPACITY] = OSD_POS(1, 6);
    osdLayoutsConfig->item_pos[0][OSD_BATTERY_REMAINING_PERCENT] = OSD_POS(1, 7);
    osdLayoutsConfig->item_pos[0][OSD_POWER_SUPPLY_IMPEDANCE] = OSD_POS(1, 8);

    osdLayoutsConfig->item_pos[0][OSD_EFFICIENCY_MAH_PER_KM] = OSD_POS(1, 5);
    osdLayoutsConfig->item_pos[0][OSD_EFFICIENCY_WH_PER_KM] = OSD_POS(1, 5);

    osdLayoutsConfig->item_pos[0][OSD_ATTITUDE_ROLL] = OSD_POS(1, 7);
    osdLayoutsConfig->item_pos[0][OSD_ATTITUDE_PITCH] = OSD_POS(1, 8);

    // avoid OSD_VARIO under OSD_CROSSHAIRS
    osdLayoutsConfig->item_pos[0][OSD_VARIO] = OSD_POS(23, 5);
    // OSD_VARIO_NUM at the right of OSD_VARIO
    osdLayoutsConfig->item_pos[0][OSD_VARIO_NUM] = OSD_POS(24, 7);
    osdLayoutsConfig->item_pos[0][OSD_HOME_DIR] = OSD_POS(14, 11);
    osdLayoutsConfig->item_pos[0][OSD_ARTIFICIAL_HORIZON] = OSD_POS(8, 6);
    osdLayoutsConfig->item_pos[0][OSD_HORIZON_SIDEBARS] = OSD_POS(8, 6);

    osdLayoutsConfig->item_pos[0][OSD_CRAFT_NAME] = OSD_POS(20, 2);
    osdLayoutsConfig->item_pos[0][OSD_PILOT_NAME] = OSD_POS(20, 3);
    osdLayoutsConfig->item_pos[0][OSD_PILOT_LOGO] = OSD_POS(20, 3);
    osdLayoutsConfig->item_pos[0][OSD_VTX_CHANNEL] = OSD_POS(8, 6);

#ifdef USE_SERIALRX_CRSF
    osdLayoutsConfig->item_pos[0][OSD_CRSF_RSSI_DBM] = OSD_POS(23, 12);
    osdLayoutsConfig->item_pos[0][OSD_CRSF_LQ] = OSD_POS(23, 11);
    osdLayoutsConfig->item_pos[0][OSD_CRSF_SNR_DB] = OSD_POS(24, 9);
    osdLayoutsConfig->item_pos[0][OSD_CRSF_TX_POWER] = OSD_POS(24, 10);
#endif

    osdLayoutsConfig->item_pos[0][OSD_ONTIME] = OSD_POS(23, 8);
    osdLayoutsConfig->item_pos[0][OSD_FLYTIME] = OSD_POS(23, 9);
    osdLayoutsConfig->item_pos[0][OSD_ONTIME_FLYTIME] = OSD_POS(23, 11) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_RTC_TIME] = OSD_POS(23, 12);
    osdLayoutsConfig->item_pos[0][OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH] = OSD_POS(23, 7);
    osdLayoutsConfig->item_pos[0][OSD_REMAINING_DISTANCE_BEFORE_RTH] = OSD_POS(23, 6);

    osdLayoutsConfig->item_pos[0][OSD_MISSION] = OSD_POS(0, 10);
    osdLayoutsConfig->item_pos[0][OSD_GPS_SATS] = OSD_POS(0, 11) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_GPS_HDOP] = OSD_POS(0, 10);

    osdLayoutsConfig->item_pos[0][OSD_GPS_LAT] = OSD_POS(0, 12);
    // Put this on top of the latitude, since it's very unlikely
    // that users will want to use both at the same time.
    osdLayoutsConfig->item_pos[0][OSD_PLUS_CODE] = OSD_POS(0, 12);
    osdLayoutsConfig->item_pos[0][OSD_FLYMODE] = OSD_POS(13, 12) | OSD_VISIBLE_FLAG;
    osdLayoutsConfig->item_pos[0][OSD_GPS_LON] = OSD_POS(18, 12);

    osdLayoutsConfig->item_pos[0][OSD_AZIMUTH] = OSD_POS(2, 12);

    osdLayoutsConfig->item_pos[0][OSD_ROLL_PIDS] = OSD_POS(2, 10);
    osdLayoutsConfig->item_pos[0][OSD_PITCH_PIDS] = OSD_POS(2, 11);
    osdLayoutsConfig->item_pos[0][OSD_YAW_PIDS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_LEVEL_PIDS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_POS_XY_PIDS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_POS_Z_PIDS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_VEL_XY_PIDS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_VEL_Z_PIDS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_HEADING_P] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_BOARD_ALIGN_ROLL] = OSD_POS(2, 10);
    osdLayoutsConfig->item_pos[0][OSD_BOARD_ALIGN_PITCH] = OSD_POS(2, 11);
    osdLayoutsConfig->item_pos[0][OSD_RC_EXPO] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_RC_YAW_EXPO] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_THROTTLE_EXPO] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_PITCH_RATE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_ROLL_RATE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_YAW_RATE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MANUAL_RC_EXPO] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MANUAL_RC_YAW_EXPO] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MANUAL_PITCH_RATE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MANUAL_ROLL_RATE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MANUAL_YAW_RATE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_NAV_FW_CRUISE_THR] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_NAV_FW_PITCH2THR] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_FW_ALT_PID_OUTPUTS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_FW_POS_PID_OUTPUTS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MC_VEL_X_PID_OUTPUTS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MC_VEL_Y_PID_OUTPUTS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MC_VEL_Z_PID_OUTPUTS] = OSD_POS(2, 12);
    osdLayoutsConfig->item_pos[0][OSD_MC_POS_XYZ_P_OUTPUTS] = OSD_POS(2, 12);

    osdLayoutsConfig->item_pos[0][OSD_POWER] = OSD_POS(15, 1);

    osdLayoutsConfig->item_pos[0][OSD_IMU_TEMPERATURE] = OSD_POS(19, 2);
    osdLayoutsConfig->item_pos[0][OSD_BARO_TEMPERATURE] = OSD_POS(19, 3);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_0_TEMPERATURE] = OSD_POS(19, 4);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_1_TEMPERATURE] = OSD_POS(19, 5);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_2_TEMPERATURE] = OSD_POS(19, 6);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_3_TEMPERATURE] = OSD_POS(19, 7);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_4_TEMPERATURE] = OSD_POS(19, 8);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_5_TEMPERATURE] = OSD_POS(19, 9);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_6_TEMPERATURE] = OSD_POS(19, 10);
    osdLayoutsConfig->item_pos[0][OSD_TEMP_SENSOR_7_TEMPERATURE] = OSD_POS(19, 11);

    osdLayoutsConfig->item_pos[0][OSD_AIR_SPEED] = OSD_POS(3, 5);
    osdLayoutsConfig->item_pos[0][OSD_WIND_SPEED_HORIZONTAL] = OSD_POS(3, 6);
    osdLayoutsConfig->item_pos[0][OSD_WIND_SPEED_VERTICAL] = OSD_POS(3, 7);

    osdLayoutsConfig->item_pos[0][OSD_GFORCE] = OSD_POS(12, 4);
    osdLayoutsConfig->item_pos[0][OSD_GFORCE_X] = OSD_POS(12, 5);
    osdLayoutsConfig->item_pos[0][OSD_GFORCE_Y] = OSD_POS(12, 6);
    osdLayoutsConfig->item_pos[0][OSD_GFORCE_Z] = OSD_POS(12, 7);

    osdLayoutsConfig->item_pos[0][OSD_VTX_POWER] = OSD_POS(3, 5);

    osdLayoutsConfig->item_pos[0][OSD_GVAR_0] = OSD_POS(1, 1);
    osdLayoutsConfig->item_pos[0][OSD_GVAR_1] = OSD_POS(1, 2);
    osdLayoutsConfig->item_pos[0][OSD_GVAR_2] = OSD_POS(1, 3);
    osdLayoutsConfig->item_pos[0][OSD_GVAR_3] = OSD_POS(1, 4);

    osdLayoutsConfig->item_pos[0][OSD_MULTI_FUNCTION] = OSD_POS(1, 4);

    osdLayoutsConfig->item_pos[0][OSD_SWITCH_INDICATOR_0] = OSD_POS(2, 7);
    osdLayoutsConfig->item_pos[0][OSD_SWITCH_INDICATOR_1] = OSD_POS(2, 8);
    osdLayoutsConfig->item_pos[0][OSD_SWITCH_INDICATOR_2] = OSD_POS(2, 9);
    osdLayoutsConfig->item_pos[0][OSD_SWITCH_INDICATOR_3] = OSD_POS(2, 10);

    osdLayoutsConfig->item_pos[0][OSD_ADSB_WARNING] = OSD_POS(2, 7);
    osdLayoutsConfig->item_pos[0][OSD_ADSB_INFO] = OSD_POS(2, 8);
#if defined(USE_ESC_SENSOR)
    osdLayoutsConfig->item_pos[0][OSD_ESC_RPM] = OSD_POS(1, 2);
    osdLayoutsConfig->item_pos[0][OSD_ESC_TEMPERATURE] = OSD_POS(1, 3);
#endif

#if defined(USE_RX_MSP) && defined(USE_MSP_RC_OVERRIDE)
    osdLayoutsConfig->item_pos[0][OSD_RC_SOURCE] = OSD_POS(3, 4);
#endif

#ifdef USE_POWER_LIMITS
    osdLayoutsConfig->item_pos[0][OSD_PLIMIT_REMAINING_BURST_TIME] = OSD_POS(3, 4);
    osdLayoutsConfig->item_pos[0][OSD_PLIMIT_ACTIVE_CURRENT_LIMIT] = OSD_POS(3, 5);
    osdLayoutsConfig->item_pos[0][OSD_PLIMIT_ACTIVE_POWER_LIMIT] = OSD_POS(3, 6);
#endif

#ifdef USE_BLACKBOX
    osdLayoutsConfig->item_pos[0][OSD_BLACKBOX] = OSD_POS(2, 10);
#endif

    // Under OSD_FLYMODE. TODO: Might not be visible on NTSC?
    osdLayoutsConfig->item_pos[0][OSD_MESSAGES] = OSD_POS(1, 13) | OSD_VISIBLE_FLAG;

    for (unsigned ii = 1; ii < OSD_LAYOUT_COUNT; ii++) {
        for (unsigned jj = 0; jj < ARRAYLEN(osdLayoutsConfig->item_pos[0]); jj++) {
            osdLayoutsConfig->item_pos[ii][jj] = osdLayoutsConfig->item_pos[0][jj] & ~OSD_VISIBLE_FLAG;
        }
    }
}

/**
 * @brief Draws the INAV and/or pilot logos on the display
 *
 * @param singular If true, only one logo will be drawn. If false, both logos will be drawn, separated by osdConfig()->inav_to_pilot_logo_spacing characters
 * @param row The row number to start drawing the logos. If not singular, both logos are drawn on the same rows.
 * @return uint8_t The row number after the logo(s).
 */
uint8_t drawLogos(bool singular, uint8_t row) {
    uint8_t logoRow = row;
    uint8_t logoColOffset = 0;
    bool usePilotLogo = (osdConfig()->use_pilot_logo && osdDisplayIsHD());
    bool useINAVLogo = (singular && !usePilotLogo) || !singular;

#ifndef DISABLE_MSP_DJI_COMPAT   // IF DJICOMPAT is in use, the pilot logo cannot be used, due to font issues.
    if (isDJICompatibleVideoSystem(osdConfig())) {
        usePilotLogo = false;
        useINAVLogo = false;
    }
#endif

    uint8_t logoSpacing = osdConfig()->inav_to_pilot_logo_spacing;

    if (logoSpacing > 0 && ((osdDisplayPort->cols % 2) != (logoSpacing % 2))) {
        logoSpacing++; // Add extra 1 character space between logos, if the odd/even of the OSD cols doesn't match the odd/even of the logo spacing
}

    // Draw Logo(s)
    if (usePilotLogo && !singular) {
        logoColOffset = ((osdDisplayPort->cols - (SYM_LOGO_WIDTH * 2)) - logoSpacing) / 2;
    } else {
        logoColOffset = floorf((osdDisplayPort->cols - SYM_LOGO_WIDTH) / 2.0f);
    }

    // Draw INAV logo
    if (useINAVLogo) {
        unsigned logo_c = SYM_LOGO_START;
        uint8_t logo_x = logoColOffset;
        for (uint8_t lRow = 0; lRow < SYM_LOGO_HEIGHT; lRow++) {
            for (uint8_t lCol = 0; lCol < SYM_LOGO_WIDTH; lCol++) {
                displayWriteChar(osdDisplayPort, logo_x + lCol, logoRow, logo_c++);
            }
            logoRow++;
        }
    }

    // Draw the pilot logo
    if (usePilotLogo) {
        unsigned logo_c = SYM_PILOT_LOGO_LRG_START;
        uint8_t logo_x = 0;
        logoRow = row;
        if (singular) {
            logo_x = logoColOffset;
        } else {
                logo_x = logoColOffset + SYM_LOGO_WIDTH + logoSpacing;
        }

        for (uint8_t lRow = 0; lRow < SYM_LOGO_HEIGHT; lRow++) {
            for (uint8_t lCol = 0; lCol < SYM_LOGO_WIDTH; lCol++) {
                displayWriteChar(osdDisplayPort, logo_x + lCol, logoRow, logo_c++);
            }
            logoRow++;
        }
    }

    if (!usePilotLogo && !useINAVLogo) {
        logoRow += SYM_LOGO_HEIGHT;
    }

    return logoRow;
}

#ifdef USE_STATS
uint8_t drawStat_Stats(uint8_t statNameX, uint8_t row, uint8_t statValueX, bool isBootStats)
{
    uint8_t buffLen = 0;
    char string_buffer[osdDisplayPort->cols - statValueX];

    if (statsConfig()->stats_enabled) {
        if (isBootStats)
            displayWrite(osdDisplayPort, statNameX, row, "ODOMETER:");
        else
            displayWrite(osdDisplayPort, statNameX, row, "ODOMETER");

        switch (osdConfig()->units) {
            case OSD_UNIT_UK:
                FALLTHROUGH;
            case OSD_UNIT_IMPERIAL:
                if (isBootStats) {
                    tfp_sprintf(string_buffer, "%5d", (uint16_t)(statsConfig()->stats_total_dist / METERS_PER_MILE));
                    buffLen = 5;
                } else {
                    uint16_t statTotalDist = (uint16_t)(statsConfig()->stats_total_dist / METERS_PER_MILE);
                    tfp_sprintf(string_buffer, ": %d", statTotalDist);
                    buffLen = 3 + sizeof(statTotalDist);
                }

                string_buffer[buffLen++] = SYM_MI;
                break;
            default:
            case OSD_UNIT_GA:
                if (isBootStats) {
                    tfp_sprintf(string_buffer, "%5d", (uint16_t)(statsConfig()->stats_total_dist / METERS_PER_NAUTICALMILE));
                    buffLen = 5;
                } else {
                    uint16_t statTotalDist = (uint16_t)(statsConfig()->stats_total_dist / METERS_PER_NAUTICALMILE);
                    tfp_sprintf(string_buffer, ": %d", statTotalDist);
                    buffLen = 3 + sizeof(statTotalDist);
                }

                string_buffer[buffLen++] = SYM_NM;
                break;
            case OSD_UNIT_METRIC_MPH:
                FALLTHROUGH;
            case OSD_UNIT_METRIC:
                if (isBootStats) {
                    tfp_sprintf(string_buffer, "%5d", (uint16_t)(statsConfig()->stats_total_dist / METERS_PER_KILOMETER));
                    buffLen = 5;
                } else {
                    uint16_t statTotalDist = (uint16_t)(statsConfig()->stats_total_dist / METERS_PER_KILOMETER);
                    tfp_sprintf(string_buffer, ": %d", statTotalDist);
                    buffLen = 3 + sizeof(statTotalDist);
                }

                string_buffer[buffLen++] = SYM_KM;
                break;
        }
        string_buffer[buffLen] = '\0';
        displayWrite(osdDisplayPort, statValueX-(isBootStats ? 5 : 0), row,  string_buffer);

        if (isBootStats)
            displayWrite(osdDisplayPort, statNameX, ++row, "TOTAL TIME:");
        else
            displayWrite(osdDisplayPort, statNameX, ++row, "TOTAL TIME");

        uint32_t tot_mins = statsConfig()->stats_total_time / 60;
        if (isBootStats)
            tfp_sprintf(string_buffer, "%d:%02dH:M%c", (int)(tot_mins / 60), (int)(tot_mins % 60), '\0');
        else
            tfp_sprintf(string_buffer, ": %d:%02d H:M%c", (int)(tot_mins / 60), (int)(tot_mins % 60), '\0');

        displayWrite(osdDisplayPort, statValueX-(isBootStats ? 7 : 0), row,  string_buffer);

#ifdef USE_ADC
        if (feature(FEATURE_VBAT) && feature(FEATURE_CURRENT_METER) && statsConfig()->stats_total_energy) {
            uint8_t buffOffset = 0;
            if (isBootStats)
                displayWrite(osdDisplayPort, statNameX, ++row, "TOTAL ENERGY:");
            else {
                displayWrite(osdDisplayPort, statNameX, ++row, "TOTAL ENERGY");
                string_buffer[0] = ':';
                buffOffset = 2;
            }

            osdFormatCentiNumber(string_buffer + buffOffset, statsConfig()->stats_total_energy / 10, 0, 2, 0, 6, true);
            displayWrite(osdDisplayPort, statValueX - (isBootStats ? 6 : 0), row,  string_buffer);
            displayWriteChar(osdDisplayPort, statValueX + (isBootStats ? 0 : 8), row, SYM_WH);

            char avgEffBuff[osdDisplayPort->cols - statValueX];

            for (uint8_t i = 0; i < osdDisplayPort->cols - statValueX; i++) {
                avgEffBuff[i] = '\0';
                string_buffer[i] = '\0';
            }

            if (statsConfig()->stats_total_dist) {
                if (isBootStats)
                    displayWrite(osdDisplayPort, statNameX, ++row, "AVG EFFICIENCY:");
                else {
                    displayWrite(osdDisplayPort, statNameX, ++row, "AVG EFFICIENCY");
                    strcat(avgEffBuff, ": ");
                }

                float_t avg_efficiency = MWH_TO_WH(statsConfig()->stats_total_energy) / METERS_TO_KILOMETERS(statsConfig()->stats_total_dist); // Wh/km
                switch (osdConfig()->units) {
                    case OSD_UNIT_UK:
                        FALLTHROUGH;
                    case OSD_UNIT_IMPERIAL:
                        osdFormatCentiNumber(string_buffer, (int32_t)(avg_efficiency * METERS_PER_MILE / 10), 0, 2, 2, 4, false);
                        string_buffer[4] = SYM_WH_MI;
                        break;
                    case OSD_UNIT_GA:
                        osdFormatCentiNumber(string_buffer, (int32_t)(avg_efficiency * METERS_PER_NAUTICALMILE / 10), 0, 2, 2, 4, false);
                        string_buffer[4] = SYM_WH_NM;
                        break;
                    default:
                    case OSD_UNIT_METRIC_MPH:
                        FALLTHROUGH;
                    case OSD_UNIT_METRIC:
                        osdFormatCentiNumber(string_buffer, (int32_t)(avg_efficiency * 100), 0, 2, 2, 4, false);
                        string_buffer[4] = SYM_WH_KM;
                        break;
                }

                if (isBootStats)
                    strcat(avgEffBuff, string_buffer);
                else
                    strcat(avgEffBuff, osdFormatTrimWhiteSpace(string_buffer));
            } else {
                strcat(avgEffBuff, "----");
            }

            displayWrite(osdDisplayPort, statValueX-(isBootStats ? 4 : 0), row++, avgEffBuff);
        }
#endif // USE_ADC
    }
    return row;
}

uint8_t drawStats(uint8_t row)
{
    uint8_t statNameX = (osdDisplayPort->cols - 22) / 2;
    uint8_t statValueX = statNameX + 21;

    return drawStat_Stats(statNameX, row, statValueX, true);
}
#endif // USE STATS

static void osdSetNextRefreshIn(uint32_t timeMs)
{
    resumeRefreshAt = micros() + timeMs * 1000;
    refreshWaitForResumeCmdRelease = true;
}

static void osdCompleteAsyncInitialization(void)
{
    if (!displayIsReady(osdDisplayPort)) {
        // Update the display.
        // XXX: Rename displayDrawScreen() and associated functions
        // to displayUpdate()
        displayDrawScreen(osdDisplayPort);
        return;
    }

    osdDisplayIsReady = true;

#if defined(USE_CANVAS)
    if (osdConfig()->force_grid) {
        osdDisplayHasCanvas = false;
    } else {
        osdDisplayHasCanvas = displayGetCanvas(&osdCanvas, osdDisplayPort);
    }
#endif

    displayBeginTransaction(osdDisplayPort, DISPLAY_TRANSACTION_OPT_RESET_DRAWING);
    displayClearScreen(osdDisplayPort);

    uint8_t y = 1;
    displayFontMetadata_t metadata;
    bool fontHasMetadata = displayGetFontMetadata(&metadata, osdDisplayPort);
    LOG_DEBUG(OSD, "Font metadata version %s: %u (%u chars)",
        fontHasMetadata ? "Y" : "N", metadata.version, metadata.charCount);

    if (fontHasMetadata && metadata.charCount > 256) {
        hasExtendedFont = true;

        y = drawLogos(false, y);
        y++;
    } else if (!fontHasMetadata) {
        const char *m = "INVALID FONT";
        displayWrite(osdDisplayPort, OSD_CENTER_S(m), y++, m);
    }

    if (fontHasMetadata && metadata.version < OSD_MIN_FONT_VERSION) {
        const char *m = "INVALID FONT VERSION";
        displayWrite(osdDisplayPort, OSD_CENTER_S(m), y++, m);
    }

    char string_buffer[30];
    tfp_sprintf(string_buffer, "INAV VERSION: %s", FC_VERSION_STRING);
    uint8_t xPos = (osdDisplayPort->cols - 19) / 2; // Automatically centre, regardless of resolution. In the case of odd number screens, bias to the left.
    displayWrite(osdDisplayPort, xPos, y++, string_buffer);
#ifdef USE_CMS
    displayWrite(osdDisplayPort, xPos+2, y++, CMS_STARTUP_HELP_TEXT1);
    displayWrite(osdDisplayPort, xPos+6, y++, CMS_STARTUP_HELP_TEXT2);
    displayWrite(osdDisplayPort, xPos+6, y++, CMS_STARTUP_HELP_TEXT3);
#endif

#ifdef USE_STATS
    y = drawStats(++y);
#endif

    displayCommitTransaction(osdDisplayPort);
    displayResync(osdDisplayPort);
    osdSetNextRefreshIn(SPLASH_SCREEN_DISPLAY_TIME);
}

void osdInit(displayPort_t *osdDisplayPortToUse)
{
    if (!osdDisplayPortToUse)
        return;

    BUILD_BUG_ON(OSD_POS_MAX != OSD_POS(63,63));

    osdDisplayPort = osdDisplayPortToUse;

#ifdef USE_CMS
    cmsDisplayPortRegister(osdDisplayPort);
#endif

    armState = ARMING_FLAG(ARMED);
    osdCompleteAsyncInitialization();
}

static void osdResetStats(void)
{
    // Reset internal OSD stats
    stats.max_distance = 0;
    stats.max_current = 0;
    stats.max_power = 0;
    stats.max_speed = 0;
    stats.max_3D_speed = 0;
    stats.max_air_speed = 0;
    stats.min_voltage = 12000;
    stats.min_rssi = 100;
    stats.min_lq = 300;
    stats.min_rssi_dbm = 0;
    stats.max_altitude = 0;
    stats.min_sats = 255;
    stats.max_sats = 0;
    stats.min_esc_temp = 300;
    stats.max_esc_temp = 0;
    stats.flightStartMAh = getMAhDrawn();
    stats.flightStartMWh = getMWhDrawn();

    // Reset external stats
    posControl.totalTripDistance = 0.0f;
    resetFlightTime();
    resetGForceStats();
}

static void osdUpdateStats(void)
{
    int32_t value;

    if (feature(FEATURE_GPS)) {
        value = osdGet3DSpeed();
        const float airspeed_estimate = getAirspeedEstimate();

        if (stats.max_3D_speed < value)
            stats.max_3D_speed = value;

        if (stats.max_speed < gpsSol.groundSpeed)
            stats.max_speed = gpsSol.groundSpeed;

        if (stats.max_air_speed < airspeed_estimate)
            stats.max_air_speed = airspeed_estimate;

        if (stats.max_distance < GPS_distanceToHome)
            stats.max_distance = GPS_distanceToHome;

        if (stats.min_sats > gpsSol.numSat)
            stats.min_sats = gpsSol.numSat;

        if (stats.max_sats < gpsSol.numSat)
            stats.max_sats = gpsSol.numSat;
    }
#if defined(USE_ESC_SENSOR)
    if (STATE(ESC_SENSOR_ENABLED)) {
        escSensorData_t * escSensor = escSensorGetData();
        bool escTemperatureValid = escSensor && escSensor->dataAge <= ESC_DATA_MAX_AGE;

        if (escTemperatureValid) {
            if (stats.min_esc_temp > escSensor->temperature)
                stats.min_esc_temp = escSensor->temperature;

            if (stats.max_esc_temp < escSensor->temperature)
                stats.max_esc_temp = escSensor->temperature;
        }
    }
#endif

    value = getBatteryVoltage();
    if (stats.min_voltage > value)
        stats.min_voltage = value;

    value = abs(getAmperage());
    if (stats.max_current < value)
        stats.max_current = value;

    value = labs(getPower());
    if (stats.max_power < value)
        stats.max_power = value;

    value = osdConvertRSSI();
    if (stats.min_rssi > value)
        stats.min_rssi = value;

    value = osdGetCrsfLQ();
    if (stats.min_lq > value)
        stats.min_lq = value;

    if (!failsafeIsReceivingRxData())
        stats.min_lq = 0;

    value = osdGetCrsfdBm();
    if (stats.min_rssi_dbm > value)
        stats.min_rssi_dbm = value;

    stats.max_altitude = MAX(stats.max_altitude, osdGetAltitude());
}

uint8_t drawStat_FlightTime(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    displayWrite(osdDisplayPort, col, row, "FLIGHT TIME");
    uint16_t flySeconds = getFlightTime();
    uint16_t flyMinutes = flySeconds / 60;
    flySeconds %= 60;
    uint16_t flyHours = flyMinutes / 60;
    flyMinutes %= 60;
    tfp_sprintf(buff, ": %02u:%02u:%02u", flyHours, flyMinutes, flySeconds);
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_FlightDistance(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];

    displayWrite(osdDisplayPort, col, row, "FLIGHT DISTANCE");
    tfp_sprintf(buff, ": ");
    osdFormatDistanceStr(buff + 2, getTotalTravelDistance());
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_MaxDistanceFromHome(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    uint8_t valueXOffset = 0;
    if (!osdDisplayIsHD()) {
        displayWrite(osdDisplayPort, col, row, "DISTANCE FROM ");
    valueXOffset = 14;
    } else {
        displayWrite(osdDisplayPort, col, row, "MAX DISTANCE FROM ");
    valueXOffset = 18;
    }
    displayWriteChar(osdDisplayPort, col + valueXOffset, row, SYM_HOME);
    tfp_sprintf(buff, ": ");
    osdFormatDistanceStr(buff + 2, stats.max_distance * 100);
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_Speed(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    char buff2[12];
    uint8_t multiValueXOffset = 0;

    displayWrite(osdDisplayPort, col, row, "MAX/AVG SPEED");

    osdFormatVelocityStr(buff2, stats.max_3D_speed, true, false);
    tfp_sprintf(buff, ": %s/", osdFormatTrimWhiteSpace(buff2));
    multiValueXOffset = strlen(buff);
    displayWrite(osdDisplayPort, statValX, row, buff);

    osdGenerateAverageVelocityStr(buff2);
    displayWrite(osdDisplayPort, statValX + multiValueXOffset, row++, osdFormatTrimWhiteSpace(buff2));

    return row;
}

uint8_t drawStat_MaximumAltitude(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    displayWrite(osdDisplayPort, col, row, "MAX ALTITUDE");
    tfp_sprintf(buff, ": ");
    osdFormatAltitudeStr(buff + 2, stats.max_altitude);
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_BatteryVoltage(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    uint8_t multiValueXOffset = 0;
    if (!osdDisplayIsHD())
        displayWrite(osdDisplayPort, col, row, "MIN VOLTS P/C");
    else
        displayWrite(osdDisplayPort, col, row, "MIN VOLTS PACK/CELL");

    // Pack voltage
    tfp_sprintf(buff, ": ");
    osdFormatCentiNumber(buff + 2, stats.min_voltage, 0, osdConfig()->main_voltage_decimals, 0, osdConfig()->main_voltage_decimals + 2, false);
    strcat(osdFormatTrimWhiteSpace(buff), "/");
    multiValueXOffset = strlen(buff);
    // AverageCell
    osdFormatCentiNumber(buff + multiValueXOffset, stats.min_voltage / getBatteryCellCount(), 0, 2, 0, 3, false);
    tfp_sprintf(buff + strlen(buff), "%c", SYM_VOLT);

    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_MaximumPowerAndCurrent(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    char outBuff[12];
    tfp_sprintf(outBuff, ": ");
    osdFormatCentiNumber(buff, stats.max_current, 0, 2, 0, 3, false);
    strcat(outBuff, osdFormatTrimWhiteSpace(buff));
    strcat(outBuff, "/");
    bool kiloWatt = osdFormatCentiNumber(buff, stats.max_power, 1000, 2, 2, 3, false);
    strcat(outBuff, osdFormatTrimWhiteSpace(buff));
    displayWrite(osdDisplayPort, statValX, row, outBuff);

    if (kiloWatt)
        displayWrite(osdDisplayPort, col, row, "MAX AMPS/K WATTS");
    else
        displayWrite(osdDisplayPort, col, row, "MAX AMPS/WATTS");

    return ++row;
}

uint8_t drawStat_UsedEnergy(uint8_t col, uint8_t row, uint8_t statValX)
{
    char    buff[12];

    if (osdDisplayIsHD())
        displayWrite(osdDisplayPort, col, row, "USED ENERGY FLT/TOT");
    else
        displayWrite(osdDisplayPort, col, row, "USED ENERGY F/T");
    tfp_sprintf(buff, ": ");
    if (osdConfig()->stats_energy_unit == OSD_STATS_ENERGY_UNIT_MAH) {
        tfp_sprintf(buff + 2, "%d/%d%c", (int)(getMAhDrawn() - stats.flightStartMAh), (int)getMAhDrawn(), SYM_MAH);
    } else {
        char preBuff[12];
        osdFormatCentiNumber(preBuff, (getMWhDrawn() - stats.flightStartMWh) / 10, 0, 2, 0, 3, false);
        strcat(buff, osdFormatTrimWhiteSpace(preBuff));
        strcat(buff, "/");
        osdFormatCentiNumber(preBuff, getMWhDrawn() / 10, 0, 2, 0, 3, false);
        strcat(buff, osdFormatTrimWhiteSpace(preBuff));
        tfp_sprintf(buff + strlen(buff), "%c", SYM_WH);
    }
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_AverageEfficiency(uint8_t col, uint8_t row, uint8_t statValX, bool forceMetric)
{
    char buff[15];
    char outBuff[15];
    int32_t totalDistance = getTotalTravelDistance();
    bool moreThanAh = false;
    bool efficiencyValid = totalDistance >= 10000;

    if (osdDisplayIsHD())
        displayWrite(osdDisplayPort, col, row, "AVG EFFICIENCY FLT/TOT");
    else
        displayWrite(osdDisplayPort, col, row, "AV EFFICIENCY F/T");

    tfp_sprintf(outBuff, ": ");
    uint8_t digits = 3U;    // Total number of digits (including decimal point)
#ifndef DISABLE_MSP_DJI_COMPAT   // IF DJICOMPAT is not supported, there's no need to check for it and change the values
    if (isDJICompatibleVideoSystem(osdConfig())) {
        // Add one digit so no switch to scaled decimal occurs above 99
        digits = 4U;
    }
#endif
    if (!forceMetric) {
        switch (osdConfig()->units) {
            case OSD_UNIT_UK:
                FALLTHROUGH;
            case OSD_UNIT_IMPERIAL:
                if (osdConfig()->stats_energy_unit == OSD_STATS_ENERGY_UNIT_MAH) {
                    if (efficiencyValid) {
                        moreThanAh = osdFormatCentiNumber(buff, (int32_t)((getMAhDrawn() - stats.flightStartMAh) * 10000.0f * METERS_PER_MILE / totalDistance), 1000, 0, 2, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                        if (osdDisplayIsHD()) {
                            if (!moreThanAh)
                                tfp_sprintf(outBuff + strlen(outBuff), "%c%c", SYM_MAH_MI_0, SYM_MAH_MI_1);
                            else
                                tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_AH_MI);

                            moreThanAh = false;
                        }

                        strcat(outBuff, "/");
                        moreThanAh = moreThanAh || osdFormatCentiNumber(buff, (int32_t)(getMAhDrawn() * 10000.0f * METERS_PER_MILE / totalDistance), 1000, 0, 2, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));

                        if (!moreThanAh)
                            tfp_sprintf(outBuff + strlen(outBuff), "%c%c", SYM_MAH_MI_0, SYM_MAH_MI_1);
                        else
                            tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_AH_MI);
                    } else {
                        tfp_sprintf(outBuff + strlen(outBuff), "---/---%c%c", SYM_MAH_MI_0, SYM_MAH_MI_1);
                    }
                } else {
                    if (efficiencyValid) {
                        osdFormatCentiNumber(buff, (int32_t)((getMWhDrawn() - stats.flightStartMWh) * 10.0f * METERS_PER_MILE / totalDistance), 0, 2, 0, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                        strcat(outBuff, "/");
                        osdFormatCentiNumber(buff, (int32_t)(getMWhDrawn() * 10.0f * METERS_PER_MILE / totalDistance), 0, 2, 0, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                    } else {
                        strcat(outBuff, "---/---");
                    }
                    tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_WH_MI);
                }
                break;
            case OSD_UNIT_GA:
                if (osdConfig()->stats_energy_unit == OSD_STATS_ENERGY_UNIT_MAH) {
                    if (efficiencyValid) {
                        moreThanAh = osdFormatCentiNumber(buff, (int32_t)((getMAhDrawn()-stats.flightStartMAh) * 10000.0f * METERS_PER_NAUTICALMILE / totalDistance), 1000, 0, 2, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                         if (osdDisplayIsHD()) {
                            if (!moreThanAh)
                                tfp_sprintf(outBuff + strlen(outBuff), "%c%c", SYM_MAH_NM_0, SYM_MAH_NM_1);
                            else
                                tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_AH_NM);

                            moreThanAh = false;
                        }

                        strcat(outBuff, "/");
                        moreThanAh = moreThanAh || osdFormatCentiNumber(buff, (int32_t)(getMAhDrawn() * 10000.0f * METERS_PER_NAUTICALMILE / totalDistance), 1000, 0, 2, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                        if (!moreThanAh) {
                            tfp_sprintf(outBuff + strlen(outBuff), "%c%c", SYM_MAH_NM_0, SYM_MAH_NM_1);
                        } else {
                            tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_AH_NM);
                        }
                    } else {
                        tfp_sprintf(outBuff + strlen(outBuff), "---/---%c%c", SYM_MAH_NM_0, SYM_MAH_NM_1);
                    }
                } else {
                    if (efficiencyValid) {
                        osdFormatCentiNumber(buff, (int32_t)((getMWhDrawn()-stats.flightStartMWh) * 10.0f * METERS_PER_NAUTICALMILE / totalDistance), 0, 2, 0, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                        strcat(outBuff, "/");
                        osdFormatCentiNumber(buff, (int32_t)(getMWhDrawn() * 10.0f * METERS_PER_NAUTICALMILE / totalDistance), 0, 2, 0, digits, false);
                        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                    } else {
                        strcat(outBuff, "---/---");
                    }
                    tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_WH_NM);
                }
                break;
            case OSD_UNIT_METRIC_MPH:
            case OSD_UNIT_METRIC:
                forceMetric = true;
                break;
        }
    }

    if (forceMetric) {
        if (osdConfig()->stats_energy_unit == OSD_STATS_ENERGY_UNIT_MAH) {
            if (efficiencyValid) {
                moreThanAh = osdFormatCentiNumber(buff, (int32_t)((getMAhDrawn() - stats.flightStartMAh) * 10000000.0f / totalDistance), 1000, 0, 2, digits, false);
                strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                if (osdDisplayIsHD()) {
                    if (!moreThanAh)
                        tfp_sprintf(outBuff + strlen(outBuff), "%c%c", SYM_MAH_KM_0, SYM_MAH_KM_1);
                    else
                        tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_AH_KM);

                    moreThanAh = false;
                }

                strcat(outBuff, "/");
                moreThanAh = moreThanAh || osdFormatCentiNumber(buff, (int32_t)(getMAhDrawn() * 10000000.0f / totalDistance), 1000, 0, 2, digits, false);
                strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                if (!moreThanAh) {
                    tfp_sprintf(outBuff + strlen(outBuff), "%c%c", SYM_MAH_KM_0, SYM_MAH_KM_1);
                } else {
                    tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_AH_KM);
                }
            } else {
                tfp_sprintf(outBuff + strlen(outBuff), "---/---%c%c", SYM_MAH_KM_0, SYM_MAH_KM_1);
            }
        } else {
            if (efficiencyValid) {
                osdFormatCentiNumber(buff, (int32_t)((getMWhDrawn() - stats.flightStartMWh) * 10000.0f / totalDistance), 0, 2, 0, digits, false);
                strcat(outBuff, osdFormatTrimWhiteSpace(buff));
                strcat(outBuff, "/");
                osdFormatCentiNumber(buff, (int32_t)(getMWhDrawn() * 10000.0f / totalDistance), 0, 2, 0, digits, false);
                strcat(outBuff, osdFormatTrimWhiteSpace(buff));
            } else {
                strcat(outBuff, "---/---");
            }
            tfp_sprintf(outBuff + strlen(outBuff), "%c", SYM_WH_KM);
        }
    }

    tfp_sprintf(outBuff + strlen(outBuff), "%c", '\0');
    displayWrite(osdDisplayPort, statValX, row++, outBuff);

    return row;
}

uint8_t drawStat_RXStats(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[20];
    uint8_t multiValueXOffset = 0;

    tfp_sprintf(buff, "MIN RSSI");
    if (rxConfig()->serialrx_provider == SERIALRX_CRSF) {
        strcat(buff, "/LQ");

        if (osdDisplayIsHD())
            strcat(buff, "/DBM");
    }
    displayWrite(osdDisplayPort, col, row, buff);

    memset(buff, '\0', strlen(buff));
    tfp_sprintf(buff, ": ");
    itoa(stats.min_rssi, buff + 2, 10);
    strcat(osdFormatTrimWhiteSpace(buff), "%");

    if (rxConfig()->serialrx_provider == SERIALRX_CRSF) {
        strcat(osdFormatTrimWhiteSpace(buff), "/");
        multiValueXOffset = strlen(buff);
        itoa(stats.min_lq, buff + multiValueXOffset, 10);
        strcat(osdFormatTrimWhiteSpace(buff), "%");

        if (osdDisplayIsHD()) {
            strcat(osdFormatTrimWhiteSpace(buff), "/");
            itoa(stats.min_rssi_dbm, buff + 2, 10);
            tfp_sprintf(buff + strlen(buff), "%c", SYM_DBM);
            displayWrite(osdDisplayPort, statValX, row++, buff);
        }
    }

    displayWrite(osdDisplayPort, statValX, row++, buff);

    if (!osdDisplayIsHD() && rxConfig()->serialrx_provider == SERIALRX_CRSF) {
        displayWrite(osdDisplayPort, col, row, "MIN RX DBM");
        memset(buff, '\0', strlen(buff));
        tfp_sprintf(buff, ": ");
        itoa(stats.min_rssi_dbm, buff + 2, 10);
        tfp_sprintf(buff + strlen(buff), "%c", SYM_DBM);
        displayWrite(osdDisplayPort, statValX, row++, buff);
    }

    return row;
}

uint8_t drawStat_GPS(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    displayWrite(osdDisplayPort, col, row, "MIN/MAX GPS SATS");
    tfp_sprintf(buff, ": %u/%u", stats.min_sats, stats.max_sats);
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_ESCTemperature(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    displayWrite(osdDisplayPort, col, row, "MIN/MAX ESC TEMP");
    tfp_sprintf(buff, ": %3d/%3d%c",
                ((osdConfig()->units == OSD_UNIT_IMPERIAL) ? (int16_t)(stats.min_esc_temp * 9 / 5.0f + 320) : stats.min_esc_temp),
                ((osdConfig()->units == OSD_UNIT_IMPERIAL) ? (int16_t)(stats.max_esc_temp * 9 / 5.0f + 320) : stats.max_esc_temp),
                ((osdConfig()->units == OSD_UNIT_IMPERIAL) ? SYM_TEMP_F : SYM_TEMP_C));
    displayWrite(osdDisplayPort, statValX, row++, buff);

    return row;
}

uint8_t drawStat_GForce(uint8_t col, uint8_t row, uint8_t statValX)
{
    char buff[12];
    char outBuff[20];

    const float max_gforce = accGetMeasuredMaxG();
    const acc_extremes_t *acc_extremes = accGetMeasuredExtremes();
    const float acc_extremes_min = acc_extremes[Z].min;
    const float acc_extremes_max = acc_extremes[Z].max;

    if (!osdDisplayIsHD())
        displayWrite(osdDisplayPort, col, row, "MAX G-FORCE");
    else
        displayWrite(osdDisplayPort, col, row, "MAX/MIN Z/MAX Z G-FORCE");

    tfp_sprintf(outBuff, ": ");
    osdFormatCentiNumber(buff, max_gforce * 100, 0, 2, 0, 3, false);

    if (!osdDisplayIsHD()) {
        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
        displayWrite(osdDisplayPort, statValX, row++, outBuff);

        displayWrite(osdDisplayPort, col, row, "MIN/MAX Z G-FORCE");
        memset(outBuff, '\0', strlen(outBuff));
        tfp_sprintf(outBuff, ": ");
    } else {
        strcat(outBuff, osdFormatTrimWhiteSpace(buff));
        strcat(outBuff, "/");
    }
    osdFormatCentiNumber(buff, acc_extremes_min * 100, 0, 2, 0, 4, false);
    strcat(outBuff, osdFormatTrimWhiteSpace(buff));
    strcat(outBuff, "/");

    osdFormatCentiNumber(buff, acc_extremes_max * 100, 0, 2, 0, 3, false);
    strcat(outBuff, osdFormatTrimWhiteSpace(buff));
    displayWrite(osdDisplayPort, statValX, row++, outBuff);

    return row;
}

uint8_t drawStat_DisarmMethod(uint8_t col, uint8_t row, uint8_t statValX)
{
    // We keep "" for backward compatibility with the Blackbox explorer and other potential usages
    const char * disarmReasonStr[DISARM_REASON_COUNT] = { "UNKNOWN", "TIMEOUT", "STICKS", "SWITCH", "SWITCH", "", "FAILSAFE", "NAV SYS", "LANDING"};

    displayWrite(osdDisplayPort, col, row, "DISARMED BY");
    displayWrite(osdDisplayPort, statValX, row, ": ");
    displayWrite(osdDisplayPort, statValX + 2, row++, disarmReasonStr[getDisarmReason()]);

    return row;
}

static void osdShowStats(bool isSinglePageStatsCompatible, uint8_t page)
{
    const char * statsHeader[2] = {"*** STATS   1/2 -> ***", "*** STATS   <- 2/2 ***"};
    uint8_t row = 1;  // Start one line down leaving space at the top of the screen.

    const uint8_t statNameX = (osdDisplayPort->cols - (osdDisplayIsHD() ? 41 : 28)) / 2;
    const uint8_t statValuesX = osdDisplayPort->cols - statNameX - (osdDisplayIsHD() ? 15 : 11);

    if (page > 1)
        page = 0;

    displayBeginTransaction(osdDisplayPort, DISPLAY_TRANSACTION_OPT_RESET_DRAWING);
    displayClearScreen(osdDisplayPort);

    if (isSinglePageStatsCompatible) {
        char buff[25];
        tfp_sprintf(buff, "*** STATS ");
#ifdef USE_BLACKBOX
#ifdef USE_SDCARD
        if (feature(FEATURE_BLACKBOX)) {
            int32_t logNumber = blackboxGetLogNumber();
            if (logNumber >= 0)
                tfp_sprintf(buff + strlen(buff), " %c%05" PRId32 " ", SYM_BLACKBOX, logNumber);
            else
                tfp_sprintf(buff + strlen(buff), " %c ", SYM_BLACKBOX);
        }
#endif
#endif
        strcat(buff, "***");

        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buff)) / 2, row++, buff);
    } else
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(statsHeader[page + 1])) / 2, row++, statsHeader[page]);

    if (isSinglePageStatsCompatible) {
        // Top 15 rows for most important stats. Max 19 rows (WTF)
        row = drawStat_FlightTime(statNameX, row, statValuesX); // 1 row
        row = drawStat_FlightDistance(statNameX, row, statValuesX); // 1 row
        if (feature(FEATURE_GPS)) row = drawStat_MaxDistanceFromHome(statNameX, row, statValuesX); // 1 row
        if (feature(FEATURE_GPS)) row = drawStat_Speed(statNameX, row, statValuesX); // 1 row
        row = drawStat_MaximumAltitude(statNameX, row, statValuesX); // 1 row
        row = drawStat_BatteryVoltage(statNameX, row, statValuesX); // 1 row
        if (feature(FEATURE_CURRENT_METER)) row = drawStat_MaximumPowerAndCurrent(statNameX, row, statValuesX); // 1 row
        if (feature(FEATURE_CURRENT_METER)) row = drawStat_UsedEnergy(statNameX, row, statValuesX); // 1 row
        if (feature(FEATURE_CURRENT_METER) && feature(FEATURE_GPS)) row = drawStat_AverageEfficiency(statNameX, row, statValuesX, false); // 1 row
        if (osdConfig()->stats_show_metric_efficiency && osdIsNotMetric() && feature(FEATURE_CURRENT_METER) && feature(FEATURE_GPS)) row = drawStat_AverageEfficiency(statNameX, row, statValuesX, true); // 1 row
        row = drawStat_RXStats(statNameX, row, statValuesX); // 1 row if non-CRSF else 2 rows
        if (feature(FEATURE_GPS)) row = drawStat_GPS(statNameX, row, statValuesX); // 1 row
        if (STATE(ESC_SENSOR_ENABLED)) row = drawStat_ESCTemperature(statNameX, row, statValuesX); // 1 row

        // Draw these if there is space space
        if (row < (osdDisplayPort->cols-3)) row = drawStat_GForce(statNameX, row, statValuesX); // 1 row HD or 2 rows SD
#ifdef USE_STATS
        if (row < (osdDisplayPort->cols-7) && statsConfig()->stats_enabled) row = drawStat_Stats(statNameX, row, statValuesX, false); // 4 rows
#endif
    } else {
        switch (page) {
            case 0:
                // Max 10 rows
                row = drawStat_FlightTime(statNameX, row, statValuesX); // 1 row
                row = drawStat_FlightDistance(statNameX, row, statValuesX); // 1 row
                if (feature(FEATURE_GPS)) row = drawStat_MaxDistanceFromHome(statNameX, row, statValuesX); // 1 row
                if (feature(FEATURE_GPS)) row = drawStat_Speed(statNameX, row, statValuesX); // 1 row
                row = drawStat_MaximumAltitude(statNameX, row, statValuesX); // 1 row
                row = drawStat_BatteryVoltage(statNameX, row, statValuesX); // 1 row
                if (feature(FEATURE_CURRENT_METER)) row = drawStat_MaximumPowerAndCurrent(statNameX, row, statValuesX); // 1 row
                if (feature(FEATURE_CURRENT_METER))row = drawStat_UsedEnergy(statNameX, row, statValuesX); // 1 row
                if (feature(FEATURE_CURRENT_METER) && feature(FEATURE_GPS)) row = drawStat_AverageEfficiency(statNameX, row, statValuesX, false); // 1 row
                if (feature(FEATURE_GPS))row = drawStat_GPS(statNameX, row, statValuesX); // 1 row
                break;
            case 1:
                // Max 10 rows
                row = drawStat_RXStats(statNameX, row, statValuesX); // 1 row if non-CRSF else 2 rows
                if (STATE(ESC_SENSOR_ENABLED)) row = drawStat_ESCTemperature(statNameX, row, statValuesX); // 1 row
                row = drawStat_GForce(statNameX, row, statValuesX); // 1 row HD or 2 rows SD
                if (osdConfig()->stats_show_metric_efficiency && osdIsNotMetric() && feature(FEATURE_CURRENT_METER) && feature(FEATURE_GPS)) row = drawStat_AverageEfficiency(statNameX, row, statValuesX, true); // 1 row
#ifdef USE_BLACKBOX
#ifdef USE_SDCARD
                if (feature(FEATURE_BLACKBOX)) {
                    char buff[12];
                    displayWrite(osdDisplayPort, statNameX, row, "BLACKBOX FILE");

                    tfp_sprintf(buff, ": %u/%u", stats.min_sats, stats.max_sats);

                    int32_t logNumber = blackboxGetLogNumber();
                    if (logNumber >= 0)
                        tfp_sprintf(buff, ": %05ld ", logNumber);
                    else
                        strcat(buff, ": INVALID");

                    displayWrite(osdDisplayPort, statValuesX, row++, buff); // 1 row
                }
#endif
#endif
#ifdef USE_STATS
                if (row < (osdDisplayPort->cols-7) && statsConfig()->stats_enabled) row = drawStat_Stats(statNameX, row, statValuesX, false); // 4 rows
#endif

                break;
        }
    }

    row = drawStat_DisarmMethod(statNameX, row, statValuesX);

    // The following has been commented out as it will be added in #9688
    // uint16_t rearmMs = (emergInflightRearmEnabled()) ? emergencyInFlightRearmTimeMS() : 0;

    if (savingSettings == true) {
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(OSD_MESSAGE_STR(OSD_MSG_SAVING_SETTNGS))) / 2, row++, OSD_MESSAGE_STR(OSD_MSG_SAVING_SETTNGS));
    /*} else if (rearmMs > 0) { // Show rearming time if settings not actively being saved. Ignore the settings saved message if rearm available.
        char emReArmMsg[23];
        tfp_sprintf(emReArmMsg, "** REARM PERIOD: ");
        tfp_sprintf(emReArmMsg + strlen(emReArmMsg), "%02d", (uint8_t)MS2S(rearmMs));
        strcat(emReArmMsg, " **\0");
        displayWrite(osdDisplayPort, statNameX, top++, OSD_MESSAGE_STR(emReArmMsg));*/
    } else if (notify_settings_saved > 0) {
        if (millis() > notify_settings_saved) {
            notify_settings_saved = 0;
        } else {
            displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(OSD_MESSAGE_STR(OSD_MSG_SETTINGS_SAVED))) / 2, row++, OSD_MESSAGE_STR(OSD_MSG_SETTINGS_SAVED));
        }
    }

    displayCommitTransaction(osdDisplayPort);
}

// HD arming screen. based on the minimum HD OSD grid size of 50 x 18
static void osdShowHDArmScreen(void)
{
    dateTime_t dt;
    char        buf[MAX(osdDisplayPort->cols, FORMATTED_DATE_TIME_BUFSIZE)];
    char        buf2[MAX(osdDisplayPort->cols, FORMATTED_DATE_TIME_BUFSIZE)];
    char        craftNameBuf[MAX_NAME_LENGTH];
    char        versionBuf[osdDisplayPort->cols];
    uint8_t     safehomeRow     = 0;
    uint8_t     armScreenRow    = 1;

    bool        showPilotOrCraftName = false;

    armScreenRow = drawLogos(false, armScreenRow);
    armScreenRow++;

    if (!osdConfig()->use_pilot_logo && osdElementEnabled(OSD_PILOT_NAME, false) && strlen(systemConfig()->pilotName) > 0) {
        osdFormatPilotName(buf2);
        showPilotOrCraftName = true;
    }

    if (osdElementEnabled(OSD_CRAFT_NAME, false) && strlen(systemConfig()->craftName) > 0) {
        osdFormatCraftName(craftNameBuf);
        if (strlen(buf2) > 0) {
            strcat(buf2, " : ");
        }
        showPilotOrCraftName = true;
    }

    if (showPilotOrCraftName) {
        tfp_sprintf(buf, "%s%s: ! ARMED !", buf2, craftNameBuf);
    } else {
        strcpy(buf, " ! ARMED !");
    }

    displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
    memset(buf, '\0', sizeof(buf));
    memset(buf2, '\0', sizeof(buf2));

#if defined(USE_GPS)
#if defined (USE_SAFE_HOME)
    if (posControl.safehomeState.distance) {
        safehomeRow = armScreenRow;
        armScreenRow +=2;
    }
#endif // USE_SAFE_HOME
#endif // USE_GPS

    if (posControl.waypointListValid && posControl.waypointCount > 0) {
#ifdef USE_MULTI_MISSION
        tfp_sprintf(buf, "MISSION %u/%u (%u WP)", posControl.loadedMultiMissionIndex, posControl.multiMissionCount, posControl.waypointCount);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
        memset(buf, '\0', sizeof(buf));
#else
        strcpy(buf, "*MISSION LOADED*");
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
        memset(buf, '\0', sizeof(buf));
#endif
    }

#if defined(USE_GPS)
    if (feature(FEATURE_GPS)) {
        if (STATE(GPS_FIX_HOME)) {
            if (osdConfig()->osd_home_position_arm_screen) {
                // Show pluscode if enabled on any OSD layout. Otherwise show GNSS cordinates.
                if (osdElementEnabled(OSD_PLUS_CODE, false)) {
                    int digits = osdConfig()->plus_code_digits;
                    olc_encode(GPS_home.lat, GPS_home.lon, digits, buf, sizeof(buf));
                    tfp_sprintf(buf2, "+CODE: %s%c", buf, '\0');
                    displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf2)) / 2, armScreenRow++, buf2);
                    memset(buf, '\0', sizeof(buf));
                    memset(buf2, '\0', sizeof(buf2));
                } else {
                    osdFormatCoordinate(buf, SYM_LAT, GPS_home.lat);
                    osdFormatCoordinate(buf2, SYM_LON, GPS_home.lon);
                    uint8_t gap = 1;
                    uint8_t col = strlen(buf) + strlen(buf2) + gap;

                    if ((osdDisplayPort->cols %2) != (col %2)) {
                        gap++;
                        col++;
                    }

                    col = (osdDisplayPort->cols - col) / 2;

                    displayWrite(osdDisplayPort, col, armScreenRow, buf);
                    displayWrite(osdDisplayPort, col + strlen(buf) + gap, armScreenRow++, buf2);
                    memset(buf, '\0', sizeof(buf));
                    memset(buf2, '\0', sizeof(buf2));
                }
            }

#if defined (USE_SAFE_HOME)
            if (posControl.safehomeState.distance) { // safehome found during arming
                if (navConfig()->general.flags.safehome_usage_mode == SAFEHOME_USAGE_OFF) {
                    strcpy(buf, "SAFEHOME FOUND; MODE OFF");
                } else {
                    osdFormatDistanceStr(buf2, posControl.safehomeState.distance);
                    tfp_sprintf(buf, "%c SAFEHOME %u @ %s", SYM_HOME, posControl.safehomeState.index, buf2);
                }
                textAttributes_t elemAttr = _TEXT_ATTRIBUTES_BLINK_BIT;
                // write this message below the ARMED message to make it obvious
                displayWriteWithAttr(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, safehomeRow, buf, elemAttr);
                memset(buf, '\0', sizeof(buf));
                memset(buf2, '\0', sizeof(buf2));
            }
#endif
        } else {
            strcpy(buf, "!NO HOME POSITION!");
            displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
            memset(buf, '\0', sizeof(buf));
            armScreenRow++;
        }
    }
#endif

    if (rtcGetDateTimeLocal(&dt)) {
        tfp_sprintf(buf, "%04u-%02u-%02u  %02u:%02u:%02u", dt.year, dt.month, dt.day, dt.hours, dt.minutes, dt.seconds);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
        memset(buf, '\0', sizeof(buf));
        armScreenRow++;
    }

    tfp_sprintf(versionBuf, "INAV VERSION: %s", FC_VERSION_STRING);
    displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(versionBuf)) / 2, armScreenRow++, versionBuf);
    armScreenRow++;

#ifdef USE_STATS
    if (armScreenRow < (osdDisplayPort->rows - 4))
        armScreenRow = drawStats(armScreenRow);
#endif // USE_STATS
    }

static void osdShowSDArmScreen(void)
{
    dateTime_t  dt;
    char        buf[MAX(osdDisplayPort->cols, FORMATTED_DATE_TIME_BUFSIZE)];
    char        buf2[MAX(osdDisplayPort->cols, FORMATTED_DATE_TIME_BUFSIZE)];
    char        craftNameBuf[MAX_NAME_LENGTH];
    char        versionBuf[osdDisplayPort->cols];
    uint8_t     armScreenRow = 1;
    uint8_t     safehomeRow = 0;
    bool        showPilotOrCraftName = false;

    strcpy(buf, "! ARMED !");
    displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
    memset(buf, '\0', sizeof(buf));
#if defined(USE_GPS)
#if defined (USE_SAFE_HOME) 
    if (posControl.safehomeState.distance) {
        safehomeRow = armScreenRow;
        armScreenRow += 2;
    }
#endif
#endif

    if (osdElementEnabled(OSD_PILOT_NAME, false) && strlen(systemConfig()->pilotName) > 0) {
        osdFormatPilotName(buf2);
        showPilotOrCraftName = true;
    }

    if (osdElementEnabled(OSD_CRAFT_NAME, false) && strlen(systemConfig()->craftName) > 0) {
        osdFormatCraftName(craftNameBuf);
        if (strlen(buf2) > 0) {
            strcat(buf2, " : ");
        }
        showPilotOrCraftName = true;
    }

    if (showPilotOrCraftName) {
        tfp_sprintf(buf, "%s%s", buf2, craftNameBuf);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf );
        memset(buf, '\0', sizeof(buf));
        memset(buf2, '\0', sizeof(buf2));
        armScreenRow++;
    }

    if (posControl.waypointListValid && posControl.waypointCount > 0) {
#ifdef USE_MULTI_MISSION
        tfp_sprintf(buf, "MISSION %u/%u (%u WP)", posControl.loadedMultiMissionIndex, posControl.multiMissionCount, posControl.waypointCount);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
        memset(buf, '\0', sizeof(buf));
#else
        strcpy(buf, "*MISSION LOADED*");
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
        memset(buf, '\0', sizeof(buf));
#endif
    }

#if defined(USE_GPS)
    if (feature(FEATURE_GPS)) {
        if (STATE(GPS_FIX_HOME)) {
            if (osdConfig()->osd_home_position_arm_screen) {
                // Show pluscode if enabled on any OSD layout. Otherwise show GNSS cordinates.
                if (osdElementEnabled(OSD_PLUS_CODE, false)) {
                    int digits = osdConfig()->plus_code_digits;
                    olc_encode(GPS_home.lat, GPS_home.lon, digits, buf, sizeof(buf));
                    tfp_sprintf(buf2, "+CODE: %s%c", buf, '\0');
                    displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf2)) / 2, armScreenRow++, buf2);
                    memset(buf, '\0', sizeof(buf));
                    memset(buf2, '\0', sizeof(buf2));
                } else {
                    osdFormatCoordinate(buf, SYM_LAT, GPS_home.lat);
                    osdFormatCoordinate(buf2, SYM_LON, GPS_home.lon);

                    uint8_t gpsStartCol = (osdDisplayPort->cols - (strlen(buf) + strlen(buf2) + 2)) / 2;
                    displayWrite(osdDisplayPort, gpsStartCol, armScreenRow, buf);
                    displayWrite(osdDisplayPort, gpsStartCol + strlen(buf) + 2, armScreenRow++, buf2);
                    memset(buf, '\0', sizeof(buf));
                    memset(buf2, '\0', sizeof(buf2));
                }
            }

#if defined (USE_SAFE_HOME)
            if (posControl.safehomeState.distance) { // safehome found during arming
                if (navConfig()->general.flags.safehome_usage_mode == SAFEHOME_USAGE_OFF) {
                    strcpy(buf, "SAFEHOME FOUND; MODE OFF");
                } else {
                    osdFormatDistanceStr(buf2, posControl.safehomeState.distance);
                    tfp_sprintf(buf, "%c SAFEHOME %u @ %s", SYM_HOME, posControl.safehomeState.index, buf2);
                }
                // write this message below the ARMED message to make it obvious
                displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, safehomeRow, buf);
                memset(buf, '\0', sizeof(buf));
                memset(buf2, '\0', sizeof(buf2));
            }
#endif
        } else {
            strcpy(buf, "!NO HOME POSITION!");
            displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
            memset(buf, '\0', sizeof(buf));
            armScreenRow++;
        }
    }
#endif

    if (rtcGetDateTimeLocal(&dt)) {
        tfp_sprintf(buf, "%04u-%02u-%02u  %02u:%02u:%02u", dt.year, dt.month, dt.day, dt.hours, dt.minutes, dt.seconds);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, armScreenRow++, buf);
        memset(buf, '\0', sizeof(buf));
        armScreenRow++;
    }

    tfp_sprintf(versionBuf, "INAV VERSION: %s", FC_VERSION_STRING);
    displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(versionBuf)) / 2, armScreenRow++, versionBuf);
    armScreenRow++;

#ifdef USE_STATS
    if (armScreenRow < (osdDisplayPort->rows - 4))
        armScreenRow = drawStats(armScreenRow);
#endif // USE_STATS
}

// called when motors armed
static void osdShowArmed(void)
{
    displayClearScreen(osdDisplayPort);

    if (osdDisplayIsHD()) {
        osdShowHDArmScreen();
    } else {
        osdShowSDArmScreen();
    }
}

static void osdFilterData(timeUs_t currentTimeUs) {
    static timeUs_t lastRefresh = 0;
    float refresh_dT = US2S(cmpTimeUs(currentTimeUs, lastRefresh));

    GForce = fast_fsqrtf(vectorNormSquared(&imuMeasuredAccelBF)) / GRAVITY_MSS;
    for (uint8_t axis = 0; axis < XYZ_AXIS_COUNT; ++axis) GForceAxis[axis] = imuMeasuredAccelBF.v[axis] / GRAVITY_MSS;

    if (lastRefresh) {
        GForce = pt1FilterApply3(&GForceFilter, GForce, refresh_dT);
        for (uint8_t axis = 0; axis < XYZ_AXIS_COUNT; ++axis) pt1FilterApply3(GForceFilterAxis + axis, GForceAxis[axis], refresh_dT);
    } else {
        pt1FilterInitRC(&GForceFilter, GFORCE_FILTER_TC, 0);
        pt1FilterReset(&GForceFilter, GForce);

        for (uint8_t axis = 0; axis < XYZ_AXIS_COUNT; ++axis) {
            pt1FilterInitRC(GForceFilterAxis + axis, GFORCE_FILTER_TC, 0);
            pt1FilterReset(GForceFilterAxis + axis, GForceAxis[axis]);
        }
    }

    lastRefresh = currentTimeUs;
}

// Detect when the user is holding the roll stick to the right
static bool osdIsPageUpStickCommandHeld(void)
{
    static int pageUpHoldCount = 1;

    bool keyHeld = false;

    if (IS_HI(ROLL)) {
         keyHeld = true;
    }

    if (!keyHeld) {
        pageUpHoldCount = 1;
    } else {
        ++pageUpHoldCount;
    }

    if (pageUpHoldCount > 20) {
        pageUpHoldCount = 1;
        return true;
    }

    return false;
}

// Detect when the user is holding the roll stick to the left
static bool osdIsPageDownStickCommandHeld(void)
{
    static int pageDownHoldCount = 1;

    bool keyHeld = false;
    if (IS_LO(ROLL)) {
        keyHeld = true;
    }

    if (!keyHeld) {
        pageDownHoldCount = 1;
    } else {
        ++pageDownHoldCount;
    }

    if (pageDownHoldCount > 20) {
        pageDownHoldCount = 1;
        return true;
    }

    return false;
}

static void osdRefresh(timeUs_t currentTimeUs)
{
    osdFilterData(currentTimeUs);

#ifdef USE_CMS
    if (IS_RC_MODE_ACTIVE(BOXOSD) && (!cmsInMenu) && !(osdConfig()->osd_failsafe_switch_layout && FLIGHT_MODE(FAILSAFE_MODE))) {
#else
    if (IS_RC_MODE_ACTIVE(BOXOSD) && !(osdConfig()->osd_failsafe_switch_layout && FLIGHT_MODE(FAILSAFE_MODE))) {
#endif
      displayClearScreen(osdDisplayPort);
      armState = ARMING_FLAG(ARMED);
      return;
    }

    bool statsSinglePageCompatible = (osdDisplayPort->rows >= OSD_STATS_SINGLE_PAGE_MIN_ROWS);
    static uint8_t statsCurrentPage = 0;
    static bool statsDisplayed = false;
    static bool statsAutoPagingEnabled = true;
    static bool isThrottleHigh = false;

    // Detect arm/disarm
    if (armState != ARMING_FLAG(ARMED)) {
        if (ARMING_FLAG(ARMED)) {
            // Display the "Arming" screen
            statsDisplayed = false;
            if (!STATE(IN_FLIGHT_EMERG_REARM))
                osdResetStats();

            osdShowArmed();
            uint16_t delay = osdConfig()->arm_screen_display_time;
            if (STATE(IN_FLIGHT_EMERG_REARM))
                delay = 500;
#if defined(USE_SAFE_HOME)
            else if (posControl.safehomeState.distance)
                delay += 3000;
#endif
            osdSetNextRefreshIn(delay);
        } else {
            // Display the "Stats" screen
            statsDisplayed = true;
            statsCurrentPage = 0;
            statsAutoPagingEnabled = osdConfig()->stats_page_auto_swap_time > 0 ? true : false;
            osdShowStats(statsSinglePageCompatible, statsCurrentPage);
            osdSetNextRefreshIn(STATS_SCREEN_DISPLAY_TIME);
            isThrottleHigh = checkStickPosition(THR_HI);
        }

        armState = ARMING_FLAG(ARMED);
    }

    // This block is entered when we're showing the "Splash", "Armed" or "Stats" screens
    if (resumeRefreshAt) {

        // Handle events only when the "Stats" screen is being displayed.
        if (statsDisplayed) {

             // Manual paging stick commands are only applicable to multi-page stats.
             // ******************************
             // For single-page stats, this effectively disables the ability to cancel the
             // automatic paging/updates with the stick commands. So unless stats_page_auto_swap_time
             // is set to 0 or greater than 4 (saved settings display interval is 5 seconds), then
             // "Saved Settings" should display if it is active within the refresh interval.
             // ******************************
             // With multi-page stats, "Saved Settings" could also be missed if the user
             // has canceled automatic paging using the stick commands, because that is only
             // updated when osdShowStats() is called. So, in that case, they would only see
             // the "Saved Settings" message if they happen to manually change pages using the
             // stick commands within the interval the message is displayed.
            bool manualPageUpRequested = false;
            bool manualPageDownRequested = false;
            if (!statsSinglePageCompatible) {
                // These methods ensure the paging stick commands are held for a brief period
                // Otherwise it can result in a race condition where the stats are
                // updated too quickly and can result in partial blanks, etc.
                if (osdIsPageUpStickCommandHeld()) {
                    manualPageUpRequested = true;
                    statsAutoPagingEnabled = false;
                } else if (osdIsPageDownStickCommandHeld()) {
                    manualPageDownRequested = true;
                    statsAutoPagingEnabled = false;
                }
            }

            if (statsAutoPagingEnabled) {
                // Alternate screens for multi-page stats.
                // Also, refreshes screen at swap interval for single-page stats.
                if (OSD_ALTERNATING_CHOICES((osdConfig()->stats_page_auto_swap_time * 1000), 2)) {
                    if (statsCurrentPage == 0) {
                        osdShowStats(statsSinglePageCompatible, statsCurrentPage);
                        statsCurrentPage = 1;
                    }
                } else {
                    if (statsCurrentPage == 1) {
                        osdShowStats(statsSinglePageCompatible, statsCurrentPage);
                        statsCurrentPage = 0;
                    }
                }
            } else {
                // Process manual page change events for multi-page stats.
                if (manualPageUpRequested) {
                    osdShowStats(statsSinglePageCompatible, 1);
                    statsCurrentPage = 1;
                } else if (manualPageDownRequested) {
                    osdShowStats(statsSinglePageCompatible, 0);
                    statsCurrentPage = 0;
                }
            }
        }

        // Handle events when either "Splash", "Armed" or "Stats" screens are displayed.
        if (currentTimeUs > resumeRefreshAt || (OSD_RESUME_UPDATES_STICK_COMMAND && !isThrottleHigh)) {
            // Time elapsed or canceled by stick commands.
            // Exit to normal OSD operation.
            displayClearScreen(osdDisplayPort);
            resumeRefreshAt = 0;
            statsDisplayed = false;
        } else {
            // Continue "Splash", "Armed" or "Stats" screens.
            displayHeartbeat(osdDisplayPort);
            isThrottleHigh = checkStickPosition(THR_HI);
        }

        return;
    }

#ifdef USE_CMS
    if (!displayIsGrabbed(osdDisplayPort)) {
        displayBeginTransaction(osdDisplayPort, DISPLAY_TRANSACTION_OPT_RESET_DRAWING);
        if (fullRedraw) {
            displayClearScreen(osdDisplayPort);
            fullRedraw = false;
        }
        osdDrawNextElement();
        displayHeartbeat(osdDisplayPort);
        displayCommitTransaction(osdDisplayPort);
#ifdef OSD_CALLS_CMS
    } else {
        cmsUpdate(currentTimeUs);
#endif
    }
#endif
}

/*
 * Called periodically by the scheduler
 */
void osdUpdate(timeUs_t currentTimeUs)
{
    static uint32_t counter = 0;

    // don't touch buffers if DMA transaction is in progress
    if (displayIsTransferInProgress(osdDisplayPort)) {
        return;
    }

    if (!osdDisplayIsReady) {
        osdCompleteAsyncInitialization();
        return;
    }

#if defined(OSD_ALTERNATE_LAYOUT_COUNT) && OSD_ALTERNATE_LAYOUT_COUNT > 0
    // Check if the layout has changed. Higher numbered
    // boxes take priority.
    unsigned activeLayout;
    if (layoutOverride >= 0) {
        activeLayout = layoutOverride;
        // Check for timed override, it will go into effect on
        // the next OSD iteration
        if (layoutOverrideUntil > 0 && millis() > layoutOverrideUntil) {
            layoutOverrideUntil = 0;
            layoutOverride = -1;
        }
    } else if (osdConfig()->osd_failsafe_switch_layout && FLIGHT_MODE(FAILSAFE_MODE)) {
        activeLayout = 0;
    } else {
#if OSD_ALTERNATE_LAYOUT_COUNT > 2
        if (IS_RC_MODE_ACTIVE(BOXOSDALT3))
            activeLayout = 3;
        else
#endif
#if OSD_ALTERNATE_LAYOUT_COUNT > 1
        if (IS_RC_MODE_ACTIVE(BOXOSDALT2))
            activeLayout = 2;
        else
#endif
        if (IS_RC_MODE_ACTIVE(BOXOSDALT1))
            activeLayout = 1;
        else
#ifdef USE_PROGRAMMING_FRAMEWORK
        if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_OSD_LAYOUT))
            activeLayout = constrain(logicConditionValuesByType[LOGIC_CONDITION_SET_OSD_LAYOUT], 0, OSD_ALTERNATE_LAYOUT_COUNT);
        else
#endif
            activeLayout = 0;
    }
    if (currentLayout != activeLayout) {
        currentLayout = activeLayout;
        osdStartFullRedraw();
    }
#endif

#define DRAW_FREQ_DENOM     4
#define STATS_FREQ_DENOM    50
    counter++;

    if ((counter % STATS_FREQ_DENOM) == 0 && ARMING_FLAG(ARMED)) {
        osdUpdateStats();
    }

    if ((counter % DRAW_FREQ_DENOM) == 0) {
        // redraw values in buffer
        osdRefresh(currentTimeUs);
    } else {
        // rest of time redraw screen
        displayDrawScreen(osdDisplayPort);
    }

#ifdef USE_CMS
    // do not allow ARM if we are in menu
    if (displayIsGrabbed(osdDisplayPort)) {
        ENABLE_ARMING_FLAG(ARMING_DISABLED_OSD_MENU);
    } else {
        DISABLE_ARMING_FLAG(ARMING_DISABLED_OSD_MENU);
    }
#endif
}

void osdStartFullRedraw(void)
{
    fullRedraw = true;
}

void osdOverrideLayout(int layout, timeMs_t duration)
{
    layoutOverride = constrain(layout, -1, ARRAYLEN(osdLayoutsConfig()->item_pos) - 1);
    if (layoutOverride >= 0 && duration > 0) {
        layoutOverrideUntil = millis() + duration;
    } else {
        layoutOverrideUntil = 0;
    }
}

int osdGetActiveLayout(bool *overridden)
{
    if (overridden) {
        *overridden = layoutOverride >= 0;
    }
    return currentLayout;
}

bool osdItemIsFixed(osd_items_e item)
{
    return item == OSD_CROSSHAIRS ||
        item == OSD_ARTIFICIAL_HORIZON ||
        item == OSD_HORIZON_SIDEBARS;
}

displayPort_t *osdGetDisplayPort(void)
{
    return osdDisplayPort;
}

displayCanvas_t *osdGetDisplayPortCanvas(void)
{
#if defined(USE_CANVAS)
    if (osdDisplayHasCanvas) {
        return &osdCanvas;
    }
#endif
    return NULL;
}

timeMs_t systemMessageCycleTime(unsigned messageCount, const char **messages){
    uint8_t i = 0;
    float factor = 1.0f;
    while (i < messageCount) {
        if ((float)strlen(messages[i]) / 15.0f > factor) {
            factor = (float)strlen(messages[i]) / 15.0f;
        }
        i++;
    }
    return osdConfig()->system_msg_display_time * factor;
}

textAttributes_t osdGetSystemMessage(char *buff, size_t buff_size, bool isCenteredText)
{
    textAttributes_t elemAttr = TEXT_ATTRIBUTES_NONE;

    if (buff != NULL) {
        const char *message = NULL;
        /* WARNING: messageBuf is shared, use accordingly */
        char messageBuf[MAX(SETTING_MAX_NAME_LENGTH, OSD_MESSAGE_LENGTH + 1)];

        /* WARNING: ensure number of messages returned does not exceed messages array size
         * Messages array set 1 larger than maximum expected message count of 6 */
        const char *messages[7];
        unsigned messageCount = 0;

        const char *failsafeInfoMessage = NULL;
        const char *invertedInfoMessage = NULL;

        if (ARMING_FLAG(ARMED)) {
            if (FLIGHT_MODE(FAILSAFE_MODE) || FLIGHT_MODE(NAV_RTH_MODE) || FLIGHT_MODE(NAV_WP_MODE) || navigationIsExecutingAnEmergencyLanding()) {
                /* ADDS MAXIMUM OF 3 MESSAGES TO TOTAL NORMALLY, 5 MESSAGES DURING FAILSAFE */
                if (navGetCurrentStateFlags() & NAV_AUTO_WP_DONE) {
                    messages[messageCount++] = STATE(LANDING_DETECTED) ? OSD_MESSAGE_STR(OSD_MSG_WP_LANDED) : OSD_MESSAGE_STR(OSD_MSG_WP_FINISHED);
                } else if (NAV_Status.state == MW_NAV_STATE_WP_ENROUTE) {
                    // Countdown display for remaining Waypoints
                    char buf[6];
                    osdFormatDistanceSymbol(buf, posControl.wpDistance, 0, 3);
                    tfp_sprintf(messageBuf, "TO WP %u/%u (%s)", getGeoWaypointNumber(posControl.activeWaypointIndex), posControl.geoWaypointCount, buf);
                    messages[messageCount++] = messageBuf;
                } else if (NAV_Status.state == MW_NAV_STATE_HOLD_TIMED) {
                    if (navConfig()->general.waypoint_enforce_altitude && !posControl.wpAltitudeReached) {
                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_ADJUSTING_WP_ALT);
                    } else {
                        // WP hold time countdown in seconds
                        timeMs_t currentTime = millis();
                        int holdTimeRemaining = posControl.waypointList[posControl.activeWaypointIndex].p1 - (int)(MS2S(currentTime - posControl.wpReachedTime));
                        holdTimeRemaining = holdTimeRemaining >= 0 ? holdTimeRemaining : 0;

                        tfp_sprintf(messageBuf, "HOLDING WP FOR %2u S", holdTimeRemaining);

                        messages[messageCount++] = messageBuf;
                    }
                }
                else {
                    const char *navStateMessage = navigationStateMessage();
                    if (navStateMessage) {
                        messages[messageCount++] = navStateMessage;
                    }
                }
#if defined(USE_SAFE_HOME)
                const char *safehomeMessage = divertingToSafehomeMessage();
                if (safehomeMessage) {
                    messages[messageCount++] = safehomeMessage;
                }
#endif
                if (FLIGHT_MODE(FAILSAFE_MODE)) {   // In FS mode while armed
                    if (NAV_Status.state == MW_NAV_STATE_LAND_SETTLE && posControl.landingDelay > 0) {
                        uint16_t remainingHoldSec = MS2S(posControl.landingDelay - millis());
                        tfp_sprintf(messageBuf, "LANDING DELAY: %3u SECONDS", remainingHoldSec);

                        messages[messageCount++] = messageBuf;
                    }

                    const char *failsafePhaseMessage = osdFailsafePhaseMessage();
                    failsafeInfoMessage = osdFailsafeInfoMessage();

                    if (failsafePhaseMessage) {
                        messages[messageCount++] = failsafePhaseMessage;
                    }
                    if (failsafeInfoMessage) {
                        messages[messageCount++] = failsafeInfoMessage;
                    }
                } else if (isWaypointMissionRTHActive()) {
                    // if RTH activated whilst WP mode selected, remind pilot to cancel WP mode to exit RTH
                    messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_WP_RTH_CANCEL);
                }
            } else if (STATE(LANDING_DETECTED)) {
                messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_LANDED);
            } else {
                /* Messages shown only when Failsafe, WP, RTH or Emergency Landing not active and landed state inactive */
                /* ADDS MAXIMUM OF 3 MESSAGES TO TOTAL */
                if (STATE(AIRPLANE)) {      /* ADDS MAXIMUM OF 3 MESSAGES TO TOTAL */
#ifdef USE_FW_AUTOLAND
                    if (canFwLandingBeCancelled()) {
                         messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_MOVE_STICKS);
                    } else
#endif
                    if (navGetCurrentStateFlags() & NAV_CTL_LAUNCH) {
                        messages[messageCount++] = navConfig()->fw.launch_manual_throttle ? OSD_MESSAGE_STR(OSD_MSG_AUTOLAUNCH_MANUAL) :
                                                                                            OSD_MESSAGE_STR(OSD_MSG_AUTOLAUNCH);
                        const char *launchStateMessage = fixedWingLaunchStateMessage();
                        if (launchStateMessage) {
                            messages[messageCount++] = launchStateMessage;
                        }
                    } else if (FLIGHT_MODE(SOARING_MODE)) {
                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_NAV_SOARING);
                    } else if (isFwAutoModeActive(BOXAUTOTUNE)) {
                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_AUTOTUNE);
                        if (FLIGHT_MODE(MANUAL_MODE)) {
                            messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_AUTOTUNE_ACRO);
                        }
                    } else if (isFwAutoModeActive(BOXAUTOTRIM) && !feature(FEATURE_FW_AUTOTRIM)) {
                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_AUTOTRIM);
                    } else if (isFixedWingLevelTrimActive()) {
                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_AUTOLEVEL);
                    }

                    if (IS_RC_MODE_ACTIVE(BOXANGLEHOLD)) {
                        int8_t navAngleHoldAxis = navCheckActiveAngleHoldAxis();
                        if (isAngleHoldLevel()) {
                            messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_ANGLEHOLD_LEVEL);
                        } else if (navAngleHoldAxis == FD_ROLL) {
                            messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_ANGLEHOLD_ROLL);
                        } else if (navAngleHoldAxis == FD_PITCH) {
                            messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_ANGLEHOLD_PITCH);
                        }
                    }
                } else if (STATE(MULTIROTOR)) {     /* ADDS MAXIMUM OF 2 MESSAGES TO TOTAL */
                    if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE)) {
                        if (posControl.cruise.multicopterSpeed >= 50.0f) {
                            char buf[6];
                            osdFormatVelocityStr(buf, posControl.cruise.multicopterSpeed, false, false);
                            tfp_sprintf(messageBuf, "(SPD %s)", buf);
                        } else {
                            strcpy(messageBuf, "(HOLD)");
                        }
                        messages[messageCount++] = messageBuf;
                    } else if (FLIGHT_MODE(HEADFREE_MODE)) {
                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_HEADFREE);
                    }
                    if (FLIGHT_MODE(NAV_ALTHOLD_MODE) && !navigationRequiresAngleMode()) {
                        /* If ALTHOLD is separately enabled for multirotor together with ANGL/HORIZON/ACRO modes
                         * then ANGL/HORIZON/ACRO are indicated by the OSD_FLYMODE field.
                         * In this case indicate ALTHOLD is active via a system message */

                        messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_ALTITUDE_HOLD);
                    }
                }
            }
        } else if (ARMING_FLAG(ARMING_DISABLED_ALL_FLAGS)) {    /* ADDS MAXIMUM OF 2 MESSAGES TO TOTAL */
            unsigned invalidIndex;

            // Check if we're unable to arm for some reason
            if (ARMING_FLAG(ARMING_DISABLED_INVALID_SETTING) && !settingsValidate(&invalidIndex)) {
                const setting_t *setting = settingGet(invalidIndex);
                settingGetName(setting, messageBuf);
                for (int ii = 0; messageBuf[ii]; ii++) {
                    messageBuf[ii] = sl_toupper(messageBuf[ii]);
                }
                invertedInfoMessage = messageBuf;
                messages[messageCount++] = invertedInfoMessage;

                invertedInfoMessage = OSD_MESSAGE_STR(OSD_MSG_INVALID_SETTING);
                messages[messageCount++] = invertedInfoMessage;
            } else {
                invertedInfoMessage = OSD_MESSAGE_STR(OSD_MSG_UNABLE_ARM);
                messages[messageCount++] = invertedInfoMessage;
                // Show the reason for not arming
                messages[messageCount++] = osdArmingDisabledReasonMessage();
            }
        } else if (!ARMING_FLAG(ARMED)) {
            if (isWaypointListValid()) {
                messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_WP_MISSION_LOADED);
            }
        }

        /* Messages that are shown regardless of Arming state */
        /* ADDS MAXIMUM OF 2 MESSAGES TO TOTAL NORMALLY, 1 MESSAGE DURING FAILSAFE */
        if (posControl.flags.wpMissionPlannerActive && !FLIGHT_MODE(FAILSAFE_MODE)) {
            messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_MISSION_PLANNER);
        }

        // The following has been commented out as it will be added in #9688
        // uint16_t rearmMs = (emergInflightRearmEnabled()) ? emergencyInFlightRearmTimeMS() : 0;

        if (savingSettings == true) {
           messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_SAVING_SETTNGS);
        /*} else if (rearmMs > 0) { // Show rearming time if settings not actively being saved. Ignore the settings saved message if rearm available.
            char emReArmMsg[23];
            tfp_sprintf(emReArmMsg, "** REARM PERIOD: ");
            tfp_sprintf(emReArmMsg + strlen(emReArmMsg), "%02d", (uint8_t)MS2S(rearmMs));
            strcat(emReArmMsg, " **\0");
            messages[messageCount++] = OSD_MESSAGE_STR(emReArmMsg);*/
        } else if (notify_settings_saved > 0) {
            if (millis() > notify_settings_saved) {
                notify_settings_saved = 0;
            } else {
                messages[messageCount++] = OSD_MESSAGE_STR(OSD_MSG_SETTINGS_SAVED);
            }
        }

        if (messageCount > 0) {
            message = messages[OSD_ALTERNATING_CHOICES(systemMessageCycleTime(messageCount, messages), messageCount)];
            if (message == failsafeInfoMessage) {
                // failsafeInfoMessage is not useful for recovering
                // a lost model, but might help avoiding a crash.
                // Blink to grab user attention.
                TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
            } else if (message == invertedInfoMessage) {
                TEXT_ATTRIBUTES_ADD_INVERTED(elemAttr);
            }
            // We're showing either failsafePhaseMessage or
            // navStateMessage. Don't BLINK here since
            // having this text available might be crucial
            // during a lost aircraft recovery and blinking
            // will cause it to be missing from some frames.
        }

        osdFormatMessage(buff, buff_size, message, isCenteredText);
    }
    return elemAttr;
}

void osdResetWarningFlags(void)
{
    osdWarningsFlags = 0;
}

static bool osdCheckWarning(bool condition, uint8_t warningFlag, uint8_t *warningsCount)
{
#define WARNING_REDISPLAY_DURATION 5000;    // milliseconds

    const timeMs_t currentTimeMs = millis();
    static timeMs_t warningDisplayStartTime = 0;
    static timeMs_t redisplayStartTimeMs = 0;
    static uint16_t osdWarningTimerDuration;
    static uint8_t newWarningFlags;

    if (condition) {    // condition required to trigger warning
        if (!(osdWarningsFlags & warningFlag)) {
            osdWarningsFlags |= warningFlag;
            newWarningFlags |= warningFlag;
            redisplayStartTimeMs = 0;
        }
#ifdef USE_DEV_TOOLS
        if (systemConfig()->groundTestMode) {
            return true;
        }
#endif
        /* Warnings displayed in full for set time before shrinking down to alert symbol with warning count only.
         * All current warnings then redisplayed for 5s on 30s rolling cycle.
         * New warnings dislayed individually for 10s */
        if (currentTimeMs > redisplayStartTimeMs) {
            warningDisplayStartTime = currentTimeMs;
            osdWarningTimerDuration = newWarningFlags ? 10000 : WARNING_REDISPLAY_DURATION;
            redisplayStartTimeMs = currentTimeMs + osdWarningTimerDuration + 30000;
        }

        if (currentTimeMs - warningDisplayStartTime < osdWarningTimerDuration) {
            return (newWarningFlags & warningFlag) || osdWarningTimerDuration == WARNING_REDISPLAY_DURATION;
        } else {
            newWarningFlags = 0;
        }
        *warningsCount += 1;
    } else if (osdWarningsFlags & warningFlag) {
        osdWarningsFlags &= ~warningFlag;
    }

    return false;
}

static textAttributes_t osdGetMultiFunctionMessage(char *buff)
{
    /* Message length limit 10 char max */

    textAttributes_t elemAttr = TEXT_ATTRIBUTES_NONE;
    static uint8_t warningsCount;
    const char *message = NULL;

#ifdef USE_MULTI_FUNCTIONS
    /* --- FUNCTIONS --- */
    multi_function_e selectedFunction = multiFunctionSelection();

    if (selectedFunction) {
        multi_function_e activeFunction = selectedFunction;

        switch (selectedFunction) {
        case MULTI_FUNC_NONE:
        case MULTI_FUNC_1:
            message = warningsCount ? "WARNINGS !" : "0 WARNINGS";
            break;
        case MULTI_FUNC_2:
            message = posControl.flags.manualEmergLandActive ? "ABORT LAND" : "EMERG LAND";
            break;
        case MULTI_FUNC_3:
#if defined(USE_SAFE_HOME)
            if (navConfig()->general.flags.safehome_usage_mode != SAFEHOME_USAGE_OFF) {
                message = MULTI_FUNC_FLAG(MF_SUSPEND_SAFEHOMES) ? "USE SFHOME" : "SUS SFHOME";
                break;
            }
#endif
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_4:
            if (navConfig()->general.flags.rth_trackback_mode != RTH_TRACKBACK_OFF) {
                message = MULTI_FUNC_FLAG(MF_SUSPEND_TRACKBACK) ? "USE TKBACK" : "SUS TKBACK";
                break;
            }
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_5:
#ifdef USE_DSHOT
            if (STATE(MULTIROTOR)) {
                message = MULTI_FUNC_FLAG(MF_TURTLE_MODE) ? "END TURTLE" : "USE TURTLE";
                break;
            }
#endif
            activeFunction++;
            FALLTHROUGH;
        case MULTI_FUNC_6:
            message = ARMING_FLAG(ARMED) ? "NOW ARMED " : "EMERG ARM ";
            break;
        case MULTI_FUNC_END:
            break;
        }

        if (activeFunction != selectedFunction) {
            setMultifunctionSelection(activeFunction);
        }

        strcpy(buff, message);

        if (isNextMultifunctionItemAvailable()) {
            // provides feedback indicating when a new selection command has been received by flight controller
            buff[9] = '>';
        }

        return elemAttr;
    }
#endif  // MULTIFUNCTION - functions only, warnings always defined

    /* --- WARNINGS --- */
    const char *messages[7];
    uint8_t messageCount = 0;
    bool warningCondition = false;
    warningsCount = 0;
    uint8_t warningFlagID = 1;

    // Low Battery
    const batteryState_e batteryState = getBatteryState();
    warningCondition = batteryState == BATTERY_CRITICAL || batteryState == BATTERY_WARNING;
    if (osdCheckWarning(warningCondition, warningFlagID, &warningsCount)) {
        messages[messageCount++] = batteryState == BATTERY_CRITICAL ? "BATT EMPTY" : "BATT LOW !";
    }

#if defined(USE_GPS)
    // GPS Fix and Failure
    if (feature(FEATURE_GPS)) {
        if (osdCheckWarning(!STATE(GPS_FIX), warningFlagID <<= 1, &warningsCount)) {
            bool gpsFailed = getHwGPSStatus() == HW_SENSOR_UNAVAILABLE;
            messages[messageCount++] = gpsFailed ? "GPS FAILED" : "NO GPS FIX";
        }
    }

    // RTH sanity (warning if RTH heads 200m further away from home than closest point)
    warningCondition = NAV_Status.state == MW_NAV_STATE_RTH_ENROUTE && !posControl.flags.rthTrackbackActive &&
                       (posControl.homeDistance - posControl.rthSanityChecker.minimalDistanceToHome) > 20000;
    if (osdCheckWarning(warningCondition, warningFlagID <<= 1, &warningsCount)) {
        messages[messageCount++] = "RTH SANITY";
    }

    // Altitude sanity (warning if significant mismatch between estimated and GPS altitude)
    if (osdCheckWarning(posControl.flags.gpsCfEstimatedAltitudeMismatch, warningFlagID <<= 1, &warningsCount)) {
        messages[messageCount++] = "ALT SANITY";
    }
#endif

#if defined(USE_MAG)
    // Magnetometer failure
    if (requestedSensors[SENSOR_INDEX_MAG] != MAG_NONE) {
        hardwareSensorStatus_e magStatus = getHwCompassStatus();
        if (osdCheckWarning(magStatus == HW_SENSOR_UNAVAILABLE || magStatus == HW_SENSOR_UNHEALTHY, warningFlagID <<= 1, &warningsCount)) {
            messages[messageCount++] = "MAG FAILED";
        }
    }
#endif
    // Vibration levels   TODO - needs better vibration measurement to be useful
    // const float vibrationLevel = accGetVibrationLevel();
    // warningCondition = vibrationLevel > 1.5f;
    // if (osdCheckWarning(warningCondition, warningFlagID <<= 1, &warningsCount)) {
        // messages[messageCount++] = vibrationLevel > 2.5f ? "BAD VIBRTN" : "VIBRATION!";
    // }

#ifdef USE_DEV_TOOLS
    if (osdCheckWarning(systemConfig()->groundTestMode, warningFlagID <<= 1, &warningsCount)) {
        messages[messageCount++] = "GRD TEST !";
    }
#endif

    if (messageCount) {
        message = messages[OSD_ALTERNATING_CHOICES(1000, messageCount)];    // display each warning on 1s cycle
        strcpy(buff, message);
        TEXT_ATTRIBUTES_ADD_BLINK(elemAttr);
    } else if (warningsCount) {
        buff[0] = SYM_ALERT;
        tfp_sprintf(buff + 1, "%u        ", warningsCount);
    }

    return elemAttr;
}

#endif // OSD

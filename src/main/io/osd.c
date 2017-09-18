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

#include "platform.h"

#ifdef OSD

#include "build/debug.h"
#include "build/version.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_osd.h"

#include "common/axis.h"
#include "common/printf.h"
#include "common/string_light.h"
#include "common/time.h"
#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/display.h"
#include "drivers/max7456_symbols.h"
#include "drivers/time.h"

#include "io/flashfs.h"
#include "io/gps.h"
#include "io/osd.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"
#include "flight/pid.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/battery.h"
#include "sensors/sensors.h"
#include "sensors/pitotmeter.h"

#ifdef USE_HARDWARE_REVISION_DETECTION
#include "hardware_revision.h"
#endif

#define VIDEO_BUFFER_CHARS_PAL    480

// Character coordinate and attributes
#define OSD_POS(x,y)  (x | (y << 5))
#define OSD_X(x)      (x & 0x001F)
#define OSD_Y(x)      ((x >> 5) & 0x001F)

// Things in both OSD and CMS

bool blinkState = true;

static timeUs_t flyTime = 0;
static uint8_t statRssi;

typedef struct statistic_s {
    uint16_t max_speed;
    int16_t min_voltage; // /10
    int16_t max_current; // /10
    int16_t min_rssi;
    int32_t max_altitude;
    uint16_t max_distance;
} statistic_t;

static statistic_t stats;

uint32_t resumeRefreshAt = 0;
#define REFRESH_1S    (1000*1000)

static bool fullRedraw = false;

static uint8_t armState;

static displayPort_t *osdDisplayPort;

#define AH_MAX_PITCH 200 // Specify maximum AHI pitch value displayed. Default 200 = 20.0 degrees
#define AH_MAX_ROLL 400  // Specify maximum AHI roll value displayed. Default 400 = 40.0 degrees
#define AH_SYMBOL_COUNT 9
#define AH_SIDEBAR_WIDTH_POS 7
#define AH_SIDEBAR_HEIGHT_POS 3

PG_REGISTER_WITH_RESET_FN(osdConfig_t, osdConfig, PG_OSD_CONFIG, 0);

/**
 * Converts altitude/distance based on the current unit system (cm or 1/100th of ft).
 * @param alt Raw altitude/distance (i.e. as taken from baro.BaroAlt)
 */
static int32_t osdConvertDistanceToUnit(int32_t dist)
{
    switch (osdConfig()->units) {
    case OSD_UNIT_IMPERIAL:
        return (dist * 328) / 100; // Convert to feet / 100
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_METRIC:
        return dist;               // Already in meter / 100
    }
    // Unreachable
    return -1;
}

/**
 * Converts altitude/distance into a string based on the current unit system.
 * @param alt Raw altitude/distance (i.e. as taken from baro.BaroAlt in centimeters)
 */
static void osdFormatDistanceStr(char* buff, int32_t dist)
{
    int32_t dist_abs = abs(osdConvertDistanceToUnit(dist));

    switch (osdConfig()->units) {
    case OSD_UNIT_IMPERIAL:
        if (dist < 0) {
            tfp_sprintf(buff, "-%d%c ", dist_abs / 100, SYM_FT);
        } else {
            tfp_sprintf(buff, "%d%c ", dist_abs / 100, SYM_FT);
        }
        break;
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_METRIC:
        if (dist < 0) {
            tfp_sprintf(buff, "-%d.%01d%c ", dist_abs / 100, (dist_abs % 100) / 10, SYM_M);
        } else {
            if (dist < 10000) { // less than 100m
                tfp_sprintf(buff, "%d.%01d%c ", dist_abs / 100, (dist_abs % 100) / 10, SYM_M);
            } else {
                tfp_sprintf(buff, "%d%c ", dist_abs / 100, SYM_M);
            }
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
    switch (osdConfig()->units) {
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_IMPERIAL:
        return (vel * 224) / 10000; // Convert to mph
    case OSD_UNIT_METRIC:
        return (vel * 36) / 1000;   // Convert to kmh
    }
    // Unreachable
    return -1;
}

/**
 * Converts velocity into a string based on the current unit system.
 * @param alt Raw velocity (i.e. as taken from gpsSol.groundSpeed in centimeters/seconds)
 */
static void osdFormatVelocityStr(char* buff, int32_t vel)
{
    switch (osdConfig()->units) {
    case OSD_UNIT_UK:
        FALLTHROUGH;
    case OSD_UNIT_IMPERIAL:
        tfp_sprintf(buff, "%2d%c", osdConvertVelocityToUnit(vel), SYM_MPH);
        break;
    case OSD_UNIT_METRIC:
        tfp_sprintf(buff, "%3d%c", osdConvertVelocityToUnit(vel), SYM_KMH);
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
    tfp_sprintf(buff + 1, "%02d:%02d", value / 60, value % 60);
}

static inline void osdFormatOnTime(char *buff)
{
    osdFormatTime(buff, micros() / 1000000, SYM_ON_M, SYM_ON_H);
}

static inline void osdFormatFlyTime(char *buff)
{
    osdFormatTime(buff, flyTime / 1000000, SYM_FLY_M, SYM_FLY_H);
}

static void osdFormatCoordinate(char *buff, char sym, int32_t val)
{
    buff[0] = sym;
    char wholeDegreeString[5];
    tfp_sprintf(wholeDegreeString, "%d", val / GPS_DEGREES_DIVIDER);
    char wholeUnshifted[32];
    tfp_sprintf(wholeUnshifted, "%d", val);
    tfp_sprintf(buff + 1, "%s.%s", wholeDegreeString, wholeUnshifted + strlen(wholeDegreeString));
}

static bool osdDrawSingleElement(uint8_t item)
{
    if (!VISIBLE(osdConfig()->item_pos[item]) || BLINK(osdConfig()->item_pos[item])) {
        return false;
    }

    uint8_t elemPosX = OSD_X(osdConfig()->item_pos[item]);
    uint8_t elemPosY = OSD_Y(osdConfig()->item_pos[item]);
    char buff[32];

    switch (item) {
    case OSD_RSSI_VALUE:
        {
            uint16_t osdRssi = rssi * 100 / 1024; // change range
            if (osdRssi >= 100)
                osdRssi = 99;

            buff[0] = SYM_RSSI;
            tfp_sprintf(buff + 1, "%2d", osdRssi);
            break;
        }

    case OSD_MAIN_BATT_VOLTAGE:
        {
            uint8_t p = calculateBatteryPercentage();
            p = (100 - p) / 16.6;
            buff[0] = SYM_BATT_FULL + p;
            tfp_sprintf(buff + 1, "%d.%1dV ", vbat / 10, vbat % 10);
            break;
        }

    case OSD_CURRENT_DRAW:
        buff[0] = SYM_AMP;
        tfp_sprintf(buff + 1, "%d.%02d ", abs(amperage) / 100, abs(amperage) % 100);
        break;

    case OSD_MAH_DRAWN:
        buff[0] = SYM_MAH;
        tfp_sprintf(buff + 1, "%d", abs(mAhDrawn));
        break;

#ifdef GPS
    case OSD_GPS_SATS:
        buff[0] = SYM_SAT_L;
        buff[1] = SYM_SAT_R;
        tfp_sprintf(buff + 2, "%2d", gpsSol.numSat);
        break;

    case OSD_GPS_SPEED:
        osdFormatVelocityStr(buff, gpsSol.groundSpeed);
        break;

    case OSD_GPS_LAT:
        osdFormatCoordinate(buff, SYM_LAT, gpsSol.llh.lat);
        break;

    case OSD_GPS_LON:
        osdFormatCoordinate(buff, SYM_LON, gpsSol.llh.lon);
        break;

    case OSD_HOME_DIR:
        {
            int16_t h = GPS_directionToHome - DECIDEGREES_TO_DEGREES(attitude.values.yaw);

            if (h < 0)
                h += 360;
            if (h >= 360)
                h -= 360;

            h = h*2/45;

            buff[0] = SYM_ARROW_UP + h;
            buff[1] = 0;
            break;
        }

    case OSD_HOME_DIST:
        buff[0] = SYM_HOME_DIST;
        osdFormatDistanceStr(&buff[1], GPS_distanceToHome * 100);
        break;

    case OSD_HEADING:
        {
            int16_t h = DECIDEGREES_TO_DEGREES(attitude.values.yaw);
            if (h < 0) h+=360;

            buff[0] = SYM_HEADING;
            tfp_sprintf(&buff[1], "%3d%c", h , SYM_DEGREES);
            break;
        }
#endif // GPS

    case OSD_ALTITUDE:
        {
            buff[0] = SYM_ALT;
#ifdef NAV
            osdFormatDistanceStr(&buff[1], getEstimatedActualPosition(Z));
#else
            osdFormatDistanceStr(&buff[1], baro.BaroAlt));
#endif
            break;
        }

    case OSD_ONTIME:
        {
            osdFormatOnTime(buff);
            break;
        }

    case OSD_FLYTIME:
        {
            osdFormatFlyTime(buff);
            break;
        }

    case OSD_ONTIME_FLYTIME:
        {
            if (ARMING_FLAG(ARMED)) {
                osdFormatFlyTime(buff);
            } else {
                osdFormatOnTime(buff);
            }
            break;
        }

    case OSD_FLYMODE:
        {
            char *p = "ACRO";

            if (isAirmodeActive()) {
                p = "AIR ";
            }

            if (FLIGHT_MODE(PASSTHRU_MODE))
                p = "PASS";
            else if (FLIGHT_MODE(FAILSAFE_MODE)) {
                p = "!FS!";
                // See failsafe.h for each phase explanation
                switch (failsafePhase()) {
                    case FAILSAFE_RX_LOSS_IDLE:
                        p = "F:RX"; // RX to indicate that failsafe is waiting
                                   // for the RX to become available
                        break;
#ifdef NAV
                    case FAILSAFE_RETURN_TO_HOME:
                        p = "F:RT"; // RT for RETURN
                        break;
#endif
                    case FAILSAFE_LANDING:
                        p = "F:LA"; // LA for LANDING
                        break;
                    case FAILSAFE_LANDED:
                        p = "F:DR"; // DR for DROP
                        break;
                    case FAILSAFE_RX_LOSS_MONITORING:
                        p = "F:WA"; // WA for WAITING
                        break;
                    case FAILSAFE_IDLE:
                    case FAILSAFE_RX_LOSS_DETECTED:
                    case FAILSAFE_RX_LOSS_RECOVERED:
                        /* Failsafe not active or phase is very brief.
                         * Nothing interesting to show.
                         */
                        break;
                }
            } else if (FLIGHT_MODE(HEADFREE_MODE))
                p = "!HF!";
            else if (FLIGHT_MODE(NAV_RTH_MODE))
                p = "RTL ";
            else if (FLIGHT_MODE(NAV_POSHOLD_MODE))
                p = " PH ";
            else if (FLIGHT_MODE(NAV_WP_MODE))
                p = " WP ";
            else if (FLIGHT_MODE(NAV_ALTHOLD_MODE))
                p = " AH ";
            else if (FLIGHT_MODE(ANGLE_MODE))
                p = "STAB";
            else if (FLIGHT_MODE(HORIZON_MODE))
                p = "HOR ";

            displayWrite(osdDisplayPort, elemPosX, elemPosY, p);
            return true;
        }

    case OSD_CRAFT_NAME:
        if (strlen(systemConfig()->name) == 0)
            strcpy(buff, "CRAFT_NAME");
        else {
            for (int i = 0; i < MAX_NAME_LENGTH; i++) {
                buff[i] = sl_toupper((unsigned char)systemConfig()->name[i]);
                if (systemConfig()->name[i] == 0)
                    break;
            }
        }
        break;

    case OSD_THROTTLE_POS:
        buff[0] = SYM_THR;
        buff[1] = SYM_THR1;
        tfp_sprintf(buff + 2, "%3d", (constrain(rcData[THROTTLE], PWM_RANGE_MIN, PWM_RANGE_MAX) - PWM_RANGE_MIN) * 100 / (PWM_RANGE_MAX - PWM_RANGE_MIN));
        break;

#ifdef VTX
    case OSD_VTX_CHANNEL:
        tfp_sprintf(buff, "CH:%2d", current_vtx_channel % CHANNELS_PER_BAND + 1);
        break;
#endif // VTX

    case OSD_CROSSHAIRS:
        elemPosX = 14 - 1; // Offset for 1 char to the left
        elemPosY = 6;
        if (displayScreenSize(osdDisplayPort) == VIDEO_BUFFER_CHARS_PAL) {
            ++elemPosY;
        }
        buff[0] = SYM_AH_CENTER_LINE;
        buff[1] = SYM_AH_CENTER;
        buff[2] = SYM_AH_CENTER_LINE_RIGHT;
        buff[3] = 0;
        break;

    case OSD_ARTIFICIAL_HORIZON:
        {
            elemPosX = 14;
            elemPosY = 6 - 4; // Top center of the AH area

            int rollAngle = constrain(attitude.values.roll, -AH_MAX_ROLL, AH_MAX_ROLL);
            int pitchAngle = constrain(attitude.values.pitch, -AH_MAX_PITCH, AH_MAX_PITCH);

            if (displayScreenSize(osdDisplayPort) == VIDEO_BUFFER_CHARS_PAL) {
                ++elemPosY;
            }

            // Convert pitchAngle to y compensation value
            pitchAngle = ((pitchAngle * 25) / AH_MAX_PITCH) - 41; // 41 = 4 * 9 + 5

            for (int x = -4; x <= 4; x++) {
                // clear the y area before writing the new horizon character
                for (int y = 0; y <= 8; y++) {
                    displayWriteChar(osdDisplayPort, elemPosX + x, elemPosY + y, SYM_BLANK);
                }
                const int y = (-rollAngle * x) / 64 - pitchAngle;
                if (y >= 0 && y <= 80) {
                    displayWriteChar(osdDisplayPort, elemPosX + x, elemPosY + (y / AH_SYMBOL_COUNT), (SYM_AH_BAR9_0 + (y % AH_SYMBOL_COUNT)));
                }
            }

            osdDrawSingleElement(OSD_HORIZON_SIDEBARS);
            osdDrawSingleElement(OSD_CROSSHAIRS);

            return true;
        }

    case OSD_HORIZON_SIDEBARS:
        {
            elemPosX = 14;
            elemPosY = 6;

            if (displayScreenSize(osdDisplayPort) == VIDEO_BUFFER_CHARS_PAL) {
                ++elemPosY;
            }

            // Draw AH sides
            const int8_t hudwidth = AH_SIDEBAR_WIDTH_POS;
            const int8_t hudheight = AH_SIDEBAR_HEIGHT_POS;
            for (int  y = -hudheight; y <= hudheight; y++) {
                displayWriteChar(osdDisplayPort, elemPosX - hudwidth, elemPosY + y, SYM_AH_DECORATION);
                displayWriteChar(osdDisplayPort, elemPosX + hudwidth, elemPosY + y, SYM_AH_DECORATION);
            }

            // AH level indicators
            displayWriteChar(osdDisplayPort, elemPosX - hudwidth + 1, elemPosY, SYM_AH_LEFT);
            displayWriteChar(osdDisplayPort, elemPosX + hudwidth - 1, elemPosY, SYM_AH_RIGHT);

            return true;
        }

#if defined(BARO) || defined(GPS)
    case OSD_VARIO:
        {
            int16_t v = getEstimatedActualVelocity(Z) / 50; //50cm = 1 arrow
            uint8_t vchars[] = {SYM_BLANK, SYM_BLANK, SYM_BLANK, SYM_BLANK, SYM_BLANK};

            if (v >= 6)
                vchars[0] = SYM_VARIO_UP_2A;
            else if (v == 5)
                vchars[0] = SYM_VARIO_UP_1A;
            if (v >=4)
                vchars[1] = SYM_VARIO_UP_2A;
            else if (v == 3)
                vchars[1] = SYM_VARIO_UP_1A;
            if (v >=2)
                vchars[2] = SYM_VARIO_UP_2A;
            else if (v == 1)
                vchars[2] = SYM_VARIO_UP_1A;
            if (v <= -2)
                vchars[2] = SYM_VARIO_DOWN_2A;
            else if (v == -1)
                vchars[2] = SYM_VARIO_DOWN_1A;
            if (v <= -4)
                vchars[3] = SYM_VARIO_DOWN_2A;
            else if (v == -3)
                vchars[3] = SYM_VARIO_DOWN_1A;
            if (v <= -6)
                vchars[4] = SYM_VARIO_DOWN_2A;
            else if (v == -5)
                vchars[4] = SYM_VARIO_DOWN_1A;

            displayWriteChar(osdDisplayPort, elemPosX, elemPosY, vchars[0]);
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY+1, vchars[1]);
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY+2, vchars[2]);
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY+3, vchars[3]);
            displayWriteChar(osdDisplayPort, elemPosX, elemPosY+4, vchars[4]);
            return true;
        }

    case OSD_VARIO_NUM:
        {
            int16_t value = getEstimatedActualVelocity(Z) / 10; //limit precision to 10cm

            tfp_sprintf(buff, "%c%d.%01d%c ", value < 0 ? '-' : ' ', abs(value / 10), abs((value % 10)), SYM_MS);
            break;
        }
#endif

    case OSD_ROLL_PIDS:
        {
            tfp_sprintf(buff, "ROL %3d %3d %3d", pidBank()->pid[PID_ROLL].P, pidBank()->pid[PID_ROLL].I, pidBank()->pid[PID_ROLL].D);
            break;
        }

    case OSD_PITCH_PIDS:
        {
            tfp_sprintf(buff, "PIT %3d %3d %3d", pidBank()->pid[PID_PITCH].P, pidBank()->pid[PID_PITCH].I, pidBank()->pid[PID_PITCH].D);
            break;
        }

    case OSD_YAW_PIDS:
        {
            tfp_sprintf(buff, "YAW %3d %3d %3d", pidBank()->pid[PID_YAW].P, pidBank()->pid[PID_YAW].I, pidBank()->pid[PID_YAW].D);
            break;
        }

    case OSD_POWER:
        {
            tfp_sprintf(buff, "%dW", amperage * vbat / 1000);
            break;
        }

    case OSD_AIR_SPEED:
        {
        #ifdef PITOT
            osdFormatVelocityStr(buff, pitot.airSpeed);
        #else
            return false;
        #endif
            break;
        }

    case OSD_RTC_TIME:
        {
            // RTC not configured will show 00:00
            dateTime_t dateTime;
            if (rtcGetDateTime(&dateTime)) {
                dateTimeUTCToLocal(&dateTime, &dateTime);
            }
            buff[0] = SYM_CLOCK;
            tfp_sprintf(buff + 1, "%02u:%02u", dateTime.hours, dateTime.minutes);
            break;
        }

    default:
        return false;
    }

    displayWrite(osdDisplayPort, elemPosX, elemPosY, buff);
    return true;
}

static uint8_t osdIncElementIndex(uint8_t elementIndex)
{
    ++elementIndex;
    if (!sensors(SENSOR_ACC)) {
        if (elementIndex == OSD_CROSSHAIRS) {
            elementIndex = OSD_ONTIME;
        }
    }
    if (!feature(FEATURE_CURRENT_METER)) {
        if (elementIndex == OSD_CURRENT_DRAW) {
            elementIndex = OSD_GPS_SPEED;
        }

    }
    if (!feature(FEATURE_GPS)) {
        if (elementIndex == OSD_GPS_SPEED) {
            elementIndex = OSD_ALTITUDE;
        }
        if (elementIndex == OSD_GPS_LON) {
            elementIndex = OSD_VARIO;
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
    while (osdDrawSingleElement(elementIndex) == false) {
        elementIndex = osdIncElementIndex(elementIndex);
    }
    elementIndex = osdIncElementIndex(elementIndex);
}

void pgResetFn_osdConfig(osdConfig_t *osdConfig)
{
    osdConfig->item_pos[OSD_ALTITUDE] = OSD_POS(1, 0) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_MAIN_BATT_VOLTAGE] = OSD_POS(12, 0) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_RSSI_VALUE] = OSD_POS(23, 0) | VISIBLE_FLAG;
    //line 2
    osdConfig->item_pos[OSD_HOME_DIST] = OSD_POS(1, 1);
    osdConfig->item_pos[OSD_HEADING] = OSD_POS(12, 1);
    osdConfig->item_pos[OSD_GPS_SPEED] = OSD_POS(23, 1);

    osdConfig->item_pos[OSD_THROTTLE_POS] = OSD_POS(1, 2) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_CURRENT_DRAW] = OSD_POS(1, 3) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_MAH_DRAWN] = OSD_POS(1, 4) | VISIBLE_FLAG;

    osdConfig->item_pos[OSD_VARIO] = OSD_POS(22,5);
    osdConfig->item_pos[OSD_VARIO_NUM] = OSD_POS(23,7);
    osdConfig->item_pos[OSD_HOME_DIR] = OSD_POS(14, 11);
    osdConfig->item_pos[OSD_ARTIFICIAL_HORIZON] = OSD_POS(8, 6) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_HORIZON_SIDEBARS] = OSD_POS(8, 6) | VISIBLE_FLAG;

    osdConfig->item_pos[OSD_CRAFT_NAME] = OSD_POS(20, 2);
    osdConfig->item_pos[OSD_VTX_CHANNEL] = OSD_POS(8, 6);

    osdConfig->item_pos[OSD_ONTIME] = OSD_POS(23, 8);
    osdConfig->item_pos[OSD_FLYTIME] = OSD_POS(23, 9);
    osdConfig->item_pos[OSD_ONTIME_FLYTIME] = OSD_POS(23, 11) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_RTC_TIME] = OSD_POS(23, 12);

    osdConfig->item_pos[OSD_GPS_SATS] = OSD_POS(0, 11) | VISIBLE_FLAG;

    osdConfig->item_pos[OSD_GPS_LAT] = OSD_POS(0, 12);
    osdConfig->item_pos[OSD_FLYMODE] = OSD_POS(12, 12) | VISIBLE_FLAG;
    osdConfig->item_pos[OSD_GPS_LON] = OSD_POS(18, 12);

    osdConfig->item_pos[OSD_ROLL_PIDS] = OSD_POS(2, 10);
    osdConfig->item_pos[OSD_PITCH_PIDS] = OSD_POS(2, 11);
    osdConfig->item_pos[OSD_YAW_PIDS] = OSD_POS(2, 12);
    osdConfig->item_pos[OSD_POWER] = OSD_POS(15, 1);

    osdConfig->item_pos[OSD_AIR_SPEED] = OSD_POS(3, 5);

    osdConfig->rssi_alarm = 20;
    osdConfig->cap_alarm = 2200;
    osdConfig->time_alarm = 10; // in minutes
    osdConfig->alt_alarm = 100; // meters or feet depend on configuration

    osdConfig->video_system = 0;
}

void osdInit(displayPort_t *osdDisplayPortToUse)
{
    if (!osdDisplayPortToUse)
        return;

    BUILD_BUG_ON(OSD_POS_MAX != OSD_POS(31,31));

    osdDisplayPort = osdDisplayPortToUse;

#ifdef CMS
    cmsDisplayPortRegister(osdDisplayPort);
#endif

    armState = ARMING_FLAG(ARMED);

    displayClearScreen(osdDisplayPort);

    char string_buffer[30];
    tfp_sprintf(string_buffer, "INAV VERSION: %s", FC_VERSION_STRING);
    displayWrite(osdDisplayPort, 5, 6, string_buffer);
#ifdef CMS
    displayWrite(osdDisplayPort, 7, 7,  CMS_STARTUP_HELP_TEXT1);
    displayWrite(osdDisplayPort, 11, 8, CMS_STARTUP_HELP_TEXT2);
    displayWrite(osdDisplayPort, 11, 9, CMS_STARTUP_HELP_TEXT3);
#endif

    displayResync(osdDisplayPort);

    resumeRefreshAt = micros() + (4 * REFRESH_1S);
}

void osdUpdateAlarms(void)
{
    // This is overdone?
    // uint16_t *itemPos = osdConfig()->item_pos;

#ifdef NAV
    int32_t alt = osdConvertDistanceToUnit(getEstimatedActualPosition(Z)) / 100;
#else
    int32_t alt = osdConvertDistanceToUnit(baro.BaroAlt) / 100;
#endif
    statRssi = rssi * 100 / 1024;

    if (statRssi < osdConfig()->rssi_alarm)
        osdConfigMutable()->item_pos[OSD_RSSI_VALUE] |= BLINK_FLAG;
    else
        osdConfigMutable()->item_pos[OSD_RSSI_VALUE] &= ~BLINK_FLAG;

    if (vbat <= (batteryWarningVoltage - 1))
        osdConfigMutable()->item_pos[OSD_MAIN_BATT_VOLTAGE] |= BLINK_FLAG;
    else
        osdConfigMutable()->item_pos[OSD_MAIN_BATT_VOLTAGE] &= ~BLINK_FLAG;

    if (STATE(GPS_FIX) == 0)
        osdConfigMutable()->item_pos[OSD_GPS_SATS] |= BLINK_FLAG;
    else
        osdConfigMutable()->item_pos[OSD_GPS_SATS] &= ~BLINK_FLAG;

    if ((flyTime / 1000000) / 60 >= osdConfig()->time_alarm && ARMING_FLAG(ARMED))
        osdConfigMutable()->item_pos[OSD_FLYTIME] |= BLINK_FLAG;
    else
        osdConfigMutable()->item_pos[OSD_FLYTIME] &= ~BLINK_FLAG;

    if (mAhDrawn >= osdConfig()->cap_alarm)
        osdConfigMutable()->item_pos[OSD_MAH_DRAWN] |= BLINK_FLAG;
    else
        osdConfigMutable()->item_pos[OSD_MAH_DRAWN] &= ~BLINK_FLAG;

    if (alt >= osdConfig()->alt_alarm)
        osdConfigMutable()->item_pos[OSD_ALTITUDE] |= BLINK_FLAG;
    else
        osdConfigMutable()->item_pos[OSD_ALTITUDE] &= ~BLINK_FLAG;
}

void osdResetAlarms(void)
{
    osdConfigMutable()->item_pos[OSD_RSSI_VALUE] &= ~BLINK_FLAG;
    osdConfigMutable()->item_pos[OSD_MAIN_BATT_VOLTAGE] &= ~BLINK_FLAG;
    osdConfigMutable()->item_pos[OSD_GPS_SATS] &= ~BLINK_FLAG;
    osdConfigMutable()->item_pos[OSD_FLYTIME] &= ~BLINK_FLAG;
    osdConfigMutable()->item_pos[OSD_MAH_DRAWN] &= ~BLINK_FLAG;
}

static void osdResetStats(void)
{
    stats.max_current = 0;
    stats.max_speed = 0;
    stats.min_voltage = 500;
    stats.max_current = 0;
    stats.min_rssi = 99;
    stats.max_altitude = 0;
}

static void osdUpdateStats(void)
{
    int16_t value;

    if (feature(FEATURE_GPS)) {
        value = gpsSol.groundSpeed;
        if (stats.max_speed < value)
            stats.max_speed = value;

        if (stats.max_distance < GPS_distanceToHome)
            stats.max_distance = GPS_distanceToHome;
    }

    if (stats.min_voltage > vbat)
        stats.min_voltage = vbat;

    value = abs(amperage / 100);
    if (stats.max_current < value)
        stats.max_current = value;

    if (stats.min_rssi > statRssi)
        stats.min_rssi = statRssi;

#ifdef NAV
    if (stats.max_altitude < getEstimatedActualPosition(Z))
        stats.max_altitude = getEstimatedActualPosition(Z);
#else
    if (stats.max_altitude < baro.BaroAlt)
        stats.max_altitude = baro.BaroAlt;
#endif
}

static void osdShowStats(void)
{
    uint8_t top = 2;
    const uint8_t statValuesX = 21;
    char buff[10];

    displayClearScreen(osdDisplayPort);
    displayWrite(osdDisplayPort, 2, top++, "  --- STATS ---");

    if (STATE(GPS_FIX)) {
        displayWrite(osdDisplayPort, 2, top, "MAX SPEED        :");
        osdFormatVelocityStr(buff, stats.max_speed);
        displayWrite(osdDisplayPort, statValuesX, top++, buff);

        displayWrite(osdDisplayPort, 2, top, "MAX DISTANCE     :");
        osdFormatDistanceStr(buff, stats.max_distance*100);
        displayWrite(osdDisplayPort, statValuesX, top++, buff);

        displayWrite(osdDisplayPort, 2, top, "TRAVELED DISTANCE:");
        osdFormatDistanceStr(buff, getTotalTravelDistance());
        displayWrite(osdDisplayPort, statValuesX, top++, buff);
    }

    displayWrite(osdDisplayPort, 2, top, "MIN BATTERY      :");
    tfp_sprintf(buff, "%d.%1dV", stats.min_voltage / 10, stats.min_voltage % 10);
    displayWrite(osdDisplayPort, statValuesX, top++, buff);

    displayWrite(osdDisplayPort, 2, top, "MIN RSSI         :");
    itoa(stats.min_rssi, buff, 10);
    strcat(buff, "%");
    displayWrite(osdDisplayPort, statValuesX, top++, buff);

    if (feature(FEATURE_CURRENT_METER)) {
        displayWrite(osdDisplayPort, 2, top, "MAX CURRENT      :");
        itoa(stats.max_current, buff, 10);
        strcat(buff, "A");
        displayWrite(osdDisplayPort, statValuesX, top++, buff);

        displayWrite(osdDisplayPort, 2, top, "USED MAH         :");
        itoa(mAhDrawn, buff, 10);
        strcat(buff, "\x07");
        displayWrite(osdDisplayPort, statValuesX, top++, buff);
    }

    displayWrite(osdDisplayPort, 2, top, "MAX ALTITUDE     :");
    osdFormatDistanceStr(buff, stats.max_altitude);
    displayWrite(osdDisplayPort, statValuesX, top++, buff);

    displayWrite(osdDisplayPort, 2, top, "FLY TIME         :");
    uint32_t flySeconds = flyTime / 1000000;
    uint16_t flyMinutes = flySeconds / 60;
    flySeconds %= 60;
    uint16_t flyHours = flyMinutes / 60;
    flyMinutes %= 60;
    tfp_sprintf(buff, "%02u:%02u:%02u", flyHours, flyMinutes, flySeconds);
    displayWrite(osdDisplayPort, statValuesX, top++, buff);
}

// called when motors armed
static void osdShowArmed(void)
{
    dateTime_t dt;
    char buf[MAX(32, FORMATTED_DATE_TIME_BUFSIZE)];
    char *date;
    char *time;
    uint8_t y = 7;

    displayClearScreen(osdDisplayPort);
    displayWrite(osdDisplayPort, 12, y, "ARMED");
    y += 2;

    if (STATE(GPS_FIX)) {
        osdFormatCoordinate(buf, SYM_LAT, GPS_home.lat);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, y, buf);
        osdFormatCoordinate(buf, SYM_LON, gpsSol.llh.lon);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(buf)) / 2, y + 1, buf);
        y += 3;
    }

    if (rtcGetDateTime(&dt)) {
        dateTimeFormatLocal(buf, &dt);
        dateTimeSplitFormatted(buf, &date, &time);

        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(date)) / 2, y, date);
        displayWrite(osdDisplayPort, (osdDisplayPort->cols - strlen(time)) / 2, y + 1, time);
    }
}

static void osdRefresh(timeUs_t currentTimeUs)
{
    static timeUs_t lastTimeUs = 0;

    // detect arm/disarm
    if (armState != ARMING_FLAG(ARMED)) {
        if (ARMING_FLAG(ARMED)) {
            osdResetStats();
            osdShowArmed(); // reset statistic etc
            resumeRefreshAt = currentTimeUs + (REFRESH_1S / 2);
        } else {
            osdShowStats(); // show statistic
            resumeRefreshAt = currentTimeUs + (60 * REFRESH_1S);
        }

        armState = ARMING_FLAG(ARMED);
    }

    if (ARMING_FLAG(ARMED)) {
        timeUs_t deltaT = currentTimeUs - lastTimeUs;
        flyTime += deltaT;
    }

    lastTimeUs = currentTimeUs;

    if (resumeRefreshAt) {
        if (cmp32(currentTimeUs, resumeRefreshAt) < 0) {
            // in timeout period, check sticks for activity to resume display.
            if (checkStickPosition(THR_HI) || checkStickPosition(PIT_HI)) {
                resumeRefreshAt = 0;
            }

            displayHeartbeat(osdDisplayPort);
            return;
        } else {
            displayClearScreen(osdDisplayPort);
            resumeRefreshAt = 0;
        }
    }

    blinkState = (currentTimeUs / 200000) % 2;

#ifdef CMS
    if (!displayIsGrabbed(osdDisplayPort)) {
        if (fullRedraw) {
            displayClearScreen(osdDisplayPort);
            fullRedraw = false;
        }
        osdUpdateAlarms();
        osdDrawNextElement();
        displayHeartbeat(osdDisplayPort);
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

#define DRAW_FREQ_DENOM     4
#define STATS_FREQ_DENOM    50
    counter++;

    if ((counter % STATS_FREQ_DENOM) == 0) {
        osdUpdateStats();
    }

    if ((counter & DRAW_FREQ_DENOM) == 0) {
        // redraw values in buffer
        osdRefresh(currentTimeUs);
    } else {
        // rest of time redraw screen
        displayDrawScreen(osdDisplayPort);
    }

#ifdef CMS
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

#endif // OSD

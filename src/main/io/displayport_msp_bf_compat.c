/*
 * This file is part of INAV Project.
 *
 * INAV is free software: you can redistribute it and/or modify
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

#include "platform.h"

#ifdef USE_MSP_DISPLAYPORT

#ifndef DISABLE_MSP_BF_COMPAT

#include "io/displayport_msp_bf_compat.h"
#include "io/bf_osd_symbols.h"
#include "drivers/osd_symbols.h"

uint8_t getBfCharacter(uint8_t ch, uint8_t page)
{
    uint16_t ech = ch | (page << 8);

    if (ech >= 0x20 && ech <= 0x5F) { // ASCII range
        return ch;
    }

    switch (ech) {
    case SYM_RSSI:
        return BF_SYM_RSSI;

    case SYM_LQ:
        return BF_SYM_LINK_QUALITY;

    case SYM_LAT:
        return BF_SYM_LAT;

    case SYM_LON:
        return BF_SYM_LON;

    case SYM_SAT_L:
        return BF_SYM_SAT_L;

    case SYM_SAT_R:
        return BF_SYM_SAT_R;

    case SYM_HOME_NEAR:
        return BF_SYM_HOMEFLAG;

    case SYM_DEGREES:
        return BF_SYM_GPS_DEGREE;

/*
    case SYM_HEADING:
        return BF_SYM_HEADING;

    case SYM_SCALE:
        return BF_SYM_SCALE;

    case SYM_HDP_L:
        return BF_SYM_HDP_L;

    case SYM_HDP_R:
        return BF_SYM_HDP_R;
*/
    case SYM_HOME:
        return BF_SYM_HOMEFLAG;

    case SYM_2RSS:
        return BF_SYM_RSSI;

/*
    case SYM_DB:
        return BF_SYM_DB

    case SYM_DBM:
        return BF_SYM_DBM;

    case SYM_SNR:
        return BF_SYM_SNR;

    case SYM_AH_DECORATION_UP:
        return BF_SYM_AH_DECORATION_UP;

    case SYM_AH_DECORATION_DOWN:
        return BF_SYM_AH_DECORATION_DOWN;

    case SYM_DIRECTION:
        return BF_SYM_DIRECTION;
*/
    case SYM_VOLT:
        return BF_SYM_VOLT;

    case SYM_MAH:
        return BF_SYM_MAH;

    case SYM_AH_KM:
        return BF_SYM_KM;

    case SYM_AH_MI:
        return BF_SYM_MILES;
/*
    case SYM_VTX_POWER:
        return BF_SYM_VTX_POWER;

    case SYM_AH_NM:
        return BF_SYM_AH_NM;

    case SYM_MAH_NM_0:
        return BF_SYM_MAH_NM_0;

    case SYM_MAH_NM_1:
        return BF_SYM_MAH_NM_1;

    case SYM_MAH_KM_0:
        return BF_SYM_MAH_KM_0;

    case SYM_MAH_KM_1:
        return BF_SYM_MAH_KM_1;

    case SYM_MILLIOHM:
        return BF_SYM_MILLIOHM;
*/
    case SYM_BATT_FULL:
        return BF_SYM_BATT_FULL;

    case SYM_BATT_5:
        return BF_SYM_BATT_5;

    case SYM_BATT_4:
        return BF_SYM_BATT_4;

    case SYM_BATT_3:
        return BF_SYM_BATT_3;

    case SYM_BATT_2:
        return BF_SYM_BATT_2;

    case SYM_BATT_1:
        return BF_SYM_BATT_1;

    case SYM_BATT_EMPTY:
        return BF_SYM_BATT_EMPTY;

    case SYM_AMP:
        return BF_SYM_AMP;
/*
    case SYM_WH:
        return BF_SYM_WH;

    case SYM_WH_KM:
        return BF_SYM_WH_KM;

    case SYM_WH_MI:
        return BF_SYM_WH_MI;

    case SYM_WH_NM:
        return BF_SYM_WH_NM;
*/
    case SYM_WATT:
        return BF_SYM_WATT;
/*
    case SYM_MW:
        return BF_SYM_MW;

    case SYM_KILOWATT:
        return BF_SYM_KILOWATT;
*/
    case SYM_FT:
        return BF_SYM_FT;
/*
    case SYM_TRIP_DIST:
        return BF_SYM_TRIP_DIST;

    case SYM_TOTAL:
        return BF_SYM_TOTAL;

    case SYM_ALT_M:
        return BF_SYM_ALT_M;

    case SYM_ALT_KM:
        return BF_SYM_ALT_KM;

    case SYM_ALT_FT:
        return BF_SYM_ALT_FT;

    case SYM_ALT_KFT:
        return BF_SYM_ALT_KFT;

    case SYM_DIST_M:
        return BF_SYM_DIST_M;

    case SYM_DIST_KM:
        return BF_SYM_DIST_KM;

    case SYM_DIST_FT:
        return BF_SYM_DIST_FT;

    case SYM_DIST_MI:
        return BF_SYM_DIST_MI;

    case SYM_DIST_NM:
        return BF_SYM_DIST_NM;
*/
    case SYM_M:
        return BF_SYM_M;

    case SYM_KM:
        return BF_SYM_KM;

    case SYM_MI:
        return BF_SYM_MILES;
/*
    case SYM_NM:
        return BF_SYM_NM;

    case SYM_WIND_HORIZONTAL:
        return BF_SYM_WIND_HORIZONTAL;

    case SYM_WIND_VERTICAL:
        return BF_SYM_WIND_VERTICAL;

    case SYM_3D_KMH:
        return BF_SYM_3D_KMH;

    case SYM_3D_MPH:
        return BF_SYM_3D_MPH;

    case SYM_3D_KT:
        return BF_SYM_3D_KT;

    case SYM_RPM:
        return BF_SYM_RPM;

    case SYM_AIR:
        return BF_SYM_AIR;
*/

    case SYM_FTS:
        return BF_SYM_FTPS;
/*
    case SYM_100FTM:
        return BF_SYM_100FTM;
*/
    case SYM_MS:
        return BF_SYM_MPS;

    case SYM_KMH:
        return BF_SYM_KPH;

    case SYM_MPH:
        return BF_SYM_MPH;
/*
    case SYM_KT:
        return BF_SYM_KT

    case SYM_MAH_MI_0:
        return BF_SYM_MAH_MI_0;

    case SYM_MAH_MI_1:
        return BF_SYM_MAH_MI_1;
*/
    case SYM_THR:
        return BF_SYM_THR;

/*
    case SYM_TEMP_F:
        return BF_SYM_TEMP_F;

    case SYM_TEMP_C:
        return BF_SYM_TEMP_C;
*/
    case SYM_BLANK:
        return BF_SYM_BLANK;
/*
    case SYM_ON_H:
        return BF_SYM_ON_H;

    case SYM_FLY_H:
        return BF_SYM_FLY_H;
*/
    case SYM_ON_M:
        return BF_SYM_ON_M;

    case SYM_FLY_M:
        return BF_SYM_FLY_M;
/*
    case SYM_GLIDESLOPE:
        return BF_SYM_GLIDESLOPE;

    case SYM_WAYPOINT:
        return BF_SYM_WAYPOINT;

    case SYM_CLOCK:
        return BF_SYM_CLOCK;

    case SYM_ZERO_HALF_TRAILING_DOT:
        return BF_SYM_ZERO_HALF_TRAILING_DOT;

    case SYM_ZERO_HALF_LEADING_DOT:
        return BF_SYM_ZERO_HALF_LEADING_DOT;

    case SYM_AUTO_THR0:
        return BF_SYM_AUTO_THR0;

    case SYM_AUTO_THR1:
        return BF_SYM_AUTO_THR1;

    case SYM_ROLL_LEFT:
        return BF_SYM_ROLL_LEFT;

    case SYM_ROLL_LEVEL:
        return BF_SYM_ROLL_LEVEL;

    case SYM_ROLL_RIGHT:
        return BF_SYM_ROLL_RIGHT;

    case SYM_PITCH_UP:
        return BF_SYM_PITCH_UP;

    case SYM_PITCH_DOWN:
        return BF_SYM_PITCH_DOWN;

    case SYM_GFORCE:
        return BF_SYM_GFORCE;

    case SYM_GFORCE_X:
        return BF_SYM_GFORCE_X;

    case SYM_GFORCE_Y:
        return BF_SYM_GFORCE_Y;

    case SYM_GFORCE_Z:
        return BF_SYM_GFORCE_Z;

    case SYM_BARO_TEMP:
        return BF_SYM_BARO_TEMP;

    case SYM_IMU_TEMP:
        return BF_SYM_IMU_TEMP;

    case SYM_TEMP:
        return BF_SYM_TEMP;

    case SYM_TEMP_SENSOR_FIRST:
        return BF_SYM_TEMP_SENSOR_FIRST;

    case SYM_ESC_TEMP:
        return BF_SYM_ESC_TEMP;

    case SYM_TEMP_SENSOR_LAST:
        return BF_SYM_TEMP_SENSOR_LAST;

    case TEMP_SENSOR_SYM_COUNT:
        return BF_TEMP_SENSOR_SYM_COUNT;
*/
    case SYM_HEADING_N:
        return BF_SYM_HEADING_N;

    case SYM_HEADING_S:
        return BF_SYM_HEADING_S;

    case SYM_HEADING_E:
        return BF_SYM_HEADING_E;

    case SYM_HEADING_W:
        return BF_SYM_HEADING_W;

    case SYM_HEADING_DIVIDED_LINE:
        return BF_SYM_HEADING_DIVIDED_LINE;

    case SYM_HEADING_LINE:
        return BF_SYM_HEADING_LINE;
/*
    case SYM_MAX:
        return BF_SYM_MAX;

    case SYM_PROFILE:
        return BF_SYM_PROFILE;

    case SYM_SWITCH_INDICATOR_LOW:
        return BF_SYM_SWITCH_INDICATOR_LOW;

    case SYM_SWITCH_INDICATOR_MID:
        return BF_SYM_SWITCH_INDICATOR_MID;

    case SYM_SWITCH_INDICATOR_HIGH:
        return BF_SYM_SWITCH_INDICATOR_HIGH;

    case SYM_AH:
        return BF_SYM_AH;

    case SYM_GLIDE_DIST:
        return BF_SYM_GLIDE_DIST;

    case SYM_GLIDE_MINS:
        return BF_SYM_GLIDE_MINS;

    case SYM_AH_V_FT_0:
        return BF_SYM_AH_V_FT_0;

    case SYM_AH_V_FT_1:
        return BF_SYM_AH_V_FT_1;

    case SYM_AH_V_M_0:
        return BF_SYM_AH_V_M_0;

    case SYM_AH_V_M_1:
        return BF_SYM_AH_V_M_1;

    case SYM_FLIGHT_MINS_REMAINING:
        return BF_SYM_FLIGHT_MINS_REMAINING;

    case SYM_FLIGHT_HOURS_REMAINING:
        return BF_SYM_FLIGHT_HOURS_REMAINING;

    case SYM_GROUND_COURSE:
        return BF_SYM_GROUND_COURSE;

    case SYM_CROSS_TRACK_ERROR:
        return BF_SYM_CROSS_TRACK_ERROR;

    case SYM_LOGO_START:
        return BF_SYM_LOGO_START;

    case SYM_LOGO_WIDTH:
        return BF_SYM_LOGO_WIDTH;

    case SYM_LOGO_HEIGHT:
        return BF_SYM_LOGO_HEIGHT;
*/
    case SYM_AH_LEFT:
        return BF_SYM_AH_LEFT;

    case SYM_AH_RIGHT:
        return BF_SYM_AH_RIGHT;
/*
    case SYM_AH_DECORATION_MIN:
        return BF_SYM_AH_DECORATION_MIN;
*/
    case SYM_AH_DECORATION:
        return BF_SYM_AH_DECORATION;
/*
    case SYM_AH_DECORATION_MAX:
        return BF_SYM_AH_DECORATION_MAX;

    case SYM_AH_DECORATION_COUNT:
        return BF_SYM_AH_DECORATION_COUNT;
*/
    case SYM_AH_CH_LEFT:
    case SYM_AH_CH_TYPE3:
    case SYM_AH_CH_TYPE4:
    case SYM_AH_CH_TYPE5:
    case SYM_AH_CH_TYPE6:
    case SYM_AH_CH_TYPE7:
    case SYM_AH_CH_TYPE8:
    case SYM_AH_CH_AIRCRAFT1:
        return BF_SYM_AH_CENTER_LINE;

    case SYM_AH_CH_RIGHT:
    case (SYM_AH_CH_TYPE3+2):
    case (SYM_AH_CH_TYPE4+2):
    case (SYM_AH_CH_TYPE5+2):
    case (SYM_AH_CH_TYPE6+2):
    case (SYM_AH_CH_TYPE7+2):
    case (SYM_AH_CH_TYPE8+2):
    case SYM_AH_CH_AIRCRAFT3:
        return BF_SYM_AH_CENTER_LINE_RIGHT;
    
    case SYM_AH_CH_AIRCRAFT0:
    case SYM_AH_CH_AIRCRAFT4:
        return ' ';

    case SYM_ARROW_UP:
        return BF_SYM_ARROW_NORTH;

    case SYM_ARROW_2:
        return BF_SYM_ARROW_2;

    case SYM_ARROW_3:
        return BF_SYM_ARROW_3;

    case SYM_ARROW_4:
        return BF_SYM_ARROW_4;

    case SYM_ARROW_RIGHT:
        return BF_SYM_ARROW_EAST;

    case SYM_ARROW_6:
        return BF_SYM_ARROW_6;

    case SYM_ARROW_7:
        return BF_SYM_ARROW_7;

    case SYM_ARROW_8:
        return BF_SYM_ARROW_8;

    case SYM_ARROW_DOWN:
        return BF_SYM_ARROW_SOUTH;

    case SYM_ARROW_10:
        return BF_SYM_ARROW_10;

    case SYM_ARROW_11:
        return BF_SYM_ARROW_11;

    case SYM_ARROW_12:
        return BF_SYM_ARROW_12;

    case SYM_ARROW_LEFT:
        return BF_SYM_ARROW_WEST;

    case SYM_ARROW_14:
        return BF_SYM_ARROW_14;

    case SYM_ARROW_15:
        return BF_SYM_ARROW_15;

    case SYM_ARROW_16:
        return BF_SYM_ARROW_16;

    case SYM_AH_H_START:
        return BF_SYM_AH_BAR9_0;

    case (SYM_AH_H_START+1):
        return BF_SYM_AH_BAR9_1;

    case (SYM_AH_H_START+2):
        return BF_SYM_AH_BAR9_2;

    case (SYM_AH_H_START+3):
        return BF_SYM_AH_BAR9_3;

    case (SYM_AH_H_START+4):
        return BF_SYM_AH_BAR9_4;

    case (SYM_AH_H_START+5):
        return BF_SYM_AH_BAR9_5;

    case (SYM_AH_H_START+6):
        return BF_SYM_AH_BAR9_6;

    case (SYM_AH_H_START+7):
        return BF_SYM_AH_BAR9_7;

    case (SYM_AH_H_START+8):
        return BF_SYM_AH_BAR9_8;

/*
    case SYM_AH_V_START:
        return BF_SYM_AH_V_START;

    case SYM_VARIO_UP_2A:
        return BF_SYM_VARIO_UP_2A;

    case SYM_VARIO_UP_1A:
        return BF_SYM_VARIO_UP_1A;

    case SYM_VARIO_DOWN_1A:
        return BF_SYM_VARIO_DOWN_1A;

    case SYM_VARIO_DOWN_2A:
        return BF_SYM_VARIO_DOWN_2A;

    case SYM_ALT:
        return BF_SYM_ALT;

    case SYM_HUD_SIGNAL_0:
        return BF_SYM_HUD_SIGNAL_0;

    case SYM_HUD_SIGNAL_1:
        return BF_SYM_HUD_SIGNAL_1;

    case SYM_HUD_SIGNAL_2:
        return BF_SYM_HUD_SIGNAL_2;

    case SYM_HUD_SIGNAL_3:
        return BF_SYM_HUD_SIGNAL_3;

    case SYM_HUD_SIGNAL_4:
        return BF_SYM_HUD_SIGNAL_4;

    case SYM_HOME_DIST:
        return BF_SYM_HOME_DIST;
*/

    case SYM_AH_CH_CENTER:
    case (SYM_AH_CH_TYPE3+1):
    case (SYM_AH_CH_TYPE4+1):
    case (SYM_AH_CH_TYPE5+1):
    case (SYM_AH_CH_TYPE6+1):
    case (SYM_AH_CH_TYPE7+1):
    case (SYM_AH_CH_TYPE8+1):
    case SYM_AH_CH_AIRCRAFT2:
        return BF_SYM_AH_CENTER;
/*
    case SYM_FLIGHT_DIST_REMAINING:
        return BF_SYM_FLIGHT_DIST_REMAINING;

    case SYM_AH_CH_TYPE3:
        return BF_SYM_AH_CH_TYPE3;

    case SYM_AH_CH_TYPE4:
        return BF_SYM_AH_CH_TYPE4;

    case SYM_AH_CH_TYPE5:
        return BF_SYM_AH_CH_TYPE5;

    case SYM_AH_CH_TYPE6:
        return BF_SYM_AH_CH_TYPE6;

    case SYM_AH_CH_TYPE7:
        return BF_SYM_AH_CH_TYPE7;

    case SYM_AH_CH_TYPE8:
        return BF_SYM_AH_CH_TYPE8;

    case SYM_AH_CH_AIRCRAFT0:
        return BF_SYM_AH_CH_AIRCRAFT0;

    case SYM_AH_CH_AIRCRAFT1:
        return BF_SYM_AH_CH_AIRCRAFT1;

    case SYM_AH_CH_AIRCRAFT2:
        return BF_SYM_AH_CH_AIRCRAFT2;

    case SYM_AH_CH_AIRCRAFT3:
        return BF_SYM_AH_CH_AIRCRAFT3;

    case SYM_AH_CH_AIRCRAFT4:
        return BF_SYM_AH_CH_AIRCRAFT4;

    case SYM_HUD_ARROWS_L1:
        return BF_SYM_HUD_ARROWS_L1;

    case SYM_HUD_ARROWS_L2:
        return BF_SYM_HUD_ARROWS_L2;

    case SYM_HUD_ARROWS_L3:
        return BF_SYM_HUD_ARROWS_L3;

    case SYM_HUD_ARROWS_R1:
        return BF_SYM_HUD_ARROWS_R1;

    case SYM_HUD_ARROWS_R2:
        return BF_SYM_HUD_ARROWS_R2;

    case SYM_HUD_ARROWS_R3:
        return BF_SYM_HUD_ARROWS_R3;

    case SYM_HUD_ARROWS_U1:
        return BF_SYM_HUD_ARROWS_U1;

    case SYM_HUD_ARROWS_U2:
        return BF_SYM_HUD_ARROWS_U2;

    case SYM_HUD_ARROWS_U3:
        return BF_SYM_HUD_ARROWS_U3;

    case SYM_HUD_ARROWS_D1:
        return BF_SYM_HUD_ARROWS_D1;

    case SYM_HUD_ARROWS_D2:
        return BF_SYM_HUD_ARROWS_D2;

    case SYM_HUD_ARROWS_D3:
        return BF_SYM_HUD_ARROWS_D3;
*/
    default:
        break;
    }

    return '?'; // Missing/not mapped character
}


#endif

#endif
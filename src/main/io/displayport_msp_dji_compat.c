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

#if defined(USE_OSD) && defined(USE_MSP_DISPLAYPORT)

#ifndef DISABLE_MSP_DJI_COMPAT

#include "io/displayport_msp_dji_compat.h"
#include "io/dji_osd_symbols.h"
#include "drivers/osd_symbols.h"
#include <string.h>

//                       0123456789
static char *dji_logo = " DJI, FIX "
                        " THE OSD  "
                        "  FOR O3  "
                        "  AND O4  ";

uint8_t getDJICharacter(uint8_t ch, uint8_t page)
{
    uint16_t ech = ch | ((page & 0x3)<< 8) ;

    if (ech >= 0x20 && ech <= 0x5F) { // ASCII range
        return ch;
    }

    if (ech >= SYM_AH_DECORATION_MIN && ech <= SYM_AH_DECORATION_MAX) {
        return DJI_SYM_AH_DECORATION;
    }

    if (ech >= SYM_LOGO_START && ech <= 297) {
        return dji_logo[(ech - SYM_LOGO_START) % (strlen(dji_logo) + 1)];
    }

    if (ech >= SYM_PILOT_LOGO_LRG_START && ech <= 511) {
        return dji_logo[(ech - SYM_LOGO_START) % (strlen(dji_logo) + 1)];
    }

    switch (ech) {
        case SYM_RSSI:
            return DJI_SYM_RSSI;

        case SYM_LQ:
            return 'Q';

        case SYM_LAT:
            return DJI_SYM_LAT;

        case SYM_LON:
            return DJI_SYM_LON;

        case SYM_SAT_L:
            return DJI_SYM_SAT_L;

        case SYM_SAT_R:
            return DJI_SYM_SAT_R;

        case SYM_HOME_NEAR:
            return DJI_SYM_HOMEFLAG;

        case SYM_DEGREES:
            return DJI_SYM_GPS_DEGREE;

/*
        case SYM_HEADING:
            return DJI_SYM_HEADING;

        case SYM_SCALE:
            return DJI_SYM_SCALE;

        case SYM_HDP_L:
            return DJI_SYM_HDP_L;

        case SYM_HDP_R:
            return DJI_SYM_HDP_R;
*/
        case SYM_HOME:
            return DJI_SYM_HOMEFLAG;

        case SYM_2RSS:
            return DJI_SYM_RSSI;

/*
        case SYM_DB:
            return DJI_SYM_DB

        case SYM_DBM:
            return DJI_SYM_DBM;

        case SYM_SNR:
            return DJI_SYM_SNR;
*/

        case SYM_AH_DECORATION_UP:
            return DJI_SYM_ARROW_SMALL_UP;

        case SYM_AH_DECORATION_DOWN:
            return DJI_SYM_ARROW_SMALL_DOWN;

        case SYM_DECORATION:
            return DJI_SYM_ARROW_SMALL_UP;
        
        case SYM_DECORATION + 1: // NE pointing arrow
            return DJI_SYM_ARROW_7;

        case SYM_DECORATION + 2: // E pointing arrow
            return DJI_SYM_ARROW_EAST;

        case SYM_DECORATION + 3: // SE pointing arrow
            return DJI_SYM_ARROW_3;

        case SYM_DECORATION + 4: // S pointing arrow
            return DJI_SYM_ARROW_SOUTH;

        case SYM_DECORATION + 5: // SW pointing arrow
            return DJI_SYM_ARROW_15;

        case SYM_DECORATION + 6: // W pointing arrow
            return DJI_SYM_ARROW_WEST;

        case SYM_DECORATION + 7: // NW pointing arrow
            return DJI_SYM_ARROW_11;

        case SYM_VOLT:
            return DJI_SYM_VOLT;

        case SYM_MAH:
            return DJI_SYM_MAH;

        case SYM_AH_KM:
            return 'K';

        case SYM_AH_MI:
            return 'M';
/*
        case SYM_VTX_POWER:
            return DJI_SYM_VTX_POWER;

        case SYM_AH_NM:
            return DJI_SYM_AH_NM;

        case SYM_MAH_NM_0:
            return DJI_SYM_MAH_NM_0;

        case SYM_MAH_NM_1:
            return DJI_SYM_MAH_NM_1;

        case SYM_MAH_KM_0:
            return DJI_SYM_MAH_KM_0;

        case SYM_MAH_KM_1:
            return DJI_SYM_MAH_KM_1;

        case SYM_MILLIOHM:
            return DJI_SYM_MILLIOHM;
*/
        case SYM_BATT_FULL:
            return DJI_SYM_BATT_FULL;

        case SYM_BATT_5:
            return DJI_SYM_BATT_5;

        case SYM_BATT_4:
            return DJI_SYM_BATT_4;

        case SYM_BATT_3:
            return DJI_SYM_BATT_3;

        case SYM_BATT_2:
            return DJI_SYM_BATT_2;

        case SYM_BATT_1:
            return DJI_SYM_BATT_1;

        case SYM_BATT_EMPTY:
            return DJI_SYM_BATT_EMPTY;

        case SYM_AMP:
            return DJI_SYM_AMP;
/*
        case SYM_WH:
            return DJI_SYM_WH;

        case SYM_WH_KM:
            return DJI_SYM_WH_KM;

        case SYM_WH_MI:
            return DJI_SYM_WH_MI;

        case SYM_WH_NM:
            return DJI_SYM_WH_NM;
*/
        case SYM_WATT:
            return DJI_SYM_WATT;
/*
        case SYM_MW:
            return DJI_SYM_MW;

        case SYM_KILOWATT:
            return DJI_SYM_KILOWATT;
*/
        case SYM_FT:
            return DJI_SYM_FT;

        case SYM_ALT_FT:
            return DJI_SYM_FT;

        case SYM_ALT_M:
            return DJI_SYM_M;

        case SYM_TOTAL:
            return DJI_SYM_FLY_H;
/*

        case SYM_ALT_KM:
            return DJI_SYM_ALT_KM;

        case SYM_ALT_KFT:
            return DJI_SYM_ALT_KFT;

        case SYM_DIST_M:
            return DJI_SYM_DIST_M;

        case SYM_DIST_KM:
            return DJI_SYM_DIST_KM;

        case SYM_DIST_FT:
            return DJI_SYM_DIST_FT;

        case SYM_DIST_MI:
            return DJI_SYM_DIST_MI;

        case SYM_DIST_NM:
            return DJI_SYM_DIST_NM;
*/
        case SYM_M:
            return DJI_SYM_M;

        case SYM_KM:
            return 'K';

        case SYM_MI:
            return 'M';
/*
        case SYM_NM:
            return DJI_SYM_NM;
*/
        case SYM_WIND_HORIZONTAL:
            return 'W';     // W for wind

/*
        case SYM_WIND_VERTICAL:
            return DJI_SYM_WIND_VERTICAL;

        case SYM_3D_KT:
            return DJI_SYM_3D_KT;
*/
        case SYM_AIR:
            return 'A';     // A for airspeed

        case SYM_3D_KMH:
            return DJI_SYM_KPH;

        case SYM_3D_MPH:
            return DJI_SYM_MPH;

        case SYM_RPM:
            return DJI_SYM_RPM;

        case SYM_FTS:
            return DJI_SYM_FTPS;
/*
        case SYM_100FTM:
            return DJI_SYM_100FTM;
*/
        case SYM_MS:
            return DJI_SYM_MPS;

        case SYM_KMH:
            return DJI_SYM_KPH;

        case SYM_MPH:
            return DJI_SYM_MPH;
/*
        case SYM_KT:
            return DJI_SYM_KT

        case SYM_MAH_MI_0:
            return DJI_SYM_MAH_MI_0;

        case SYM_MAH_MI_1:
            return DJI_SYM_MAH_MI_1;
*/
        case SYM_THR:
            return DJI_SYM_THR;

        case SYM_TEMP_F:
            return DJI_SYM_F;

        case SYM_TEMP_C:
            return DJI_SYM_C;

        case SYM_BLANK:
            return DJI_SYM_BLANK;

        case SYM_ON_H:
            return DJI_SYM_ON_H;

        case SYM_FLY_H:
            return DJI_SYM_FLY_H;

        case SYM_ON_M:
            return DJI_SYM_ON_M;

        case SYM_FLY_M:
            return DJI_SYM_FLY_M;
/*
        case SYM_GLIDESLOPE:
            return DJI_SYM_GLIDESLOPE;

        case SYM_WAYPOINT:
            return DJI_SYM_WAYPOINT;

        case SYM_CLOCK:
            return DJI_SYM_CLOCK;

        case SYM_ZERO_HALF_TRAILING_DOT:
            return DJI_SYM_ZERO_HALF_TRAILING_DOT;

        case SYM_ZERO_HALF_LEADING_DOT:
            return DJI_SYM_ZERO_HALF_LEADING_DOT;
*/

        case SYM_AUTO_THR0:
            return 'A';

        case SYM_AUTO_THR1:
            return DJI_SYM_THR;

        case SYM_ROLL_LEFT:
            return DJI_SYM_ROLL;

        case SYM_ROLL_LEVEL:
            return DJI_SYM_ROLL;

        case SYM_ROLL_RIGHT:
            return DJI_SYM_ROLL;

        case SYM_PITCH_UP:
            return DJI_SYM_PITCH;

        case SYM_PITCH_DOWN:
            return DJI_SYM_PITCH;

        case SYM_GFORCE:
            return 'G';

/*
        case SYM_GFORCE_X:
            return DJI_SYM_GFORCE_X;

        case SYM_GFORCE_Y:
            return DJI_SYM_GFORCE_Y;

        case SYM_GFORCE_Z:
            return DJI_SYM_GFORCE_Z;
*/
        case SYM_BARO_TEMP:
            return DJI_SYM_TEMPERATURE;

        case SYM_IMU_TEMP:
            return DJI_SYM_TEMPERATURE;

        case SYM_TEMP:
            return DJI_SYM_TEMPERATURE;

        case SYM_ESC_TEMP:
            return DJI_SYM_TEMPERATURE;
/*
        case SYM_TEMP_SENSOR_FIRST:
            return DJI_SYM_TEMP_SENSOR_FIRST;

        case SYM_TEMP_SENSOR_LAST:
            return DJI_SYM_TEMP_SENSOR_LAST;

        case TEMP_SENSOR_SYM_COUNT:
            return DJI_TEMP_SENSOR_SYM_COUNT;
*/
        case SYM_HEADING_N:
            return DJI_SYM_HEADING_N;

        case SYM_HEADING_S:
            return DJI_SYM_HEADING_S;

        case SYM_HEADING_E:
            return DJI_SYM_HEADING_E;

        case SYM_HEADING_W:
            return DJI_SYM_HEADING_W;

        case SYM_HEADING_DIVIDED_LINE:
            return DJI_SYM_HEADING_DIVIDED_LINE;

        case SYM_HEADING_LINE:
            return DJI_SYM_HEADING_LINE;

        case SYM_MAX:
            return DJI_SYM_MAX;
/*
        case SYM_PROFILE:
            return DJI_SYM_PROFILE;
*/
        case SYM_SWITCH_INDICATOR_LOW:
            return DJI_SYM_STICK_OVERLAY_SPRITE_LOW;

        case SYM_SWITCH_INDICATOR_MID:
            return DJI_SYM_STICK_OVERLAY_SPRITE_MID;

        case SYM_SWITCH_INDICATOR_HIGH:
            return DJI_SYM_STICK_OVERLAY_SPRITE_HIGH;
/*
        case SYM_AH:
            return DJI_SYM_AH;

        case SYM_GLIDE_DIST:
            return DJI_SYM_GLIDE_DIST;

        case SYM_GLIDE_MINS:
            return DJI_SYM_GLIDE_MINS;

        case SYM_AH_V_FT_0:
            return DJI_SYM_AH_V_FT_0;

        case SYM_AH_V_FT_1:
            return DJI_SYM_AH_V_FT_1;

        case SYM_AH_V_M_0:
            return DJI_SYM_AH_V_M_0;

        case SYM_AH_V_M_1:
            return DJI_SYM_AH_V_M_1;

        case SYM_FLIGHT_MINS_REMAINING:
            return DJI_SYM_FLIGHT_MINS_REMAINING;

        case SYM_FLIGHT_HOURS_REMAINING:
            return DJI_SYM_FLIGHT_HOURS_REMAINING;

        case SYM_GROUND_COURSE:
            return DJI_SYM_GROUND_COURSE;

        case SYM_CROSS_TRACK_ERROR:
            return DJI_SYM_CROSS_TRACK_ERROR;
*/

        case SYM_AH_LEFT:
            return DJI_SYM_AH_LEFT;

        case SYM_AH_RIGHT:
            return DJI_SYM_AH_RIGHT;
/*
        case SYM_AH_DECORATION_COUNT:
            return DJI_SYM_AH_DECORATION_COUNT;
*/
        case SYM_AH_CH_LEFT:
        case SYM_AH_CH_AIRCRAFT1:
            return DJI_SYM_CROSSHAIR_LEFT;
        case SYM_AH_CH_CENTER:
        case SYM_AH_CH_AIRCRAFT2:
            return DJI_SYM_CROSSHAIR_CENTRE;
        case SYM_AH_CH_RIGHT:
        case SYM_AH_CH_AIRCRAFT3:
            return DJI_SYM_CROSSHAIR_RIGHT;
        
        case SYM_AH_CH_AIRCRAFT0:
        case SYM_AH_CH_AIRCRAFT4:
            return DJI_SYM_BLANK;

        case SYM_AH_CH_TYPE3:
            return DJI_SYM_NONE;
        case (SYM_AH_CH_TYPE3+1):
            return DJI_SYM_SMALL_CROSSHAIR;
        case (SYM_AH_CH_TYPE3+2):
            return DJI_SYM_NONE;
        
        case SYM_AH_CH_TYPE4:
            return DJI_SYM_HYPHEN;
        case (SYM_AH_CH_TYPE4+1):
            return DJI_SYM_SMALL_CROSSHAIR;
        case (SYM_AH_CH_TYPE4+2):
            return DJI_SYM_HYPHEN;
        
        case SYM_AH_CH_TYPE5:
            return DJI_SYM_STICK_OVERLAY_HORIZONTAL;
        case (SYM_AH_CH_TYPE5+1):
            return DJI_SYM_SMALL_CROSSHAIR;
        case (SYM_AH_CH_TYPE5+2):
            return DJI_SYM_STICK_OVERLAY_HORIZONTAL;
        
        case SYM_AH_CH_TYPE6:
            return DJI_SYM_NONE;
        case (SYM_AH_CH_TYPE6+1):
            return DJI_SYM_STICK_OVERLAY_SPRITE_MID;
        case (SYM_AH_CH_TYPE6+2):
            return DJI_SYM_NONE;
        
        case SYM_AH_CH_TYPE7:
            return DJI_SYM_ARROW_SMALL_LEFT;
        case (SYM_AH_CH_TYPE7+1):
            return DJI_SYM_SMALL_CROSSHAIR;
        case (SYM_AH_CH_TYPE7+2):
            return DJI_SYM_ARROW_SMALL_RIGHT;
        
        case SYM_AH_CH_TYPE8:
            return DJI_SYM_AH_LEFT;
        case (SYM_AH_CH_TYPE8+1):
            return DJI_SYM_SMALL_CROSSHAIR;
        case (SYM_AH_CH_TYPE8+2):
            return DJI_SYM_AH_RIGHT;

        case SYM_ARROW_UP:
            return DJI_SYM_ARROW_NORTH;

        case SYM_ARROW_2:
            return DJI_SYM_ARROW_8;

        case SYM_ARROW_3:
            return DJI_SYM_ARROW_7;

        case SYM_ARROW_4:
            return DJI_SYM_ARROW_6;

        case SYM_ARROW_RIGHT:
            return DJI_SYM_ARROW_EAST;

        case SYM_ARROW_6:
            return DJI_SYM_ARROW_4;

        case SYM_ARROW_7:
            return DJI_SYM_ARROW_3;

        case SYM_ARROW_8:
            return DJI_SYM_ARROW_2;

        case SYM_ARROW_DOWN:
            return DJI_SYM_ARROW_SOUTH;

        case SYM_ARROW_10:
            return DJI_SYM_ARROW_16;

        case SYM_ARROW_11:
            return DJI_SYM_ARROW_15;

        case SYM_ARROW_12:
            return DJI_SYM_ARROW_14;

        case SYM_ARROW_LEFT:
            return DJI_SYM_ARROW_WEST;

        case SYM_ARROW_14:
            return DJI_SYM_ARROW_12;

        case SYM_ARROW_15:
            return DJI_SYM_ARROW_11;

        case SYM_ARROW_16:
            return DJI_SYM_ARROW_10;

        case SYM_AH_H_START:
            return DJI_SYM_AH_BAR9_0;

        case (SYM_AH_H_START+1):
            return DJI_SYM_AH_BAR9_1;

        case (SYM_AH_H_START+2):
            return DJI_SYM_AH_BAR9_2;

        case (SYM_AH_H_START+3):
            return DJI_SYM_AH_BAR9_3;

        case (SYM_AH_H_START+4):
            return DJI_SYM_AH_BAR9_4;

        case (SYM_AH_H_START+5):
            return DJI_SYM_AH_BAR9_5;

        case (SYM_AH_H_START+6):
            return DJI_SYM_AH_BAR9_6;

        case (SYM_AH_H_START+7):
            return DJI_SYM_AH_BAR9_7;

        case (SYM_AH_H_START+8):
            return DJI_SYM_AH_BAR9_8;

        // DJI does not have vertical artificial horizon. replace with middle horizontal one
        case SYM_AH_V_START:
        case (SYM_AH_V_START+1):
        case (SYM_AH_V_START+2):
        case (SYM_AH_V_START+3):
        case (SYM_AH_V_START+4):
        case (SYM_AH_V_START+5):
            return DJI_SYM_AH_BAR9_4;

        // DJI for ESP_RADAR Symbols
        case SYM_HUD_CARDINAL:
            return DJI_SYM_ARROW_SOUTH;
        case SYM_HUD_CARDINAL + 1:
            return DJI_SYM_ARROW_16;
        case SYM_HUD_CARDINAL + 2:
            return DJI_SYM_ARROW_15;
        case SYM_HUD_CARDINAL + 3:
            return DJI_SYM_ARROW_WEST;
        case SYM_HUD_CARDINAL + 4:
            return DJI_SYM_ARROW_12;
        case SYM_HUD_CARDINAL + 5:
            return DJI_SYM_ARROW_11;
        case SYM_HUD_CARDINAL + 6:
            return DJI_SYM_ARROW_NORTH;
        case SYM_HUD_CARDINAL + 7:
            return DJI_SYM_ARROW_7;
        case SYM_HUD_CARDINAL + 8:
            return DJI_SYM_ARROW_6;
        case SYM_HUD_CARDINAL + 9:
            return DJI_SYM_ARROW_EAST;
        case SYM_HUD_CARDINAL + 10:
            return DJI_SYM_ARROW_3;
        case SYM_HUD_CARDINAL + 11:
            return DJI_SYM_ARROW_2;

        case SYM_HUD_SIGNAL_0:
            return DJI_SYM_AH_BAR9_1;
        case SYM_HUD_SIGNAL_1:
            return DJI_SYM_AH_BAR9_3;
        case SYM_HUD_SIGNAL_2:
            return DJI_SYM_AH_BAR9_4;
        case SYM_HUD_SIGNAL_3:
            return DJI_SYM_AH_BAR9_5;
        case SYM_HUD_SIGNAL_4:
            return DJI_SYM_AH_BAR9_7;

        case SYM_VARIO_UP_2A:
            return DJI_SYM_ARROW_SMALL_UP;

        case SYM_VARIO_UP_1A:
            return DJI_SYM_ARROW_SMALL_UP;

        case SYM_VARIO_DOWN_1A:
            return DJI_SYM_ARROW_SMALL_DOWN;

        case SYM_VARIO_DOWN_2A:
            return DJI_SYM_ARROW_SMALL_DOWN;

        case SYM_ALT:
            return DJI_SYM_ALTITUDE;
/*
        case SYM_HUD_SIGNAL_0:
            return DJI_SYM_HUD_SIGNAL_0;

        case SYM_HUD_SIGNAL_1:
            return DJI_SYM_HUD_SIGNAL_1;

        case SYM_HUD_SIGNAL_2:
            return DJI_SYM_HUD_SIGNAL_2;

        case SYM_HUD_SIGNAL_3:
            return DJI_SYM_HUD_SIGNAL_3;

        case SYM_HUD_SIGNAL_4:
            return DJI_SYM_HUD_SIGNAL_4;

        case SYM_HOME_DIST:
            return DJI_SYM_HOME_DIST;

        case SYM_FLIGHT_DIST_REMAINING:
            return DJI_SYM_FLIGHT_DIST_REMAINING;
*/
        case SYM_HUD_ARROWS_L1:
            return DJI_SYM_ARROW_SMALL_LEFT;

        case SYM_HUD_ARROWS_L2:
            return DJI_SYM_ARROW_SMALL_LEFT;

        case SYM_HUD_ARROWS_L3:
            return DJI_SYM_ARROW_SMALL_LEFT;

        case SYM_HUD_ARROWS_R1:
            return DJI_SYM_ARROW_SMALL_RIGHT;

        case SYM_HUD_ARROWS_R2:
            return DJI_SYM_ARROW_SMALL_RIGHT;

        case SYM_HUD_ARROWS_R3:
            return DJI_SYM_ARROW_SMALL_RIGHT;

        case SYM_HUD_ARROWS_U1:
            return DJI_SYM_ARROW_SMALL_UP;

        case SYM_HUD_ARROWS_U2:
            return DJI_SYM_ARROW_SMALL_UP;

        case SYM_HUD_ARROWS_U3:
            return DJI_SYM_ARROW_SMALL_UP;

        case SYM_HUD_ARROWS_D1:
            return DJI_SYM_ARROW_SMALL_DOWN;

        case SYM_HUD_ARROWS_D2:
            return DJI_SYM_ARROW_SMALL_DOWN;

        case SYM_HUD_ARROWS_D3:
            return DJI_SYM_ARROW_SMALL_DOWN;

        default:
            break;
    }

    return (osdConfig()->highlight_djis_missing_characters) ? '?' : SYM_BLANK; // Missing/not mapped character
}

#endif

#endif

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

#include <stdint.h>

#include "platform.h"
#include "flight/imu.h"
#include "io/osd.h"
#include "drivers/osd.h"
#include "drivers/osd_symbols.h"
#include "drivers/display.h"
#include "navigation/navigation.h"
#include "common/printf.h"

#ifdef USE_OSD

#define HUD_DRAWN_MAXCHARS 48 // 8 POI (1 home, 4 radar, 3 waypoints) x 6 chars max for each

static int8_t hud_drawn[HUD_DRAWN_MAXCHARS][2];
static int8_t hud_drawn_pt;
extern displayPort_t *osdDisplayPort;

/* Overwrite all previously written positions on the OSD with a blank
 */

void osdHudClear()
{
    for (int i = 0; i < HUD_DRAWN_MAXCHARS; i++) {
        if (hud_drawn[i][0] > -1) {
            displayWriteChar(osdDisplayPort, hud_drawn[i][0], hud_drawn[i][1], SYM_BLANK);
            hud_drawn[i][0] = -1;
        }
    }
    hud_drawn_pt = 0;
}

/* Write a single char on the OSD, and record the position for the next loop
 */

int osdHudWrite(uint8_t px, uint8_t py, uint16_t symb, bool crush)
{
    if (!crush) {
        uint16_t c;
        if (displayReadCharWithAttr(osdDisplayPort, px, py, &c, NULL) && c != SYM_BLANK) {
            return false;
        }
    }

    displayWriteChar(osdDisplayPort, px, py, symb);
    hud_drawn[hud_drawn_pt][0] = px;
    hud_drawn[hud_drawn_pt][1] = py;
    hud_drawn_pt++;
    if (hud_drawn_pt >= HUD_DRAWN_MAXCHARS)
        { hud_drawn_pt = 0; }
    return true;
}

/* Constrain an angle between -180 and +180°
 */

int16_t hudWrap180(int16_t angle)
{
    while (angle < -179) {
        angle += 360;
    }
    while (angle > 180) {
        angle -= 360;
    }
    return angle;
}

/* Constrain an angle between 0 and +360°
 */

int16_t hudWrap360(int16_t angle)
{
    while (angle < 0) {
        angle += 360;
    }
    while (angle > 360) {
        angle -= 360;
    }
    return angle;
}

/* Radar, get the nearest POI
 */

int radarGetNearestPoi()
{
    int poi = -1;
    uint16_t min = 10000; // 10kms

    for (int i = 0; i < RADAR_MAX_POIS; i++) {
         if ((radar_pois[i].distance > 0) && (radar_pois[i].distance < min)) { // (radar_pois[i].state == 1)
            min = radar_pois[i].distance;
            poi = i;
        }
    }
    return poi;
}

/* Radar, get the farthest POI
 */

int radarGetFarthestPoi()
{
    int poi = -1;
    uint16_t max = 0;

    for (int i = 0; i < RADAR_MAX_POIS; i++) {
         if ((radar_pois[i].distance > max) && (radar_pois[i].distance <= osdConfig()->hud_disp_maxdist)) { // (radar_pois[i].state == 1)
            max = radar_pois[i].distance;
            poi = i;
        }
    }
    return poi;
}
/* Display one POI on the hud, centered on crosshair position.
 * poiDistance and poiAltitude in meters, poiAltitude is relative to the aircraft (negative means below)
 */

void osdHudDrawPoi(uint32_t poiDistance, int16_t poiDirection, int32_t poiAltitude, uint8_t poiSignal, uint16_t poiSymbol)
{
    int poi_x;
    int poi_y;
    uint8_t center_x;
    uint8_t center_y;
    bool poi_is_oos = 0;

    uint8_t minX = osdConfig()->hud_margin_h + 1;
    uint8_t maxX = osdDisplayPort->cols - osdConfig()->hud_margin_h - 2;
    uint8_t minY = osdConfig()->hud_margin_v;
    uint8_t maxY = osdDisplayPort->rows - osdConfig()->hud_margin_v - 2;

    osdCrosshairPosition(&center_x, &center_y);

    int16_t error_x = hudWrap180(poiDirection - DECIDEGREES_TO_DEGREES(osdGetHeading()));

    if ((error_x > -(osdConfig()->camera_fov_h / 2)) && (error_x < osdConfig()->camera_fov_h / 2)) { // POI might be in sight, extra geometry needed
        float scaled_x = sin_approx(DEGREES_TO_RADIANS(error_x)) / sin_approx(DEGREES_TO_RADIANS(osdConfig()->camera_fov_h / 2));
        poi_x = center_x + 15 * scaled_x;

        if (poi_x > maxX) { // Out of hud area to the right
            poi_x = maxX;
            poi_is_oos = 1;
            }
        else if (poi_x < minX) { // Out of hud area to the left
            poi_x = minX;
            poi_is_oos = 1;
            }
        else { // On sight, lets compute the vertical then
            float poi_angle = atan2_approx(-poiAltitude, poiDistance);
            poi_angle = RADIANS_TO_DEGREES(poi_angle);
            int16_t plane_angle = attitude.values.pitch / 10;
            int camera_angle = osdConfig()->camera_uptilt;
            int16_t error_y = poi_angle - plane_angle + camera_angle;
            float scaled_y = sin_approx(DEGREES_TO_RADIANS(error_y)) / sin_approx(DEGREES_TO_RADIANS(osdConfig()->camera_fov_v / 2));
            poi_y = constrain(center_y + (osdDisplayPort->rows / 2) * scaled_y, minY, maxY - 1);
        }
    }
    else {
        poi_is_oos = 1; // POI is out of view for sure
        poi_x = (error_x > 0 ) ? maxX : minX;
    }

    if (poi_is_oos) {
        uint16_t c;
        uint8_t d = SYM_DIRECTION + (4 + (error_x + 180 + 22) / 45) % 8;
        poi_y = center_y - 2;

        while (displayReadCharWithAttr(osdDisplayPort, poi_x, poi_y, &c, NULL) && c != SYM_BLANK && poi_y < maxY - 3) { // Stacks the out-of-sight POI from top to bottom
            poi_y += 2;
        }

        osdHudWrite(poi_x + 1, poi_y, d, 1);
    }

    osdHudWrite(poi_x, poi_y, poiSymbol, 1);

    if (poiSignal < 5) { // 0 to 4 = signal bars, 5 = Display nothing (Navigation point)
        osdHudWrite(poi_x - 1, poi_y, SYM_HUD_SIGNAL_0 + poiSignal, 1);
        }

    char buff[3];
    if ((osd_unit_e)osdConfig()->units == OSD_UNIT_IMPERIAL) {
        osdFormatCentiNumber(buff, CENTIMETERS_TO_CENTIFEET(poiDistance * 100), FEET_PER_MILE, 0, 3, 3);
    }
    else {
        osdFormatCentiNumber(buff, poiDistance * 100, METERS_PER_KILOMETER, 0, 3, 3);
    }

    osdHudWrite(poi_x - 1, poi_y + 1, buff[0], 0);
    osdHudWrite(poi_x , poi_y + 1, buff[1], 0);
    osdHudWrite(poi_x + 1, poi_y + 1, buff[2], 0);
}


/* Draw the crosshair
 */

void osdHudDrawCrosshair(uint8_t px, uint8_t py)
{
    static const uint16_t crh_style_all[] = {
        SYM_AH_CH_LEFT, SYM_AH_CH_CENTER, SYM_AH_CH_RIGHT,
        SYM_AH_CH_AIRCRAFT1, SYM_AH_CH_AIRCRAFT2, SYM_AH_CH_AIRCRAFT3,
        SYM_AH_CH_TYPE3, SYM_AH_CH_TYPE3 + 1, SYM_AH_CH_TYPE3 + 2,
        SYM_AH_CH_TYPE4, SYM_AH_CH_TYPE4 + 1, SYM_AH_CH_TYPE4 + 2,
        SYM_AH_CH_TYPE5, SYM_AH_CH_TYPE5 + 1, SYM_AH_CH_TYPE5 + 2,
        SYM_AH_CH_TYPE6, SYM_AH_CH_TYPE6 + 1, SYM_AH_CH_TYPE6 + 2,
        SYM_AH_CH_TYPE7, SYM_AH_CH_TYPE7 + 1, SYM_AH_CH_TYPE7 + 2,
    };

    uint8_t crh_crosshair = (osd_crosshairs_style_e)osdConfig()->crosshairs_style;

    displayWriteChar(osdDisplayPort, px - 1, py,crh_style_all[crh_crosshair * 3]);
    displayWriteChar(osdDisplayPort, px, py, crh_style_all[crh_crosshair * 3 + 1]);
    displayWriteChar(osdDisplayPort, px + 1, py, crh_style_all[crh_crosshair * 3 + 2]);
}


/* Draw the homing arrows around the crosshair
 */

void osdHudDrawHoming(uint8_t px, uint8_t py)
{
    int crh_l = SYM_BLANK;
    int crh_r = SYM_BLANK;
    int crh_u = SYM_BLANK;
    int crh_d = SYM_BLANK;

    int16_t crh_diff_head = hudWrap180(GPS_directionToHome - DECIDEGREES_TO_DEGREES(osdGetHeading()));
    float crh_focus_scale = 0.7 + (osd_homing_focus_e)osdConfig()->homing_focus * 0.3; // Narrow = 0.70, Medium = 1.0, Wide = 1.3

    int crh_h1 = OSD_HOMING_LIM_H1 * crh_focus_scale;
    int crh_h2 = OSD_HOMING_LIM_H2 * crh_focus_scale;
    int crh_h3 = OSD_HOMING_LIM_H3 * crh_focus_scale;

    if (crh_diff_head <= -162 || crh_diff_head >= 162) {
        crh_l = SYM_HUD_ARROWS_L3;
        crh_r = SYM_HUD_ARROWS_R3;
    } else if (crh_diff_head > -162 && crh_diff_head <= -126) {
        crh_l = SYM_HUD_ARROWS_L3;
        crh_r = SYM_HUD_ARROWS_R2;
    } else if (crh_diff_head > -126 && crh_diff_head <= -90) {
        crh_l = SYM_HUD_ARROWS_L3;
        crh_r = SYM_HUD_ARROWS_R1;
    } else if (crh_diff_head > -90 && crh_diff_head <= -crh_h3) {
        crh_l = SYM_HUD_ARROWS_L3;
    } else if (crh_diff_head > -crh_h3 && crh_diff_head <= -crh_h2) {
        crh_l = SYM_HUD_ARROWS_L2;
    } else if (crh_diff_head > -crh_h2 && crh_diff_head <= -crh_h1) {
        crh_l = SYM_HUD_ARROWS_L1;
    } else if (crh_diff_head >= crh_h1 && crh_diff_head < crh_h2) {
        crh_r = SYM_HUD_ARROWS_R1;
    } else if (crh_diff_head >= crh_h2 && crh_diff_head < crh_h3) {
        crh_r = SYM_HUD_ARROWS_R2;
    } else if (crh_diff_head >= crh_h3 && crh_diff_head < 90) {
        crh_r = SYM_HUD_ARROWS_R3;
    } else if (crh_diff_head >= 90 && crh_diff_head < 126) {
        crh_l = SYM_HUD_ARROWS_L1;
        crh_r = SYM_HUD_ARROWS_R3;
    } else if (crh_diff_head >= 126 && crh_diff_head < 162) {
        crh_l = SYM_HUD_ARROWS_L2;
        crh_r = SYM_HUD_ARROWS_R3;
    }

    if (ABS(crh_diff_head) < 90) {

        int32_t crh_altitude = osdGetAltitude() / 100;
        int32_t crh_distance = GPS_distanceToHome;

        float crh_home_angle = atan2_approx(crh_altitude, crh_distance);
        crh_home_angle = RADIANS_TO_DEGREES(crh_home_angle);
        int crh_plane_angle = attitude.values.pitch / 10;
        int crh_camera_angle = osdConfig()->camera_uptilt;
        int crh_diff_vert = crh_home_angle - crh_plane_angle + crh_camera_angle;

        int crh_v1 = OSD_HOMING_LIM_V1 * crh_focus_scale;
        int crh_v2 = OSD_HOMING_LIM_V2 * crh_focus_scale;
        int crh_v3 = OSD_HOMING_LIM_V3 * crh_focus_scale;

        if (crh_diff_vert > -crh_v2 && crh_diff_vert <= -crh_v1 ) {
            crh_u = SYM_HUD_ARROWS_U1;
        } else if (crh_diff_vert > -crh_v3 && crh_diff_vert <= -crh_v2) {
            crh_u = SYM_HUD_ARROWS_U2;
        } else if (crh_diff_vert <= -crh_v3) {
            crh_u = SYM_HUD_ARROWS_U3;
        } else if (crh_diff_vert >= crh_v1  && crh_diff_vert < crh_v2) {
            crh_d = SYM_HUD_ARROWS_D1;
        } else if (crh_diff_vert >= crh_v2 && crh_diff_vert < crh_v3) {
            crh_d = SYM_HUD_ARROWS_D2;
        } else if (crh_diff_vert >= crh_v3) {
            crh_d = SYM_HUD_ARROWS_D3;
        }
    }

    displayWriteChar(osdDisplayPort, px - 2, py, crh_l);
    displayWriteChar(osdDisplayPort, px + 2, py, crh_r);
    displayWriteChar(osdDisplayPort, px, py - 1, crh_u);
    displayWriteChar(osdDisplayPort, px, py + 1, crh_d);
}


/* Debug overlay
 */

void osdHudDrawDebug(uint8_t px, uint8_t py)
{
    int poi_id = radarGetNearestPoi();
    char buftmp[15];

    if (poi_id >= 0) {
        tfp_sprintf(buftmp, "%c%10d", SYM_LAT,  radar_pois[poi_id].gps.lat);
        displayWrite(osdDisplayPort, px, py, buftmp);
        tfp_sprintf(buftmp, "%c%10d", SYM_LON,  radar_pois[poi_id].gps.lon);
        displayWrite(osdDisplayPort, px, py + 1, buftmp);
        tfp_sprintf(buftmp, "%c%10d", SYM_ALT_M,  radar_pois[poi_id].gps.alt);
        displayWrite(osdDisplayPort, px, py + 2, buftmp);
        tfp_sprintf(buftmp, "%3d", radar_pois[poi_id].ticker);
        displayWrite(osdDisplayPort, px, py + 3, buftmp);
        tfp_sprintf(buftmp, "%c%6d", SYM_DIST_M ,  radar_pois[poi_id].distance);
        displayWrite(osdDisplayPort, px, py + 7, buftmp);
        tfp_sprintf(buftmp, "%c%6d", SYM_DEGREES,  radar_pois[poi_id].direction);
        displayWrite(osdDisplayPort, px, py + 8, buftmp);
        tfp_sprintf(buftmp, "%c%6d", SYM_ALT_M,  radar_pois[poi_id].altitude);
        displayWrite(osdDisplayPort, px , py + 9, buftmp);
    }
    else {
        displayWrite(osdDisplayPort, px, py, "NO POI FOUND ");
    }
}

#endif // USE_OSD

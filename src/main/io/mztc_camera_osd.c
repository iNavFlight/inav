/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR ANY PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "io/mztc_camera.h"
#include "io/osd.h"
#include "io/osd_common.h"
#include "io/osd/mztc_camera_osd.h"

// OSD element IDs for MassZero Thermal Camera
#define MZTC_OSD_ELEMENT_TEMPERATURE    0x01
#define MZTC_OSD_ELEMENT_STATUS         0x02
#define MZTC_OSD_ELEMENT_ALERTS         0x03
#define MZTC_OSD_ELEMENT_CALIBRATION    0x04
#define MZTC_OSD_ELEMENT_CONNECTION     0x05

// OSD element positions (can be configured)
#define MZTC_OSD_POS_TEMPERATURE_X      2
#define MZTC_OSD_POS_TEMPERATURE_Y      2
#define MZTC_OSD_POS_STATUS_X            2
#define MZTC_OSD_POS_STATUS_Y            3
#define MZTC_OSD_POS_ALERTS_X            2
#define MZTC_OSD_POS_ALERTS_Y            4
#define MZTC_OSD_POS_CALIBRATION_X       2
#define MZTC_OSD_POS_CALIBRATION_Y       5
#define MZTC_OSD_POS_CONNECTION_X        2
#define MZTC_OSD_POS_CONNECTION_Y        6

// OSD element visibility flags
#define MZTC_OSD_VISIBLE_ALWAYS          0x01
#define MZTC_OSD_VISIBLE_ALERTS          0x02
#define MZTC_OSD_VISIBLE_CALIBRATION     0x04
#define MZTC_OSD_VISIBLE_ERROR           0x08

// OSD configuration structure is defined in io/osd/mztc_camera_osd.h

// Remove duplicate PG registration from this file.
// Use the OSD implementation and parameter group defined in src/main/io/osd/mztc_camera_osd.c.

// Internal state
static bool mztcOsdInitialized = false;
static uint32_t mztcOsdLastUpdate = 0;
static uint8_t mztcOsdUpdateInterval = 100; // Update every 100ms

// Forward declarations
static void mztcOsdDrawTemperature(void);
static void mztcOsdDrawStatus(void);
static void mztcOsdDrawAlerts(void);
static void mztcOsdDrawCalibration(void);
static void mztcOsdDrawConnection(void);
static bool mztcOsdShouldDisplay(uint8_t visibility_flag);
static void mztcOsdDrawText(uint8_t x, uint8_t y, const char *text, uint8_t attributes);

// Initialize MassZero Thermal Camera OSD
void mztcOsdInit(void)
{
    if (mztcOsdInitialized) {
        return;
    }

    // Check if OSD is available
    if (!osdIsSupported()) {
        return;
    }

    mztcOsdInitialized = true;
    mztcOsdLastUpdate = millis();

    debug[2] = 0xBB; // Debug indicator
}

// Update MassZero Thermal Camera OSD elements
void mztcOsdUpdate(void)
{
    if (!mztcOsdInitialized || !mztcOsdConfig()->enabled) {
        return;
    }

    uint32_t now = millis();

    // Check if it's time for an update
    if ((now - mztcOsdLastUpdate) < mztcOsdUpdateInterval) {
        return;
    }

    // Draw OSD elements
    if (mztcOsdConfig()->temperature_display) {
        mztcOsdDrawTemperature();
    }

    if (mztcOsdConfig()->status_display) {
        mztcOsdDrawStatus();
    }

    if (mztcOsdConfig()->alerts_display) {
        mztcOsdDrawAlerts();
    }

    if (mztcOsdConfig()->calibration_display) {
        mztcOsdDrawCalibration();
    }

    if (mztcOsdConfig()->connection_display) {
        mztcOsdDrawConnection();
    }

    mztcOsdLastUpdate = now;
}

// Draw temperature display
static void mztcOsdDrawTemperature(void)
{
    if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_ALWAYS)) {
        return;
    }

    const mztcStatus_t *status = mztcGetStatus();
    const mztcConfig_t *config = mztcConfig();
    
    char tempText[32];
    float displayTemp = status->ambient_temperature;
    
    // Convert temperature based on configured unit
    switch (config->temperature_unit) {
        case MZTC_UNIT_FAHRENHEIT:
            displayTemp = (displayTemp * 9.0f / 5.0f) + 32.0f;
            snprintf(tempText, sizeof(tempText), "TEMP:%.1f°F", displayTemp);
            break;
        case MZTC_UNIT_KELVIN:
            displayTemp = displayTemp + 273.15f;
            snprintf(tempText, sizeof(tempText), "TEMP:%.1fK", displayTemp);
            break;
        default: // Celsius
            snprintf(tempText, sizeof(tempText), "TEMP:%.1f°C", displayTemp);
            break;
    }
    
    uint8_t x = MZTC_OSD_POS_TEMPERATURE_X + mztcOsdConfig()->position_x;
    uint8_t y = MZTC_OSD_POS_TEMPERATURE_Y + mztcOsdConfig()->position_y;
    
    mztcOsdDrawText(x, y, tempText, 0);
}

// Draw status display
static void mztcOsdDrawStatus(void)
{
    if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_ALWAYS)) {
        return;
    }

    const mztcStatus_t *status = mztcGetStatus();
    const mztcConfig_t *config = mztcConfig();
    
    char statusText[32];
    const char *modeStr = "UNK";
    
    // Get mode string
    switch (config->mode) {
        case MZTC_MODE_DISABLED: modeStr = "OFF"; break;
        case MZTC_MODE_STANDBY: modeStr = "STBY"; break;
        case MZTC_MODE_CONTINUOUS: modeStr = "CONT"; break;
        case MZTC_MODE_TRIGGERED: modeStr = "TRIG"; break;
        case MZTC_MODE_ALERT: modeStr = "ALRT"; break;
        case MZTC_MODE_RECORDING: modeStr = "REC"; break;
        case MZTC_MODE_CALIBRATION: modeStr = "CAL"; break;
        case MZTC_MODE_SURVEILLANCE: modeStr = "SURV"; break;
    }
    
    snprintf(statusText, sizeof(statusText), "MZTC:%s %dHz", modeStr, config->update_rate);
    
    uint8_t x = MZTC_OSD_POS_STATUS_X + mztcOsdConfig()->position_x;
    uint8_t y = MZTC_OSD_POS_STATUS_Y + mztcOsdConfig()->position_y;
    
    mztcOsdDrawText(x, y, statusText, 0);
}

// Draw alerts display
static void mztcOsdDrawAlerts(void)
{
    const mztcStatus_t *status = mztcGetStatus();
    const mztcConfig_t *config = mztcConfig();
    
    if (!config->temperature_alerts) {
        return; // Alerts disabled
    }
    
    // Check if we should display alerts
    bool showAlert = false;
    if (status->ambient_temperature > config->alert_high_temp) {
        showAlert = true;
    } else if (status->ambient_temperature < config->alert_low_temp) {
        showAlert = true;
    }
    
    if (!showAlert && !mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_ALERTS)) {
        return;
    }
    
    char alertText[32];
    if (status->ambient_temperature > config->alert_high_temp) {
        snprintf(alertText, sizeof(alertText), "HOT:%.1f°C", status->ambient_temperature);
    } else if (status->ambient_temperature < config->alert_low_temp) {
        snprintf(alertText, sizeof(alertText), "COLD:%.1f°C", status->ambient_temperature);
    } else {
        snprintf(alertText, sizeof(alertText), "TEMP:OK");
    }
    
    uint8_t x = MZTC_OSD_POS_ALERTS_X + mztcOsdConfig()->position_x;
    uint8_t y = MZTC_OSD_POS_ALERTS_Y + mztcOsdConfig()->position_y;
    
    // Use different attributes for alerts
    uint8_t attributes = showAlert ? 0x80 : 0; // Highlight alerts
    
    mztcOsdDrawText(x, y, alertText, attributes);
}

// Draw calibration display
static void mztcOsdDrawCalibration(void)
{
    const mztcStatus_t *status = mztcGetStatus();
    
    // Check if we should display calibration info
    if (status->status == MZTC_STATUS_CALIBRATING) {
        if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_CALIBRATION)) {
            return;
        }
    } else if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_ALWAYS)) {
        return;
    }
    
    char calText[32];
    if (status->status == MZTC_STATUS_CALIBRATING) {
        snprintf(calText, sizeof(calText), "CAL:IN PROGRESS");
    } else {
        snprintf(calText, sizeof(calText), "CAL:%dmin ago", status->last_calibration);
    }
    
    uint8_t x = MZTC_OSD_POS_CALIBRATION_X + mztcOsdConfig()->position_x;
    uint8_t y = MZTC_OSD_POS_CALIBRATION_Y + mztcOsdConfig()->position_y;
    
    uint8_t attributes = (status->status == MZTC_STATUS_CALIBRATING) ? 0x80 : 0;
    
    mztcOsdDrawText(x, y, calText, attributes);
}

// Draw connection display
static void mztcOsdDrawConnection(void)
{
    if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_ALWAYS)) {
        return;
    }

    const mztcStatus_t *status = mztcGetStatus();
    
    char connText[32];
    const char *connStr = "UNK";
    
    // Get connection quality string
    if (status->connection_quality >= 80) {
        connStr = "EXC";
    } else if (status->connection_quality >= 60) {
        connStr = "GOOD";
    } else if (status->connection_quality >= 40) {
        connStr = "FAIR";
    } else if (status->connection_quality >= 20) {
        connStr = "POOR";
    } else {
        connStr = "BAD";
    }
    
    snprintf(connText, sizeof(connText), "CONN:%s %d%%", connStr, status->connection_quality);
    
    uint8_t x = MZTC_OSD_POS_CONNECTION_X + mztcOsdConfig()->position_x;
    uint8_t y = MZTC_OSD_POS_CONNECTION_Y + mztcOsdConfig()->position_y;
    
    mztcOsdDrawText(x, y, connText, 0);
}

// Check if OSD element should be displayed
static bool mztcOsdShouldDisplay(uint8_t visibility_flag)
{
    if (!mztcOsdConfig()->enabled) {
        return false;
    }
    
    // Check visibility flags
    if (mztcOsdConfig()->visibility_flags & visibility_flag) {
        return true;
    }
    
    // Check for error conditions
    const mztcStatus_t *status = mztcGetStatus();
    if (status->error_flags && (visibility_flag & MZTC_OSD_VISIBLE_ERROR)) {
        return true;
    }
    
    return false;
}

// Draw text on OSD
static void mztcOsdDrawText(uint8_t x, uint8_t y, const char *text, uint8_t attributes)
{
    if (!osdIsSupported()) {
        return;
    }
    
    // Use OSD common functions to draw text
    // This is a simplified implementation - in practice you'd use the actual OSD drawing functions
    osdElementDrawText(x, y, text, attributes);
}

// Set OSD element visibility
void mztcOsdSetVisibility(uint8_t flags)
{
    mztcOsdConfigMutable()->visibility_flags = flags;
}

// Set OSD position offset
void mztcOsdSetPosition(uint8_t x, uint8_t y)
{
    mztcOsdConfigMutable()->position_x = x;
    mztcOsdConfigMutable()->position_y = y;
}

// Enable/disable specific OSD elements
void mztcOsdSetElementEnabled(uint8_t element, bool enabled)
{
    mztcOsdConfig_t *config = mztcOsdConfigMutable();
    
    switch (element) {
        case MZTC_OSD_ELEMENT_TEMPERATURE:
            config->temperature_display = enabled ? 1 : 0;
            break;
        case MZTC_OSD_ELEMENT_STATUS:
            config->status_display = enabled ? 1 : 0;
            break;
        case MZTC_OSD_ELEMENT_ALERTS:
            config->alerts_display = enabled ? 1 : 0;
            break;
        case MZTC_OSD_ELEMENT_CALIBRATION:
            config->calibration_display = enabled ? 1 : 0;
            break;
        case MZTC_OSD_ELEMENT_CONNECTION:
            config->connection_display = enabled ? 1 : 0;
            break;
    }
}

// Get OSD configuration
const mztcOsdConfig_t* mztcOsdGetConfig(void)
{
    return mztcOsdConfig();
}

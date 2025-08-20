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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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
#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/osd.h"
#include "drivers/time.h"

#include "io/osd/mztc_camera_osd.h"
#include "io/mztc_camera.h"

// Parameter group ID for MassZero Thermal Camera OSD configuration (defined in parameter_group_ids.h)

// Default OSD configuration values
#define MZTC_OSD_DEFAULT_ENABLED        1
#define MZTC_OSD_DEFAULT_TEMP_DISPLAY   1
#define MZTC_OSD_DEFAULT_STATUS_DISPLAY 1
#define MZTC_OSD_DEFAULT_ALERTS_DISPLAY 1
#define MZTC_OSD_DEFAULT_CALIB_DISPLAY  1
#define MZTC_OSD_DEFAULT_CONN_DISPLAY   1
#define MZTC_OSD_DEFAULT_POS_X          10
#define MZTC_OSD_DEFAULT_POS_Y          10
#define MZTC_OSD_DEFAULT_VISIBILITY     MZTC_OSD_VISIBLE_ALWAYS

// Parameter group for MassZero Thermal Camera OSD configuration
PG_REGISTER_WITH_RESET_TEMPLATE(mztcOsdConfig_t, mztcOsdConfig, PG_MZTC_OSD_CONFIG, 0);

PG_RESET_TEMPLATE(mztcOsdConfig_t, mztcOsdConfig,
    .enabled = MZTC_OSD_DEFAULT_ENABLED,
    .temperature_display = MZTC_OSD_DEFAULT_TEMP_DISPLAY,
    .status_display = MZTC_OSD_DEFAULT_STATUS_DISPLAY,
    .alerts_display = MZTC_OSD_DEFAULT_ALERTS_DISPLAY,
    .calibration_display = MZTC_OSD_DEFAULT_CALIB_DISPLAY,
    .connection_display = MZTC_OSD_DEFAULT_CONN_DISPLAY,
    .position_x = MZTC_OSD_DEFAULT_POS_X,
    .position_y = MZTC_OSD_DEFAULT_POS_Y,
    .visibility_flags = MZTC_OSD_DEFAULT_VISIBILITY
);

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

// Initialize MassZero Thermal Camera OSD
void mztcOsdInit(void)
{
    if (mztcOsdInitialized) {
        return;
    }

    mztcOsdInitialized = true;
    mztcOsdLastUpdate = 0;

    // MassZero Thermal Camera OSD initialized
}

// Update MassZero Thermal Camera OSD elements
void mztcOsdUpdate(void)
{
    if (!mztcOsdInitialized || !mztcOsdConfig()->enabled) {
        return;
    }

    uint32_t now = millis();
    if (now - mztcOsdLastUpdate < mztcOsdUpdateInterval) {
        return;
    }

    mztcOsdLastUpdate = now;

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
}

// Draw temperature display
static void mztcOsdDrawTemperature(void)
{
    const mztcStatus_t *status = mztcGetStatus();
    if (!status) {
        return;
    }

    // TODO: Use these when OSD drawing is implemented
    // uint8_t x = mztcOsdConfig()->position_x;
    // uint8_t y = mztcOsdConfig()->position_y;
    
    // Draw temperature info
    char temp_str[32];
    snprintf(temp_str, sizeof(temp_str), "TEMP: %.1fC", (double)status->ambient_temperature);
    
    // Note: In a real implementation, you would use the OSD drawing functions
    // For now, we'll just use debug output
    // OSD: Temperature display at position
}

// Draw status display
static void mztcOsdDrawStatus(void)
{
    const mztcStatus_t *status = mztcGetStatus();
    if (!status) {
        return;
    }

    // TODO: Use these when OSD drawing is implemented
    // uint8_t x = mztcOsdConfig()->position_x;
    // uint8_t y = mztcOsdConfig()->position_y + 1;
    
    char status_str[32];
    snprintf(status_str, sizeof(status_str), "MZTC: %s", (status->connection_quality > 0) ? "ON" : "OFF");
    
    // OSD: Status display at position
}

// Draw alerts display
static void mztcOsdDrawAlerts(void)
{
    if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_ALERTS)) {
        return;
    }

    const mztcStatus_t *status = mztcGetStatus();
    if (!status) {
        return;
    }

    // TODO: Use these when OSD drawing is implemented
    // uint8_t x = mztcOsdConfig()->position_x;
    // uint8_t y = mztcOsdConfig()->position_y + 2;
    
    // Check for temperature alerts
    if (status->ambient_temperature > mztcConfig()->alert_high_temp) {
        // OSD: HIGH TEMP ALERT
    } else if (status->ambient_temperature < mztcConfig()->alert_low_temp) {
        // OSD: LOW TEMP ALERT
    }
}

// Draw calibration display
static void mztcOsdDrawCalibration(void)
{
    if (!mztcOsdShouldDisplay(MZTC_OSD_VISIBLE_CALIBRATION)) {
        return;
    }

    const mztcStatus_t *status = mztcGetStatus();
    if (!status) {
        return;
    }

    // TODO: Use these when OSD drawing is implemented
    // uint8_t x = mztcOsdConfig()->position_x;
    // uint8_t y = mztcOsdConfig()->position_y + 3;
    
    if (status->status == MZTC_STATUS_CALIBRATING) {
        // OSD: CALIBRATING
    }
}

// Draw connection display
static void mztcOsdDrawConnection(void)
{
    const mztcStatus_t *status = mztcGetStatus();
    if (!status) {
        return;
    }

    // TODO: Use these when OSD drawing is implemented
    // uint8_t x = mztcOsdConfig()->position_x;
    // uint8_t y = mztcOsdConfig()->position_y + 4;
    
    char conn_str[32];
    snprintf(conn_str, sizeof(conn_str), "CONN: %s", (status->connection_quality > 0) ? "OK" : "FAIL");
    
    // OSD: Connection status display
}

// Check if OSD element should be displayed
static bool mztcOsdShouldDisplay(uint8_t visibility_flag)
{
    return (mztcOsdConfig()->visibility_flags & visibility_flag) != 0;
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
    switch (element) {
        case MZTC_OSD_ELEMENT_TEMPERATURE:
            mztcOsdConfigMutable()->temperature_display = enabled;
            break;
        case MZTC_OSD_ELEMENT_STATUS:
            mztcOsdConfigMutable()->status_display = enabled;
            break;
        case MZTC_OSD_ELEMENT_ALERTS:
            mztcOsdConfigMutable()->alerts_display = enabled;
            break;
        case MZTC_OSD_ELEMENT_CALIBRATION:
            mztcOsdConfigMutable()->calibration_display = enabled;
            break;
        case MZTC_OSD_ELEMENT_CONNECTION:
            mztcOsdConfigMutable()->connection_display = enabled;
            break;
    }
}

// Get OSD configuration
const mztcOsdConfig_t* mztcOsdGetConfig(void)
{
    return mztcOsdConfig();
}

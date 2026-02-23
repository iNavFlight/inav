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

#include "platform.h"

#ifdef USE_MZTC

#include "build/debug.h"
#include "common/printf.h"
#include "io/mztc_camera.h"
#include "io/mztc_camera_cli.h"

// CLI command: mztc status
static void mztcStatusCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    const mztcStatus_t *status = mztcGetStatus();
    if (!status) {
        cliPrint("MassZero Thermal Camera: ERROR - Status unavailable\n");
        return;
    }
    
    cliPrint("MassZero Thermal Camera Status:\n");
    cliPrint("  Enabled: %s\n", mztcConfig()->enabled ? "YES" : "NO");
    cliPrint("  Connected: %s\n", mztcIsConnected() ? "YES" : "NO");
    cliPrint("  Status: %d\n", status->status);
    cliPrint("  Mode: %d\n", status->mode);
    cliPrint("  Connection Quality: %d%%\n", status->connection_quality);
    cliPrint("  Last Calibration: %d minutes ago\n", status->last_calibration);
    cliPrint("  Camera Temperature: %.1f°C\n", status->camera_temperature);
    cliPrint("  Ambient Temperature: %.1f°C\n", status->ambient_temperature);
    cliPrint("  Frame Count: %lu\n", status->frame_count);
    cliPrint("  Error Flags: 0x%02X\n", status->error_flags);
    cliPrint("  Last Frame: %lu ms ago\n", millis() - status->last_frame_time);
}

// CLI command: mztc calibrate
static void mztcCalibrateCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    if (!mztcIsConnected()) {
        cliPrint("MassZero Thermal Camera: NOT CONNECTED\n");
        return;
    }
    
    if (mztcTriggerCalibration()) {
        cliPrint("MassZero Thermal Camera: Calibration triggered\n");
    } else {
        cliPrint("MassZero Thermal Camera: Calibration failed\n");
    }
}

// CLI command: mztc init_status
static void mztcInitStatusCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    if (!mztcIsConnected()) {
        cliPrint("MassZero Thermal Camera: NOT CONNECTED\n");
        return;
    }
    
    if (mztcGetInitStatus()) {
        cliPrint("MassZero Thermal Camera: Init status request sent\n");
    } else {
        cliPrint("MassZero Thermal Camera: Init status request failed\n");
    }
}

// CLI command: mztc save_config
static void mztcSaveConfigCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    if (!mztcIsConnected()) {
        cliPrint("MassZero Thermal Camera: NOT CONNECTED\n");
        return;
    }
    
    if (mztcSaveConfiguration()) {
        cliPrint("MassZero Thermal Camera: Configuration saved to camera flash\n");
    } else {
        cliPrint("MassZero Thermal Camera: Save configuration failed\n");
    }
}

// CLI command: mztc restore_defaults
static void mztcRestoreDefaultsCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    if (!mztcIsConnected()) {
        cliPrint("MassZero Thermal Camera: NOT CONNECTED\n");
        return;
    }
    
    if (mztcRestoreDefaults()) {
        cliPrint("MassZero Thermal Camera: Restored to factory defaults\n");
    } else {
        cliPrint("MassZero Thermal Camera: Restore defaults failed\n");
    }
}

// CLI command: mztc reconnect
static void mztcReconnectCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    mztcRequestReconnect();
    cliPrint("MassZero Thermal Camera: Reconnection requested\n");
}

// CLI command: mztc preset
static void mztcPresetCommand(char *cmdline)
{
    if (!mztcIsEnabled()) {
        cliPrint("MassZero Thermal Camera: DISABLED\n");
        return;
    }
    
    char *preset = strtok(cmdline, " ");
    if (!preset) {
        cliPrint("Usage: mztc preset <preset_name>\n");
        cliPrint("Available presets: fire_detection, search_rescue, surveillance, rapid_changes, industrial\n");
        return;
    }
    
    mztcConfig_t *config = mztcConfigMutable();
    
    if (strcmp(preset, "fire_detection") == 0) {
        // Fire Detection Mode
        config->mode = MZTC_MODE_ALERT;
        config->palette_mode = MZTC_PALETTE_WHITE_HOT;
        config->brightness = 70;
        config->contrast = 80;
        config->digital_enhancement = 75;
        config->spatial_denoise = 30;
        config->temporal_denoise = 40;
        config->auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;
        config->ffc_interval = 3;
        config->temperature_alerts = 1;
        config->alert_high_temp = 60.0f;
        config->alert_low_temp = -10.0f;
        config->update_rate = 15;
        cliPrint("MassZero Thermal Camera: Fire Detection preset applied\n");
    }
    else if (strcmp(preset, "search_rescue") == 0) {
        // Search and Rescue Mode
        config->mode = MZTC_MODE_CONTINUOUS;
        config->palette_mode = MZTC_PALETTE_FUSION_1;
        config->brightness = 60;
        config->contrast = 70;
        config->digital_enhancement = 60;
        config->spatial_denoise = 50;
        config->temporal_denoise = 60;
        config->auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;
        config->ffc_interval = 5;
        config->temperature_alerts = 1;
        config->alert_high_temp = 45.0f;
        config->alert_low_temp = 15.0f;
        config->update_rate = 9;
        config->zoom_level = MZTC_ZOOM_2X;
        cliPrint("MassZero Thermal Camera: Search and Rescue preset applied\n");
    }
    else if (strcmp(preset, "surveillance") == 0) {
        // Surveillance Mode
        config->mode = MZTC_MODE_SURVEILLANCE;
        config->palette_mode = MZTC_PALETTE_BLACK_HOT;
        config->brightness = 50;
        config->contrast = 60;
        config->digital_enhancement = 50;
        config->spatial_denoise = 60;
        config->temporal_denoise = 70;
        config->auto_shutter = MZTC_SHUTTER_TIME_ONLY;
        config->ffc_interval = 10;
        config->temperature_alerts = 1;
        config->alert_high_temp = 50.0f;
        config->alert_low_temp = 0.0f;
        config->update_rate = 5;
        config->zoom_level = MZTC_ZOOM_4X;
        cliPrint("MassZero Thermal Camera: Surveillance preset applied\n");
    }
    else if (strcmp(preset, "rapid_changes") == 0) {
        // Rapid Environment Changes Mode
        config->mode = MZTC_MODE_CONTINUOUS;
        config->palette_mode = MZTC_PALETTE_RAINBOW;
        config->brightness = 55;
        config->contrast = 65;
        config->digital_enhancement = 70;
        config->spatial_denoise = 40;
        config->temporal_denoise = 30;
        config->auto_shutter = MZTC_SHUTTER_TEMP_ONLY;
        config->ffc_interval = 2;
        config->temperature_alerts = 1;
        config->alert_high_temp = 80.0f;
        config->alert_low_temp = -20.0f;
        config->update_rate = 20;
        config->bad_pixel_removal = 1;
        config->vignetting_correction = 1;
        cliPrint("MassZero Thermal Camera: Rapid Changes preset applied\n");
    }
    else if (strcmp(preset, "industrial") == 0) {
        // Industrial Inspection Mode
        config->mode = MZTC_MODE_CONTINUOUS;
        config->palette_mode = MZTC_PALETTE_IRON_RED_1;
        config->brightness = 60;
        config->contrast = 75;
        config->digital_enhancement = 65;
        config->spatial_denoise = 45;
        config->temporal_denoise = 55;
        config->auto_shutter = MZTC_SHUTTER_TIME_AND_TEMP;
        config->ffc_interval = 7;
        config->temperature_alerts = 1;
        config->alert_high_temp = 100.0f;
        config->alert_low_temp = -30.0f;
        config->update_rate = 12;
        config->zoom_level = MZTC_ZOOM_1X;
        cliPrint("MassZero Thermal Camera: Industrial Inspection preset applied\n");
    }
    else {
        cliPrint("Unknown preset: %s\n", preset);
        cliPrint("Available presets: fire_detection, search_rescue, surveillance, rapid_changes, industrial\n");
        return;
    }
    
    // Apply the configuration to the camera if connected
    if (mztcIsConnected()) {
        mztcSendConfiguration();
        cliPrint("Configuration sent to camera\n");
    }
}

// CLI command: mztc help
static void mztcHelpCommand(char *cmdline)
{
    UNUSED(cmdline);
    
    cliPrint("MassZero Thermal Camera Commands:\n");
    cliPrint("  mztc status          - Show camera status\n");
    cliPrint("  mztc calibrate       - Trigger manual calibration (FFC)\n");
    cliPrint("  mztc init_status     - Get camera initialization status\n");
    cliPrint("  mztc save_config     - Save configuration to camera flash\n");
    cliPrint("  mztc restore_defaults - Restore camera to factory defaults\n");
    cliPrint("  mztc reconnect       - Reconnect to camera\n");
    cliPrint("  mztc preset <name>   - Apply application preset\n");
    cliPrint("  mztc help            - Show this help\n");
    cliPrint("\nAvailable presets:\n");
    cliPrint("  fire_detection       - Fire detection and hot spot identification\n");
    cliPrint("  search_rescue        - Human detection and body heat signatures\n");
    cliPrint("  surveillance         - Long-range monitoring and perimeter security\n");
    cliPrint("  rapid_changes        - Fast-changing environments (weather, altitude)\n");
    cliPrint("  industrial           - Equipment monitoring and thermal analysis\n");
}

// CLI command table
static const cliCommand_t mztcCommands[] = {
    { "mztc", "status", mztcStatusCommand, "Show MassZero Thermal Camera status" },
    { "mztc", "calibrate", mztcCalibrateCommand, "Trigger manual calibration (FFC)" },
    { "mztc", "init_status", mztcInitStatusCommand, "Get camera initialization status" },
    { "mztc", "save_config", mztcSaveConfigCommand, "Save configuration to camera flash" },
    { "mztc", "restore_defaults", mztcRestoreDefaultsCommand, "Restore camera to factory defaults" },
    { "mztc", "reconnect", mztcReconnectCommand, "Reconnect to camera" },
    { "mztc", "preset", mztcPresetCommand, "Apply application-specific preset" },
    { "mztc", "help", mztcHelpCommand, "Show MassZero Thermal Camera help" },
};

// Register CLI commands
void mztcCliInit(void)
{
    for (size_t i = 0; i < ARRAYLEN(mztcCommands); i++) {
        cliAddCommand(&mztcCommands[i]);
    }
}

#endif // USE_MZTC
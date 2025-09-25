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
    cliPrint("  mztc help            - Show this help\n");
}

// CLI command table
static const cliCommand_t mztcCommands[] = {
    { "mztc", "status", mztcStatusCommand, "Show MassZero Thermal Camera status" },
    { "mztc", "calibrate", mztcCalibrateCommand, "Trigger manual calibration (FFC)" },
    { "mztc", "init_status", mztcInitStatusCommand, "Get camera initialization status" },
    { "mztc", "save_config", mztcSaveConfigCommand, "Save configuration to camera flash" },
    { "mztc", "restore_defaults", mztcRestoreDefaultsCommand, "Restore camera to factory defaults" },
    { "mztc", "reconnect", mztcReconnectCommand, "Reconnect to camera" },
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

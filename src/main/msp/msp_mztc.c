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

#include "platform.h"

#include "build/debug.h"

#include "common/streambuf.h"
#include "common/utils.h"

#include "drivers/system.h"

#include "io/mztc_camera.h"
#include "config/mztc_camera.h"

#ifdef USE_MZTC

#include "msp/msp.h"
#include "msp/msp_mztc.h"
#include "common/streambuf.h"

// Initialize MSP MassZero Thermal Camera handlers
void mspMztcInit(void)
{
    // Register MSP command handlers
    // This would typically be done through a registration system
    // For now, we'll handle commands in the main MSP processing loop
}

// Process MassZero Thermal Camera MSP V2 commands
mspResult_e mspMztcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply)
{
    if (!cmd || !reply) {
        return MSP_RESULT_ERROR;
    }

    switch (cmd->cmd) {
        case MSP2_MZTC_CONFIG: {
            // Get MassZero Thermal Camera configuration
            if (sbufBytesRemaining(&reply->buf) < (int)sizeof(msp_mztc_config_t)) {
                return MSP_RESULT_ERROR;
            }

            msp_mztc_config_t *config = (msp_mztc_config_t*)reply->buf.ptr;
            const mztcConfig_t *cfg = mztcConfig();

            config->enabled = cfg->enabled;
            config->port = cfg->port;
            config->baudrate = cfg->baudrate;
            config->mode = cfg->mode;
            config->update_rate = cfg->update_rate;
            config->temperature_unit = cfg->temperature_unit;
            config->palette_mode = cfg->palette_mode;
            config->auto_shutter = cfg->auto_shutter;
            config->digital_enhancement = cfg->digital_enhancement;
            config->spatial_denoise = cfg->spatial_denoise;
            config->temporal_denoise = cfg->temporal_denoise;
            config->brightness = cfg->brightness;
            config->contrast = cfg->contrast;
            config->zoom_level = cfg->zoom_level;
            config->mirror_mode = cfg->mirror_mode;
            config->crosshair_enabled = cfg->crosshair_enabled;
            config->temperature_alerts = cfg->temperature_alerts;
            config->alert_high_temp = cfg->alert_high_temp;
            config->alert_low_temp = cfg->alert_low_temp;
            config->ffc_interval = cfg->ffc_interval;
            config->bad_pixel_removal = cfg->bad_pixel_removal;
            config->vignetting_correction = cfg->vignetting_correction;

            sbufAdvance(&reply->buf, sizeof(msp_mztc_config_t));
            return MSP_RESULT_ACK;
        }

        case MSP2_SET_MZTC_CONFIG: {
            // Set MassZero Thermal Camera configuration
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_config_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_config_t *config = (const msp_mztc_config_t*)cmd->buf.ptr;
            mztcConfig_t *cfgMutable = mztcConfigMutable();

            // Validate parameters
            if (config->update_rate < MZTC_MIN_UPDATE_RATE || config->update_rate > MZTC_MAX_UPDATE_RATE) {
                return MSP_RESULT_ERROR;
            }

            if (config->ffc_interval < MZTC_MIN_FFC_INTERVAL || config->ffc_interval > MZTC_MAX_FFC_INTERVAL) {
                return MSP_RESULT_ERROR;
            }

            // Update configuration
            cfgMutable->enabled = config->enabled;
            cfgMutable->port = config->port;
            cfgMutable->baudrate = config->baudrate;
            cfgMutable->mode = config->mode;
            cfgMutable->update_rate = config->update_rate;
            cfgMutable->temperature_unit = config->temperature_unit;
            cfgMutable->palette_mode = config->palette_mode;
            cfgMutable->auto_shutter = config->auto_shutter;
            cfgMutable->digital_enhancement = config->digital_enhancement;
            cfgMutable->spatial_denoise = config->spatial_denoise;
            cfgMutable->temporal_denoise = config->temporal_denoise;
            cfgMutable->brightness = config->brightness;
            cfgMutable->contrast = config->contrast;
            cfgMutable->zoom_level = config->zoom_level;
            cfgMutable->mirror_mode = config->mirror_mode;
            cfgMutable->crosshair_enabled = config->crosshair_enabled;
            cfgMutable->temperature_alerts = config->temperature_alerts;
            cfgMutable->alert_high_temp = config->alert_high_temp;
            cfgMutable->alert_low_temp = config->alert_low_temp;
            cfgMutable->ffc_interval = config->ffc_interval;
            cfgMutable->bad_pixel_removal = config->bad_pixel_removal;
            cfgMutable->vignetting_correction = config->vignetting_correction;

            return MSP_RESULT_ACK;
        }

        case MSP2_MZTC_STATUS: {
            // Get MassZero Thermal Camera status
            if (sbufBytesRemaining(&reply->buf) < (int)sizeof(msp_mztc_status_t)) {
                return MSP_RESULT_ERROR;
            }

            msp_mztc_status_t *status = (msp_mztc_status_t*)reply->buf.ptr;
            const mztcStatus_t *mztcStatus = mztcGetStatus();

            if (mztcStatus) {
                status->status = mztcStatus->status;
                status->mode = mztcStatus->mode;
                status->connection_quality = mztcStatus->connection_quality;
                status->last_calibration = mztcStatus->last_calibration;
                status->camera_temperature = mztcStatus->camera_temperature;
                status->ambient_temperature = mztcStatus->ambient_temperature;
                status->frame_count = mztcStatus->frame_count;
                status->error_flags = mztcStatus->error_flags;
                status->last_frame_time = mztcStatus->last_frame_time;
            } else {
                // Return default values if status is not available
                status->status = MZTC_STATUS_OFFLINE;
                status->mode = 0;
                status->connection_quality = 0;
                status->last_calibration = 0;
                status->camera_temperature = 0.0f;
                status->ambient_temperature = 0.0f;
                status->frame_count = 0;
                status->error_flags = 0;
                status->last_frame_time = 0;
            }

            sbufAdvance(&reply->buf, sizeof(msp_mztc_status_t));
            return MSP_RESULT_ACK;
        }

        case MSP2_MZTC_FRAME_DATA: {
            // Get thermal frame data
            if (sbufBytesRemaining(&reply->buf) < (int)sizeof(msp_mztc_frame_t)) {
                return MSP_RESULT_ERROR;
            }

            msp_mztc_frame_t *frame = (msp_mztc_frame_t*)reply->buf.ptr;
            
            // For now, return dummy data
            // In a real implementation, this would get actual thermal frame data
            frame->width = 160;  // Typical thermal camera resolution
            frame->height = 120;
            frame->min_temp = 20.0f;
            frame->max_temp = 80.0f;
            frame->center_temp = 25.0f;
            frame->hottest_temp = 80.0f;
            frame->coldest_temp = 20.0f;
            frame->hottest_x = 80;
            frame->hottest_y = 60;
            frame->coldest_x = 0;
            frame->coldest_y = 0;
            
            // Clear thermal data (would contain actual pixel values)
            memset(frame->data, 0, sizeof(frame->data));

            sbufAdvance(&reply->buf, sizeof(msp_mztc_frame_t));
            return MSP_RESULT_ACK;
        }

        case MSP2_MZTC_CALIBRATE: {
            // Trigger calibration (FFC)
            if (mztcTriggerCalibration()) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_MZTC_MODE: {
            // Set operating mode
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_mode_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_mode_t *mode = (const msp_mztc_mode_t*)cmd->buf.ptr;
            
            if (mztcSetMode((mztcMode_e)mode->mode)) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_MZTC_PALETTE: {
            // Set color palette
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_palette_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_palette_t *palette = (const msp_mztc_palette_t*)cmd->buf.ptr;
            
            if (mztcSetPalette((mztcPaletteMode_e)palette->palette)) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_MZTC_ZOOM: {
            // Set zoom level
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_zoom_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_zoom_t *zoom = (const msp_mztc_zoom_t*)cmd->buf.ptr;
            
            if (mztcSetZoom((mztcZoomLevel_e)zoom->zoom)) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_MZTC_SHUTTER: {
            // Trigger manual shutter
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_shutter_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_shutter_t *shutter = (const msp_mztc_shutter_t*)cmd->buf.ptr;
            
            if (mztcTriggerCalibration()) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_SET_MZTC_ALERTS: {
            // Configure temperature alerts
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_alerts_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_alerts_t *alerts = (const msp_mztc_alerts_t*)cmd->buf.ptr;
            
            if (mztcSetTemperatureAlerts(alerts->enabled, alerts->high_temp, alerts->low_temp)) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_SET_MZTC_IMAGE_PARAMS: {
            // Set image parameters
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_image_params_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_image_params_t *params = (const msp_mztc_image_params_t*)cmd->buf.ptr;
            
            if (mztcSetImageParams(params->brightness, params->contrast, params->enhancement)) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        case MSP2_SET_MZTC_CORRECTION: {
            // Set correction parameters
            if (sbufBytesRemaining(&cmd->buf) < (int)sizeof(msp_mztc_correction_t)) {
                return MSP_RESULT_ERROR;
            }

            const msp_mztc_correction_t *correction = (const msp_mztc_correction_t*)cmd->buf.ptr;
            
            if (mztcSetDenoising(correction->spatial_denoise, correction->temporal_denoise)) {
                return MSP_RESULT_ACK;
            } else {
                return MSP_RESULT_ERROR;
            }
        }

        default:
            return MSP_RESULT_NO_REPLY;
    }
}

#endif // USE_MZTC

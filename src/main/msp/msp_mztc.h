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

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "config/mztc_camera.h"

#ifdef USE_MZTC

/*
 * MassZero Thermal Camera MSP V2 commands.
 *
 * These live in INAV's own 0x2000-0x2FFF range. The 0x3000 block belongs to
 * the Betaflight compatibility commands. MSP2_BETAFLIGHT_BIND is 0x3000.
 * MSP2_RX_BIND is 0x3001. That block is out of reach here.
 *
 * Every command listed below is handled in fc_msp.c. There are no reserved or
 * aliased identifiers: an ID exists only if something answers it.
 *
 * All payloads are little endian. Every field is read and written one at a
 * time with the sbuf helpers. No C struct is cast over the stream buffer, which
 * keeps compiler padding and alignment off the wire.
 */

// Out (flight controller to host)
#define MSP2_MZTC_CONFIG                0x2240
#define MSP2_MZTC_STATUS                0x2241

// In (host to flight controller)
#define MSP2_SET_MZTC_CONFIG            0x2242
#define MSP2_SET_MZTC_PRESET            0x2243
#define MSP2_SET_MZTC_PALETTE           0x2244
#define MSP2_SET_MZTC_ZOOM              0x2245
#define MSP2_SET_MZTC_SHUTTER           0x2246
#define MSP2_SET_MZTC_IMAGE_PARAMS      0x2247
#define MSP2_SET_MZTC_CORRECTION        0x2248
#define MSP2_SET_MZTC_VIGNETTING        0x2249

/*
 * Payload layouts
 *
 * MSP2_MZTC_CONFIG (out) and MSP2_SET_MZTC_CONFIG (in), 11 bytes.
 * The port and its baud rate are not here. They come from the Ports tab.
 *   u8  preset
 *   u8  palette_mode
 *   u8  auto_shutter
 *   u8  digital_enhancement
 *   u8  spatial_denoise
 *   u8  temporal_denoise
 *   u8  brightness
 *   u8  contrast
 *   u8  zoom_level
 *   u8  mirror_mode
 *   u8  ffc_interval
 *
 * MSP2_MZTC_STATUS (out), 7 bytes:
 *   u8  status
 *   u8  preset
 *   u8  connected
 *   u8  connection_quality
 *   u16 last_calibration       (minutes)
 *   u8  error_flags
 *
 * MSP2_SET_MZTC_PRESET   (in), 1 byte: u8 preset
 * MSP2_SET_MZTC_PALETTE  (in), 1 byte: u8 palette
 * MSP2_SET_MZTC_ZOOM     (in), 1 byte: u8 zoom_level
 * MSP2_SET_MZTC_SHUTTER  (in), 0 or 1 bytes. Triggers a manual shutter cycle,
 *                        which is the camera's flat field correction. Any
 *                        payload byte is ignored.
 * MSP2_SET_MZTC_IMAGE_PARAMS (in), 3 bytes: u8 brightness, u8 contrast,
 *                        u8 enhancement
 * MSP2_SET_MZTC_CORRECTION   (in), 2 bytes: u8 spatial, u8 temporal
 * MSP2_SET_MZTC_VIGNETTING   (in), 0 or 1 bytes. Runs one vignetting
 *                        correction. The camera manual requires the lens to be
 *                        pointed at a uniform surface first, so this is an
 *                        action and never a stored setting.
 */

#define MSP2_MZTC_CONFIG_PAYLOAD_SIZE           11
#define MSP2_MZTC_STATUS_PAYLOAD_SIZE           7
#define MSP2_SET_MZTC_IMAGE_PARAMS_PAYLOAD_SIZE 3
#define MSP2_SET_MZTC_CORRECTION_PAYLOAD_SIZE   2

#endif // USE_MZTC

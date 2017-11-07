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

#pragma once

#include "drivers/serial.h"

//
// The protocol for Runcam Device definition
//
#define RCDEVICE_PROTOCOL_HEADER                                    0xCC

#define RCDEVICE_PROTOCOL_MAX_PACKET_SIZE                           64
#define RCDEVICE_PROTOCOL_MAX_DATA_SIZE                             62
#define RCDEVICE_PROTOCOL_MAX_DATA_SIZE_WITH_CRC_FIELD              63

// Commands
#define RCDEVICE_PROTOCOL_COMMAND_GET_DEVICE_INFO                   0x00
// camera control
#define RCDEVICE_PROTOCOL_COMMAND_CAMERA_CONTROL                    0x01
// 5 key osd cable simulation
#define RCDEVICE_PROTOCOL_COMMAND_5KEY_SIMULATION_PRESS             0x02
#define RCDEVICE_PROTOCOL_COMMAND_5KEY_SIMULATION_RELEASE           0x03
#define RCDEVICE_PROTOCOL_COMMAND_5KEY_CONNECTION                   0x04

// Feature Flag sets, it's a uint16_t flag
typedef enum {
    RCDEVICE_PROTOCOL_FEATURE_SIMULATE_POWER_BUTTON    = (1 << 0),
    RCDEVICE_PROTOCOL_FEATURE_SIMULATE_WIFI_BUTTON     = (1 << 1),
    RCDEVICE_PROTOCOL_FEATURE_CHANGE_MODE              = (1 << 2),
    RCDEVICE_PROTOCOL_FEATURE_SIMULATE_5_KEY_OSD_CABLE = (1 << 3),
} rcdevice_features_e;

// Operation of Camera Button Simulation
typedef enum {
    RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_WIFI_BTN        = 0x00,
    RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_POWER_BTN       = 0x01,
    RCDEVICE_PROTOCOL_CAM_CTRL_CHANGE_MODE              = 0x02,
    RCDEVICE_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION = 0xFF
} rcdevice_camera_control_opeation_e;

// Operation Of 5 Key OSD Cable Simulation
typedef enum {
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_NONE  = 0x00,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET   = 0x01,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_LEFT  = 0x02,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_RIGHT = 0x03,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_UP    = 0x04,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_DOWN  = 0x05
} rcdevice_5key_simulation_operation_e;

// Operation of RCDEVICE_PROTOCOL_COMMAND_5KEY_CONNECTION
typedef enum { 
    RCDEVICE_PROTOCOL_5KEY_CONNECTION_OPEN = 0x01, 
    RCDEVICE_PROTOCOL_5KEY_CONNECTION_CLOSE = 0x02 
} RCDEVICE_5key_connection_event_e;

typedef enum {
    RCDEVICE_CAM_KEY_NONE,
    RCDEVICE_CAM_KEY_ENTER,
    RCDEVICE_CAM_KEY_LEFT,
    RCDEVICE_CAM_KEY_UP,
    RCDEVICE_CAM_KEY_RIGHT,
    RCDEVICE_CAM_KEY_DOWN,
    RCDEVICE_CAM_KEY_CONNECTION_CLOSE,
    RCDEVICE_CAM_KEY_CONNECTION_OPEN,
    RCDEVICE_CAM_KEY_RELEASE,
} rcdeviceCamSimulationKeyEvent_e;

typedef enum {
    RCDEVICE_PROTOCOL_RCSPLIT_VERSION = 0x00, // this is used to indicate the
                                              // device that using rcsplit
                                              // firmware version that <= 1.1.0
    RCDEVICE_PROTOCOL_VERSION_1_0 = 0x01,
    RCDEVICE_PROTOCOL_UNKNOWN
} rcdevice_protocol_version_e;

typedef struct runcamDeviceConnectionEventResponse_s {
    uint8_t type : 4;
    uint8_t resultCode : 4;
} runcamDeviceConnectionEventResponse_t;

typedef struct runcamDeviceGetDeviceInfoResponse_s {
    uint8_t protocolVersion;
    uint16_t features;
} runcamDeviceGetDeviceInfoResponse_t;
// end of Runcam Device definition

// Old version defination(RCSplit firmware v1.0.0 and v1.1.0)
// packet header and tail
#define RCSPLIT_PACKET_HEADER       0x55
#define RCSPLIT_PACKET_CMD_CTRL     0x01
#define RCSPLIT_PACKET_TAIL         0xaa

typedef enum {
    RCSPLIT_CTRL_ARGU_INVALID     = 0x0,
    RCSPLIT_CTRL_ARGU_WIFI_BTN    = 0x1,
    RCSPLIT_CTRL_ARGU_POWER_BTN   = 0x2,
    RCSPLIT_CTRL_ARGU_CHANGE_MODE = 0x3,
    RCSPLIT_CTRL_ARGU_WHO_ARE_YOU = 0xFF,
} rcsplit_ctrl_argument_e;
// end of old version protocol definition

typedef struct runcamDeviceInfo_s {
    rcdevice_protocol_version_e protocolVersion;
    uint16_t features;
} runcamDeviceInfo_t;

typedef struct runcamDevice_s {
    serialPort_t *serialPort;
    uint8_t buffer[RCDEVICE_PROTOCOL_MAX_PACKET_SIZE];
    runcamDeviceInfo_t info;
} runcamDevice_t;

bool runcamDeviceInit(runcamDevice_t *device);

// camera button simulation
bool runcamDeviceSimulateCameraButton(runcamDevice_t *device, uint8_t operation);

// 5 key osd cable simulation
bool runcamDeviceOpen5KeyOSDCableConnection(runcamDevice_t *device);
bool runcamDeviceClose5KeyOSDCableConnection(runcamDevice_t *device);
bool runcamDeviceSimulate5KeyOSDCableButtonPress(runcamDevice_t *device, uint8_t operation);
bool runcamDeviceSimulate5KeyOSDCableButtonRelease(runcamDevice_t *device);

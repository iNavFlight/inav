/*
 * This file is part of INAV.
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
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

#include "platform.h"

#include "common/time.h"
#include "drivers/gimbal_common.h"
#include "drivers/headtracker_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_SERIAL_GIMBAL


#define HTKATTITUDE_SYNC0  0xA5
#define HTKATTITUDE_SYNC1  0x5A
typedef struct gimbalHtkAttitudePkt_s
{
	uint8_t  sync[2];       //data synchronization 0xA5, 0x5A
	uint8_t mode:3;        //Gimbal Mode [0~7] [Only 0 1 2 modes are supported for the time being]
	int16_t  sensibility:5; // Stabilization sensibility [-16~15]
	uint8_t reserved:4;    //hold on to one's reserve
	int32_t  roll:12;       //Roll angle [-2048~2047] => [-180~180]
	int32_t  tilt:12;       //Pich angle [-2048~2047] => [-180~180]
	int32_t  pan:12;        //Yaw angle [-2048~2047] => [-180~180]
	uint8_t  crch;          //Data validation H
	uint8_t  crcl;          //Data validation L
} __attribute__((packed)) gimbalHtkAttitudePkt_t;


#define HEADTRACKER_PAYLOAD_SIZE (sizeof(gimbalHtkAttitudePkt_t) - 4)

typedef enum {
    WAITING_HDR1,
    WAITING_HDR2,
    WAITING_PAYLOAD,
    WAITING_CRCH,
    WAITING_CRCL,
} gimbalHeadtrackerState_e;

typedef struct gimbalSerialHtrkState_s {
    uint8_t  payloadSize;
    gimbalHeadtrackerState_e state;
    gimbalHtkAttitudePkt_t attitude;
} gimbalSerialHtrkState_t;

typedef struct gimbalSerialConfig_s {
    bool singleUart;
} gimbalSerialConfig_t;

PG_DECLARE(gimbalSerialConfig_t, gimbalSerialConfig);

int16_t gimbal_scale12(int16_t inputMin, int16_t inputMax, int16_t value);

int16_t gimbal2pwm(int16_t value);

bool gimbalSerialInit(void);
bool gimbalSerialDetect(void);
void gimbalSerialProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime);
bool gimbalSerialIsReady(const gimbalDevice_t *gimbalDevice);
gimbalDevType_e gimbalSerialGetDeviceType(const gimbalDevice_t *gimbalDevice);
bool gimbalSerialHasHeadTracker(const gimbalDevice_t *gimbalDevice);
void gimbalSerialHeadTrackerReceive(uint16_t c, void *data);


#if (defined(USE_HEADTRACKER) && defined(USE_HEADTRACKER_SERIAL))
bool gimbalSerialHeadTrackerInit(void);
bool gimbalSerialHeadTrackerDetect(void);
void headtrackerSerialProcess(headTrackerDevice_t *headTrackerDevice, timeUs_t currentTimeUs);
headTrackerDevType_e headtrackerSerialGetDeviceType(const headTrackerDevice_t *headTrackerDevice);
bool headTrackerSerialIsReady(const headTrackerDevice_t *headTrackerDevice);
bool headTrackerSerialIsValid(const headTrackerDevice_t *headTrackerDevice);
int headTrackerSerialGetPanPWM(const headTrackerDevice_t *headTrackerDevice);
int headTrackerSerialGetTiltPWM(const headTrackerDevice_t *headTrackerDevice);
int headTrackerSerialGetRollPWM(const headTrackerDevice_t *headTrackerDevice);
#endif

#endif

#ifdef __cplusplus
}
#endif
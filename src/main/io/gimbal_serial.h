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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_SERIAL_GIMBAL

#define HTKATTITUDE_SYNC0  0xA5
#define HTKATTITUDE_SYNC1  0x5A
typedef struct gimbalHtkAttitudePkt_s
{
	uint8_t  sync[2];   //data synchronization 0xA5, 0x5A
	uint64_t mode:3;     //Gimbal Mode [0~7] [Only 0 1 2 modes are supported for the time being]
	int64_t  sensibility:5;     //Cloud sensibility [-16~15]
	uint64_t reserved:4;     //hold on to one's reserve
	int64_t  roll:12;    //Roll angle [-2048~2047] => [-180~180]
	int64_t  pitch:12;    //Pich angle [-2048~2047] => [-180~180]
	int64_t  yaw:12;    //Yaw angle [-2048~2047] => [-180~180]
	uint64_t crch:8;    //Data validation H
	uint64_t crcl:8;    //Data validation L
} __attribute__((packed)) gimbalHtkAttitudePkt_t;

int16_t gimbal_scale12(int16_t inputMin, int16_t inputMax, int16_t value);

bool gimbalSerialInit(void);
bool gimbalSerialDetect(void);
void gimbalSerialProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTime);
bool gimbalSerialIsReady(const gimbalDevice_t *gimbalDevice);
gimbalDevType_e gimbalSerialGetDeviceType(const gimbalDevice_t *gimbalDevice);

#endif

#ifdef __cplusplus
}
#endif
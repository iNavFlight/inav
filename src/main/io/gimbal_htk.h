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

#define GIMBAL_HTK_MODE_DEFAULT GIMBAL_HTK_MODE_FOLLOW

#define HTKATTITUDE_SYNC0  0xA5
#define HTKATTITUDE_SYNC1  0x5A
typedef struct
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
} __attribute__((packed)) GimbalHtkAttitudePkt_t;

uint8_t gimbal_scale8(int8_t inputMin, int8_t inputMax, int8_t outputMin, int8_t outputMax, int8_t value);
uint16_t gimbal_scale16(int16_t inputMin, int16_t inputMax, int16_t outputMin, int16_t outputMax, int16_t value);

void gimbal_htk_update(void);
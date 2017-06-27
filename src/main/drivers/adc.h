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

#include "drivers/io_types.h"

typedef enum {
    ADC_BATTERY = 0,
    ADC_RSSI = 1,
    ADC_CURRENT = 2,
	ADC_AIRSPEED = 3,
    ADC_FUNCTION_COUNT
} adcFunction_e;

typedef enum {
    ADC_CHANNEL_NONE = 0,
    ADC_CHANNEL_1 = 1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_MAX = ADC_CHANNEL_4,
    ADC_CHANNEL_COUNT
} adcChannel_e;

typedef struct drv_adc_config_s {
    uint8_t channelVBat;
    uint8_t channelRSSI;
    uint8_t channelCurrentMeter;
    uint8_t channelAirSpeed;
} drv_adc_config_t;

void adcInit(drv_adc_config_t *init);
uint16_t adcGetChannel(uint8_t channel);
bool adcIsFunctionAssigned(uint8_t function);

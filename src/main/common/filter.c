/*
 * filter.c
 *
 *  Created on: 24 jun. 2015
 *      Author: borisb
 *      Author: digitalentity
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "common/axis.h"
#include "common/filter.h"

extern uint16_t cycleTime;

// PT1 Low Pass filter (when no dT specified it will be calculated from the cycleTime)
float filterApplyPt1(float input, filterStatePt1_t *filter, uint8_t f_cut, float dT) {

	// Pre calculate and store RC
	if (!filter->RC) {
		filter->RC = 1.0f / ( 2.0f * (float)M_PI * f_cut );
	}

    filter->state = filter->state + dT / (filter->RC + dT) * (input - filter->state);

    return filter->state;
}

// 7 Tap FIR filter as described here:
// Thanks to Qcopter & BorisB
void filterApply9TapFIR(int16_t data[3], int16_t state[3][9], int8_t coeff[9])
{
    int32_t FIRsum;
    int axis, i;

    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        FIRsum = 0;
        for (i = 0; i <= 7; i++) {
            state[axis][i] = state[axis][i + 1];
            FIRsum += state[axis][i] * (int16_t)coeff[i];
        }
        state[axis][8] = data[axis];
        FIRsum += state[axis][8] * coeff[8];
        data[axis] = FIRsum / 256;
    }
}
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

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#include "common/axis.h"
#include "common/maths.h"

#include "drivers/sensor.h"
#include "drivers/compass.h"
#include "drivers/compass_hmc5883l.h"
#include "drivers/gpio.h"
#include "drivers/light_led.h"

#include "sensors/boardalignment.h"
#include "config/runtime_config.h"
#include "config/config.h"

#include "sensors/sensors.h"
#include "sensors/compass.h"

#include "io/rc_controls.h"

// TODO: Remove when compass-mot is done
//#include "debug.h"

#ifdef NAZE
#include "hardware_revision.h"
#endif

mag_t mag;                   // mag access functions

extern uint32_t currentTime; // FIXME dependency on global variable, pass it in instead.

int16_t magADCRaw[XYZ_AXIS_COUNT];
int32_t magADC[XYZ_AXIS_COUNT];
sensor_align_e magAlign = 0;
#ifdef MAG
static uint8_t magInit = 0;
static uint8_t magUpdatedAtLeastOnce = 0;

void compassInit(void)
{
    // initialize and calibration. turn on led during mag calibration (calibration routine blinks it)
    LED1_ON;
    mag.init();
    LED1_OFF;
    magInit = 1;
}

bool isCompassReady(void)
{
    return magUpdatedAtLeastOnce;
}

static sensorCalibrationState_t calState;

void updateCompass(flightDynamicsTrims_t *magZero, flightDynamicsTrims_t *magZeroHover, escAndServoConfig_t *escAndServoConfig, navConfig_t *navConfig)
{
    static uint32_t calStartedAt = 0;
    static int16_t magPrev[XYZ_AXIS_COUNT];
    uint32_t axis;

    mag.read(magADCRaw);
    for (axis = 0; axis < XYZ_AXIS_COUNT; axis++) magADC[axis] = magADCRaw[axis];  // int32_t copy to work with
    alignSensors(magADC, magADC, magAlign);

    if (STATE(CALIBRATE_MAG)) {
        calStartedAt = currentTime;

        for (axis = 0; axis < 3; axis++) {
            magZero->raw[axis] = 0;
            magPrev[axis] = 0;
        }

        sensorCalibrationResetState(&calState);
        DISABLE_STATE(CALIBRATE_MAG);
    }

    if (magInit) {              // we apply offset only once mag calibration is done
        float thrComp = 0;
        if (ARMING_FLAG(ARMED)) {
            // TODO: Read min/max throttle settings. Is rcCommand the correct one to use?
            //thrComp = (float)(rcCommand[THROTTLE]-1000)/(1300-1000); //(thr-thr_min)/(thr_hover-thr_min)
            thrComp = (float)(rcCommand[THROTTLE] - escAndServoConfig->minthrottle) / (navConfig->mc_hover_throttle - escAndServoConfig->minthrottle); //(thr-thr_min)/(thr_hover-thr_min)
        }

        magADC[X] -= magZero->raw[X] + magZeroHover->raw[X] * thrComp;
        magADC[Y] -= magZero->raw[Y] + magZeroHover->raw[Y] * thrComp;
        magADC[Z] -= magZero->raw[Z] + magZeroHover->raw[Z] * thrComp;
        
        // debug[0] = escAndServoConfig->minthrottle;
        // debug[1] = escAndServoConfig->maxthrottle;
        // debug[2] = navConfig->mc_hover_throttle;
        // debug[3] = thrComp * 1000;
        
        // debug[0] = rcCommand[THROTTLE];
        // debug[1] = thrComp * 1000;
        // debug[2] = magZeroHover->raw[X] * thrComp;
    }

    if (calStartedAt != 0) {
        if ((currentTime - calStartedAt) < 30000000) {    // 30s: you have 30s to turn the multi in all directions
            LED0_TOGGLE;

            float diffMag = 0;
            float avgMag = 0;

            for (axis = 0; axis < 3; axis++) {
                diffMag += (magADC[axis] - magPrev[axis]) * (magADC[axis] - magPrev[axis]);
                avgMag += (magADC[axis] + magPrev[axis]) * (magADC[axis] + magPrev[axis]) / 4.0f;
            }

            // sqrtf(diffMag / avgMag) is a rough approximation of tangent of angle between magADC and magPrev. tan(8 deg) = 0.14
            if ((avgMag > 0.01f) && ((diffMag / avgMag) > (0.14f * 0.14f))) {
                sensorCalibrationPushSampleForOffsetCalculation(&calState, magADC);

                for (axis = 0; axis < 3; axis++) {
                    magPrev[axis] = magADC[axis];
                }
            }
        } else {
            float magZerof[3];
            sensorCalibrationSolveForOffset(&calState, magZerof);

            for (axis = 0; axis < 3; axis++) {
                magZero->raw[axis] = lrintf(magZerof[axis]);
            }

            calStartedAt = 0;
            persistentFlagSet(FLAG_MAG_CALIBRATION_DONE);
            saveConfigAndNotify();
        }
    }

    magUpdatedAtLeastOnce = 1;
}
#endif

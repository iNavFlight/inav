/* 
 * This file is part of the INAV distribution https://github.com/iNavFlight/inav.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * The code in this file is a derivative work of EmuFlight distribution https://github.com/emuflight/EmuFlight/
 * 
 */

#include "platform.h"

#ifdef USE_Q_TUNE

FILE_COMPILE_FOR_SPEED

#include <stdlib.h>
#include "q_tune.h"
#include <math.h>

#include "common/filter.h"

#define Q_TUNE_WINDOW_LENGTH 20

typedef struct currentSample_s {
    float setpoint;
    float measurement;
} currentSample_t;

typedef struct samples_s {
    uint8_t index;
    float setpointRaw[Q_TUNE_WINDOW_LENGTH];
    float measurementRaw[Q_TUNE_WINDOW_LENGTH];
    float setpointFiltered[Q_TUNE_WINDOW_LENGTH];
    float measurementFiltered[Q_TUNE_WINDOW_LENGTH];
    float error[Q_TUNE_WINDOW_LENGTH];
    pt1Filter_t setpointFilter;
    pt1Filter_t measurementFilter;
} samples_t;

static currentSample_t currentSample[XYZ_AXIS_COUNT];
static samples_t samples[XYZ_AXIS_COUNT];

void qTunePushSample(const flight_dynamics_index_t axis, const float setpoint, const float measurement) {
    currentSample[axis].setpoint = setpoint;
    currentSample[axis].measurement = measurement;
}

void qTuneProcessTask(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
            samples[i].index = 0;
            pt1FilterInit(&samples[i].setpointFilter, Q_TUNE_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
            pt1FilterInit(&samples[i].measurementFilter, Q_TUNE_LPF_HZ, Q_TUNE_UPDATE_US * 1e-6f);
        }

        initialized = true;
    }

    // Step 1 - pick last sample and start filling in the data

    for (int i = 0; i < XYZ_AXIS_COUNT; i++) {
        currentSample_t *sample = &currentSample[i];

        if (sample->setpoint != 0.0f) {
            samples_t *samples = &samples[i];

            // Step 2 - fill in the data
            samples->setpointRaw[samples->index] = sample->setpoint;
            samples->measurementRaw[samples->index] = sample->measurement;

            // Step 3 - filter the data
            samples->setpointFiltered[samples->index] = pt1FilterApply(&samples->setpointFilter, samples->setpointRaw[samples->index]);
            samples->measurementFiltered[samples->index] = pt1FilterApply(&samples->measurementFilter, samples->measurementRaw[samples->index]);

            // Step 4 - calculate the error
            samples->error[samples->index] = samples->setpointFiltered[samples->index] - samples->measurementFiltered[samples->index];

            // Step 5 - increment the index
            samples->index = (samples->index + 1) % Q_TUNE_WINDOW_LENGTH;
        }
    }

}

#endif
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
 */

#include "common/axis.h" 
#include "common/time.h"

#define Q_TUNE_UPDATE_RATE_HZ 150
#define Q_TUNE_MEASUREMENT_LPF_HZ 75
#define Q_TUNE_SETPOINT_LPF_HZ 10
#define Q_TUNE_ERROR_HPF_HZ 10

#define Q_TUNE_UPDATE_US (1000000 / Q_TUNE_UPDATE_RATE_HZ)

void qTunePushSample(const flight_dynamics_index_t axis, const float setpoint, const float measurement, const float iTerm);
void qTuneProcessTask(timeUs_t currentTimeUs);
void qTunePushGyroPeakFrequency(const flight_dynamics_index_t axis, const float frequency);
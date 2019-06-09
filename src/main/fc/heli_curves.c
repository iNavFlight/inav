/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include "heli_curves.h"
#include "rc_controls.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "flight/mixer.h"
#include "common/maths.h"

PG_REGISTER_WITH_RESET_TEMPLATE(heliCurvesConfig_t, heliCurvesConfig, PG_HELI_CURVES_CONFIG, 0);

PG_RESET_TEMPLATE(heliCurvesConfig_t, heliCurvesConfig,
	.accelerationRC = 30,
	.accelerationCollectivePitch = 0,
	.throttleFromCollectivePitch = {10000, 9000, 8400, 8000, 7660, 7400, 7660, 8000, 8400, 9000, 10000}
);

static uint16_t getThrottleFromCollectivePitch(int16_t cp)
{
	int throttleBlock = (cp+500)/100;
	int16_t lowThrottle = (int16_t)heliCurvesConfig()->throttleFromCollectivePitch[throttleBlock];
	int16_t highThrottle = (int16_t)heliCurvesConfig()->throttleFromCollectivePitch[throttleBlock+1];
	int32_t blockPercent = (cp+500)%100;
	return (uint16_t)scaleRange(blockPercent, 0, 100, lowThrottle, highThrottle);
}

int16_t calculateCollectivePitchAndUpdateThrottle(void)
{
	uint16_t minThrottle = motorConfig()->minthrottle;
	uint16_t maxThrottle = motorConfig()->maxthrottle;
	uint16_t rc = rcCommand[THROTTLE];
	uint16_t rcPercent;
	if (rc<minThrottle) {
		rcPercent = 0;
	} else if (rc>maxThrottle) {
		rcPercent = 10000;
	} else {
		rcPercent = (uint16_t)scaleRange(rc, minThrottle, maxThrottle, 0, 10000);
	}
	uint16_t accelerationRC = (uint16_t)heliCurvesConfig()->accelerationRC*100;
	int16_t collectivePitch;
	uint16_t throttle;
	if (rcPercent<accelerationRC) { //Initial main rotor acceleration
		uint16_t accPercent = (uint16_t)((uint32_t)rcPercent*10000/accelerationRC);
		collectivePitch = heliCurvesConfig()->accelerationCollectivePitch;
		uint16_t accelerationThrottle = getThrottleFromCollectivePitch(collectivePitch);
		throttle = (uint16_t)((uint32_t)accPercent*accelerationThrottle/10000);
	} else { //Normal flight
		uint16_t pitchPercent = (uint16_t)scaleRange(rcPercent, accelerationRC, 10000, 0, 10000);
		int16_t accCollectivePitch = heliCurvesConfig()->accelerationCollectivePitch;
		collectivePitch = (int16_t)scaleRange(pitchPercent, 0, 10000, accCollectivePitch, 500);
		throttle = getThrottleFromCollectivePitch(collectivePitch);
	}
	rcCommand[THROTTLE] = (uint16_t)scaleRange(throttle, 0, 10000, minThrottle, maxThrottle);
	return collectivePitch;
}

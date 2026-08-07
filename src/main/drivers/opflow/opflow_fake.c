/*
 * This file is part of INAV Project.
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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>

#include "build/build_config.h"

#include "common/utils.h"

#include "opflow.h"
#include "opflow_fake.h"

#ifdef USE_OPFLOW_FAKE
static opflowData_t fakeData;

void fakeOpflowSet(timeDelta_t deltaTime, int32_t flowRateX, int32_t flowRateY, int16_t quality)
{
    fakeData.deltaTime = deltaTime;
    fakeData.flowRateRaw[0] = flowRateX;
    fakeData.flowRateRaw[1] = flowRateY;
    fakeData.flowRateRaw[2] = 0;
    fakeData.quality = quality;
}

static bool fakeOpflowInit(opflowDev_t * dev)
{
    UNUSED(dev);
    return true;
}

static bool fakeOpflowUpdate(opflowDev_t * dev)
{
    memcpy(&dev->rawData, &fakeData, sizeof(opflowData_t));
    return true;
}

bool fakeOpflowDetect(opflowDev_t * dev)
{
    dev->initFn = &fakeOpflowInit;
    dev->updateFn = &fakeOpflowUpdate;

    memset(&dev->rawData, 0, sizeof(opflowData_t));

    return true;
}
#endif

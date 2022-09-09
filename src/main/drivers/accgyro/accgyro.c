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
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/build_config.h"

#include "common/log.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"

const gyroFilterAndRateConfig_t * chooseGyroConfig(uint8_t desiredLpf, uint16_t desiredRateHz, const gyroFilterAndRateConfig_t * configs, int count)
{
    int i;
    int8_t selectedLpf = configs[0].gyroLpf;
    const gyroFilterAndRateConfig_t * candidate = &configs[0];

    // Choose closest supported LPF value
    for (i = 1; i < count; i++) {
        if (ABS(desiredLpf - configs[i].gyroLpf) < ABS(desiredLpf - selectedLpf)) {
            selectedLpf = configs[i].gyroLpf;
            candidate = &configs[i];
        }
    }

    // Now find the closest update rate
    for (i = 0; i < count; i++) {
        if ((configs[i].gyroLpf == selectedLpf) && (ABS(desiredRateHz - candidate->gyroRateHz) > ABS(desiredRateHz - configs[i].gyroRateHz))) {
            candidate = &configs[i];
        }
    }

    LOG_V(GYRO, "GYRO CONFIG { %d, %d } -> { %d, %d}; regs 0x%02X, 0x%02X",
                desiredLpf, desiredRateHz,
                candidate->gyroLpf, candidate->gyroRateHz,
                candidate->gyroConfigValues[0], candidate->gyroConfigValues[1]);

    return candidate;
}

bool gyroCheckDataReady(gyroDev_t* gyro)
{
    bool ret;
    if (gyro->dataReady) {
        ret = true;
        gyro->dataReady = false;
    } else {
        ret = false;
    }
    return ret;
}

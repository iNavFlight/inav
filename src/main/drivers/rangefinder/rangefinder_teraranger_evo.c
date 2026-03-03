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
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "common/maths.h"

#if defined(USE_RANGEFINDER) && defined(USE_RANGEFINDER_TERARANGER_EVO_I2C)

//MANUAL
//https://www.mouser.lt/datasheet/2/944/User_Manual_for_TeraRanger_Evo_single_point_distan-1729051.pdf?srsltid=AfmBOork4pNRMVMmnZ12zIk3yt79aeE066Hq5VpcBHv4wAnv-FSBLAKc

#include "build/build_config.h"

#include "common/crc.h"

#include "drivers/time.h"
#include "drivers/bus_i2c.h"

#include "drivers/rangefinder/rangefinder.h"
#include "drivers/rangefinder/rangefinder_teraranger_evo.h"

#include "build/debug.h"

#define TERARANGER_EVO_DETECTION_CONE_DECIDEGREES             900
#define TERARANGER_EVO_DETECTION_CONE_EXTENDED_DECIDEGREES    900

#define TERARANGER_EVO_I2C_ADDRESS                    0x31
#define TERARANGER_EVO_I2C_REGISTRY_TRIGGER_READING   0x00 // Write this command to the TeraRanger Evo and after a wait of approximately 500us read the 3 byte distance response
#define TERARANGER_EVO_I2C_REGISTRY_WHO_AM_I          0x01 // Write this value to TeraRanger Evo via I2C and the device responds with 0xA
#define TERARANGER_EVO_I2C_REGISTRY_CHANGE_BASE_ADDR  0xA2 // This command assigns a base address that will be memorised by the TerRanger Evo ie. power cycling the Evo will not restore the default I2C address.

#define TERARANGER_EVO_I2C_ANSWER_WHO_AM_I            0xA1

#define TERARANGER_EVO_VALUE_TOO_CLOSE                0x0000
#define TERARANGER_EVO_VALUE_OUT_OF_RANGE             0xffff


static struct {
    int32_t     teraRangerMeasurementCm;
    uint8_t     dataBuff[3];
} teraRangerEvo = {
    .teraRangerMeasurementCm = RANGEFINDER_NO_NEW_DATA,
    .dataBuff = { 0 },
};

static void triggerNewReading(rangefinderDev_t *rangefinder){
    busWrite(rangefinder->busDev, TERARANGER_EVO_I2C_REGISTRY_TRIGGER_READING, 0x00); //request to next measure, scheduler is much slower than 500uS to we need to wait between write and read
}

static void teraRangerInit(rangefinderDev_t *rangefinder){
    triggerNewReading(rangefinder);
}

static bool deviceDetect(busDevice_t * busDev){
    for (int retry = 0; retry < 5; retry++) {
        uint8_t whoIamResult;

        delay(150);

        bool ack = busRead(busDev, TERARANGER_EVO_I2C_REGISTRY_WHO_AM_I, &whoIamResult);
        if (ack && whoIamResult == TERARANGER_EVO_I2C_ANSWER_WHO_AM_I) {
            return true;
        }
    };

    return false;
}

static bool checkCrc(void){
    return teraRangerEvo.dataBuff[2] == crc8_update(0, teraRangerEvo.dataBuff, 2);
}

void teraRangerUpdate(rangefinderDev_t *rangefinder){
    if (busReadBuf(rangefinder->busDev, TERARANGER_EVO_I2C_REGISTRY_TRIGGER_READING, teraRangerEvo.dataBuff, 3)) {
        if (!checkCrc()) {
            teraRangerEvo.teraRangerMeasurementCm = RANGEFINDER_NO_NEW_DATA;
            triggerNewReading(rangefinder);
            return;
        }

        const int32_t teraRangerMeasurementMM = ((int32_t)teraRangerEvo.dataBuff[0] << 8 | (int32_t)teraRangerEvo.dataBuff[1]);
        if (teraRangerMeasurementMM == TERARANGER_EVO_VALUE_TOO_CLOSE || teraRangerMeasurementMM == TERARANGER_EVO_VALUE_OUT_OF_RANGE) {
            teraRangerEvo.teraRangerMeasurementCm = RANGEFINDER_OUT_OF_RANGE;
            triggerNewReading(rangefinder);
            return;
        }

        teraRangerEvo.teraRangerMeasurementCm = MILLIMETERS_TO_CENTIMETERS(teraRangerMeasurementMM);
    } else {
        teraRangerEvo.teraRangerMeasurementCm = RANGEFINDER_HARDWARE_FAILURE;
    }

    triggerNewReading(rangefinder);
}


int32_t teraRangerGetDistance(rangefinderDev_t *rangefinder){
    UNUSED(rangefinder);
    return teraRangerEvo.teraRangerMeasurementCm;
}

bool teraRangerDetect(rangefinderDev_t *rangefinder){
    rangefinder->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_TERARANGER_EVO_I2C, 0, OWNER_RANGEFINDER);
    if (rangefinder->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(rangefinder->busDev)) {
        busDeviceDeInit(rangefinder->busDev);
        return false;
    }

    rangefinder->delayMs = RANGEFINDER_TERA_EVO_TASK_PERIOD_MS;
    rangefinder->maxRangeCm = RANGEFINDER_TERA_EVO_MAX_RANGE_CM;
    rangefinder->detectionConeDeciDegrees = TERARANGER_EVO_DETECTION_CONE_DECIDEGREES;
    rangefinder->detectionConeExtendedDeciDegrees = TERARANGER_EVO_DETECTION_CONE_EXTENDED_DECIDEGREES;

    rangefinder->init = &teraRangerInit;
    rangefinder->update = &teraRangerUpdate;
    rangefinder->read = &teraRangerGetDistance;

    return true;
}

#endif
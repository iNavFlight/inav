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

#ifdef USE_MAG_IST8310

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"

#include "drivers/time.h"
#include "drivers/nvic.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/light_led.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_ist8310.h"

#define IST8310_ADDRESS 0x0C

// Hardware descriptor
#if defined(MAG_I2C_BUS)
#endif

/* ist8310 Slave Address Select : default address 0x0C
 *        CAD1  |  CAD0   |  Address
 *    ------------------------------
 *         VSS   |   VSS  |  0CH
 *         VSS   |   VDD  |  0DH
 *         VDD   |   VSS  |  0EH
 *         VDD   |   VDD  |  0FH
 * if CAD1 and CAD0 are floating, I2C address will be 0EH
 *
 *
 * CTRL_REGA: Control Register 1
 * Read Write
 * Default value: 0x0A
 * 7:4  0   Reserved.
 * 3:0  DO2-DO0: Operating mode setting
 *        DO3  |  DO2 |  DO1 |  DO0 |   mode
 *    ------------------------------------------------------
 *         0   |   0  |  0   |  0   |   Stand-By mode
 *         0   |   0  |  0   |  1   |   Single measurement mode
 *                                       Others: Reserved
 *
 * CTRL_REGB: Control Register 2
 * Read Write
 * Default value: 0x0B
 * 7:4  0   Reserved.
 * 3    DREN : Data ready enable control:
 *      0: disable
 *      1: enable
 * 2    DRP: DRDY pin polarity control
 *      0: active low
 *      1: active high
 * 1    0   Reserved.
 * 0    SRST: Soft reset, perform Power On Reset (POR) routine
 *      0: no action
 *      1: start immediately POR routine
 *      This bit will be set to zero after POR routine
 */

#define IST8310_REG_STAT1 0x02
#define IST8310_DRDY_MASK 0x01
#define IST8310_REG_WHOAMI 0x00

// I2C Contorl Register
#define IST8310_REG_CNTRL1 0x0A
#define IST8310_REG_CNTRL2 0x0B
#define IST8310_REG_AVERAGE 0x41
#define IST8310_REG_PDCNTL 0x42

// Parameter
#define IST8310_ODR_SINGLE 0x01

// Device ID (ist8310 -> 0x10)
#define IST8310_CHIP_ID 0x10
#define IST8310_AVG_16  0x24
#define IST8310_PULSE_DURATION_NORMAL 0xC0
#define IST8310_RANGE_XY 16000  // mG, 1600 uT
#define IST8310_RANGE_Z  25000  // mG, 2500 uT

#define IST8310_CNTRL2_RESET 0x01
#define IST8310_CNTRL2_DRPOL 0x04
#define IST8310_CNTRL2_DRENA 0x08

static bool ist8310Init(magDev_t * mag)
{
    // A reboot without a power cycle leaves the chip as it was, and it may never measure again
    busWrite(mag->busDev, IST8310_REG_CNTRL2, IST8310_CNTRL2_RESET);
    delay(50);  // power-on reset routine: 50 ms max

    busWrite(mag->busDev, IST8310_REG_AVERAGE, IST8310_AVG_16);
    delay(5);

    busWrite(mag->busDev, IST8310_REG_PDCNTL, IST8310_PULSE_DURATION_NORMAL);
    delay(5);

    // Single measurement is the only mode the datasheet defines besides stand-by
    busWrite(mag->busDev, IST8310_REG_CNTRL1, IST8310_ODR_SINGLE);

    return true;
}

static bool ist8310Read(magDev_t * mag)
{
    // STAT1 and the data in one read: in single measurement mode the data only changes after a trigger
    uint8_t buf[7];
    if (!busReadBuf(mag->busDev, IST8310_REG_STAT1, buf, 7)) {
        return false;
    }

    uint8_t LSB2FSV = 3; // 3mG - 14 bit
    const int x = (int16_t)(buf[2] << 8 | buf[1]) * LSB2FSV;
    const int y = (int16_t)(buf[4] << 8 | buf[3]) * LSB2FSV;
    const int z = (int16_t)(buf[6] << 8 | buf[5]) * LSB2FSV;

    // Past the measuring range a sample can only be corrupt, and one spoils a compass calibration
    const bool inRange = ABS(x) <= IST8310_RANGE_XY && ABS(y) <= IST8310_RANGE_XY && ABS(z) <= IST8310_RANGE_Z;

    if ((buf[0] & IST8310_DRDY_MASK) && inRange) {
        // Invert Y axis to convert from left to right coordinate system
        mag->magADCRaw[X] =  x;
        mag->magADCRaw[Y] = -y;
        mag->magADCRaw[Z] =  z;
    }

    // Also restarts a measurement that never completed: the task is far slower than its 6 ms
    busWrite(mag->busDev, IST8310_REG_CNTRL1, IST8310_ODR_SINGLE);

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        delay(10);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, IST8310_REG_WHOAMI, &sig);

        if (ack && sig == IST8310_CHIP_ID) {
            return true;
        }
    }

    return false;
}

bool ist8310Detect(magDev_t * mag)
{
    for (uint8_t index = 0; index < 2; ++index) {
        mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_IST8310_0 + index, mag->magSensorToUse, OWNER_COMPASS);
        if (mag->busDev == NULL) {
            continue;
        }

        if (deviceDetect(mag)) {
            mag->init = ist8310Init;
            mag->read = ist8310Read;
            return true;
        } else {
            busDeviceDeInit(mag->busDev);
        }
    }

    return false;
}

#endif

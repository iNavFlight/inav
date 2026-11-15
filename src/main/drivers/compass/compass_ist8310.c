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

#define IST8310_REG_DATA 0x03
#define IST8310_REG_STAT1 0x02
#define IST8310_DRDY_MASK 0x01
#define IST8310_REG_WHOAMI 0x00

// I2C Contorl Register
#define IST8310_REG_CNTRL1 0x0A
#define IST8310_REG_CNTRL2 0x0B
#define IST8310_REG_AVERAGE 0x41
#define IST8310_REG_PDCNTL 0x42

// Parameter
// ODR = Output Data Rate, we use single measure mode for getting more data.
#define IST8310_ODR_SINGLE 0x01
#define IST8310_ODR_10_HZ 0x03
#define IST8310_ODR_20_HZ 0x05
#define IST8310_ODR_50_HZ 0x07
#define IST8310_ODR_100_HZ 0x06

// Device ID (ist8310 -> 0x10)
#define IST8310_CHIP_ID 0x10
#define IST8310_AVG_16  0x24
#define IST8310_PULSE_DURATION_NORMAL 0xC0

#define IST8310_CNTRL2_RESET 0x01
#define IST8310_CNTRL2_DRPOL 0x04
#define IST8310_CNTRL2_DRENA 0x08

static bool ist8310Init(magDev_t * mag)
{
    busWrite(mag->busDev, IST8310_REG_CNTRL1, IST8310_ODR_50_HZ);
    delay(5);

    busWrite(mag->busDev, IST8310_REG_AVERAGE, IST8310_AVG_16);
    delay(5);

    busWrite(mag->busDev, IST8310_REG_PDCNTL, IST8310_PULSE_DURATION_NORMAL);
    delay(5);

    return true;
}

// Status polls about 1 ms apart, enough to cover a whole 20 ms output period
#define IST8310_DRDY_MAX_RETRIES 25

// Targets of the non-blocking reads, filled by the bus driver in the background
static uint8_t ist8310Status;
static uint8_t ist8310Data[6];

// Two transfers per sample: the status byte first, the data only when DRDY confirms a fresh sample
static busReadStepResult_e ist8310ReadStart(magDev_t * mag, bool firstStep)
{
    static bool statusStarted = false;
    static uint8_t statusRetries = 0;

    if (firstStep) {
        statusStarted = false;
        statusRetries = 0;
        ist8310Status = 0;
    }

    if (!statusStarted) {
        if (!busReadBufStart(mag->busDev, IST8310_REG_STAT1, &ist8310Status, 1)) {
            return BUS_READ_STEP_BUSY;
        }
        statusStarted = true;
        return BUS_READ_STEP_NEXT;
    }

    // The IST8310 updates its data registers once per output-data period. Only read
    // them once DRDY is set; a read racing an update would mix old and new samples
    // across axes, corrupting the mag vector and calibration.
    if (!(ist8310Status & IST8310_DRDY_MASK)) {
        // DRDY is low while the sensor converts. The task period is a multiple of the sensor's
        // output period, so waiting a whole task period would hit the same gap again and again
        // and the values would freeze. Poll the status until the conversion is over instead.
        if (statusRetries < IST8310_DRDY_MAX_RETRIES) {
            if (!busReadBufStart(mag->busDev, IST8310_REG_STAT1, &ist8310Status, 1)) {
                return BUS_READ_STEP_BUSY;
            }
            statusRetries++;
            return BUS_READ_STEP_NEXT;
        }
        return BUS_READ_STEP_LAST;     // nothing new to fetch, ist8310Read() keeps the previous values
    }

    if (!busReadBufStart(mag->busDev, IST8310_REG_DATA, ist8310Data, sizeof(ist8310Data))) {
        return BUS_READ_STEP_BUSY;
    }

    return BUS_READ_STEP_LAST;
}

static bool ist8310Read(magDev_t * mag)
{
    // When no fresh sample is ready, return success so the caller keeps the previous values
    if (!(ist8310Status & IST8310_DRDY_MASK)) {
        return true;
    }

    const uint8_t LSB2FSV = 3; // 3mG - 14 bit

    // Invert Y axis to co convert from left to right coordinate system
    mag->magADCRaw[X] =  (int16_t)(ist8310Data[1] << 8 | ist8310Data[0]) * LSB2FSV;
    mag->magADCRaw[Y] = -(int16_t)(ist8310Data[3] << 8 | ist8310Data[2]) * LSB2FSV;
    mag->magADCRaw[Z] =  (int16_t)(ist8310Data[5] << 8 | ist8310Data[4]) * LSB2FSV;

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
            mag->readStart = ist8310ReadStart;
            mag->read = ist8310Read;
            return true;
        } else {
            busDeviceDeInit(mag->busDev);
        }
    }

    return false;
}

#endif

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

#include "platform.h"

#if defined(USE_RANGEFINDER) && defined(USE_RANGEFINDER_SRF10)

#include "build/build_config.h"


#include "drivers/time.h"
#include "drivers/bus_i2c.h"

#include "drivers/rangefinder/rangefinder.h"
#include "drivers/rangefinder/rangefinder_srf10.h"

// Technical specification is at: http://robot-electronics.co.uk/htm/srf10tech.htm
#define SRF10_MAX_RANGE_CM 600 // 6m, from SFR10 spec sheet
// see http://www.robot-electronics.co.uk/htm/sonar_faq.htm for cone angles
// FAQ states 55 degrees, conservatively reduced to 50 degrees here
#define SRF10_DETECTION_CONE_DECIDEGREES 500
#define SRF10_DETECTION_CONE_EXTENDED_DECIDEGREES 500

// from spec sheet, when range set to 1100cm
// speed of sound is 340m/s, echo time for 1100cm is 65ms
#define SRF10_MinimumFiringIntervalFor1100cmRangeMs 65
// echo time for 600cm is 36ms, round this up to 40
#define SRF10_MinimumFiringIntervalFor600cmRangeMs 40

// SRF10 hardware constants
#define SRF10_Address 0xE0
#define SRF10_AddressI2C (SRF10_Address>>1) // the I2C 7 bit address

#define SRF10_READ_SoftwareRevision 0x00
#define SRF10_READ_Unused 0x01 // (read value returned is 0x80)
#define SRF10_READ_Unused_ReturnValue 0x80
#define SRF10_READ_RangeHighByte 0x02
#define SRF10_READ_RangeLowByte 0x03

#define SRF10_WRITE_CommandRegister 0x00
#define SRF10_WRITE_MaxGainRegister 0x01 //(default gain value is 16)
#define SRF10_WRITE_RangeRegister 0x02 //(default range value is 255)

#define SRF10_COMMAND_InitiateRangingInches 0x50
#define SRF10_COMMAND_InitiateRangingCm 0x51
#define SRF10_COMMAND_InitiateRangingMicroSeconds 0x53

#define SRF10_COMMAND_SetGain_40 0x00
#define SRF10_COMMAND_SetGain_100 0x06
#define SRF10_COMMAND_SetGain_200 0x09
#define SRF10_COMMAND_SetGain_300 0x0B
#define SRF10_COMMAND_SetGain_400 0x0D
#define SRF10_COMMAND_SetGain_500 0x0E
#define SRF10_COMMAND_SetGain_600 0x0F
#define SRF10_COMMAND_SetGain_700 0x10 // default and maximum
#define SRF10_COMMAND_SetGain_Max 0x10 // default

#define SRF10_COMMAND_ChangeAddress1 0xA0
#define SRF10_COMMAND_ChangeAddress2 0xAA
#define SRF10_COMMAND_ChangeAddress3 0xA5

// The range is (RangeRegister + 1) * 43mm
#define SRF10_RangeValue43mm 0
#define SRF10_RangeValue86mm 1
#define SRF10_RangeValue1m 24
#define SRF10_RangeValue4m 93
#define SRF10_RangeValue6m 139 // maximum range
#define SRF10_RangeValue11m 0xFF // exceeds actual maximum range

STATIC_UNIT_TESTED volatile int32_t srf10measurementCm = RANGEFINDER_OUT_OF_RANGE;
static int16_t minimumFiringIntervalMs;
static uint32_t timeOfLastMeasurementMs;
static bool isSensorResponding = true;

static void srf10_init(rangefinderDev_t * rangefinder)
{
    busWrite(rangefinder->busDev, SRF10_WRITE_MaxGainRegister, SRF10_COMMAND_SetGain_Max);
    busWrite(rangefinder->busDev, SRF10_WRITE_RangeRegister, SRF10_RangeValue6m);

    // initiate first ranging command
    busWrite(rangefinder->busDev, SRF10_WRITE_CommandRegister, SRF10_COMMAND_InitiateRangingCm);

    timeOfLastMeasurementMs = millis();
}

/*
 * Start a range reading
 * Called periodically by the scheduler
 */
// Targets of the non-blocking reads, filled by the bus driver in the background
static uint8_t srf10Revision;
static uint8_t srf10RangeHigh;
static uint8_t srf10RangeLow;

typedef enum {
    SRF10_STEP_REVISION = 0,    // the revision register reads 0xFF while a ranging is in progress
    SRF10_STEP_RANGE_HIGH,
    SRF10_STEP_RANGE_LOW,
    SRF10_STEP_FIRE,            // start the next ranging
} srf10Step_e;

static timeDelta_t srf10_start_reading(rangefinderDev_t * rangefinder)
{
    static srf10Step_e step = SRF10_STEP_REVISION;
    static bool transferPending = false;
    bool busError = false;

    if (transferPending) {
        if (busIsBusy(rangefinder->busDev, &busError) && !busError) {
            return RANGEFINDER_UPDATE_RETRY_US;
        }
        transferPending = false;
        isSensorResponding = !busError;

        if (busError) {
            step = SRF10_STEP_REVISION;
            return 0;
        }

        switch (step) {
            case SRF10_STEP_REVISION:
                step = (srf10Revision != 0xFF) ? SRF10_STEP_RANGE_HIGH : SRF10_STEP_FIRE;
                break;
            case SRF10_STEP_RANGE_HIGH:
                step = SRF10_STEP_RANGE_LOW;
                break;
            case SRF10_STEP_RANGE_LOW:
                srf10measurementCm = srf10RangeHigh << 8 | srf10RangeLow;
                if (srf10measurementCm > SRF10_MAX_RANGE_CM) {
                    srf10measurementCm = RANGEFINDER_OUT_OF_RANGE;
                }
                step = SRF10_STEP_FIRE;
                break;
            default:
                step = SRF10_STEP_REVISION;
                return 0;                   // cycle complete, next one after the regular period
        }
    }

    switch (step) {
        case SRF10_STEP_REVISION:
            transferPending = busReadBufStart(rangefinder->busDev, SRF10_READ_SoftwareRevision, &srf10Revision, 1);
            return RANGEFINDER_UPDATE_RETRY_US;
        case SRF10_STEP_RANGE_HIGH:
            transferPending = busReadBufStart(rangefinder->busDev, SRF10_READ_RangeHighByte, &srf10RangeHigh, 1);
            return RANGEFINDER_UPDATE_RETRY_US;
        case SRF10_STEP_RANGE_LOW:
            transferPending = busReadBufStart(rangefinder->busDev, SRF10_READ_RangeLowByte, &srf10RangeLow, 1);
            return RANGEFINDER_UPDATE_RETRY_US;
        default: {
            const timeMs_t timeNowMs = millis();
            if (timeNowMs > timeOfLastMeasurementMs + minimumFiringIntervalMs) {
                timeOfLastMeasurementMs = timeNowMs;
                transferPending = busWriteStart(rangefinder->busDev, SRF10_WRITE_CommandRegister, SRF10_COMMAND_InitiateRangingCm);
                return RANGEFINDER_UPDATE_RETRY_US;
            }
            step = SRF10_STEP_REVISION;
            return 0;
        }
    }
}

/**
 * Get the distance that was measured by the last pulse, in centimeters.
 */
static int32_t srf10_get_distance(rangefinderDev_t *dev)
{
    UNUSED(dev);
    if (isSensorResponding) {
        return srf10measurementCm;
    }
    else {
        return RANGEFINDER_HARDWARE_FAILURE;
    }
}

static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < 5; retry++) {
        uint8_t inquiryResult;

        delay(150);

        bool ack = busRead(busDev, SRF10_READ_Unused, &inquiryResult);
        if (ack && inquiryResult == SRF10_READ_Unused_ReturnValue) {
            return true;
        }
    };

    return false;
}


bool srf10Detect(rangefinderDev_t * rangefinder)
{
    rangefinder->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_SRF10, 0, OWNER_RANGEFINDER);
    if (rangefinder->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(rangefinder->busDev)) {
        busDeviceDeInit(rangefinder->busDev);
        return false;
    }

    rangefinder->delayMs = SRF10_MinimumFiringIntervalFor600cmRangeMs + 10; // set up the SRF10 hardware for a range of 6m + margin of 10ms
    rangefinder->maxRangeCm = SRF10_MAX_RANGE_CM;
    rangefinder->detectionConeDeciDegrees = SRF10_DETECTION_CONE_DECIDEGREES;
    rangefinder->detectionConeExtendedDeciDegrees = SRF10_DETECTION_CONE_EXTENDED_DECIDEGREES;

    rangefinder->init = &srf10_init;
    rangefinder->update = &srf10_start_reading;
    rangefinder->read = &srf10_get_distance;

    return true;
}
#endif

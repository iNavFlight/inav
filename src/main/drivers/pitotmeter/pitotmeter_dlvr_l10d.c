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

#include <platform.h>
#include <build/debug.h>

#include "common/log.h"
#include "common/utils.h"
#include "common/maths.h"
#include "drivers/bus_i2c.h"
#include "drivers/time.h"
#include "drivers/pitotmeter/pitotmeter.h"

#define DLVR_L10D_ADDR                 0x28     // this var is not used !!!

// //---------------------------------------------------
// //---------------------------------------------------
// #define C_TO_KELVIN(temp) (temp + 273.15f)
// #define KELVIN_TO_C(temp) (temp - 273.15f)
// #define F_TO_KELVIN(temp) C_TO_KELVIN(((temp - 32) * 5/9))

// #define M_PER_SEC_TO_KNOTS 1.94384449f
// #define KNOTS_TO_M_PER_SEC (1/M_PER_SEC_TO_KNOTS)

// #define KM_PER_HOUR_TO_M_PER_SEC 0.27777778f

// // Gas Constant is from Aerodynamics for Engineering Students, Third Edition, E.L.Houghton and N.B.Carruthers
// #define ISA_GAS_CONSTANT 287.26f
// #define ISA_LAPSE_RATE 0.0065f

// // Standard Sea Level values
// // Ref: https://en.wikipedia.org/wiki/Standard_sea_level
// #define SSL_AIR_DENSITY         1.225f // kg/m^3
// #define SSL_AIR_PRESSURE 101325.01576f // Pascal
// #define SSL_AIR_TEMPERATURE    288.15f // K

#define INCH_OF_H2O_TO_PASCAL   248.84f
//#define INCH_OF_H2O_TO_PASCAL 249.09

#define INCH_H2O_TO_PASCAL(press) (INCH_OF_H2O_TO_PASCAL * (press))

#define RANGE_INCH_H2O      10
#define DLVR_OFFSET_CORR    -9.0f   // check for other samples of DLVR-L10D; should be 0
#define DLVR_OFFSET         8192.0f
#define DLVR_SCALE          16384.0f


typedef struct __attribute__ ((__packed__)) dlvrCtx_s {
    bool     dataValid;
    uint32_t dlvr_ut;
    uint32_t dlvr_up;
} dlvrCtx_t;

STATIC_ASSERT(sizeof(dlvrCtx_t) < BUS_SCRATCHPAD_MEMORY_SIZE, busDevice_scratchpad_memory_too_small);

static bool dlvr_start(pitotDev_t * pitot)
{
    UNUSED(pitot);
    return true;
}

static bool dlvr_read(pitotDev_t * pitot)
{
    uint8_t rxbuf1[4];
    // uint8_t rxbuf2[4];

    dlvrCtx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);
    ctx->dataValid = false;

    if (!busReadBuf(pitot->busDev, 0xFF, rxbuf1, 4)) {
        return false;
    }

    // if (!busReadBuf(pitot->busDev, 0xFF, rxbuf2, 4)) {
    //     return false;
    // }

    // status = 00 -> ok, new data
	// status = 01 -> reserved
    // status = 10 -> ok, data stale
    // status = 11 -> error
	// check the status of the first read:
    const uint8_t status = ((rxbuf1[0] & 0xC0) >> 6);
    if (status == 2 || status == 3) {
        return false;
    }

    int16_t dP_raw1, dT_raw1;
    // int16_t dP_raw2, dT_raw2;

    dP_raw1 = 0x3FFF & ((rxbuf1[0] << 8) + rxbuf1[1]);
    dT_raw1 = (0xFFE0 & ((rxbuf1[2] << 8) + rxbuf1[3])) >> 5;
    // dP_raw2 = 0x3FFF & ((rxbuf2[0] << 8) + rxbuf2[1]);
    // dT_raw2 = (0xFFE0 & ((rxbuf2[2] << 8) + rxbuf2[3])) >> 5;

    // // reject any double reads where the value has shifted in the upper more than 0xFF
    // if (ABS(dP_raw1 - dP_raw2) > 0xFF || ABS(dT_raw1 - dT_raw2) > 0xFF) {
    //     return false;
    // }

    // LOG_DEBUG( PITOT, "dP_raw1 = %f; dP_raw2 =  %f", (double)dP_raw1, (double)dP_raw2 );

    // Data valid, update ut/up values
    ctx->dataValid = true;
    // ctx->dlvr_up = (dP_raw1 + dP_raw2) / 2;
    // ctx->dlvr_ut = (dT_raw1 + dT_raw2) / 2;
    ctx->dlvr_up = dP_raw1;
    ctx->dlvr_ut = dT_raw1;
    return true;
}

static void dlvr_calculate(pitotDev_t * pitot, float *pressure, float *temperature)
{
    dlvrCtx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);

  
    // pressure in inchH2O
    float dP_inchH2O = 1.25f *  2.0f * RANGE_INCH_H2O  * (((float)ctx->dlvr_up - (DLVR_OFFSET + DLVR_OFFSET_CORR) ) / DLVR_SCALE); 

    LOG_DEBUG( PITOT, "dP_adc = %f; dP_inchH2O =  %f; dP_Pa = %f", (double)ctx->dlvr_up, (double)dP_inchH2O, (double)(INCH_H2O_TO_PASCAL( dP_inchH2O)) );

    debug[4] = (int32_t)(ctx->dlvr_up *100);
    debug[5] = (int32_t)((ctx->dlvr_up - (DLVR_OFFSET + DLVR_OFFSET_CORR)) *100);

    // temperature in deg C
    float T_C = (float)ctx->dlvr_ut * (200.0f / 2047.0f) - 50.0f;     

    //LOG_DEBUG( PITOT, "t_adc = %f; T_C = %f", (double)ctx->dlvr_ut, (double)T_C );

    // result must fit inside the range
    if ((dP_inchH2O > RANGE_INCH_H2O) || (dP_inchH2O < -RANGE_INCH_H2O)) {
        // Debug("DLVR: Out of range pressure %f", dP_inchH2O);
        return;
    }

    if (pressure) {
        *pressure = INCH_H2O_TO_PASCAL( dP_inchH2O);   // Pa
    }

    if (temperature) {
        *temperature = C_TO_KELVIN( T_C); // K
    }
}

bool dlvrDetect(pitotDev_t * pitot)
{
    uint8_t rxbuf[4];
    bool ack = false;

    pitot->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_DLVR, 0, OWNER_AIRSPEED);
    if (pitot->busDev == NULL) {
        return false;
    }

    // Read twice to fix:
    // Sending a start-stop condition without any transitions on the SCL line (no clock pulses in between) creates a
    // communication error for the next communication, even if the next start condition is correct and the clock pulse is applied.
    // An additional start condition must be sent, which results in restoration of proper communication.
    ack = busReadBuf(pitot->busDev, 0xFF, rxbuf, 4);
    ack = busReadBuf(pitot->busDev, 0xFF, rxbuf, 4);
    if (!ack) {
        return false;
    }

    // Initialize busDev data
    dlvrCtx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);
    ctx->dataValid = false;
    ctx->dlvr_ut = 0;
    ctx->dlvr_up = 0;

    // Initialize pitotDev object
    pitot->delay = 10;      // 10000
    pitot->start = dlvr_start;
    pitot->get = dlvr_read;
    pitot->calculate = dlvr_calculate;
    return true;
}

/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"
#include "build/debug.h"

#include "common/utils.h"
#include "common/maths.h"
#include "drivers/time.h"
#include "drivers/bus_i2c.h"
#include "drivers/pitotmeter/pitotmeter.h"
#include "drivers/pitotmeter/pitotmeter_ms5525.h"

// MS5525 I2C Addresses
#define MS5525_ADDR_1             0x76
#define MS5525_ADDR_2             0x77

#define CMD_RESET               0x1E // ADC reset command
#define CMD_ADC_READ            0x00 // ADC read command
#define CMD_ADC_CONV            0x40 // ADC conversion command
#define CMD_ADC_D1              0x00 // ADC D1 conversion (Pressure)
#define CMD_ADC_D2              0x10 // ADC D2 conversion (Temperature)
#define CMD_ADC_4096            0x08 // ADC OSR=4096
#define CMD_PROM_RD             0xA0 // Prom read command
#define PROM_NB                 8

typedef struct __attribute__ ((__packed__)) ms5525Ctx_s {
    uint16_t c[6]; // c1 through c6
    uint32_t up;   // up (24 bits) + step (top 8 bits)
    uint32_t ut;   // ut (24 bits)
} ms5525Ctx_t;

STATIC_ASSERT(sizeof(ms5525Ctx_t) <= BUS_SCRATCHPAD_MEMORY_SIZE, busDevice_scratchpad_memory_too_small);

static int8_t ms5525_crc(uint16_t *prom)
{
    int32_t i, j;
    uint32_t res = 0;
    uint8_t crc = prom[7] & 0xF;
    prom[7] &= 0xFF00;

    bool blankEeprom = true;

    for (i = 0; i < 16; i++) {
        if (prom[i >> 1]) {
            blankEeprom = false;
        }
        if (i & 1)
            res ^= ((prom[i >> 1]) & 0x00FF);
        else
            res ^= (prom[i >> 1] >> 8);
        for (j = 8; j > 0; j--) {
            if (res & 0x8000)
                res ^= 0x1800;
            res <<= 1;
        }
    }
    prom[7] |= crc;
    if (!blankEeprom && crc == ((res >> 12) & 0xF))
        return 0;

    return -1;
}

static bool ms5525_read_adc(pitotDev_t *pitot, uint32_t *result)
{
    uint8_t rxbuf[3];
    if (busReadBuf(pitot->busDev, CMD_ADC_READ, rxbuf, 3)) {
        *result = (rxbuf[0] << 16) | (rxbuf[1] << 8) | rxbuf[2];
        return true;
    }
    return false;
}

static bool ms5525_start(pitotDev_t * pitot)
{
    ms5525Ctx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);
    uint8_t step = ctx->up >> 24;
    
    if (step == 0) {
        return busWrite(pitot->busDev, CMD_ADC_CONV + CMD_ADC_D1 + CMD_ADC_4096, 1);
    } else {
        return busWrite(pitot->busDev, CMD_ADC_CONV + CMD_ADC_D2 + CMD_ADC_4096, 1);
    }
}

static bool ms5525_read(pitotDev_t * pitot)
{
    ms5525Ctx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);
    uint8_t step = ctx->up >> 24;
    
    uint32_t adc_val = 0;
    if (!ms5525_read_adc(pitot, &adc_val)) {
        return false;
    }
    
    if (step == 0) {
        ctx->up = adc_val | (1 << 24);
        return true; 
    } else {
        ctx->ut = adc_val;
        ctx->up &= 0x00FFFFFF; // clear step back to 0
        return true; 
    }
}

static void ms5525_calculate(pitotDev_t * pitot, float *pressure, float *temperature)
{
    ms5525Ctx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);

    uint32_t up = ctx->up & 0x00FFFFFF;
    uint32_t ut = ctx->ut;

    if (up == 0 || ut == 0) {
        return; // Wait until both are read at least once
    }

    // 5525DSO-pp001DS coefficients (1 psi):
    // Q1=15, Q2=17, Q3=7, Q4=5, Q5=7, Q6=21
    int64_t dT   = (int64_t)ut - ((int64_t)ctx->c[4] << 7); // c[5] is c[4] (0-indexed)
    int64_t off  = ((int64_t)ctx->c[1] << 17) + (((int64_t)ctx->c[3] * dT) >> 5); // c[2]=c[1], c[4]=c[3]
    int64_t sens = ((int64_t)ctx->c[0] << 15) + (((int64_t)ctx->c[2] * dT) >> 7); // c[1]=c[0], c[3]=c[2]
    int64_t temp = 2000 + ((dT * (int64_t)ctx->c[5]) >> 21); // c[6]=c[5]

    int64_t p_raw = ((((int64_t)up * sens) >> 21) - off) >> 15;
    
    // Convert 1 PSI sensor output to Pa
    // 1 psi = 6894.757 Pa. p_raw is 0.0001 psi per bit -> 0.6894757 Pa per bit.
    
    if (pressure) {
        *pressure = (float)p_raw * 0.6894757f;
    }

    if (temperature) {
        *temperature = C_TO_KELVIN(temp / 100.0f);
    }
}

static bool deviceDetect(busDevice_t * dev)
{
    // Verify an I2C transaction works: read PROM
    uint8_t rxbuf[2];
    bool ack = busReadBuf(dev, CMD_PROM_RD, rxbuf, 2);
    if (ack) {
        return true;
    }
    return false;
}

bool ms5525Detect(pitotDev_t * pitot)
{
    pitot->busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_MS5525, 0, OWNER_AIRSPEED);
    if (pitot->busDev == NULL) {
        return false;
    }

    // Try primary address 0x76
    pitot->busDev->busdev.i2c.address = MS5525_ADDR_1;
    if (!deviceDetect(pitot->busDev)) {
        // Fallback to secondary 0x77
        pitot->busDev->busdev.i2c.address = MS5525_ADDR_2;
        if (!deviceDetect(pitot->busDev)) {
            busDeviceDeInit(pitot->busDev);
            return false;
        }
    }

    // Sensor found, initialize
    busWrite(pitot->busDev, CMD_RESET, 1);
    delay(5);

    ms5525Ctx_t * ctx = busDeviceGetScratchpadMemory(pitot->busDev);
    
    // Read PROM 
    uint16_t prom[8];
    for (int i = 0; i < PROM_NB; i++) {
        uint8_t rxbuf[2] = { 0, 0 };
        busReadBuf(pitot->busDev, CMD_PROM_RD + i * 2, rxbuf, 2);
        prom[i] = (rxbuf[0] << 8 | rxbuf[1]);
    }

    // Check CRC
    if (ms5525_crc(prom) != 0) {
        busDeviceDeInit(pitot->busDev);
        return false;
    }

    // Copy to ctx starting from c1 to c6
    for (int i = 0; i < 6; i++) {
        ctx->c[i] = prom[i+1];
    }

    // Setup Context
    ctx->up = 0;
    ctx->ut = 0;

    // Pitot delays
    pitot->delay = 10000; // max 9.04ms for OSR4096
    pitot->calibThreshold = 0.00005f;
    pitot->start = ms5525_start;
    pitot->get = ms5525_read;
    pitot->calculate = ms5525_calculate;

    return true;
}

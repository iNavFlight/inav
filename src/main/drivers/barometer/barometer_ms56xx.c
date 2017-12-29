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

#include "build/build_config.h"
#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/time.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_ms56xx.h"

#if defined(USE_BARO_MS5607) || defined(USE_BARO_MS5611)

// MS56xx, Standard address 0x77
#define MS56XX_ADDR             0x77

#define CMD_RESET               0x1E // ADC reset command
#define CMD_ADC_READ            0x00 // ADC read command
#define CMD_ADC_CONV            0x40 // ADC conversion command
#define CMD_ADC_D1              0x00 // ADC D1 conversion
#define CMD_ADC_D2              0x10 // ADC D2 conversion
#define CMD_ADC_256             0x00 // ADC OSR=256
#define CMD_ADC_512             0x02 // ADC OSR=512
#define CMD_ADC_1024            0x04 // ADC OSR=1024
#define CMD_ADC_2048            0x06 // ADC OSR=2048
#define CMD_ADC_4096            0x08 // ADC OSR=4096
#define CMD_PROM_RD             0xA0 // Prom read command
#define PROM_NB                 8

STATIC_UNIT_TESTED uint32_t ms56xx_ut;  // static result of temperature measurement
STATIC_UNIT_TESTED uint32_t ms56xx_up;  // static result of pressure measurement
STATIC_UNIT_TESTED uint16_t ms56xx_c[PROM_NB];  // on-chip ROM
static uint8_t ms56xx_osr = CMD_ADC_4096;

STATIC_UNIT_TESTED int8_t ms56xx_crc(uint16_t *prom)
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

static uint32_t ms56xx_read_adc(baroDev_t *baro)
{
    uint8_t rxbuf[3];
    busReadBuf(baro->busDev, CMD_ADC_READ, rxbuf, 3);
    return (rxbuf[0] << 16) | (rxbuf[1] << 8) | rxbuf[2];
}

static bool ms56xx_start_ut(baroDev_t *baro)
{
    return busWrite(baro->busDev, CMD_ADC_CONV + CMD_ADC_D2 + ms56xx_osr, 1);
}

static bool ms56xx_get_ut(baroDev_t *baro)
{
    ms56xx_ut = ms56xx_read_adc(baro);
    return true;
}

static bool ms56xx_start_up(baroDev_t *baro)
{
    return busWrite(baro->busDev, CMD_ADC_CONV + CMD_ADC_D1 + ms56xx_osr, 1);
}

static bool ms56xx_get_up(baroDev_t *baro)
{
    ms56xx_up = ms56xx_read_adc(baro);
    return true;
}

#ifdef USE_BARO_MS5611
STATIC_UNIT_TESTED bool ms5611_calculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);
    uint32_t press;
    int64_t temp;
    int64_t delt;
    int64_t dT = (int64_t)ms56xx_ut - ((uint64_t)ms56xx_c[5] * 256);
    int64_t off = ((int64_t)ms56xx_c[2] << 16) + (((int64_t)ms56xx_c[4] * dT) >> 7);
    int64_t sens = ((int64_t)ms56xx_c[1] << 15) + (((int64_t)ms56xx_c[3] * dT) >> 8);
    temp = 2000 + ((dT * (int64_t)ms56xx_c[6]) >> 23);

    if (temp < 2000) { // temperature lower than 20degC
        delt = temp - 2000;
        delt = 5 * delt * delt;
        off -= delt >> 1;
        sens -= delt >> 2;
        if (temp < -1500) { // temperature lower than -15degC
            delt = temp + 1500;
            delt = delt * delt;
            off -= 7 * delt;
            sens -= (11 * delt) >> 1;
        }
    temp -= ((dT * dT) >> 31);
    }
    press = ((((int64_t)ms56xx_up * sens) >> 21) - off) >> 15;

    if (pressure)
        *pressure = press;
    if (temperature)
        *temperature = temp;

    return true;
}
#endif

#ifdef USE_BARO_MS5607
STATIC_UNIT_TESTED bool ms5607_calculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);
    uint32_t press;
    int64_t temp;
    int64_t delt;
    int64_t dT = (int64_t)ms56xx_ut - ((uint64_t)ms56xx_c[5] << 8);
    int64_t off = ((int64_t)ms56xx_c[2] << 17) + (((int64_t)ms56xx_c[4] * dT) >> 6);
    int64_t sens = ((int64_t)ms56xx_c[1] << 16) + (((int64_t)ms56xx_c[3] * dT) >> 7);
    temp = 2000 + ((dT * (int64_t)ms56xx_c[6]) >> 23);

    if (temp < 2000) { // temperature lower than 20degC
        delt = temp - 2000;
        delt = delt * delt;
        off -= (61 * delt) >> 4;
        sens -= 2 * delt;
        if (temp < -1500) { // temperature lower than -15degC
            delt = temp + 1500;
            delt = delt * delt;
            off -= 15 * delt;
            sens -= 8 * delt;
        }
        temp -= ((dT * dT) >> 31);
    }
    press = ((((int64_t)ms56xx_up * sens) >> 21) - off) >> 15;

    if (pressure)
        *pressure = press;
    if (temperature)
        *temperature = temp;

    return true;
}
#endif

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * dev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        uint8_t sig = 0;

        delay(10);

        bool ack = busRead(dev, CMD_PROM_RD, &sig);
        if (ack && sig != 0xFF) {
            return true;
        }
    };

    return false;
}

static bool deviceInit(baroDev_t *baro)
{
    busSetSpeed(baro->busDev, BUS_SPEED_STANDARD);

    busWrite(baro->busDev, CMD_RESET, 1);
    delay(5);

    // read all coefficients
    for (int i = 0; i < PROM_NB; i++) {
        uint8_t rxbuf[2] = { 0, 0 };
        busReadBuf(baro->busDev, CMD_PROM_RD + i * 2, rxbuf, 2);
        ms56xx_c[i] = (rxbuf[0] << 8 | rxbuf[1]);
    }

    // check crc, bail out if wrong - we are probably talking to BMP085 w/o XCLR line!
    if (ms56xx_crc(ms56xx_c) != 0) {
        return false;
    }

    baro->ut_delay = 10000;
    baro->up_delay = 10000;
    baro->start_ut = ms56xx_start_ut;
    baro->get_ut = ms56xx_get_ut;
    baro->start_up = ms56xx_start_up;
    baro->get_up = ms56xx_get_up;

    return true;
}

#ifdef USE_BARO_MS5607
bool ms5607Detect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_MS5607, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    if (!deviceInit(baro)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    baro->calculate = ms5607_calculate;

    return true;
}
#endif

#ifdef USE_BARO_MS5611
bool ms5611Detect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_MS5611, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    if (!deviceInit(baro)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    baro->calculate = ms5611_calculate;

    return true;
}
#endif

#endif

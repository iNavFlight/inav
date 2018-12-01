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

 ******************************************************************************
 * @brief   contains definitions from the ST MEMS team
 * @author  MEMS Application Team
 * @version V1.0.0
 * @date    11-February-2015
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 * */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <platform.h>
#include "build/build_config.h"
#include "build/debug.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_lps25h.h"

#if defined(USE_BARO_LPS25H)

#define LPS25HB_WHO_AM_I_ADDR               0x0F
#define LPS25HB_RES_CONF_ADDR               0x10
#define LPS25HB_CTRL_REG1_ADDR              0x20
#define LPS25HB_CTRL_REG2_ADDR              0x21
#define LPS25HB_STATUS_REG_ADDR             0x27
#define LPS25HB_PRESS_POUT_XL_ADDR          0x28
#define LPS25HB_PRESS_OUT_L_ADDR            0x29
#define LPS25HB_PRESS_OUT_H_ADDR            0x2A
#define LPS25HB_TEMP_OUT_L_ADDR             0x2B
#define LPS25HB_TEMP_OUT_H_ADDR             0x2C

#define LPS25HB_I2C_MULTIPLEBYTE_CMD        0x80
#define LPS25HB_SPI_MULTIPLEBYTE_CMD        0x40

#define TEMPERATURE_READY_MASK              (1 << 0)
#define PRESSURE_READY_MASK                 (1 << 1)

#define I_AM_LPS25HB                        0xBD

#define LPS25HB_MODE_MASK                   0x80
#define LPS25HB_MODE_POWERDOWN              0x00
#define LPS25HB_MODE_ACTIVE                 0x80

#define LPS25HB_ODR_MASK                    0x70
#define LPS25HB_ODR_ONE_SHOT                0x00
#define LPS25HB_ODR_1Hz                     0x10
#define LPS25HB_ODR_7Hz                     0x20
#define LPS25HB_ODR_12_5Hz                  0x30
#define LPS25HB_ODR_25Hz                    0x40

#define LPS25HB_DIFF_EN_MASK                0x08
#define LPS25HB_DIFF_DISABLE                0x00
#define LPS25HB_DIFF_ENABLE                 0x08

#define LPS25HB_BDU_MASK                    0x04
#define LPS25HB_BDU_CONT                    0x00
#define LPS25HB_BDU_READ                    0x04


#define LPS25HB_RESET_MEMORY_MASK           0x80
#define LPS25HB_NORMAL_MODE                 0x00
#define LPS25HB_RESET_MEMORY                0x80


#define LPS25HB_P_RES_MASK                  0x03
#define LPS25HB_P_RES_AVG_8                 0x00
#define LPS25HB_P_RES_AVG_32                0x01
#define LPS25HB_P_RES_AVG_128               0x02
#define LPS25HB_P_RES_AVG_512               0x03

#define LPS25HB_T_RES_MASK                  0x0C
#define LPS25HB_T_RES_AVG_8                 0x00
#define LPS25HB_T_RES_AVG_16                0x04
#define LPS25HB_T_RES_AVG_32                0x08
#define LPS25HB_T_RES_AVG_64                0x0C

#define LPS25H_CHIP_ID                      0xBD

static int32_t pressureRaw;
static int32_t temperatureRaw;

static bool lps25hbRebootCmd(baroDev_t * baro)
{
    uint8_t tmpreg;

    /* Read CTRL_REG5 register */
    bool ack = busRead(baro->busDev, LPS25HB_CTRL_REG2_ADDR, &tmpreg);
    if (!ack) {
        return false;
    }

    /* Enable or Disable the reboot memory */
    tmpreg |= LPS25HB_RESET_MEMORY;

    /* Write value to MEMS CTRL_REG5 regsister */
    ack = busWrite(baro->busDev, LPS25HB_CTRL_REG2_ADDR, tmpreg);
    if (!ack) {
        return false;
    }

    return true;
}

static bool lps25hbPower(baroDev_t * baro, bool enable)
{
    uint8_t tmpreg;

    /* Read the register content */
    bool ack = busRead(baro->busDev, LPS25HB_CTRL_REG1_ADDR, &tmpreg);
    if (!ack) {
        return false;
    }

    tmpreg &= ~LPS25HB_MODE_MASK;

    if (enable) {
        tmpreg |= LPS25HB_MODE_ACTIVE;
    }
    else {
        tmpreg |= LPS25HB_MODE_POWERDOWN;
    }

    /* Write register */
    ack = busWrite(baro->busDev, LPS25HB_CTRL_REG1_ADDR, tmpreg);
    if (!ack) {
        return false;
    }

    return true;
}

static bool lps25hbInit(baroDev_t * baro)
{
    uint8_t tmp1 = 0x00;

    /* Configure the low level interface ---------------------------------------*/
    if (!lps25hbRebootCmd(baro)) {
        return false;
    }

    delay(100);

    if (!lps25hbPower(baro, false)) {
        return false;
    }

    bool ack = busRead(baro->busDev, LPS25HB_CTRL_REG1_ADDR, &tmp1);
    if (!ack) {
        return false;
    }

    /* Output Data Rate selection */
    tmp1 = (tmp1 & (~(LPS25HB_ODR_MASK))) | LPS25HB_ODR_25Hz;

    /* Interrupt circuit selection */
    tmp1 = (tmp1 & (~(LPS25HB_DIFF_EN_MASK))) | LPS25HB_DIFF_ENABLE;

    /* Block Data Update selection - continuous mode */
    tmp1 = (tmp1 & (~(LPS25HB_BDU_MASK))) | LPS25HB_BDU_CONT;

    ack = busWrite(baro->busDev, LPS25HB_CTRL_REG1_ADDR, tmp1);
    if (!ack) {
        return false;
    }

    tmp1 = 0;
    ack = busRead(baro->busDev, LPS25HB_RES_CONF_ADDR, &tmp1);
    if (!ack) {
        return false;
    }

    // PressureResolution
    tmp1 = (tmp1 & (~(LPS25HB_P_RES_MASK))) | LPS25HB_P_RES_AVG_512;

    // TemperatureResolution
    tmp1 = (tmp1 & (~(LPS25HB_T_RES_MASK))) | LPS25HB_T_RES_AVG_32;

    ack = busWrite(baro->busDev, LPS25HB_RES_CONF_ADDR, tmp1);
    if (!ack) {
        return false;
    }

    if (!lps25hbPower(baro, true)) {
        return false;
    }

    return true;
}


// check the status register bit to be present
static bool isDataReady(baroDev_t * baro, uint8_t bitMask)
{
    uint8_t tmp = 0;

    bool ack = busRead(baro->busDev, LPS25HB_STATUS_REG_ADDR, &tmp);
    if (!ack) {
        return false;
    }

    return (tmp & bitMask);
}

static bool lps25h_start_up(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}

static bool lps25h_get_up(baroDev_t * baro)
{
    uint8_t buffer[3] = {0};

    if (isDataReady(baro, PRESSURE_READY_MASK)) {
        if (baro->busDev->busType == BUSTYPE_SPI) {
            busReadBuf(baro->busDev, LPS25HB_SPI_MULTIPLEBYTE_CMD | LPS25HB_PRESS_POUT_XL_ADDR, buffer, sizeof(buffer));
        }
        else {
            busReadBuf(baro->busDev, LPS25HB_I2C_MULTIPLEBYTE_CMD | LPS25HB_PRESS_POUT_XL_ADDR, buffer, sizeof(buffer));
        }

        pressureRaw = (int32_t)((uint32_t)buffer[2] << 16 | (uint32_t)buffer[1] << 8 | (uint32_t)buffer[0]);
    }

    return true;
}

static bool lps25h_start_ut(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}

static bool lps25h_get_ut(baroDev_t * baro)
{
    uint8_t data[2] = {0};

    if (isDataReady(baro, TEMPERATURE_READY_MASK)) {
        if (baro->busDev->busType == BUSTYPE_SPI) {
            busReadBuf(baro->busDev, LPS25HB_SPI_MULTIPLEBYTE_CMD | LPS25HB_TEMP_OUT_L_ADDR, data, sizeof(data));
        }
        else {
            busReadBuf(baro->busDev, LPS25HB_I2C_MULTIPLEBYTE_CMD | LPS25HB_TEMP_OUT_L_ADDR, data, sizeof(data));
        }

        temperatureRaw = (int16_t)((uint16_t)data[1] << 8 | (uint16_t)data[0]);
    }

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        uint8_t chipId = 0;

        delay(100);

        bool ack = busRead(busDev, LPS25HB_WHO_AM_I_ADDR, &chipId);
        // verified: id is 189 (0xBD)

        if (ack && chipId == LPS25H_CHIP_ID) {
            return true;
        }
    };

    return false;
}

STATIC_UNIT_TESTED bool lps25h_calculate(baroDev_t * baro, int32_t * pressure, int32_t * temperature)
{
    UNUSED(baro);

    if (pressure) {
        // Pout(hPa) = PRESS_OUT / 4096
        // Pout(Pa) = Pout(hPa) * 100
        *pressure = pressureRaw * 100 / 4096;
    }

    if (temperature) {
        // We need temperature in 0.01 degC
        *temperature = 4250 + temperatureRaw * 100 / 480;
    }

    return true;
}

bool lps25hDetect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_LPS25H, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    busSetSpeed(baro->busDev, BUS_SPEED_STANDARD);

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    lps25hbInit(baro);

    baro->ut_delay = 0;
    baro->get_ut = lps25h_get_ut;
    baro->start_ut = lps25h_start_ut;

    baro->up_delay = 1000;
    baro->start_up = lps25h_start_up;
    baro->get_up = lps25h_get_up;

    baro->calculate = lps25h_calculate;

    return true;
}


#endif

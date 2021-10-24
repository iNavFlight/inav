/*
 * This file is part of iNav.
 *
 * iNav is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * iNav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with iNav.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "platform.h"

#ifdef USE_MAG_MLX90393

#include <stdbool.h>
#include <stdint.h>

#include <math.h>

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#define MLX90393_MEASURE_3D                         0b00001110

#define MLX90393_NOP                                0b00000000
#define MLX90393_START_BURST_MODE                   0b00010000 //uses with zyxt flags
#define MLX90393_START_WAKE_UP_ON_CHANGE_MODE       0b00100000 //uses with zyxt flags
#define MLX90393_START_SINGLE_MEASUREMENT_MODE      0b00110000 //uses with zyxt flags
#define MLX90393_READ_MEASUREMENT                   0b01000000 //uses with zyxt flags
#define MLX90393_READ_REGISTER                      0b01010000
#define MLX90393_WRITE_REGISTER                     0b01100000
#define MLX90393_EXIT_MODE                          0b10000000
#define MLX90393_MEMORY_RECALL                      0b11010000
#define MLX90393_MEMORY_STORE                       0b11100000
#define MLX90393_RESET                              0b11110000

#define MLX90393_BURST_MODE_FLAG                    0b10000000
#define MLX90393_WAKE_UP_ON_CHANGE_MODE_FLAG        0b01000000
#define MLX90393_SINGLE_MEASUREMENT_MODE_FLAG       0b00100000
#define MLX90393_ERROR_FLAG                         0b00010000
#define MLX90393_SINGLE_ERROR_DETECTION_FLAG        0b00001000
#define MLX90393_RESET_FLAG                         0b00000100
#define MLX90393_D1_FLAG                            0b00000010
#define MLX90393_D0_FLAG                            0b00000001

#define DETECTION_MAX_RETRY_COUNT   5

#define REG_BUF_LEN                 3

// Register 1
#define GAIN_SEL_HALLCONF_REG       0x0 //from datasheet 0x0 << 2 = 0x0 
// GAIN - 0b111
// Hall_conf - 0xC
#define GAIN_SEL_HALLCONF_REG_VALUE 0x007C
// Register 2
#define TCMP_EN_REG                 0x4 //from datasheet 0x1 << 2 = 0x4
// Burst Data Rate 0b000100
#define TCMP_EN_REG_VALUE           0x62C4
// Register 3
#define RES_XYZ_OSR_FLT_REG         0x8 //from datasheet 0x2 << 2 = 0x8 
// Oversampling 0b01
// Filtering 0b111
#define RES_XYZ_OSR_FLT_REG_VALUE   0x001D


static void mlx_write_register(magDev_t * mag, uint8_t reg_val, uint16_t value) {

    uint8_t buf[REG_BUF_LEN] = {0};

    buf[0] = (value >> 8) & 0xFF;
    buf[1] = (value >> 0) & 0xFF;
    buf[2] = reg_val;

    busWriteBuf(mag->busDev, MLX90393_WRITE_REGISTER, buf, REG_BUF_LEN);

    // PAUSE
    delay(20);
    // To skip ACK FLAG of Write
    uint8_t sig = 0;
    busRead(mag->busDev, MLX90393_NOP, &sig);

    return;
}

// =======================================================================================
static bool mlx90393Read(magDev_t * mag)
{

    uint8_t buf[7] = {0};

    busReadBuf(mag->busDev, MLX90393_READ_MEASUREMENT | MLX90393_MEASURE_3D, buf, 7);

    mag->magADCRaw[X] = ((short)(buf[1] << 8 | buf[2]));
    mag->magADCRaw[Y] = ((short)(buf[3] << 8 | buf[4]));
    mag->magADCRaw[Z] = ((short)(buf[5] << 8 | buf[6]));

    return true;

}

static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        uint8_t sig = 0;

        bool ack = busRead(mag->busDev, MLX90393_RESET, &sig);
        delay(20);

        if (ack && ((sig & MLX90393_RESET_FLAG) == MLX90393_RESET_FLAG)) { // Check Reset Flag -> MLX90393
            return true;
        }
    }
    return false;
}

//--------------------------------------------------
static bool mlx90393Init(magDev_t * mag)
{
    uint8_t sig = 0;

    delay(20);
    // Remove reset flag
    busRead(mag->busDev, MLX90393_NOP, &sig);
    // Exit if already in burst mode. (For example when external i2c source power the bus.)
    busRead(mag->busDev, MLX90393_EXIT_MODE, &sig);

    // Writing Registers
    mlx_write_register(mag, GAIN_SEL_HALLCONF_REG, GAIN_SEL_HALLCONF_REG_VALUE);
    mlx_write_register(mag, TCMP_EN_REG, TCMP_EN_REG_VALUE);
    mlx_write_register(mag, RES_XYZ_OSR_FLT_REG, RES_XYZ_OSR_FLT_REG_VALUE);

    // Start burst mode
    busRead(mag->busDev, MLX90393_START_BURST_MODE | MLX90393_MEASURE_3D, &sig);

    return true;
}

// ==========================================================================

bool mlx90393Detect(magDev_t * mag)
{
    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_MLX90393, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(mag)) {
        busDeviceDeInit(mag->busDev);
        return false;
    }

    mag->init = mlx90393Init;
    mag->read = mlx90393Read;

    return true;
}


#endif

/*
    MCP9808 for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#define I2C_HELPERS_AUTOMATIC_DRV TRUE

#include "hal.h"
#include "i2c_helpers.h"
#include "mcp9808.h"

// http://www.mouser.com/ds/2/268/25095A-15487.pdf
// http://ww1.microchip.com/downloads/en/DeviceDoc/25095A.pdf


/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* I2C Register */
#define MCP9808_REG_CONFIG             0x01
#define MCP9808_REG_UPPER_TEMP         0x02
#define MCP9808_REG_LOWER_TEMP         0x03
#define MCP9808_REG_CRIT_TEMP          0x04
#define MCP9808_REG_AMBIENT_TEMP       0x05
#define MCP9808_REG_MANUF_ID           0x06
#define MCP9808_REG_DEVICE_ID          0x07
#define MCP9808_REG_RESOLUTION         0x08

/* Config */
#define MCP9808_REG_CONFIG_SHUTDOWN    0x0100
#define MCP9808_REG_CONFIG_CRITLOCKED  0x0080
#define MCP9808_REG_CONFIG_WINLOCKED   0x0040
#define MCP9808_REG_CONFIG_INTCLR      0x0020
#define MCP9808_REG_CONFIG_ALERTSTAT   0x0010
#define MCP9808_REG_CONFIG_ALERTCTRL   0x0008
#define MCP9808_REG_CONFIG_ALERTSEL    0x0002
#define MCP9808_REG_CONFIG_ALERTPOL    0x0002
#define MCP9808_REG_CONFIG_ALERTMODE   0x0001

/* Device Id */
#define MCP9808_MANUF_ID               0x0054
#define MCP9808_DEVICE_ID              0x0400

/* Resolution */
#define MCP9808_RES_2                  0x00  /* 1/2  = 0.5    */
#define MCP9808_RES_4                  0x01  /* 1/4  = 0.25   */
#define MCP9808_RES_8                  0x10  /* 1/8  = 0.125  */
#define MCP9808_RES_16                 0x11  /* 1/16 = 0.0625 */

/* Time in milli-seconds */
#define MCP9808_DELAY_ACQUIRE_RES_2   30
#define MCP9808_DELAY_ACQUIRE_RES_4   65
#define MCP9808_DELAY_ACQUIRE_RES_8  130
#define MCP9808_DELAY_ACQUIRE_RES_16 250

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static inline msg_t
_apply_config(MCP9808_drv *drv) {
    struct __attribute__((packed)) {
	uint8_t  reg;
	uint16_t conf;
    } tx = { MCP9808_REG_CONFIG, cpu_to_be16(drv->cfg) };
    
    return i2c_send((uint8_t*)&tx, sizeof(tx));
}

static inline msg_t
_decode_measure(MCP9808_drv *drv,
	uint16_t val, float *temperature) {
    
    /* Temperature */
    if (temperature) {
	float temp   = val & 0x0fff;
	if (val & 0x1000) temp -= 0x1000;

	float factor = 16.0F;
	switch(drv->resolution) {
	case RES_2 : factor =  2.0F; break;
	case RES_4 : factor =  4.0F; break;
	case RES_8 : factor =  8.0F; break;
	case RES_16: factor = 16.0F; break;
	}

	*temperature = temp / factor;
    }

    /* Ok */
    return MSG_OK;
}


/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void
MCP9808_init(MCP9808_drv *drv, MCP9808_config *config) {
    drv->config      = config;
    drv->cfg         = 0;
    drv->resolution  = RES_16; /* power up default */
    drv->state       = SENSOR_INIT;
}

msg_t
MCP9808_check(MCP9808_drv *drv) {
    uint16_t manuf, device;

    msg_t msg;
    if (((msg = i2c_reg_recv16_be(MCP9808_REG_MANUF_ID,  &manuf )) < MSG_OK) ||
	((msg = i2c_reg_recv16_be(MCP9808_REG_DEVICE_ID, &device)) < MSG_OK))
	return msg;
    
    if ((manuf != MCP9808_MANUF_ID) || (device != MCP9808_DEVICE_ID))
	return SENSOR_NOTFOUND;

    return MSG_OK;
}

msg_t
MCP9808_setResolution(MCP9808_drv *drv, MCP9808_resolution_t res) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t resolution;
    } tx = { MCP9808_REG_RESOLUTION, res };

    msg_t msg;
    if ((msg = i2c_send((uint8_t*)&tx, sizeof(tx))) < MSG_OK)
	return msg;

    drv->resolution = res;
    return MSG_OK;
}

msg_t
MCP9808_start(MCP9808_drv *drv) {
    drv->cfg &= ~(MCP9808_REG_CONFIG_SHUTDOWN);
    return _apply_config(drv);
}

msg_t
MCP9808_stop(MCP9808_drv *drv) {
    drv->cfg |= (MCP9808_REG_CONFIG_SHUTDOWN);
    return _apply_config(drv);
}

unsigned int
MCP9808_getAcquisitionTime(MCP9808_drv *drv) {
    switch(drv->resolution) {
    case RES_2 : return MCP9808_DELAY_ACQUIRE_RES_2;
    case RES_4 : return MCP9808_DELAY_ACQUIRE_RES_4;
    case RES_8 : return MCP9808_DELAY_ACQUIRE_RES_8;
    case RES_16: return MCP9808_DELAY_ACQUIRE_RES_16;
    }
    osalDbgAssert(false, "OOPS");
    return 0;
}

msg_t
MCP9808_readMeasure(MCP9808_drv *drv,
	float *temperature) {

    msg_t msg;
    uint16_t val;

    if ((msg = i2c_reg_recv16_be(MCP9808_REG_AMBIENT_TEMP, &val)) < MSG_OK)
	return msg;

    return _decode_measure(drv, val, temperature);    
}


msg_t
MCP9808_readTemperature(MCP9808_drv *drv,
	float *temperature) {
    osalDbgAssert(drv->state == SENSOR_STARTED, "invalid state");

    msg_t msg;
    uint16_t val;

    if ((msg = i2c_reg_recv16_be(MCP9808_REG_AMBIENT_TEMP, &val)) < MSG_OK)
	return msg;

    return _decode_measure(drv, val, temperature);    
}

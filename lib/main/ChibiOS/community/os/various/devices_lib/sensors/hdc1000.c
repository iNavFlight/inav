/*
    HDC100x for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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

/**
 * @file    HDC1000.c
 * @brief   HDC1000 interface module code.
 *
 * @addtogroup hdc1000
 * @{
 */

#define I2C_HELPERS_AUTOMATIC_DRV TRUE

#include "hal.h"
#include "i2c_helpers.h"
#include "hdc1000.h"

/* DOC: http://www.ti.com/lit/ds/symlink/hdc1008.pdf
 */

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* I2C Register */
#define HDC1000_REG_TEMP_HUMID     0x00
#define HDC1000_REG_TEMP           0x00
#define HDC1000_REG_HUMID          0x01
#define HDC1000_REG_CONFIG         0x02
#define HDC1000_REG_SERIAL         0xFB
#define HDC1000_REG_SERIAL_1       0xFB
#define HDC1000_REG_SERIAL_2       0xFC
#define HDC1000_REG_SERIAL_3       0xFD
#define HDC1000_REG_MANUF_ID       0xFE
#define HDC1000_REG_DEVICE_ID      0xFF

/* Configuration */
#define HDC1000_CONFIG_RST         (1 << 15)
#define HDC1000_CONFIG_HEATER      (1 << 13)
#define HDC1000_CONFIG_MODE_ONE    (0 << 12)
#define HDC1000_CONFIG_MODE_BOTH   (1 << 12)
#define HDC1000_CONFIG_BATT        (1 << 11)
#define HDC1000_CONFIG_TRES_14     (0)
#define HDC1000_CONFIG_TRES_11     (1 << 10)
#define HDC1000_CONFIG_HRES_14     (0)
#define HDC1000_CONFIG_HRES_11     (1 << 8)
#define HDC1000_CONFIG_HRES_8      (1 << 9)

/* Value */
#define HDC1000_MANUF_ID           0x5449
#define HDC1000_DEVICE_ID          0x1000

/* Delay in micro seconds */
#define HDC1000_DELAY_ACQUIRE_SAFETY     1000
#define HDC1000_DELAY_ACQUIRE_TRES_14    6350
#define HDC1000_DELAY_ACQUIRE_TRES_11    3650
#define HDC1000_DELAY_ACQUIRE_HRES_14    6500
#define HDC1000_DELAY_ACQUIRE_HRES_11    3850
#define HDC1000_DELAY_ACQUIRE_HRES_8     2500
#define HDC1000_DELAY_STARTUP           15000

// Deefault config (high res)
#define HDC1000_CONFIG_RES    (HDC1000_CONFIG_TRES_14        |		\
			       HDC1000_CONFIG_HRES_14)
#define HDC1000_DELAY_ACQUIRE (HDC1000_DELAY_ACQUIRE_TRES_14 + 		\
			       HDC1000_DELAY_ACQUIRE_HRES_14)

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
_apply_config(HDC1000_drv *drv) {
    struct __attribute__((packed)) {
	uint8_t  reg;
	uint16_t conf;
    } tx = { HDC1000_REG_CONFIG, cpu_to_be16(drv->cfg) };
    
    return i2c_send((uint8_t*)&tx, sizeof(tx));
}

static inline msg_t
_decode_measure(HDC1000_drv *drv,
	uint32_t val, float *temperature, float *humidity) {
    (void)drv;
    
    /* Temperature */
    if (temperature) {
	float temp = (val >> 16);
	temp /= 65536;
	temp *= 165;
	temp -= 40;
	*temperature = temp;
    }

    /* Humidiy */
    if (humidity) {
	float hum  = (val & 0xFFFF);
	hum /= 65535;
	hum *= 100;
	*humidity = hum;
    }

    /* ok */
    return MSG_OK;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void
HDC1000_init(HDC1000_drv *drv, HDC1000_config *config) {
    drv->config   = config;
    drv->cfg      =  HDC1000_CONFIG_RST | HDC1000_CONFIG_MODE_BOTH |
	             HDC1000_CONFIG_RES;
    drv->delay    = (HDC1000_DELAY_ACQUIRE +
		     HDC1000_DELAY_ACQUIRE_SAFETY) / 1000;
    drv->state    = SENSOR_INIT;
}

msg_t
HDC1000_check(HDC1000_drv *drv) {
    uint16_t manuf, device;

    msg_t msg;
    if (((msg = i2c_reg_recv16_be(HDC1000_REG_MANUF_ID,  &manuf )) < MSG_OK) ||
	((msg = i2c_reg_recv16_be(HDC1000_REG_DEVICE_ID, &device)) < MSG_OK))
	return msg;
    
    if ((manuf != HDC1000_MANUF_ID) || (device != HDC1000_DEVICE_ID))
	return SENSOR_NOTFOUND;

    return MSG_OK;
}


msg_t
HDC1000_start(HDC1000_drv *drv) {
    osalDbgAssert((drv->state == SENSOR_INIT   ) ||
		  (drv->state == SENSOR_ERROR  ) ||
		  (drv->state == SENSOR_STOPPED),
		  "invalid state");
    msg_t msg;    
    if ((msg = _apply_config(drv)) < MSG_OK) {
	drv->state = SENSOR_ERROR;
	return msg;
    }
    drv->state = SENSOR_STARTED;
    return MSG_OK;
}

msg_t
HDC1000_stop(HDC1000_drv *drv) {
    drv->state = SENSOR_STOPPED;
    return MSG_OK;
}

msg_t
HDC1000_setHeater(HDC1000_drv *drv, bool on) {
    if (on) { drv->cfg |=  HDC1000_CONFIG_HEATER; }
    else    { drv->cfg &= ~HDC1000_CONFIG_HEATER; }

    msg_t msg;    
    if ((msg = _apply_config(drv)) < MSG_OK) {
	drv->state = SENSOR_ERROR;
	return msg;
    }
    return MSG_OK;
}

msg_t
HDC1000_startMeasure(HDC1000_drv *drv) {
    msg_t msg;
    osalDbgAssert(drv->state == SENSOR_STARTED, "invalid state");
    if ((msg = i2c_reg(HDC1000_REG_TEMP_HUMID)) < MSG_OK)
	return msg;
    drv->state = SENSOR_MEASURING;
    return MSG_OK;
}


msg_t
HDC1000_readSerial(HDC1000_drv *drv, uint8_t *serial) {
    msg_t msg;
    osalDbgAssert(drv->state == SENSOR_STARTED, "invalid state");

    if (((msg = i2c_reg_recv16(HDC1000_REG_SERIAL_1,
			       (uint16_t*)&serial[0])) < MSG_OK) ||
	((msg = i2c_reg_recv16(HDC1000_REG_SERIAL_2,
			       (uint16_t*)&serial[2])) < MSG_OK) ||
	((msg = i2c_reg_recv8 (HDC1000_REG_SERIAL_3,
			       (uint8_t*) &serial[4])) < MSG_OK))
	return msg;
    return MSG_OK;
}


msg_t
HDC1000_readMeasure(HDC1000_drv *drv,
	float *temperature, float *humidity) {
    msg_t    msg;
    uint32_t val;

    osalDbgAssert((drv->state == SENSOR_MEASURING) ||
		  (drv->state == SENSOR_READY    ),
		  "invalid state");

    if ((msg = i2c_recv32_be(&val)) < MSG_OK) {
	drv->state = SENSOR_ERROR;
	return msg;
    }

    drv->state = SENSOR_STARTED;

    return _decode_measure(drv, val, temperature, humidity);
}

msg_t
HDC1000_readTemperatureHumidity(HDC1000_drv *drv,
	float *temperature, float *humidity) {
    msg_t    msg;
    uint32_t val;

    osalDbgAssert(drv->state == SENSOR_STARTED, "invalid state");

    /* Request value */
    if ((msg = i2c_reg(HDC1000_REG_TEMP_HUMID)) < MSG_OK)
	return msg;

    /* Wait */
    osalThreadSleepMilliseconds(drv->delay);
	
    /* Get value */
    if ((msg = i2c_recv32_be(&val)) < MSG_OK) {
	drv->state = SENSOR_ERROR;
	return msg;
    }

    return _decode_measure(drv, val, temperature, humidity);
}


/** @} */

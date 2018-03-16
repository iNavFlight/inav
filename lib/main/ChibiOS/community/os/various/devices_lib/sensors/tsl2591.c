/*
    TSL2591 for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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
 *
 * DOC: http://ams.com/eng/content/download/389383/1251117/221235
 */

#define I2C_HELPERS_AUTOMATIC_DRV TRUE

#include "hal.h"
#include "i2c_helpers.h"
#include "tsl2591.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define TSL2591_LUX_DF            (408.0F)
#define TSL2591_LUX_COEFB         (1.64F)  // CH0 coefficient 
#define TSL2591_LUX_COEFC         (0.59F)  // CH1 coefficient A
#define TSL2591_LUX_COEFD         (0.86F)  // CH2 coefficient B

/* I2C registers */
#define TSL2591_REG_ENABLE        0x00
#define TSL2591_REG_CONFIG        0x01 /**< @brief gain and integration */
#define TSL2591_REG_AILTL         0x04
#define TSL2591_REG_AILTH         0x05
#define TSL2591_REG_AIHTL         0x06
#define TSL2591_REG_AIHTH         0x07
#define TSL2591_REG_NPAILTL       0x08
#define TSL2591_REG_NPAILTH       0x09
#define TSL2591_REG_NPAIHTL       0x0A
#define TSL2591_REG_NPAIHTH       0x0B
#define TSL2591_REG_PERSIST       0x0C
#define TSL2591_REG_PID           0x11 /**< @brief Package ID */
#define TSL2591_REG_ID            0x12 /**< @brief Device ID */
#define TSL2591_REG_STATUS        0x13 /**< @brief Device status */
#define TSL2591_REG_C0DATAL       0x14 /**< @brief CH0 ADC low  data byte */
#define TSL2591_REG_C0DATAH       0x15 /**< @brief CH0 ADC high data byte */
#define TSL2591_REG_C1DATAL       0x16 /**< @brief CH1 ADC low  data byte */
#define TSL2591_REG_C1DATAH       0x17 /**< @brief CH1 ADC high data byte */

#define TSL2591_REG_COMMAND       0x80 /**< @brief Select command register */
#define TSL2591_REG_NORMAL        0x20 /**< @brief Normal opearation       */
#define TSL2591_REG_SPECIAL       0x60 /**< @brief Special function        */

#define TSL2591_ID_TSL2591        0x50

#define TSL2591_VISIBLE           (2)       // channel 0 - channel 1
#define TSL2591_INFRARED          (1)       // channel 1
#define TSL2591_FULLSPECTRUM      (0)       // channel 0

#define TSL2591_ENABLE_POWERON    (0x01)
#define TSL2591_ENABLE_POWEROFF   (0x00)
#define TSL2591_ENABLE_AEN        (0x02)
#define TSL2591_ENABLE_AIEN       (0x10)

#define TSL2591_CONTROL_RESET     (0x80)


/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static inline uint32_t
calculateIlluminance(TSL2591_integration_time_t integration_time,
		     TSL2591_gain_t gain,
		     uint16_t broadband, uint16_t ir) {
  uint16_t atime, again;

  /* Check for overflow conditions first */
  if ((broadband == 0xFFFF) | (ir == 0xFFFF)) {
      return 0xFFFFFFFF; /* Signal overflow */
  }

  switch (integration_time) {
  case TSL2591_INTEGRATIONTIME_100MS : atime =   100; break;
  case TSL2591_INTEGRATIONTIME_200MS : atime =   200; break;
  case TSL2591_INTEGRATIONTIME_300MS : atime =   300; break;
  case TSL2591_INTEGRATIONTIME_400MS : atime =   400; break;
  case TSL2591_INTEGRATIONTIME_500MS : atime =   500; break;
  case TSL2591_INTEGRATIONTIME_600MS : atime =   600; break;
  }
  
  switch (gain) {
  case TSL2591_GAIN_1X              : again =     1; break;
  case TSL2591_GAIN_25X             : again =    25; break;
  case TSL2591_GAIN_415X            : again =   415; break;
  case TSL2591_GAIN_10000X          : again = 10000; break;
  }

  // cpl = (ATIME * AGAIN) / DF
  float cpl  = ((float)(atime * again)) / ((float)TSL2591_LUX_DF);
  float lux1 = ( ((float)broadband) - (TSL2591_LUX_COEFB * (float)ir) ) / cpl;
  float lux2 = ( (TSL2591_LUX_COEFC * (float)broadband) -
		 (TSL2591_LUX_COEFD * (float)ir       ) ) / cpl;
  
  return (uint32_t) (lux1 > lux2 ? lux1 : lux2);
}

static inline msg_t
_readChannel(TSL2591_drv *drv, uint16_t *broadband, uint16_t *ir) {
    msg_t msg;
    if (((msg = i2c_reg_recv16_le(
	  TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_C0DATAL,
	  broadband)) < MSG_OK) ||
	((msg = i2c_reg_recv16_le(
	  TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_C1DATAL,
	  ir       )) < MSG_OK))
	return msg;

    return MSG_OK;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void
TSL2591_init(TSL2591_drv *drv, TSL2591_config *config) {
    drv->config           = config;
    drv->gain             = TSL2591_GAIN_1X;
    drv->integration_time = TSL2591_INTEGRATIONTIME_100MS;
    drv->state            = SENSOR_INIT;
}
  
msg_t
TSL2591_check(TSL2591_drv *drv) {
    uint8_t id;

    msg_t msg;
    if ((msg = i2c_reg_recv8(TSL2591_REG_COMMAND | TSL2591_REG_NORMAL |
			     TSL2591_REG_ID, &id)) < MSG_OK)
	return msg;

    if (id != TSL2591_ID_TSL2591)
	return SENSOR_NOTFOUND;

    return MSG_OK;
}

msg_t
TSL2591_start(TSL2591_drv *drv) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx_config = {
	TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_CONFIG,
	(uint8_t)(drv->integration_time | drv->gain) };

    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx_start = {
	TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_ENABLE,
	TSL2591_ENABLE_POWERON };

    msg_t msg;

    if (((msg = i2c_send((uint8_t*)&tx_config, sizeof(tx_config))) < MSG_OK) ||
	((msg = i2c_send((uint8_t*)&tx_start,  sizeof(tx_start ))) < MSG_OK)) {
	drv->state = SENSOR_ERROR;
	return msg;
    }
    
    drv->state = SENSOR_STARTED;
    return MSG_OK;
}

msg_t
TSL2591_stop(TSL2591_drv *drv) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx_stop = {
	TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_ENABLE,
	TSL2591_ENABLE_POWEROFF };
    
    return i2c_send((uint8_t*)&tx_stop, sizeof(tx_stop));
}

msg_t
TSL2591_setIntegrationTime(TSL2591_drv *drv,
	TSL2591_integration_time_t time) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx = { TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_CONFIG,
	     (uint8_t)(time | drv->gain) };
    
    msg_t msg;
    if ((msg = i2c_send((uint8_t*)&tx, sizeof(tx))) < MSG_OK)
	return msg;
    
    drv->integration_time = time;
    
    return MSG_OK;
}

msg_t
TSL2591_setGain(TSL2591_drv *drv,
	TSL2591_gain_t gain) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx = { TSL2591_REG_COMMAND | TSL2591_REG_NORMAL | TSL2591_REG_CONFIG,
	     (uint8_t)(drv->integration_time | gain) };

    msg_t msg;
    if ((msg = i2c_send((uint8_t*)&tx, sizeof(tx))) < MSG_OK)
	return msg;

    drv->gain = gain;

    return MSG_OK;
}

unsigned int
TSL2591_getAcquisitionTime(TSL2591_drv *drv) {
    switch (drv->integration_time) {
    case TSL2591_INTEGRATIONTIME_100MS : return 100;
    case TSL2591_INTEGRATIONTIME_200MS : return 200;
    case TSL2591_INTEGRATIONTIME_300MS : return 300; 
    case TSL2591_INTEGRATIONTIME_400MS : return 400; 
    case TSL2591_INTEGRATIONTIME_500MS : return 500; 
    case TSL2591_INTEGRATIONTIME_600MS : return 600; 
    }
    return -1;
}


msg_t
TSL2591_readIlluminance(TSL2591_drv *drv,
	unsigned int *illuminance) {
    uint16_t broadband;
    uint16_t ir;

    /* Read channels */
    msg_t msg;
    if ((msg = _readChannel(drv, &broadband, &ir)) < MSG_OK)
	return msg;

    /* Calculate illuminance */
    *illuminance =
	calculateIlluminance(drv->integration_time, drv->gain,
			     broadband, ir);
    /* Ok */
    return SENSOR_OK;
}


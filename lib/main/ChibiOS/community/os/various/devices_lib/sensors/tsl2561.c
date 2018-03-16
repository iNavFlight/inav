/*
    TSL2561 for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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
 * Illuminance calculation code provided by www.taosinc.com
 * DOC: http://ams.com/eng/content/download/250096/975518/143687
 */
#define I2C_HELPERS_AUTOMATIC_DRV TRUE

#include "hal.h"
#include "i2c_helpers.h"
#include "tsl2561.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

// Integration time in µs
#define TSL2561_DELAY_INTTIME_SHORT     13700  //   13.7 ms
#define TSL2561_DELAY_INTTIME_MEDIUM   120000  //  120.0 ms
#define TSL2561_DELAY_INTTIME_LONG     450000  //  450.0 ms


#define TSL2561_COMMAND_BIT       (0x80)
#define TSL2561_CLEAR_BIT         (0x40)
#define TSL2561_WORD_BIT          (0x20)
#define TSL2561_BLOCK_BIT         (0x10)

#define TSL2561_CONTROL_POWERON   (0x03)
#define TSL2561_CONTROL_POWEROFF  (0x00)

#define TSL2561_LUX_LUXSCALE      (14)
#define TSL2561_LUX_RATIOSCALE    (9)
#define TSL2561_LUX_CHSCALE       (10)      // Scale channel values by 2^10
#define TSL2561_LUX_CHSCALE_TINT0 (0x7517)  // 322/11 * 2^TSL2561_LUX_CHSCALE
#define TSL2561_LUX_CHSCALE_TINT1 (0x0FE7)  // 322/81 * 2^TSL2561_LUX_CHSCALE


// I2C Register
#define TSL2561_REG_CONTROL           0x00
#define TSL2561_REG_TIMING            0x01
#define TSL2561_REG_THRESHHOLDLLOW    0x02
#define TSL2561_REG_THRESHHOLDLHIGH   0x03
#define TSL2561_REG_THRESHHOLDHLOW    0x04
#define TSL2561_REG_THRESHHOLDHHIGH   0x05
#define TSL2561_REG_INTERRUPT         0x06
#define TSL2561_REG_CRC               0x08
#define TSL2561_REG_ID                0x0A
#define TSL2561_REG_DATA0LOW          0x0C
#define TSL2561_REG_DATA0HIGH         0x0D
#define TSL2561_REG_DATA1LOW          0x0E
#define TSL2561_REG_DATA1HIGH         0x0F


// Auto-gain thresholds
#define TSL2561_AGC_THI_SHORT      (4850)    // Max value at Ti 13ms = 5047
#define TSL2561_AGC_TLO_SHORT      (100)
#define TSL2561_AGC_THI_MEDIUM     (36000)   // Max value at Ti 101ms = 37177
#define TSL2561_AGC_TLO_MEDIUM     (200)
#define TSL2561_AGC_THI_LONG       (63000)   // Max value at Ti 402ms = 65535
#define TSL2561_AGC_TLO_LONG       (500)

// Clipping thresholds
#define TSL2561_CLIPPING_SHORT     (4900)
#define TSL2561_CLIPPING_MEDIUM    (37000)
#define TSL2561_CLIPPING_LONG      (65000)

// T, FN and CL package values
#define TSL2561_LUX_K1T           (0x0040)  // 0.125   * 2^RATIO_SCALE
#define TSL2561_LUX_B1T           (0x01f2)  // 0.0304  * 2^LUX_SCALE
#define TSL2561_LUX_M1T           (0x01be)  // 0.0272  * 2^LUX_SCALE
#define TSL2561_LUX_K2T           (0x0080)  // 0.250   * 2^RATIO_SCALE
#define TSL2561_LUX_B2T           (0x0214)  // 0.0325  * 2^LUX_SCALE
#define TSL2561_LUX_M2T           (0x02d1)  // 0.0440  * 2^LUX_SCALE
#define TSL2561_LUX_K3T           (0x00c0)  // 0.375   * 2^RATIO_SCALE
#define TSL2561_LUX_B3T           (0x023f)  // 0.0351  * 2^LUX_SCALE
#define TSL2561_LUX_M3T           (0x037b)  // 0.0544  * 2^LUX_SCALE
#define TSL2561_LUX_K4T           (0x0100)  // 0.50    * 2^RATIO_SCALE
#define TSL2561_LUX_B4T           (0x0270)  // 0.0381  * 2^LUX_SCALE
#define TSL2561_LUX_M4T           (0x03fe)  // 0.0624  * 2^LUX_SCALE
#define TSL2561_LUX_K5T           (0x0138)  // 0.61    * 2^RATIO_SCALE
#define TSL2561_LUX_B5T           (0x016f)  // 0.0224  * 2^LUX_SCALE
#define TSL2561_LUX_M5T           (0x01fc)  // 0.0310  * 2^LUX_SCALE
#define TSL2561_LUX_K6T           (0x019a)  // 0.80    * 2^RATIO_SCALE
#define TSL2561_LUX_B6T           (0x00d2)  // 0.0128  * 2^LUX_SCALE
#define TSL2561_LUX_M6T           (0x00fb)  // 0.0153  * 2^LUX_SCALE
#define TSL2561_LUX_K7T           (0x029a)  // 1.3     * 2^RATIO_SCALE
#define TSL2561_LUX_B7T           (0x0018)  // 0.00146 * 2^LUX_SCALE
#define TSL2561_LUX_M7T           (0x0012)  // 0.00112 * 2^LUX_SCALE
#define TSL2561_LUX_K8T           (0x029a)  // 1.3     * 2^RATIO_SCALE
#define TSL2561_LUX_B8T           (0x0000)  // 0.000   * 2^LUX_SCALE
#define TSL2561_LUX_M8T           (0x0000)  // 0.000   * 2^LUX_SCALE

// CS package values
#define TSL2561_LUX_K1C           (0x0043)  // 0.130   * 2^RATIO_SCALE
#define TSL2561_LUX_B1C           (0x0204)  // 0.0315  * 2^LUX_SCALE
#define TSL2561_LUX_M1C           (0x01ad)  // 0.0262  * 2^LUX_SCALE
#define TSL2561_LUX_K2C           (0x0085)  // 0.260   * 2^RATIO_SCALE
#define TSL2561_LUX_B2C           (0x0228)  // 0.0337  * 2^LUX_SCALE
#define TSL2561_LUX_M2C           (0x02c1)  // 0.0430  * 2^LUX_SCALE
#define TSL2561_LUX_K3C           (0x00c8)  // 0.390   * 2^RATIO_SCALE
#define TSL2561_LUX_B3C           (0x0253)  // 0.0363  * 2^LUX_SCALE
#define TSL2561_LUX_M3C           (0x0363)  // 0.0529  * 2^LUX_SCALE
#define TSL2561_LUX_K4C           (0x010a)  // 0.520   * 2^RATIO_SCALE
#define TSL2561_LUX_B4C           (0x0282)  // 0.0392  * 2^LUX_SCALE
#define TSL2561_LUX_M4C           (0x03df)  // 0.0605  * 2^LUX_SCALE
#define TSL2561_LUX_K5C           (0x014d)  // 0.65    * 2^RATIO_SCALE
#define TSL2561_LUX_B5C           (0x0177)  // 0.0229  * 2^LUX_SCALE
#define TSL2561_LUX_M5C           (0x01dd)  // 0.0291  * 2^LUX_SCALE
#define TSL2561_LUX_K6C           (0x019a)  // 0.80    * 2^RATIO_SCALE
#define TSL2561_LUX_B6C           (0x0101)  // 0.0157  * 2^LUX_SCALE
#define TSL2561_LUX_M6C           (0x0127)  // 0.0180  * 2^LUX_SCALE
#define TSL2561_LUX_K7C           (0x029a)  // 1.3     * 2^RATIO_SCALE
#define TSL2561_LUX_B7C           (0x0037)  // 0.00338 * 2^LUX_SCALE
#define TSL2561_LUX_M7C           (0x002b)  // 0.00260 * 2^LUX_SCALE
#define TSL2561_LUX_K8C           (0x029a)  // 1.3     * 2^RATIO_SCALE
#define TSL2561_LUX_B8C           (0x0000)  // 0.000   * 2^LUX_SCALE
#define TSL2561_LUX_M8C           (0x0000)  // 0.000   * 2^LUX_SCALE


/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#define CEILING(x,y) (((x) + (y) - 1) / (y))

static inline unsigned int
calculateIlluminance(TSL2561_integration_time_t integration_time,
		     TSL2561_gain_t gain,
		     uint16_t broadband, uint16_t ir,
		     unsigned int partno) {
    unsigned long channel_1;
    unsigned long channel_0;  

    /* Get value for channel scaling, and clipping */
    uint16_t      clip_threshold = 0;
    unsigned long channel_scale  = 0;
    switch (integration_time) {
    case TSL2561_INTEGRATIONTIME_SHORT:
	clip_threshold = TSL2561_CLIPPING_SHORT;
	channel_scale  = TSL2561_LUX_CHSCALE_TINT0;
 	break;
    case TSL2561_INTEGRATIONTIME_MEDIUM:
	clip_threshold = TSL2561_CLIPPING_MEDIUM;
	channel_scale  = TSL2561_LUX_CHSCALE_TINT1;
	break;
    case TSL2561_INTEGRATIONTIME_LONG:
	clip_threshold = TSL2561_CLIPPING_LONG;
	channel_scale  = (1 << TSL2561_LUX_CHSCALE);
	break;
    default:
	// assert failed
	break;
    }

    /* Check for saturated sensor (ie: clipping) */
    if ((broadband > clip_threshold) || (ir > clip_threshold)) {
	return TSL2561_OVERLOADED;
    }
    
    /* Scale for gain (1x or 16x) */
    if (gain == TSL2561_GAIN_1X)
	channel_scale <<= 4;
    
    /* Scale the channel values */
    channel_0 = (broadband * channel_scale) >> TSL2561_LUX_CHSCALE;
    channel_1 = (ir        * channel_scale) >> TSL2561_LUX_CHSCALE;

    /* Find the ratio of the channel values (Channel_1/Channel_0) */
    unsigned long _ratio = 0;
    if (channel_0 != 0)
	_ratio = (channel_1 << (TSL2561_LUX_RATIOSCALE+1)) / channel_0;
    unsigned long ratio = (_ratio + 1) >> 1; /* round the ratio value */

    /* Find linear approximation */
    unsigned int b = 0;
    unsigned int m = 0;

    switch (partno) {
#if TSL2561_WITH_CS
    case 0x1: // 0001 = TSL2561 CS
	if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1C))
	    { b=TSL2561_LUX_B1C; m=TSL2561_LUX_M1C; }
	else if (ratio <= TSL2561_LUX_K2C)
	    { b=TSL2561_LUX_B2C; m=TSL2561_LUX_M2C; }
	else if (ratio <= TSL2561_LUX_K3C)
	    { b=TSL2561_LUX_B3C; m=TSL2561_LUX_M3C; }
	else if (ratio <= TSL2561_LUX_K4C)
	    { b=TSL2561_LUX_B4C; m=TSL2561_LUX_M4C; }
	else if (ratio <= TSL2561_LUX_K5C)
	    { b=TSL2561_LUX_B5C; m=TSL2561_LUX_M5C; }
	else if (ratio <= TSL2561_LUX_K6C)
	    { b=TSL2561_LUX_B6C; m=TSL2561_LUX_M6C; }
	else if (ratio <= TSL2561_LUX_K7C)
	    { b=TSL2561_LUX_B7C; m=TSL2561_LUX_M7C; }
	else if (ratio > TSL2561_LUX_K8C)
	    { b=TSL2561_LUX_B8C; m=TSL2561_LUX_M8C; }
	break;
#endif
#if TSL2561_WITH_T_FN_CL
    case 0x5: // 0101 = TSL2561 T/FN/CL
	if ((ratio >= 0) && (ratio <= TSL2561_LUX_K1T))
	    { b=TSL2561_LUX_B1T; m=TSL2561_LUX_M1T; }
	else if (ratio <= TSL2561_LUX_K2T)
	    { b=TSL2561_LUX_B2T; m=TSL2561_LUX_M2T; }
	else if (ratio <= TSL2561_LUX_K3T)
	    { b=TSL2561_LUX_B3T; m=TSL2561_LUX_M3T; }
	else if (ratio <= TSL2561_LUX_K4T)
	    { b=TSL2561_LUX_B4T; m=TSL2561_LUX_M4T; }
	else if (ratio <= TSL2561_LUX_K5T)
	    { b=TSL2561_LUX_B5T; m=TSL2561_LUX_M5T; }
	else if (ratio <= TSL2561_LUX_K6T)
	    { b=TSL2561_LUX_B6T; m=TSL2561_LUX_M6T; }
	else if (ratio <= TSL2561_LUX_K7T)
	    { b=TSL2561_LUX_B7T; m=TSL2561_LUX_M7T; }
	else if (ratio > TSL2561_LUX_K8T)
	    { b=TSL2561_LUX_B8T; m=TSL2561_LUX_M8T; }
	break;
#endif
    default:
	// assert failed
	break;
    }
    
    /* Compute illuminance */
    long ill = ((channel_0 * b) - (channel_1 * m));
    if (ill < 0) ill = 0;                /* Do not allow negative lux value */
    ill  += (1 << (TSL2561_LUX_LUXSCALE-1)); /* Round lsb (2^(LUX_SCALE-1)) */
    ill >>= TSL2561_LUX_LUXSCALE;                  /* Strip fractional part */
    
    /* Signal I2C had no errors */
    return ill;
}

static inline msg_t
_readChannel(TSL2561_drv *drv, uint16_t *broadband, uint16_t *ir) {
    msg_t msg;
    if (((msg = i2c_reg_recv16_le(
	  TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REG_DATA0LOW,
	  broadband)) < MSG_OK) ||
	((msg = i2c_reg_recv16_le(
	  TSL2561_COMMAND_BIT | TSL2561_WORD_BIT | TSL2561_REG_DATA1LOW,
	  ir       )) < MSG_OK))
	return msg;
    return MSG_OK;
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

void
TSL2561_init(TSL2561_drv *drv, TSL2561_config *config) {
    drv->config           = config;
    drv->gain             = TSL2561_GAIN_1X;
    drv->integration_time = TSL2561_INTEGRATIONTIME_LONG;
    drv->state            = SENSOR_INIT;

    i2c_reg_recv8(TSL2561_COMMAND_BIT | TSL2561_REG_ID,
		 (uint8_t*)&drv->id);
}

msg_t
TSL2561_check(TSL2561_drv *drv) {
    uint8_t rx;

    msg_t msg;
    if ((msg = i2c_reg_recv8(TSL2561_REG_ID, &rx)) < MSG_OK)
	return msg;
    if (!(rx & 0x0A))
	return SENSOR_NOTFOUND;
    return MSG_OK;
}
  
msg_t
TSL2561_stop(TSL2561_drv *drv) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx = { TSL2561_COMMAND_BIT | TSL2561_REG_CONTROL,
	     TSL2561_CONTROL_POWEROFF };

    return i2c_send((uint8_t*)&tx, sizeof(tx));
}

msg_t
TSL2561_start(TSL2561_drv *drv) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx = { TSL2561_COMMAND_BIT | TSL2561_REG_CONTROL,
	     TSL2561_CONTROL_POWERON };
    
    return i2c_send((uint8_t*)&tx, sizeof(tx));
}

msg_t
TSL2561_setIntegrationTime(TSL2561_drv *drv,
	TSL2561_integration_time_t time) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx = { TSL2561_COMMAND_BIT | TSL2561_REG_TIMING,
	     (uint8_t)(time | drv->gain) };
        
    msg_t msg;
    if ((msg = i2c_send((uint8_t*)&tx, sizeof(tx))) < MSG_OK)
	return msg;
    
    drv->integration_time = time;
    
    return MSG_OK;
}

msg_t
TSL2561_setGain(TSL2561_drv *drv,
	TSL2561_gain_t gain) {
    struct __attribute__((packed)) {
	uint8_t reg;
	uint8_t conf;
    } tx = { TSL2561_COMMAND_BIT | TSL2561_REG_TIMING,
	     (uint8_t)(drv->integration_time | gain) };
    
    msg_t msg;
    if ((msg = i2c_send((uint8_t*)&tx, sizeof(tx))) < MSG_OK)
	return msg;

    drv->gain = gain;

    return MSG_OK;
}

unsigned int
TSL2561_getAcquisitionTime(TSL2561_drv *drv) {
    switch (drv->integration_time) {
    case TSL2561_INTEGRATIONTIME_SHORT:
	return CEILING(TSL2561_DELAY_INTTIME_SHORT , 1000);
    case TSL2561_INTEGRATIONTIME_MEDIUM:
	return CEILING(TSL2561_DELAY_INTTIME_MEDIUM, 1000);
    case TSL2561_INTEGRATIONTIME_LONG:
	return CEILING(TSL2561_DELAY_INTTIME_LONG  , 1000);
    }
    return -1;
}


msg_t
TSL2561_readIlluminance(TSL2561_drv *drv,
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
			     broadband, ir, drv->id.partno);
    /* Ok */
    return SENSOR_OK;
}


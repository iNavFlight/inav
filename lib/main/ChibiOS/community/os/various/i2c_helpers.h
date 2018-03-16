/*
    Copyright (C) 2016 Stephane D'Alu

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

#ifndef I2C_HELPERS_H
#define I2C_HELPERS_H

#include "hal.h"
#include "bswap.h"


typedef struct {
    /**
     * @brief Pointer to the I2C driver.
     */
    I2CDriver *driver;
    /**
     * @brief I2C address.
     */
    i2caddr_t addr;
} I2CHelper;



#if !defined(I2C_HELPERS_AUTOMATIC_DRV) || (I2C_HELPERS_AUTOMATIC_DRV == FALSE)

#define i2c_send(i2c, txbuf, txbytes)					\
    _i2c_send(i2c, txbuf, txbytes)
#define i2c_transmit(i2c, txbuf, txbytes, rxbuf, rxbytes)		\
    _i2c_transmit(i2c, txbuf, txbytes, rxbuf, rxbytes)
#define i2c_receive(i2, rxbuf, rxbytes)					\
    _i2c_receive(i2c, rxbuf, rxbytes)

#define i2c_send_timeout(i2c, txbuf, txbytes)				\
    _i2c_send(i2c, txbuf, txbytes)
#define i2c_transmit_timeout(i2c, txbuf, txbytes, rxbuf, rxbytes)	\
    _i2c_transmit(i2c, txbuf, txbytes, rxbuf, rxbytes)
#define i2c_receive_timeout(i2, rxbuf, rxbytes)				\
    _i2c_receive(i2c, rxbuf, rxbytes)

#define i2c_reg(i2c, reg)						\
    _i2c_reg(i2c, reg) 

#define i2c_reg_recv8(i2c, reg, val)					\
    _i2c_reg_recv8(i2c, reg, val)		
#define i2c_reg_recv16(i2c, reg, val)					\
    _i2c_reg_recv16(i2c, reg, val)		
#define i2c_reg_recv16_le(i2c, reg, val)				\
    _i2c_reg_recv16_le(i2c, reg, val)
#define i2c_reg_recv16_be(i2c, reg, val)				\
    _i2c_reg_recv16_be(i2c, reg, val) 
#define i2c_reg_recv32(i2c, reg, val)					\
    _i2c_reg_recv32(i2c, reg, val)
#define i2c_reg_recv32_le(i2c, reg, val)				\
    _i2c_reg_recv32_le(i2c, reg, val)
#define i2c_reg_recv32_be(i2c, reg, val)				\
    _i2c_reg_recv32_be(i2c, reg, val)

#define i2c_recv8(i2c, val)						\
    _i2c_recv8(i2c, val)		
#define i2c_recv16(i2c, val)						\
    _i2c_recv16(i2c, val)		
#define i2c_recv16_le(i2c, val)						\
    _i2c_recv16_le(i2c, val)
#define i2c_recv16_be(i2c, val)						\
    _i2c_recv16_be(i2c, val) 
#define i2c_recv32(i2c, val)						\
    _i2c_recv32(i2c, val)
#define i2c_recv32_le(i2c, val)						\
    _i2c_recv32_le(i2c, val)
#define i2c_recv32_be(i2c, val)						\
    _i2c_recv32_be(i2c, val)

#else

#define i2c_send(txbuf, txbytes)					\
    _i2c_send(&drv->config->i2c, txbuf, txbytes)
#define i2c_transmit(txbuf, txbytes, rxbuf, rxbytes)			\
    _i2c_transmit(&drv->config->i2c, txbuf, txbytes, rxbuf, rxbytes)
#define i2c_receive(rxbuf, rxbytes)					\
    _i2c_receive(&drv->config->i2c, rxbuf, rxbytes)

#define i2c_send_timeout(txbuf, txbytes)				\
    _i2c_send(&drv->config->i2c, txbuf, txbytes)
#define i2c_transmit_timeout(txbuf, txbytes, rxbuf, rxbytes)	\
    _i2c_transmit(&drv->config->i2c, txbuf, txbytes, rxbuf, rxbytes)
#define i2c_receive_timeout(rxbuf, rxbytes)				\
    _i2c_receive(&drv->config->i2c, rxbuf, rxbytes)


#define i2c_reg(reg)							\
    _i2c_reg(&drv->config->i2c, reg) 

#define i2c_reg_recv8(reg, val)						\
    _i2c_reg_recv8(&drv->config->i2c, reg, val)		
#define i2c_reg_recv16(reg, val)					\
    _i2c_reg_recv16(&drv->config->i2c, reg, val)		
#define i2c_reg_recv16_le(reg, val)					\
    _i2c_reg_recv16_le(&drv->config->i2c, reg, val)
#define i2c_reg_recv16_be(reg, val)					\
    _i2c_reg_recv16_be(&drv->config->i2c, reg, val) 
#define i2c_reg_recv32(reg, val)					\
    _i2c_reg_recv32(&drv->config->i2c, reg, val)
#define i2c_reg_recv32_le(reg, val)					\
    _i2c_reg_recv32_le(&drv->config->i2c, reg, val)
#define i2c_reg_recv32_be(reg, val)					\
    _i2c_reg_recv32_be(&drv->config->i2c, reg, val)

#define i2c_recv8(val)							\
    _i2c_recv8(&drv->config->i2c, val)		
#define i2c_recv16(val)							\
    _i2c_recv16(&drv->config->i2c, val)		
#define i2c_recv16_le(val)						\
    _i2c_recv16_le(&drv->config->i2c, val)
#define i2c_recv16_be(val)						\
    _i2c_recv16_be(&drv->config->i2c, val) 
#define i2c_recv32(val)							\
    _i2c_recv32(&drv->config->i2c, val)
#define i2c_recv32_le(val)						\
    _i2c_recv32_le(&drv->config->i2c, val)
#define i2c_recv32_be(val)						\
    _i2c_recv32_be(&drv->config->i2c, val)

#endif





static inline msg_t
_i2c_send(I2CHelper *i2c, const uint8_t *txbuf, size_t txbytes) {
    return i2cMasterTransmitTimeout(i2c->driver, i2c->addr,
			    txbuf, txbytes, NULL, 0, TIME_INFINITE);
};

static inline msg_t
_i2c_transmit(I2CHelper *i2c, const uint8_t *txbuf, size_t txbytes,
	     uint8_t *rxbuf, size_t rxbytes) {
    return i2cMasterTransmitTimeout(i2c->driver, i2c->addr,
			    txbuf, txbytes, rxbuf, rxbytes, TIME_INFINITE);
}

static inline msg_t
_i2c_receive(I2CHelper *i2c, uint8_t *rxbuf, size_t rxbytes) {
    return i2cMasterReceiveTimeout(i2c->driver, i2c->addr,
			    rxbuf, rxbytes, TIME_INFINITE);
};



static inline msg_t
_i2c_send_timeout(I2CHelper *i2c, const uint8_t *txbuf, size_t txbytes,
		 systime_t timeout) {
    return i2cMasterTransmitTimeout(i2c->driver, i2c->addr,
				    txbuf, txbytes, NULL, 0, timeout);
};

static inline msg_t
_i2c_transmit_timeout(I2CHelper *i2c, const uint8_t *txbuf, size_t txbytes,
		     uint8_t *rxbuf, size_t rxbytes, systime_t timeout) {
    return i2cMasterTransmitTimeout(i2c->driver, i2c->addr,
				    txbuf, txbytes, rxbuf, rxbytes, timeout);
}

static inline msg_t
_i2c_receive_timeout(I2CHelper *i2c, uint8_t *rxbuf, size_t rxbytes, systime_t timeout) {
    return i2cMasterReceiveTimeout(i2c->driver, i2c->addr,
			    rxbuf, rxbytes, timeout);
};


/*======================================================================*/


static inline msg_t
_i2c_reg(I2CHelper *i2c, uint8_t reg) {
    return _i2c_transmit(i2c, &reg, sizeof(reg), NULL, 0);
};

/*======================================================================*/

static inline msg_t
_i2c_reg_recv8(I2CHelper *i2c, uint8_t reg, uint8_t *val) {
    return _i2c_transmit(i2c, &reg, sizeof(reg), (uint8_t*)val, sizeof(val));
};

static inline msg_t
_i2c_reg_recv16(I2CHelper *i2c, uint8_t reg, uint16_t *val) {
    return _i2c_transmit(i2c, &reg, sizeof(reg), (uint8_t*)val, sizeof(val));
};

static inline msg_t
_i2c_reg_recv16_le(I2CHelper *i2c, uint8_t reg, uint16_t *val) {
    int msg = _i2c_reg_recv16(i2c, reg, val);
    if (msg >= 0) *val = le16_to_cpu(*val);
    return msg;
};

static inline msg_t
_i2c_reg_recv16_be(I2CHelper *i2c, uint8_t reg, uint16_t *val) {
    int msg = _i2c_reg_recv16(i2c, reg, val);
    if (msg >= 0) *val = be16_to_cpu(*val);
    return msg;
};    

static inline msg_t
_i2c_reg_recv32(I2CHelper *i2c, uint8_t reg, uint32_t *val) {
    return _i2c_transmit(i2c, &reg, sizeof(reg), (uint8_t*)val, sizeof(val));
};

static inline msg_t
_i2c_reg_recv32_le(I2CHelper *i2c, uint8_t reg, uint32_t *val) {
    int msg = _i2c_reg_recv32(i2c, reg, val);
    if (msg >= 0) *val = le32_to_cpu(*val);
    return msg;
};

static inline msg_t
_i2c_reg_recv32_be(I2CHelper *i2c, uint8_t reg, uint32_t *val) {
    int msg = _i2c_reg_recv32(i2c, reg, val);
    if (msg >= 0) *val = be32_to_cpu(*val);
    return msg;
};


/*======================================================================*/

static inline msg_t
_i2c_recv8(I2CHelper *i2c, uint8_t *val) {
    return _i2c_receive(i2c, (uint8_t*)val, sizeof(val));
};

static inline msg_t
_i2c_recv16(I2CHelper *i2c, uint16_t *val) {
    return _i2c_receive(i2c, (uint8_t*)val, sizeof(val));
};

static inline msg_t
_i2c_recv16_le(I2CHelper *i2c, uint16_t *val) {
    int msg = _i2c_recv16(i2c, val);
    if (msg >= 0) *val = le16_to_cpu(*val);
    return msg;
};

static inline msg_t
_i2c_recv16_be(I2CHelper *i2c, uint16_t *val) {
    int msg = _i2c_recv16(i2c, val);
    if (msg >= 0) *val = be16_to_cpu(*val);
    return msg;
};

static inline msg_t
_i2c_recv32(I2CHelper *i2c, uint32_t *val) {
    return _i2c_receive(i2c, (uint8_t*)val, sizeof(val));
};

static inline msg_t
_i2c_recv32_le(I2CHelper *i2c, uint32_t *val) {
    int msg = _i2c_recv32(i2c, val);
    if (msg >= 0) *val = le32_to_cpu(*val);
    return msg;
};

static inline msg_t
_i2c_recv32_be(I2CHelper *i2c, uint32_t *val) {
    int msg = _i2c_recv32(i2c, val);
    if (msg >= 0) *val = be32_to_cpu(*val);
    return msg;
};

#endif

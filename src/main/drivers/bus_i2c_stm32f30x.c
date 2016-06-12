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
#include <stdlib.h>

#include <platform.h>

#include "build_config.h"

#include "gpio.h"
#include "system.h"

#include "bus_i2c.h"

#ifndef SOFT_I2C

#define I2C_SHORT_TIMEOUT             ((uint32_t)0x1000)
#define I2C_LONG_TIMEOUT             ((uint32_t)(10 * I2C_SHORT_TIMEOUT))

#define I2C1_SCL_GPIO        GPIOB
#define I2C1_SCL_GPIO_AF     GPIO_AF_4
#define I2C1_SCL_PIN         GPIO_Pin_6
#define I2C1_SCL_PIN_SOURCE  GPIO_PinSource6
#define I2C1_SCL_CLK_SOURCE  RCC_AHBPeriph_GPIOB
#define I2C1_SDA_GPIO        GPIOB
#define I2C1_SDA_GPIO_AF     GPIO_AF_4
#define I2C1_SDA_PIN         GPIO_Pin_7
#define I2C1_SDA_PIN_SOURCE  GPIO_PinSource7
#define I2C1_SDA_CLK_SOURCE  RCC_AHBPeriph_GPIOB

#if !defined(I2C2_SCL_GPIO)
#define I2C2_SCL_GPIO        GPIOF
#define I2C2_SCL_GPIO_AF     GPIO_AF_4
#define I2C2_SCL_PIN         GPIO_Pin_6
#define I2C2_SCL_PIN_SOURCE  GPIO_PinSource6
#define I2C2_SCL_CLK_SOURCE  RCC_AHBPeriph_GPIOF
#define I2C2_SDA_GPIO        GPIOA
#define I2C2_SDA_GPIO_AF     GPIO_AF_4
#define I2C2_SDA_PIN         GPIO_Pin_10
#define I2C2_SDA_PIN_SOURCE  GPIO_PinSource10
#define I2C2_SDA_CLK_SOURCE  RCC_AHBPeriph_GPIOA

#endif


typedef struct i2cDevice_t {
    I2C_TypeDef *dev;
    GPIO_TypeDef *gpioscl;
    uint16_t scl;
    uint32_t scl_periph;
    uint16_t scl_source;
    uint8_t scl_af;
    GPIO_TypeDef *gpiosda;
    uint16_t sda;
    uint32_t sda_periph;
    uint16_t sda_source;
    uint8_t sda_af;
    uint32_t peripheral;
    uint32_t clock;
} i2cDevice_t;

static const i2cDevice_t i2cHardwareMap[] = {
    { I2C1, I2C1_SCL_GPIO, I2C1_SCL_PIN, I2C1_SCL_CLK_SOURCE, I2C1_SCL_PIN_SOURCE, I2C1_SCL_GPIO_AF, I2C1_SDA_GPIO,
    		I2C1_SDA_PIN, I2C1_SDA_CLK_SOURCE, I2C1_SDA_PIN_SOURCE, I2C1_SDA_GPIO_AF, RCC_APB1Periph_I2C1, RCC_I2C1CLK_SYSCLK },
    { I2C2, I2C2_SCL_GPIO, I2C2_SCL_PIN, I2C2_SCL_CLK_SOURCE, I2C2_SCL_PIN_SOURCE, I2C2_SCL_GPIO_AF, I2C2_SDA_GPIO,
    		I2C2_SDA_PIN, I2C2_SDA_CLK_SOURCE, I2C2_SDA_PIN_SOURCE, I2C2_SDA_GPIO_AF, RCC_APB1Periph_I2C2, RCC_I2C2CLK_SYSCLK },
};

static uint32_t i2cTimeout;
static volatile uint16_t i2cErrorCount = 0;

///////////////////////////////////////////////////////////////////////////////
// I2C TimeoutUserCallback
///////////////////////////////////////////////////////////////////////////////

static bool i2cOverClock;

void i2cSetOverclock(uint8_t OverClock) {
    i2cOverClock = (OverClock) ? true : false;
}

uint32_t i2cTimeoutUserCallback(I2C_TypeDef *I2Cx)
{
    i2cErrorCount++;
    return false;
}

void i2cInitPort(I2CDevice bus)
{
	I2C_TypeDef *I2Cx = i2cHardwareMap[bus].dev;
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

	RCC_AHBPeriphClockCmd(i2cHardwareMap[bus].scl_periph | i2cHardwareMap[bus].sda_periph, ENABLE);
	RCC_APB1PeriphClockCmd(i2cHardwareMap[bus].peripheral, ENABLE);
	RCC_I2CCLKConfig(i2cHardwareMap[bus].clock);

	//i2cUnstick(I2Cx);                                         // Clock out stuff to make sure slaves arent stuck

	GPIO_PinAFConfig(i2cHardwareMap[bus].gpioscl, i2cHardwareMap[bus].scl_source, i2cHardwareMap[bus].scl_af);
	GPIO_PinAFConfig(i2cHardwareMap[bus].gpiosda, i2cHardwareMap[bus].sda_source, i2cHardwareMap[bus].sda_af);

	GPIO_StructInit(&GPIO_InitStructure);
	I2C_StructInit(&I2C_InitStructure);

	// Init pins

        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	GPIO_InitStructure.GPIO_Pin = i2cHardwareMap[bus].scl;
	GPIO_Init(i2cHardwareMap[bus].gpioscl, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = i2cHardwareMap[bus].sda;
	GPIO_Init(i2cHardwareMap[bus].gpiosda, &GPIO_InitStructure);

	I2C_StructInit(&I2C_InitStructure);

        I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
        I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
        I2C_InitStructure.I2C_DigitalFilter = 0x00;
        I2C_InitStructure.I2C_OwnAddress1 = 0x00;
        I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
        I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
        if (i2cOverClock) {
            I2C_InitStructure.I2C_Timing = 0x00500E30; // 1000 Khz, 72Mhz Clock, Analog Filter Delay ON, Setup 40, Hold 4.
        } else {
            I2C_InitStructure.I2C_Timing = 0x00E0257A; // 400 Khz, 72Mhz Clock, Analog Filter Delay ON, Rise 100, Fall 10
        }
        //I2C_InitStructure.I2C_Timing              = 0x8000050B;

	I2C_Init(I2Cx, &I2C_InitStructure);

	I2C_Cmd(I2Cx, ENABLE);
}

void i2cInit(I2CDevice index)
{
    i2cInitPort(index);
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

bool i2cWrite(uint8_t addr_, uint8_t reg, uint8_t data, I2CDevice bus)
{
	I2C_TypeDef *I2Cx = i2cHardwareMap[bus].dev;
    addr_ <<= 1;

    /* Test on BUSY Flag */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_BUSY) != RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(I2Cx, addr_, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);

    /* Wait until TXIS flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Send Register address */
    I2C_SendData(I2Cx, (uint8_t) reg);

    /* Wait until TCR flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_TCR) == RESET)
    {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(I2Cx, addr_, 1, I2C_AutoEnd_Mode, I2C_No_StartStop);

    /* Wait until TXIS flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Write data to TXDR */
    I2C_SendData(I2Cx, data);

    /* Wait until STOPF flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF) == RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Clear STOPF flag */
    I2C_ClearFlag(I2Cx, I2C_ICR_STOPCF);

    return true;
}

bool i2cRead(uint8_t addr_, uint8_t reg, uint8_t len, uint8_t* buf, I2CDevice bus)
{
	I2C_TypeDef *I2Cx = i2cHardwareMap[bus].dev;
    addr_ <<= 1;

    /* Test on BUSY Flag */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_BUSY) != RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(I2Cx, addr_, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);

    /* Wait until TXIS flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Send Register address */
    I2C_SendData(I2Cx, (uint8_t) reg);

    /* Wait until TC flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_TC) == RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(I2Cx, addr_, len, I2C_AutoEnd_Mode, I2C_Generate_Start_Read);

    /* Wait until all data are received */
    while (len) {
        /* Wait until RXNE flag is set */
        i2cTimeout = I2C_LONG_TIMEOUT;
        while (I2C_GetFlagStatus(I2Cx, I2C_ISR_RXNE) == RESET) {
            if ((i2cTimeout--) == 0) {
                return i2cTimeoutUserCallback(I2Cx);
            }
        }

        /* Read data from RXDR */
        *buf = I2C_ReceiveData(I2Cx);
        /* Point to the next location where the byte read will be saved */
        buf++;

        /* Decrement the read bytes counter */
        len--;
    }

    /* Wait until STOPF flag is set */
    i2cTimeout = I2C_LONG_TIMEOUT;
    while (I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF) == RESET) {
        if ((i2cTimeout--) == 0) {
            return i2cTimeoutUserCallback(I2Cx);
        }
    }

    /* Clear STOPF flag */
    I2C_ClearFlag(I2Cx, I2C_ICR_STOPCF);

    /* If all operations OK */
    return true;
}

#endif

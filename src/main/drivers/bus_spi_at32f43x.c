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

#ifdef USE_SPI

#include "drivers/bus_spi.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/rcc.h"

#ifndef SPI1_SCK_PIN
#define SPI1_NSS_PIN    PA4
#define SPI1_SCK_PIN    PA5
#define SPI1_MISO_PIN   PA6
#define SPI1_MOSI_PIN   PA7
#endif

#ifndef SPI2_SCK_PIN
#define SPI2_NSS_PIN    PB12
#define SPI2_SCK_PIN    PB13
#define SPI2_MISO_PIN   PB14
#define SPI2_MOSI_PIN   PB15
#endif

#ifndef SPI3_SCK_PIN
#define SPI3_NSS_PIN    PA15
#define SPI3_SCK_PIN    PC10
#define SPI3_MISO_PIN   PC11
#define SPI3_MOSI_PIN   PC12
#endif

#ifndef SPI1_NSS_PIN
#define SPI1_NSS_PIN NONE
#endif
#ifndef SPI2_NSS_PIN
#define SPI2_NSS_PIN NONE
#endif
#ifndef SPI3_NSS_PIN
#define SPI3_NSS_PIN NONE
#endif

#if defined(AT32F43x)
    #if defined(USE_SPI_DEVICE_1)
    static const uint32_t spiDivisorMapFast[] = {
        SPI_MCLK_DIV_256,     // SPI_CLOCK_INITIALIZATON      328.125 KBits/s
        SPI_MCLK_DIV_128,     // SPI_CLOCK_SLOW               656.25 KBits/s
        SPI_MCLK_DIV_16,      // SPI_CLOCK_STANDARD           10.5 MBits/s
        SPI_MCLK_DIV_8,       // SPI_CLOCK_FAST               21.0 MBits/s
        SPI_MCLK_DIV_4        // SPI_CLOCK_ULTRAFAST          42.0 MBits/s
    };
    #endif

    #if defined(USE_SPI_DEVICE_2) || defined(USE_SPI_DEVICE_3)
    static const uint32_t spiDivisorMapSlow[] = {
        SPI_MCLK_DIV_256,     // SPI_CLOCK_INITIALIZATON      164.062 KBits/s
        SPI_MCLK_DIV_128,     // SPI_CLOCK_SLOW               656.25 KBits/s
        SPI_MCLK_DIV_16,      // SPI_CLOCK_STANDARD           10.5 MBits/s
        SPI_MCLK_DIV_8,       // SPI_CLOCK_FAST               21.0 MBits/s
        SPI_MCLK_DIV_8        // SPI_CLOCK_ULTRAFAST          21.0 MBits/s
    };
    #endif

    static spiDevice_t spiHardwareMap[] = {
    #ifdef USE_SPI_DEVICE_1
        { .dev = SPI1, .nss = IO_TAG(SPI1_NSS_PIN), .sck = IO_TAG(SPI1_SCK_PIN), .miso = IO_TAG(SPI1_MISO_PIN), .mosi = IO_TAG(SPI1_MOSI_PIN), .rcc = RCC_APB2(SPI1), .af = GPIO_MUX_5, .divisorMap = spiDivisorMapFast },
    #else
        { .dev = NULL },    // No SPI1
    #endif
    #ifdef USE_SPI_DEVICE_2
        { .dev = SPI2, .nss = IO_TAG(SPI2_NSS_PIN), .sck = IO_TAG(SPI2_SCK_PIN), .miso = IO_TAG(SPI2_MISO_PIN), .mosi = IO_TAG(SPI2_MOSI_PIN), .rcc = RCC_APB1(SPI2), .af = GPIO_MUX_6, .divisorMap = spiDivisorMapSlow },
    #else
        { .dev = NULL },    // No SPI2
    #endif
    #ifdef USE_SPI_DEVICE_3
        { .dev = SPI3, .nss = IO_TAG(SPI3_NSS_PIN), .sck = IO_TAG(SPI3_SCK_PIN), .miso = IO_TAG(SPI3_MISO_PIN), .mosi = IO_TAG(SPI3_MOSI_PIN), .rcc = RCC_APB1(SPI3), .af = GPIO_MUX_6, .divisorMap = spiDivisorMapSlow },
    #else
        { .dev = NULL },    // No SPI3
    #endif
        { .dev = NULL },    // No SPI4
    };
#else
#error "Invalid CPU"
#endif

SPIDevice spiDeviceByInstance(spi_type *instance)
{
    if (instance == SPI1)
        return SPIDEV_1;

    if (instance == SPI2)
        return SPIDEV_2;

    if (instance == SPI3)
        return SPIDEV_3;

    return SPIINVALID;
}

bool spiInitDevice(SPIDevice device, bool leadingEdge)
{
    spiDevice_t *spi = &(spiHardwareMap[device]);

    if (!spi->dev) {
        return false;
    }

    if (spi->initDone) {
        return true;
    }
    // Enable SPI clock
    RCC_ClockCmd(spi->rcc, ENABLE);
    RCC_ResetCmd(spi->rcc, ENABLE);

    IOInit(IOGetByTag(spi->sck),  OWNER_SPI, RESOURCE_SPI_SCK,  device + 1);
    IOInit(IOGetByTag(spi->miso), OWNER_SPI, RESOURCE_SPI_MISO, device + 1);
    IOInit(IOGetByTag(spi->mosi), OWNER_SPI, RESOURCE_SPI_MOSI, device + 1);

    IOConfigGPIOAF(IOGetByTag(spi->sck),  SPI_IO_AF_SCK_CFG, spi->af);
    IOConfigGPIOAF(IOGetByTag(spi->miso), SPI_IO_AF_MISO_CFG, spi->af);
    IOConfigGPIOAF(IOGetByTag(spi->mosi), SPI_IO_AF_CFG, spi->af);

    if (spi->nss) {
        IOConfigGPIOAF(IOGetByTag(spi->nss), SPI_IO_CS_CFG, spi->af);
    }

    spi_i2s_reset(spi->dev);
    spi_init_type spi_init_struct;
    spi_default_para_init (&spi_init_struct);
    spi_init_struct.master_slave_mode = SPI_MODE_MASTER;
    spi_init_struct.transmission_mode = SPI_TRANSMIT_FULL_DUPLEX;
    spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
    spi_init_struct.mclk_freq_division = SPI_MCLK_DIV_8;
    spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
    spi_init_struct.cs_mode_selection = SPI_CS_SOFTWARE_MODE;

    spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
    spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;

    if (leadingEdge) {
        // SPI_MODE0
       spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_LOW;
       spi_init_struct.clock_phase = SPI_CLOCK_PHASE_1EDGE;
    } else {
        // SPI_MODE3
       spi_init_struct.clock_polarity = SPI_CLOCK_POLARITY_HIGH;
       spi_init_struct.clock_phase = SPI_CLOCK_PHASE_2EDGE;

    }
    spi_init(spi->dev, &spi_init_struct);
    spi_crc_polynomial_set (spi->dev, 0x07);
    spi_crc_enable (spi->dev, TRUE); // enable crc
    spi_enable (spi->dev, TRUE);

    if (spi->nss) {
        // Drive NSS high to disable connected SPI device.
        IOHi(IOGetByTag(spi->nss));
    }
    spi->initDone = true;
    return true;
}

uint32_t spiTimeoutUserCallback(spi_type *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return -1;
    }
    spiHardwareMap[device].errorCount++;
    return spiHardwareMap[device].errorCount;
}

// return uint8_t value or -1 when failure
uint8_t spiTransferByte(spi_type *instance, uint8_t data)
{
    uint16_t spiTimeout = 1000;

    //while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET)
    while (spi_i2s_flag_get(instance, SPI_I2S_TDBE_FLAG) == RESET)
        if ((spiTimeout--) == 0)
            return spiTimeoutUserCallback(instance);
  
    //SPI_I2S_SendData(instance, data);
    spi_i2s_data_transmit(instance, data);

    spiTimeout = 1000;
    //while (SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_RXNE) == RESET)
    while (spi_i2s_flag_get(instance, SPI_I2S_RDBF_FLAG) == RESET)
        if ((spiTimeout--) == 0)
            return spiTimeoutUserCallback(instance);

    return ((uint8_t) spi_i2s_data_receive(instance));
}

/**
 * Return true if the bus is currently in the middle of a transmission.
 */
bool spiIsBusBusy(spi_type *instance)
{
    //return SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_TXE) == RESET || SPI_I2S_GetFlagStatus(instance, SPI_I2S_FLAG_BSY) == SET;
    return spi_i2s_flag_get(instance, SPI_I2S_TDBE_FLAG) == RESET || spi_i2s_flag_get(instance, SPI_I2S_BF_FLAG) == SET;
}

bool spiTransfer(spi_type *instance, uint8_t *out, const uint8_t *in, int len)
{
    uint16_t spiTimeout = 1000;

    instance->dt;
    while (len--) {
        uint8_t b = in ? *(in++) : 0xFF;
        while (spi_i2s_flag_get(instance, SPI_I2S_TDBE_FLAG) == RESET) {
            if ((spiTimeout--) == 0)
                return spiTimeoutUserCallback(instance);
        }
        spi_i2s_data_transmit(instance, b);
        spiTimeout = 1000;
        while (spi_i2s_flag_get(instance, SPI_I2S_RDBF_FLAG) == RESET) {
            if ((spiTimeout--) == 0)
                return spiTimeoutUserCallback(instance);
        }
        b = spi_i2s_data_receive(instance);
        if (out)
            *(out++) = b;
    }

    return true;
}

void spiSetSpeed(spi_type *instance, SPIClockSpeed_e speed)
{
    #define BR_CLEAR_MASK 0xFFC7
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return;
    }
 
    spi_enable (instance, FALSE); 

    // #define BR_BITS ((BIT(5) | BIT(4) | BIT(3)))
    // const uint16_t tempRegister = (instance->ctrl1 & ~BR_BITS);
    // instance->ctrl1 = tempRegister | (spiHardwareMap[device].divisorMap[speed] << 3);
    // #undef BR_BITS

    uint16_t tempRegister = instance->ctrl1;
    tempRegister &= BR_CLEAR_MASK;
    tempRegister |= (spiHardwareMap[device].divisorMap[speed] << 3);
    instance->ctrl1 = tempRegister;
    
    spi_enable (instance, TRUE);
}

uint16_t spiGetErrorCounter(spi_type *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device == SPIINVALID) {
        return 0;
    }
    return spiHardwareMap[device].errorCount;
}

void spiResetErrorCounter(spi_type *instance)
{
    SPIDevice device = spiDeviceByInstance(instance);
    if (device != SPIINVALID) {
        spiHardwareMap[device].errorCount = 0;
    }
}

spi_type * spiInstanceByDevice( SPIDevice device )
{
    return spiHardwareMap[device].dev;
}
#endif // USE_SPI

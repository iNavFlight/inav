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

#include "build/debug.h"

#include "common/utils.h"

#include "drivers/io.h"
#include "drivers/time.h"

#include "drivers/bus_i2c.h"
#include "drivers/nvic.h"
#include "io_impl.h"
#include "rcc.h"

#if !defined(SOFT_I2C) && defined(USE_I2C)

#define CLOCKSPEED 800000    // i2c clockspeed 400kHz default (conform specs), 800kHz  and  1200kHz (Betaflight default)

static void i2cUnstick(IO_t scl, IO_t sda);

#if defined(USE_I2C_PULLUP)
#define IOCFG_I2C IO_CONFIG(GPIO_MODE_AF_OD, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_PULLUP)
#else
#define IOCFG_I2C IOCFG_AF_OD
#endif

#ifndef I2C1_SCL
#define I2C1_SCL PB6
#endif

#ifndef I2C1_SDA
#define I2C1_SDA PB7
#endif

#ifndef I2C2_SCL
#define I2C2_SCL PB10
#endif
#ifndef I2C2_SDA
#define I2C2_SDA PB11
#endif

#ifndef I2C3_SCL
#define I2C3_SCL PA8
#endif
#ifndef I2C3_SDA
#define I2C3_SDA PB4
#endif

#if defined(USE_I2C_DEVICE_4)
#ifndef I2C4_SCL
#define I2C4_SCL PD12
#endif
#ifndef I2C4_SDA
#define I2C4_SDA PD13
#endif
#endif

static i2cDevice_t i2cHardwareMap[I2CDEV_COUNT] = {

#if defined(STM32F7)
    { .dev = I2C1, .scl = IO_TAG(I2C1_SCL), .sda = IO_TAG(I2C1_SDA), .rcc = RCC_APB1(I2C1), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C1_EV_IRQn, .er_irq = I2C1_ER_IRQn, .af = GPIO_AF4_I2C1 },
    { .dev = I2C2, .scl = IO_TAG(I2C2_SCL), .sda = IO_TAG(I2C2_SDA), .rcc = RCC_APB1(I2C2), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C2_EV_IRQn, .er_irq = I2C2_ER_IRQn, .af = GPIO_AF4_I2C2 },
    { .dev = I2C3, .scl = IO_TAG(I2C3_SCL), .sda = IO_TAG(I2C3_SDA), .rcc = RCC_APB1(I2C3), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C3_EV_IRQn, .er_irq = I2C3_ER_IRQn, .af = GPIO_AF4_I2C3 },
#if defined(USE_I2C_DEVICE_4)
    { .dev = I2C4, .scl = IO_TAG(I2C4_SCL), .sda = IO_TAG(I2C4_SDA), .rcc = RCC_APB1(I2C4), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C4_EV_IRQn, .er_irq = I2C4_ER_IRQn, .af = GPIO_AF4_I2C4 }
#endif
#elif defined(STM32H7)
    { .dev = I2C1, .scl = IO_TAG(I2C1_SCL), .sda = IO_TAG(I2C1_SDA), .rcc = RCC_APB1L(I2C1), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C1_EV_IRQn, .er_irq = I2C1_ER_IRQn, .af = GPIO_AF4_I2C1 },
    { .dev = I2C2, .scl = IO_TAG(I2C2_SCL), .sda = IO_TAG(I2C2_SDA), .rcc = RCC_APB1L(I2C2), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C2_EV_IRQn, .er_irq = I2C2_ER_IRQn, .af = GPIO_AF4_I2C2 },
    { .dev = I2C3, .scl = IO_TAG(I2C3_SCL), .sda = IO_TAG(I2C3_SDA), .rcc = RCC_APB1L(I2C3), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C3_EV_IRQn, .er_irq = I2C3_ER_IRQn, .af = GPIO_AF4_I2C3 },
#if defined(USE_I2C_DEVICE_4)
    { .dev = I2C4, .scl = IO_TAG(I2C4_SCL), .sda = IO_TAG(I2C4_SDA), .rcc = RCC_APB4(I2C4), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C4_EV_IRQn, .er_irq = I2C4_ER_IRQn, .af = GPIO_AF4_I2C4 }
#endif
#endif
};

static volatile uint16_t i2cErrorCount = 0;

// Note that I2C_TIMEOUT is in us, while the HAL
// functions expect the timeout to be in ticks.
// Since we're setting up the ticks a 1khz, each
// tick equals 1ms.
#define I2C_DEFAULT_TIMEOUT     (I2C_TIMEOUT / 1000)

typedef struct {
    bool initialised;
    I2C_HandleTypeDef handle;

    // Interrupt driven transfer started by i2cReadStart() / i2cWriteStart(), finished by the HAL callbacks below
    volatile bool busy;
    volatile bool error;
    timeUs_t startUs;           // start time, used to detect a transfer that never completes
    uint8_t txByte;             // data of a non-blocking single byte write, must outlive the call
} i2cState_t;

static i2cState_t i2cState[I2CDEV_COUNT];

// Transfer statistics shown with debug_mode = I2C, shared by all buses
typedef struct {
    uint16_t    eventIrqs;      // event interrupts during the current / last transfer
    uint16_t    errorIrqs;      // error interrupts since boot
    uint16_t    lastTransferUs; // start to completion of the last non-blocking transfer
    uint16_t    startCallUs;    // time spent in the last i2cStartTransfer() call
} i2cDebugStats_t;

static i2cDebugStats_t i2cStats;

static void i2cDebugPublish(void)
{
    DEBUG_SET(DEBUG_I2C, 0, i2cStats.eventIrqs);
    DEBUG_SET(DEBUG_I2C, 1, i2cStats.errorIrqs);
    DEBUG_SET(DEBUG_I2C, 2, i2cStats.lastTransferUs);
    DEBUG_SET(DEBUG_I2C, 5, i2cErrorCount);
    DEBUG_SET(DEBUG_I2C, 7, i2cStats.startCallUs);
}

static void i2cEventIrq(I2CDevice device)
{
    i2cStats.eventIrqs++;
    HAL_I2C_EV_IRQHandler(&i2cState[device].handle);
}

static void i2cErrorIrq(I2CDevice device)
{
    i2cStats.errorIrqs++;
    HAL_I2C_ER_IRQHandler(&i2cState[device].handle);
}

static i2cState_t *i2cStateFromHandle(const I2C_HandleTypeDef *hi2c)
{
    for (unsigned i = 0; i < ARRAYLEN(i2cState); i++) {
        if (&i2cState[i].handle == hi2c) {
            return &i2cState[i];
        }
    }
    return NULL;
}

// HAL completion hooks, called from the I2C interrupt handlers once an interrupt driven transfer is over
static void i2cTransferFinished(I2C_HandleTypeDef *hi2c, bool error)
{
    i2cState_t *state = i2cStateFromHandle(hi2c);
    if (!state) {
        return;
    }

    i2cStats.lastTransferUs = microsISR() - state->startUs;
    state->error = error;
    state->busy = false;
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2cTransferFinished(hi2c, false);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2cTransferFinished(hi2c, false);
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2cTransferFinished(hi2c, false);
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2cTransferFinished(hi2c, false);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    i2cTransferFinished(hi2c, true);
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2cTransferFinished(hi2c, true);
}

void i2cSetSpeed(uint8_t speed)
{
    for (unsigned int i = 0; i < ARRAYLEN(i2cHardwareMap); i++) {
        i2cHardwareMap[i].speed = speed;
    }
}

void I2C1_ER_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_1);
}

void I2C1_EV_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_1);
}

void I2C2_ER_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_2);
}

void I2C2_EV_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_2);
}

void I2C3_ER_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_3);
}

void I2C3_EV_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_3);
}

#ifdef USE_I2C_DEVICE_4
void I2C4_ER_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_4);
}

void I2C4_EV_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_4);
}
#endif


static bool i2cHandleHardwareFailure(I2CDevice device)
{
    i2cErrorCount++;
    i2cInit(device);
    return false;
}

// Block until no interrupt driven transfer is in progress. Returns false if a stuck transfer had to be aborted.
static bool i2cWaitForIdle(I2CDevice device)
{
    i2cState_t * state = &(i2cState[device]);
    const timeUs_t startUs = micros();

    while (state->busy) {
        if (cmpTimeUs(micros(), startUs) >= I2C_TIMEOUT) {
            return i2cHandleHardwareFailure(device);
        }
    }

    return true;
}

// Start an interrupt driven transfer. Returns false if the bus is busy or the transfer could not be started.
static bool i2cStartTransfer(I2CDevice device, uint8_t addr_, uint8_t reg_, bool allowRawAccess, bool reading, uint8_t len, uint8_t *buf)
{
    if (device == I2CINVALID || device >= I2CDEV_COUNT)
        return false;

    i2cState_t * state = &(i2cState[device]);

    if (!state->initialised || state->busy)
        return false;

    if (reading && len == 0)
        return false;

    const timeUs_t callStartUs = micros();
    i2cStats.eventIrqs = 0;

    state->error = false;
    state->startUs = callStartUs;
    state->busy = true;

    HAL_StatusTypeDef status;

    if (reading) {
        if (reg_ == 0xFF && allowRawAccess) {
            status = HAL_I2C_Master_Receive_IT(&state->handle, addr_ << 1, buf, len);
        }
        else {
            status = HAL_I2C_Mem_Read_IT(&state->handle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, buf, len);
        }
    }
    else if (len == 0) {
        if (reg_ == 0xFF && allowRawAccess) {
            state->busy = false;
            return false;   // nothing to send
        }
        // Register byte alone, the HAL refuses zero length transfers
        state->txByte = reg_;
        status = HAL_I2C_Master_Transmit_IT(&state->handle, addr_ << 1, &state->txByte, 1);
    }
    else {
        if (reg_ == 0xFF && allowRawAccess) {
            status = HAL_I2C_Master_Transmit_IT(&state->handle, addr_ << 1, buf, len);
        }
        else {
            status = HAL_I2C_Mem_Write_IT(&state->handle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, buf, len);
        }
    }

    if (status != HAL_OK) {
        state->busy = false;
        if (status == HAL_BUSY) {
            return false;   // HAL is still finishing the previous transfer (STOP in progress), try again later
        }
        return i2cHandleHardwareFailure(device);
    }

    i2cStats.startCallUs = micros() - callStartUs;
    i2cDebugPublish();

    return true;
}

bool i2cReadStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf, bool allowRawAccess)
{
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, true, len, buf);
}

bool i2cWriteBufferStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    // the platform API takes a non-const buffer, the data is only read
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, len_, (uint8_t *)data);
}

bool i2cWriteStart(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t data, bool allowRawAccess)
{
    if (device == I2CINVALID || device >= I2CDEV_COUNT || i2cState[device].busy)
        return false;   // don't touch txByte while it may still be going out

    i2cState[device].txByte = data;
    return i2cStartTransfer(device, addr_, reg_, allowRawAccess, false, 1, &i2cState[device].txByte);
}

bool i2cBusy(I2CDevice device, bool *error)
{
    if (device == I2CINVALID || device >= I2CDEV_COUNT) {
        if (error) {
            *error = true;
        }
        return false;
    }

    i2cState_t * state = &(i2cState[device]);

    if (state->busy && cmpTimeUs(micros(), state->startUs) >= I2C_TIMEOUT) {
        // No completion callback within the timeout, the transfer is stuck - reset the peripheral
        i2cHandleHardwareFailure(device);
        state->error = true;
    }

    if (error) {
        *error = state->error;
    }

    i2cDebugPublish();

    return state->busy;
}

bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    if (device == I2CINVALID)
        return false;

    i2cState_t * state = &(i2cState[device]);

    if (!state->initialised)
        return false;

    // Let a non-blocking transfer of another device finish first
    if (!i2cWaitForIdle(device))
        return false;

    HAL_StatusTypeDef status;

    if (len_ == 0 && !(reg_ == 0xFF && allowRawAccess)) {
        // Register byte alone, the HAL refuses zero length transfers
        uint8_t regByte = reg_;
        status = HAL_I2C_Master_Transmit(&state->handle, addr_ << 1, &regByte, 1, I2C_DEFAULT_TIMEOUT);
    }
    else if ((reg_ == 0xFF || len_ == 0) && allowRawAccess) {
        status = HAL_I2C_Master_Transmit(&state->handle, addr_ << 1, (uint8_t *)data, len_, I2C_DEFAULT_TIMEOUT);
    }
    else {
        status = HAL_I2C_Mem_Write(&state->handle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, len_, I2C_DEFAULT_TIMEOUT);
    }

    if (status != HAL_OK)
        return i2cHandleHardwareFailure(device);

    return true;
}

bool i2cWrite(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t data, bool allowRawAccess)
{
    return i2cWriteBuffer(device, addr_, reg_, 1, &data, allowRawAccess);
}

bool i2cRead(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len, uint8_t* buf, bool allowRawAccess)
{
    if (device == I2CINVALID)
        return false;

    i2cState_t * state = &(i2cState[device]);

    if (!state->initialised)
        return false;

    // Let a non-blocking transfer of another device finish first
    if (!i2cWaitForIdle(device))
        return false;

    HAL_StatusTypeDef status;

    if (reg_ == 0xFF && allowRawAccess) {
        status = HAL_I2C_Master_Receive(&state->handle, addr_ << 1,buf, len, I2C_DEFAULT_TIMEOUT);
    }
    else {
        status = HAL_I2C_Mem_Read(&state->handle, addr_ << 1, reg_, I2C_MEMADD_SIZE_8BIT,buf, len, I2C_DEFAULT_TIMEOUT);
    }

    if (status != HAL_OK)
        return i2cHandleHardwareFailure(device);

    return true;
}

/*
 * Compute SCLDEL, SDADEL, SCLH and SCLL for TIMINGR register according to reference manuals.
 */
static void i2cClockComputeRaw(uint32_t pclkFreq, int i2cFreqKhz, int presc, int dfcoeff,
                       uint8_t *scldel, uint8_t *sdadel, uint16_t *sclh, uint16_t *scll)
{
    // Values from I2C-SMBus specification
    uint16_t trmax;      // Raise time (max)
    uint16_t tfmax;      // Fall time (max)
    uint8_t  tsuDATmin;  // SDA setup time (min)
    uint8_t  thdDATmin;  // SDA hold time (min)

    // Silicon specific values, from datasheet
    uint8_t  tAFmin;     // Analog filter delay (min)
    //uint8_t  tAFmax;     // Analog filter delay (max)

    // Actual (estimated) values
    uint16_t tr = 100;   // Raise time
    uint16_t tf = 100;   // Fall time
    uint8_t  tAF = 70;   // Analog filter delay

    if (i2cFreqKhz > 400) {
        // Fm+ (Fast mode plus)
        trmax = 120;
        tfmax = 120;
        tsuDATmin = 50;
        thdDATmin = 0;
    } else {
        // Fm (Fast mode)
        trmax = 300;
        tfmax = 300;
        tsuDATmin = 100;
        thdDATmin = 0;
    }

    tAFmin = 50;
    //tAFmax = 90;  // Unused

    // Convert pclkFreq into nsec
    float tI2cclk = 1000000000.0f / pclkFreq;

    // Convert target i2cFreq into cycle time (nsec)
    float tSCL = 1000000.0f / i2cFreqKhz;

    uint32_t SCLDELmin = (trmax + tsuDATmin)/((presc + 1) * tI2cclk) - 1;

    uint32_t SDADELmin = (tfmax + thdDATmin - tAFmin - ((dfcoeff + 3) * tI2cclk)) / ((presc + 1) * tI2cclk);

    float tsync1 = tf + tAF + dfcoeff * tI2cclk + 3 * tI2cclk;
    float tsync2 = tr + tAF + dfcoeff * tI2cclk + 3 * tI2cclk;

    float tSCLHL = tSCL - tsync1 - tsync2;
    float SCLHL = tSCLHL / ((presc + 1) * tI2cclk) - 1;

    uint32_t SCLH = SCLHL / 4.75;  // STM32CubeMX seems to use a value like this
    uint32_t SCLL = (uint32_t)(SCLHL + 0.5f) - SCLH;

    *scldel = SCLDELmin;
    *sdadel = SDADELmin;
    *sclh = SCLH - 1;
    *scll = SCLL - 1;
}

static uint32_t i2cClockTIMINGR(uint32_t pclkFreq, int i2cFreqKhz, int dfcoeff)
{
#define TIMINGR(presc, scldel, sdadel, sclh, scll) \
    ((presc << 28)|(scldel << 20)|(sdadel << 16)|(sclh << 8)|(scll << 0))

    uint8_t scldel;
    uint8_t sdadel;
    uint16_t sclh;
    uint16_t scll;

    for (int presc = 1; presc < 15; presc++) {
        i2cClockComputeRaw(pclkFreq, i2cFreqKhz, presc, dfcoeff, &scldel, &sdadel, &sclh, &scll);

        // If all fields are not overflowing, return TIMINGR.
        // Otherwise, increase prescaler and try again.
        if ((scldel < 16) && (sdadel < 16) && (sclh < 256) && (scll < 256)) {
            return TIMINGR(presc, scldel, sdadel, sclh, scll);
        }
    }
    return 0; // Shouldn't reach here
}

void i2cInit(I2CDevice device)
{
    i2cDevice_t * hardware = &(i2cHardwareMap[device]);
    i2cState_t * state = &(i2cState[device]);
    I2C_HandleTypeDef * pHandle = &state->handle;


    if (hardware->dev == NULL)
        return;

/*
    if (state->initialised)
        return;
*/

    // Enable RCC
    RCC_ClockCmd(hardware->rcc, ENABLE);

    IO_t scl = IOGetByTag(hardware->scl);
    IO_t sda = IOGetByTag(hardware->sda);

    IOInit(scl, OWNER_I2C, RESOURCE_I2C_SCL, RESOURCE_INDEX(device));
    IOInit(sda, OWNER_I2C, RESOURCE_I2C_SDA, RESOURCE_INDEX(device));

    i2cUnstick(scl, sda);

    // Init pins
    IOConfigGPIOAF(scl, IOCFG_I2C, hardware->af);
    IOConfigGPIOAF(sda, IOCFG_I2C, hardware->af);

    // Init I2C peripheral
    if (state->initialised) {
        // Re-initialisation after a failure: drop whatever transfer the HAL still thinks is running
        HAL_I2C_DeInit(pHandle);
    }
    state->busy = false;
    state->error = false;

    pHandle->Instance = hardware->dev;

/*
    switch (i2c->speed) {
        case I2C_SPEED_400KHZ:
        default:
            pHandle->Init.Timing = 0x00A01B5B;  // 400kHz, Rise 100ns, Fall 10ns    0x00500B6A
            break;

        case I2C_SPEED_800KHZ:
            pHandle->Init.Timing = 0x00401B1B;  // 800khz, Rise 40, Fall 4
            break;

        case I2C_SPEED_100KHZ:
            pHandle->Init.Timing = 0x60100544;  // 100kHz, Rise 100ns, Fall 10ns
            break;

        case I2C_SPEED_200KHZ:
            pHandle->Init.Timing = 0x00A01EDF;  // 200kHz, Rise 100ns, Fall 10ns
            break;
    }
*/

    // Compute TIMINGR value based on peripheral clock for this device instance

    uint32_t i2cPclk;

#if defined(STM32F7) || defined(STM32G4)
    // F7 Clock source configured in startup/system_stm32f7xx.c as:
    //   I2C1234 : PCLK1
    // G4 Clock source configured in startup/system_stm32g4xx.c as:
    //   I2C1234 : PCLK1
    i2cPclk = HAL_RCC_GetPCLK1Freq();
#elif defined(STM32H7)
    // Clock sources configured in startup/system_stm32h7xx.c as:
    //   I2C123 : D2PCLK1 (rcc_pclk1 for APB1)
    //   I2C4   : D3PCLK1 (rcc_pclk4 for APB4)
    i2cPclk = (hardware->dev == I2C4) ? HAL_RCCEx_GetD3PCLK1Freq() : HAL_RCC_GetPCLK1Freq();
#else
    #error Unknown MCU type
#endif

    switch (hardware->speed) {
        case I2C_SPEED_400KHZ:
        default:
            pHandle->Init.Timing = i2cClockTIMINGR(i2cPclk, 400, 0);    // 400kHz, Rise 100ns, Fall 10ns    0x00500B6A
            break;

        case I2C_SPEED_800KHZ:
            pHandle->Init.Timing = i2cClockTIMINGR(i2cPclk, 800, 0);    // 800khz, Rise 40, Fall 4
            break;

        case I2C_SPEED_100KHZ:
            pHandle->Init.Timing = i2cClockTIMINGR(i2cPclk, 100, 0);    // 100kHz, Rise 100ns, Fall 10ns
            break;

        case I2C_SPEED_200KHZ:
            pHandle->Init.Timing = i2cClockTIMINGR(i2cPclk, 200, 0);    // 200kHz, Rise 100ns, Fall 10ns
            break;
    }

    pHandle->Init.OwnAddress1     = 0x0;
    pHandle->Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    pHandle->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    pHandle->Init.OwnAddress2     = 0x0;
    pHandle->Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    pHandle->Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;


    HAL_I2C_Init(pHandle);
    /* Enable the Analog I2C Filter */
    HAL_I2CEx_ConfigAnalogFilter(pHandle, I2C_ANALOGFILTER_ENABLE);

    HAL_NVIC_SetPriority(hardware->er_irq, NVIC_PRIO_I2C_ER, 0);
    HAL_NVIC_EnableIRQ(hardware->er_irq);

    HAL_NVIC_SetPriority(hardware->ev_irq, NVIC_PRIO_I2C_EV, 0);
    HAL_NVIC_EnableIRQ(hardware->ev_irq);

    state->initialised = true;
}

uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

static void i2cUnstick(IO_t scl, IO_t sda)
{
    int i;

    IOHi(scl);
    IOHi(sda);

    IOConfigGPIO(scl, IOCFG_OUT_OD);
    IOConfigGPIO(sda, IOCFG_OUT_OD);

    // Analog Devices AN-686
    // We need 9 clock pulses + STOP condition
    for (i = 0; i < 9; i++) {
        // Wait for any clock stretching to finish
        int timeout = 100;
        while (!IORead(scl) && timeout) {
            delayMicroseconds(5);
            timeout--;
        }

        // Pull low
        IOLo(scl); // Set bus low
        delayMicroseconds(5);
        IOHi(scl); // Set bus high
        delayMicroseconds(5);
    }

    // Generate a stop condition in case there was none
    IOLo(scl);
    delayMicroseconds(5);
    IOLo(sda);
    delayMicroseconds(5);

    IOHi(scl); // Set bus scl high
    delayMicroseconds(5);
    IOHi(sda); // Set bus sda high
}

#endif

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
#include "io_impl.h"
#include "drivers/io.h"
#include "drivers/time.h"
#include "rcc.h"
#include "drivers/bus_i2c.h"
#include "drivers/nvic.h"
#include "drivers/i2c_application.h"


#if !defined(SOFT_I2C) && defined(USE_I2C)

#define CLOCKSPEED 800000    // i2c clockspeed 400kHz default (conform specs), 800kHz  and  1200kHz (Betaflight default)

#define I2Cx_ADDRESS                     0x00
// Clock period in us during unstick transfer
#define UNSTICK_CLK_US 10
// Allow 500us for clock strech to complete during unstick
#define UNSTICK_CLK_STRETCH (500/UNSTICK_CLK_US)

static void i2cUnstick(IO_t scl, IO_t sda);

#if defined(USE_I2C_PULLUP)
#define IOCFG_I2C IOCFG_AF_OD_UP
#else
#define IOCFG_I2C IOCFG_AF_OD
#endif

#ifndef I2C1_SCL
#define I2C1_SCL PA9
#endif

#ifndef I2C1_SDA
#define I2C1_SDA PA10
#endif

#ifndef I2C2_SCL
#define I2C2_SCL PD12
#endif
#ifndef I2C2_SDA
#define I2C2_SDA PD13
#endif

#ifndef I2C3_SCL
#define I2C3_SCL PC0
#endif
#ifndef I2C3_SDA
#define I2C3_SDA PC1
#endif

//Define thi I2C hardware map
static i2cDevice_t i2cHardwareMap[I2CDEV_COUNT] = {
    { .dev = I2C1, .scl = IO_TAG(I2C1_SCL), .sda = IO_TAG(I2C1_SDA), .rcc = RCC_APB1(I2C1), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C1_EVT_IRQn, .er_irq = I2C1_ERR_IRQn, .af = GPIO_MUX_4 },
    { .dev = I2C2, .scl = IO_TAG(I2C2_SCL), .sda = IO_TAG(I2C2_SDA), .rcc = RCC_APB1(I2C2), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C2_EVT_IRQn, .er_irq = I2C2_ERR_IRQn, .af = GPIO_MUX_4 },
    { .dev = I2C3, .scl = IO_TAG(I2C3_SCL), .sda = IO_TAG(I2C3_SDA), .rcc = RCC_APB1(I2C3), .speed = I2C_SPEED_400KHZ, .ev_irq = I2C3_EVT_IRQn, .er_irq = I2C3_ERR_IRQn, .af = GPIO_MUX_4 },
};

static volatile uint16_t i2cErrorCount = 0;

// Note that I2C_TIMEOUT is in us, while the HAL
// functions expect the timeout to be in ticks.
// Since we're setting up the ticks a 1khz, each
// tick equals 1ms.
// AT32F4 i2c TIMEOUT USING loop times 
#define I2C_DEFAULT_TIMEOUT     (I2C_TIMEOUT*288 / 1000  )
//#define I2C_DEFAULT_TIMEOUT     (0xD80)

typedef struct {
    bool initialised;
    i2c_handle_type handle;

    // Interrupt driven transfer started by i2cReadStart() / i2cWriteStart(). The i2c_application layer finishes it
    // from the interrupt handlers: handle.status becomes I2C_END, or handle.error_code reports the failure.
    bool pending;
    timeUs_t startUs;           // start time, used to detect a transfer that never completes
    uint8_t txByte;             // data of a non-blocking single byte write, must outlive the call
} i2cState_t;

static i2cState_t i2cState[I2CDEV_COUNT];

// Transfer statistics shown with debug_mode = I2C, shared by all buses
typedef struct {
    uint16_t    eventIrqs;      // interrupts during the current / last transfer
    uint16_t    errorIrqs;      // error interrupts / aborted transfers since boot
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
    i2c_evt_irq_handler(&i2cState[device].handle);
}

static void i2cErrorIrq(I2CDevice device)
{
    i2cStats.errorIrqs++;
    i2c_err_irq_handler(&i2cState[device].handle);
}

void i2cSetSpeed(uint8_t speed)
{
    for (unsigned int i = 0; i < ARRAYLEN(i2cHardwareMap); i++) {
        i2cHardwareMap[i].speed = speed;
    }
}
 
//I2C1_ERR_IRQHandler
void I2C1_ERR_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_1);
}

void I2C1_EVT_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_1);
}

void I2C2_ERR_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_2);
}

void I2C2_EVT_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_2);
}

void I2C3_ERR_IRQHandler(void)
{
    i2cErrorIrq(I2CDEV_3);
}

void I2C3_EVT_IRQHandler(void)
{
    i2cEventIrq(I2CDEV_3);
}

static bool i2cHandleHardwareFailure(I2CDevice device)
{
    i2cErrorCount++;
    i2cInit(device);
    return false;
}

// Pick up the outcome of a pending interrupt driven transfer and reset a stuck one
static void i2cPollPending(I2CDevice device)
{
    i2cState_t * state = &(i2cState[device]);

    if (!state->pending) {
        return;
    }

    // The end-of-transfer status is private to i2c_application.c, i2c_wait_end() with a zero timeout only probes it:
    // I2C_ERR_TIMEOUT means the transfer is still running, anything else means it finished
    if (state->handle.error_code != I2C_OK || i2c_wait_end(&state->handle, 0) != I2C_ERR_TIMEOUT) {
        i2cStats.lastTransferUs = micros() - state->startUs;
        state->pending = false;
    }
    else if (cmpTimeUs(micros(), state->startUs) >= I2C_TIMEOUT) {
        // No completion within the timeout, the transfer is stuck - reset the peripheral
        i2cHandleHardwareFailure(device);
        state->handle.error_code = I2C_ERR_TIMEOUT;
    }
}

// Block until no interrupt driven transfer is in progress
static void i2cWaitForIdle(I2CDevice device)
{
    do {
        i2cPollPending(device);
    } while (i2cState[device].pending);
}

/*
 * Start an interrupt driven transfer. The i2c_application layer sends the slave and register address synchronously
 * (a few tens of microseconds), the data phase runs from the interrupt handlers.
 * Returns false if the bus is busy or the transfer could not be started.
 */
static bool i2cStartTransfer(I2CDevice device, uint8_t addr_, uint8_t reg_, bool allowRawAccess, bool reading, uint8_t len, uint8_t *buf)
{
    if (device == I2CINVALID || device >= I2CDEV_COUNT)
        return false;

    i2cState_t * state = &(i2cState[device]);

    if (!state->initialised)
        return false;

    if (reading && len == 0)
        return false;

    i2cPollPending(device);
    if (state->pending)
        return false;

    const timeUs_t callStartUs = micros();
    i2cStats.eventIrqs = 0;

    i2c_status_type status;

    if (reading) {
        if (reg_ == 0xFF && allowRawAccess) {
            status = i2c_master_receive_int(&state->handle, addr_ << 1, buf, len, I2C_DEFAULT_TIMEOUT);
        }
        else {
            status = i2c_memory_read_int(&state->handle, I2C_MEM_ADDR_WIDIH_8, addr_ << 1, reg_, buf, len, I2C_DEFAULT_TIMEOUT);
        }
    }
    else if (len == 0) {
        if (reg_ == 0xFF && allowRawAccess) {
            return false;   // nothing to send
        }
        // Register byte alone, sent as a plain one byte transfer
        state->txByte = reg_;
        status = i2c_master_transmit_int(&state->handle, addr_ << 1, &state->txByte, 1, I2C_DEFAULT_TIMEOUT);
    }
    else {
        if (reg_ == 0xFF && allowRawAccess) {
            status = i2c_master_transmit_int(&state->handle, addr_ << 1, buf, len, I2C_DEFAULT_TIMEOUT);
        }
        else {
            status = i2c_memory_write_int(&state->handle, I2C_MEM_ADDR_WIDIH_8, addr_ << 1, reg_, buf, len, I2C_DEFAULT_TIMEOUT);
        }
    }

    if (status == I2C_ERR_STEP_1) {
        return false;   // bus still busy, try again later
    }

    if (status != I2C_OK) {
        /* wait for the stop flag to be set and clear it */
        i2c_wait_flag(&state->handle, I2C_STOPF_FLAG, I2C_EVENT_CHECK_NONE, I2C_DEFAULT_TIMEOUT);
        i2c_flag_clear(state->handle.i2cx, I2C_STOPF_FLAG);
        return i2cHandleHardwareFailure(device);
    }

    state->startUs = callStartUs;
    state->pending = true;

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
    if (device == I2CINVALID || device >= I2CDEV_COUNT)
        return false;

    i2cPollPending(device);
    if (i2cState[device].pending)
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

    i2cPollPending(device);

    if (error) {
        *error = state->handle.error_code != I2C_OK;
    }

    i2cDebugPublish();

    return state->pending;
}

bool i2cWriteBuffer(I2CDevice device, uint8_t addr_, uint8_t reg_, uint8_t len_, const uint8_t *data, bool allowRawAccess)
{
    if (device == I2CINVALID)
        return false;

    i2cState_t * state = &(i2cState[device]);

    if (!state->initialised)
        return false;

    // Let a non-blocking transfer of another device finish first
    i2cWaitForIdle(device);

    i2c_status_type status;

    if ((reg_ == 0xFF || len_ == 0) && allowRawAccess) {
        status = i2c_master_transmit(&state->handle, addr_ << 1, CONST_CAST(uint8_t*, data), len_, I2C_DEFAULT_TIMEOUT);
        if(status !=  I2C_OK)
        {
          /* wait for the stop flag to be set  */
          i2c_wait_flag(&state->handle, I2C_STOPF_FLAG, I2C_EVENT_CHECK_NONE, I2C_DEFAULT_TIMEOUT);

          /* clear stop flag */
      	  i2c_flag_clear(state->handle.i2cx, I2C_STOPF_FLAG);
        }
    }
    else {
        status = i2c_memory_write(&state->handle,I2C_MEM_ADDR_WIDIH_8, addr_ << 1, reg_, CONST_CAST(uint8_t*, data), len_, I2C_DEFAULT_TIMEOUT);
        //status = i2c_memory_write_int(&state->handle,I2C_MEM_ADDR_WIDIH_8, addr_ << 1, reg_,  data, len_, I2C_DEFAULT_TIMEOUT);
        
        if(status !=  I2C_OK)
        {
          /* wait for the stop flag to be set  */
          i2c_wait_flag(&state->handle, I2C_STOPF_FLAG, I2C_EVENT_CHECK_NONE, I2C_DEFAULT_TIMEOUT);

          /* clear stop flag */
      	  i2c_flag_clear(state->handle.i2cx, I2C_STOPF_FLAG);
        }
    }

    if (status == I2C_ERR_STEP_1) {//BUSY
        return false;
    }

    if (status != I2C_OK)
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
    i2cWaitForIdle(device);

    i2c_status_type status;
    if (reg_ == 0xFF && allowRawAccess) { 
        status = i2c_master_receive(&state->handle, addr_ << 1,buf, len, I2C_DEFAULT_TIMEOUT);
        if(status !=  I2C_OK)
        {
          /* wait for the stop flag to be set  */
          i2c_wait_flag(&state->handle, I2C_STOPF_FLAG, I2C_EVENT_CHECK_NONE, I2C_DEFAULT_TIMEOUT);

          /* clear stop flag */
      	  i2c_flag_clear(state->handle.i2cx, I2C_STOPF_FLAG);
        }

    }
    else { 
        status = i2c_memory_read(&state->handle, I2C_MEM_ADDR_WIDIH_8,addr_ << 1, reg_, buf, len, I2C_DEFAULT_TIMEOUT);

        if(status !=  I2C_OK)
        {
          /* wait for the stop flag to be set  */
          i2c_wait_flag(&state->handle, I2C_STOPF_FLAG, I2C_EVENT_CHECK_NONE, I2C_DEFAULT_TIMEOUT);

          /* clear stop flag */
      	  i2c_flag_clear(state->handle.i2cx, I2C_STOPF_FLAG);
        }
        
    }

    if (status != I2C_OK)
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
    uint16_t trmax;      // Rise time (max)
    uint16_t tfmax;      // Fall time (max)
    uint8_t tsuDATmin;   // SDA setup time (min)
    uint8_t thdDATmin;   // SDA hold time (min)
    uint16_t tHIGHmin;   // High period of SCL clock (min)
    uint16_t tLOWmin;    // Low period of SCL clock (min)

    // Silicon specific values, from datasheet
    uint8_t tAFmin = 50; // Analog filter delay (min)

    // Actual (estimated) values
    uint8_t tr = 100;   // Rise time
    uint8_t tf = 10;    // Fall time

    if (i2cFreqKhz > 400) {
        // Fm+ (Fast mode plus)
        trmax = 120;
        tfmax = 120;
        tsuDATmin = 50;
        thdDATmin = 0;
        tHIGHmin = 260;
        tLOWmin = 500;
    } else {
        // Fm (Fast mode)
        trmax = 300;
        tfmax = 300;
        tsuDATmin = 100;
        thdDATmin = 0;
        tHIGHmin = 600;
        tLOWmin = 1300;
    }

    // Convert pclkFreq into nsec
    float tI2cclk = 1000000000.0f / pclkFreq;

    // Convert target i2cFreq into cycle time (nsec)
    float tSCL = 1000000.0f / i2cFreqKhz;

    uint32_t SCLDELmin = (trmax + tsuDATmin) / ((presc + 1) * tI2cclk) - 1;
    uint32_t SDADELmin = (tfmax + thdDATmin - tAFmin - ((dfcoeff + 3) * tI2cclk)) / ((presc + 1) * tI2cclk);

    float tsync1 = tf + tAFmin + dfcoeff * tI2cclk + 2 * tI2cclk;
    float tsync2 = tr + tAFmin + dfcoeff * tI2cclk + 2 * tI2cclk;

    float tSCLH = tHIGHmin * tSCL / (tHIGHmin + tLOWmin) - tsync2;
    float tSCLL = tSCL - tSCLH - tsync1 - tsync2;

    uint32_t SCLH = tSCLH / ((presc + 1) * tI2cclk) - 1;
    uint32_t SCLL = tSCLL / ((presc + 1) * tI2cclk) - 1;

    while (tsync1 + tsync2 + ((SCLH + 1) + (SCLL + 1)) * ((presc + 1) * tI2cclk) < tSCL) {
        SCLH++;
    }

    *scldel = SCLDELmin;
    *sdadel = SDADELmin;
    *sclh = SCLH;
    *scll = SCLL;
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

    //I2C_HandleTypeDef * pHandle = &state->handle;
    i2c_handle_type * pHandle = &state->handle;

    if (hardware->dev == NULL)
        return;

    IO_t scl = IOGetByTag(hardware->scl);
    IO_t sda = IOGetByTag(hardware->sda);

    IOInit(scl, OWNER_I2C, RESOURCE_I2C_SCL, RESOURCE_INDEX(device));
    IOInit(sda, OWNER_I2C, RESOURCE_I2C_SDA, RESOURCE_INDEX(device));
    // Enable RCC
    RCC_ClockCmd(hardware->rcc, ENABLE);
    
    i2cUnstick(scl, sda);

    // Init pins
    // PB13 routes I2C3_SCL through AF7; its AF4 function is I2C3_SMBA.
    const uint8_t sclAf = (hardware->dev == I2C3 && hardware->scl == IO_TAG(PB13))
        ? GPIO_MUX_7 : hardware->af;
    IOConfigGPIOAF(scl, IOCFG_I2C, sclAf);
    IOConfigGPIOAF(sda, IOCFG_I2C, hardware->af);

    // Init I2C peripheral
    pHandle->i2cx = hardware->dev;
    i2c_reset(pHandle->i2cx);
    // Compute TIMINGR value based on peripheral clock for this device instance
    uint32_t i2cPclk; 
 
    #if defined(AT32F43x)  
        crm_clocks_freq_type clocks_struct;
        crm_clocks_freq_get(&clocks_struct);
        i2cPclk = clocks_struct.apb1_freq;   

    #else
        #error Unknown MCU type
    #endif
   
 
    // switch (hardware->speed) {
    //     case I2C_SPEED_400KHZ:
    //     default:
    //         i2c_init(pHandle->i2cx, 15, 0x10F03863);    // 400kHz, Rise 100ns, Fall 10ns  0x10C03863
    //         break;
 
    //     case I2C_SPEED_800KHZ:  
    //         i2c_init(pHandle->i2cx, 15, 0x00E03259);    // 800khz, Rise 40, Fall 4
    //         break;

    //     case I2C_SPEED_100KHZ:
    //         i2c_init(pHandle->i2cx, 15, 0x30E0AEAE);     // 100kHz, Rise 100ns, Fall 10ns 0x30607EE0 0x30607DDE
    //         break;
            
    //     case I2C_SPEED_200KHZ:
    //         i2c_init(pHandle->i2cx, 15, 0x10F078D6);      // 200kHz, Rise 100ns, Fall 10ns  0x10C078D6
    //         break;
    // }
 
  
 switch (hardware->speed) {
        case I2C_SPEED_400KHZ:
        default:
             i2c_init(pHandle->i2cx, 0x0f, i2cClockTIMINGR(i2cPclk, 400, 0));
            break;

        case I2C_SPEED_800KHZ:
             i2c_init(pHandle->i2cx, 0x0f, i2cClockTIMINGR(i2cPclk, 800, 0));
            break;

        case I2C_SPEED_100KHZ:
            i2c_init(pHandle->i2cx, 0x0f, i2cClockTIMINGR(i2cPclk, 100, 0));
            break;

        case I2C_SPEED_200KHZ:
            i2c_init(pHandle->i2cx, 0x0f, i2cClockTIMINGR(i2cPclk, 200, 0));
            break;
    }

    i2c_own_address1_set(pHandle->i2cx, I2C_ADDRESS_MODE_7BIT, 0x0);
    //i2c_own_address2_enable(pHandle->i2cx, false); // enable or disable own address 2
    //i2c_own_address2_set(pHandle->i2cx, I2C_ADDRESS_MODE_7BIT, 0x0);
    //i2c_general_call_enable(pHandle->i2cx, false); // enable or disable general call mode
    //i2c_clock_stretch_enable(pHandle->i2cx, true); // enable or disable clock stretch
    
    nvic_irq_enable(hardware->er_irq,NVIC_PRIO_I2C_ER, 0);
    nvic_irq_enable(hardware->ev_irq, NVIC_PRIO_I2C_EV,0);

    i2c_enable(pHandle->i2cx, TRUE);

    state->pending = false;
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
        int timeout = UNSTICK_CLK_STRETCH;
        while (!IORead(scl) && timeout) {
            delayMicroseconds(UNSTICK_CLK_US);
            timeout--;
        }

        // Pull low
        IOLo(scl); // Set bus low
        delayMicroseconds(UNSTICK_CLK_US/2);
        IOHi(scl); // Set bus high
        delayMicroseconds(UNSTICK_CLK_US/2);
    }

    // Generate a stop condition in case there was none
    IOLo(scl);
    delayMicroseconds(UNSTICK_CLK_US/2);
    IOLo(sda);
    delayMicroseconds(UNSTICK_CLK_US/2);

    IOHi(scl); // Set bus scl high
    delayMicroseconds(UNSTICK_CLK_US/2);
    IOHi(sda); // Set bus sda high
}


#endif

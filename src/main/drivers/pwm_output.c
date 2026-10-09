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
#include <math.h>
#include <string.h>

#include "platform.h"

#if !defined(SITL_BUILD) && !defined(RP2350)

#include "build/atomic.h"
#include "build/debug.h"
#include "build/build_config.h"

#include "common/log.h"
#include "common/maths.h"
#include "common/circular_queue.h"

#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/nvic.h"
#include "drivers/bidir_dshot.h"
#include "drivers/time.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "io/motor_srxl2.h"
#include "io/servo_sbus.h"
#include "sensors/esc_sensor.h"

#include "config/feature.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "drivers/timer_impl.h"
#include "drivers/timer.h"

#define MULTISHOT_5US_PW    (MULTISHOT_TIMER_HZ * 5 / 1000000.0f)
#define MULTISHOT_20US_MULT (MULTISHOT_TIMER_HZ * 20 / 1000000.0f / 1000.0f)

#ifdef USE_DSHOT
/* Timer clock per DSHOT rate: DSHOT_MOTOR_BITLENGTH (20) ticks make one bit, so
 * 12 MHz / 20 = 600 kbit/s. */
#define MOTOR_DSHOT600_HZ     12000000
#define MOTOR_DSHOT300_HZ     6000000
#define MOTOR_DSHOT150_HZ     3000000
/* Fastest rate INAV supports; static buffers that depend on the bit period (keep-alive
 * buffer) are sized from it, so update this when a faster rate is added. */
#define MOTOR_DSHOT_FASTEST_HZ MOTOR_DSHOT600_HZ


#define DSHOT_MOTOR_BIT_0       7
#define DSHOT_MOTOR_BIT_1       14
#define DSHOT_MOTOR_BITLENGTH   20

#define DSHOT_DMA_BUFFER_SIZE   18 /* resolution + frame reset (2us) */
#define MAX_DMA_TIMERS          8

#ifdef USE_DSHOT_BIDIR
/* Telemetry reply window, measured from the frame start: the 18-slot frame, then the ESC's
 * ~30 us turnaround and its 21-bit GCR reply at 5/4 of the DSHOT bit rate (17 slots).
 * DSHOT_TELEMETRY_DEADTIME_US covers the turnaround plus margin for ESC timing variation. */
#define DSHOT_TELEMETRY_DEADTIME_US     45
#define DSHOT_TELEMETRY_WINDOW_SLOTS    (DSHOT_DMA_BUFFER_SIZE + 17)
#define GCR_TELEMETRY_INPUT_LEN MAX_GCR_EDGES
/* A bidir port captures the GCR edges of the ESC reply into the same buffer it sends the
 * frame from, so the buffer is sized for the larger of the two */
#define DSHOT_PORT_DMA_BUFFER_SIZE GCR_TELEMETRY_INPUT_LEN
#else
#define DSHOT_PORT_DMA_BUFFER_SIZE DSHOT_DMA_BUFFER_SIZE
#endif

/* Keep-alive frame replayed by circular DMA while the CPU is stalled by a flash write:
 * 16 data bits followed by an idle (line low) gap of DSHOT_KEEPALIVE_GAP_US.
 * One DMA slot is one DSHOT bit period, so the gap needs gapUs * dshotHz / bitLength slots. */
#define DSHOT_KEEPALIVE_GAP_US      40
#define DSHOT_KEEPALIVE_SLOTS(dshotHz)  (16 + (DSHOT_KEEPALIVE_GAP_US * ((dshotHz) / 1000000) + DSHOT_MOTOR_BITLENGTH - 1) / DSHOT_MOTOR_BITLENGTH)
/* The buffer is static, so it is sized for the fastest rate: the shorter the bit period,
 * the more slots a 40 us gap needs (40 slots at DSHOT600, 28 at DSHOT300, 22 at DSHOT150).
 * Only DSHOT_KEEPALIVE_SLOTS(actual rate) slots are used at run time. */
#define DSHOT_KEEPALIVE_BUFFER_SIZE     DSHOT_KEEPALIVE_SLOTS(MOTOR_DSHOT_FASTEST_HZ)
/* Bound for the waits at the keep-alive transitions: two keep-alive cycles at DSHOT150 */
#define DSHOT_KEEPALIVE_WAIT_TIMEOUT_US 400

#define DSHOT_COMMAND_DELAY_US 1000
#define DSHOT_COMMAND_INTERVAL_US 10000
#define DSHOT_COMMAND_QUEUE_LENGTH 8
#define DHSOT_COMMAND_QUEUE_SIZE   DSHOT_COMMAND_QUEUE_LENGTH * sizeof(dshotCommands_e)
#endif

typedef void (*pwmWriteFuncPtr)(uint8_t index, uint16_t value);  // function pointer used to write motors

#ifdef USE_DSHOT_DMAR
    timerDMASafeType_t dmaBurstBuffer[MAX_DMA_TIMERS][DSHOT_DMA_BUFFER_SIZE * 4];
#endif

#ifdef USE_DSHOT
// Every motor replays the same zero-throttle keep-alive frame, so one buffer feeds all DMA streams
#ifdef USE_DSHOT_DMAR
static DMA_RAM timerDMASafeType_t dshotKeepaliveBuffer[DSHOT_KEEPALIVE_BUFFER_SIZE * 4];
#else
static DMA_RAM timerDMASafeType_t dshotKeepaliveBuffer[DSHOT_KEEPALIVE_BUFFER_SIZE];
#endif
#endif

typedef struct {
    TCH_t * tch;
    bool configured;
    uint16_t value;

    // PWM parameters
    volatile timCCR_t *ccr;         // Shortcut for timer CCR register
    float pulseOffset;
    float pulseScale;

#ifdef USE_DSHOT
    // DSHOT parameters
    timerDMASafeType_t dmaBuffer[DSHOT_PORT_DMA_BUFFER_SIZE];
#ifdef USE_DSHOT_DMAR
    timerDMASafeType_t *dmaBurstBuffer;
#endif
#ifdef USE_DSHOT_BIDIR
    bool telemetryInputActive;
#endif
#endif
} pwmOutputPort_t;

typedef struct {
    pwmOutputPort_t *   pwmPort;        // May be NULL if motor doesn't use the PWM port
    uint16_t            value;          // Used to keep track of last motor value
    bool                requestTelemetry;
} pwmOutputMotor_t;

static DMA_RAM pwmOutputPort_t pwmOutputPorts[MAX_PWM_OUTPUTS];

static pwmOutputMotor_t        motors[MAX_MOTORS];
static motorPwmProtocolTypes_e initMotorProtocol;
static pwmWriteFuncPtr         motorWritePtr = NULL;    // Function to write value to motors

static pwmOutputPort_t *       servos[MAX_SERVOS];
static pwmWriteFuncPtr         servoWritePtr = NULL;    // Function to write value to motors

static pwmOutputPort_t  beeperPwmPort;
static pwmOutputPort_t *beeperPwm;
static uint16_t beeperFrequency = 0;

static uint8_t allocatedOutputPortCount = 0;

static bool pwmMotorsEnabled = true;

#ifdef USE_DSHOT
static timeUs_t digitalMotorUpdateIntervalUs = 0;
static timeUs_t digitalMotorLastUpdateUs;
static timeUs_t lastCommandSent = 0;
static timeUs_t commandPostDelay = 0;
#ifdef USE_DSHOT_BIDIR
static bool dshotTelemetryPending = false;
/* One stamp for the whole frame, taken in the main loop just before the frames start, so
 * the completion ISR has no clock to read. Turning a port back to output resets the
 * counter and period of its timer, which every other port on that timer shares, so no
 * port may switch back while a sibling could still be capturing: the reply window is
 * therefore waited out once, for all ports, before any of them is touched */
static timeUs_t dshotTelemetryFrameStampUs = 0;
static timeUs_t dshotTelemetryDeadtimeUs = 0;
// DEBUG_ESC: cycle counter at the frame start, debug[motor] = cycles until its capture is armed
static uint32_t dshotFrameStartCycles = 0;
#endif

static circularBuffer_t commandsCircularBuffer;
static uint8_t commandsBuff[DHSOT_COMMAND_QUEUE_SIZE];
static currentExecutingCommand_t currentExecutingCommand;

static uint16_t prepareDshotPacket(const uint16_t value, bool requestTelemetry);
uint32_t getDshotHz(motorPwmProtocolTypes_e pwmProtocolType);
#ifndef USE_DSHOT_DMAR
static void loadDmaBufferDshot(timerDMASafeType_t *dmaBuffer, uint16_t packet);
#else
static void loadDmaBufferDshotStride(timerDMASafeType_t *dmaBuffer, int stride, uint16_t packet);
#endif

#ifdef USE_DSHOT_DMAR
burstDmaTimer_t burstDmaTimers[MAX_DMA_TIMERS];
uint8_t burstDmaTimersCount = 0;
#endif

#ifdef USE_DSHOT_BIDIR
// GCR decode algorithm ported from Betaflight (GPLv3)
static uint16_t dshotDecodeTelemetryPacket(const uint32_t buffer[], uint32_t count);
static bool pwmDshotDecodeTelemetry(void);
static void pwmDshotSetDirectionOutput(pwmOutputPort_t *port);
static void pwmDshotSetDirectionInput(pwmOutputPort_t *port);
static void pwmDshotDmaIrqHandler(DMA_t descriptor);
// Set while bidir pins are plain GPIO, disconnected from their timer: parked low until
// the first frame, or held by the pull only during a flash write. The next frame
// reconnects them (dshotConnectOutputs()).
static bool dshotPinsDisconnected = false;
#endif
#endif

static void pwmOutConfigTimer(pwmOutputPort_t * p, TCH_t * tch, uint32_t hz, uint16_t period, uint16_t value)
{
    p->tch = tch;

    timerConfigBase(p->tch, period, hz);
    timerPWMConfigChannel(p->tch, value);
    timerPWMStart(p->tch);

    timerEnable(p->tch);

    p->ccr = timerCCR(p->tch);
    *p->ccr = 0;
}

static pwmOutputPort_t *pwmOutAllocatePort(void)
{
    if (allocatedOutputPortCount >= MAX_PWM_OUTPUTS) {
        LOG_ERROR(PWM, "Attempt to allocate PWM output beyond MAX_PWM_OUTPUT_PORTS");
        return NULL;
    }

    pwmOutputPort_t *p = &pwmOutputPorts[allocatedOutputPortCount++];

    p->tch = NULL;
    p->configured = false;

    return p;
}

static pwmOutputPort_t *pwmOutConfig(const timerHardware_t *timHw, resourceOwner_e owner, uint32_t hz, uint16_t period, uint16_t value, bool enableOutput)
{
    // Attempt to allocate TCH
    TCH_t * tch = timerGetTCH(timHw);
    if (tch == NULL) {
        return NULL;
    }

    // Allocate motor output port
    pwmOutputPort_t *p = pwmOutAllocatePort();
    if (p == NULL) {
        return NULL;
    }

    const IO_t io = IOGetByTag(timHw->tag);
    IOInit(io, owner, RESOURCE_OUTPUT, allocatedOutputPortCount);

    pwmOutConfigTimer(p, tch, hz, period, value);

    if (enableOutput) {
        IOConfigGPIOAF(io, IOCFG_AF_PP, timHw->alternateFunction);
    }
    else {
        // If PWM outputs are disabled - configure as GPIO and drive low
        IOConfigGPIO(io, IOCFG_OUT_OD);
        IOLo(io);
    }

    return p;
}

static void pwmWriteNull(uint8_t index, uint16_t value)
{
    (void)index;
    (void)value;
}

static void pwmWriteStandard(uint8_t index, uint16_t value)
{
    if (motors[index].pwmPort) {
        *(motors[index].pwmPort->ccr) = lrintf((value * motors[index].pwmPort->pulseScale) + motors[index].pwmPort->pulseOffset);
    }
}

void pwmWriteMotor(uint8_t index, uint16_t value)
{
    if (motorWritePtr && index < MAX_MOTORS && pwmMotorsEnabled) {
        motorWritePtr(index, value);
    }
}

void pwmShutdownPulsesForAllMotors(uint8_t motorCount)
{
    for (int index = 0; index < motorCount; index++) {
        // Set the compare register to 0, which stops the output pulsing if the timer overflows
        if (motors[index].pwmPort) {
            *(motors[index].pwmPort->ccr) = 0;
        }
    }
}

void pwmDisableMotors(void)
{
    pwmMotorsEnabled = false;
}

void pwmEnableMotors(void)
{
    pwmMotorsEnabled = true;
}

#ifdef USE_DSHOT
/*
 * Wait (bounded) until the keep-alive stream of this port is sending its idle padding.
 * CCR holds the slot last written by DMA: a bit length while a data bit is in flight,
 * 0 in the padding. Waiting for the data -> padding transition leaves a full
 * DSHOT_KEEPALIVE_GAP_US before the DMA wraps to the next frame, so the stream can be
 * stopped with the last frame complete and the line low.
 */
static void dshotWaitForKeepalivePadding(const pwmOutputPort_t *port)
{
    const timeUs_t start = micros();
    while (*port->ccr == 0 && (micros() - start) < DSHOT_KEEPALIVE_WAIT_TIMEOUT_US);
    while (*port->ccr != 0 && (micros() - start) < DSHOT_KEEPALIVE_WAIT_TIMEOUT_US);
}
#endif

void pwmSetMotorDMACircular(bool circular)
{
#ifdef USE_DSHOT
    if (!isMotorProtocolDshot()) {
        return;
    }

    int motorCount = getMotorCount();
    const uint32_t dshotHz = getDshotHz(initMotorProtocol);
    const uint32_t keepaliveSlots = DSHOT_KEEPALIVE_SLOTS(dshotHz);
    // A frame started by pwmCompleteMotorUpdate() may still be in flight for this long
    const uint32_t frameUs = DSHOT_DMA_BUFFER_SIZE * DSHOT_MOTOR_BITLENGTH * 1000000UL / dshotHz;

#ifdef USE_DSHOT_BIDIR
    // No keep-alive frames for bidir: the ESC sees a frame gap for the flash write, as in
    // Betaflight, with the line held at its idle level only by the pull.
    if (useDshotTelemetry) {
        if (circular && !dshotPinsDisconnected) {
            // Complete the DSHOT exchange before the write: let a frame in flight finish,
            // with the deadtime as margin for its completion IRQ, then wait out and decode
            // the reply, which turns every port back to output
            delayMicroseconds(frameUs + DSHOT_TELEMETRY_DEADTIME_US);
            while (!pwmDshotDecodeTelemetry()) { }

            // Disconnect the pins from the timers so nothing drives the line during the
            // write; the next frame after it reconnects them
            for (int i = 0; i < motorCount; i++) {
                const pwmOutputPort_t *port = motors[i].pwmPort;
                if (port && port->configured) {
                    const bool baseInverted = port->tch->timHw->output & TIMER_OUTPUT_INVERTED;
                    IOConfigGPIO(IOGetByTag(port->tch->timHw->tag), baseInverted ? IOCFG_IPD : IOCFG_IPU);
                }
            }
            dshotPinsDisconnected = true;
        }
        return;
    }
#endif

    if (circular) {
        // A frame started by pwmCompleteMotorUpdate() may still be in flight: let it finish
        // and keep the line low for one full gap before the keep-alive stream starts
        delayMicroseconds(frameUs + DSHOT_KEEPALIVE_GAP_US);

        // Load a zero-throttle packet into the shared keep-alive buffer. The padding slots
        // must be zero (line low between frames); DMA_RAM is NOLOAD and not cleared at
        // startup, so clear it explicitly.
        uint16_t packet = prepareDshotPacket(0, false);
        ZERO_FARRAY(dshotKeepaliveBuffer);
#ifdef USE_DSHOT_DMAR
        for (int i = 0; i < motorCount; i++) {
            if (motors[i].pwmPort && motors[i].pwmPort->configured) {
                loadDmaBufferDshotStride(&dshotKeepaliveBuffer[motors[i].pwmPort->tch->timHw->channelIndex], 4, packet);
            }
        }
#else
        loadDmaBufferDshot(dshotKeepaliveBuffer, packet);
#endif
    }

#ifdef USE_DSHOT_DMAR
    // Burst DMA: one DMA stream per timer, shared across channels
    for (int i = 0; i < burstDmaTimersCount; i++) {
        burstDmaTimer_t *burstDmaTimer = &burstDmaTimers[i];
        // Find the first motor using this timer to get the TCH for DMA state
        for (int m = 0; m < motorCount; m++) {
            if (motors[m].pwmPort && motors[m].pwmPort->configured && motors[m].pwmPort->tch
                && motors[m].pwmPort->tch->timHw->tim == burstDmaTimer->timer) {
                if (circular) {
                    impl_pwmBurstDMASetCircular(burstDmaTimer, motors[m].pwmPort->tch, true, dshotKeepaliveBuffer, keepaliveSlots * 4);
                } else {
                    // Atomic so no ISR can delay the stop past the padding into the next frame
                    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
                        dshotWaitForKeepalivePadding(motors[m].pwmPort);
                        impl_pwmBurstDMASetCircular(burstDmaTimer, motors[m].pwmPort->tch, false, burstDmaTimer->dmaBurstBuffer, DSHOT_DMA_BUFFER_SIZE * 4);
                    }
                }
                break;
            }
        }
    }
#else
    // Per-channel DMA: one DMA stream per motor
    for (int i = 0; i < motorCount; i++) {
        if (motors[i].pwmPort && motors[i].pwmPort->configured && motors[i].pwmPort->tch) {
            if (circular) {
                impl_timerPWMSetDMACircular(motors[i].pwmPort->tch, true, dshotKeepaliveBuffer, keepaliveSlots);
            } else {
                // Atomic so no ISR can delay the stop past the padding into the next frame
                ATOMIC_BLOCK(NVIC_PRIO_MAX) {
                    dshotWaitForKeepalivePadding(motors[i].pwmPort);
                    impl_timerPWMSetDMACircular(motors[i].pwmPort->tch, false, motors[i].pwmPort->dmaBuffer, DSHOT_DMA_BUFFER_SIZE);
                }
            }
        }
    }
#endif

    if (!circular) {
        // The streams were stopped while sending padding, so CCR is already 0 and the lines
        // stay low until pwmCompleteMotorUpdate() sends the next frame. Enforce it in case a
        // wait above timed out; the timer would otherwise keep repeating the last bit.
        for (int i = 0; i < motorCount; i++) {
            if (motors[i].pwmPort && motors[i].pwmPort->configured) {
                *motors[i].pwmPort->ccr = 0;
            }
        }
    }
#else
    UNUSED(circular);
#endif
}

bool isMotorBrushed(uint16_t motorPwmRateHz)
{
    return (motorPwmRateHz > 500);
}

static pwmOutputPort_t * motorConfigPwm(const timerHardware_t *timerHardware, float sMin, float sLen, uint32_t motorPwmRateHz, bool enableOutput)
{
    const uint32_t baseClockHz = timerGetBaseClockHW(timerHardware);
    const uint32_t prescaler = ((baseClockHz / motorPwmRateHz) + 0xffff) / 0x10000; /* rounding up */
    const uint32_t timerHz = baseClockHz / prescaler;
    const uint32_t period = timerHz / motorPwmRateHz;

    pwmOutputPort_t * port = pwmOutConfig(timerHardware, OWNER_MOTOR, timerHz, period, 0, enableOutput);

    if (port) {
        port->pulseScale = ((sLen == 0) ? period : (sLen * timerHz)) / 1000.0f;
        port->pulseOffset = (sMin * timerHz) - (port->pulseScale * 1000);
        port->configured = true;
    }

    return port;
}

#ifdef USE_DSHOT
uint32_t getDshotHz(motorPwmProtocolTypes_e pwmProtocolType)
{
    switch (pwmProtocolType) {
        case(PWM_TYPE_DSHOT600):
            return MOTOR_DSHOT600_HZ;
        case(PWM_TYPE_DSHOT300):
            return MOTOR_DSHOT300_HZ;
        default:
        case(PWM_TYPE_DSHOT150):
            return MOTOR_DSHOT150_HZ;
    }
}

#ifdef USE_DSHOT_DMAR
static uint8_t getBurstDmaTimerIndex(TIM_TypeDef *timer)
{
    for (int i = 0; i < burstDmaTimersCount; i++) {
        if (burstDmaTimers[i].timer == timer) {
            return i;
        }
    }
    burstDmaTimers[burstDmaTimersCount++].timer = timer;
    return burstDmaTimersCount - 1;
}
#endif

#ifdef USE_DSHOT_BIDIR
// The CC1..CC4 DMA request bits sit next to each other in DIER on every supported MCU
static uint32_t dshotDmaSource(const pwmOutputPort_t *port)
{
#if defined(USE_HAL_DRIVER) || !defined(AT32F43x)
    STATIC_ASSERT(TIM_DMA_CC2 == TIM_DMA_CC1 << 1 && TIM_DMA_CC4 == TIM_DMA_CC1 << 3, tim_dma_cc_bits_adjacent);
    return TIM_DMA_CC1 << port->tch->timHw->channelIndex;
#else
    STATIC_ASSERT(TMR_C2_DMA_REQUEST == TMR_C1_DMA_REQUEST << 1 && TMR_C4_DMA_REQUEST == TMR_C1_DMA_REQUEST << 3, tmr_dma_request_bits_adjacent);
    return TMR_C1_DMA_REQUEST << port->tch->timHw->channelIndex;
#endif
}

#if defined(USE_HAL_DRIVER)
static uint32_t dshotDmaStream(const pwmOutputPort_t *port)
{
    STATIC_ASSERT(LL_DMA_STREAM_0 == 0 && LL_DMA_STREAM_7 == 7, ll_dma_stream_is_index);
    return DMATAG_GET_STREAM(port->tch->timHw->dmaTag);
}

// The LL driver only exposes per-channel LL_TIM_{En,Dis}ableDMAReq_CC1..CC4;
// there is no runtime-channel variant, so set/clear the DIER bit directly.
static inline void LL_TIM_EnableDMAReq_CCx(TIM_TypeDef *TIMx, uint16_t dmaSources)
{
    SET_BIT(TIMx->DIER, dmaSources & (TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3 | TIM_DMA_CC4));
}

static inline void LL_TIM_DisableDMAReq_CCx(TIM_TypeDef *TIMx, uint16_t dmaSources)
{
    CLEAR_BIT(TIMx->DIER, dmaSources & (TIM_DMA_CC1 | TIM_DMA_CC2 | TIM_DMA_CC3 | TIM_DMA_CC4));
}
#endif

#if defined(USE_HAL_DRIVER)
// Register slices of this port's timer channel for the direction switch: the channel's
// byte in CCMR1/CCMR2 and its nibble in CCER. Derived on the fly, the shifts cost a few cycles.
typedef struct {
    volatile uint32_t *ccmr;
    uint32_t ccmrShift;
    uint32_t ccmrMask;      // the channel's byte plus its OCxM[3] bit
    uint32_t ccerShift;
    uint32_t ccerMask;
} dshotChannelRegs_t;

static dshotChannelRegs_t dshotChannelRegs(const pwmOutputPort_t *port)
{
    const uint32_t channelIndex = port->tch->timHw->channelIndex;
    TIM_TypeDef *tim = port->tch->timHw->tim;
    dshotChannelRegs_t regs;
    regs.ccmr = (channelIndex < 2) ? &tim->CCMR1 : &tim->CCMR2;
    regs.ccmrShift = 8 * (channelIndex & 1);
    regs.ccmrMask = (0xFF | TIM_CCMR1_OC1M_3) << regs.ccmrShift;
    regs.ccerShift = 4 * channelIndex;
    regs.ccerMask = 0xF << regs.ccerShift;
    return regs;
}

// CCMRx channel byte for capture: CCxS=01 (TI direct), ICxPSC=0, ICxF=0010 (fDTS, N=4)
#define DSHOT_CCMR_INPUT    (TIM_CCMR1_CC1S_0 | TIM_CCMR1_IC1F_1)
// CCMRx channel byte for output: CCxS=00, OCxM=110 (PWM1); preload is added once CCR is zeroed
#define DSHOT_CCMR_OUTPUT   (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
// CCER channel nibble for capture: both edges, channel on
#define DSHOT_CCER_INPUT    (TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC1NP)
#else
// The channel identifiers are evenly spaced, so channelIndex maps onto them linearly
static uint32_t dshotTimChannel(const pwmOutputPort_t *port)
{
#if defined(AT32F43x)
    STATIC_ASSERT(TMR_SELECT_CHANNEL_4 == TMR_SELECT_CHANNEL_1 + 3 * (TMR_SELECT_CHANNEL_2 - TMR_SELECT_CHANNEL_1), tmr_channel_ids_linear);
    return TMR_SELECT_CHANNEL_1 + port->tch->timHw->channelIndex * (TMR_SELECT_CHANNEL_2 - TMR_SELECT_CHANNEL_1);
#else
    STATIC_ASSERT(TIM_Channel_4 == TIM_Channel_1 + 3 * (TIM_Channel_2 - TIM_Channel_1), tim_channel_ids_linear);
    return TIM_Channel_1 + port->tch->timHw->channelIndex * (TIM_Channel_2 - TIM_Channel_1);
#endif
}
#endif

static uint16_t dshotDecodeTelemetryPacket(const uint32_t buffer[], uint32_t count)
{
    uint32_t value = 0;
    uint32_t oldValue = buffer[0];
    int bits = 0;

    for (uint32_t i = 1; i <= count; i++) {
        int len;
        if (i < count) {
            const int diff = buffer[i] - oldValue;
            if (bits >= 21) {
                break;
            }
            len = (diff + 8) / 16;
            if (len < 1 || len > 21) {
                // Edges closer than half a GCR bit or further apart than a whole frame
                return DSHOT_TELEMETRY_INVALID;
            }
        } else {
            len = 21 - bits;
            if (len < 1) {
                // All bits accounted for, no trailing run left to infer
                break;
            }
        }

        value <<= len;
        value |= 1 << (len - 1);
        if (i < count) {
            oldValue = buffer[i];
        }
        bits += len;
    }

    if (bits != 21) {
        return DSHOT_TELEMETRY_INVALID;
    }

    static const uint32_t decode[32] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 10, 11, 0, 13, 14, 15,
        0, 0, 2, 3, 0, 5, 6, 7, 0, 0, 8, 1, 0, 4, 12, 0
    };

    uint32_t decodedValue = decode[value & 0x1f];
    decodedValue |= decode[(value >> 5) & 0x1f] << 4;
    decodedValue |= decode[(value >> 10) & 0x1f] << 8;
    decodedValue |= decode[(value >> 15) & 0x1f] << 12;

    uint32_t csum = decodedValue;
    csum ^= csum >> 8;
    csum ^= csum >> 4;

    if ((csum & 0xf) != 0xf) {
        return DSHOT_TELEMETRY_INVALID;
    }

    return decodedValue >> 4;
}

static void pwmDshotSetDirectionOutput(pwmOutputPort_t *port)
{
    // Bidirectional DSHOT is carried on inverted (idle-high) signalling - flip the
    // channel's normal polarity whenever telemetry is active, matching Betaflight.
    const bool inverted = ((port->tch->timHw->output & TIMER_OUTPUT_INVERTED) != 0) ^ useDshotTelemetry;

#if defined(USE_HAL_DRIVER)
    // Telemetry input capture widens ARR to 0xffff (see pwmDshotSetDirectionInput);
    // restore the DSHOT bit period now, bypassing the ARR preload shadow so it
    // takes effect immediately instead of after the next update event.
    TIM_TypeDef *tim = port->tch->timHw->tim;
    CLEAR_BIT(tim->CR1, TIM_CR1_ARPE);
    tim->ARR = DSHOT_MOTOR_BITLENGTH - 1;
    SET_BIT(tim->CR1, TIM_CR1_ARPE);
    tim->CNT = 0;

    // Channel back to PWM1 output. CCxS is only writable with the channel off, and CCR is
    // zeroed before OCxPE goes back on so the stale capture value can't give one bit period
    // of output before the first update event. Idle state (CR2 OISx) is untouched.
    const dshotChannelRegs_t regs = dshotChannelRegs(port);
    CLEAR_BIT(tim->CCER, regs.ccerMask);
    MODIFY_REG(*regs.ccmr, regs.ccmrMask, DSHOT_CCMR_OUTPUT << regs.ccmrShift);
    *port->ccr = 0;
    SET_BIT(*regs.ccmr, TIM_CCMR1_OC1PE << regs.ccmrShift);
    uint32_t ccer;
    if (port->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        ccer = TIM_CCER_CC1NE | (inverted ? TIM_CCER_CC1NP : 0);
    } else {
        ccer = TIM_CCER_CC1E | (inverted ? TIM_CCER_CC1P : 0);
    }
    MODIFY_REG(tim->CCER, regs.ccerMask, ccer << regs.ccerShift);

    // Stream back to memory-to-peripheral; timerPWMPrepareDMA() sets count and addresses and
    // enables it for the next frame. The capture may still be draining (the decode loop only
    // cleared EN), so wait for EN to read 0 before touching CR, then clear all five event
    // flags: EN=1 is ignored while any of them is set (RM0433).
    DMA_Stream_TypeDef *stream = port->tch->dma->ref;
    for (uint32_t timeout = 10000; timeout && (stream->CR & DMA_SxCR_EN); timeout--) { }
    DMA_CLEAR_FLAG(port->tch->dma, DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF | DMA_IT_DMEIF | DMA_IT_FEIF);
    stream->CR = (stream->CR & ~(DMA_SxCR_DIR | DMA_SxCR_EN)) | DMA_SxCR_DIR_0;
    port->telemetryInputActive = false;
#elif defined(AT32F43x)
    // Telemetry input capture widens the period to 0xffff (see pwmDshotSetDirectionInput);
    // restore the DSHOT bit period now, bypassing the period buffer so it takes effect
    // immediately, and reset the counter so a stale count above the new period doesn't
    // have to run all the way round (TMR2/TMR5 are 32-bit) before the first compare.
    tmr_period_buffer_enable(port->tch->timHw->tim, FALSE);
    tmr_period_value_set(port->tch->timHw->tim, DSHOT_MOTOR_BITLENGTH - 1);
    tmr_period_buffer_enable(port->tch->timHw->tim, TRUE);
    tmr_counter_value_set(port->tch->timHw->tim, 0);

    tmr_output_config_type output = {0};
    tmr_output_default_para_init(&output);
    output.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    if (port->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        output.oc_output_state = FALSE;
        output.occ_output_state = TRUE;
        output.occ_polarity = inverted ? TMR_OUTPUT_ACTIVE_LOW : TMR_OUTPUT_ACTIVE_HIGH;
        output.occ_idle_state = FALSE;
    } else {
        output.oc_output_state = TRUE;
        output.occ_output_state = FALSE;
        output.oc_polarity = inverted ? TMR_OUTPUT_ACTIVE_LOW : TMR_OUTPUT_ACTIVE_HIGH;
        output.oc_idle_state = TRUE;
    }
    tmr_output_channel_config(port->tch->timHw->tim, dshotTimChannel(port), &output);
    tmr_channel_value_set(port->tch->timHw->tim, dshotTimChannel(port), 0);
    tmr_output_channel_buffer_enable(port->tch->timHw->tim, dshotTimChannel(port), TRUE);
    dma_channel_enable(port->tch->dma->ref, FALSE);
    port->tch->dma->ref->maddr = (uint32_t)port->dmaBuffer;
    port->tch->dma->ref->dtcnt = DSHOT_DMA_BUFFER_SIZE;
    port->telemetryInputActive = false;
#else
    TIM_ARRPreloadConfig(port->tch->timHw->tim, DISABLE);
    TIM_SetAutoreload(port->tch->timHw->tim, DSHOT_MOTOR_BITLENGTH - 1);
    TIM_ARRPreloadConfig(port->tch->timHw->tim, ENABLE);
    TIM_SetCounter(port->tch->timHw->tim, 0);
    TIM_OCInitTypeDef init;
    TIM_OCStructInit(&init);
    init.TIM_OCMode = TIM_OCMode_PWM1;
    init.TIM_Pulse = 0;
    if (port->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        init.TIM_OutputState = TIM_OutputState_Disable;
        init.TIM_OutputNState = TIM_OutputNState_Enable;
        init.TIM_OCNPolarity = inverted ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
        init.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    } else {
        init.TIM_OutputState = TIM_OutputState_Enable;
        init.TIM_OutputNState = TIM_OutputNState_Disable;
        init.TIM_OCPolarity = inverted ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
        init.TIM_OCIdleState = TIM_OCIdleState_Set;
    }
    switch (port->tch->timHw->channelIndex) {
    case 0: TIM_OC1Init(port->tch->timHw->tim, &init); TIM_OC1PreloadConfig(port->tch->timHw->tim, TIM_OCPreload_Enable); break;
    case 1: TIM_OC2Init(port->tch->timHw->tim, &init); TIM_OC2PreloadConfig(port->tch->timHw->tim, TIM_OCPreload_Enable); break;
    case 2: TIM_OC3Init(port->tch->timHw->tim, &init); TIM_OC3PreloadConfig(port->tch->timHw->tim, TIM_OCPreload_Enable); break;
    default: TIM_OC4Init(port->tch->timHw->tim, &init); TIM_OC4PreloadConfig(port->tch->timHw->tim, TIM_OCPreload_Enable); break;
    }
    DMA_Cmd(port->tch->dma->ref, DISABLE);
    port->tch->dma->ref->CR = (port->tch->dma->ref->CR & ~(DMA_DIR_MemoryToPeripheral | DMA_DIR_MemoryToMemory)) | DMA_DIR_MemoryToPeripheral;
    port->tch->dma->ref->M0AR = (uint32_t)port->dmaBuffer;
    DMA_SetCurrDataCounter(port->tch->dma->ref, DSHOT_DMA_BUFFER_SIZE);
    port->telemetryInputActive = false;
#endif
}

FAST_CODE static void pwmDshotSetDirectionInput(pwmOutputPort_t *port)
{
#if defined(USE_HAL_DRIVER)
    // Runs in the DMA completion ISR for every motor in turn, so it only writes registers:
    // no HAL/LL init calls, and the stream is reconfigured in place (hardware clears its EN
    // bit at end of transfer, the wait below only covers the flag/EN ordering).
    // pwmOutputPorts lives in non-cacheable DMA_RAM, so read tch once.
    TCH_t *tch = port->tch;
    TIM_TypeDef *tim = tch->timHw->tim;
    const dshotChannelRegs_t regs = dshotChannelRegs(port);

    // Widen ARR so the free-running counter doesn't wrap every DSHOT bit period
    // (20 ticks) while timing GCR edges, which span ~21 bits per telemetry frame.
    // ARPE is set, so this lands at the next update event, within one bit period.
    tim->ARR = 0xffff;

#if defined(STM32H7)
    // H7 errata workaround (matches Betaflight): reconfiguring the channel from output
    // compare to input capture can glitch the pin for a cycle while CCMR/CCER are mid-update.
    // Drive it as a plain GPIO output at the preset ODR level meanwhile. Only MODER changes;
    // pull, speed and AF stay as dshotConnectOutputs() set them.
    const IO_t io = IOGetByTag(tch->timHw->tag);
    GPIO_TypeDef *gpio = IO_GPIO(io);
    const uint32_t moderShift = 2 * IO_GPIOPinIdx(io);
    MODIFY_REG(gpio->MODER, GPIO_MODER_MODE0_Msk << moderShift, GPIO_MODER_MODE0_0 << moderShift);
#endif

    // Channel to input capture, both edges, filter N=4. CCxS is only writable with the channel off.
    CLEAR_BIT(tim->CCER, regs.ccerMask);
    MODIFY_REG(*regs.ccmr, regs.ccmrMask, DSHOT_CCMR_INPUT << regs.ccmrShift);
    MODIFY_REG(tim->CCER, regs.ccerMask, DSHOT_CCER_INPUT << regs.ccerShift);

#if defined(STM32H7)
    MODIFY_REG(gpio->MODER, GPIO_MODER_MODE0_Msk << moderShift, GPIO_MODER_MODE0_1 << moderShift);
#endif

    // Stream to peripheral-to-memory: addresses, FIFO and request stay from the output frame.
    // Clear all five event flags first, EN=1 is ignored while any of them is set (RM0433).
    DMA_Stream_TypeDef *stream = tch->dma->ref;
    for (uint32_t timeout = 1000; timeout && (stream->CR & DMA_SxCR_EN); timeout--) { }
    DMA_CLEAR_FLAG(tch->dma, DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF | DMA_IT_DMEIF | DMA_IT_FEIF);
    const uint32_t cr = (stream->CR & ~(DMA_SxCR_DIR | DMA_SxCR_EN)) | DMA_SxCR_TCIE;
    stream->CR = cr;
    stream->NDTR = GCR_TELEMETRY_INPUT_LEN;
    stream->CR = cr | DMA_SxCR_EN;
    LL_TIM_EnableDMAReq_CCx(tim, TIM_DMA_CC1 << tch->timHw->channelIndex);
#elif defined(AT32F43x)
    // Widen the period so the free-running counter doesn't wrap every DSHOT bit period
    // (20 ticks) while timing GCR edges, which span ~21 bits per telemetry frame.
    tmr_period_buffer_enable(port->tch->timHw->tim, TRUE);
    tmr_period_value_set(port->tch->timHw->tim, 0xffff);

    tmr_input_config_type input;
    tmr_input_default_para_init(&input);
    input.input_channel_select = dshotTimChannel(port);
    input.input_mapped_select = TMR_CC_CHANNEL_MAPPED_DIRECT;
    input.input_filter_value = 2;
    input.input_polarity_select = TMR_INPUT_BOTH_EDGE;
    tmr_input_channel_init(port->tch->timHw->tim, &input, TMR_CHANNEL_INPUT_DIV_1);
    port->tch->dma->ref->paddr = (uint32_t)port->ccr;
    port->tch->dma->ref->maddr = (uint32_t)port->dmaBuffer;
    port->tch->dma->ref->dtcnt = GCR_TELEMETRY_INPUT_LEN;
    dma_channel_enable(port->tch->dma->ref, TRUE);
    tmr_dma_request_enable(port->tch->timHw->tim, dshotDmaSource(port), TRUE);
#else
    TIM_ARRPreloadConfig(port->tch->timHw->tim, ENABLE);
    TIM_SetAutoreload(port->tch->timHw->tim, 0xffff);
    TIM_ICInitTypeDef init;
    TIM_ICStructInit(&init);
    init.TIM_Channel = dshotTimChannel(port);
    init.TIM_ICSelection = TIM_ICSelection_DirectTI;
    init.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    init.TIM_ICFilter = 2;
    init.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    TIM_ICInit(port->tch->timHw->tim, &init);
    DMA_Cmd(port->tch->dma->ref, DISABLE);
    port->tch->dma->ref->CR &= ~(DMA_DIR_MemoryToPeripheral | DMA_DIR_MemoryToMemory); // P→M
    port->tch->dma->ref->PAR = (uint32_t)port->ccr;
    port->tch->dma->ref->M0AR = (uint32_t)port->dmaBuffer;
    DMA_SetCurrDataCounter(port->tch->dma->ref, GCR_TELEMETRY_INPUT_LEN);
    // Clear stale DMA flags (TCIF/HTIF from completed output DMA) before
    // enabling input capture, otherwise a spurious TC fires immediately and
    // kills the capture DMA before any edges can be recorded.
    DMA_CLEAR_FLAG(port->tch->dma, DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF);
    DMA_Cmd(port->tch->dma->ref, ENABLE);
    TIM_DMACmd(port->tch->timHw->tim, dshotDmaSource(port), ENABLE);
#endif
    port->telemetryInputActive = true;
}

// Connect bidir DSHOT pins to their timer AF (idle-high). Called on the first
// real motor update so the line only goes high microseconds before frames start,
// keeping it low through the ESC's boot-time bootloader-entry window.
static void dshotConnectOutputs(void)
{
    for (int index = 0; index < getMotorCount(); index++) {
        pwmOutputPort_t *port = motors[index].pwmPort;
        if (port && port->configured) {
            const timerHardware_t *timHw = port->tch->timHw;
            const IO_t io = IOGetByTag(timHw->tag);
            // The pull must match this channel's *un-flipped* polarity
            // (pwmDshotSetDirectionOutput flips OCPolarity for bidir, but the GCR
            // listen phase still idles toward the same physical level either way).
            const bool baseInverted = timHw->output & TIMER_OUTPUT_INVERTED;
#if defined(STM32H7)
            // Preset ODR to the bidir idle level once - the H7 OC->IC erratum workaround
            // in pwmDshotSetDirectionInput() disconnects the pin from the timer and relies
            // on this cached ODR bit (matches Betaflight) instead of redriving the level.
            baseInverted ? IOLo(io) : IOHi(io);
#endif
            IOConfigGPIOAF(io, baseInverted ? IOCFG_AF_PP_PD : IOCFG_AF_PP_UP, timHw->alternateFunction);
        }
    }
}

// Takes over the stream's completion interrupt from the timer driver on bidir ports: once
// the frame is out the port is turned round to capture the ESC reply, and
// pwmDshotDecodeTelemetry() turns it back before the next frame. Runs once per motor per
// frame, back to back, so it lives in ITCM where available.
FAST_CODE static void pwmDshotDmaIrqHandler(DMA_t descriptor)
{
    if (!DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        return;
    }

    // userParam is the motor index (see motorConfigDshot())
    pwmOutputPort_t *port = motors[descriptor->userParam].pwmPort;

#if defined(USE_HAL_DRIVER)
    // The stream disabled itself at end of transfer (normal mode). The DMA request must go
    // too: CCR is 0 after the frame, so compare matches keep a request pending and the stream
    // would serve it as a bogus first capture the moment it is re-enabled for input.
    LL_TIM_DisableDMAReq_CCx(port->tch->timHw->tim, dshotDmaSource(port));
#elif defined(AT32F43x)
    dma_channel_enable(port->tch->dma->ref, FALSE);
    tmr_dma_request_enable(port->tch->timHw->tim, dshotDmaSource(port), FALSE);
#else
    DMA_Cmd(port->tch->dma->ref, DISABLE);
    TIM_DMACmd(port->tch->timHw->tim, dshotDmaSource(port), DISABLE);
#endif

    // Same bookkeeping as the timer driver's handler (timerPWMDMAInProgress())
    if (port->tch->dmaState == TCH_DMA_ACTIVE) {
        port->tch->dmaState = TCH_DMA_IDLE;
    }

    if (!port->telemetryInputActive) {
        pwmDshotSetDirectionInput(port);
        dshotTelemetryPending = true;

        // DEBUG_ESC: debug[motor] = cycles from the frame start to this capture being armed
        // (H7: /480 = us, F7: /216)
        if (debugMode == DEBUG_ESC && descriptor->userParam < DEBUG32_VALUE_COUNT) {
            debug[descriptor->userParam] = ticks() - dshotFrameStartCycles;
        }
    }

    DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
}

// DMA has already captured the telemetry waveform before this runs. Keep the
// decode path out of the fast motor-update caller on constrained F7 targets.
static bool NOINLINE pwmDshotDecodeTelemetry(void)
{
    if (!dshotTelemetryPending) {
        return true;
    }

    // Wait out the reply window of the frame before switching any port back (see
    // dshotTelemetryFrameStampUs)
    if ((micros() - dshotTelemetryFrameStampUs) < dshotTelemetryDeadtimeUs) {
        return false;
    }

    // Cleared before the loop: a completion IRQ landing while the loop runs re-arms it, so a
    // port the loop has already passed is picked up by the next call instead of being left
    // in input for the following frame
    dshotTelemetryPending = false;

    for (int motorIndex = 0; motorIndex < getMotorCount(); motorIndex++) {
        pwmOutputPort_t *port = motors[motorIndex].pwmPort;
        if (!port || !port->configured || !port->telemetryInputActive) {
            continue;
        }

        uint32_t edges = 0;
#if defined(USE_HAL_DRIVER)
        const uint32_t streamLL = dshotDmaStream(port);
        edges = GCR_TELEMETRY_INPUT_LEN - LL_DMA_GetDataLength(port->tch->dma->dma, streamLL);
        LL_TIM_DisableDMAReq_CCx(port->tch->timHw->tim, dshotDmaSource(port));
        LL_DMA_DisableStream(port->tch->dma->dma, streamLL);
#elif defined(AT32F43x)
        edges = GCR_TELEMETRY_INPUT_LEN - dma_data_number_get(port->tch->dma->ref);
        tmr_dma_request_enable(port->tch->timHw->tim, dshotDmaSource(port), FALSE);
        dma_channel_enable(port->tch->dma->ref, FALSE);
#else
        edges = GCR_TELEMETRY_INPUT_LEN - DMA_GetCurrDataCounter(port->tch->dma->ref);
        TIM_DMACmd(port->tch->timHw->tim, dshotDmaSource(port), DISABLE);
        DMA_Cmd(port->tch->dma->ref, DISABLE);
#endif

        // Too few edges is no reply at all
        uint16_t rawValue = DSHOT_TELEMETRY_NOEDGE;
        if (edges > MIN_GCR_EDGES) {
            // No cache maintenance: pwmOutputPorts is DMA_RAM, which the H7 MPU maps non-cacheable
            rawValue = dshotDecodeTelemetryPacket((const uint32_t *)port->dmaBuffer, edges);
        }

        const uint16_t processed = dshotProcessPacket(rawValue, motorIndex);
        if (processed != DSHOT_TELEMETRY_INVALID && processed != DSHOT_TELEMETRY_NOEDGE) {
#ifdef USE_ESC_SENSOR
            // Nothing to publish before the first eRPM value (an EDT frame may come first)
            escSensorData_t data;
            if (getDshotEscSensorData(&data, motorIndex)) {
                escSensorSetDshotData(motorIndex, data.rpm, data.temperature, data.voltage, data.current);
            }
#endif
        }

        pwmDshotSetDirectionOutput(port);
    }

    // Re-armed during the loop: some port is in input again, hold the frame until it is back
    return !dshotTelemetryPending;
}
#endif // USE_DSHOT_BIDIR

static pwmOutputPort_t * motorConfigDshot(const timerHardware_t * timerHardware, uint32_t dshotHz, bool enableOutput, uint8_t motorIndex)
{
    UNUSED(motorIndex);
    // Try allocating new port
    pwmOutputPort_t * port = pwmOutConfig(timerHardware, OWNER_MOTOR, dshotHz, DSHOT_MOTOR_BITLENGTH, 0, enableOutput);

    if (!port) {
        return NULL;
    }

#ifdef USE_DSHOT_BIDIR
    if (enableOutput && useDshotTelemetry) {
        // Bidir signalling idles high, but holding the line high with no edges
        // during ESC boot triggers the BLHeli/Bluejay bootloader-entry check
        // (~150ms of continuous high right after the ESC's startup melody). The
        // first INAV frame goes out seconds after this pin is configured, so the
        // ESC would enter its bootloader, time out and reset - replaying the
        // startup melody. Park the pin so the ESC sees low, like normal DSHOT
        // idle, and defer the AF connect until the first frame goes out
        // (dshotConnectOutputs()). On TIMER_OUTPUT_INVERTED hardware the pin
        // level is re-inverted downstream, so park high there.
        const IO_t io = IOGetByTag(timerHardware->tag);
        IOConfigGPIO(io, IOCFG_OUT_PP);
        (timerHardware->output & TIMER_OUTPUT_INVERTED) ? IOHi(io) : IOLo(io);
        dshotPinsDisconnected = true;
    }
#endif

    // Configure timer DMA
#ifdef USE_DSHOT_DMAR
    uint8_t burstDmaTimerIndex = getBurstDmaTimerIndex(timerHardware->tim);
    if (burstDmaTimerIndex >= MAX_DMA_TIMERS) {
        return NULL;
    }

    port->dmaBurstBuffer = &dmaBurstBuffer[burstDmaTimerIndex][0];
    burstDmaTimer_t *burstDmaTimer = &burstDmaTimers[burstDmaTimerIndex];
    burstDmaTimer->dmaBurstBuffer = port->dmaBurstBuffer;

    if (timerPWMConfigDMABurst(burstDmaTimer, port->tch, port->dmaBurstBuffer, sizeof(port->dmaBurstBuffer[0]), DSHOT_DMA_BUFFER_SIZE)) {
        port->configured = true;
    }
#else
    if (timerPWMConfigChannelDMA(port->tch, port->dmaBuffer, sizeof(port->dmaBuffer[0]), DSHOT_DMA_BUFFER_SIZE)) {
        // Only mark as DSHOT channel if DMA was set successfully
        ZERO_FARRAY(port->dmaBuffer);
        port->configured = true;
#ifdef USE_DSHOT_BIDIR
        if (useDshotTelemetry) {
            // Inverted signalling and per-frame direction switching; the port takes the
            // stream's completion interrupt over from the timer driver
            dshotTelemetryDeadtimeUs = DSHOT_TELEMETRY_DEADTIME_US + 1000000 * (DSHOT_TELEMETRY_WINDOW_SLOTS * DSHOT_MOTOR_BITLENGTH) / dshotHz;
#if defined(USE_HAL_DRIVER)
            // FIFO threshold 1/4 for both directions (the timer driver set FULL), so captured
            // edges reach memory one word at a time and the decode never finds a partial reply
            // still sitting in the FIFO. Set once here; the direction switch leaves FCR alone.
            port->tch->dma->ref->FCR = DMA_SxFCR_DMDIS;
#endif
            pwmDshotSetDirectionOutput(port);
            // The handler gets the motor index: it reaches the port through motors[] and uses
            // the index straight as the debug slot
            dmaSetHandler(port->tch->dma, pwmDshotDmaIrqHandler, NVIC_PRIO_TIMER_DMA, motorIndex);
        }
#endif
    }
#endif

    return port;
}

#ifdef USE_DSHOT_DMAR
static void loadDmaBufferDshotStride(timerDMASafeType_t *dmaBuffer, int stride, uint16_t packet)
{
    int i;
    for (i = 0; i < 16; i++) {
        dmaBuffer[i * stride] = (packet & 0x8000) ? DSHOT_MOTOR_BIT_1 : DSHOT_MOTOR_BIT_0;  // MSB first
        packet <<= 1;
    }
    dmaBuffer[i++ * stride] = 0;
    dmaBuffer[i++ * stride] = 0;
}
#else
static void loadDmaBufferDshot(timerDMASafeType_t *dmaBuffer, uint16_t packet)
{
    for (int i = 0; i < 16; i++) {
        dmaBuffer[i] = (packet & 0x8000) ? DSHOT_MOTOR_BIT_1 : DSHOT_MOTOR_BIT_0;  // MSB first
        packet <<= 1;
    }
    // Frame reset slots: rewritten every time, a bidir port captures edges into this buffer
    dmaBuffer[16] = 0;
    dmaBuffer[17] = 0;
}
#endif

static uint16_t prepareDshotPacket(const uint16_t value, bool requestTelemetry)
{
    // The telemetry bit asks for a serial telemetry packet on the ESC's telemetry wire;
    // the eRPM reply on the motor line is triggered by the inverted checksum alone, so
    // bidir leaves the bit to the serial ESC sensor's round-robin requests
    uint16_t packet = (value << 1) | (requestTelemetry ? 1 : 0);

    // compute checksum
    int csum = 0;
    int csum_data = packet;
    for (int i = 0; i < 3; i++) {
        csum ^=  csum_data;   // xor data by nibbles
        csum_data >>= 4;
    }
#ifdef USE_DSHOT_BIDIR
    if (useDshotTelemetry) {
        csum = ~csum;
    }
#endif
    csum &= 0xf;

    // append checksum
    packet = (packet << 4) | csum;

    return packet;
}
#endif

#if defined(USE_DSHOT)
static void motorConfigDigitalUpdateInterval(uint16_t motorPwmRateHz)
{
    digitalMotorUpdateIntervalUs = 1000000 / motorPwmRateHz;
    digitalMotorLastUpdateUs = 0;
}

static void pwmWriteDigital(uint8_t index, uint16_t value)
{
    // Just keep track of motor value, actual update happens in pwmCompleteMotorUpdate()
    // DSHOT and some other digital protocols use 11-bit throttle range [0;2047]
    motors[index].value = constrain(value, 0, 2047);
}

bool isMotorProtocolDshot(void)
{
    // We look at cached `initMotorProtocol` to make sure we are consistent with the initialized config
    // motorConfig()->motorPwmProtocol may change at run time which will cause uninitialized structures to be used
    return getMotorProtocolProperties(initMotorProtocol)->isDSHOT;
}

bool isMotorProtocolDigital(void)
{
    return isMotorProtocolDshot();
}

void pwmRequestMotorTelemetry(int motorIndex)
{
    if (!isMotorProtocolDigital()) {
        return;
    }

    const int motorCount = getMotorCount();
    for (int index = 0; index < motorCount; index++) {
        if (motors[index].pwmPort && motors[index].pwmPort->configured && index == motorIndex) {
            motors[index].requestTelemetry = true;
        }
    }
}

#ifdef USE_DSHOT
void sendDShotCommand(dshotCommands_e cmd) {
    circularBufferPushElement(&commandsCircularBuffer, (uint8_t *) &cmd);
}

void initDShotCommands(void) {
    circularBufferInit(&commandsCircularBuffer, commandsBuff,DHSOT_COMMAND_QUEUE_SIZE, sizeof(dshotCommands_e));

    currentExecutingCommand.remainingRepeats = 0;
}

static int getDShotCommandRepeats(dshotCommands_e cmd) {
    int repeats = 1;

    switch (cmd) {
        case DSHOT_CMD_EXTENDED_TELEMETRY_ENABLE:
        case DSHOT_CMD_SPIN_DIRECTION_NORMAL:
        case DSHOT_CMD_SPIN_DIRECTION_REVERSED:
            repeats = 10;
            break;
        default:
            break;
    }

    return repeats;
}

// NOINLINE: pwmCompleteMotorUpdate() is inlined into the ITCM scheduler; commands are rare
static bool NOINLINE executeDShotCommands(void){
    
    timeUs_t tNow = micros();

    if(currentExecutingCommand.remainingRepeats == 0) {
       const int isTherePendingCommands = !circularBufferIsEmpty(&commandsCircularBuffer);
        if (isTherePendingCommands && (tNow - lastCommandSent > DSHOT_COMMAND_INTERVAL_US)){
            //Load the command
            dshotCommands_e cmd;
            circularBufferPopHead(&commandsCircularBuffer, (uint8_t *) &cmd);
            currentExecutingCommand.cmd = cmd;
            currentExecutingCommand.remainingRepeats = getDShotCommandRepeats(cmd);
            commandPostDelay = DSHOT_COMMAND_INTERVAL_US;
        } else {
            if (commandPostDelay) {
                if (tNow - lastCommandSent < commandPostDelay) {
                    return false;
                }
                commandPostDelay = 0;
            }

            return true;
        }  
    }
    for (uint8_t i = 0; i < getMotorCount(); i++) {
         motors[i].requestTelemetry = true;
         motors[i].value = currentExecutingCommand.cmd;
    }
    if (tNow - lastCommandSent >= DSHOT_COMMAND_DELAY_US) {
        currentExecutingCommand.remainingRepeats--; 
        lastCommandSent = tNow;
        return true;
    } else {
        return false;
    }
}
#endif

void pwmCompleteMotorUpdate(void) {
    // This only makes sense for digital motor protocols
    if (!isMotorProtocolDigital()) {
        return;
    }

#ifdef USE_DSHOT_BIDIR
    if (useDshotTelemetry && !pwmDshotDecodeTelemetry()) {
        return;
    }
#endif

    int motorCount = getMotorCount();
    timeUs_t currentTimeUs = micros();

#ifdef USE_DSHOT_BIDIR
    if (useDshotTelemetry) {
        escSensorFrameWindowUpdate(currentTimeUs / 1000);
    }
#endif

    // Enforce motor update rate
    if ((digitalMotorUpdateIntervalUs == 0) || ((currentTimeUs - digitalMotorLastUpdateUs) <= digitalMotorUpdateIntervalUs)) {
        return;
    }

    digitalMotorLastUpdateUs = currentTimeUs;

#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {

#ifdef USE_DSHOT_BIDIR
        if (dshotPinsDisconnected) {
            dshotConnectOutputs();
            dshotPinsDisconnected = false;
        }
#endif

        if (!executeDShotCommands()) {
            return;
        }

#ifdef USE_DSHOT_DMAR
        for (int index = 0; index < motorCount; index++) {
            if (motors[index].pwmPort && motors[index].pwmPort->configured) {
                uint16_t packet = prepareDshotPacket(motors[index].value, motors[index].requestTelemetry);
                loadDmaBufferDshotStride(&motors[index].pwmPort->dmaBurstBuffer[motors[index].pwmPort->tch->timHw->channelIndex], 4, packet);
                motors[index].requestTelemetry = false;
            }
        }

        for (int burstDmaTimerIndex = 0; burstDmaTimerIndex < burstDmaTimersCount; burstDmaTimerIndex++) {
            burstDmaTimer_t *burstDmaTimer = &burstDmaTimers[burstDmaTimerIndex];
            pwmBurstDMAStart(burstDmaTimer, DSHOT_DMA_BUFFER_SIZE * 4);
        }
#else
        // Generate DMA buffers
        for (int index = 0; index < motorCount; index++) {
            if (motors[index].pwmPort && motors[index].pwmPort->configured) {
                uint16_t packet = prepareDshotPacket(motors[index].value, motors[index].requestTelemetry);
                loadDmaBufferDshot(motors[index].pwmPort->dmaBuffer, packet);
                timerPWMPrepareDMA(motors[index].pwmPort->tch, DSHOT_DMA_BUFFER_SIZE);
                motors[index].requestTelemetry = false;
            }
        }

#ifdef USE_DSHOT_BIDIR
        if (useDshotTelemetry) {
            // Reference for the reply window; the completion ISRs don't read the clock
            dshotTelemetryFrameStampUs = micros();
            if (debugMode == DEBUG_ESC) {
                dshotFrameStartCycles = ticks();
            }
        }
#endif

        // Start DMA on all timers
        for (int index = 0; index < motorCount; index++) {
            if (motors[index].pwmPort && motors[index].pwmPort->configured) {
                timerPWMStartDMA(motors[index].pwmPort->tch);
            }
        }
#endif
    }
#endif
}

#else // digital motor protocol

// This stub is needed to avoid ESC_SENSOR dependency on DSHOT
void pwmRequestMotorTelemetry(int motorIndex)
{
    UNUSED(motorIndex);
}

#endif

void pwmMotorPreconfigure(void)
{
    // Keep track of initial motor protocol
    initMotorProtocol = motorConfig()->motorPwmProtocol;
#ifdef USE_DSHOT_BIDIR
    useDshotTelemetry = motorConfig()->useDshotTelemetry && getMotorProtocolProperties(initMotorProtocol)->isDSHOT;
#endif

#ifdef BRUSHED_MOTORS
    initMotorProtocol = PWM_TYPE_BRUSHED;   // Override proto
#endif

    // Protocol-specific configuration
    switch (initMotorProtocol) {
        default:
            motorWritePtr = pwmWriteNull;
            break;

        case PWM_TYPE_STANDARD:
        case PWM_TYPE_BRUSHED:
        case PWM_TYPE_ONESHOT125:
        case PWM_TYPE_MULTISHOT:
            motorWritePtr = pwmWriteStandard;
            break;

#ifdef USE_DSHOT
        case PWM_TYPE_DSHOT600:
        case PWM_TYPE_DSHOT300:
        case PWM_TYPE_DSHOT150:
            motorConfigDigitalUpdateInterval(getEscUpdateFrequency());
            motorWritePtr = pwmWriteDigital;
            break;
#endif

#ifdef USE_MOTOR_SRXL2
        case PWM_TYPE_SRXL2:
            /* Nothing to fall back on if this fails: the pin is a UART pin, not a
             * timer output, so there is no PWM to degrade to. Leaving
             * motorWritePtr null keeps the motor unwritten, which is the honest
             * outcome of a port that was never assigned. */
            if (srxl2MotorInitialize()) {
                srxl2MotorSetReverseChannel(motorConfig()->srxl2ReverseChannel);
                srxl2MotorSetTelemetryRate(motorConfig()->srxl2TelemetryRate);
                motorWritePtr = srxl2MotorUpdate;
            }
            break;
#endif
    }
}

/**
 * This function return the PWM frequency based on ESC protocol. We allow customer rates only for Brushed motors
 */ 
uint32_t getEscUpdateFrequency(void) {
    switch (initMotorProtocol) {
        case PWM_TYPE_BRUSHED:
            return motorConfig()->motorPwmRate;

        case PWM_TYPE_STANDARD:
            return 400;

        case PWM_TYPE_MULTISHOT:
            return 2000;

        case PWM_TYPE_DSHOT150:
            return 4000;

        case PWM_TYPE_DSHOT300:
            return 8000;

        case PWM_TYPE_DSHOT600:
            return 16000;

        case PWM_TYPE_ONESHOT125:
        default:
            return 1000;

    }
}

bool pwmMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, bool enableOutput)
{
    switch (initMotorProtocol) {
    case PWM_TYPE_BRUSHED:
        motors[motorIndex].pwmPort = motorConfigPwm(timerHardware, 0.0f, 0.0f, getEscUpdateFrequency(), enableOutput);
        break;

    case PWM_TYPE_ONESHOT125:
        motors[motorIndex].pwmPort = motorConfigPwm(timerHardware, 125e-6f, 125e-6f, getEscUpdateFrequency(), enableOutput);
        break;

    case PWM_TYPE_MULTISHOT:
        motors[motorIndex].pwmPort = motorConfigPwm(timerHardware, 5e-6f, 20e-6f, getEscUpdateFrequency(), enableOutput);
        break;

#ifdef USE_DSHOT
    case PWM_TYPE_DSHOT600:
    case PWM_TYPE_DSHOT300:
    case PWM_TYPE_DSHOT150:
        motors[motorIndex].pwmPort = motorConfigDshot(timerHardware, getDshotHz(initMotorProtocol), enableOutput, motorIndex);
        break;
#endif

    case PWM_TYPE_STANDARD:
        motors[motorIndex].pwmPort = motorConfigPwm(timerHardware, 1e-3f, 1e-3f, getEscUpdateFrequency(), enableOutput);
        break;

    default:
        motors[motorIndex].pwmPort = NULL;
        break;
    }

    return (motors[motorIndex].pwmPort != NULL);
}

// Helper function for ESC passthrough
ioTag_t pwmGetMotorPinTag(int motorIndex)
{
    if (motors[motorIndex].pwmPort) {
        return motors[motorIndex].pwmPort->tch->timHw->tag;
    }
    else {
        return IOTAG_NONE;
    }
}

// Helper function for ESC passthrough: hand the pin back to its timer after the 4-way
// interface drove it as plain GPIO. The alternate function has to be given explicitly:
// on HAL targets a bare IOConfigGPIO(IOCFG_AF_PP) programs AF0 and the timer loses the pin.
void pwmRestoreMotorPin(int motorIndex)
{
    if (motors[motorIndex].pwmPort) {
        const timerHardware_t *timHw = motors[motorIndex].pwmPort->tch->timHw;
        IOConfigGPIOAF(IOGetByTag(timHw->tag), IOCFG_AF_PP, timHw->alternateFunction);
    }
}

static void pwmServoWriteStandard(uint8_t index, uint16_t value)
{
    if (index < MAX_SERVOS && servos[index]) {
        *servos[index]->ccr = value;
    }
}

#ifdef USE_SERVO_SBUS
static void sbusPwmWriteStandard(uint8_t index, uint16_t value)
{
    pwmServoWriteStandard(index, value);
    sbusServoUpdate(index, value);
}
#endif

void pwmServoPreconfigure(void)
{
    // Protocol-specific configuration
    switch (servoConfig()->servo_protocol) {
        default:
        case SERVO_TYPE_PWM:
            servoWritePtr = pwmServoWriteStandard;
            break;

#ifdef USE_SERVO_SBUS
        case SERVO_TYPE_SBUS:
            sbusServoInitialize();
            servoWritePtr = sbusServoUpdate;
            break;

        case SERVO_TYPE_SBUS_PWM:
            sbusServoInitialize();
            servoWritePtr = sbusPwmWriteStandard;
            break;
#endif
    }
}

bool pwmServoConfig(const timerHardware_t *timerHardware, uint8_t servoIndex, uint16_t servoPwmRate, uint16_t servoCenterPulse, bool enableOutput)
{
    pwmOutputPort_t * port = pwmOutConfig(timerHardware, OWNER_SERVO, PWM_TIMER_HZ, PWM_TIMER_HZ / servoPwmRate, servoCenterPulse, enableOutput);

    if (port) {
        servos[servoIndex] = port;
        return true;
    }

    return false;
}

void pwmWriteServo(uint8_t index, uint16_t value)
{
    if (servoWritePtr && index < MAX_SERVOS) {
        servoWritePtr(index, value);
    }
}

void pwmWriteBeeper(bool onoffBeep)
{
    if (beeperPwm == NULL)
        return;

    if (onoffBeep == true) {
        *beeperPwm->ccr = (1000000 / beeperFrequency) / 2;
    } else {
        *beeperPwm->ccr = 0;
    }
}

bool beeperPwmInit(ioTag_t tag, uint16_t frequency)
{
    beeperPwm = NULL;

    const timerHardware_t *timHw = timerGetByTag(tag, TIM_USE_BEEPER);

    if (timHw) {
        // Attempt to allocate TCH
        TCH_t * tch = timerGetTCH(timHw);
        if (tch == NULL) {
            return false;
        }

        beeperPwm = &beeperPwmPort;
        beeperFrequency = frequency;
        IOConfigGPIOAF(IOGetByTag(tag), IOCFG_AF_PP, timHw->alternateFunction);
        pwmOutConfigTimer(beeperPwm, tch, PWM_TIMER_HZ, 1000000 / beeperFrequency, (1000000 / beeperFrequency) / 2);
        return true;
    }

    return false;
}

#endif

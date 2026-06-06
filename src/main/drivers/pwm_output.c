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

#if !defined(SITL_BUILD)

#include "build/debug.h"
#include "build/build_config.h"

#include "common/log.h"
#include "common/maths.h"
#include "common/circular_queue.h"

#include "drivers/io.h"
#include "drivers/dshot.h"
#include "drivers/nvic.h"
#include "drivers/timer.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
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
#define MOTOR_DSHOT600_HZ     12000000
#define MOTOR_DSHOT300_HZ     6000000
#define MOTOR_DSHOT150_HZ     3000000


#define DSHOT_MOTOR_BIT_0       7
#define DSHOT_MOTOR_BIT_1       14
#define DSHOT_MOTOR_BITLENGTH   20

#define DSHOT_DMA_BUFFER_SIZE   18 /* resolution + frame reset (2us) */
#define DSHOT_TELEMETRY_DEADTIME_US 35
#define GCR_TELEMETRY_INPUT_LEN MAX_GCR_EDGES
#define MAX_DMA_TIMERS          8

#define DSHOT_COMMAND_DELAY_US 1000
#define DSHOT_COMMAND_INTERVAL_US 10000
#define DSHOT_COMMAND_QUEUE_LENGTH 8
#define DHSOT_COMMAND_QUEUE_SIZE   DSHOT_COMMAND_QUEUE_LENGTH * sizeof(dshotCommands_e)
#endif

typedef void (*pwmWriteFuncPtr)(uint8_t index, uint16_t value);  // function pointer used to write motors

#ifdef USE_DSHOT_DMAR
    timerDMASafeType_t dmaBurstBuffer[MAX_DMA_TIMERS][DSHOT_DMA_BUFFER_SIZE * 4];
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
    timerDMASafeType_t dmaBuffer[GCR_TELEMETRY_INPUT_LEN];
#ifdef USE_DSHOT_DMAR
    timerDMASafeType_t *dmaBurstBuffer;
#endif
    bool telemetryInputActive;
    timeUs_t telemetryInputStampUs;
    timeUs_t dshotTelemetryDeadtimeUs;
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
static bool dshotTelemetryPending = false;
    
static circularBuffer_t commandsCircularBuffer;
static uint8_t commandsBuff[DHSOT_COMMAND_QUEUE_SIZE];
static currentExecutingCommand_t currentExecutingCommand;

static uint16_t prepareDshotPacket(const uint16_t value, bool requestTelemetry);
#ifndef USE_DSHOT_DMAR
static void loadDmaBufferDshot(timerDMASafeType_t *dmaBuffer, uint16_t packet);
#else
static void loadDmaBufferDshotStride(timerDMASafeType_t *dmaBuffer, int stride, uint16_t packet);
#endif

#ifdef USE_DSHOT_DMAR
burstDmaTimer_t burstDmaTimers[MAX_DMA_TIMERS];
uint8_t burstDmaTimersCount = 0;
#endif

// GCR decode algorithm ported from Betaflight (GPLv3)
static uint16_t dshotDecodeTelemetryPacket(const uint32_t buffer[], uint32_t count);
static bool pwmDshotDecodeTelemetry(void);
static void pwmDshotSetDirectionOutput(pwmOutputPort_t *port);
static void pwmDshotSetDirectionInput(pwmOutputPort_t *port);
static void pwmDshotDmaIrqHandler(DMA_t descriptor);
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

void pwmSetMotorDMACircular(bool circular)
{
#ifdef USE_DSHOT
    if (!isMotorProtocolDshot()) {
        return;
    }

    int motorCount = getMotorCount();

    if (circular) {
        // Load zero-throttle packets directly into DMA buffers,
        // bypassing the rate limiter in pwmCompleteMotorUpdate()
        uint16_t packet = prepareDshotPacket(0, false);
        for (int i = 0; i < motorCount; i++) {
            if (motors[i].pwmPort && motors[i].pwmPort->configured) {
#ifdef USE_DSHOT_DMAR
                loadDmaBufferDshotStride(&motors[i].pwmPort->dmaBurstBuffer[motors[i].pwmPort->tch->timHw->channelIndex], 4, packet);
#else
                loadDmaBufferDshot(motors[i].pwmPort->dmaBuffer, packet);
#endif
            }
        }
    }

#ifdef USE_DSHOT_DMAR
    // Burst DMA: one DMA stream per timer, shared across channels
    for (int i = 0; i < burstDmaTimersCount; i++) {
        burstDmaTimer_t *burstDmaTimer = &burstDmaTimers[i];
        // Find the first motor using this timer to get the TCH for DMA state
        for (int m = 0; m < motorCount; m++) {
            if (motors[m].pwmPort && motors[m].pwmPort->configured && motors[m].pwmPort->tch
                && motors[m].pwmPort->tch->timHw->tim == burstDmaTimer->timer) {
                impl_pwmBurstDMASetCircular(burstDmaTimer, motors[m].pwmPort->tch, circular, DSHOT_DMA_BUFFER_SIZE * 4);
                break;
            }
        }
    }
#else
    // Per-channel DMA: one DMA stream per motor
    for (int i = 0; i < motorCount; i++) {
        if (motors[i].pwmPort && motors[i].pwmPort->configured && motors[i].pwmPort->tch) {
            impl_timerPWMSetDMACircular(motors[i].pwmPort->tch, circular, DSHOT_DMA_BUFFER_SIZE);
        }
    }
#endif
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

static uint32_t dshotDmaSource(const pwmOutputPort_t *port)
{
#if defined(USE_HAL_DRIVER) || !defined(AT32F43x)
    static const uint32_t sources[] = { TIM_DMA_CC1, TIM_DMA_CC2, TIM_DMA_CC3, TIM_DMA_CC4 };
#else
    static const uint32_t sources[] = { TMR_C1_DMA_REQUEST, TMR_C2_DMA_REQUEST, TMR_C3_DMA_REQUEST, TMR_C4_DMA_REQUEST };
#endif
    return sources[port->tch->timHw->channelIndex];
}

static uint32_t dshotDmaStream(const pwmOutputPort_t *port)
{
#if defined(USE_HAL_DRIVER)
    static const uint32_t streams[] = {
        LL_DMA_STREAM_0, LL_DMA_STREAM_1, LL_DMA_STREAM_2, LL_DMA_STREAM_3,
        LL_DMA_STREAM_4, LL_DMA_STREAM_5, LL_DMA_STREAM_6, LL_DMA_STREAM_7
    };
    return streams[DMATAG_GET_STREAM(port->tch->timHw->dmaTag)];
#else
    UNUSED(port);
    return 0;
#endif
}

static uint32_t dshotTimChannel(const pwmOutputPort_t *port)
{
#if defined(USE_HAL_DRIVER)
    static const uint32_t channels[] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4 };
#elif defined(AT32F43x)
    static const uint32_t channels[] = { TMR_SELECT_CHANNEL_1, TMR_SELECT_CHANNEL_2, TMR_SELECT_CHANNEL_3, TMR_SELECT_CHANNEL_4 };
#else
    static const uint32_t channels[] = { TIM_Channel_1, TIM_Channel_2, TIM_Channel_3, TIM_Channel_4 };
#endif
    return channels[port->tch->timHw->channelIndex];
}

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
        } else {
            len = 21 - bits;
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
#if defined(USE_HAL_DRIVER)
    TIM_OC_InitTypeDef init = {0};
    init.OCMode = TIM_OCMODE_PWM1;
    init.OCIdleState = TIM_OCIDLESTATE_SET;
    init.OCPolarity = TIM_OCPOLARITY_LOW;
    init.OCNIdleState = TIM_OCNIDLESTATE_SET;
    init.OCNPolarity = TIM_OCNPOLARITY_LOW;
    init.Pulse = 0;
    init.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(port->tch->timCtx->timHandle, &init, dshotTimChannel(port));
    if (port->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        HAL_TIMEx_PWMN_Start(port->tch->timCtx->timHandle, dshotTimChannel(port));
    } else {
        HAL_TIM_PWM_Start(port->tch->timCtx->timHandle, dshotTimChannel(port));
    }

    const uint32_t streamLL = dshotDmaStream(port);
    DMA_TypeDef *dmaBase = port->tch->dma->dma;
    LL_DMA_DisableStream(dmaBase, streamLL);
    while (LL_DMA_IsEnabledStream(dmaBase, streamLL)) { }
    LL_DMA_ConfigAddresses(dmaBase, streamLL, (uint32_t)port->dmaBuffer, (uint32_t)port->ccr, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(dmaBase, streamLL, DSHOT_DMA_BUFFER_SIZE);
    LL_DMA_EnableIT_TC(dmaBase, streamLL);
    port->telemetryInputActive = false;
#elif defined(AT32F43x)
    tmr_output_config_type output = {0};
    tmr_output_default_para_init(&output);
    output.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    if (port->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        output.oc_output_state = FALSE;
        output.occ_output_state = TRUE;
        output.occ_polarity = TMR_OUTPUT_ACTIVE_LOW;
        output.occ_idle_state = FALSE;
    } else {
        output.oc_output_state = TRUE;
        output.occ_output_state = FALSE;
        output.oc_polarity = TMR_OUTPUT_ACTIVE_LOW;
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
        init.TIM_OCNPolarity = TIM_OCPolarity_Low;
        init.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    } else {
        init.TIM_OutputState = TIM_OutputState_Enable;
        init.TIM_OutputNState = TIM_OutputNState_Disable;
        init.TIM_OCPolarity = TIM_OCPolarity_Low;
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

static void pwmDshotSetDirectionInput(pwmOutputPort_t *port)
{
#if defined(USE_HAL_DRIVER)
    TIM_IC_InitTypeDef init = {0};
    init.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
    init.ICSelection = TIM_ICSELECTION_DIRECTTI;
    init.ICPrescaler = TIM_ICPSC_DIV1;
    init.ICFilter = 2;
    HAL_TIM_IC_ConfigChannel(port->tch->timCtx->timHandle, &init, dshotTimChannel(port));
    HAL_TIM_IC_Start(port->tch->timCtx->timHandle, dshotTimChannel(port));

    const uint32_t streamLL = dshotDmaStream(port);
    DMA_TypeDef *dmaBase = port->tch->dma->dma;
    LL_DMA_DisableStream(dmaBase, streamLL);
    while (LL_DMA_IsEnabledStream(dmaBase, streamLL)) { }
    LL_DMA_ConfigAddresses(dmaBase, streamLL, (uint32_t)port->ccr, (uint32_t)port->dmaBuffer, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(dmaBase, streamLL, GCR_TELEMETRY_INPUT_LEN);
    LL_DMA_EnableIT_TC(dmaBase, streamLL);
    LL_DMA_EnableStream(dmaBase, streamLL);
    LL_TIM_EnableDMAReq_CCx(port->tch->timHw->tim, dshotDmaSource(port));
#elif defined(AT32F43x)
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
    port->telemetryInputStampUs = micros();
    port->telemetryInputActive = true;
}

static void pwmDshotDmaIrqHandler(DMA_t descriptor)
{
    if (!DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        return;
    }

    pwmOutputPort_t *port = &pwmOutputPorts[descriptor->userParam];

#if defined(USE_HAL_DRIVER)
    const uint32_t streamLL = dshotDmaStream(port);
    LL_DMA_DisableStream(port->tch->dma->dma, streamLL);
    LL_TIM_DisableDMAReq_CCx(port->tch->timHw->tim, dshotDmaSource(port));
#elif defined(AT32F43x)
    dma_channel_enable(port->tch->dma->ref, FALSE);
    tmr_dma_request_enable(port->tch->timHw->tim, dshotDmaSource(port), FALSE);
#else
    DMA_Cmd(port->tch->dma->ref, DISABLE);
    TIM_DMACmd(port->tch->timHw->tim, dshotDmaSource(port), DISABLE);
#endif

    if (useDshotTelemetry && !port->telemetryInputActive) {
        pwmDshotSetDirectionInput(port);
        dshotTelemetryPending = true;
    }

    DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
}

static bool pwmDshotDecodeTelemetry(void)
{
    if (!useDshotTelemetry || !dshotTelemetryPending) {
        return true;
    }

    const timeUs_t now = micros();
    for (int motorIndex = 0; motorIndex < getMotorCount(); motorIndex++) {
        pwmOutputPort_t *port = motors[motorIndex].pwmPort;
        if (!port || !port->configured || !port->telemetryInputActive) {
            continue;
        }

        if ((now - port->telemetryInputStampUs) < port->dshotTelemetryDeadtimeUs) {
            return false;
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

        if (edges > MIN_GCR_EDGES) {
            const uint16_t rawValue = dshotDecodeTelemetryPacket((const uint32_t *)port->dmaBuffer, edges);
            const uint16_t processed = dshotProcessPacket(rawValue, motorIndex);

            if (processed != DSHOT_TELEMETRY_INVALID && processed != DSHOT_TELEMETRY_NOEDGE) {
#ifdef USE_ESC_SENSOR
                escSensorData_t data;
                if (getDshotEscSensorData(&data, motorIndex)) {
                    escSensorSetDshotData(motorIndex, computeRpm((int16_t)data.rpm), data.temperature, data.voltage, data.current);
                } else {
                    escSensorSetDshotData(motorIndex, (uint32_t)getDshotRpm(motorIndex), 0, 0, 0);
                }
#endif
            }
        }

        pwmDshotSetDirectionOutput(port);
    }

    dshotTelemetryPending = false;
    return true;
}

static pwmOutputPort_t * motorConfigDshot(const timerHardware_t * timerHardware, uint32_t dshotHz, bool enableOutput)
{
    // Try allocating new port
    pwmOutputPort_t * port = pwmOutConfig(timerHardware, OWNER_MOTOR, dshotHz, DSHOT_MOTOR_BITLENGTH, 0, enableOutput);

    if (!port) {
        return NULL;
    }

    if (enableOutput && useDshotTelemetry) {
        IOConfigGPIOAF(IOGetByTag(timerHardware->tag), IOCFG_AF_PP_UP, timerHardware->alternateFunction);
    }

    // Configure timer DMA
#ifdef USE_DSHOT_DMAR
    if (!useDshotTelemetry) {
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
    } else
#endif
    {
        if (timerPWMConfigChannelDMA(port->tch, port->dmaBuffer, sizeof(port->dmaBuffer[0]), GCR_TELEMETRY_INPUT_LEN)) {
        // Only mark as DSHOT channel if DMA was set successfully
        ZERO_FARRAY(port->dmaBuffer);
        port->configured = true;
            port->dshotTelemetryDeadtimeUs = DSHOT_TELEMETRY_DEADTIME_US + 1000000 * (16 * DSHOT_MOTOR_BITLENGTH) / dshotHz;
            pwmDshotSetDirectionOutput(port);
            dmaSetHandler(port->tch->dma, pwmDshotDmaIrqHandler, NVIC_PRIO_TIMER_DMA, allocatedOutputPortCount - 1);
        }
    }

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
    dmaBuffer[16] = 0;
    dmaBuffer[17] = 0;
}
#endif

static uint16_t prepareDshotPacket(const uint16_t value, bool requestTelemetry)
{
    if (useDshotTelemetry) {
        requestTelemetry = true;
    }

    uint16_t packet = (value << 1) | (requestTelemetry ? 1 : 0);

    // compute checksum
    int csum = 0;
    int csum_data = packet;
    for (int i = 0; i < 3; i++) {
        csum ^=  csum_data;   // xor data by nibbles
        csum_data >>= 4;
    }
#ifdef USE_DSHOT
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
        case DSHOT_CMD_SPIN_DIRECTION_NORMAL:
        case DSHOT_CMD_SPIN_DIRECTION_REVERSED:
            repeats = 10;
            break;
        default:
            break;
    }

    return repeats;
}

static bool executeDShotCommands(void){
    
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

    if (useDshotTelemetry && !pwmDshotDecodeTelemetry()) {
        return;
    }

    int motorCount = getMotorCount();
    timeUs_t currentTimeUs = micros();

    // Enforce motor update rate
    if ((digitalMotorUpdateIntervalUs == 0) || ((currentTimeUs - digitalMotorLastUpdateUs) <= digitalMotorUpdateIntervalUs)) {
        return;
    }

    digitalMotorLastUpdateUs = currentTimeUs;

#ifdef USE_DSHOT
    if (isMotorProtocolDshot()) {

        if (!executeDShotCommands()) {
            return;
        }

#ifdef USE_DSHOT_DMAR
        if (!useDshotTelemetry) {
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
        } else
#endif
        {
        // Generate DMA buffers
        for (int index = 0; index < motorCount; index++) {
            if (motors[index].pwmPort && motors[index].pwmPort->configured) {
                uint16_t packet = prepareDshotPacket(motors[index].value, motors[index].requestTelemetry);
                loadDmaBufferDshot(motors[index].pwmPort->dmaBuffer, packet);
                timerPWMPrepareDMA(motors[index].pwmPort->tch, DSHOT_DMA_BUFFER_SIZE);
                motors[index].requestTelemetry = false;
            }
        }

        // Start DMA on all timers
        for (int index = 0; index < motorCount; index++) {
            if (motors[index].pwmPort && motors[index].pwmPort->configured) {
                timerPWMStartDMA(motors[index].pwmPort->tch);
            }
        }
        }
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
#ifdef USE_DSHOT
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
        motors[motorIndex].pwmPort = motorConfigDshot(timerHardware, getDshotHz(initMotorProtocol), enableOutput);
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

void beeperPwmInit(ioTag_t tag, uint16_t frequency)
{
    beeperPwm = NULL;

    const timerHardware_t *timHw = timerGetByTag(tag, TIM_USE_BEEPER);

    if (timHw) {
        // Attempt to allocate TCH
        TCH_t * tch = timerGetTCH(timHw);
        if (tch == NULL) {
            return;
        }

        beeperPwm = &beeperPwmPort;
        beeperFrequency = frequency;
        IOConfigGPIOAF(IOGetByTag(tag), IOCFG_AF_PP, timHw->alternateFunction);
        pwmOutConfigTimer(beeperPwm, tch, PWM_TIMER_HZ, 1000000 / beeperFrequency, (1000000 / beeperFrequency) / 2);
    }
}

#endif

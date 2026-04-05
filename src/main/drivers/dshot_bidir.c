/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <string.h>

#include "platform.h"

#ifdef USE_DSHOT_BIDIR

#include "build/debug.h"

#include "common/time.h"

#include "drivers/dma.h"
#include "drivers/io.h"
#include "drivers/nvic.h"
#include "drivers/pwm_output.h"
#include "drivers/timer.h"

#include "flight/mixer.h"
#include "sensors/rpm_source.h"

#define DSHOT_BIDIR_MOTOR_LIMIT            4
#define DSHOT_BIDIR_CAPTURE_BUFFER_LEN     32
#define DSHOT_BIDIR_MIN_GCR_EDGES          7
#define DSHOT_BIDIR_INVALID                0xFFFF
#define DSHOT_BIDIR_CAPTURE_DEADTIME_US    35
#define DSHOT_BIDIR_TIMER_BIT_COUNT        16
#define DSHOT_BIDIR_TIMER_BIT_LENGTH       20
#define DSHOT_BIDIR_OUTPUT_PERIOD          (DSHOT_BIDIR_TIMER_BIT_LENGTH - 1)
#define DSHOT_BIDIR_CAPTURE_FILTER         2
#define DSHOT_BIDIR_BURST_LENGTH           (18 * 4)
#define DSHOT_BIDIR_ERPM_PER_LSB           100.0f

typedef enum {
    DSHOT_BIDIR_STATE_READY = 0,
    DSHOT_BIDIR_STATE_OUTPUT_PENDING,
    DSHOT_BIDIR_STATE_CAPTURING,
} dshotBidirState_e;

typedef struct {
    TCH_t *tch;
    DMA_t dma;
    uint16_t dmaSource;
    bool useInterruptCapture;
    bool configured;
    volatile uint8_t irqEdgeCount;
    timerCallbacks_t irqCallbacks;
    uint32_t captureBuffer[DSHOT_BIDIR_CAPTURE_BUFFER_LEN];
    DMA_InitTypeDef inputDmaInit;
} dshotBidirMotor_t;

typedef struct {
    TIM_TypeDef *timer;
    TCH_t *burstTch;
    DMA_t burstDma;
    timerDMASafeType_t *dmaBurstBuffer;
    DMA_InitTypeDef burstOutputDmaInit;
    uint32_t captureWindowUs;
    timeUs_t frameStartedUs;
    volatile timeUs_t captureStartedUs;
    volatile dshotBidirState_e state;
    bool burstHandlerInstalled;
} dshotBidirContext_t;

static dshotBidirContext_t dshotBidirCtx;
static dshotBidirMotor_t dshotBidirMotors[DSHOT_BIDIR_MOTOR_LIMIT];
static const timerCallbacks_t dshotBidirCallbacksDisabled = { 0 };
static uint32_t dshotBidirReadCount = 0;
static uint32_t dshotBidirInvalidPacketCount = 0;
static uint32_t dshotBidirNoEdgeCount = 0;
static uint32_t dshotBidirTimeoutCount = 0;
static uint8_t dshotBidirLastEdgeCount[DSHOT_BIDIR_MOTOR_LIMIT];
static uint16_t dshotBidirLastRawValue[DSHOT_BIDIR_MOTOR_LIMIT];
static uint32_t dshotBidirLastErpmValue[DSHOT_BIDIR_MOTOR_LIMIT];
static uint32_t dshotBidirLastRpmValue[DSHOT_BIDIR_MOTOR_LIMIT];

static void dshotBidirUpdateDebug(uint32_t readDelta, uint32_t invalidDelta, uint32_t noEdgeDelta, uint32_t timeoutDelta)
{
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 0, dshotBidirLastRpmValue[0]);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 1, dshotBidirLastRpmValue[1]);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 2, dshotBidirLastRpmValue[2]);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 3, dshotBidirLastRpmValue[3]);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 4, readDelta);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 5, invalidDelta);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 6, noEdgeDelta);
    DEBUG_SET(DEBUG_DSHOT_BIDIR, 7, timeoutDelta);
}

static uint16_t dshotBidirTimerDmaSource(uint8_t channelIndex)
{
    switch (channelIndex) {
        case 0:
            return TIM_DMA_CC1;
        case 1:
            return TIM_DMA_CC2;
        case 2:
            return TIM_DMA_CC3;
        case 3:
            return TIM_DMA_CC4;
        default:
            return 0;
    }
}

static uint16_t dshotBidirTimerChannel(uint8_t channelIndex)
{
    switch (channelIndex) {
        case 0:
            return TIM_Channel_1;
        case 1:
            return TIM_Channel_2;
        case 2:
            return TIM_Channel_3;
        case 3:
            return TIM_Channel_4;
        default:
            return TIM_Channel_1;
    }
}

static uint32_t dshotBidirDecodeTelemetryPacket(const uint32_t buffer[], uint32_t count)
{
    uint32_t value = 0;
    uint32_t oldValue = buffer[0];
    int bits = 0;
    int len;

    for (uint32_t i = 1; i <= count; i++) {
        if (i < count) {
            const int diff = buffer[i] - oldValue;
            if (bits >= 21) {
                break;
            }
            len = (diff + 8) / 16;
        } else {
            len = 21 - bits;
        }

        if (len <= 0 || (bits + len) > 21) {
            return DSHOT_BIDIR_INVALID;
        }

        value <<= len;
        value |= 1 << (len - 1);
        oldValue = buffer[i];
        bits += len;
    }

    if (bits != 21) {
        return DSHOT_BIDIR_INVALID;
    }

    static const uint32_t decodeTable[32] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 9, 10, 11, 0, 13, 14, 15,
        0, 0, 2, 3, 0, 5, 6, 7, 0, 0, 8, 1, 0, 4, 12, 0
    };

    uint32_t decodedValue = decodeTable[value & 0x1f];
    decodedValue |= decodeTable[(value >> 5) & 0x1f] << 4;
    decodedValue |= decodeTable[(value >> 10) & 0x1f] << 8;
    decodedValue |= decodeTable[(value >> 15) & 0x1f] << 12;

    uint32_t csum = decodedValue;
    csum ^= csum >> 8;
    csum ^= csum >> 4;

    if ((csum & 0xf) != 0xf) {
        return DSHOT_BIDIR_INVALID;
    }

    return decodedValue >> 4;
}

static uint32_t dshotBidirDecodeErpmTelemetryValue(uint16_t value)
{
    if (value == 0x0fff) {
        return 0;
    }

    value = (value & 0x01ff) << ((value & 0xfe00) >> 9);
    if (!value) {
        return DSHOT_BIDIR_INVALID;
    }

    return (1000000U * 60U / 100U + value / 2U) / value;
}

static uint32_t dshotBidirComputeRpm(uint32_t erpm)
{
    const uint8_t polePairs = motorConfig()->motorPoleCount / 2;

    if (polePairs == 0) {
        return 0;
    }

    return lrintf((float)erpm * DSHOT_BIDIR_ERPM_PER_LSB / polePairs);
}

static bool dshotBidirOutputInverted(const dshotBidirMotor_t *motor)
{
    return (motor->tch->timHw->output & TIMER_OUTPUT_INVERTED) == 0;
}

static ioConfig_t dshotBidirGetIoConfig(const dshotBidirMotor_t *motor)
{
    return (motor->tch->timHw->output & TIMER_OUTPUT_INVERTED) ? IOCFG_AF_PP_PD : IOCFG_AF_PP_UP;
}

static void dshotBidirConfigureOutputIo(const dshotBidirMotor_t *motor)
{
    const IO_t io = IOGetByTag(motor->tch->timHw->tag);
    IOConfigGPIOAF(io, dshotBidirGetIoConfig(motor), motor->tch->timHw->alternateFunction);
}

static void dshotBidirConfigureOutputChannel(const dshotBidirMotor_t *motor, uint16_t value)
{
    TIM_OCInitTypeDef outputInit;

    TIM_OCStructInit(&outputInit);
    outputInit.TIM_OCMode = TIM_OCMode_PWM1;
    outputInit.TIM_Pulse = value;

    if (motor->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        outputInit.TIM_OutputState = TIM_OutputState_Disable;
        outputInit.TIM_OutputNState = TIM_OutputNState_Enable;
        outputInit.TIM_OCNPolarity = dshotBidirOutputInverted(motor) ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
        outputInit.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    } else {
        outputInit.TIM_OutputState = TIM_OutputState_Enable;
        outputInit.TIM_OutputNState = TIM_OutputState_Disable;
        outputInit.TIM_OCPolarity = dshotBidirOutputInverted(motor) ? TIM_OCPolarity_Low : TIM_OCPolarity_High;
        outputInit.TIM_OCIdleState = TIM_OCIdleState_Set;
    }

    switch (motor->tch->timHw->channelIndex) {
        case 0:
            TIM_OC1Init(motor->tch->timHw->tim, &outputInit);
            TIM_OC1PreloadConfig(motor->tch->timHw->tim, TIM_OCPreload_Enable);
            break;
        case 1:
            TIM_OC2Init(motor->tch->timHw->tim, &outputInit);
            TIM_OC2PreloadConfig(motor->tch->timHw->tim, TIM_OCPreload_Enable);
            break;
        case 2:
            TIM_OC3Init(motor->tch->timHw->tim, &outputInit);
            TIM_OC3PreloadConfig(motor->tch->timHw->tim, TIM_OCPreload_Enable);
            break;
        case 3:
            TIM_OC4Init(motor->tch->timHw->tim, &outputInit);
            TIM_OC4PreloadConfig(motor->tch->timHw->tim, TIM_OCPreload_Enable);
            break;
    }
}

static void dshotBidirInvalidateAll(void)
{
    for (uint8_t i = 0; i < DSHOT_BIDIR_MOTOR_LIMIT; i++) {
        rpmSourceInvalidateDshotBidir(i);
    }
}

static void dshotBidirIrqCaptureCallback(TCH_t *tch, uint32_t value)
{
    if (dshotBidirCtx.state != DSHOT_BIDIR_STATE_CAPTURING) {
        return;
    }

    for (uint8_t i = 0; i < DSHOT_BIDIR_MOTOR_LIMIT; i++) {
        dshotBidirMotor_t *motor = &dshotBidirMotors[i];
        if (!motor->configured || !motor->useInterruptCapture || motor->tch != tch) {
            continue;
        }

        const uint8_t edgeCount = motor->irqEdgeCount;
        if (edgeCount < DSHOT_BIDIR_CAPTURE_BUFFER_LEN) {
            motor->captureBuffer[edgeCount] = value;
            motor->irqEdgeCount = edgeCount + 1;
        }
        return;
    }
}

static void dshotBidirConfigureInputCapture(TCH_t *tch)
{
    TIM_ICInitTypeDef icInit;

    TIM_ICStructInit(&icInit);
    icInit.TIM_Channel = dshotBidirTimerChannel(tch->timHw->channelIndex);
    icInit.TIM_ICPolarity = TIM_ICPolarity_BothEdge;
    icInit.TIM_ICSelection = TIM_ICSelection_DirectTI;
    icInit.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    icInit.TIM_ICFilter = DSHOT_BIDIR_CAPTURE_FILTER;

    TIM_ICInit(tch->timHw->tim, &icInit);
}

static void dshotBidirRestoreOutputChannel(dshotBidirMotor_t *motor)
{
    if (!motor->configured) {
        return;
    }

    timerChCaptureDisable(motor->tch);
    if (motor->useInterruptCapture) {
        timerChConfigCallbacks(motor->tch, (timerCallbacks_t *)&dshotBidirCallbacksDisabled);
    }

    dshotBidirConfigureOutputIo(motor);
    dshotBidirConfigureOutputChannel(motor, 0);
    timerPWMStart(motor->tch);
}

static void dshotBidirRestoreBurstDma(void)
{
    if (!dshotBidirCtx.burstDma || !dshotBidirCtx.dmaBurstBuffer) {
        return;
    }

    DMA_Cmd(dshotBidirCtx.burstDma->ref, DISABLE);
    TIM_DMACmd(dshotBidirCtx.timer, dshotBidirCtx.burstTch->timCtx->DMASource, DISABLE);
    DMA_DeInit(dshotBidirCtx.burstDma->ref);
    DMA_Init(dshotBidirCtx.burstDma->ref, &dshotBidirCtx.burstOutputDmaInit);
    DMA_ITConfig(dshotBidirCtx.burstDma->ref, DMA_IT_TC, ENABLE);
}

static void dshotBidirStartInputCapture(void)
{
    TIM_TypeDef *timer = dshotBidirCtx.timer;

    if (!timer) {
        dshotBidirCtx.state = DSHOT_BIDIR_STATE_READY;
        return;
    }

    TIM_ARRPreloadConfig(timer, ENABLE);
    timer->ARR = 0xFFFF;
    TIM_SetCounter(timer, 0);

    for (uint8_t i = 0; i < DSHOT_BIDIR_MOTOR_LIMIT; i++) {
        dshotBidirMotor_t *motor = &dshotBidirMotors[i];
        if (!motor->configured) {
            continue;
        }

        memset(motor->captureBuffer, 0, sizeof(motor->captureBuffer));
        motor->irqEdgeCount = 0;
        dshotBidirConfigureInputCapture(motor->tch);

        if (motor->useInterruptCapture) {
            timerChConfigCallbacks(motor->tch, &motor->irqCallbacks);
            timerChCaptureEnable(motor->tch);
            continue;
        }

        DMA_Cmd(motor->dma->ref, DISABLE);
        TIM_DMACmd(timer, motor->dmaSource, DISABLE);
        DMA_DeInit(motor->dma->ref);
        DMA_Init(motor->dma->ref, &motor->inputDmaInit);
        DMA_ITConfig(motor->dma->ref, DMA_IT_TC, DISABLE);
        DMA_Cmd(motor->dma->ref, ENABLE);
        TIM_DMACmd(timer, motor->dmaSource, ENABLE);
    }

    dshotBidirCtx.captureStartedUs = micros();
    dshotBidirCtx.state = DSHOT_BIDIR_STATE_CAPTURING;
}

static void dshotBidirRestoreReadyState(void)
{
    if (dshotBidirCtx.timer) {
        dshotBidirCtx.timer->ARR = DSHOT_BIDIR_OUTPUT_PERIOD;
        TIM_SetCounter(dshotBidirCtx.timer, 0);
    }

    dshotBidirRestoreBurstDma();

    for (uint8_t i = 0; i < DSHOT_BIDIR_MOTOR_LIMIT; i++) {
        dshotBidirRestoreOutputChannel(&dshotBidirMotors[i]);
    }

    dshotBidirCtx.state = DSHOT_BIDIR_STATE_READY;
}

static uint32_t dshotBidirGetCapturedEdgeCount(const dshotBidirMotor_t *motor)
{
    if (motor->useInterruptCapture) {
        return motor->irqEdgeCount;
    }

    return DSHOT_BIDIR_CAPTURE_BUFFER_LEN - DMA_GetCurrDataCounter(motor->dma->ref);
}

static void dshotBidirStopInputCapture(void)
{
    for (uint8_t i = 0; i < DSHOT_BIDIR_MOTOR_LIMIT; i++) {
        dshotBidirMotor_t *motor = &dshotBidirMotors[i];
        if (!motor->configured) {
            continue;
        }

        if (motor->useInterruptCapture) {
            timerChCaptureDisable(motor->tch);
            timerChConfigCallbacks(motor->tch, (timerCallbacks_t *)&dshotBidirCallbacksDisabled);
            continue;
        }

        DMA_Cmd(motor->dma->ref, DISABLE);
        TIM_DMACmd(dshotBidirCtx.timer, motor->dmaSource, DISABLE);
    }
}

static void dshotBidirDecodeCapture(void)
{
    uint32_t readDelta = 0;
    uint32_t invalidDelta = 0;
    uint32_t noEdgeDelta = 0;

    for (uint8_t i = 0; i < DSHOT_BIDIR_MOTOR_LIMIT; i++) {
        dshotBidirMotor_t *motor = &dshotBidirMotors[i];
        if (!motor->configured) {
            rpmSourceInvalidateDshotBidir(i);
            dshotBidirLastEdgeCount[i] = 0;
            dshotBidirLastRawValue[i] = 0;
            dshotBidirLastErpmValue[i] = 0;
            dshotBidirLastRpmValue[i] = 0;
            continue;
        }

        const uint32_t edgeCount = dshotBidirGetCapturedEdgeCount(motor);
        dshotBidirLastEdgeCount[i] = edgeCount;
        if (edgeCount <= DSHOT_BIDIR_MIN_GCR_EDGES) {
            dshotBidirNoEdgeCount++;
            noEdgeDelta++;
            rpmSourceInvalidateDshotBidir(i);
            dshotBidirLastRawValue[i] = 0;
            dshotBidirLastErpmValue[i] = 0;
            dshotBidirLastRpmValue[i] = 0;
            continue;
        }

        dshotBidirReadCount++;
        readDelta++;
        const uint32_t rawValue = dshotBidirDecodeTelemetryPacket(motor->captureBuffer, edgeCount);
        if (rawValue == DSHOT_BIDIR_INVALID) {
            dshotBidirInvalidPacketCount++;
            invalidDelta++;
            rpmSourceInvalidateDshotBidir(i);
            dshotBidirLastRawValue[i] = 0;
            dshotBidirLastErpmValue[i] = 0;
            dshotBidirLastRpmValue[i] = 0;
            continue;
        }

        const uint32_t erpm = dshotBidirDecodeErpmTelemetryValue((uint16_t)rawValue);
        if (erpm == DSHOT_BIDIR_INVALID) {
            dshotBidirInvalidPacketCount++;
            invalidDelta++;
            rpmSourceInvalidateDshotBidir(i);
            dshotBidirLastRawValue[i] = (uint16_t)rawValue;
            dshotBidirLastErpmValue[i] = 0;
            dshotBidirLastRpmValue[i] = 0;
            continue;
        }

        const uint32_t rpm = dshotBidirComputeRpm(erpm);
        dshotBidirLastRawValue[i] = (uint16_t)rawValue;
        dshotBidirLastErpmValue[i] = erpm;
        dshotBidirLastRpmValue[i] = rpm;
        rpmSourceSetDshotBidirRpm(i, rpm);
    }

    dshotBidirUpdateDebug(readDelta, invalidDelta, noEdgeDelta, 0);
}

static void dshotBidirBurstDmaHandler(DMA_t descriptor)
{
    if (!DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        return;
    }

    DMA_Cmd(descriptor->ref, DISABLE);
    TIM_DMACmd(dshotBidirCtx.timer, dshotBidirCtx.burstTch->timCtx->DMASource, DISABLE);
    DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);

    if (dshotBidirCtx.state == DSHOT_BIDIR_STATE_OUTPUT_PENDING) {
        dshotBidirStartInputCapture();
    }
}

void dshotBidirInit(uint32_t dshotHz)
{
    memset(&dshotBidirCtx, 0, sizeof(dshotBidirCtx));
    memset(dshotBidirMotors, 0, sizeof(dshotBidirMotors));
    memset(dshotBidirLastEdgeCount, 0, sizeof(dshotBidirLastEdgeCount));
    memset(dshotBidirLastRawValue, 0, sizeof(dshotBidirLastRawValue));
    memset(dshotBidirLastErpmValue, 0, sizeof(dshotBidirLastErpmValue));
    memset(dshotBidirLastRpmValue, 0, sizeof(dshotBidirLastRpmValue));
    dshotBidirReadCount = 0;
    dshotBidirInvalidPacketCount = 0;
    dshotBidirNoEdgeCount = 0;
    dshotBidirTimeoutCount = 0;

    dshotBidirCtx.captureWindowUs = DSHOT_BIDIR_CAPTURE_DEADTIME_US +
        (1000000U * (DSHOT_BIDIR_TIMER_BIT_COUNT * DSHOT_BIDIR_TIMER_BIT_LENGTH)) / dshotHz;
    dshotBidirCtx.state = DSHOT_BIDIR_STATE_READY;
    dshotBidirUpdateDebug(0, 0, 0, 0);
}

bool dshotBidirAttachMotor(uint8_t motorIndex, TCH_t *tch, void *dmaBurstBuffer)
{
    if (motorIndex >= DSHOT_BIDIR_MOTOR_LIMIT || !tch || tch->timHw->tim != TIM4) {
        return false;
    }

    dshotBidirMotor_t *motor = &dshotBidirMotors[motorIndex];

    motor->tch = tch;
    motor->dmaSource = dshotBidirTimerDmaSource(tch->timHw->channelIndex);
    motor->dma = (tch->timHw->dmaTag != DMA_NONE) ? dmaGetByTag(tch->timHw->dmaTag) : NULL;
    motor->useInterruptCapture = (motor->dma == NULL);
    motor->irqCallbacks.callbackEdge = dshotBidirIrqCaptureCallback;

    if (!motor->useInterruptCapture) {
        if (motor->dma != tch->timCtx->dmaBurstRef) {
            if (dmaGetOwner(motor->dma) != OWNER_FREE) {
                return false;
            }

            dmaInit(motor->dma, OWNER_TIMER, motorIndex);
        }

        DMA_StructInit(&motor->inputDmaInit);
        motor->inputDmaInit.DMA_Channel = dmaGetChannelByTag(tch->timHw->dmaTag);
        motor->inputDmaInit.DMA_PeripheralBaseAddr = (uint32_t)timerCCR(tch);
        motor->inputDmaInit.DMA_Memory0BaseAddr = (uint32_t)motor->captureBuffer;
        motor->inputDmaInit.DMA_DIR = DMA_DIR_PeripheralToMemory;
        motor->inputDmaInit.DMA_BufferSize = DSHOT_BIDIR_CAPTURE_BUFFER_LEN;
        motor->inputDmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        motor->inputDmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
        motor->inputDmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
        motor->inputDmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
        motor->inputDmaInit.DMA_Mode = DMA_Mode_Normal;
        motor->inputDmaInit.DMA_Priority = DMA_Priority_High;
    }

    if (!dshotBidirCtx.timer) {
        dshotBidirCtx.timer = tch->timHw->tim;
    }

    if (!dshotBidirCtx.burstDma && tch->timCtx->dmaBurstRef && dmaBurstBuffer && motor->dma == tch->timCtx->dmaBurstRef) {
        dshotBidirCtx.burstDma = tch->timCtx->dmaBurstRef;
        dshotBidirCtx.burstTch = tch;
        dshotBidirCtx.dmaBurstBuffer = (timerDMASafeType_t *)dmaBurstBuffer;

        DMA_StructInit(&dshotBidirCtx.burstOutputDmaInit);
        dshotBidirCtx.burstOutputDmaInit.DMA_Channel = dmaGetChannelByTag(tch->timHw->dmaTag);
        dshotBidirCtx.burstOutputDmaInit.DMA_PeripheralBaseAddr = (uint32_t)&tch->timHw->tim->DMAR;
        dshotBidirCtx.burstOutputDmaInit.DMA_Memory0BaseAddr = (uint32_t)dmaBurstBuffer;
        dshotBidirCtx.burstOutputDmaInit.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        dshotBidirCtx.burstOutputDmaInit.DMA_BufferSize = DSHOT_BIDIR_BURST_LENGTH;
        dshotBidirCtx.burstOutputDmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        dshotBidirCtx.burstOutputDmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
        dshotBidirCtx.burstOutputDmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
        dshotBidirCtx.burstOutputDmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
        dshotBidirCtx.burstOutputDmaInit.DMA_Mode = DMA_Mode_Normal;
        dshotBidirCtx.burstOutputDmaInit.DMA_Priority = DMA_Priority_High;

        dmaSetHandler(dshotBidirCtx.burstDma, dshotBidirBurstDmaHandler, NVIC_PRIO_TIMER_DMA, 0);
        dshotBidirCtx.burstHandlerInstalled = true;
    }

    motor->configured = true;
    dshotBidirConfigureOutputIo(motor);
    dshotBidirConfigureOutputChannel(motor, 0);
    timerPWMStart(motor->tch);
    return dshotBidirCtx.burstHandlerInstalled;
}

bool dshotBidirUpdate(void)
{
    const timeUs_t now = micros();

    if (dshotBidirCtx.state == DSHOT_BIDIR_STATE_OUTPUT_PENDING) {
        if (cmpTimeUs(now, dshotBidirCtx.frameStartedUs) < (timeDelta_t)dshotBidirCtx.captureWindowUs) {
            return false;
        }

        dshotBidirTimeoutCount++;
        dshotBidirInvalidateAll();
        dshotBidirUpdateDebug(0, 0, 0, 1);
        dshotBidirRestoreReadyState();
        return true;
    }

    if (dshotBidirCtx.state != DSHOT_BIDIR_STATE_CAPTURING) {
        return true;
    }

    if (cmpTimeUs(now, dshotBidirCtx.captureStartedUs) < (timeDelta_t)dshotBidirCtx.captureWindowUs) {
        return false;
    }

    dshotBidirStopInputCapture();
    dshotBidirDecodeCapture();
    dshotBidirRestoreReadyState();
    return true;
}

void dshotBidirOnFrameStarted(void)
{
    dshotBidirCtx.frameStartedUs = micros();
    dshotBidirCtx.state = DSHOT_BIDIR_STATE_OUTPUT_PENDING;
}

uint32_t dshotBidirGetReadCount(void)
{
    return dshotBidirReadCount;
}

uint32_t dshotBidirGetInvalidPacketCount(void)
{
    return dshotBidirInvalidPacketCount;
}

uint32_t dshotBidirGetNoEdgeCount(void)
{
    return dshotBidirNoEdgeCount;
}

uint32_t dshotBidirGetTimeoutCount(void)
{
    return dshotBidirTimeoutCount;
}

uint8_t dshotBidirGetLastEdgeCount(uint8_t motorIndex)
{
    if (motorIndex >= DSHOT_BIDIR_MOTOR_LIMIT) {
        return 0;
    }

    return dshotBidirLastEdgeCount[motorIndex];
}

uint16_t dshotBidirGetLastRawValue(uint8_t motorIndex)
{
    if (motorIndex >= DSHOT_BIDIR_MOTOR_LIMIT) {
        return 0;
    }

    return dshotBidirLastRawValue[motorIndex];
}

uint32_t dshotBidirGetLastErpmValue(uint8_t motorIndex)
{
    if (motorIndex >= DSHOT_BIDIR_MOTOR_LIMIT) {
        return 0;
    }

    return dshotBidirLastErpmValue[motorIndex];
}

uint32_t dshotBidirGetLastRpmValue(uint8_t motorIndex)
{
    if (motorIndex >= DSHOT_BIDIR_MOTOR_LIMIT) {
        return 0;
    }

    return dshotBidirLastRpmValue[motorIndex];
}

#else

void dshotBidirInit(uint32_t dshotHz)
{
    UNUSED(dshotHz);
}

bool dshotBidirAttachMotor(uint8_t motorIndex, TCH_t *tch, void *dmaBurstBuffer)
{
    UNUSED(motorIndex);
    UNUSED(tch);
    UNUSED(dmaBurstBuffer);
    return false;
}

bool dshotBidirUpdate(void)
{
    return true;
}

void dshotBidirOnFrameStarted(void)
{
}

uint32_t dshotBidirGetReadCount(void)
{
    return 0;
}

uint32_t dshotBidirGetInvalidPacketCount(void)
{
    return 0;
}

uint32_t dshotBidirGetNoEdgeCount(void)
{
    return 0;
}

uint32_t dshotBidirGetTimeoutCount(void)
{
    return 0;
}

uint8_t dshotBidirGetLastEdgeCount(uint8_t motorIndex)
{
    UNUSED(motorIndex);
    return 0;
}

uint16_t dshotBidirGetLastRawValue(uint8_t motorIndex)
{
    UNUSED(motorIndex);
    return 0;
}

uint32_t dshotBidirGetLastErpmValue(uint8_t motorIndex)
{
    UNUSED(motorIndex);
    return 0;
}

uint32_t dshotBidirGetLastRpmValue(uint8_t motorIndex)
{
    UNUSED(motorIndex);
    return 0;
}

#endif

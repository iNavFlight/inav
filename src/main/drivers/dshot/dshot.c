/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DSHOT: the protocol layer for every DSHOT output (throttle and telemetry request per motor,
 * packet checksum, DSHOT commands, frame rate) and the output driver for the motor timer
 * channels. Each channel sends its frame by DMA into its CCR, one slot per bit; with
 * USE_DSHOT_DMAR one burst stream per timer feeds all of its channels. For bidirectional
 * DSHOT (USE_DSHOT_BIDIR_TIMER) the channel is turned round to input capture after the frame
 * to time the edges of the ESC's GCR reply and back to output before the next frame; the
 * decoded values go to bidir_dshot.c. On AT32 bidirectional DSHOT is bitbanged instead
 * (dshot_bitbang.c), this module then only prepares the packets.
 *
 * The motor driver (pwm_output.c) hands the DSHOT protocols to this module: dshotInit() once,
 * dshotMotorConfig() per motor, dshotCompleteMotorUpdate() every loop, and the ESC passthrough
 * and flash-write hooks.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_DSHOT) && !defined(SITL_BUILD) && !defined(RP2350)

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/circular_queue.h"
#include "common/log.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/dma.h"
#include "drivers/dshot/bidir_dshot.h"
#include "drivers/dshot/dshot.h"
#include "drivers/dshot/dshot_bitbang.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/nvic.h"
#include "drivers/pwm_mapping.h"
#include "drivers/pwm_output.h"
#include "drivers/resource.h"
#include "drivers/time.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"

#include "flight/mixer.h"

#include "sensors/esc_sensor.h"

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

#ifdef USE_DSHOT_BIDIR_TIMER
/* Telemetry reply window, measured from the frame start: the 18-slot frame, then the ESC's
 * ~30 us turnaround and its 21-bit GCR reply at 5/4 of the DSHOT bit rate (17 slots).
 * DSHOT_TELEMETRY_DEADTIME_US covers the turnaround plus margin for ESC timing variation. */
#define DSHOT_TELEMETRY_DEADTIME_US     45
#define DSHOT_TELEMETRY_WINDOW_SLOTS    (DSHOT_DMA_BUFFER_SIZE + 17)
#define GCR_TELEMETRY_INPUT_LEN MAX_GCR_EDGES
/* A bidir motor captures the GCR edges of the ESC reply into the same buffer it sends the
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
#define DSHOT_COMMAND_QUEUE_SIZE   (DSHOT_COMMAND_QUEUE_LENGTH * sizeof(dshotCommands_e))

typedef struct {
    // Set by the mixer, sent with the next frame
    uint16_t value;
    bool requestTelemetry;

    // Timer output of the motor: tch is NULL while it has none (bitbanged, or the channel
    // was taken), configured once its DMA is set up and frames go out on it
    TCH_t *tch;
    volatile timCCR_t *ccr;         // Shortcut for timer CCR register
    bool configured;
    timerDMASafeType_t dmaBuffer[DSHOT_PORT_DMA_BUFFER_SIZE];
#ifdef USE_DSHOT_DMAR
    timerDMASafeType_t *dmaBurstBuffer;
#endif
#ifdef USE_DSHOT_BIDIR_TIMER
    bool telemetryInputActive;
#endif
} dshotMotor_t;

typedef struct {
    dshotCommands_e cmd;
    int remainingRepeats;
} currentExecutingCommand_t;

// The frame (and capture) buffers are DMA targets; DMA_RAM is not cleared at startup, dshotInit() does
static DMA_RAM dshotMotor_t motors[MAX_MOTORS];

#ifdef USE_DSHOT_DMAR
static timerDMASafeType_t dmaBurstBuffer[MAX_DMA_TIMERS][DSHOT_DMA_BUFFER_SIZE * 4];
static burstDmaTimer_t burstDmaTimers[MAX_DMA_TIMERS];
static uint8_t burstDmaTimersCount = 0;
#endif

// Every motor replays the same zero-throttle keep-alive frame, so one buffer feeds all DMA streams
#ifdef USE_DSHOT_DMAR
static DMA_RAM timerDMASafeType_t dshotKeepaliveBuffer[DSHOT_KEEPALIVE_BUFFER_SIZE * 4];
#else
static DMA_RAM timerDMASafeType_t dshotKeepaliveBuffer[DSHOT_KEEPALIVE_BUFFER_SIZE];
#endif

static uint32_t dshotHz = 0;                // Timer clock of the configured rate, DSHOT_MOTOR_BITLENGTH ticks per bit
static timeUs_t dshotUpdateIntervalUs = 0;
static timeUs_t dshotLastUpdateUs;
static timeUs_t lastCommandSent = 0;
static timeUs_t commandPostDelay = 0;
#ifdef USE_DSHOT_BIDIR_TIMER
static bool dshotTelemetryPending = false;
/* One stamp for the whole frame, taken in the main loop just before the frames start, so
 * the completion ISR has no clock to read. Turning a motor back to output resets the
 * counter and period of its timer, which every other motor on that timer shares, so no
 * motor may switch back while a sibling could still be capturing: the reply window is
 * therefore waited out once, for all motors, before any of them is touched */
static timeUs_t dshotTelemetryFrameStampUs = 0;
static timeUs_t dshotTelemetryDeadtimeUs = 0;
// DEBUG_ESC: cycle counter at the frame start, debug[motor] = cycles until its capture is armed
static uint32_t dshotFrameStartCycles = 0;
// Set while bidir pins are plain GPIO, disconnected from their timer: parked low until
// the first frame, or held by the pull only during a flash write. The next frame
// reconnects them (dshotConnectOutputs()).
static bool dshotPinsDisconnected = false;
#endif

static circularBuffer_t commandsCircularBuffer;
static uint8_t commandsBuff[DSHOT_COMMAND_QUEUE_SIZE];
static currentExecutingCommand_t currentExecutingCommand;

static uint32_t getDshotHz(motorPwmProtocolTypes_e pwmProtocolType)
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
#else
static void loadDmaBufferDshot(timerDMASafeType_t *dmaBuffer, uint16_t packet)
{
    for (int i = 0; i < 16; i++) {
        dmaBuffer[i] = (packet & 0x8000) ? DSHOT_MOTOR_BIT_1 : DSHOT_MOTOR_BIT_0;  // MSB first
        packet <<= 1;
    }
    // Frame reset slots: rewritten every time, a bidir motor captures edges into this buffer
    dmaBuffer[16] = 0;
    dmaBuffer[17] = 0;
}
#endif

#ifdef USE_DSHOT_BIDIR_TIMER
// The CC1..CC4 DMA request bits sit next to each other in DIER on every supported MCU
static uint32_t dshotDmaSource(const dshotMotor_t *motor)
{
    STATIC_ASSERT(TIM_DMA_CC2 == TIM_DMA_CC1 << 1 && TIM_DMA_CC4 == TIM_DMA_CC1 << 3, tim_dma_cc_bits_adjacent);
    return TIM_DMA_CC1 << motor->tch->timHw->channelIndex;
}

// Per-channel DMA request bit in DIER; the LL and SPL drivers only offer fixed-channel variants
static inline void dshotDmaReqEnable(TIM_TypeDef *tim, uint32_t dmaSource)
{
    SET_BIT(tim->DIER, dmaSource);
}

static inline void dshotDmaReqDisable(TIM_TypeDef *tim, uint32_t dmaSource)
{
    CLEAR_BIT(tim->DIER, dmaSource);
}

// Register slices of this motor's timer channel for the direction switch: the channel's
// byte in CCMR1/CCMR2 and its nibble in CCER. Derived on the fly, the shifts cost a few cycles.
typedef struct {
    __typeof__(((TIM_TypeDef *)0)->CCMR1) *ccmr;   // CCMRx is 16 bits wide on F4, 32 on F7/H7
    uint32_t ccmrShift;
    uint32_t ccmrMask;      // the channel's byte plus its OCxM[3] bit
    uint32_t ccerShift;
    uint32_t ccerMask;
} dshotChannelRegs_t;

static dshotChannelRegs_t dshotChannelRegs(const dshotMotor_t *motor)
{
    const uint32_t channelIndex = motor->tch->timHw->channelIndex;
    TIM_TypeDef *tim = motor->tch->timHw->tim;
    dshotChannelRegs_t regs;
    regs.ccmr = (channelIndex < 2) ? &tim->CCMR1 : &tim->CCMR2;
    regs.ccmrShift = 8 * (channelIndex & 1);
    regs.ccmrMask = 0xFF << regs.ccmrShift;
#ifdef TIM_CCMR1_OC1M_3
    // F7/H7 keep a fourth OCxM bit outside the channel byte; F4 has none
    regs.ccmrMask |= TIM_CCMR1_OC1M_3 << regs.ccmrShift;
#endif
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

// Edge timestamps to GCR bits, ported from Betaflight (GPLv3); the symbols are decoded in bidir_dshot.c
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

    return dshotDecodeGcr(value);
}

static void dshotSetDirectionOutput(dshotMotor_t *motor)
{
    // Bidirectional DSHOT is carried on inverted (idle-high) signalling - flip the
    // channel's normal polarity whenever telemetry is active, matching Betaflight.
    const bool inverted = ((motor->tch->timHw->output & TIMER_OUTPUT_INVERTED) != 0) ^ useDshotTelemetry;

    // Telemetry input capture widens ARR to 0xffff (see dshotSetDirectionInput);
    // restore the DSHOT bit period now, bypassing the ARR preload shadow so it
    // takes effect immediately instead of after the next update event.
    TIM_TypeDef *tim = motor->tch->timHw->tim;
    CLEAR_BIT(tim->CR1, TIM_CR1_ARPE);
    tim->ARR = DSHOT_MOTOR_BITLENGTH - 1;
    SET_BIT(tim->CR1, TIM_CR1_ARPE);
    tim->CNT = 0;

    // Channel back to PWM1 output. CCxS is only writable with the channel off, and CCR is
    // zeroed before OCxPE goes back on so the stale capture value can't give one bit period
    // of output before the first update event. Idle state (CR2 OISx) is untouched.
    const dshotChannelRegs_t regs = dshotChannelRegs(motor);
    CLEAR_BIT(tim->CCER, regs.ccerMask);
    MODIFY_REG(*regs.ccmr, regs.ccmrMask, DSHOT_CCMR_OUTPUT << regs.ccmrShift);
    *motor->ccr = 0;
    SET_BIT(*regs.ccmr, TIM_CCMR1_OC1PE << regs.ccmrShift);
    uint32_t ccer;
    if (motor->tch->timHw->output & TIMER_OUTPUT_N_CHANNEL) {
        ccer = TIM_CCER_CC1NE | (inverted ? TIM_CCER_CC1NP : 0);
    } else {
        ccer = TIM_CCER_CC1E | (inverted ? TIM_CCER_CC1P : 0);
    }
    MODIFY_REG(tim->CCER, regs.ccerMask, ccer << regs.ccerShift);

    // Stream back to memory-to-peripheral; timerPWMPrepareDMA() sets count and addresses and
    // enables it for the next frame. The capture may still be draining (the decode loop only
    // cleared EN), so wait for EN to read 0 before touching CR, then clear all five event
    // flags: EN=1 is ignored while any of them is set (RM0433).
    DMA_Stream_TypeDef *stream = motor->tch->dma->ref;
    for (uint32_t timeout = 10000; timeout && (stream->CR & DMA_SxCR_EN); timeout--) { }
    DMA_CLEAR_FLAG(motor->tch->dma, DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_TEIF | DMA_IT_DMEIF | DMA_IT_FEIF);
    stream->CR = (stream->CR & ~(DMA_SxCR_DIR | DMA_SxCR_EN)) | DMA_SxCR_DIR_0;
    motor->telemetryInputActive = false;
}

FAST_CODE static void dshotSetDirectionInput(dshotMotor_t *motor)
{
    // Runs in the DMA completion ISR for every motor in turn, so it only writes registers:
    // no HAL/LL init calls, and the stream is reconfigured in place (hardware clears its EN
    // bit at end of transfer, the wait below only covers the flag/EN ordering).
    // motors[] lives in non-cacheable DMA_RAM, so read tch once.
    TCH_t *tch = motor->tch;
    TIM_TypeDef *tim = tch->timHw->tim;
    const dshotChannelRegs_t regs = dshotChannelRegs(motor);

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
    dshotDmaReqEnable(tim, dshotDmaSource(motor));
    motor->telemetryInputActive = true;
}

// Connect bidir DSHOT pins to their timer AF (idle-high). Called on the first
// real motor update so the line only goes high microseconds before frames start,
// keeping it low through the ESC's boot-time bootloader-entry window.
static void dshotConnectOutputs(void)
{
    for (int index = 0; index < getMotorCount(); index++) {
        const dshotMotor_t *motor = &motors[index];
        if (motor->configured) {
            const timerHardware_t *timHw = motor->tch->timHw;
            const IO_t io = IOGetByTag(timHw->tag);
            // The pull must match this channel's *un-flipped* polarity
            // (dshotSetDirectionOutput flips OCPolarity for bidir, but the GCR
            // listen phase still idles toward the same physical level either way).
            const bool baseInverted = timHw->output & TIMER_OUTPUT_INVERTED;
#if defined(STM32H7)
            // Preset ODR to the bidir idle level once - the H7 OC->IC erratum workaround
            // in dshotSetDirectionInput() disconnects the pin from the timer and relies
            // on this cached ODR bit (matches Betaflight) instead of redriving the level.
            baseInverted ? IOLo(io) : IOHi(io);
#endif
            IOConfigGPIOAF(io, baseInverted ? IOCFG_AF_PP_PD : IOCFG_AF_PP_UP, timHw->alternateFunction);
        }
    }
}

// Takes over the stream's completion interrupt from the timer driver on bidir motors: once
// the frame is out the channel is turned round to capture the ESC reply, and
// dshotDecodeTelemetry() turns it back before the next frame. Runs once per motor per
// frame, back to back, so it lives in ITCM where available.
FAST_CODE static void dshotDmaIrqHandler(DMA_t descriptor)
{
    if (!DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        return;
    }

    // userParam is the motor index (see dshotTimerMotorConfig())
    dshotMotor_t *motor = &motors[descriptor->userParam];

    // The stream disabled itself at end of transfer (normal mode). The DMA request must go
    // too: CCR is 0 after the frame, so compare matches keep a request pending and the stream
    // would serve it as a bogus first capture the moment it is re-enabled for input.
    dshotDmaReqDisable(motor->tch->timHw->tim, dshotDmaSource(motor));

    // Same bookkeeping as the timer driver's handler (timerPWMDMAInProgress())
    if (motor->tch->dmaState == TCH_DMA_ACTIVE) {
        motor->tch->dmaState = TCH_DMA_IDLE;
    }

    if (!motor->telemetryInputActive) {
        dshotSetDirectionInput(motor);
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
static bool NOINLINE dshotDecodeTelemetry(void)
{
    if (!dshotTelemetryPending) {
        return true;
    }

    // Wait out the reply window of the frame before switching any motor back (see
    // dshotTelemetryFrameStampUs)
    if ((micros() - dshotTelemetryFrameStampUs) < dshotTelemetryDeadtimeUs) {
        return false;
    }

    // Cleared before the loop: a completion IRQ landing while the loop runs re-arms it, so a
    // motor the loop has already passed is picked up by the next call instead of being left
    // in input for the following frame
    dshotTelemetryPending = false;

    for (int motorIndex = 0; motorIndex < getMotorCount(); motorIndex++) {
        dshotMotor_t *motor = &motors[motorIndex];
        if (!motor->configured || !motor->telemetryInputActive) {
            continue;
        }

        uint32_t edges = 0;
        edges = GCR_TELEMETRY_INPUT_LEN - motor->tch->dma->ref->NDTR;
        dshotDmaReqDisable(motor->tch->timHw->tim, dshotDmaSource(motor));
        CLEAR_BIT(motor->tch->dma->ref->CR, DMA_SxCR_EN);

        // Too few edges is no reply at all
        uint16_t rawValue = DSHOT_TELEMETRY_NOEDGE;
        if (edges > MIN_GCR_EDGES) {
            // No cache maintenance: motors[] is DMA_RAM, which the H7 MPU maps non-cacheable
            rawValue = dshotDecodeTelemetryPacket((const uint32_t *)motor->dmaBuffer, edges);
        }

        const uint16_t processed = dshotProcessPacket(rawValue, motorIndex);
        if (processed != DSHOT_TELEMETRY_INVALID && processed != DSHOT_TELEMETRY_NOEDGE) {
#ifdef USE_ESC_SENSOR
            // Nothing to publish without a fresh eRPM value (an EDT frame may come first, or outlive it)
            escSensorData_t data;
            if (getDshotEscSensorData(&data, motorIndex)) {
                escSensorSetDshotData(motorIndex, data.rpm, data.temperature, data.voltage, data.current);
            }
#endif
        }

        dshotSetDirectionOutput(motor);
    }

    // Re-armed during the loop: some motor is in input again, hold the frame until it is back
    return !dshotTelemetryPending;
}
#endif // USE_DSHOT_BIDIR_TIMER

// Timer output of one motor: claims the output's timer channel and pin, as pwmOutConfig()
// does for the PWM protocols, and sets up the DMA that sends the frames. True once the motor
// has its channel; without DMA it stays unconfigured and sends nothing.
static bool dshotTimerMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, bool enableOutput)
{
    dshotMotor_t *motor = &motors[motorIndex];

    TCH_t *tch = timerGetTCH(timerHardware);
    if (tch == NULL) {
        return false;
    }

    const IO_t io = IOGetByTag(timerHardware->tag);
    IOInit(io, OWNER_MOTOR, RESOURCE_OUTPUT, RESOURCE_INDEX(motorIndex));

    motor->tch = tch;
    motor->configured = false;
    timerConfigBase(tch, DSHOT_MOTOR_BITLENGTH, dshotHz);
    timerPWMConfigChannel(tch, 0);
    timerPWMStart(tch);
    timerEnable(tch);
    motor->ccr = timerCCR(tch);
    *motor->ccr = 0;

    if (enableOutput) {
        IOConfigGPIOAF(io, IOCFG_AF_PP, timerHardware->alternateFunction);
    } else {
        // If PWM outputs are disabled - configure as GPIO and drive low
        IOConfigGPIO(io, IOCFG_OUT_OD);
        IOLo(io);
    }

#ifdef USE_DSHOT_BIDIR_TIMER
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
        IOConfigGPIO(io, IOCFG_OUT_PP);
        (timerHardware->output & TIMER_OUTPUT_INVERTED) ? IOHi(io) : IOLo(io);
        dshotPinsDisconnected = true;
    }
#endif

    // Configure timer DMA
#ifdef USE_DSHOT_DMAR
    uint8_t burstDmaTimerIndex = getBurstDmaTimerIndex(timerHardware->tim);
    if (burstDmaTimerIndex >= MAX_DMA_TIMERS) {
        motor->tch = NULL;
        return false;
    }

    motor->dmaBurstBuffer = &dmaBurstBuffer[burstDmaTimerIndex][0];
    burstDmaTimer_t *burstDmaTimer = &burstDmaTimers[burstDmaTimerIndex];
    burstDmaTimer->dmaBurstBuffer = motor->dmaBurstBuffer;

    if (timerPWMConfigDMABurst(burstDmaTimer, motor->tch, motor->dmaBurstBuffer, sizeof(motor->dmaBurstBuffer[0]), DSHOT_DMA_BUFFER_SIZE)) {
        motor->configured = true;
    }
#else
    if (timerPWMConfigChannelDMA(motor->tch, motor->dmaBuffer, sizeof(motor->dmaBuffer[0]), DSHOT_DMA_BUFFER_SIZE)) {
        // Only mark as DSHOT channel if DMA was set successfully
        ZERO_FARRAY(motor->dmaBuffer);
        motor->configured = true;
#ifdef USE_DSHOT_BIDIR_TIMER
        if (useDshotTelemetry) {
            // Inverted signalling and per-frame direction switching; the motor takes the
            // stream's completion interrupt over from the timer driver
            dshotTelemetryDeadtimeUs = DSHOT_TELEMETRY_DEADTIME_US + 1000000 * (DSHOT_TELEMETRY_WINDOW_SLOTS * DSHOT_MOTOR_BITLENGTH) / dshotHz;
#if defined(USE_HAL_DRIVER)
            // FIFO threshold 1/4 for both directions (the F7/H7 timer driver set FULL), so
            // captured edges reach memory one word at a time and the decode never finds a
            // partial reply still sitting in the FIFO. Set once here; the direction switch
            // leaves FCR alone. The F4 driver runs the stream in direct mode, which already
            // writes every word straight away.
            motor->tch->dma->ref->FCR = DMA_SxFCR_DMDIS;
#endif
            dshotSetDirectionOutput(motor);
            // The handler gets the motor index: it reaches the motor through motors[] and uses
            // the index straight as the debug slot
            dmaSetHandler(motor->tch->dma, dshotDmaIrqHandler, NVIC_PRIO_TIMER_DMA, motorIndex);
        }
#endif
    }
#endif

    return true;
}

void dshotInit(motorPwmProtocolTypes_e protocol, uint32_t updateRateHz)
{
    ZERO_FARRAY(motors);

    dshotHz = getDshotHz(protocol);
    dshotUpdateIntervalUs = 1000000 / updateRateHz;
    dshotLastUpdateUs = 0;

#ifdef USE_DSHOT_BIDIR
    useDshotTelemetry = motorConfig()->useDshotTelemetry;
#endif
#ifdef USE_DSHOT_BITBANG
    // Bidirectional DSHOT is bitbanged on this MCU (dshot_bitbang.c); plain DSHOT
    // stays on the timers
    if (useDshotTelemetry) {
        dshotBitbangInit(dshotHz / DSHOT_MOTOR_BITLENGTH);
    }
#endif
}

void dshotWriteMotor(uint8_t index, uint16_t value)
{
    // Just keep track of motor value, actual update happens in dshotCompleteMotorUpdate()
    // DSHOT and some other digital protocols use 11-bit throttle range [0;2047]
    motors[index].value = constrain(value, 0, 2047);
}

// A motor with an output behind it: a timer channel, or a bitbanged pin
static bool isMotorOutputConfigured(int index)
{
#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive()) {
        return dshotBitbangMotorIsConfigured(index);
    }
#endif
    return motors[index].configured;
}

void dshotRequestTelemetry(uint8_t motorIndex)
{
    const int motorCount = getMotorCount();
    for (int index = 0; index < motorCount; index++) {
        if (isMotorOutputConfigured(index) && index == motorIndex) {
            motors[index].requestTelemetry = true;
        }
    }
}

void sendDShotCommand(dshotCommands_e cmd)
{
    circularBufferPushElement(&commandsCircularBuffer, (uint8_t *) &cmd);
}

void initDShotCommands(void)
{
    circularBufferInit(&commandsCircularBuffer, commandsBuff, DSHOT_COMMAND_QUEUE_SIZE, sizeof(dshotCommands_e));

    currentExecutingCommand.remainingRepeats = 0;
}

static int getDShotCommandRepeats(dshotCommands_e cmd)
{
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

// NOINLINE: dshotCompleteMotorUpdate() is inlined into the ITCM scheduler; commands are rare
static bool NOINLINE executeDShotCommands(void)
{
    timeUs_t tNow = micros();

    if (currentExecutingCommand.remainingRepeats == 0) {
        const int isTherePendingCommands = !circularBufferIsEmpty(&commandsCircularBuffer);
        if (isTherePendingCommands && (tNow - lastCommandSent > DSHOT_COMMAND_INTERVAL_US)) {
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

void dshotCompleteMotorUpdate(bool motorsEnabled)
{
#ifdef USE_DSHOT_BIDIR_TIMER
    if (useDshotTelemetry && !dshotDecodeTelemetry()) {
        return;
    }
#endif
#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive() && !dshotBitbangDecodeTelemetry()) {
        return;
    }
#endif

    const int motorCount = getMotorCount();
    const timeUs_t currentTimeUs = micros();

#ifdef USE_DSHOT_BIDIR
    if (useDshotTelemetry) {
        escSensorFrameWindowUpdate(currentTimeUs / 1000);
    }
#endif

    // Enforce motor update rate
    if ((dshotUpdateIntervalUs == 0) || ((currentTimeUs - dshotLastUpdateUs) <= dshotUpdateIntervalUs)) {
        return;
    }

    dshotLastUpdateUs = currentTimeUs;

#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive()) {
        // No frames while the motors are disabled: the 4-way interface owns the pins then
        if (!motorsEnabled || !executeDShotCommands()) {
            return;
        }
        for (int index = 0; index < motorCount; index++) {
            if (dshotBitbangMotorIsConfigured(index)) {
                dshotBitbangWrite(index, prepareDshotPacket(motors[index].value, motors[index].requestTelemetry));
                motors[index].requestTelemetry = false;
            }
        }
        dshotBitbangUpdateComplete();
        return;
    }
#else
    UNUSED(motorsEnabled);
#endif

#ifdef USE_DSHOT_BIDIR_TIMER
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
        if (motors[index].configured) {
            uint16_t packet = prepareDshotPacket(motors[index].value, motors[index].requestTelemetry);
            loadDmaBufferDshotStride(&motors[index].dmaBurstBuffer[motors[index].tch->timHw->channelIndex], 4, packet);
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
        if (motors[index].configured) {
            uint16_t packet = prepareDshotPacket(motors[index].value, motors[index].requestTelemetry);
            loadDmaBufferDshot(motors[index].dmaBuffer, packet);
            timerPWMPrepareDMA(motors[index].tch, DSHOT_DMA_BUFFER_SIZE);
            motors[index].requestTelemetry = false;
        }
    }

#ifdef USE_DSHOT_BIDIR_TIMER
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
        if (motors[index].configured) {
            timerPWMStartDMA(motors[index].tch);
        }
    }
#endif
}

bool dshotMotorConfig(const timerHardware_t *timerHardware, uint8_t motorIndex, bool enableOutput)
{
#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive()) {
        // Only the pin is taken, its timer channel stays free; the pin joins its GPIO
        // port's group in dshotBitbangPostInit(), once every other timer user has chosen
        return dshotBitbangMotorConfig(timerHardware, motorIndex, enableOutput);
    }
#endif
    return dshotTimerMotorConfig(timerHardware, motorIndex, enableOutput);
}

// After every other timer and DMA user has had its pick (fc_init). Bitbang DSHOT needs a
// whole spare timer: without one the motors go back to plain DSHOT on their own timer
// channels and bidirectional telemetry stays off for this boot, rather than blocking arming
// over a feature the airframe can fly without.
bool dshotMotorPostInit(void)
{
#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive() && !dshotBitbangPostInit()) {
        LOG_ERROR(PWM, "Bitbang DSHOT unavailable (status %d), bidirectional DSHOT off", dshotBitbangGetStatus());
        dshotBitbangRelease();
        useDshotTelemetry = false;

        for (int index = 0; index < getMotorCount(); index++) {
            const timerHardware_t *timHw;
            bool enableOutput;
            if (!dshotBitbangGetMotorOutput(index, &timHw, &enableOutput)) {
                continue;
            }
            if (!dshotTimerMotorConfig(timHw, index, enableOutput)) {
                return false;
            }
        }
    }
#endif
    return true;
}

/*
 * Wait (bounded) until the keep-alive stream of this motor is sending its idle padding.
 * CCR holds the slot last written by DMA: a bit length while a data bit is in flight,
 * 0 in the padding. Waiting for the data -> padding transition leaves a full
 * DSHOT_KEEPALIVE_GAP_US before the DMA wraps to the next frame, so the stream can be
 * stopped with the last frame complete and the line low.
 */
static void dshotWaitForKeepalivePadding(const dshotMotor_t *motor)
{
    const timeUs_t start = micros();
    while (*motor->ccr == 0 && (micros() - start) < DSHOT_KEEPALIVE_WAIT_TIMEOUT_US);
    while (*motor->ccr != 0 && (micros() - start) < DSHOT_KEEPALIVE_WAIT_TIMEOUT_US);
}

void dshotSetDMACircular(bool circular)
{
    const int motorCount = getMotorCount();
    const uint32_t keepaliveSlots = DSHOT_KEEPALIVE_SLOTS(dshotHz);
    // A frame started by dshotCompleteMotorUpdate() may still be in flight for this long
    const uint32_t frameUs = DSHOT_DMA_BUFFER_SIZE * DSHOT_MOTOR_BITLENGTH * 1000000UL / dshotHz;

#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive()) {
        // Bitbang pins idle as plain GPIO: before the write finish the exchange in flight
        // and leave the lines to their pulls; the next frame takes the pins back
        if (circular) {
            dshotBitbangDisconnect();
        }
        return;
    }
#endif

#ifdef USE_DSHOT_BIDIR_TIMER
    // No keep-alive frames for bidir: the ESC sees a frame gap for the flash write, as in
    // Betaflight, with the line held at its idle level only by the pull.
    if (useDshotTelemetry) {
        if (circular && !dshotPinsDisconnected) {
            // Complete the DSHOT exchange before the write: let a frame in flight finish,
            // with the deadtime as margin for its completion IRQ, then wait out and decode
            // the reply, which turns every motor back to output
            delayMicroseconds(frameUs + DSHOT_TELEMETRY_DEADTIME_US);
            while (!dshotDecodeTelemetry()) { }

            // Disconnect the pins from the timers so nothing drives the line during the
            // write; the next frame after it reconnects them
            for (int i = 0; i < motorCount; i++) {
                const dshotMotor_t *motor = &motors[i];
                if (motor->configured) {
                    const bool baseInverted = motor->tch->timHw->output & TIMER_OUTPUT_INVERTED;
                    IOConfigGPIO(IOGetByTag(motor->tch->timHw->tag), baseInverted ? IOCFG_IPD : IOCFG_IPU);
                }
            }
            dshotPinsDisconnected = true;
        }
        return;
    }
#endif

    if (circular) {
        // A frame started by dshotCompleteMotorUpdate() may still be in flight: let it finish
        // and keep the line low for one full gap before the keep-alive stream starts
        delayMicroseconds(frameUs + DSHOT_KEEPALIVE_GAP_US);

        // Load a zero-throttle packet into the shared keep-alive buffer. The padding slots
        // must be zero (line low between frames); DMA_RAM is NOLOAD and not cleared at
        // startup, so clear it explicitly.
        uint16_t packet = prepareDshotPacket(0, false);
        ZERO_FARRAY(dshotKeepaliveBuffer);
#ifdef USE_DSHOT_DMAR
        for (int i = 0; i < motorCount; i++) {
            if (motors[i].configured) {
                loadDmaBufferDshotStride(&dshotKeepaliveBuffer[motors[i].tch->timHw->channelIndex], 4, packet);
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
            if (motors[m].configured && motors[m].tch->timHw->tim == burstDmaTimer->timer) {
                if (circular) {
                    impl_pwmBurstDMASetCircular(burstDmaTimer, motors[m].tch, true, dshotKeepaliveBuffer, keepaliveSlots * 4);
                } else {
                    // Atomic so no ISR can delay the stop past the padding into the next frame
                    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
                        dshotWaitForKeepalivePadding(&motors[m]);
                        impl_pwmBurstDMASetCircular(burstDmaTimer, motors[m].tch, false, burstDmaTimer->dmaBurstBuffer, DSHOT_DMA_BUFFER_SIZE * 4);
                    }
                }
                break;
            }
        }
    }
#else
    // Per-channel DMA: one DMA stream per motor
    for (int i = 0; i < motorCount; i++) {
        if (motors[i].configured) {
            if (circular) {
                impl_timerPWMSetDMACircular(motors[i].tch, true, dshotKeepaliveBuffer, keepaliveSlots);
            } else {
                // Atomic so no ISR can delay the stop past the padding into the next frame
                ATOMIC_BLOCK(NVIC_PRIO_MAX) {
                    dshotWaitForKeepalivePadding(&motors[i]);
                    impl_timerPWMSetDMACircular(motors[i].tch, false, motors[i].dmaBuffer, DSHOT_DMA_BUFFER_SIZE);
                }
            }
        }
    }
#endif

    if (!circular) {
        // The streams were stopped while sending padding, so CCR is already 0 and the lines
        // stay low until dshotCompleteMotorUpdate() sends the next frame. Enforce it in case a
        // wait above timed out; the timer would otherwise keep repeating the last bit.
        for (int i = 0; i < motorCount; i++) {
            if (motors[i].configured) {
                *motors[i].ccr = 0;
            }
        }
    }
}

void dshotStop(void)
{
#ifdef USE_DSHOT_BITBANG
    // The 4-way interface takes the pins next: finish the frame and reply in flight first
    if (dshotBitbangIsActive()) {
        dshotBitbangStop();
    }
#endif
}

// Helper function for ESC passthrough
ioTag_t dshotGetMotorPinTag(uint8_t motorIndex)
{
#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive()) {
        return dshotBitbangGetMotorPinTag(motorIndex);
    }
#endif
    if (motors[motorIndex].tch) {
        return motors[motorIndex].tch->timHw->tag;
    }
    return IOTAG_NONE;
}

// Helper function for ESC passthrough: hand the pin back to its timer after the 4-way
// interface drove it as plain GPIO. The alternate function has to be given explicitly:
// on HAL targets a bare IOConfigGPIO(IOCFG_AF_PP) programs AF0 and the timer loses the pin.
void dshotRestoreMotorPin(uint8_t motorIndex)
{
#ifdef USE_DSHOT_BITBANG
    if (dshotBitbangIsActive()) {
        dshotBitbangRestoreMotorPin(motorIndex);
        return;
    }
#endif
    if (motors[motorIndex].tch) {
        const timerHardware_t *timHw = motors[motorIndex].tch->timHw;
        IOConfigGPIOAF(IOGetByTag(timHw->tag), IOCFG_AF_PP, timHw->alternateFunction);
    }
}

#endif // USE_DSHOT && !SITL_BUILD && !RP2350

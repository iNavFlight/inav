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
 *
 * Ported from Betaflight's dshot_bitbang.c and dshot_bitbang_stdperiph.c (AT32), GPLv3.
 */

/*
 * Bitbanged bidirectional DSHOT.
 *
 * The motor pins are plain GPIO outputs. For every GPIO port that carries motors there is
 * one DMA channel writing 32-bit words into the port's set/clear register (scr), one word
 * per third of a DSHOT bit: all pins to the active level, the pins sending a 0 back to idle,
 * every pin back to idle. The words are paced by the compare event of a spare TMR1/TMR8
 * channel, so one timer channel and one DMA channel serve every motor on the port and the
 * motor outputs' own timer channels are not used at all.
 *
 * After the frame the port's pins are switched to inputs and the same DMA channel reads the
 * port's input register, three samples per GCR bit of the ESC reply, into a buffer that
 * dshotBitbangDecode() walks per motor pin once the window has closed.
 *
 * This is the only bidirectional DSHOT on AT32F43x: timer channel 4 cannot capture both
 * edges there (see AT32_BIDIR_DSHOT.md in the repository root). Plain DSHOT keeps using the
 * timer driver in dshot.c, which also owns the frame contents, the DSHOT commands and
 * the update rate; this module only drives the pins and reads the replies.
 *
 * DEBUG_ESC: debug[0] replies decoded, debug[1] windows without a reply, debug[2] replies
 * that failed to decode, debug[3] windows cut short, debug[4] port groups, debug[5] status
 * (dshotBitbangStatus_e).
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_DSHOT_BITBANG

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/log.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/dshot/bidir_dshot.h"
#include "drivers/dma.h"
#include "drivers/dshot/dshot_bitbang.h"
#include "drivers/dshot/dshot_bitbang_decode.h"
#include "drivers/io.h"
#include "drivers/io_impl.h"
#include "drivers/nvic.h"
#include "drivers/pwm_mapping.h"
#include "drivers/rcc.h"
#include "drivers/time.h"
#include "drivers/timer.h"
#include "drivers/timer_impl.h"

#include "flight/mixer.h"

// Pin groups: one per GPIO port that carries motors. Each needs a pacer channel, so the
// limit is the eight channels of TMR8 and TMR1; four cover PA, PB, PC and one more.
#define BB_MAX_PORTS                4
// Pacer timers: TMR8 and TMR1, the advanced timers with four channels each
#define BB_MAX_PACERS               2

#define BB_FRAME_BITS               16
#define BB_STATES_PER_BIT           3   // all pins active, the 0-bit pins back to idle, all pins back to idle
// One more bit period at idle after the frame, so the ESC samples the last bit before the
// pins turn into inputs (the level on a floating line is undefined)
#define BB_HOLD_STATES              3
#define BB_OUTPUT_BUFFER_LENGTH     (BB_FRAME_BITS * BB_STATES_PER_BIT + BB_HOLD_STATES)   // 51 words

// Input window in samples, one sample per pacer period at DSHOT_BITBANG_TELEMETRY_OVERSAMPLE
// times the GCR bit rate: the ESC's ~30 us turnaround, the 21-bit reply and slack.
// Betaflight's figure, sized for DSHOT600 (134 samples) plus six spare.
#define BB_INPUT_BUFFER_LENGTH      140

// Compare value of the pacer channel: the DMA request fires this many counts into each period
#define BB_PACER_COMPARE            10

// Bound on waiting for the DMA channel enable bit to drop after clearing it: the hardware
// only clears it once the transfer in flight is done, a handful of bus cycles
#define BB_DMA_STOP_SPIN_LIMIT      100

// Every status flag of a channel: global, full transfer, half transfer, transfer error.
// Writing the global flag clears the other three with it, the rest are named for clarity.
#define BB_DMA_FLAGS                (0x1 | DMA_IT_TCIF | DMA_IT_HTIF | DMA_IT_DMEIF)

// The AT32 headers have no MODIFY_REG
#define BB_MODIFY_REG(reg, mask, value) ((reg) = ((reg) & ~(mask)) | (value))

typedef enum {
    BB_DIRECTION_OUTPUT = 0,
    BB_DIRECTION_INPUT
} bbDirection_e;

// A compare channel that can pace a port group
typedef struct {
    tmr_type *tim;
    uint8_t channelIndex;       // 0..3
    uint32_t dmaMuxId;          // DMAMUX request of this channel's compare event
} bbPacerChannel_t;

// A timer used as pacer; its period is shared by every port group on it
typedef struct {
    tmr_type *tim;
    uint16_t dmaSources;        // TMR_Cx_DMA_REQUEST bits of the port groups on this timer
} bbPacer_t;

// One GPIO port with motors on it
typedef struct {
    int portIndex;
    gpio_type *gpio;
    const bbPacerChannel_t *pacerChannel;
    bbPacer_t *pacer;
    uint16_t dmaSource;         // TMR_Cx_DMA_REQUEST of the pacer channel
    DMA_t dma;

    // cfgr (mode) bits of the motor pins on this port: the mask, and their input and
    // output settings
    uint32_t gpioModeMask;
    uint32_t gpioModeInput;
    uint32_t gpioModeOutput;
    // scr word that puts every motor pin of the port at its idle level
    uint32_t gpioIdleScr;

    // DMA channel control register for either direction, enable bit clear: the direction
    // switch rewrites the channel from these instead of running the SDK init
    uint32_t dmaCtrlOutput;
    uint32_t dmaCtrlInput;
    // Pacer period for sending and for sampling
    uint16_t outputPeriod;
    uint16_t inputPeriod;

    uint32_t *outputBuffer;
    uint16_t *inputBuffer;

    volatile uint8_t direction;         // bbDirection_e, where the pins and the DMA point
    volatile bool telemetryPending;     // reply window of the last frame still open
    bool telemetryAborted;              // the window was cut short, the samples are not a whole reply
} bbPort_t;

typedef struct {
    IO_t io;                    // set once the motor has been handed to this driver
    const timerHardware_t *timHw;   // the output as the target defines it, for the fallback to the timers
    ioTag_t tag;
    uint8_t pinIndex;
    bool enabled;               // outputs enabled at all (FEATURE_PWM_OUTPUT_ENABLE)
    bool inverted;              // the line idles high (bidirectional signalling on non-inverting hardware)
    ioConfig_t iocfg;
    bbPort_t *port;
    bool configured;            // in a port group, frames go out on it
} bbMotor_t;

// TMR8 first: on the AT32 targets TMR1 CH1 is often the LED strip
static const bbPacerChannel_t bbPacerChannels[] = {
    { TMR8, 0, DMAMUX_DMAREQ_ID_TMR8_CH1 },
    { TMR8, 1, DMAMUX_DMAREQ_ID_TMR8_CH2 },
    { TMR8, 2, DMAMUX_DMAREQ_ID_TMR8_CH3 },
    { TMR8, 3, DMAMUX_DMAREQ_ID_TMR8_CH4 },
    { TMR1, 0, DMAMUX_DMAREQ_ID_TMR1_CH1 },
    { TMR1, 1, DMAMUX_DMAREQ_ID_TMR1_CH2 },
    { TMR1, 2, DMAMUX_DMAREQ_ID_TMR1_CH3 },
    { TMR1, 3, DMAMUX_DMAREQ_ID_TMR1_CH4 },
};

static bbPacer_t bbPacers[BB_MAX_PACERS];
static int bbPacerCount = 0;
static bbPort_t bbPorts[BB_MAX_PORTS];
static int bbPortCount = 0;
static bbMotor_t bbMotors[MAX_MOTORS];

// Plain RAM: the ADC's DMA buffer lives there too, and the CPU has no cache to keep coherent
static uint32_t bbOutputBuffers[BB_MAX_PORTS][BB_OUTPUT_BUFFER_LENGTH];
static uint16_t bbInputBuffers[BB_MAX_PORTS][BB_INPUT_BUFFER_LENGTH];

static bool bbActive = false;           // selected for this boot
static bool bbReady = false;            // port groups set up, frames may go out
static dshotBitbangStatus_e bbStatus = DSHOT_BITBANG_STATUS_OK;
static uint32_t bbOutputStateHz;        // pacer rate while sending: three states per bit
static uint32_t bbInputSampleHz;        // pacer rate while sampling
static bool bbFrameInFlight = false;
static timeUs_t bbFrameStartUs = 0;
// Backstop for a reply window that never closes: frame plus window plus a quarter
static timeUs_t bbTelemetryTimeoutUs = 0;

// DEBUG_ESC counters
static uint32_t bbStatDecoded = 0;
static uint32_t bbStatNoEdge = 0;
static uint32_t bbStatInvalid = 0;
static uint32_t bbStatAborted = 0;

// The channel identifiers are evenly spaced, so channelIndex maps onto them linearly
static tmr_channel_select_type bbTimChannel(uint8_t channelIndex)
{
    STATIC_ASSERT(TMR_SELECT_CHANNEL_4 == TMR_SELECT_CHANNEL_1 + 3 * (TMR_SELECT_CHANNEL_2 - TMR_SELECT_CHANNEL_1), tmr_channel_ids_linear);
    return (tmr_channel_select_type)(TMR_SELECT_CHANNEL_1 + channelIndex * (TMR_SELECT_CHANNEL_2 - TMR_SELECT_CHANNEL_1));
}

// The CC1..CC4 DMA request bits sit next to each other in IDEN
static uint16_t bbDmaSource(uint8_t channelIndex)
{
    STATIC_ASSERT(TMR_C2_DMA_REQUEST == TMR_C1_DMA_REQUEST << 1 && TMR_C4_DMA_REQUEST == TMR_C1_DMA_REQUEST << 3, tmr_dma_request_bits_adjacent);
    return TMR_C1_DMA_REQUEST << channelIndex;
}

// A pacer takes the whole timer (period and prescaler), so no channel of it may belong to
// anyone else. Every timer user allocates through timerGetTCH(), which records the channel
// in timerCtx; this driver does not, so its own pacers do not show up here.
static bool bbTimerIsFree(tmr_type *tim)
{
    const uint8_t timerIndex = lookupTimerIndex(tim);
    if (timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
        return false;
    }

    const timHardwareContext_t *timCtx = timerCtx[timerIndex];
    if (timCtx == NULL) {
        return true;
    }

    for (int channel = 0; channel < CC_CHANNELS_PER_TIMER; channel++) {
        if (timCtx->ch[channel].timHw != NULL) {
            return false;
        }
    }

    return true;
}

static bbPacer_t *bbPacerGet(tmr_type *tim, uint16_t outputPeriod)
{
    for (int i = 0; i < bbPacerCount; i++) {
        if (bbPacers[i].tim == tim) {
            return &bbPacers[i];
        }
    }

    if (bbPacerCount >= BB_MAX_PACERS) {
        return NULL;
    }

    const uint8_t timerIndex = lookupTimerIndex(tim);
    if (timerIndex >= HARDWARE_TIMER_DEFINITION_COUNT) {
        return NULL;
    }

    bbPacer_t *pacer = &bbPacers[bbPacerCount++];
    pacer->tim = tim;
    pacer->dmaSources = 0;

    // timerInit() only clocks the timers in the target's timer table; the pacer may not be one
    RCC_ClockCmd(timerDefinitions[timerIndex].rcc, ENABLE);

    // Free running from here on; the port groups gate its DMA requests. The period is
    // buffered, so the direction switches below change it at the next overflow instead of
    // mid-period (Betaflight does the same).
    tmr_counter_enable(tim, FALSE);
    tmr_base_init(tim, outputPeriod, 0);
    tmr_clock_source_div_set(tim, TMR_CLOCK_DIV1);
    tmr_cnt_dir_set(tim, TMR_COUNT_UP);
    tmr_period_buffer_enable(tim, TRUE);
    tmr_counter_enable(tim, TRUE);

    return pacer;
}

// The pacer channel is output compare with nothing muxed to it: only its compare event is
// used, as the DMA request
static void bbPacerChannelInit(const bbPacerChannel_t *pacerChannel)
{
    tmr_type *tim = pacerChannel->tim;
    const tmr_channel_select_type channel = bbTimChannel(pacerChannel->channelIndex);

    tmr_output_config_type output;
    tmr_output_default_para_init(&output);
    output.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    output.oc_idle_state = TRUE;
    output.oc_output_state = TRUE;
    output.oc_polarity = TMR_OUTPUT_ACTIVE_LOW;
    tmr_output_channel_config(tim, channel, &output);

    tmr_channel_value_set(tim, channel, BB_PACER_COMPARE);
    tmr_output_channel_buffer_enable(tim, channel, TRUE);
    tmr_channel_enable(tim, channel, TRUE);
}

static DMA_t bbFindFreeDma(void)
{
    for (unsigned index = 0; ; index++) {
        DMA_t dma = dmaGetByIndex(index);
        if (dma == NULL) {
            return NULL;
        }
        if (dmaGetOwner(dma) == OWNER_FREE) {
            return dma;
        }
    }
}

static void bbDmaIrqHandler(DMA_t descriptor);

static void bbDmaSetup(bbPort_t *port)
{
    dmaInit(port->dma, OWNER_DSHOT_BITBANG, RESOURCE_INDEX(port - bbPorts));
    dmaSetHandler(port->dma, bbDmaIrqHandler, NVIC_PRIO_TIMER_DMA, (uint32_t)port);

    dma_channel_type *channel = port->dma->ref;
    dma_reset(channel);

    // Run the SDK init once per direction only to learn the control register value for it;
    // the direction switch then rewrites the channel from that value
    dma_init_type init;
    dma_default_para_init(&init);
    init.peripheral_inc_enable = FALSE;
    init.memory_inc_enable = TRUE;
    init.loop_mode_enable = FALSE;
    init.priority = DMA_PRIORITY_VERY_HIGH;

    // Sampling: the port's input register, one halfword per pacer period
    init.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    init.buffer_size = BB_INPUT_BUFFER_LENGTH;
    init.peripheral_base_addr = (uint32_t)&port->gpio->idt;
    init.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
    init.memory_base_addr = (uint32_t)port->inputBuffer;
    init.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    dma_init(channel, &init);
    port->dmaCtrlInput = channel->ctrl | DMA_FDT_INT;

    // Sending: one set/clear word per pacer period
    init.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
    init.buffer_size = BB_OUTPUT_BUFFER_LENGTH;
    init.peripheral_base_addr = (uint32_t)&port->gpio->scr;
    init.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_WORD;
    init.memory_base_addr = (uint32_t)port->outputBuffer;
    init.memory_data_width = DMA_MEMORY_DATA_WIDTH_WORD;
    dma_init(channel, &init);
    port->dmaCtrlOutput = channel->ctrl | DMA_FDT_INT;

    // dma_reset() does not touch the mux, but keep the request routing after the inits anyway
    dmaMuxEnable(port->dma, port->pacerChannel->dmaMuxId);
}

// Channel registers for one direction; the channel must be disabled
static void bbDmaLoad(bbPort_t *port, bbDirection_e direction)
{
    dma_channel_type *channel = port->dma->ref;

    if (direction == BB_DIRECTION_INPUT) {
        channel->ctrl = port->dmaCtrlInput;
        channel->dtcnt = BB_INPUT_BUFFER_LENGTH;
        channel->paddr = (uint32_t)&port->gpio->idt;
        channel->maddr = (uint32_t)port->inputBuffer;
    } else {
        channel->ctrl = port->dmaCtrlOutput;
        channel->dtcnt = BB_OUTPUT_BUFFER_LENGTH;
        channel->paddr = (uint32_t)&port->gpio->scr;
        channel->maddr = (uint32_t)port->outputBuffer;
    }
}

// Clearing the enable bit is a request; the hardware drops it once the transfer in flight
// is done, and the channel registers may not be written before that
static bool bbDmaStop(bbPort_t *port)
{
    dma_channel_type *channel = port->dma->ref;

    dma_channel_enable(channel, FALSE);
    for (unsigned spin = 0; spin < BB_DMA_STOP_SPIN_LIMIT; spin++) {
        if (!channel->ctrl_bit.chen) {
            return true;
        }
    }

    return false;
}

// Pins back to outputs at their idle level and the channel reloaded for a frame, not yet
// enabled. false if the channel would not stop: its registers were left alone and the
// pins stay inputs, off a line something is evidently still driving.
static bool bbSwitchToOutput(bbPort_t *port)
{
    if (!bbDmaStop(port)) {
        return false;
    }
    DMA_CLEAR_FLAG(port->dma, BB_DMA_FLAGS);

    // Idle level first, so that is what the pins drive the moment they become outputs
    port->gpio->scr = port->gpioIdleScr;
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        BB_MODIFY_REG(port->gpio->cfgr, port->gpioModeMask, port->gpioModeOutput);
    }

    bbDmaLoad(port, BB_DIRECTION_OUTPUT);
    port->pacer->tim->pr = port->outputPeriod;
    port->direction = BB_DIRECTION_OUTPUT;

    return true;
}

// Pins to inputs and the channel sampling them; runs in the completion interrupt of the frame
static void bbSwitchToInput(bbPort_t *port)
{
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        BB_MODIFY_REG(port->gpio->cfgr, port->gpioModeMask, port->gpioModeInput);
    }

    bbDmaLoad(port, BB_DIRECTION_INPUT);

    // Restart the period for sampling. Every port group on this timer finished its frame on
    // the same compare event, so none of them is still sending. The counter is zeroed first:
    // the new period is shorter, and a count above it would run to the 16-bit wrap.
    tmr_type *tim = port->pacer->tim;
    tim->cval = 0;
    tim->pr = port->inputPeriod;

    port->direction = BB_DIRECTION_INPUT;
    dma_channel_enable(port->dma->ref, TRUE);
}

static void bbDmaIrqHandler(DMA_t descriptor)
{
    bbPort_t *port = (bbPort_t *)descriptor->userParam;
    tmr_type *tim = port->pacer->tim;

    // The request goes first: a request the timer has already raised is not withdrawn by
    // IDEN (AT32 errata 1.10.1), so stop raising new ones before the channel stops serving them
    tmr_dma_request_enable(tim, port->dmaSource, FALSE);
    dma_channel_enable(port->dma->ref, FALSE);

    const bool transferError = DMA_GET_FLAG_STATUS(descriptor, DMA_IT_DMEIF) != 0;
    DMA_CLEAR_FLAG(descriptor, BB_DMA_FLAGS);

    if (port->direction == BB_DIRECTION_INPUT || transferError) {
        // The reply window has been sampled, or the transfer died and nothing more comes
        // this frame: either way the buffer is not touched again until the next frame
        if (transferError) {
            port->telemetryAborted = true;
        }
        port->telemetryPending = false;
        return;
    }

    // Frame out: turn the port round and have the pacer clock the samples in
    bbSwitchToInput(port);
    tmr_dma_request_enable(tim, port->dmaSource, TRUE);
}

static bool bbTelemetryPending(void)
{
    bool pending = false;
    for (int i = 0; i < bbPortCount; i++) {
        pending |= bbPorts[i].telemetryPending;
    }
    return pending;
}

// Find or set up the group of the GPIO port this pin belongs to
static bbPort_t *bbPortGet(IO_t io)
{
    const int portIndex = IO_GPIOPortIdx(io);

    for (int i = 0; i < bbPortCount; i++) {
        if (bbPorts[i].portIndex == portIndex) {
            return &bbPorts[i];
        }
    }

    if (bbPortCount >= BB_MAX_PORTS) {
        bbStatus = DSHOT_BITBANG_STATUS_TOO_MANY_PORTS;
        return NULL;
    }

    // First unused channel of a timer nobody else has a channel on
    const bbPacerChannel_t *pacerChannel = NULL;
    for (unsigned i = 0; i < ARRAYLEN(bbPacerChannels) && pacerChannel == NULL; i++) {
        const bbPacerChannel_t *candidate = &bbPacerChannels[i];
        if (!bbTimerIsFree(candidate->tim)) {
            continue;
        }
        bool taken = false;
        for (int p = 0; p < bbPortCount; p++) {
            taken |= bbPorts[p].pacerChannel == candidate;
        }
        if (!taken) {
            pacerChannel = candidate;
        }
    }
    if (pacerChannel == NULL) {
        bbStatus = DSHOT_BITBANG_STATUS_NO_PACER;
        return NULL;
    }

    DMA_t dma = bbFindFreeDma();
    if (dma == NULL) {
        bbStatus = DSHOT_BITBANG_STATUS_NO_DMA;
        return NULL;
    }

    bbPort_t *port = &bbPorts[bbPortCount];
    memset(port, 0, sizeof(*port));
    port->portIndex = portIndex;
    port->gpio = IO_GPIO(io);
    port->pacerChannel = pacerChannel;
    port->dmaSource = bbDmaSource(pacerChannel->channelIndex);
    port->dma = dma;
    port->outputBuffer = bbOutputBuffers[bbPortCount];
    port->inputBuffer = bbInputBuffers[bbPortCount];

    const uint32_t timerClockHz = timerClock(pacerChannel->tim);
    port->outputPeriod = timerClockHz / bbOutputStateHz - 1;
    port->inputPeriod = timerClockHz / bbInputSampleHz - 1;

    port->pacer = bbPacerGet(pacerChannel->tim, port->outputPeriod);
    if (port->pacer == NULL) {
        bbStatus = DSHOT_BITBANG_STATUS_NO_PACER;
        return NULL;
    }
    port->pacer->dmaSources |= port->dmaSource;
    bbPacerChannelInit(pacerChannel);

    bbDmaSetup(port);
    port->direction = BB_DIRECTION_OUTPUT;

    bbPortCount++;
    return port;
}

// The fixed part of the motor's bits in the port's output buffer: every bit starts with the
// pin going active and ends with it back at idle; the middle word is written per frame
static void bbPortAddMotor(bbPort_t *port, const bbMotor_t *motor)
{
    const uint32_t pinMask = 1u << motor->pinIndex;
    // scr: low half sets the pin, high half clears it
    const uint32_t toHigh = pinMask;
    const uint32_t toLow = pinMask << 16;
    const uint32_t toIdle = motor->inverted ? toHigh : toLow;
    const uint32_t toActive = motor->inverted ? toLow : toHigh;

    port->gpioModeMask |= 0x3u << (motor->pinIndex * 2);
    port->gpioModeInput |= (uint32_t)GPIO_MODE_INPUT << (motor->pinIndex * 2);
    port->gpioModeOutput |= (uint32_t)GPIO_MODE_OUTPUT << (motor->pinIndex * 2);
    port->gpioIdleScr |= toIdle;

    uint32_t *buffer = port->outputBuffer;
    for (int bit = 0; bit < BB_FRAME_BITS; bit++) {
        buffer[bit * BB_STATES_PER_BIT + 0] |= toActive;
        buffer[bit * BB_STATES_PER_BIT + 2] |= toIdle;
    }
    // Hold: idle again for the extra bit period, then no change
    buffer[BB_FRAME_BITS * BB_STATES_PER_BIT] |= toIdle;
}

void dshotBitbangInit(uint32_t bitRateHz)
{
    bbActive = true;
    bbStatus = DSHOT_BITBANG_STATUS_OK;

    bbOutputStateHz = bitRateHz * BB_STATES_PER_BIT;
    // GCR replies run at 5/4 of the DSHOT bit rate
    bbInputSampleHz = bitRateHz * 5 / 4 * DSHOT_BITBANG_TELEMETRY_OVERSAMPLE;

    const uint32_t frameUs = BB_OUTPUT_BUFFER_LENGTH * 1000000UL / bbOutputStateHz;
    const uint32_t windowUs = BB_INPUT_BUFFER_LENGTH * 1000000UL / bbInputSampleHz;
    bbTelemetryTimeoutUs = (frameUs + windowUs) * 5 / 4;
}

bool dshotBitbangIsActive(void)
{
    return bbActive;
}

dshotBitbangStatus_e dshotBitbangGetStatus(void)
{
    return bbStatus;
}

bool dshotBitbangMotorConfig(const timerHardware_t *timHw, uint8_t motorIndex, bool enableOutput)
{
    if (!bbActive || motorIndex >= MAX_MOTORS) {
        return false;
    }

    const IO_t io = IOGetByTag(timHw->tag);
    if (io == NULL) {
        return false;
    }

    bbMotor_t *motor = &bbMotors[motorIndex];
    motor->io = io;
    motor->timHw = timHw;
    motor->tag = timHw->tag;
    motor->pinIndex = IO_GPIOPinIdx(io);
    motor->enabled = enableOutput;
    IOInit(io, OWNER_MOTOR, RESOURCE_OUTPUT, RESOURCE_INDEX(motorIndex));

    if (!enableOutput) {
        // Outputs disabled: plain GPIO driven low, as the timer driver does
        IOConfigGPIO(io, IOCFG_OUT_OD);
        IOLo(io);
        return true;
    }

    // Bidirectional signalling idles high; hardware with an inverter on the output flips it
    const bool hardwareInverted = (timHw->output & TIMER_OUTPUT_INVERTED) != 0;
    motor->inverted = hardwareInverted != useDshotTelemetry;
    // The pull holds the line at idle while the port listens for the reply
    motor->iocfg = IO_CONFIG(GPIO_MODE_OUTPUT, GPIO_DRIVE_STRENGTH_STRONGER, GPIO_OUTPUT_PUSH_PULL, motor->inverted ? GPIO_PULL_UP : GPIO_PULL_DOWN);

    // Holding the line at its idle-high level with no edges during ESC boot triggers the
    // BLHeli/Bluejay bootloader-entry check (~150 ms of continuous high right after the
    // startup melody), and the first frame goes out seconds after this. Park the pin so the
    // ESC sees low, like plain DSHOT idle; the first frame drives the idle level
    // (bbSwitchToOutput()) microseconds before it starts. On inverting hardware the level
    // is re-inverted downstream, so park high there.
    IOWrite(io, hardwareInverted);
    IOConfigGPIO(io, motor->iocfg);

    return true;
}

bool dshotBitbangMotorIsConfigured(uint8_t motorIndex)
{
    return motorIndex < MAX_MOTORS && bbMotors[motorIndex].configured;
}

bool dshotBitbangGetMotorOutput(uint8_t motorIndex, const timerHardware_t **timHw, bool *enableOutput)
{
    if (motorIndex >= MAX_MOTORS || bbMotors[motorIndex].io == NULL) {
        return false;
    }

    *timHw = bbMotors[motorIndex].timHw;
    *enableOutput = bbMotors[motorIndex].enabled;
    return true;
}

void dshotBitbangRelease(void)
{
    // Port groups that were set up before the failure: hand their DMA channels back and
    // leave the channels silent. The pin stays a parked GPIO output; the timer driver
    // reconfigures it as its alternate function.
    for (int i = 0; i < bbPortCount; i++) {
        bbPort_t *port = &bbPorts[i];
        tmr_dma_request_enable(port->pacer->tim, port->dmaSource, FALSE);
        dma_channel_enable(port->dma->ref, FALSE);
        dma_reset(port->dma->ref);
        tmr_channel_enable(port->pacerChannel->tim, bbTimChannel(port->pacerChannel->channelIndex), FALSE);
        port->dma->irqHandlerCallback = NULL;
        port->dma->owner = OWNER_FREE;
        port->dma->resourceIndex = 0;
    }
    // The pacer timers were free, nothing else expects them running
    for (int i = 0; i < bbPacerCount; i++) {
        tmr_counter_enable(bbPacers[i].tim, FALSE);
    }
    for (int motorIndex = 0; motorIndex < MAX_MOTORS; motorIndex++) {
        bbMotors[motorIndex].configured = false;
        bbMotors[motorIndex].port = NULL;
    }

    bbPortCount = 0;
    bbPacerCount = 0;
    bbFrameInFlight = false;
    bbReady = false;
    bbActive = false;
}

bool dshotBitbangPostInit(void)
{
    if (!bbActive) {
        return true;
    }

    memset(bbOutputBuffers, 0, sizeof(bbOutputBuffers));
    memset(bbInputBuffers, 0, sizeof(bbInputBuffers));

    for (int motorIndex = 0; motorIndex < MAX_MOTORS; motorIndex++) {
        bbMotor_t *motor = &bbMotors[motorIndex];
        if (motor->io == NULL || !motor->enabled) {
            continue;
        }

        bbPort_t *port = bbPortGet(motor->io);
        if (port == NULL) {
            LOG_ERROR(PWM, "Bitbang DSHOT: no pacer or DMA for motor %d (status %d)", motorIndex, bbStatus);
            return false;
        }

        motor->port = port;
        bbPortAddMotor(port, motor);
        motor->configured = true;
    }

    // The pins stay parked until the first frame
    bbReady = true;
    return true;
}

void dshotBitbangWrite(uint8_t motorIndex, uint16_t packet)
{
    if (motorIndex >= MAX_MOTORS || !bbMotors[motorIndex].configured) {
        return;
    }

    const bbMotor_t *motor = &bbMotors[motorIndex];
    uint32_t *buffer = motor->port->outputBuffer;
    // The middle state of a bit: the pin back to idle for a 0, no change for a 1
    const uint32_t toIdle = motor->inverted ? (1u << motor->pinIndex) : (1u << (motor->pinIndex + 16));

    for (int bit = 0; bit < BB_FRAME_BITS; bit++, packet <<= 1) {
        uint32_t *middle = &buffer[bit * BB_STATES_PER_BIT + 1];
        if (packet & 0x8000) {
            *middle &= ~toIdle;
        } else {
            *middle |= toIdle;
        }
    }
}

void dshotBitbangUpdateComplete(void)
{
    if (!bbReady) {
        return;
    }

    for (int i = 0; i < bbPortCount; i++) {
        bbPort_t *port = &bbPorts[i];
        port->telemetryAborted = false;
        if (!bbSwitchToOutput(port)) {
            // Nothing goes out on a port whose channel would not stop
            port->telemetryAborted = true;
            port->telemetryPending = false;
            continue;
        }
        // The window is pending from the frame start: the decode must not read the buffer
        // before the completion interrupt of the frame has even turned the port round
        port->telemetryPending = true;
        dma_channel_enable(port->dma->ref, TRUE);
    }

    bbFrameStartUs = micros();
    bbFrameInFlight = true;

    // Every port group on a timer starts on its next compare event
    for (int i = 0; i < bbPacerCount; i++) {
        tmr_dma_request_enable(bbPacers[i].tim, bbPacers[i].dmaSources, TRUE);
    }
}

bool dshotBitbangDecodeTelemetry(void)
{
    if (!bbReady || !bbFrameInFlight) {
        return true;
    }

    if (bbTelemetryPending()) {
        if ((micros() - bbFrameStartUs) < bbTelemetryTimeoutUs) {
            return false;
        }

        // A window that never closed: stop sampling and skip those buffers, they may hold
        // part of a reply that would still decode to a wrong but valid value. Atomic so the
        // completion interrupt can't turn the port round between these steps.
        for (int i = 0; i < bbPortCount; i++) {
            bbPort_t *port = &bbPorts[i];
            ATOMIC_BLOCK(NVIC_PRIO_MAX) {
                if (port->telemetryPending) {
                    tmr_dma_request_enable(port->pacer->tim, port->dmaSource, FALSE);
                    dma_channel_enable(port->dma->ref, FALSE);
                    port->telemetryAborted = true;
                    port->telemetryPending = false;
                    bbStatAborted++;
                }
            }
        }
    }
    bbFrameInFlight = false;

    for (int motorIndex = 0; motorIndex < getMotorCount(); motorIndex++) {
        const bbMotor_t *motor = &bbMotors[motorIndex];
        if (!motor->configured || motor->port->telemetryAborted) {
            continue;
        }

        const uint16_t rawValue = dshotBitbangDecode(motor->port->inputBuffer, BB_INPUT_BUFFER_LENGTH, motor->pinIndex, motor->inverted);
        if (rawValue == DSHOT_TELEMETRY_NOEDGE) {
            bbStatNoEdge++;
        } else if (rawValue == DSHOT_TELEMETRY_INVALID) {
            bbStatInvalid++;
        } else {
            bbStatDecoded++;
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
    }

    if (debugMode == DEBUG_ESC) {
        debug[0] = bbStatDecoded;
        debug[1] = bbStatNoEdge;
        debug[2] = bbStatInvalid;
        debug[3] = bbStatAborted;
        debug[4] = bbPortCount;
        debug[5] = bbStatus;
    }

    return true;
}

void dshotBitbangStop(void)
{
    if (!bbReady) {
        return;
    }

    // Let the reply windows of the frame in flight close; the backstop bounds the wait
    const timeUs_t startUs = micros();
    while (bbFrameInFlight && bbTelemetryPending() && (micros() - startUs) < bbTelemetryTimeoutUs) { }

    // Atomic per port: a completion interrupt landing in between would turn the port
    // round again and re-enable its pacer request
    for (int i = 0; i < bbPortCount; i++) {
        bbPort_t *port = &bbPorts[i];
        ATOMIC_BLOCK(NVIC_PRIO_MAX) {
            tmr_dma_request_enable(port->pacer->tim, port->dmaSource, FALSE);
            port->telemetryPending = false;
            port->telemetryAborted = true;
            bbSwitchToOutput(port);
        }
    }
    bbFrameInFlight = false;
}

void dshotBitbangDisconnect(void)
{
    dshotBitbangStop();

    // Inputs: the pull of each pin keeps its line at idle, nothing drives it. The next
    // frame turns the ports back to outputs as it does after every reply window.
    for (int i = 0; i < bbPortCount; i++) {
        bbPort_t *port = &bbPorts[i];
        ATOMIC_BLOCK(NVIC_PRIO_MAX) {
            BB_MODIFY_REG(port->gpio->cfgr, port->gpioModeMask, port->gpioModeInput);
        }
    }
}

ioTag_t dshotBitbangGetMotorPinTag(uint8_t motorIndex)
{
    if (motorIndex >= MAX_MOTORS || bbMotors[motorIndex].io == NULL) {
        return IOTAG_NONE;
    }
    return bbMotors[motorIndex].tag;
}

void dshotBitbangRestoreMotorPin(uint8_t motorIndex)
{
    if (motorIndex >= MAX_MOTORS || bbMotors[motorIndex].io == NULL) {
        return;
    }

    const bbMotor_t *motor = &bbMotors[motorIndex];
    if (!motor->enabled) {
        IOConfigGPIO(motor->io, IOCFG_OUT_OD);
        IOLo(motor->io);
        return;
    }

    // Back to an output at idle; frames resume with pwmEnableMotors()
    IOWrite(motor->io, motor->inverted);
    IOConfigGPIO(motor->io, motor->iocfg);
}

#endif // USE_DSHOT_BITBANG

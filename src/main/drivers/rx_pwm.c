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

#if defined(USE_RX_PPM)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/utils.h"

#include "drivers/time.h"

#include "drivers/nvic.h"
#include "drivers/io.h"
#include "timer.h"

#include "pwm_output.h"
#include "pwm_mapping.h"

#include "rx_pwm.h"

#define PPM_CAPTURE_COUNT       16
#define INPUT_FILTER_TICKS      10
#define PPM_TIMER_PERIOD        0x10000

void pwmICConfig(TIM_TypeDef *tim, uint8_t channel, uint16_t polarity);

static uint16_t captures[PPM_CAPTURE_COUNT];

static uint8_t ppmFrameCount = 0;
static uint8_t lastPPMFrameCount = 0;
static uint8_t ppmCountDivisor = 1;

typedef struct ppmDevice_s {
    uint8_t  pulseIndex;
    uint32_t currentCapture;
    uint32_t currentTime;
    uint32_t deltaTime;
    uint32_t captures[PPM_CAPTURE_COUNT];
    uint32_t largeCounter;
    int8_t   numChannels;
    int8_t   numChannelsPrevFrame;
    uint8_t  stableFramesSeenCount;

    bool     tracking;
    bool     overflowed;
} ppmDevice_t;

ppmDevice_t ppmDev;

#define PPM_IN_MIN_SYNC_PULSE_US            2700    // microseconds
#define PPM_IN_MIN_CHANNEL_PULSE_US         750     // microseconds
#define PPM_IN_MAX_CHANNEL_PULSE_US         2250    // microseconds
#define PPM_STABLE_FRAMES_REQUIRED_COUNT    25
#define PPM_IN_MIN_NUM_CHANNELS             4
#define PPM_IN_MAX_NUM_CHANNELS             PPM_CAPTURE_COUNT

bool isPPMDataBeingReceived(void)
{
    return (ppmFrameCount != lastPPMFrameCount);
}

void resetPPMDataReceivedState(void)
{
    lastPPMFrameCount = ppmFrameCount;
}

#define MIN_CHANNELS_BEFORE_PPM_FRAME_CONSIDERED_VALID 4

static void ppmInit(void)
{
    ppmDev.pulseIndex   = 0;
    ppmDev.currentCapture = 0;
    ppmDev.currentTime  = 0;
    ppmDev.deltaTime    = 0;
    ppmDev.largeCounter = 0;
    ppmDev.numChannels  = -1;
    ppmDev.numChannelsPrevFrame = -1;
    ppmDev.stableFramesSeenCount = 0;
    ppmDev.tracking     = false;
    ppmDev.overflowed   = false;
}

static void ppmOverflowCallback(struct TCH_s * tch, uint32_t capture)
{
    UNUSED(tch);

    ppmDev.largeCounter += capture + 1;
    if (capture == PPM_TIMER_PERIOD - 1) {
        ppmDev.overflowed = true;
    }
}

static void ppmEdgeCallback(struct TCH_s * tch, uint32_t capture)
{
    UNUSED(tch);

    int32_t i;

    uint32_t previousTime = ppmDev.currentTime;
    uint32_t previousCapture = ppmDev.currentCapture;

    /* Grab the new count */
    uint32_t currentTime = capture;

    /* Convert to 32-bit timer result */
    currentTime += ppmDev.largeCounter;

    if (capture < previousCapture) {
        if (ppmDev.overflowed) {
            currentTime += PPM_TIMER_PERIOD;
        }
    }

    // Divide to match output protocol
    currentTime = currentTime / ppmCountDivisor;

    /* Capture computation */
    if (currentTime > previousTime) {
        ppmDev.deltaTime    = currentTime - (previousTime + (ppmDev.overflowed ? (PPM_TIMER_PERIOD / ppmCountDivisor) : 0));
    } else {
        ppmDev.deltaTime    = (PPM_TIMER_PERIOD / ppmCountDivisor) + currentTime - previousTime;
    }

    ppmDev.overflowed = false;


    /* Store the current measurement */
    ppmDev.currentTime = currentTime;
    ppmDev.currentCapture = capture;

#if 0
    static uint32_t deltaTimes[20];
    static uint8_t deltaIndex = 0;

    deltaIndex = (deltaIndex + 1) % 20;
    deltaTimes[deltaIndex] = ppmDev.deltaTime;
    UNUSED(deltaTimes);
#endif


#if 0
    static uint32_t captureTimes[20];
    static uint8_t captureIndex = 0;

    captureIndex = (captureIndex + 1) % 20;
    captureTimes[captureIndex] = capture;
    UNUSED(captureTimes);
#endif

    /* Sync pulse detection */
    if (ppmDev.deltaTime > PPM_IN_MIN_SYNC_PULSE_US) {
        if (ppmDev.pulseIndex == ppmDev.numChannelsPrevFrame
            && ppmDev.pulseIndex >= PPM_IN_MIN_NUM_CHANNELS
            && ppmDev.pulseIndex <= PPM_IN_MAX_NUM_CHANNELS) {
            /* If we see n simultaneous frames of the same
               number of channels we save it as our frame size */
            if (ppmDev.stableFramesSeenCount < PPM_STABLE_FRAMES_REQUIRED_COUNT) {
                ppmDev.stableFramesSeenCount++;
            } else {
                ppmDev.numChannels = ppmDev.pulseIndex;
            }
        } else {
            ppmDev.stableFramesSeenCount = 0;
        }

        /* Check if the last frame was well formed */
        if (ppmDev.pulseIndex == ppmDev.numChannels && ppmDev.tracking) {
            /* The last frame was well formed */
            for (i = 0; i < ppmDev.numChannels; i++) {
                captures[i] = ppmDev.captures[i];
            }
            for (i = ppmDev.numChannels; i < PPM_IN_MAX_NUM_CHANNELS; i++) {
                captures[i] = PPM_RCVR_TIMEOUT;
            }
            ppmFrameCount++;
        }

        ppmDev.tracking   = true;
        ppmDev.numChannelsPrevFrame = ppmDev.pulseIndex;
        ppmDev.pulseIndex = 0;

        /* We rely on the supervisor to set captureValue to invalid
           if no valid frame is found otherwise we ride over it */
    } else if (ppmDev.tracking) {
        /* Valid pulse duration 0.75 to 2.5 ms*/
        if (ppmDev.deltaTime > PPM_IN_MIN_CHANNEL_PULSE_US
            && ppmDev.deltaTime < PPM_IN_MAX_CHANNEL_PULSE_US
            && ppmDev.pulseIndex < PPM_IN_MAX_NUM_CHANNELS) {
            ppmDev.captures[ppmDev.pulseIndex] = ppmDev.deltaTime;
            ppmDev.pulseIndex++;
        } else {
            /* Not a valid pulse duration */
            ppmDev.tracking = false;
            for (i = 0; i < PPM_CAPTURE_COUNT; i++) {
                ppmDev.captures[i] = PPM_RCVR_TIMEOUT;
            }
        }
    }
}

bool ppmInConfig(const timerHardware_t *timerHardwarePtr)
{
    static timerCallbacks_t callbacks;
    TCH_t * tch = timerGetTCH(timerHardwarePtr);
    if (tch == NULL) {
        return false;
    }

    ppmInit();

    IO_t io = IOGetByTag(timerHardwarePtr->tag);
    IOInit(io, OWNER_PPMINPUT, RESOURCE_INPUT, 0);
    IOConfigGPIOAF(io, timerHardwarePtr->ioMode, timerHardwarePtr->alternateFunction);

    timerConfigure(tch, (uint16_t)PPM_TIMER_PERIOD, PWM_TIMER_HZ);
    timerChInitCallbacks(&callbacks, (void*)&ppmDev, &ppmEdgeCallback, &ppmOverflowCallback);
    timerChConfigCallbacks(tch, &callbacks);
    timerChConfigIC(tch, true, INPUT_FILTER_TICKS);
    timerChCaptureEnable(tch);

    return true;
}

uint16_t ppmRead(uint8_t channel)
{
    return captures[channel];
}
#endif

#include "lulu.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "common/filter.h"
#include "common/maths.h"
#include "common/utils.h"

#ifdef __ARM_ACLE
#include <arm_acle.h>
#endif /* __ARM_ACLE */
#include <fenv.h>

void luluFilterInit(luluFilter_t *filter, int N) {
    if (N > 15) {
        N = 15;
    }
    if (N < 1) {
        N = 1;
    }
    filter->N = N;
    filter->windowSize = filter->N * 2 + 1;
    filter->windowBufIndex = 0;

    memset(filter->luluInterim, 0, sizeof(float) * (filter->windowSize));
    memset(filter->luluInterimB, 0, sizeof(float) * (filter->windowSize));
}

FAST_CODE float fixRoad(float *series, float *seriesB, int index, int filterN, int windowSize) {
    register float curVal = 0;
    register float curValB = 0;
    for (int N = 1; N <= filterN; N++) {
        int indexNeg = (index + windowSize - 2 * N) % windowSize;
        register int curIndex = (indexNeg + 1) % windowSize;
        register float prevVal = series[indexNeg];
        register float prevValB = seriesB[indexNeg];
        register int indexPos = (curIndex + N) % windowSize;
        for (int i = windowSize - 2 * N; i < windowSize - N; i++) {
            if (indexPos >= windowSize) {
                indexPos = 0;
            }
            if (curIndex >= windowSize) {
                curIndex = 0;
            }

            curVal = series[curIndex];
            curValB = seriesB[curIndex];
            register float nextVal = series[indexPos];
            register float nextValB = seriesB[indexPos];

            if (prevVal < curVal && curVal > nextVal) {
                float maxValue = MAX(prevVal, nextVal);
                series[curIndex] = maxValue;
            }

            if (prevValB < curValB && curValB > nextValB) {
                float maxValue = MAX(prevValB, nextValB);
                seriesB[curIndex] = maxValue;
            }
            prevVal = curVal;
            prevValB = curValB;
            curIndex++;
            indexPos++;
        }

        curIndex = (indexNeg + 1) % windowSize;
        prevVal = series[indexNeg];
        prevValB = seriesB[indexNeg];
        indexPos = (curIndex + N) % windowSize;
        for (int i = windowSize - 2 * N; i < windowSize - N; i++) {
            if (indexPos >= windowSize) {
                indexPos = 0;
            }
            if (curIndex >= windowSize) {
                curIndex = 0;
            }

            curVal = series[curIndex];
            curValB = seriesB[curIndex];
            register float nextVal = series[indexPos];
            register float nextValB = seriesB[indexPos];

            if (prevVal > curVal && curVal < nextVal) {
                float minValue = MIN(prevVal, nextVal);
                series[curIndex] = minValue;
            }

            if (prevValB > curValB && curValB < nextValB) {
                float minValue = MIN(prevValB, nextValB);
                seriesB[curIndex] = minValue;
            }
            prevVal = curVal;
            prevValB = curValB; 
            curIndex++;
            indexPos++;
        }
    }
    int finalIndex = (index + windowSize - filterN) % windowSize;
    curVal = series[finalIndex];
    curValB = seriesB[finalIndex];
    return (curVal - curValB) / 2;
}

FAST_CODE float luluFilterPartialApply(luluFilter_t *filter, float input) {
    // This is the value N of the LULU filter.
    register int filterN = filter->N;
    // This is the total window size for the rolling buffer
    register int filterWindow = filter->windowSize;

    register int windowIndex = filter->windowBufIndex;
    register float inputVal = input;
    register int newIndex = (windowIndex + 1) % filterWindow;
    filter->windowBufIndex = newIndex;
    filter->luluInterim[windowIndex] = inputVal;
    filter->luluInterimB[windowIndex] = -inputVal;
    return fixRoad(filter->luluInterim, filter->luluInterimB, windowIndex, filterN, filterWindow);
}

FAST_CODE float luluFilterApply(luluFilter_t *filter, float input) {
    // This is the UL filter
    float resultA = luluFilterPartialApply(filter, input);
    // We use the median interpretation of this filter to remove bias in the output
    return resultA;
}
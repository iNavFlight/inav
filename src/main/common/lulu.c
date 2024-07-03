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

void luluFilterInit(luluFilter_t *filter, int N)
{
	filter->N = constrain(N, 1, 15);
	filter->windowSize = filter->N * 2 + 1;
	filter->windowBufIndex = 0;

	memset(filter->luluInterim, 0, sizeof(float) * (filter->windowSize));
	memset(filter->luluInterimB, 0, sizeof(float) * (filter->windowSize));
}

FAST_CODE float fixRoad(float *series, float *seriesB, int index, int filterN, int windowSize)
{
	float curVal = 0;
	float curValB = 0;
	for (int N = 1; N <= filterN; N++)
	{
		int indexNeg = (index + windowSize - 2 * N) % windowSize;
	    int curIndex = (indexNeg + 1) % windowSize;
	    float prevVal = series[indexNeg];
		float prevValB = seriesB[indexNeg];
		int indexPos = (curIndex + N) % windowSize;
		
        for (int i = windowSize - 2 * N; i < windowSize - N; i++)
		{
			if (indexPos >= windowSize)
			{
				indexPos = 0;
			}
			if (curIndex >= windowSize)
			{
				curIndex = 0;
			}
			// curIndex = (2 - 1) % 3 = 1
			curVal = series[curIndex];
			curValB = seriesB[curIndex];
			float nextVal = series[indexPos];
			float nextValB = seriesB[indexPos];
			// onbump (s, 1, 1, 3)
			// if(onBump(series, curIndex, N, windowSize))
			if (prevVal < curVal && curVal > nextVal)
			{
				float maxValue = MAX(prevVal, nextVal);

				series[curIndex] = maxValue;
			    int k = curIndex;
				for (int j = 1; j < N; j++)
				{
					if (++k >= windowSize)
					{
						k = 0;
					}
					series[k] = maxValue;
				}
			}

			if (prevValB < curValB && curValB > nextValB)
			{
				float maxValue = MAX(prevValB, nextValB);

				curVal = maxValue;
				seriesB[curIndex] = maxValue;
				int k = curIndex;
				for (int j = 1; j < N; j++)
				{
					if (++k >= windowSize)
					{
						k = 0;
					}
					seriesB[k] = maxValue;
				}
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
		for (int i = windowSize - 2 * N; i < windowSize - N; i++)
		{
			if (indexPos >= windowSize)
			{
				indexPos = 0;
			}
			if (curIndex >= windowSize)
			{
				curIndex = 0;
			}
			// curIndex = (2 - 1) % 3 = 1
			curVal = series[curIndex];
			curValB = seriesB[curIndex];
			float nextVal = series[indexPos];
			float nextValB = seriesB[indexPos];

			if (prevVal > curVal && curVal < nextVal)
			{
				float minValue = MIN(prevVal, nextVal);

				curVal = minValue;
				series[curIndex] = minValue;
				int k = curIndex;
				for (int j = 1; j < N; j++)
				{
					if (++k >= windowSize)
					{
						k = 0;
					}
					series[k] = minValue;
				}
			}

			if (prevValB > curValB && curValB < nextValB)
			{
				float minValue = MIN(prevValB, nextValB);
				curValB = minValue;
				seriesB[curIndex] = minValue;
				int k = curIndex;
				for (int j = 1; j < N; j++)
				{
					if (++k >= windowSize)
					{
						k = 0;
					}
					seriesB[k] = minValue;
				}
			}
			prevVal = curVal;
			prevValB = curValB; 
			curIndex++;
			indexPos++;
		}
	}
	return (curVal - curValB) / 2;
}

FAST_CODE float luluFilterPartialApply(luluFilter_t *filter, float input)
{
	// This is the value N of the LULU filter.
	int filterN = filter->N;
	// This is the total window size for the rolling buffer
	int filterWindow = filter->windowSize;

	int windowIndex = filter->windowBufIndex;
	float inputVal = input;
	int newIndex = (windowIndex + 1) % filterWindow;
	filter->windowBufIndex = newIndex;
	filter->luluInterim[windowIndex] = inputVal;
	filter->luluInterimB[windowIndex] = -inputVal;
	return fixRoad(filter->luluInterim, filter->luluInterimB, windowIndex, filterN, filterWindow);
}

FAST_CODE float luluFilterApply(luluFilter_t *filter, float input)
{
	// This is the UL filter
	float resultA = luluFilterPartialApply(filter, input);
	// We use the median interpretation of this filter to remove bias in the output
	return resultA;
}

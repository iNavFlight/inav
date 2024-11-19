#pragma once

// Max N = 15
typedef struct
{
    int windowSize;
    int windowBufIndex;
    int N;
    float luluInterim[32] __attribute__((aligned(128)));
    float luluInterimB[32];
} luluFilter_t;

void luluFilterInit(luluFilter_t *filter, int N);
float luluFilterApply(luluFilter_t *filter, float input);

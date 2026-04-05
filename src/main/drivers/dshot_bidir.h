#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/timer.h"

void dshotBidirInit(uint32_t dshotHz);
bool dshotBidirAttachMotor(uint8_t motorIndex, TCH_t *tch, void *dmaBurstBuffer);
bool dshotBidirUpdate(void);
void dshotBidirOnFrameStarted(void);
uint32_t dshotBidirGetReadCount(void);
uint32_t dshotBidirGetInvalidPacketCount(void);
uint32_t dshotBidirGetNoEdgeCount(void);
uint32_t dshotBidirGetTimeoutCount(void);
uint8_t dshotBidirGetLastEdgeCount(uint8_t motorIndex);
uint16_t dshotBidirGetLastRawValue(uint8_t motorIndex);
uint32_t dshotBidirGetLastErpmValue(uint8_t motorIndex);
uint32_t dshotBidirGetLastRpmValue(uint8_t motorIndex);

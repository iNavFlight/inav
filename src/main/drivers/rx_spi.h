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
#pragma once

#include <stdint.h>

#define RX_SPI_MAX_PAYLOAD_SIZE 35

bool rxSpiDeviceInit(void);
uint8_t rxSpiTransferByte(uint8_t data);
void rxSpiWriteByte(uint8_t data);
void rxSpiWriteCommand(uint8_t command, uint8_t data);
void rxSpiWriteCommandMulti(uint8_t command, const uint8_t *data, uint8_t length);
uint8_t rxSpiReadCommand(uint8_t command, uint8_t commandData);
void rxSpiReadCommandMulti(uint8_t command, uint8_t commandData, uint8_t *retData, uint8_t length);

void rxSpiExtiInit(void);
bool rxSpiExtiConfigured(void);
bool rxSpiGetExtiState(void);
bool rxSpiPollExti(void);
void rxSpiResetExti(void);

bool rxSpiCheckIrq(void);


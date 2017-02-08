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

#include "io/serial.h"
#define IBUS_CHECKSUM_SIZE (2)
#if defined(TELEMETRY) && defined(TELEMETRY_IBUS)
static uint8_t respondToIbusRequest(uint8_t ibusPacket[static IBUS_RX_BUF_LEN]);
void initSharedIbusTelemetry(serialPort_t *port);
#endif //defined(TELEMETRY) && defined(TELEMETRY_IBUS)
static bool isChecksumOk(uint8_t ibusPacket[static IBUS_CHECKSUM_SIZE], size_t packetLength, uint16_t calculatedChecksum);

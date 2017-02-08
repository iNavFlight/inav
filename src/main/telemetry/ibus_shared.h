#pragma once

#include "io/serial.h"
#define IBUS_CHECKSUM_SIZE (2)
#if defined(TELEMETRY) && defined(TELEMETRY_IBUS)
static uint8_t respondToIbusRequest(uint8_t ibusPacket[static IBUS_RX_BUF_LEN]);
void initSharedIbusTelemetry(serialPort_t *port);
#endif //defined(TELEMETRY) && defined(TELEMETRY_IBUS)
static bool isChecksumOk(uint8_t ibusPacket[static IBUS_CHECKSUM_SIZE], size_t packetLength, uint16_t calculatedChecksum);

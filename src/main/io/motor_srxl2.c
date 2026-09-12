/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * SRXL2 bus master for Spektrum Smart ESCs ("Smart Throttle").
 *
 * Wire format and state machine follow "Specification for Spektrum SRXL2",
 * Rev K (https://github.com/SpektrumRC/SRXL2). Packet structures are the ones
 * INAV already carries in rx/srxl2_types.h for the receiver side, so the two
 * ends of the protocol share one definition.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_MOTOR_SRXL2

#include "build/debug.h"

#include "common/crc.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "io/serial.h"
#include "io/motor_srxl2.h"

#include "rx/srxl2_types.h"

/*---------------------------------------------------------------------------
 * Wire constants
 *-------------------------------------------------------------------------*/

#define SRXL2_MAGIC                 0xA6
#define SRXL2_MAX_FRAME             80

#define SRXL2_BAUD_LOW              115200
#define SRXL2_BAUD_HIGH             400000
#define SRXL2_BAUD_BIT_400K         0x01    /* baudSupported bit for 400000 */

#define SRXL2_PORT_OPTIONS          (SERIAL_STOPBITS_1 | SERIAL_PARITY_NO | SERIAL_BIDIR)

/* Device IDs. rx/srxl2_types.h names the flight controller range; the ESC range
 * is the adjacent one in the same table of the specification. */
#define SRXL2_ESC_ID_FIRST          0x40
#define SRXL2_ESC_ID_LAST           0x4F
#define SRXL2_OUR_DEVICE_ID         FlightControllerDefault     /* 0x30 */

/* Control Data commands */
#define SRXL2_CMD_CHANNEL_DATA      0x00
#define SRXL2_CMD_CHANNEL_FAILSAFE  0x01

/* X-Bus telemetry sensor IDs */
#define SRXL2_TELEM_SENSOR_ESC      0x20

/*
 * Specification 7.2.1: a master that shares its UART line with a throttle PWM
 * line must not speak first. It waits for a slave handshake, and if none
 * arrives within this window it concludes there is no SRXL2 device present.
 * Sending a handshake into a plain PWM ESC is explicitly called out in the spec
 * as able to cause "unintended movement or erratic behavior", so this silence
 * is a safety property, not an optimisation.
 */
#define SRXL2_LISTEN_WINDOW_MS      200

/* Spec 7.2.1 step 3: an unprompted slave repeats its handshake every 50 ms. */
#define SRXL2_HANDSHAKE_RETRY_MS    50

/* Declare the link dead if the ESC stops answering for this long. */
#define SRXL2_LINK_TIMEOUT_MS       500

/* Telemetry older than this is reported as stale rather than current. */
#define SRXL2_TELEM_STALE_MS        1000

/*
 * Channel value scaling, the exact inverse of what rx/srxl2.c applies when it
 * decodes channel data: us = 988 + (value >> 6). 1500 us therefore maps onto
 * 0x8000, which the specification calls "Servo Center".
 */
#define SRXL2_PULSE_OFFSET_US       988
#define SRXL2_PULSE_SHIFT           6

/* Throttle is channel index 0 by Spektrum convention. */
#define SRXL2_CHANNEL_THROTTLE      0

/*---------------------------------------------------------------------------
 * State
 *-------------------------------------------------------------------------*/

typedef enum {
    SRXL2_MOTOR_DISABLED = 0,
    SRXL2_MOTOR_LISTENING,      /* silent, waiting for a slave to announce itself */
    SRXL2_MOTOR_ABSENT,         /* listen window expired, no SRXL2 device on the wire */
    SRXL2_MOTOR_HANDSHAKING,
    SRXL2_MOTOR_RUNNING,
} srxl2MotorState_e;

typedef struct {
    uint8_t  buf[SRXL2_MAX_FRAME];
    uint8_t  len;
    uint8_t  expected;          /* 0 until the length byte has been seen */
} srxl2RxAssembler_t;

static serialPort_t       *srxl2Port = NULL;
static srxl2MotorState_e   srxl2State = SRXL2_MOTOR_DISABLED;
static srxl2RxAssembler_t  rxAsm;

static timeMs_t  stateEnteredMs;
static timeMs_t  lastRxMs;
static timeMs_t  lastHandshakeTxMs;

static uint8_t   escDeviceId;            /* 0 until discovered */
static uint8_t   escBaudSupported;
static uint32_t  negotiatedBaud = SRXL2_BAUD_LOW;

static uint16_t  channelValue[32];
static uint32_t  channelMask;
static bool      failsafeActive;
static uint8_t   reverseChannel1Based = 5;   /* Avian default: "Thrust Rev." = CH5 */

static srxl2EscTelemetry_t escTelemetry;

/* Diagnostics, surfaced through DEBUG_SET so a bench session can see the link
 * without a debugger attached. */
static uint32_t statTxFrames, statRxFrames, statCrcErrors, statHandshakes;

/*---------------------------------------------------------------------------
 * Helpers
 *-------------------------------------------------------------------------*/

static inline uint16_t srxl2UsToValue(uint16_t us)
{
    if (us < SRXL2_PULSE_OFFSET_US) {
        us = SRXL2_PULSE_OFFSET_US;
    }
    const uint32_t v = ((uint32_t)(us - SRXL2_PULSE_OFFSET_US)) << SRXL2_PULSE_SHIFT;
    return (v > 0xFFFC) ? 0xFFFC : (uint16_t)v;
}

static void srxl2SetState(srxl2MotorState_e next)
{
    srxl2State = next;
    stateEnteredMs = millis();
}

/* Append the CRC and push the frame. Payload length must already be written
 * into buf[2] per the specification's framing. */
static void srxl2SendFrame(uint8_t *buf, uint8_t len)
{
    if (!srxl2Port || len < 5 || len > SRXL2_MAX_FRAME) {
        return;
    }

    const uint16_t crc = crc16_ccitt_update(0, buf, len - 2);
    buf[len - 2] = (uint8_t)(crc >> 8);
    buf[len - 1] = (uint8_t)(crc & 0xFF);

    serialWriteBuf(srxl2Port, buf, len);
    statTxFrames++;
}

static void srxl2SendHandshake(uint8_t destinationId, uint8_t baudSupported)
{
    uint8_t buf[sizeof(Srxl2HandshakeFrame)];
    Srxl2HandshakeFrame *f = (Srxl2HandshakeFrame *)buf;

    f->header.id = SRXL2_MAGIC;
    f->header.packetType = Handshake;
    f->header.length = sizeof(Srxl2HandshakeFrame);

    f->payload.sourceDeviceId = SRXL2_OUR_DEVICE_ID;
    f->payload.destinationDeviceId = destinationId;
    f->payload.priority = 10;
    f->payload.baudSupported = baudSupported;
    f->payload.info = 0;            /* non-RF device, no telemetry over RF */
    /* Unique ID only has to make a simultaneous-reply collision improbable. */
    f->payload.uniqueId = 0x494E4156;   /* "INAV" */

    srxl2SendFrame(buf, sizeof(Srxl2HandshakeFrame));
}

/*---------------------------------------------------------------------------
 * Telemetry decoding: STRU_TELE_ESC, big-endian on the wire
 *-------------------------------------------------------------------------*/

static inline uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((p[0] << 8) | p[1]);
}

static void srxl2DecodeEscTelemetry(const uint8_t *payload)
{
    /* payload[0] is the sensor id, payload[1] a secondary id. */
    const uint16_t rpm        = be16(&payload[2]);
    const uint16_t voltsIn    = be16(&payload[4]);
    const uint16_t tempFet    = be16(&payload[6]);
    const uint16_t currentMot = be16(&payload[8]);
    const uint16_t tempBec    = be16(&payload[10]);
    const uint8_t  currentBec = payload[12];
    const uint8_t  voltsBec   = payload[13];
    const uint8_t  throttle   = payload[14];
    const uint8_t  powerOut   = payload[15];

    memset(&escTelemetry, 0, sizeof(escTelemetry));

    /* 0xFFFF / 0xFF mean "no data" and must not be mistaken for a reading. */
    if (rpm != 0xFFFF)        { escTelemetry.rpm = (uint32_t)rpm * 10; }
    if (voltsIn != 0xFFFF)    { escTelemetry.voltage = voltsIn; }                 /* already 0.01 V */
    if (currentMot != 0xFFFF) { escTelemetry.current = currentMot; }              /* 10 mA == 0.01 A */
    if (tempFet != 0xFFFF)    { escTelemetry.temperatureFet = (int16_t)tempFet; } /* 0.1 degC */
    if (tempBec != 0xFFFF)    { escTelemetry.temperatureBec = (int16_t)tempBec; }
    if (currentBec != 0xFF)   { escTelemetry.currentBec = (uint16_t)currentBec * 10; }  /* 100 mA -> 0.01 A */
    if (voltsBec != 0xFF)     { escTelemetry.voltageBec = (uint16_t)voltsBec * 5; }     /* 0.05 V -> 0.01 V */
    if (throttle != 0xFF)     { escTelemetry.throttlePercent = MIN((uint8_t)(throttle / 2), 100); }
    if (powerOut != 0xFF)     { escTelemetry.powerPercent = MIN((uint8_t)(powerOut / 2), 100); }

    escTelemetry.lastUpdateMs = millis();
    escTelemetry.valid = true;
}

/*---------------------------------------------------------------------------
 * Received frame handling
 *-------------------------------------------------------------------------*/

static void srxl2HandleHandshake(const uint8_t *buf)
{
    const Srxl2HandshakeFrame *f = (const Srxl2HandshakeFrame *)buf;
    const uint8_t src = f->payload.sourceDeviceId;

    if (src < SRXL2_ESC_ID_FIRST || src > SRXL2_ESC_ID_LAST) {
        /* Something else on the bus. We only drive ESCs here. */
        return;
    }

    escDeviceId = src;
    escBaudSupported = f->payload.baudSupported;
    statHandshakes++;

    /* Answer the slave, then tell the whole bus which baud rate we settled on.
     * Only claim 400000 if both ends offer it. */
    const uint8_t baud = (escBaudSupported & SRXL2_BAUD_BIT_400K) ? SRXL2_BAUD_BIT_400K : 0;

    srxl2SendHandshake(escDeviceId, baud);
    srxl2SendHandshake(Broadcast, baud);

    negotiatedBaud = baud ? SRXL2_BAUD_HIGH : SRXL2_BAUD_LOW;
    if (negotiatedBaud != SRXL2_BAUD_LOW) {
        serialSetBaudRate(srxl2Port, negotiatedBaud);
    }

    srxl2SetState(SRXL2_MOTOR_RUNNING);
}

static void srxl2HandleTelemetry(const uint8_t *buf, uint8_t len)
{
    /* Telemetry packet: header(3) + destDeviceId(1) + 16 byte payload + crc(2) */
    if (len < 3 + 1 + 16 + 2) {
        return;
    }
    const uint8_t *payload = &buf[4];
    if (payload[0] == SRXL2_TELEM_SENSOR_ESC) {
        srxl2DecodeEscTelemetry(payload);
    }
}

static void srxl2HandleFrame(const uint8_t *buf, uint8_t len)
{
    const uint16_t crc = crc16_ccitt_update(0, buf, len - 2);
    if (buf[len - 2] != (uint8_t)(crc >> 8) || buf[len - 1] != (uint8_t)(crc & 0xFF)) {
        statCrcErrors++;
        return;
    }

    statRxFrames++;
    lastRxMs = millis();

    switch (buf[1]) {
    case Handshake:
        srxl2HandleHandshake(buf);
        break;
    case TelemetrySensorData:
        srxl2HandleTelemetry(buf, len);
        break;
    default:
        break;
    }
}

static void srxl2DrainRx(void)
{
    while (serialRxBytesWaiting(srxl2Port)) {
        const uint8_t c = serialRead(srxl2Port);

        if (rxAsm.len == 0) {
            if (c != SRXL2_MAGIC) {
                continue;       /* resynchronise on the magic byte */
            }
            rxAsm.expected = 0;
        }

        rxAsm.buf[rxAsm.len++] = c;

        if (rxAsm.len == 3) {
            rxAsm.expected = rxAsm.buf[2];
            if (rxAsm.expected < 5 || rxAsm.expected > SRXL2_MAX_FRAME) {
                rxAsm.len = 0;  /* bogus length, drop and resynchronise */
                continue;
            }
        }

        if (rxAsm.expected && rxAsm.len >= rxAsm.expected) {
            srxl2HandleFrame(rxAsm.buf, rxAsm.len);
            rxAsm.len = 0;
            rxAsm.expected = 0;
        }
    }
}

/*---------------------------------------------------------------------------
 * Public API
 *-------------------------------------------------------------------------*/

bool srxl2MotorInitialize(void)
{
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_ESC_SRXL2);
    if (!portConfig) {
        return false;
    }

    srxl2Port = openSerialPort(portConfig->identifier, FUNCTION_ESC_SRXL2, NULL, NULL,
                               SRXL2_BAUD_LOW, MODE_RXTX, SRXL2_PORT_OPTIONS);
    if (!srxl2Port) {
        return false;
    }

    memset(&rxAsm, 0, sizeof(rxAsm));
    memset(&escTelemetry, 0, sizeof(escTelemetry));
    memset(channelValue, 0, sizeof(channelValue));
    channelMask = 0;
    escDeviceId = 0;
    failsafeActive = false;
    negotiatedBaud = SRXL2_BAUD_LOW;

    /* Silent until a slave speaks - see SRXL2_LISTEN_WINDOW_MS. */
    srxl2SetState(SRXL2_MOTOR_LISTENING);
    lastRxMs = millis();

    return true;
}

void srxl2MotorUpdate(uint8_t index, uint16_t value)
{
    if (index >= SRXL2_ESC_MAX_MOTORS) {
        return;
    }
    /* One ESC per bus, so motor 0 is the throttle channel. */
    channelValue[SRXL2_CHANNEL_THROTTLE] = srxl2UsToValue(value);
    channelMask |= (1u << SRXL2_CHANNEL_THROTTLE);
}

void srxl2MotorSetReverse(bool armed)
{
    if (reverseChannel1Based == 0) {
        return;     /* reverse not configured */
    }
    const uint8_t idx = reverseChannel1Based - 1;
    if (idx >= 32) {
        return;
    }
    channelValue[idx] = srxl2UsToValue(armed ? 2000 : 1000);
    channelMask |= (1u << idx);
}

void srxl2MotorSetReverseChannel(uint8_t channel1Based)
{
    reverseChannel1Based = channel1Based;
}

void srxl2MotorSetFailsafe(bool failsafe)
{
    failsafeActive = failsafe;
}

void srxl2MotorSendUpdate(void)
{
    if (srxl2State != SRXL2_MOTOR_RUNNING || !escDeviceId) {
        return;
    }

    uint8_t buf[SRXL2_MAX_FRAME];
    uint8_t n = 0;

    buf[n++] = SRXL2_MAGIC;
    buf[n++] = ControlData;
    buf[n++] = 0;                               /* length, patched below */
    buf[n++] = failsafeActive ? SRXL2_CMD_CHANNEL_FAILSAFE : SRXL2_CMD_CHANNEL_DATA;
    buf[n++] = escDeviceId;                     /* replyId: ask this ESC for telemetry */

    buf[n++] = 0;                               /* rssi: we are not an RF device */
    buf[n++] = 0;                               /* frameLosses low */
    buf[n++] = 0;                               /* frameLosses high */

    buf[n++] = (uint8_t)(channelMask & 0xFF);
    buf[n++] = (uint8_t)((channelMask >> 8) & 0xFF);
    buf[n++] = (uint8_t)((channelMask >> 16) & 0xFF);
    buf[n++] = (uint8_t)((channelMask >> 24) & 0xFF);

    for (uint8_t ch = 0; ch < 32; ch++) {
        if (channelMask & (1u << ch)) {
            buf[n++] = (uint8_t)(channelValue[ch] & 0xFF);
            buf[n++] = (uint8_t)(channelValue[ch] >> 8);
        }
    }

    n += 2;                                     /* room for the CRC */
    buf[2] = n;
    srxl2SendFrame(buf, n);
}

void srxl2MotorProcess(void)
{
    if (!srxl2Port || srxl2State == SRXL2_MOTOR_DISABLED) {
        return;
    }

    srxl2DrainRx();

    const timeMs_t now = millis();

    switch (srxl2State) {
    case SRXL2_MOTOR_LISTENING:
        /* Deliberately transmit nothing. If the window closes without a slave
         * handshake, there is no SRXL2 device here and we stay quiet for good
         * rather than risk driving a PWM ESC with serial data. */
        if (now - stateEnteredMs >= SRXL2_LISTEN_WINDOW_MS) {
            srxl2SetState(SRXL2_MOTOR_ABSENT);
        }
        break;

    case SRXL2_MOTOR_ABSENT:
        /* A slave can still appear later - it repeats its handshake on reset -
         * and handling that costs nothing, so keep listening without speaking. */
        break;

    case SRXL2_MOTOR_HANDSHAKING:
        if (now - lastHandshakeTxMs >= SRXL2_HANDSHAKE_RETRY_MS) {
            lastHandshakeTxMs = now;
            srxl2SendHandshake(Broadcast, SRXL2_BAUD_BIT_400K);
        }
        break;

    case SRXL2_MOTOR_RUNNING:
        if (now - lastRxMs >= SRXL2_LINK_TIMEOUT_MS) {
            /* Lost the ESC. Drop back to 115200, which is where a slave that
             * has just reset will be listening, and wait for it again. */
            serialSetBaudRate(srxl2Port, SRXL2_BAUD_LOW);
            negotiatedBaud = SRXL2_BAUD_LOW;
            escDeviceId = 0;
            escTelemetry.valid = false;
            srxl2SetState(SRXL2_MOTOR_LISTENING);
        }
        break;

    default:
        break;
    }

    DEBUG_SET(DEBUG_ALWAYS, 0, srxl2State);
    DEBUG_SET(DEBUG_ALWAYS, 1, escDeviceId);
    DEBUG_SET(DEBUG_ALWAYS, 2, statRxFrames);
    DEBUG_SET(DEBUG_ALWAYS, 3, statCrcErrors);
}

bool srxl2MotorIsConnected(void)
{
    return srxl2State == SRXL2_MOTOR_RUNNING && escDeviceId != 0;
}

bool srxl2MotorGetTelemetry(uint8_t index, srxl2EscTelemetry_t *out)
{
    if (index >= SRXL2_ESC_MAX_MOTORS || !out || !escTelemetry.valid) {
        return false;
    }
    if (millis() - escTelemetry.lastUpdateMs > SRXL2_TELEM_STALE_MS) {
        return false;
    }
    *out = escTelemetry;
    return true;
}

#endif /* USE_MOTOR_SRXL2 */

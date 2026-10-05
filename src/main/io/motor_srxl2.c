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
 * SRXL2 bus master for Spektrum Smart ESCs ("Smart Throttle"), following
 * "Specification for Spektrum SRXL2" Rev K (https://github.com/SpektrumRC/SRXL2).
 * Packet structures come from rx/srxl2_types.h, shared with the receiver side.
 *
 * The specification describes the bus and says nothing about any particular ESC.
 * What an ESC decides for itself is kept to the named constants and the
 * reverse-channel setter below, so what needs confirming against real hardware
 * is countable. docs/Spektrum Smart ESC.md has the bench measurements.
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

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/runtime_config.h"

#include "sensors/battery.h"

#include "io/serial.h"
#include "io/motor_srxl2.h"

#include "rx/srxl2_types.h"

/*---------------------------------------------------------------------------
 * Wire constants
 *-------------------------------------------------------------------------*/

#define SRXL2_MAGIC                 0xA6
#define SRXL2_MAX_FRAME             80
#define SRXL2_MIN_FRAME             5

#define SRXL2_BAUD_LOW              115200
#define SRXL2_BAUD_HIGH             400000
#define SRXL2_BAUD_BIT_400K         0x01    /* baudSupported bit for 400000 */

#define SRXL2_PORT_OPTIONS          (SERIAL_STOPBITS_1 | SERIAL_PARITY_NO | SERIAL_BIDIR)

/* ESC device IDs. rx/srxl2_types.h names the flight controller range; this is
 * the adjacent range in the same table of the specification. */
#define SRXL2_ESC_ID_FIRST          0x40
#define SRXL2_ESC_ID_LAST           0x4F

// Flight controller type, unit ID 1. Not unit 0: specification 7.1.1 has a device whose
// lower nibble is 0 announce itself unprompted, which is the slave's behaviour, and a
// master doing it would collide with the ESC's own announcements
#define SRXL2_OUR_DEVICE_ID         0x31

// The bus picks its master by lowest device ID and we never implement standing down, since
// this port is a dedicated link to one ESC, which cannot outrank us from 0x40. It would be
// wrong on a bus shared with a Spektrum receiver, which is master at 0x21

// The protocol's failsafe channel-data command, 0x01, is deliberately never sent: INAV owns
// failsafe and keeps commanding the motors all the way down, so handing throttle authority
// to the ESC mid-landing would be the opposite of helpful. A dead wire is covered by the
// ESC's own receive timeout, which needs nothing from us
#define SRXL2_CMD_CHANNEL_DATA      0x00

/* Reply ID 0x00 means "no reply wanted" (specification 7.1.1). */
#define SRXL2_REPLY_NONE            0x00

// The frame carries the throttle and, when configured, the reverse channel. Nothing else:
// the ESC reads CH1 and the channel its own Thrust Rev. parameter names. How many channels
// the mask holds makes no difference to the ESC, measured from one to eight; the telemetry
// request rate does, see SRXL2_TELEM_REQUEST_MIN

/* X-Bus telemetry sensor IDs */
#define SRXL2_TELEM_SENSOR_ESC      0x20

// Specification 7.2.1: an ESC with unit ID 0 repeats an unprompted handshake every 50 ms
// for the first 200 ms after reset, so listen a little longer than that before polling. An
// ESC with a non-zero unit ID never announces itself and would otherwise never be found
#define SRXL2_LISTEN_WINDOW_MS      250
#define SRXL2_HANDSHAKE_INTERVAL_MS 50

// Rate at which Control Data goes out once the link is up, which is not the motor update
// rate: the specification has the master emit one packet per RF frame, tens of hertz. 115200
// baud would not carry more anyway, an eighteen-byte frame being about 1.6 ms on the wire
#define SRXL2_CONTROL_INTERVAL_MS   20      /* 50 Hz */

// Ask for telemetry every Nth Control Data packet: a reply forces a half-duplex turnaround
// for values that change slowly. Every fifth frame at 50 Hz asks ten times a second, which
// arrives as about 1.1 ESC readings a second once the ESC's own sensor rotation is counted
#define SRXL2_TELEM_REQUEST_DEFAULT 5

// Never ask on every frame. An Avian asked at 50 Hz answers, and its telemetry is correct,
// but it stops obeying the throttle and reports zero per cent from every stick position.
// Every 2nd, 3rd and 4th frame all behave, and dropping back recovers it without a power
// cycle. Measured on an Avian 70A against a fixed 1250 us command
#define SRXL2_TELEM_REQUEST_MIN     2

// Declare the link dead if the ESC stops answering for this long. A running Avian never
// speaks unprompted, so only a telemetry reply refreshes this timer and a request rate
// slower than the timeout would disconnect a healthy ESC on schedule. It answers about two
// requests in three, so the margin also has to cover consecutive misses. This is why the
// telemetry rate table stops at one request every five frames
#define SRXL2_LINK_TIMEOUT_MS       500

// How long after the link comes up before the ESC will actually turn the motor. An Avian
// announces itself within 300 ms of gaining power but then plays its startup tones for about
// five seconds, and only drives the motor once the last of them has sounded. Timed on the
// bench against the tones. Arming waits for this rather than for the handshake alone, which
// costs nothing: nobody arms that soon after connecting the battery
#define SRXL2_READY_DELAY_MS        6500

// Telemetry older than this reads as stale. Generous next to the link timeout on purpose:
// the ESC rotates its reply between three sensors, so its own readings arrive about once a
// second and a tighter window made a healthy sensor flicker. An ESC that has actually
// stopped is caught by SRXL2_LINK_TIMEOUT_MS, which invalidates the reading anyway
#define SRXL2_TELEM_STALE_MS        3000

// Calibration phases end themselves: the high one has to outlast a person reaching for a
// battery lead, the low one only the ESC's tones. Neither may persist, one is full throttle
#define SRXL2_CAL_WAIT_TIMEOUT_MS   60000   /* time to walk over and plug the battery in */
#define SRXL2_CAL_MANUAL_TIMEOUT_MS 30000

// How long to hold full throttle after the ESC gains power, then how long to hold minimum.
// The published tone timings suggest three and five seconds; measured on an Avian 70A that
// lands on the edge of the window and stores nothing, while four and seven store every time.
// The extra second either side costs nothing: the sequence runs once, on a bench, with the
// propeller off. docs/Spektrum Smart ESC.md has the before and after figures
#define SRXL2_CAL_SETTLE_MS         4000

/* Long enough for the cell-count tones and the closing long tone. */
#define SRXL2_CAL_LOW_MS            7000

/* Endpoints presented during calibration, on INAV's usual motor scale. */
#define SRXL2_CAL_HIGH_US           2000
#define SRXL2_CAL_LOW_US            1000

// Channel value scaling, the exact inverse of what rx/srxl2.c applies when it decodes:
// us = 988 + (value >> 6). 1500 us lands on 0x8000, which the specification calls Servo
// Center, and the shift leaves the low two bits clear as it requires. It deliberately stops
// short of the ends of the 0..65532 range, because a Spektrum receiver's full travel decodes
// to 988..2012 us, and looking like a receiver is what an ESC's stored endpoints expect.
// The 1.2 % of travel this costs at the top beats stretching 1000..2000 over the full range,
// which would move the centre, and the centre is where Reverse brake mode takes no thrust
#define SRXL2_PULSE_OFFSET_US       988
#define SRXL2_PULSE_SHIFT           6
#define SRXL2_VALUE_MAX             0xFFFC  /* specification caps values here */

// Throttle is channel index 0 by Spektrum convention, confirmed against an Avian 70A
#define SRXL2_CHANNEL_THROTTLE      0

// How many channels the driver keeps values for. The wire's mask is 32 bits wide, but only
// the throttle and the reverse channel are ever written, and esc_srxl2_reverse_channel
// cannot name anything above 9
#define SRXL2_CHANNEL_COUNT         10

/*---------------------------------------------------------------------------
 * State
 *-------------------------------------------------------------------------*/

typedef enum {
    SRXL2_DISABLED = 0,
    SRXL2_LISTENING,        /* silent, giving an auto-announcing ESC room to speak */
    SRXL2_POLLING,          /* actively asking the ESC device ID to answer */
    SRXL2_FINALISING,       /* broadcasting the agreed baud rate */
    SRXL2_RUNNING,
} srxl2State_e;

// One per ESC. Each is an independent bus with its own handshake, baud negotiation, framing
// and telemetry; nothing is shared but our own device ID, which is safe precisely because
// the buses never hear each other
typedef struct {
    serialPort_t  *port;
    srxl2State_e   state;

    uint8_t   rxBuf[SRXL2_MAX_FRAME];
    uint8_t   rxLen;
    uint8_t   rxExpected;

    timeMs_t  stateEnteredMs;
    timeMs_t  lastRxMs;
    timeMs_t  lastTxMs;
    timeMs_t  lastControlMs;

    uint8_t   deviceId;                 /* 0 until discovered */
    uint8_t   baudSupported;
    uint8_t   pollId;           /* offset from SRXL2_ESC_ID_FIRST, while polling */
    timeMs_t  runningSinceMs;   /* when the link came up, for SRXL2_READY_DELAY_MS */
    timeMs_t  lastKeepaliveMs;  /* last handshake answered to a running ESC */
    uint8_t   agreedBaudBits;
    bool      baudSwitchPending;        /* waiting for TX to drain */

    uint16_t  channelValue[SRXL2_CHANNEL_COUNT];
    uint32_t  channelMask;
    uint8_t   telemRequestCounter;

    srxl2EscTelemetry_t telemetry;

    uint32_t  statTxFrames, statRxFrames, statCrcErrors, statHandshakes;
    uint32_t  statEchoFrames;           /* our own frames heard back on a single wire */
} srxl2Esc_t;

static srxl2Esc_t esc[SRXL2_ESC_MAX_MOTORS];
static uint8_t    escCount;                 /* ports successfully opened */

/* Shared, because these describe the aircraft rather than one bus. */
static uint8_t   reverseChannel1Based = 7;  /* Spektrum ship "Thrust Rev." on CH7 */

// Control frames between telemetry requests: at a 20 ms interval, 5 asks at 10 Hz. Held as
// a divisor rather than a rate because that is what the transmit path counts
static uint8_t   telemRequestEvery = SRXL2_TELEM_REQUEST_DEFAULT;

static srxl2CalPhase_e calPhase = SRXL2_CAL_OFF;
static srxl2CalResult_e calLastResult = SRXL2_CAL_ACCEPTED;
static timeMs_t        calPhaseMs;       /* when the current phase began */
static uint32_t        calHandshakeMark; /* total handshakes when the phase began */

/*---------------------------------------------------------------------------
 * Helpers
 *-------------------------------------------------------------------------*/

static inline uint16_t srxl2UsToValue(uint16_t us)
{
    if (us < SRXL2_PULSE_OFFSET_US) {
        us = SRXL2_PULSE_OFFSET_US;
    }
    const uint32_t v = ((uint32_t)(us - SRXL2_PULSE_OFFSET_US)) << SRXL2_PULSE_SHIFT;
    return (v > SRXL2_VALUE_MAX) ? SRXL2_VALUE_MAX : (uint16_t)v;
}

static inline uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((p[0] << 8) | p[1]);
}

static bool srxl2ReverseChannelUsable(uint8_t channel1Based);

// Rebuilt rather than accumulated, so the frame does not change shape depending on what has
// been called since power-up, and so that clearing the reverse channel stops sending it
static void srxl2BuildChannelMask(srxl2Esc_t *e)
{
    uint32_t mask = 1u << SRXL2_CHANNEL_THROTTLE;
    if (srxl2ReverseChannelUsable(reverseChannel1Based)) {
        mask |= 1u << (reverseChannel1Based - 1);
    }
    e->channelMask = mask;
}

static void srxl2SetState(srxl2Esc_t *e, srxl2State_e next)
{
    e->state = next;
    e->stateEnteredMs = millis();
}

// Handshakes seen across every bus. The calibration watches this to notice an ESC gaining
// power, and the first to speak is signal enough: they all share a pack
static uint32_t srxl2TotalHandshakes(void)
{
    uint32_t total = 0;
    for (uint8_t i = 0; i < escCount; i++) {
        total += esc[i].statHandshakes;
    }
    return total;
}

// Append the CRC and push the frame; buf[2] must already hold the length. Returns false when
// the frame did not go out, which matters for one frame only: a dropped Control Data is of
// no consequence, another follows in 20 ms, but raising the baud rate while only believing
// the broadcast was sent leaves the ESC behind at the old rate with no way back short of a
// power cycle. Seen on real hardware, and not recoverable in flight
static bool srxl2SendFrame(srxl2Esc_t *e, uint8_t *buf, uint8_t len)
{
    if (!e->port || len < SRXL2_MIN_FRAME || len > SRXL2_MAX_FRAME) {
        return false;
    }

    if (serialTxBytesFree(e->port) < len) {
        return false;
    }

    const uint16_t crc = crc16_ccitt_update(0, buf, len - 2);
    buf[len - 2] = (uint8_t)(crc >> 8);
    buf[len - 1] = (uint8_t)(crc & 0xFF);

    serialWriteBuf(e->port, buf, len);
    e->lastTxMs = millis();
    e->statTxFrames++;
    return true;
}

static bool srxl2SendHandshake(srxl2Esc_t *e, uint8_t destinationId, uint8_t baudField)
{
    uint8_t buf[sizeof(Srxl2HandshakeFrame)];
    Srxl2HandshakeFrame *f = (Srxl2HandshakeFrame *)buf;

    f->header.id = SRXL2_MAGIC;
    f->header.packetType = Handshake;
    f->header.length = sizeof(Srxl2HandshakeFrame);

    f->payload.sourceDeviceId = SRXL2_OUR_DEVICE_ID;
    f->payload.destinationDeviceId = destinationId;
    f->payload.priority = 10;
    /* When polling a specific device this advertises what we can do; in the
     * broadcast it states what every device must switch to. */
    f->payload.baudSupported = baudField;
    f->payload.info = 0;                    /* non-RF device, no RF telemetry */
    f->payload.uniqueId = 0x494E4156;       /* "INAV"; only has to make a
                                             * simultaneous-reply collision
                                             * improbable. The same value on
                                             * every bus is fine, because each
                                             * bus has exactly one master. */

    return srxl2SendFrame(e, buf, sizeof(Srxl2HandshakeFrame));
}

/* Tell the bus which rate everyone moves to, and arrange to follow once the
 * frame has actually left the port. Switching immediately would clock the tail
 * of that very frame out at the new rate and lose it. */
static void srxl2Finalise(srxl2Esc_t *e)
{
    e->agreedBaudBits = SRXL2_BAUD_BIT_400K & e->baudSupported;

    /* Only arm the switch if the broadcast is actually on its way. If the port
     * had no room, stay where we are and try again on the next pass: a rate the
     * ESC was never told about is worse than a slow negotiation. */
    if (!srxl2SendHandshake(e, Broadcast, e->agreedBaudBits)) {
        return;
    }

    e->baudSwitchPending = (e->agreedBaudBits & SRXL2_BAUD_BIT_400K) != 0;
    srxl2SetState(e, SRXL2_FINALISING);
}

/*---------------------------------------------------------------------------
 * Telemetry decoding: STRU_TELE_ESC, big-endian on the wire
 *-------------------------------------------------------------------------*/

static void srxl2DecodeEscTelemetry(srxl2Esc_t *e, const uint8_t *payload)
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

    srxl2EscTelemetry_t *t = &e->telemetry;
    memset(t, 0, sizeof(*t));

    /* 0xFFFF and 0xFF mean "no data" and must not be taken for readings -
     * 0xFFFF volts at 0.01 V per count would otherwise look like 655 V. */
    if (rpm != 0xFFFF)        { t->rpm = (uint32_t)rpm * 10; t->fields |= SRXL2_TELEM_FIELD_RPM; }
    if (voltsIn != 0xFFFF)    { t->voltage = voltsIn; t->fields |= SRXL2_TELEM_FIELD_VOLTAGE; }
    if (currentMot != 0xFFFF) { t->current = currentMot; t->fields |= SRXL2_TELEM_FIELD_CURRENT; }
    if (tempFet != 0xFFFF)    { t->temperatureFet = (int16_t)tempFet; t->fields |= SRXL2_TELEM_FIELD_TEMP_FET; }
    if (tempBec != 0xFFFF)    { t->temperatureBec = (int16_t)tempBec; t->fields |= SRXL2_TELEM_FIELD_TEMP_BEC; }
    if (currentBec != 0xFF)   { t->currentBec = (uint16_t)currentBec * 10; } /* 100 mA -> 0.01 A */
    if (voltsBec != 0xFF)     { t->voltageBec = (uint16_t)voltsBec * 5; }    /* 0.05 V -> 0.01 V */
    if (throttle != 0xFF)     { t->throttlePercent = MIN((uint8_t)(throttle / 2), 100); }
    if (powerOut != 0xFF)     { t->powerPercent = MIN((uint8_t)(powerOut / 2), 100); }

    t->lastUpdateMs = millis();
    t->valid = true;
}

/*---------------------------------------------------------------------------
 * Received frame handling
 *-------------------------------------------------------------------------*/

static void srxl2HandleHandshake(srxl2Esc_t *e, const uint8_t *buf)
{
    const Srxl2HandshakeFrame *f = (const Srxl2HandshakeFrame *)buf;
    const uint8_t src = f->payload.sourceDeviceId;

    if (src < SRXL2_ESC_ID_FIRST || src > SRXL2_ESC_ID_LAST) {
        /* Something else on the bus. This driver only speaks to ESCs. */
        return;
    }

    // Enter the negotiation once, and only from the states still looking for an ESC.
    // Restarting it on every handshake makes the two ends ping-pong: our broadcast draws a
    // handshake, which would restart the sequence, and a bus looping through FINALISING
    // never reaches its first control frame. A slave that genuinely reset is not missed,
    // since it comes back at 115200 and the link timeout drops us to POLLING to find it
    if (e->state == SRXL2_FINALISING || e->state == SRXL2_RUNNING) {
        /* Still answer a running ESC, so it knows the master is there - but say
         * nothing mid-negotiation, where another broadcast is what causes the
         * loop. */
        /* At most one of these per link timeout. A slave that answers this answer,
         * and the specification does not forbid one, would otherwise trade handshakes
         * with us as fast as the wire allows, crowding out the control frames. */
        if (e->state == SRXL2_RUNNING && e->deviceId == src
            && (millis() - e->lastKeepaliveMs) >= SRXL2_LINK_TIMEOUT_MS) {
            e->lastKeepaliveMs = millis();
            srxl2SendHandshake(e, src, SRXL2_BAUD_BIT_400K);
        }
        return;
    }

    e->deviceId = src;
    e->baudSupported = f->payload.baudSupported;

    // Counted here rather than on every handshake, so it means "a negotiation started". The
    // calibration reads it as an ESC gaining power, which a keepalive answer is not
    e->statHandshakes++;

    // Answer the slave so it knows who the master is, then finalise. This also covers the
    // ESC being powered after the flight controller, the normal case on a bench: by then we
    // are in POLLING, which accepts a handshake, so the ESC is still found
    srxl2SendHandshake(e, e->deviceId, SRXL2_BAUD_BIT_400K);
    srxl2Finalise(e);
}

static void srxl2HandleTelemetry(srxl2Esc_t *e, const uint8_t *buf, uint8_t len)
{
    /* header(3) + destDeviceId(1) + 16 byte payload + crc(2) */
    if (len < 3 + 1 + 16 + 2) {
        return;
    }
    const uint8_t *payload = &buf[4];
    if (payload[0] == SRXL2_TELEM_SENSOR_ESC) {
        srxl2DecodeEscTelemetry(e, payload);
    }
}

static void srxl2HandleFrame(srxl2Esc_t *e, const uint8_t *buf, uint8_t len)
{
    const uint16_t crc = crc16_ccitt_update(0, buf, len - 2);
    if (buf[len - 2] != (uint8_t)(crc >> 8) || buf[len - 1] != (uint8_t)(crc & 0xFF)) {
        e->statCrcErrors++;
        return;
    }

    /* On a single wire the receiver hears what this driver has just transmitted, and
     * the serial layer passes it up like anything else. Our own frames must not count
     * as the ESC answering: with the control frame going out every 20 ms, the link
     * timeout would never fire and an ESC that had been unplugged would still look
     * connected. Which frames can only be ours is known: the control data this driver
     * sends, and a handshake carrying our own source ID. */
    if (buf[1] == ControlData) {
        e->statEchoFrames++;
        return;
    }

    if (buf[1] == Handshake && len >= sizeof(Srxl2HandshakeFrame)
        && ((const Srxl2HandshakeFrame *)buf)->payload.sourceDeviceId == SRXL2_OUR_DEVICE_ID) {
        e->statEchoFrames++;
        return;
    }

    e->statRxFrames++;
    e->lastRxMs = millis();

    switch (buf[1]) {
    case Handshake:
        /* Length checked before the payload is read: a short frame that happens
         * to carry a valid CRC would otherwise have its device ID and baud
         * fields taken from whatever the receive buffer held last. */
        if (len >= sizeof(Srxl2HandshakeFrame)) {
            srxl2HandleHandshake(e, buf);
        }
        break;
    case TelemetrySensorData:
        srxl2HandleTelemetry(e, buf, len);
        break;
    default:
        break;
    }
}

static void srxl2DrainRx(srxl2Esc_t *e)
{
    while (serialRxBytesWaiting(e->port)) {
        const uint8_t c = serialRead(e->port);

        if (e->rxLen == 0) {
            if (c != SRXL2_MAGIC) {
                continue;               /* resynchronise on the magic byte */
            }
            e->rxExpected = 0;
        }

        e->rxBuf[e->rxLen++] = c;

        if (e->rxLen == 3) {
            e->rxExpected = e->rxBuf[2];
            if (e->rxExpected < SRXL2_MIN_FRAME || e->rxExpected > SRXL2_MAX_FRAME) {
                e->rxLen = 0;           /* bogus length, drop and resynchronise */
                continue;
            }
        }

        if (e->rxExpected && e->rxLen >= e->rxExpected) {
            srxl2HandleFrame(e, e->rxBuf, e->rxLen);
            e->rxLen = 0;
            e->rxExpected = 0;
        }
    }
}

/*---------------------------------------------------------------------------
 * Control data
 *-------------------------------------------------------------------------*/

static void srxl2SendControlData(srxl2Esc_t *e)
{
    uint8_t buf[SRXL2_MAX_FRAME];
    uint8_t n = 0;

    /* Request telemetry only every so often: the reply shares the wire with the
     * control data, so asking on every frame halves the headroom for no gain on a
     * sensor whose values move slowly. */
    uint8_t replyId = SRXL2_REPLY_NONE;
    if (++e->telemRequestCounter >= telemRequestEvery) {
        e->telemRequestCounter = 0;
        replyId = e->deviceId;
    }

    // Calibration substitutes the throttle as the frame is built, rather than staging it,
    // so no mixer path can write over it between the two. It substitutes rather than stores,
    // so that the phase ending is enough to restore what the mixer staged: writing into
    // channelValue would leave full throttle there until the mixer happened to run again.
    // Every ESC is calibrated at once, sharing a battery and therefore a power-up window.
    uint16_t throttle = e->channelValue[SRXL2_CHANNEL_THROTTLE];
    if (calPhase != SRXL2_CAL_OFF) {
        const bool high = (calPhase == SRXL2_CAL_WAIT_BATTERY)
                       || (calPhase == SRXL2_CAL_SETTLE)
                       || (calPhase == SRXL2_CAL_HIGH_MANUAL);
        throttle = srxl2UsToValue(high ? SRXL2_CAL_HIGH_US : SRXL2_CAL_LOW_US);
    }

    buf[n++] = SRXL2_MAGIC;
    buf[n++] = ControlData;
    buf[n++] = 0;                       /* length, patched below */
    buf[n++] = SRXL2_CMD_CHANNEL_DATA;
    buf[n++] = replyId;

    // RSSI has to read as a healthy link even though we are not an RF device: Spektrum's own
    // receiver code takes a received zero as loss of link, so sending 0 would announce a
    // failed link on every frame
    buf[n++] = 100;
    buf[n++] = 0;                       /* frameLosses low */
    buf[n++] = 0;                       /* frameLosses high */

    buf[n++] = (uint8_t)(e->channelMask & 0xFF);
    buf[n++] = (uint8_t)((e->channelMask >> 8) & 0xFF);
    buf[n++] = (uint8_t)((e->channelMask >> 16) & 0xFF);
    buf[n++] = (uint8_t)((e->channelMask >> 24) & 0xFF);

    // A contiguous block, little-endian, lowest index first. Channels the driver has nothing
    // to say on are padded at their minimum and never at centre: on an aircraft ESC 1500 us
    // is half throttle, so if the ESC read throttle on an unexpected index, centred padding
    // would spin the motor while minimum padding leaves it idle
    for (uint8_t ch = 0; ch < SRXL2_CHANNEL_COUNT; ch++) {
        if (e->channelMask & (1u << ch)) {
            const uint16_t v = (ch == SRXL2_CHANNEL_THROTTLE) ? throttle : e->channelValue[ch];
            buf[n++] = (uint8_t)(v & 0xFF);
            buf[n++] = (uint8_t)(v >> 8);
        }
    }

    n += 2;                             /* room for the CRC */
    buf[2] = n;
    srxl2SendFrame(e, buf, n);
}

/*---------------------------------------------------------------------------
 * Public API
 *-------------------------------------------------------------------------*/

bool srxl2MotorInitialize(void)
{
    memset(esc, 0, sizeof(esc));
    escCount = 0;

    // One ESC per port, so open every port assigned the function. The enumeration follows
    // UART order, so motor 1 is the lowest-numbered assigned UART: nothing on an SRXL2 bus
    // says which motor an ESC drives, so the wiring order has to carry it
    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_ESC_SRXL2);

    while (portConfig && escCount < SRXL2_ESC_MAX_MOTORS) {
        serialPort_t *port = openSerialPort(portConfig->identifier, FUNCTION_ESC_SRXL2,
                                            NULL, NULL, SRXL2_BAUD_LOW, MODE_RXTX,
                                            SRXL2_PORT_OPTIONS);
        if (port) {
            srxl2Esc_t *e = &esc[escCount++];

            e->port = port;

            /* Every channel starts at its lowest value rather than zero, so the
             * first frame after a handshake cannot be read as something
             * unexpected whichever index the ESC happens to care about. */
            for (uint8_t ch = 0; ch < SRXL2_CHANNEL_COUNT; ch++) {
                e->channelValue[ch] = srxl2UsToValue(1000);
            }
            srxl2BuildChannelMask(e);

            const timeMs_t now = millis();
            e->lastRxMs = now;
            e->lastTxMs = now;
            e->lastControlMs = now;

            srxl2SetState(e, SRXL2_LISTENING);
        }

        portConfig = findNextSerialPortConfig(FUNCTION_ESC_SRXL2);
    }

    return escCount > 0;
}

void srxl2MotorUpdate(uint8_t index, uint16_t value)
{
    if (index >= escCount) {
        /* No port for this motor. There is nothing sensible to do here - no timer
         * output to fall back on - so the shortfall is reported through
         * srxl2MotorCount() and caught at arming rather than absorbed. */
        return;
    }
    /* Staging only. The wire is driven at its own rate from srxl2MotorProcess(),
     * not at whatever rate the mixer happens to run. */
    esc[index].channelValue[SRXL2_CHANNEL_THROTTLE] = srxl2UsToValue(value);
}

// Whether a configured reverse channel can be used. Zero means the model has no reverse, and
// anything landing on the throttle channel is refused: srxl2MotorSetReverse() writes its
// channel unconditionally at task rate, so an aliased one would overwrite the staged throttle
// hundreds of times a second. The setting's own range cannot express the hole, so the check
// belongs here; the upper bound is the width of the channel array
static bool srxl2ReverseChannelUsable(uint8_t channel1Based)
{
    if (channel1Based == 0 || channel1Based > SRXL2_CHANNEL_COUNT) {
        return false;
    }
    return (channel1Based - 1) != SRXL2_CHANNEL_THROTTLE;
}

void srxl2MotorSetReverse(bool armed)
{
    if (!srxl2ReverseChannelUsable(reverseChannel1Based)) {
        return;     /* reverse not configured, or the channel is not usable */
    }
    const uint8_t idx = reverseChannel1Based - 1;
    const uint16_t v = srxl2UsToValue(armed ? 2000 : 1000);

    /* Every ESC, because the mixer decides a direction for the aircraft rather
     * than for one motor. Reversing one side of a twin and not the other is the
     * one outcome here worth engineering against. */
    for (uint8_t i = 0; i < escCount; i++) {
        esc[i].channelValue[idx] = v;
    }
}

void srxl2MotorSetReverseChannel(uint8_t channel1Based)
{
    reverseChannel1Based = srxl2ReverseChannelUsable(channel1Based) ? channel1Based : 0;

    /* The block has to reach it, and this may be called after the ports opened. */
    for (uint8_t i = 0; i < escCount; i++) {
        srxl2BuildChannelMask(&esc[i]);
    }
}

void srxl2MotorSetTelemetryRate(srxl2TelemetryRate_e rate)
{
    /* Indexed by srxl2TelemetryRate_e. Each entry is how many 50 Hz control
     * frames pass between requests; the setting is named for what comes back,
     * which is roughly a ninth of what is asked for. */
    static const uint8_t divisor[] = { 5, 2, 3 };

    uint8_t every = (rate < ARRAYLEN(divisor)) ? divisor[rate] : SRXL2_TELEM_REQUEST_DEFAULT;

    /* Clamped here as well as in the table, so that no future entry - or a
     * configuration written by an older build, where index 1 meant every
     * frame - can ask at a rate the ESC answers but will not fly at. */
    telemRequestEvery = (every < SRXL2_TELEM_REQUEST_MIN) ? SRXL2_TELEM_REQUEST_MIN : every;
}

/* Checked on both sides rather than trusting the caller, because one of these
 * phases commands full throttle with the aircraft disarmed. */
static srxl2CalResult_e srxl2CalCommonChecks(void)
{
    if (ARMING_FLAG(ARMED)) {
        return SRXL2_CAL_REJECT_ARMED;
    }
    if (escCount == 0) {
        return SRXL2_CAL_REJECT_NO_PORT;
    }
    return SRXL2_CAL_ACCEPTED;
}

srxl2CalResult_e srxl2MotorCalibrationBegin(void)
{
    const srxl2CalResult_e common = srxl2CalCommonChecks();
    if (common != SRXL2_CAL_ACCEPTED) {
        return (calLastResult = common);
    }

    /* Detecting the ESC powering up is the whole mechanism, so say so plainly
     * instead of starting a sequence that can never advance. */
    if (!isBatteryVoltageConfigured()) {
        return (calLastResult = SRXL2_CAL_REJECT_NO_VOLTAGE_SENSOR);
    }

    /*
     * Refuse if the pack is already in. The ESC only reads its endpoints as it
     * powers up, so starting with it already running would achieve nothing - and
     * it would mean presenting full throttle to an ESC that can act on it.
     */
    if (getBatteryState() != BATTERY_NOT_PRESENT) {
        return (calLastResult = SRXL2_CAL_REJECT_BATTERY_PRESENT);
    }

    calPhase = SRXL2_CAL_WAIT_BATTERY;
    calPhaseMs = millis();
    calHandshakeMark = srxl2TotalHandshakes();
    return (calLastResult = SRXL2_CAL_ACCEPTED);
}

srxl2CalResult_e srxl2MotorCalibrationManual(srxl2CalPhase_e phase)
{
    const srxl2CalResult_e common = srxl2CalCommonChecks();
    if (common != SRXL2_CAL_ACCEPTED) {
        return (calLastResult = common);
    }

    // The high phase commands full throttle, so it may not start against an ESC that already
    // has power. Boards that cannot sense the pack report it absent and are unaffected, which
    // is the case this manual path exists for
    if (phase == SRXL2_CAL_HIGH_MANUAL && getBatteryState() != BATTERY_NOT_PRESENT) {
        return (calLastResult = SRXL2_CAL_REJECT_BATTERY_PRESENT);
    }

    calPhase = phase;
    calPhaseMs = millis();
    return (calLastResult = SRXL2_CAL_ACCEPTED);
}

void srxl2MotorCalibrationAbort(void)
{
    calPhase = SRXL2_CAL_OFF;
}

srxl2CalPhase_e srxl2MotorCalibrationPhase(void)
{
    return calPhase;
}

srxl2CalResult_e srxl2MotorCalibrationLastResult(void)
{
    return calLastResult;
}

/* Advance the unattended sequence. Every phase leaves on a deadline, so nothing
 * here can strand the output at full throttle. */
static void srxl2CalProcess(timeMs_t now)
{
    if (calPhase == SRXL2_CAL_OFF) {
        return;
    }

    if (ARMING_FLAG(ARMED)) {
        calPhase = SRXL2_CAL_OFF;
        return;
    }

    const timeMs_t elapsed = now - calPhaseMs;

    switch (calPhase) {
    case SRXL2_CAL_WAIT_BATTERY:
        // What opens the window is the ESC gaining power, which is an event, so both signals
        // have to be events too: the pack appearing, or a fresh handshake on any bus. A test
        // on persistent state instead would skip the wait on any board that had already
        // talked to its ESC once
        if (getBatteryState() != BATTERY_NOT_PRESENT || srxl2TotalHandshakes() != calHandshakeMark) {
            calPhase = SRXL2_CAL_SETTLE;
            calPhaseMs = now;
        } else if (elapsed >= SRXL2_CAL_WAIT_TIMEOUT_MS) {
            calPhase = SRXL2_CAL_OFF;
        }
        break;

    case SRXL2_CAL_SETTLE:
        if (elapsed >= SRXL2_CAL_SETTLE_MS) {
            calPhase = SRXL2_CAL_LOW;
            calPhaseMs = now;
        }
        break;

    case SRXL2_CAL_LOW:
        if (elapsed >= SRXL2_CAL_LOW_MS) {
            calPhase = SRXL2_CAL_OFF;
        }
        break;

    case SRXL2_CAL_HIGH_MANUAL:
    case SRXL2_CAL_LOW_MANUAL:
        if (elapsed >= SRXL2_CAL_MANUAL_TIMEOUT_MS) {
            calPhase = SRXL2_CAL_OFF;
        }
        break;

    default:
        calPhase = SRXL2_CAL_OFF;
        break;
    }
}

void srxl2MotorSendUpdate(void)
{
    /* Nothing to do: values are staged by srxl2MotorUpdate() and transmitted on
     * the protocol's own schedule. Kept so the motor output layer can call the
     * same hook it calls for every other protocol. */
}

/* One bus, advanced by one tick. */
static void srxl2ProcessEsc(srxl2Esc_t *e, timeMs_t now)
{
    srxl2DrainRx(e);

    // A frame handled in the line above stamps itself with a time taken after the one this
    // cycle began with. Left alone, the unsigned difference against that older stamp wraps,
    // the reading it just brought reads as 49 days old, and the telemetry is thrown away
    // until the next frame arrives. Measured on an Avian: about one frame in a hundred, and
    // with the ESC answering roughly twice a second, up to a second of telemetry lost each
    // time. So the time is taken again, now that everything received has been accounted for
    now = millis();

    /* A deferred baud change completes as soon as the broadcast has left. */
    if (e->baudSwitchPending && isSerialTransmitBufferEmpty(e->port)) {
        serialSetBaudRate(e->port, SRXL2_BAUD_HIGH);
        e->baudSwitchPending = false;
    }

    switch (e->state) {
    case SRXL2_LISTENING:
        /* Silent on purpose; an auto-announcing ESC is handled in
         * srxl2HandleHandshake(), which moves us on. */
        if (now - e->stateEnteredMs >= SRXL2_LISTEN_WINDOW_MS) {
            srxl2SetState(e, SRXL2_POLLING);
        }
        break;

    case SRXL2_POLLING:
        if (now - e->lastTxMs >= SRXL2_HANDSHAKE_INTERVAL_MS) {
            // Walk the whole ESC range rather than the default ID alone: an ESC with a
            // non-zero unit ID never announces itself, and polling is the only way to find
            // it. Polling the same ID on every bus is not a collision, each ESC hearing only
            // its own master. What finds an Avian, though, is its own announcement at
            // power-up: a running one answered none of 128 handshakes, 128 broadcasts and
            // 319 telemetry requests, so a board that reboots under a powered ESC never
            // links, and no amount of asking changes that
            srxl2SendHandshake(e, SRXL2_ESC_ID_FIRST + e->pollId, SRXL2_BAUD_BIT_400K);
            e->pollId++;
            if (SRXL2_ESC_ID_FIRST + e->pollId > SRXL2_ESC_ID_LAST) {
                e->pollId = 0;
            }
        }
        break;

    case SRXL2_FINALISING:
        /* Hold until the broadcast is out and any baud change has taken, then
         * start driving the ESC. */
        if (!e->baudSwitchPending && isSerialTransmitBufferEmpty(e->port)) {
            e->lastRxMs = now;          /* do not time out on the handshake gap */
            e->lastControlMs = now;
            e->runningSinceMs = now;
            srxl2SetState(e, SRXL2_RUNNING);
        }
        break;

    case SRXL2_RUNNING:
        if (now - e->lastControlMs >= SRXL2_CONTROL_INTERVAL_MS) {
            e->lastControlMs = now;
            srxl2SendControlData(e);
        }

        if (e->telemetry.valid && (now - e->telemetry.lastUpdateMs) > SRXL2_TELEM_STALE_MS) {
            e->telemetry.valid = false;
        }

        if (now - e->lastRxMs >= SRXL2_LINK_TIMEOUT_MS) {
            e->telemetry.valid = false;

            // Silence from the ESC is not a reason to stop commanding it. An Avian holds
            // throttle indefinitely with no telemetry request sent at all; what stops it is
            // the absence of control frames, and it picks the throttle back up by itself
            // when they return. Tearing the link down here would cause the outage it means
            // to detect, and permanently, since a running Avian answers no discovery. The
            // one case that does need it is a slave that reset, which comes back at 115200
            // and cannot be heard from 400000
            if (e->agreedBaudBits != 0) {
                serialSetBaudRate(e->port, SRXL2_BAUD_LOW);
                e->baudSwitchPending = false;
                e->agreedBaudBits = 0;
                e->deviceId = 0;
                srxl2SetState(e, SRXL2_POLLING);
            }
        }
        break;

    default:
        break;
    }
}

void srxl2MotorProcess(void)
{
    if (escCount == 0) {
        return;
    }

    const timeMs_t now = millis();

    srxl2CalProcess(now);

    for (uint8_t i = 0; i < escCount; i++) {
        srxl2ProcessEsc(&esc[i], now);
    }

    // The first two words carry a nibble per ESC, so a twin can be diagnosed without a debug
    // channel per bus. Both fit: the state enum is small, and ESC device IDs run 0x40..0x4F,
    // so the low nibble identifies the unit, with bit 3 marking "found" against unit 0
    uint16_t states = 0, ids = 0;
    uint32_t rxFrames = 0, crcErrors = 0;
    for (uint8_t i = 0; i < escCount; i++) {
        states |= (uint16_t)(esc[i].state & 0x07) << (4 * i);
        if (esc[i].deviceId) {
            ids |= (uint16_t)((esc[i].deviceId & 0x07) | 0x08) << (4 * i);
        }
        rxFrames += esc[i].statRxFrames;
        crcErrors += esc[i].statCrcErrors;
    }

    DEBUG_SET(DEBUG_ALWAYS, 0, states);
    DEBUG_SET(DEBUG_ALWAYS, 1, ids);
    DEBUG_SET(DEBUG_ALWAYS, 2, rxFrames);
    DEBUG_SET(DEBUG_ALWAYS, 3, crcErrors);
}

uint8_t srxl2MotorCount(void)
{
    return escCount;
}

// Whether every ESC is not merely being driven, but answering. Deliberately stricter than
// being in RUNNING: since a telemetry gap no longer tears the link down, a board whose ESC
// was unplugged stays in RUNNING and commands a motor that is not there. Right for a machine
// already flying, wrong for one about to arm, so the arming check asks this instead
bool srxl2MotorIsConnected(void)
{
    if (escCount == 0) {
        return false;
    }
    const timeMs_t now = millis();
    for (uint8_t i = 0; i < escCount; i++) {
        if (esc[i].state != SRXL2_RUNNING || esc[i].deviceId == 0) {
            return false;
        }
        if (now - esc[i].lastRxMs >= SRXL2_LINK_TIMEOUT_MS) {
            return false;
        }
        if (now - esc[i].runningSinceMs < SRXL2_READY_DELAY_MS) {
            return false;       /* linked, but still sounding its startup tones */
        }
    }
    return true;
}

bool srxl2MotorGetTelemetry(uint8_t index, srxl2EscTelemetry_t *out)
{
    if (index >= escCount || !out || !esc[index].telemetry.valid) {
        return false;
    }
    if (millis() - esc[index].telemetry.lastUpdateMs > SRXL2_TELEM_STALE_MS) {
        return false;
    }
    *out = esc[index].telemetry;
    return true;
}

#endif /* USE_MOTOR_SRXL2 */

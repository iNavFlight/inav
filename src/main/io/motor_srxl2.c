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
 * Wire format and session handling follow "Specification for Spektrum SRXL2",
 * Rev K (https://github.com/SpektrumRC/SRXL2). Packet structures are the ones
 * INAV already carries in rx/srxl2_types.h for the receiver side, so both ends
 * of the protocol share a single definition.
 *
 * The specification describes the bus; it says nothing about any particular ESC.
 * Everything an individual ESC decides for itself - which channel it reads as
 * throttle, how an auxiliary channel arms reverse, whether it offers 400000 baud
 * - is deliberately confined to the named constants and the reverse-channel
 * setter below, so the places that need confirming against real hardware are
 * countable rather than scattered.
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

/*
 * Our own device ID: flight controller type, unit ID 1.
 *
 * Not unit 0. Specification 7.1.1: a device whose lower nibble is 0 announces
 * itself with an unprompted handshake at startup. That behaviour belongs to the
 * slave, and a bus master that announced itself would both collide with the
 * ESC's own announcements and break the silence the next comment describes.
 */
#define SRXL2_OUR_DEVICE_ID         0x31

/*
 * The bus arbitrates who is master by device ID, lowest wins: a device that sees
 * a handshake from a lower ID stands down. We never implement that side of it,
 * because this port is a dedicated link to an ESC rather than a shared bus, and
 * an ESC at 0x40 cannot outrank 0x31. Worth knowing before anyone wires a
 * Spektrum receiver onto the same pin, where it would be master at 0x21 and this
 * driver would be wrong to keep polling.
 */

/*
 * Control Data commands.
 *
 * The protocol also has a failsafe channel-data command, 0x01, which a receiver
 * sends when its RF link is gone so each device applies its own failsafe. This
 * driver never sends it, and that is deliberate.
 *
 * On this bus the master is the flight controller and there is no RF link: the
 * link is a wire. INAV owns failsafe, and it handles it by substituting channel
 * values and continuing to fly - LAND and RTH actively command the motors all the
 * way down. Telling the ESC the link had failed would hand throttle authority to
 * the ESC's own behaviour in the middle of INAV's landing, which is the opposite
 * of helpful.
 *
 * What does protect against this wire dying is the ESC's own receive timeout,
 * which needs no cooperation from us: if the flight controller stops sending, the
 * ESC falls back on its own, and that is the case where its failsafe is the right
 * authority.
 */
#define SRXL2_CMD_CHANNEL_DATA      0x00

/* Reply ID 0x00 means "no reply wanted" (specification 7.1.1). */
#define SRXL2_REPLY_NONE            0x00

/*
 * The frame carries the throttle, and the reverse channel when one is
 * configured. Nothing else: the ESC reads CH1 for throttle and the channel its
 * own Thrust Rev. parameter names, and has no use for the rest.
 *
 * An earlier version sent a block of eight, on the strength of a bench session
 * where one channel gave 0.0 % and eight gave 30 % - read at the time as "an
 * Avian will not arm from a single-channel frame". That reading was confounded:
 * the two runs also asked for telemetry at different rates, and the rate was
 * what mattered. Rerun with the request rate held fixed, masks of one, two
 * adjacent, two spread, four and eight channels all gave the same 14.0 % and the
 * same 6610 rpm from the same 1250 us command; rerun asking on every frame, all
 * five gave 0.0 %, the single-channel mask included. See SRXL2_TELEM_REQUEST_MIN
 * for what that rate does.
 *
 * Two channels rather than eight is twelve bytes a frame less on a wire the
 * telemetry reply has to share.
 */

/* X-Bus telemetry sensor IDs */
#define SRXL2_TELEM_SENSOR_ESC      0x20

/*
 * Specification 7.2.1: an ESC with unit ID 0 repeats an unprompted handshake
 * every 50 ms for the first 200 ms after reset. Stay silent for slightly longer
 * than that so the common case is discovered without contending with those
 * announcements, then start polling - an ESC configured with a non-zero unit ID
 * never announces itself and would otherwise never be found.
 */
#define SRXL2_LISTEN_WINDOW_MS      250
#define SRXL2_HANDSHAKE_INTERVAL_MS 50

/*
 * Rate at which Control Data goes out once the link is up.
 *
 * Not the motor update rate. The specification has the master emit one Control
 * Data packet at the rate RF frames arrive, which is tens of hertz, and an ESC
 * is not expecting kilohertz. 115200 baud would not carry it either: an
 * eighteen-byte frame is about 1.6 ms on the wire.
 */
#define SRXL2_CONTROL_INTERVAL_MS   20      /* 50 Hz */

/*
 * Ask for telemetry on every Nth Control Data packet rather than on all of them.
 * Telemetry is a reply, so requesting it every frame doubles bus occupancy and
 * forces a half-duplex turnaround each time, for values that change slowly.
 * Every fifth frame at 50 Hz asks ten times a second, which reaches the flight
 * controller as about 1.1 ESC readings a second once the ESC's own sensor
 * rotation is accounted for.
 */
#define SRXL2_TELEM_REQUEST_DEFAULT 5

/*
 * Never ask on every frame. An Avian keeps the link up when asked at 50 Hz - it
 * answers, and its telemetry is correct - but it stops obeying the throttle and
 * reports zero per cent from every stick position. Dropping back to every second
 * frame restores it immediately, without a power cycle. Measured repeatedly on an
 * Avian 70A, against a fixed 1250 us command: every 2nd, 3rd and 4th frame all
 * gave 14 % and about 6600 rpm, every frame gave nothing at all.
 */
#define SRXL2_TELEM_REQUEST_MIN     2

/*
 * Declare the link dead if the ESC stops answering for this long.
 *
 * This is the reason the telemetry table stops at one request every five frames.
 * An Avian never speaks unprompted once it is running, so the only thing that
 * refreshes this timer is a telemetry reply, and a request rate slower than the
 * timeout disconnects a healthy ESC on schedule. Measured, it answers about two
 * requests in three, so the margin has to cover consecutive misses as well:
 * every fifth frame is a request every 100 ms and a reply typically every 150,
 * so three misses in a row still land inside this window.
 *
 * Losing the link is not a recoverable event on this hardware - a running Avian
 * answers no discovery of any kind - so a timeout that can fire on a healthy
 * link would mean a motor that stops and cannot be brought back without
 * removing power.
 */
#define SRXL2_LINK_TIMEOUT_MS       500

/*
 * Telemetry older than this is reported as stale rather than current.
 *
 * Generous compared with the link timeout on purpose. The ESC rotates its reply
 * between three sensors and answers about two requests in three, so at the
 * default rate its own readings arrive about once a second even though the link
 * is being exercised ten times as often: a one-second window made a healthy
 * sensor flicker between valid and absent. What notices an ESC that has actually
 * stopped is SRXL2_LINK_TIMEOUT_MS, which invalidates the reading anyway.
 */
#define SRXL2_TELEM_STALE_MS        3000

/*
 * Calibration phases end themselves. The high phase has to outlast a human
 * reaching for a battery lead; the low phase only has to outlast the ESC's
 * cell-count tones. Neither may persist, because one of them commands full
 * throttle.
 */
#define SRXL2_CAL_WAIT_TIMEOUT_MS   60000   /* time to walk over and plug the battery in */
#define SRXL2_CAL_MANUAL_TIMEOUT_MS 30000

/*
 * How long to keep holding full throttle after the ESC gains power, and then how
 * long to hold minimum.
 *
 * These were three and five seconds, read off the published tone timings: the
 * manual's window opens at the two short tones and lasts five, and the tones
 * follow power-up by a second or so. Measured against an Avian 70 A, that is not
 * enough. The same ESC, calibrated twice in a row from the same state:
 *
 *   3 s high, 5 s low - the ESC sounds its tones and stores nothing. Throttle
 *       still ignored below channel value 12220, saturated from 50820, so 41 %
 *       of the range does nothing and the stick reaches full power at 78 %.
 *   4 s high, 7 s low - stored. Responds from 2687 and saturates at 64307,
 *       94 % of the channel used, and the throttle it reports back tracks the
 *       throttle commanded to within a point across the whole range: 1050 us
 *       gives 5 %, 1500 gives 50 %, 2000 gives 100 %.
 *
 * So the extra second either side is what lands inside the window rather than
 * on its edge. They cost nothing - the sequence runs once, on a bench, with the
 * propeller off.
 */
#define SRXL2_CAL_SETTLE_MS         4000

/* Long enough for the cell-count tones and the closing long tone. */
#define SRXL2_CAL_LOW_MS            7000

/* Endpoints presented during calibration, on INAV's usual motor scale. */
#define SRXL2_CAL_HIGH_US           2000
#define SRXL2_CAL_LOW_US            1000

/*
 * Channel value scaling, the exact inverse of what rx/srxl2.c applies when it
 * decodes channel data: us = 988 + (value >> 6). 1500 us therefore maps onto
 * 0x8000, which the specification calls "Servo Center", and the shift leaves the
 * low two bits clear as the specification requires.
 *
 * This deliberately does not reach the ends of the 0..65532 range: 1000 us lands
 * on 768 and 2000 us on 64768, because a Spektrum receiver's full travel decodes
 * to 988..2012 us rather than 1000..2000. That is the point - it makes us look
 * like a receiver, which is what the ESC was calibrated against.
 *
 * Spektrum ESCs learn their endpoints from the signal during the ESC/Radio
 * calibration in their manual, and INAV cannot perform that procedure: it wants
 * full throttle present when the battery is connected, and INAV outputs
 * mincommand while disarmed. So the calibration is done with a Spektrum
 * transmitter, or left at the factory default, and the range the ESC remembers
 * is a receiver's. Matching it is why this scaling is the right one.
 *
 * The cost is about 1.2 percent of travel at the top. That is the better half of
 * the trade, because the alternative - stretching 1000..2000 us across the full
 * range - moves the centre, and the centre is where an ESC in Reverse brake mode
 * takes zero thrust. A slightly low maximum is a worse throttle curve; a
 * misplaced centre is creeping thrust at neutral.
 */
#define SRXL2_PULSE_OFFSET_US       988
#define SRXL2_PULSE_SHIFT           6
#define SRXL2_VALUE_MAX             0xFFFC  /* specification caps values here */

/* Throttle is channel index 0 by Spektrum convention. Unverified against an
 * actual ESC: this is one of the constants to confirm on a bench. */
#define SRXL2_CHANNEL_THROTTLE      0

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

/*
 * One of these per ESC.
 *
 * Each instance is an independent bus with its own handshake, baud negotiation,
 * receive framing and telemetry. Nothing is shared between them except our own
 * device ID, which is allowed precisely because they are separate buses and
 * never hear each other.
 */
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
    uint8_t   agreedBaudBits;
    bool      baudSwitchPending;        /* waiting for TX to drain */

    uint16_t  channelValue[32];
    uint32_t  channelMask;
    uint8_t   telemRequestCounter;

    srxl2EscTelemetry_t telemetry;

    uint32_t  statTxFrames, statRxFrames, statCrcErrors, statHandshakes;
} srxl2Esc_t;

static srxl2Esc_t esc[SRXL2_ESC_MAX_MOTORS];
static uint8_t    escCount;                 /* ports successfully opened */

/* Shared, because these describe the aircraft rather than one bus. */
static uint8_t   reverseChannel1Based = 7;  /* Spektrum ship "Thrust Rev." on CH7 */

/*
 * Control frames between telemetry requests. The control interval is 20 ms, so a
 * divisor of 5 asks at 10 Hz. Held as a divisor rather than a rate because that is
 * what the transmit path actually counts.
 */
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

/*
 * Which channels this frame carries: the throttle, plus the reverse channel when
 * one is configured. Rebuilt rather than accumulated, so the frame does not
 * change shape depending on what has been called since power-up - and so that
 * clearing the reverse channel actually stops sending it.
 */
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

/*
 * Handshakes seen across every bus. The calibration watches this to notice an
 * ESC gaining power, and with more than one ESC the first to speak is signal
 * enough - they are all being powered from the same pack.
 */
static uint32_t srxl2TotalHandshakes(void)
{
    uint32_t total = 0;
    for (uint8_t i = 0; i < escCount; i++) {
        total += esc[i].statHandshakes;
    }
    return total;
}

/*
 * Append the CRC and push the frame. buf[2] must already hold the frame length
 * as the specification's framing requires.
 *
 * Returns false when the frame did not go out, which the caller has to care
 * about for the one frame where it matters. A port that has backed up - a slow
 * link, a stalled DMA - drops whatever does not fit, and for Control Data that
 * is of no consequence, since another follows in 20 ms and the ESC tolerates
 * 250 ms of silence. For the broadcast that moves the bus to a new rate it is
 * the difference between a working link and a dead one: raise the rate having
 * only believed that frame was sent, and the ESC is left behind at the old rate
 * with no way back short of a power cycle. That failure has been seen on real
 * hardware, and it is not recoverable in flight.
 */
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

    /*
     * The negotiation is entered once, and only from the states that are still
     * looking for an ESC. Re-entering it on every handshake that arrives is a trap:
     * our own finalise broadcasts, the slave answers the broadcast with a
     * handshake, and if that answer restarts the sequence the two ping-pong
     * handshakes indefinitely. Two ways that bites -
     *
     *   - control data is sent on a timer reset on entering RUNNING, so a bus
     *     looping back through FINALISING never reaches the first control frame:
     *     the link looks established and the motor never turns;
     *   - each restart queues another broadcast, so on a slow or busy port the
     *     transmit buffer never drains and the bus never leaves FINALISING at all.
     *
     * A slave that genuinely reset is not missed by this. It comes back at 115200
     * while we are at 400000, so nothing it says is intelligible, and the link
     * timeout drops us to POLLING at the low rate to find it again.
     */
    if (e->state == SRXL2_FINALISING || e->state == SRXL2_RUNNING) {
        /* Still answer a running ESC, so it knows the master is there - but say
         * nothing mid-negotiation, where another broadcast is what causes the
         * loop. */
        if (e->state == SRXL2_RUNNING && e->deviceId == src) {
            srxl2SendHandshake(e, src, SRXL2_BAUD_BIT_400K);
        }
        return;
    }

    e->deviceId = src;
    e->baudSupported = f->payload.baudSupported;

    /* Counted here rather than on every handshake frame, so it means "a
     * negotiation started" and not merely "a handshake went past". The
     * calibration uses it as the signal that an ESC has just gained power, and a
     * running ESC answering our keepalive is not that. */
    e->statHandshakes++;

    /* Answer the slave so it knows who the master is, then finalise.
     *
     * This also covers the ESC being powered after the flight controller, which is
     * the normal case on a bench: the board comes up on USB and the ESC only boots
     * when the battery goes in, long after the listen window closed. By then we are
     * in POLLING, which is one of the states that accepts a handshake, so the ESC
     * is still found. */
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

    /* Calibration overrides the throttle here rather than at staging time, so no
     * mixer path can quietly write over it between the two. Every ESC is
     * calibrated at once: they share a battery, so they power up together, and
     * the window the sequence aims at is the same window for all of them. */
    if (calPhase != SRXL2_CAL_OFF) {
        const bool high = (calPhase == SRXL2_CAL_WAIT_BATTERY)
                       || (calPhase == SRXL2_CAL_SETTLE)
                       || (calPhase == SRXL2_CAL_HIGH_MANUAL);
        e->channelValue[SRXL2_CHANNEL_THROTTLE] =
            srxl2UsToValue(high ? SRXL2_CAL_HIGH_US : SRXL2_CAL_LOW_US);
    }

    buf[n++] = SRXL2_MAGIC;
    buf[n++] = ControlData;
    buf[n++] = 0;                       /* length, patched below */
    buf[n++] = SRXL2_CMD_CHANNEL_DATA;
    buf[n++] = replyId;

    /*
     * RSSI has to read as a healthy link, even though we are not an RF device.
     * This is not a guess: Spektrum's own receiver code treats a received zero
     * as loss of link -
     *
     *     if (channelData->rssi == 0) { globalResult = RX_FRAME_FAILSAFE; }
     *
     * - so sending 0 would be telling the ESC that the link is gone on every
     * frame.
     */
    buf[n++] = 100;
    buf[n++] = 0;                       /* frameLosses low */
    buf[n++] = 0;                       /* frameLosses high */

    buf[n++] = (uint8_t)(e->channelMask & 0xFF);
    buf[n++] = (uint8_t)((e->channelMask >> 8) & 0xFF);
    buf[n++] = (uint8_t)((e->channelMask >> 16) & 0xFF);
    buf[n++] = (uint8_t)((e->channelMask >> 24) & 0xFF);

    /*
     * A contiguous block, little-endian, lowest index first. The channels this
     * driver has nothing to say on are filled, and filled at their **minimum**.
     *
     * Never at centre. That is reasonable for a surface ESC, where centre means
     * stopped, and dangerous for an aircraft one, where 1500 us is half
     * throttle: were the ESC to read throttle on an index other than the one
     * expected, centred padding would spin the motor at half power while
     * minimum padding leaves it idle. Wrong guess, safe outcome - which is the
     * same reasoning that used to argue for sending nothing at all, before an
     * ESC made it clear that sending nothing means never arming.
     */
    for (uint8_t ch = 0; ch < 32; ch++) {
        if (e->channelMask & (1u << ch)) {
            buf[n++] = (uint8_t)(e->channelValue[ch] & 0xFF);
            buf[n++] = (uint8_t)(e->channelValue[ch] >> 8);
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

    /*
     * One ESC per port, so open every port that was assigned the function, up to
     * the array size. The enumeration follows serialConfig's port order, which is
     * UART order, so motor 1 is the lowest-numbered assigned UART, motor 2 the
     * next, and so on. That is the only mapping available: nothing on an SRXL2
     * bus says which motor an ESC drives, so the wiring order has to carry it.
     */
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
            for (uint8_t ch = 0; ch < 32; ch++) {
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

/*
 * Whether a configured reverse channel can actually be used.
 *
 * Zero means the model has no reverse. Anything that would land on the throttle
 * channel is refused outright: srxl2MotorSetReverse() runs at task rate and
 * writes its channel unconditionally, so a reverse channel aliased onto the
 * throttle would overwrite the mixer's staged throttle several hundred times a
 * second - holding the motor at idle whenever reverse was released, and
 * commanding full throttle whenever it was armed. The setting's own range
 * (0..9) cannot express "zero, or five to nine", so the check belongs here.
 *
 * The upper bound is the width of the channel array and of the wire's mask.
 * Spektrum documents Smart ESC reverse as available on channels 5 to 9 only, but
 * that is the ESC's restriction rather than the protocol's, so it is enforced by
 * the setting and the Configurator rather than refused here.
 */
static bool srxl2ReverseChannelUsable(uint8_t channel1Based)
{
    if (channel1Based == 0 || channel1Based > 32) {
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

    /*
     * The high phase commands full throttle, so it may not start against an ESC
     * that already has power. The unattended sequence refuses this and the
     * manual one did not, which left the more dangerous of the two - a person
     * typing a command, rather than a wizard that walks them through it -
     * without the guard. Boards that cannot sense the pack report it absent and
     * are unaffected, which is the case this manual path exists for.
     */
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
        /*
         * What opens the window is the ESC *gaining* power, which is an event, so
         * both signals have to be events too: the pack appearing, or a fresh
         * handshake arriving on any bus. An earlier version tested
         * escDeviceId != 0, which is persistent state left over from the last
         * time the ESC was seen, so the wait was skipped outright on any board
         * that had already talked to its ESC once.
         */
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
            /* Walk the whole ESC range rather than only the default ID. An ESC
             * with a non-zero unit ID never announces itself, so polling is the
             * only way it could be found - and polling one address was not
             * that, it was polling the one address that does announce.
             *
             * Every bus is polled at the same ID, which is not a collision: each
             * ESC is alone on its wire and hears only its own master.
             *
             * What finds an Avian is not this, though: it is the ESC's own
             * announcement at power-up, caught here because polling is what we
             * happen to be doing when the ESC boots. A running Avian answers
             * none of this - measured, with the ESC alive and the bus otherwise
             * quiet: 128 handshakes to 0x40, 128 broadcasts, 128 spread across
             * 0x40..0x4F and 319 control frames asking for telemetry all drew
             * exactly nothing. It announces six times in the 300 ms after reset
             * and is mute from then on. A board that reboots under a powered ESC
             * therefore never links, and no amount of asking changes that. */
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
            /* Lost the ESC. Go back to 115200, where a slave that has just reset
             * will be listening, and look for it again. */
            serialSetBaudRate(e->port, SRXL2_BAUD_LOW);
            e->baudSwitchPending = false;
            e->agreedBaudBits = 0;
            e->deviceId = 0;
            e->telemetry.valid = false;
            srxl2SetState(e, SRXL2_POLLING);
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

    /*
     * The first two words carry a nibble per ESC, so a twin can be diagnosed
     * without a debug channel per bus: which bus is stuck, and which has found
     * its ESC. Both fit: the state enum is small, and ESC device IDs run
     * 0x40..0x4F, so the low nibble identifies the unit. Bit 3 is set alongside
     * it to distinguish unit 0 from "nothing found".
     */
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

bool srxl2MotorIsConnected(void)
{
    if (escCount == 0) {
        return false;
    }
    for (uint8_t i = 0; i < escCount; i++) {
        if (esc[i].state != SRXL2_RUNNING || esc[i].deviceId == 0) {
            return false;
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

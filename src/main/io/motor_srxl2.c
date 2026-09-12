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
 * Every fifth frame at 50 Hz gives 10 Hz, which is in line with what INAV's
 * other ESC telemetry backends deliver.
 */
#define SRXL2_TELEM_REQUEST_EVERY   5

/* Declare the link dead if the ESC stops answering for this long. */
#define SRXL2_LINK_TIMEOUT_MS       500

/* Telemetry older than this is reported as stale rather than current. */
#define SRXL2_TELEM_STALE_MS        1000

/*
 * Calibration phases end themselves. The high phase has to outlast a human
 * reaching for a battery lead; the low phase only has to outlast the ESC's
 * cell-count tones. Neither may persist, because one of them commands full
 * throttle.
 */
#define SRXL2_CAL_WAIT_TIMEOUT_MS   60000   /* time to walk over and plug the battery in */
#define SRXL2_CAL_MANUAL_TIMEOUT_MS 30000

/*
 * How long to keep holding full throttle after the ESC gains power.
 *
 * The manual's window opens at the two short tones and lasts five seconds. Those
 * tones follow the power-up sequence by a second or so, so dropping three
 * seconds after power-up lands inside it with room on both sides. This is the
 * one number in the sequence taken from the published tone timings rather than
 * measured, and the first thing to adjust if an ESC refuses the calibration.
 */
#define SRXL2_CAL_SETTLE_MS         3000

/* Long enough for the cell-count tones and the closing long tone. */
#define SRXL2_CAL_LOW_MS            5000

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

static serialPort_t  *srxl2Port = NULL;
static srxl2State_e   srxl2State = SRXL2_DISABLED;

static uint8_t   rxBuf[SRXL2_MAX_FRAME];
static uint8_t   rxLen;
static uint8_t   rxExpected;

static timeMs_t  stateEnteredMs;
static timeMs_t  lastRxMs;
static timeMs_t  lastTxMs;
static timeMs_t  lastControlMs;

static uint8_t   escDeviceId;               /* 0 until discovered */
static uint8_t   escBaudSupported;
static uint8_t   agreedBaudBits;
static bool      baudSwitchPending;         /* waiting for TX to drain */

static uint16_t  channelValue[32];
static uint32_t  channelMask;
static uint8_t   reverseChannel1Based = 5;  /* Avian "Thrust Rev." default: CH5 */
static uint8_t   telemRequestCounter;

static srxl2EscTelemetry_t escTelemetry;

static srxl2CalPhase_e calPhase = SRXL2_CAL_OFF;
static timeMs_t        calPhaseMs;      /* when the current phase began */
static uint32_t        calHandshakeMark; /* handshake count when the phase began */

static uint32_t  statTxFrames, statRxFrames, statCrcErrors, statHandshakes;

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

static void srxl2SetState(srxl2State_e next)
{
    srxl2State = next;
    stateEnteredMs = millis();
}

/* Append the CRC and push the frame. buf[2] must already hold the frame length
 * as the specification's framing requires. */
static void srxl2SendFrame(uint8_t *buf, uint8_t len)
{
    if (!srxl2Port || len < SRXL2_MIN_FRAME || len > SRXL2_MAX_FRAME) {
        return;
    }

    const uint16_t crc = crc16_ccitt_update(0, buf, len - 2);
    buf[len - 2] = (uint8_t)(crc >> 8);
    buf[len - 1] = (uint8_t)(crc & 0xFF);

    serialWriteBuf(srxl2Port, buf, len);
    lastTxMs = millis();
    statTxFrames++;
}

static void srxl2SendHandshake(uint8_t destinationId, uint8_t baudField)
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
                                             * improbable */

    srxl2SendFrame(buf, sizeof(Srxl2HandshakeFrame));
}

/* Tell the bus which rate everyone moves to, and arrange to follow once the
 * frame has actually left the port. Switching immediately would clock the tail
 * of that very frame out at the new rate and lose it. */
static void srxl2Finalise(void)
{
    agreedBaudBits = SRXL2_BAUD_BIT_400K & escBaudSupported;
    srxl2SendHandshake(Broadcast, agreedBaudBits);
    baudSwitchPending = (agreedBaudBits & SRXL2_BAUD_BIT_400K) != 0;
    srxl2SetState(SRXL2_FINALISING);
}

/*---------------------------------------------------------------------------
 * Telemetry decoding: STRU_TELE_ESC, big-endian on the wire
 *-------------------------------------------------------------------------*/

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

    /* 0xFFFF and 0xFF mean "no data" and must not be taken for readings -
     * 0xFFFF volts at 0.01 V per count would otherwise look like 655 V. */
    if (rpm != 0xFFFF)        { escTelemetry.rpm = (uint32_t)rpm * 10; }
    if (voltsIn != 0xFFFF)    { escTelemetry.voltage = voltsIn; }                 /* already 0.01 V */
    if (currentMot != 0xFFFF) { escTelemetry.current = currentMot; }              /* 10 mA == 0.01 A */
    if (tempFet != 0xFFFF)    { escTelemetry.temperatureFet = (int16_t)tempFet; } /* 0.1 degC */
    if (tempBec != 0xFFFF)    { escTelemetry.temperatureBec = (int16_t)tempBec; }
    if (currentBec != 0xFF)   { escTelemetry.currentBec = (uint16_t)currentBec * 10; } /* 100 mA -> 0.01 A */
    if (voltsBec != 0xFF)     { escTelemetry.voltageBec = (uint16_t)voltsBec * 5; }    /* 0.05 V -> 0.01 V */
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
        /* Something else on the bus. This driver only speaks to ESCs. */
        return;
    }

    escDeviceId = src;
    escBaudSupported = f->payload.baudSupported;
    statHandshakes++;

    /* Answer the slave so it knows who the master is, then finalise.
     *
     * This also covers the ESC being powered after the flight controller, which
     * is the normal case on a bench: the board comes up on USB and the ESC only
     * boots when the battery goes in, long after our listen window closed. Its
     * handshake arrives while we are already RUNNING and has to be honoured, or
     * the ESC is never found at all. */
    srxl2SendHandshake(escDeviceId, SRXL2_BAUD_BIT_400K);
    srxl2Finalise();
}

static void srxl2HandleTelemetry(const uint8_t *buf, uint8_t len)
{
    /* header(3) + destDeviceId(1) + 16 byte payload + crc(2) */
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

        if (rxLen == 0) {
            if (c != SRXL2_MAGIC) {
                continue;               /* resynchronise on the magic byte */
            }
            rxExpected = 0;
        }

        rxBuf[rxLen++] = c;

        if (rxLen == 3) {
            rxExpected = rxBuf[2];
            if (rxExpected < SRXL2_MIN_FRAME || rxExpected > SRXL2_MAX_FRAME) {
                rxLen = 0;              /* bogus length, drop and resynchronise */
                continue;
            }
        }

        if (rxExpected && rxLen >= rxExpected) {
            srxl2HandleFrame(rxBuf, rxLen);
            rxLen = 0;
            rxExpected = 0;
        }
    }
}

/*---------------------------------------------------------------------------
 * Control data
 *-------------------------------------------------------------------------*/

static void srxl2SendControlData(void)
{
    uint8_t buf[SRXL2_MAX_FRAME];
    uint8_t n = 0;

    /* Request telemetry only occasionally - see SRXL2_TELEM_REQUEST_EVERY. */
    uint8_t replyId = SRXL2_REPLY_NONE;
    if (++telemRequestCounter >= SRXL2_TELEM_REQUEST_EVERY) {
        telemRequestCounter = 0;
        replyId = escDeviceId;
    }

    /* Calibration overrides the throttle here rather than at staging time, so no
     * mixer path can quietly write over it between the two. */
    if (calPhase != SRXL2_CAL_OFF) {
        const bool high = (calPhase == SRXL2_CAL_WAIT_BATTERY)
                       || (calPhase == SRXL2_CAL_SETTLE)
                       || (calPhase == SRXL2_CAL_HIGH_MANUAL);
        channelValue[SRXL2_CHANNEL_THROTTLE] =
            srxl2UsToValue(high ? SRXL2_CAL_HIGH_US : SRXL2_CAL_LOW_US);
        channelMask |= (1u << SRXL2_CHANNEL_THROTTLE);
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

    buf[n++] = (uint8_t)(channelMask & 0xFF);
    buf[n++] = (uint8_t)((channelMask >> 8) & 0xFF);
    buf[n++] = (uint8_t)((channelMask >> 16) & 0xFF);
    buf[n++] = (uint8_t)((channelMask >> 24) & 0xFF);

    /*
     * Only the channels we actually mean, little-endian, lowest index first.
     *
     * Deliberately not padded with centred values on the channels we do not
     * use. Doing that is reasonable for a surface ESC, where centre means
     * stopped, and dangerous for an aircraft one, where 1500 us is half
     * throttle: if the ESC turned out to read throttle on an index we did not
     * expect, padding would spin the motor at 50 percent, while sending nothing
     * there simply leaves it idle. Wrong guess, safe outcome.
     */
    for (uint8_t ch = 0; ch < 32; ch++) {
        if (channelMask & (1u << ch)) {
            buf[n++] = (uint8_t)(channelValue[ch] & 0xFF);
            buf[n++] = (uint8_t)(channelValue[ch] >> 8);
        }
    }

    n += 2;                             /* room for the CRC */
    buf[2] = n;
    srxl2SendFrame(buf, n);
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

    rxLen = 0;
    rxExpected = 0;
    memset(&escTelemetry, 0, sizeof(escTelemetry));
    memset(channelValue, 0, sizeof(channelValue));
    channelMask = 0;
    escDeviceId = 0;
    escBaudSupported = 0;
    agreedBaudBits = 0;
    baudSwitchPending = false;
    telemRequestCounter = 0;

    /* Start the throttle channel at its lowest value rather than zero, so the
     * first frame after a handshake cannot be read as something unexpected. */
    channelValue[SRXL2_CHANNEL_THROTTLE] = srxl2UsToValue(1000);
    channelMask |= (1u << SRXL2_CHANNEL_THROTTLE);

    const timeMs_t now = millis();
    lastRxMs = now;
    lastTxMs = now;
    lastControlMs = now;

    srxl2SetState(SRXL2_LISTENING);
    return true;
}

void srxl2MotorUpdate(uint8_t index, uint16_t value)
{
    if (index >= SRXL2_ESC_MAX_MOTORS) {
        return;
    }
    /* Staging only. The wire is driven at its own rate from srxl2MotorProcess(),
     * not at whatever rate the mixer happens to run. */
    channelValue[SRXL2_CHANNEL_THROTTLE] = srxl2UsToValue(value);
    channelMask |= (1u << SRXL2_CHANNEL_THROTTLE);
}

void srxl2MotorSetReverse(bool armed)
{
    if (reverseChannel1Based == 0 || reverseChannel1Based > 32) {
        return;     /* reverse not configured */
    }
    const uint8_t idx = reverseChannel1Based - 1;
    channelValue[idx] = srxl2UsToValue(armed ? 2000 : 1000);
    channelMask |= (1u << idx);
}

void srxl2MotorSetReverseChannel(uint8_t channel1Based)
{
    reverseChannel1Based = channel1Based;
}

/* Checked on both sides rather than trusting the caller, because one of these
 * phases commands full throttle with the aircraft disarmed. */
static srxl2CalResult_e srxl2CalCommonChecks(void)
{
    if (ARMING_FLAG(ARMED)) {
        return SRXL2_CAL_REJECT_ARMED;
    }
    if (!srxl2Port) {
        return SRXL2_CAL_REJECT_NO_PORT;
    }
    return SRXL2_CAL_ACCEPTED;
}

srxl2CalResult_e srxl2MotorCalibrationBegin(void)
{
    const srxl2CalResult_e common = srxl2CalCommonChecks();
    if (common != SRXL2_CAL_ACCEPTED) {
        return common;
    }

    /* Detecting the ESC powering up is the whole mechanism, so say so plainly
     * instead of starting a sequence that can never advance. */
    if (!isBatteryVoltageConfigured()) {
        return SRXL2_CAL_REJECT_NO_VOLTAGE_SENSOR;
    }

    /*
     * Refuse if the pack is already in. The ESC only reads its endpoints as it
     * powers up, so starting with it already running would achieve nothing - and
     * it would mean presenting full throttle to an ESC that can act on it.
     */
    if (getBatteryState() != BATTERY_NOT_PRESENT) {
        return SRXL2_CAL_REJECT_BATTERY_PRESENT;
    }

    calPhase = SRXL2_CAL_WAIT_BATTERY;
    calPhaseMs = millis();
    calHandshakeMark = statHandshakes;
    return SRXL2_CAL_ACCEPTED;
}

srxl2CalResult_e srxl2MotorCalibrationManual(srxl2CalPhase_e phase)
{
    const srxl2CalResult_e common = srxl2CalCommonChecks();
    if (common != SRXL2_CAL_ACCEPTED) {
        return common;
    }

    calPhase = phase;
    calPhaseMs = millis();
    return SRXL2_CAL_ACCEPTED;
}

void srxl2MotorCalibrationAbort(void)
{
    calPhase = SRXL2_CAL_OFF;
}

srxl2CalPhase_e srxl2MotorCalibrationPhase(void)
{
    return calPhase;
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
         * handshake arriving. An earlier version tested escDeviceId != 0, which
         * is persistent state left over from the last time the ESC was seen, so
         * the wait was skipped outright on any board that had already talked to
         * its ESC once.
         */
        if (getBatteryState() != BATTERY_NOT_PRESENT || statHandshakes != calHandshakeMark) {
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

void srxl2MotorProcess(void)
{
    if (!srxl2Port || srxl2State == SRXL2_DISABLED) {
        return;
    }

    srxl2DrainRx();

    const timeMs_t now = millis();

    srxl2CalProcess(now);

    /* A deferred baud change completes as soon as the broadcast has left. */
    if (baudSwitchPending && isSerialTransmitBufferEmpty(srxl2Port)) {
        serialSetBaudRate(srxl2Port, SRXL2_BAUD_HIGH);
        baudSwitchPending = false;
    }

    switch (srxl2State) {
    case SRXL2_LISTENING:
        /* Silent on purpose; an auto-announcing ESC is handled in
         * srxl2HandleHandshake(), which moves us on. */
        if (now - stateEnteredMs >= SRXL2_LISTEN_WINDOW_MS) {
            srxl2SetState(SRXL2_POLLING);
        }
        break;

    case SRXL2_POLLING:
        if (now - lastTxMs >= SRXL2_HANDSHAKE_INTERVAL_MS) {
            /* Poll the default ESC ID. An ESC with a non-zero unit ID never
             * announces itself, so without this it would never be found. */
            srxl2SendHandshake(SRXL2_ESC_ID_FIRST, SRXL2_BAUD_BIT_400K);
        }
        break;

    case SRXL2_FINALISING:
        /* Hold until the broadcast is out and any baud change has taken, then
         * start driving the ESC. */
        if (!baudSwitchPending && isSerialTransmitBufferEmpty(srxl2Port)) {
            lastRxMs = now;             /* do not time out on the handshake gap */
            lastControlMs = now;
            srxl2SetState(SRXL2_RUNNING);
        }
        break;

    case SRXL2_RUNNING:
        if (now - lastControlMs >= SRXL2_CONTROL_INTERVAL_MS) {
            lastControlMs = now;
            srxl2SendControlData();
        }

        if (escTelemetry.valid && (now - escTelemetry.lastUpdateMs) > SRXL2_TELEM_STALE_MS) {
            escTelemetry.valid = false;
        }

        if (now - lastRxMs >= SRXL2_LINK_TIMEOUT_MS) {
            /* Lost the ESC. Go back to 115200, where a slave that has just reset
             * will be listening, and look for it again. */
            serialSetBaudRate(srxl2Port, SRXL2_BAUD_LOW);
            baudSwitchPending = false;
            agreedBaudBits = 0;
            escDeviceId = 0;
            escTelemetry.valid = false;
            srxl2SetState(SRXL2_POLLING);
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
    return srxl2State == SRXL2_RUNNING && escDeviceId != 0;
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

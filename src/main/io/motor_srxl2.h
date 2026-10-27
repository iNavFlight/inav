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
 * SRXL2 motor output: acts as the SRXL2 bus master towards a Spektrum Smart
 * ESC, which is what "Smart Throttle" is on the wire.
 *
 * Unlike every other motor protocol in INAV, this one is a UART protocol rather
 * than a timer waveform, so the ESC signal wire goes to a serial pin assigned
 * FUNCTION_ESC_SRXL2 and not to a motor pad. The structure deliberately mirrors
 * io/servo_sbus.c, which is the existing precedent for a serial protocol
 * driving outputs.
 *
 * Protocol reference: "Specification for Spektrum SRXL2", Rev K
 * (https://github.com/SpektrumRC/SRXL2, MIT). Half-duplex single wire,
 * 115200 baud for the handshake, optionally negotiated up to 400000.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/*
 * One ESC per bus, several buses.
 *
 * The specification assigns ESCs device IDs 0x40..0x4F so a single bus could
 * carry several, but each would need a distinct unit ID and the specification
 * states that setting one over SRXL2 "is not implemented" - it expects physical
 * switches, which Avian ESCs do not have. So each ESC gets its own port, and a
 * multi-motor model needs as many ports as motors.
 *
 * Four is a practical ceiling: every instance costs a UART, and the protocol's
 * update rate suits aircraft rather than multirotors anyway.
 */
#define SRXL2_ESC_MAX_MOTORS        4

/* Which measurements a telemetry frame actually carried. The wire has a "no
 * data" code per field (0xFFFF, or 0xFF for the byte-sized ones), and a missing
 * measurement must not reach a consumer as a confident zero: nothing downstream
 * can tell 0.00 A meaning "idle" from 0.00 A meaning "this ESC does not report
 * current". */
typedef enum {
    SRXL2_TELEM_FIELD_RPM      = (1 << 0),
    SRXL2_TELEM_FIELD_VOLTAGE  = (1 << 1),
    SRXL2_TELEM_FIELD_CURRENT  = (1 << 2),
    SRXL2_TELEM_FIELD_TEMP_FET = (1 << 3),
    SRXL2_TELEM_FIELD_TEMP_BEC = (1 << 4),
} srxl2TelemetryField_e;

/* Decoded ESC telemetry, from STRU_TELE_ESC (X-Bus sensor ID 0x20).
 * Units are INAV's, not the wire's: the wire sends 10 rpm, 0.01 V, 10 mA and
 * 0.1 degree steps, and this struct is already converted.
 * A field the ESC reports as "no data" is left at 0 and its bit in `fields` is
 * clear; `valid` says only that a frame arrived and is recent. */
typedef struct {
    uint32_t rpm;               /* electrical rpm */
    uint16_t voltage;           /* 0.01 V */
    uint16_t current;           /* 0.01 A */
    int16_t  temperatureFet;    /* 0.1 degC */
    int16_t  temperatureBec;    /* 0.1 degC */
    uint16_t currentBec;        /* 0.01 A */
    uint16_t voltageBec;        /* 0.01 V */
    uint8_t  throttlePercent;   /* 0..100 */
    uint8_t  powerPercent;      /* 0..100 */
    uint32_t lastUpdateMs;
    uint8_t  fields;            /* srxl2TelemetryField_e bits actually reported */
    bool     valid;
} srxl2EscTelemetry_t;

/*
 * ESC throttle-range calibration.
 *
 * Spektrum ESCs learn their endpoints from the signal present as the battery is
 * connected: full throttle first, then low within five seconds. A Spektrum
 * transmitter does this with the stick, and INAV otherwise cannot, because it
 * outputs mincommand while disarmed - which leaves anyone without a Spektrum
 * radio unable to calibrate the ESC at all.
 *
 * These phases override the throttle channel so the sequence can be driven from
 * the flight controller. HIGH and LOW both abort on their own after a timeout, so
 * the output cannot be left commanding full throttle.
 */
typedef enum {
    SRXL2_CAL_OFF = 0,
    SRXL2_CAL_WAIT_BATTERY,     /* full throttle held, waiting for the ESC to power up */
    SRXL2_CAL_SETTLE,           /* ESC powered: holding high while it accepts the endpoint */
    SRXL2_CAL_LOW,              /* low throttle so it accepts the other endpoint */
    SRXL2_CAL_HIGH_MANUAL,      /* fallback for boards with no voltage sensing */
    SRXL2_CAL_LOW_MANUAL,
} srxl2CalPhase_e;

typedef enum {
    SRXL2_CAL_ACCEPTED = 0,
    SRXL2_CAL_REJECT_ARMED,
    SRXL2_CAL_REJECT_NO_PORT,
    SRXL2_CAL_REJECT_BATTERY_PRESENT,
    SRXL2_CAL_REJECT_NO_VOLTAGE_SENSOR,
} srxl2CalResult_e;

/*
 * Run the whole sequence unattended: refuses to start unless the battery is
 * disconnected, then holds full throttle, waits for the ESC to power up, and
 * times the drop to low itself. The five second window in the manual starts at
 * the ESC's tones, which only a person standing there can hear, so timing it
 * from the moment the ESC gains power is the only way to hit it without asking
 * the operator to type against a stopwatch.
 */
srxl2CalResult_e srxl2MotorCalibrationBegin(void);

/* Drive one endpoint by hand. For boards that cannot sense battery voltage and
 * therefore cannot detect the ESC powering up. */
srxl2CalResult_e srxl2MotorCalibrationManual(srxl2CalPhase_e phase);

void            srxl2MotorCalibrationAbort(void);
srxl2CalPhase_e srxl2MotorCalibrationPhase(void);

/*
 * Why the last request to start a calibration was refused, or SRXL2_CAL_ACCEPTED
 * if it was not. MSP cannot carry a reason back on an IN command, so a caller that
 * only saw the command fail could not tell the operator anything useful - and the
 * useful thing here is precisely which precondition was not met.
 */
srxl2CalResult_e srxl2MotorCalibrationLastResult(void);

/*
 * Open the serial port assigned FUNCTION_ESC_SRXL2 and start the handshake.
 * Returns false if no port is assigned or it could not be opened, in which case
 * the caller must fall back to leaving the motors unwritten - this protocol has
 * no silent degradation to PWM, because the pin is not a timer output.
 */
bool srxl2MotorInitialize(void);

/*
 * Stage one motor value. Takes microseconds on INAV's usual 1000..2000 scale
 * (or the reversible-motor scale, where the neutral sits in the middle), so it
 * is interchangeable with pwmWriteMotor() as a motorWritePtr target.
 * Staging only: nothing reaches the wire until srxl2MotorSendUpdate().
 */
void srxl2MotorUpdate(uint8_t index, uint16_t value);

/*
 * Arm or release the Thrust Reverse channel.
 *
 * Reverse on an Avian is not a sub-neutral throttle value: the ESC's
 * "Thrust Rev." parameter selects an auxiliary channel (named CH5..CH9 on a
 * Spektrum transmitter) that arms the Reverse Brake, and Brake Type must be set
 * to Reverse. So the master has to send that channel too, and which one it is
 * has to match how the ESC was programmed.
 */
void srxl2MotorSetReverse(bool armed);

/*
 * Which auxiliary channel arms the Reverse Brake, given as the 1-based channel
 * number the ESC is programmed with (the Avian offers CH5..CH9). Zero disables
 * reverse entirely. Must match the ESC's "Thrust Rev." setting, because nothing
 * on the wire advertises it.
 */
void srxl2MotorSetReverseChannel(uint8_t channel1Based);

/*
 * How often ESC telemetry arrives. Named after the rate the ESC actually delivers,
 * not the rate we ask at: an Avian answers roughly two requests in three and
 * rotates its reply between three sensors - a text page, a battery page and the
 * ESC page - so rpm and current land at about a ninth of the request rate.
 * Measured on an Avian 70A: asking 25 times a second yields 2.7 ESC frames a
 * second, 10 yields 1.1, 5 yields 0.5, 2 yields 0.2.
 *
 * The reply shares the throttle wire, so this trades bus headroom against how
 * promptly rpm and voltage move - which only matters to the RPM filter,
 * everything else being a display.
 *
 * Nothing slower than 1 Hz is offered, and that is a link-liveness limit rather
 * than a taste: the ESC only ever speaks when asked, so the request rate is also
 * the rate at which the master learns the ESC is still there. See
 * SRXL2_LINK_TIMEOUT_MS.
 */
typedef enum {
    /* 1 Hz is first so that it is zero. The setting lives in a field appended to
     * motorConfig_t that fell inside existing padding, so a configuration saved
     * before it existed loads index 0 rather than the reset default - which must
     * therefore be the value we would have chosen anyway, not the fastest one. */
    SRXL2_TELEM_1HZ = 0,
    SRXL2_TELEM_3HZ,
    SRXL2_TELEM_2HZ,
} srxl2TelemetryRate_e;

void srxl2MotorSetTelemetryRate(srxl2TelemetryRate_e rate);

/*
 * Emit the staged values as an SRXL2 Control Data packet. Intended to be called
 * once per motor update from pwmCompleteMotorUpdate().
 */
void srxl2MotorSendUpdate(void);

/*
 * Drain the receive buffer and advance the master state machine: handshake,
 * baud negotiation, telemetry collection, timeout recovery. Must be called
 * from task context, not from an ISR.
 *
 * Wants roughly a 5 ms cadence: that is the tick Spektrum's own reference
 * application advances its state machine with, and it has to run several times
 * faster than the Control Data interval for received frames to be picked up
 * promptly on a half-duplex wire. TASK_PWMDRIVER, which already exists for the
 * SBUS servo output, runs at 200 Hz and fits exactly.
 */
void srxl2MotorProcess(void);

/* How many ports were found and opened. Fewer than the model has motors means
 * some motor has nowhere to send its command, which the caller must treat as a
 * configuration error rather than carrying on. */
uint8_t srxl2MotorCount(void);

/* True once every opened ESC has answered the handshake and is still
 * responding. One silent ESC on a twin is asymmetric thrust, so this is
 * deliberately all of them rather than any. */
bool srxl2MotorIsConnected(void);

/*
 * Latest telemetry for a motor. Returns false if no ESC is bound to that index
 * or nothing has been received yet; the caller should not read *out in that
 * case. Feeds sensors/esc_sensor.c.
 */
bool srxl2MotorGetTelemetry(uint8_t index, srxl2EscTelemetry_t *out);

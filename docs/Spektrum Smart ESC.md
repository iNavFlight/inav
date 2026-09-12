# Spektrum Smart ESC (SRXL2)

Spektrum's "Smart Throttle" is the SRXL2 protocol carried on the ESC's throttle
signal wire: the ESC is an SRXL2 device and the receiver, or here the flight
controller, is the bus master. The same single wire carries the throttle one way
and telemetry the other.

INAV can take that master role, which makes two things available that a PWM
connection cannot give you:

* **Thrust reverse.** On an Avian this is only reachable over Smart Throttle. Wired
  as a conventional PWM ESC there is no way to tell it to reverse at all.
* **Telemetry with no extra wire.** Voltage, current, rpm and temperatures arrive on
  the throttle wire. There is no telemetry lead to run and no ESC telemetry pad to
  connect.

## Wiring

The ESC's normal three-wire servo lead goes to a **UART**, not to a motor pad:

| Wire | Goes to |
|---|---|
| signal | the **TX pin** of a port assigned `Spektrum Smart ESC (SRXL2)` |
| +5 V (BEC) | as usual |
| ground | as usual |

The signal goes to TX rather than RX because the protocol is half duplex: one
conductor carries both directions, and the UART is put into single-wire mode.

Nothing connects to the flight controller's ESC telemetry pad. That pad is for ESCs
with a separate telemetry lead, such as BLHeli or HobbyWing; a Smart ESC has no such
lead.

The motor pad that would normally have driven this ESC is simply left unused.

## Setting it up

1. **Ports tab** — assign `Spektrum Smart ESC (SRXL2)` to a spare UART.
2. **Outputs tab** — set the ESC protocol to `SRXL2`.
3. **Outputs tab** — set **Motor poles** correctly. This matters more than usual; see
   the RPM filter section below.
4. If the ESC is programmed for reverse, set **Thrust Reverse channel** to the
   channel its own `Thrust Rev.` parameter selects.

Both steps 1 and 2 are needed. This protocol has **no fallback to PWM**: the pin is
a UART pin, not a timer output, so a motor protocol of `SRXL2` with no port assigned
means the motor is never driven. The Outputs tab warns when that is the case.

If your firmware was not built with this support the protocol does not appear in the
list at all, and neither does the port function. It is enabled by default on H7 and
AT32 targets; other targets can add `#define USE_MOTOR_SRXL2` to their `target.h`.

## One ESC per port

A single SRXL2 bus can address several ESCs, at device IDs 0x40 to 0x43, but each
needs a distinct unit ID and the specification states that setting a unit ID over
SRXL2 "is not implemented" — it expects physical switches or jumpers, which Avian
ESCs do not have. So in practice one ESC per bus.

This driver drives one ESC, on one port. Nothing in the protocol prevents a motor
per port, and for a twin-engine fixed-wing that would be reasonable, but note that
SRXL2 sends at tens of hertz by design, where DSHOT sends at kilohertz. That is
ample for an aircraft holding a cruise throttle and nowhere near enough for a
multirotor, whatever the wiring.

## Throttle range calibration

A Spektrum ESC learns its throttle endpoints from the signal present as it powers
up: full throttle first, then low within five seconds of the tones that acknowledge
it. That normally needs a Spektrum transmitter, so INAV can drive the sequence
itself for anyone who does not own one.

**Outputs tab**, Throttle range calibration:

1. Remove the propeller and disconnect the battery. The wizard refuses to start with
   the battery connected, and the button stays inert until you confirm both.
2. Press **Start calibration**. Full throttle goes on the wire — with no battery,
   nothing can spin.
3. Connect the battery. The ESC sounds its tones, and the throttle drops to minimum
   by itself about three seconds later.
4. A long tone means the range was stored.

From the CLI the same thing is `esc_calibrate start`, with `esc_calibrate high` and
`low` available for boards that cannot sense battery voltage and therefore cannot
detect the ESC powering up.

Either phase ends on its own if left alone, and arming cancels a sequence in
progress.

## Telemetry and the RPM filter

Telemetry is read from the SRXL2 link and feeds everything that consumes ESC
telemetry: OSD, Blackbox, current estimation, and the gyro RPM filter. It can be
switched off with `esc_srxl2_telemetry`, which exists because telemetry shares the
throttle wire and so cannot be declined by leaving a port unassigned as it would be
for a conventional ESC.

**Set Motor poles correctly before enabling the RPM filter.** The wire carries
electrical rpm, and INAV converts it to mechanical rpm using the pole count. A wrong
pole count puts the notch at the wrong frequency, which is worse than having no
notch at all.

Two things to weigh before turning `rpm_gyro_filter_enabled` on:

* Telemetry arrives at roughly 10 Hz, not at the loop rate. On a single-motor
  aircraft holding a cruise throttle, rpm and the vibration peak both move slowly
  and that is adequate. It is not equivalent to bidirectional DSHOT.
* INAV's own advice for this setting applies unchanged: turn it on only once ESC
  telemetry is working and the reported rpm looks right.

## Reverse

Reverse on an Avian is not a throttle value below neutral. The ESC's `Thrust Rev.`
parameter selects an auxiliary channel, named CH5 to CH9 on a Spektrum transmitter,
that arms the Reverse Brake, and `Brake Type` must be set to `Reverse`.

So two things have to agree:

* the ESC, programmed with `Brake Type = Reverse` and a `Thrust Rev.` channel;
* INAV, with `esc_srxl2_reverse_channel` set to the same channel.

Nothing on the wire advertises which channel the ESC is watching, so a mismatch
simply means reverse never engages.

INAV arms that channel from the mixer: when it has decided the motor should run
backwards, reverse is armed. There is no separate switch to set, deliberately — a
switch of its own could disagree with the direction INAV had chosen, and thrust
reverse is the last place that should happen.

Note that `Brake Type = Reverse` changes what the throttle range means to the ESC:
centre becomes zero thrust. Configure INAV for reversible motors to match, or at
minimum throttle the aircraft will push backwards.

## Settings

| Setting | Meaning |
|---|---|
| `motor_pwm_protocol = SRXL2` | drive motors over SRXL2 |
| `esc_srxl2_reverse_channel` | 1-based channel the ESC uses to arm reverse; 0 disables |
| `esc_srxl2_telemetry` | read telemetry from the SRXL2 link |
| `motor_poles` | required for correct rpm, see above |

## Reference

Protocol: [Specification for Spektrum SRXL2](https://github.com/SpektrumRC/SRXL2),
published by Horizon Hobby under the MIT licence. The library in that repository
implements the device side; the bus master side is not part of the open release, so
the master in INAV is written from the specification.

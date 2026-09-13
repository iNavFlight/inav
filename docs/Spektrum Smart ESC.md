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

1. **Ports tab** — assign `Spektrum Smart ESC (SRXL2)` to a spare UART, one per
   motor. They are matched to motors in port order; see below.
2. **Outputs tab** — set the ESC protocol to `SRXL2`.
3. **Outputs tab** — set **Motor poles** correctly. This matters more than usual; see
   the RPM filter section below.
4. If the ESC is programmed for reverse, set **Thrust Reverse: ESC channel** to the
   channel its own `Thrust Rev.` parameter selects, then assign the **THRUST
   REVERSE** mode to a switch in the Modes tab. See below.

Both steps 1 and 2 are needed. This protocol has **no fallback to PWM**: the pin is
a UART pin, not a timer output, so a motor protocol of `SRXL2` with no port assigned
means the motor is never driven. The Outputs tab warns when that is the case.

If your firmware was not built with this support the protocol does not appear in the
list at all, and neither does the port function. It is enabled by default on H7 and
AT32 targets; other targets can add `#define USE_MOTOR_SRXL2` to their `target.h`.

## One ESC per port, several ports

A single SRXL2 bus can address several ESCs, at device IDs 0x40 to 0x4F, but each
would need a distinct unit ID and the specification states that setting a unit ID
over SRXL2 "is not implemented" — it expects physical switches or jumpers, which
Avian ESCs do not have. So it is one ESC per bus, and a model with several motors
needs a port for each.

Up to four are supported. **Motors are matched to ports in order:** motor 1 is the
lowest-numbered assigned UART, motor 2 the next, and so on. Nothing on the wire
says which motor an ESC drives, so the wiring order is what carries that.

If there are fewer ports than the mixer has motors, the board **refuses to arm**
and reports `Not enough motor outputs/timers`. A motor with no port has nowhere to
send its command and no timer output to fall back on, so a twin that can only
drive one side must not be allowed into the air. The Outputs tab says so before it
gets that far.

Each port is an independent bus: its own handshake, its own baud negotiation, its
own telemetry. A twin with one ESC unplugged therefore reports the link as down
rather than partly up.

Note that SRXL2 sends at tens of hertz by design, where DSHOT sends at kilohertz.
That is ample for an aircraft holding a cruise throttle and nowhere near enough
for a multirotor, whatever the wiring.

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

With more than one ESC all of them are calibrated together. They share a battery,
so they power up together and the window the sequence aims at is the same window
for all of them.

## Telemetry and the RPM filter

Telemetry is read from the SRXL2 link and feeds everything that consumes ESC
telemetry: OSD, Blackbox, current estimation, and the gyro RPM filter. Each port's
ESC reports as its own motor, so motor 2's telemetry is motor 2's. It can be
switched off with `esc_srxl2_telemetry`, which exists because telemetry shares the
throttle wire and so cannot be declined by leaving a port unassigned as it would be
for a conventional ESC.

**Set Motor poles correctly before enabling the RPM filter.** The wire carries
electrical rpm, and INAV converts it to mechanical rpm using the pole count. A wrong
pole count puts the notch at the wrong frequency, which is worse than having no
notch at all.

Two things to weigh before turning `rpm_gyro_filter_enabled` on:

* Telemetry arrives at whatever `esc_srxl2_telemetry_rate` asks for, 10 Hz by
  default, not at the loop rate. On an aircraft holding a cruise throttle, rpm and
  the vibration peak both move slowly and that is adequate. Raising it to 50 Hz
  tracks better and costs bus headroom, since the reply shares the throttle wire.
  Even at 50 Hz this is not equivalent to bidirectional DSHOT, which reports every
  loop.
* INAV's own advice for this setting applies unchanged: turn it on only once ESC
  telemetry is working and the reported rpm looks right.

## Reverse

Reverse on a Smart ESC is a switch, not a throttle value below neutral. The ESC's
`Thrust Rev.` parameter names an auxiliary channel; when that channel goes high the
ESC reverses, and Spektrum describe the effect plainly — *"flipping the designated
switch reverses motor rotation, throttle will still control motor speed"*. So the
throttle goes on meaning throttle.

Three things have to agree:

* the ESC, programmed with a `Thrust Rev.` channel (and `Brake Type = Reverse` on
  models that have it);
* `esc_srxl2_reverse_channel`, set to that same channel;
* a switch, assigned to the **THRUST REVERSE** mode in the Modes tab.

Spektrum allow channels **5 to 9** for this and ship **channel 7** as the factory
default. Nothing on the wire advertises which one the ESC is watching, so a
mismatch simply means reverse never engages — silently. Check the channel against
your own ESC's programming rather than trusting the default: the parameter is not
present on every Avian model, and where it is present the range and default have
varied.

The channel setting is not a transmitter channel. It selects a slot on the SRXL2
wire between the flight controller and the ESC. The transmitter switch is chosen in
the Modes tab like any other mode.

`esc_srxl2_reverse_channel` applies to every ESC on the model, so a twin needs both
ESCs programmed with the same `Thrust Rev.` channel. Reverse is armed on all of
them together and never on one alone: asymmetric reverse thrust on a twin is the
outcome most worth engineering against.

Reverse is released whenever the aircraft is disarmed, so a machine that landed
under reverse does not sit on the ground with it still armed.

### If your ESC uses a centred throttle instead

Some Spektrum documentation shows a bipolar throttle scale for the reverse brake
mode, where centre is zero thrust and below centre is reverse. That is not what the
switch-and-normal-throttle wording above describes, and the two cannot both be true
of the same ESC, so this is worth checking on your own hardware before the first
flight.

If yours behaves that way, enable INAV's own `FEATURE_REVERSIBLE_MOTORS`. The
driver honours that too: when the mixer decides the motor should run backwards, the
reverse channel is armed, exactly as the THRUST REVERSE mode would. Either route
works and neither masks the other.

Be aware of what that feature does to an aeroplane, though, which is why it is not
the default route:

* the throttle stick becomes centre-zero, so forward thrust lives only in the top
  half of the stick;
* chopping the throttle on short final then commands reverse thrust in the air;
* arming requires the throttle stick **centred** rather than down, and an ARM
  switch becomes mandatory — INAV disarms continuously without one.

For an aeroplane that wants reverse only on the landing roll, the mode is the right
answer and the feature is not.

### What reverse cannot do

Reverse is a manual, stick-and-switch capability. INAV's automatic throttle paths —
RTH, autoland, failsafe, launch — all clamp throttle to at least idle, so none of
them can call for reverse thrust. An automatic landing will not use it.

## Settings

| Setting | Meaning |
|---|---|
| `motor_pwm_protocol = SRXL2` | drive motors over SRXL2 |
| `esc_srxl2_reverse_channel` | SRXL2 channel the ESC watches for reverse, 5 to 9; Spektrum default 7, 0 disables |
| `esc_srxl2_telemetry` | read telemetry from the SRXL2 link |
| `esc_srxl2_telemetry_rate` | how often the ESC is asked: 50, 25, 10, 5 or 2 Hz |
| `motor_poles` | required for correct rpm, see above |

## Reference

Protocol: [Specification for Spektrum SRXL2](https://github.com/SpektrumRC/SRXL2),
published by Horizon Hobby under the MIT licence. The library in that repository
implements the device side; the bus master side is not part of the open release, so
the master in INAV is written from the specification.

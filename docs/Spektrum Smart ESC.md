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

**Do this before the first flight.** It is not a convenience. Measured on an Avian
70 A, sweeping the channel value across its whole range and reading back the
throttle the ESC reports:

| | uncalibrated | after calibration |
|---|---|---|
| starts responding at | 12220 (1178 us) | 2687 (1029 us) |
| saturates at | 50820 (1781 us) | 64307 (1993 us) |
| share of the channel used | 59 % | 94 % |

Uncalibrated, the bottom sixth of the throttle does nothing and the top fifth is
already at full power, so the stick reaches everything it will ever reach at
about three quarters travel - and nothing says so. Calibrated, what the ESC
reports tracks what INAV commands to within a point across the whole range:
1050 us gives 5 %, 1500 gives 50 %, 2000 gives 100 %. A pilot who skips this
finds out about it on the takeoff roll.

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

**Current needs care until this is settled.** On the bench the ESC reported 2.85 A
while the supply feeding it measured 0.999 A at the same instant, at about 40 %
throttle. That is the relationship expected if the field is motor current rather
than pack current, since an ESC is a converter and pack current is roughly motor
current times duty. It is not proven: a plain scale error in the ESC's own sensor
would look identical at a single operating point, and telling the two apart needs
readings at several throttle settings. Until then, prefer the board's own sensor
with `current_meter_type = ADC` wherever one exists. The two are alternatives
rather than additive - INAV takes current from one source - and a shunt in the
battery lead also sees what the servos and the video transmitter draw, which no
ESC can report.

**Set Motor poles correctly before enabling the RPM filter.** The wire carries
electrical rpm, and INAV converts it to mechanical rpm using the pole count. A wrong
pole count puts the notch at the wrong frequency, which is worse than having no
notch at all.

**The rpm this delivers is too slow for the filter to track a changing throttle,
and that is a hardware limit, not a tuning one.** Measured on an Avian 70 A: the
ESC answers about two requests in three and rotates its reply between a text
page, a battery page and the ESC page, so rpm arrives at roughly a ninth of the
request rate - 2.7 readings a second at the fastest setting the link tolerates,
1.1 at the default. There is no margin to recover, either: requesting on every
frame makes the ESC stop obeying the throttle, and this ESC advertises no support
for 400000 baud, so the wire cannot be made faster.

For comparison, bidirectional DSHOT reports rpm every loop. Two to three orders
of magnitude separate the two, so:

* **On a multirotor, leave `rpm_gyro_filter_enabled` off.** The vibration peak
  moves with the throttle several times a second, and a notch updated twice a
  second spends most of its time in the wrong place - which is worse than no
  notch, because it attenuates signal rather than noise.
* **On a fixed wing holding a cruise throttle** the peak moves slowly and the
  filter is arguably useful. Arguably: that the update rate is adequate there is
  reasoning from how slowly cruise rpm changes, not something measured in flight.
  If you try it, compare a logged flight with it on and off before trusting it.
* INAV's own advice for this setting applies unchanged: turn it on only once ESC
  telemetry is working and the reported rpm looks right.

## Reverse

Reverse on a Smart ESC is a switch, not a throttle value below neutral. The ESC's
`Thrust Rev.` parameter names an auxiliary channel; when that channel goes high the
ESC reverses, and Spektrum describe the effect plainly — *"flipping the designated
switch reverses motor rotation, throttle will still control motor speed"*. So the
throttle goes on meaning throttle.

Three things have to agree:

* the ESC, with **both** of its own parameters set: `Brake Type` to `Reverse`,
  which is what enables reversing at all, and `Thrust Rev.`, which only chooses
  the channel that arms it. Spektrum's programming instructions are explicit that
  one does not work without the other - *"Thrust Rev - Use this option to select
  the channel used to activate motor reversing. Reverse must set in the Brake Type
  menu"*. They also recommend `Brake Force` of 7 alongside `Brake Type = Reverse`;
* `esc_srxl2_reverse_channel`, set to that same channel;
* a switch, assigned to the **THRUST REVERSE** mode in the Modes tab.

Pick a channel nothing else uses. This is Spektrum's own warning about the
parameter, and it is about flight behaviour rather than tidiness: *"Reverse mode
needs to be assigned to an OPEN channel on your transmitter, this channel is
selected in ESC menu item #15 using in conjunction with another function can cause
unexpected behavior in flight."* On this link the channel is a slot between the
flight controller and the ESC rather than a transmitter channel, so the flight
controller is what has to leave it alone - which is why the setting refuses the
throttle slot.

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

### Check it on the bench before the first flight

Some Spektrum documentation shows a bipolar throttle scale for the reverse brake
mode, where centre is zero thrust and below centre is reverse. That contradicts the
switch-and-normal-throttle behaviour above, and the two cannot both be true of the
same ESC. INAV drives the switch arrangement, which is the one Spektrum state in
words, so it is worth confirming your ESC is that kind.

An Avian 70 A with `Brake Type = Reverse` and the factory `Thrust Rev.` channel
was confirmed to be that kind: with channel 7 low and then high, against an
unchanged 1250 us throttle, the motor ran counter-clockwise, clockwise,
counter-clockwise, clockwise, reporting the same 14.0 % and about 6610 rpm in
every case. The throttle never changed meaning, and telemetry says nothing about
which way the shaft is turning - only an eye on the motor can tell you.

**Propeller off.** Arm with THRUST REVERSE off and watch the motor at *minimum*
throttle:

* **stopped**, or idling gently forward — the switch arrangement, which is what
  this driver expects. Nothing more to do.
* **spinning backwards hard** — the other kind. Disarm. INAV does not drive that
  arrangement over SRXL2: the throttle it sends would be read as reverse thrust
  through most of the stick.

`FEATURE_REVERSIBLE_MOTORS` is not the answer to the second case and is cleared
automatically when the protocol is SRXL2. It recentres INAV's own throttle output,
which on a switch-type ESC means roughly half throttle at the point the stick says
stop.

### Engaging it with the motor running

Nothing stops the switch being thrown at speed, on the ESC's side or INAV's:
`srxl2MotorSetReverse()` writes its channel whenever the mode is active, without
consulting the throttle. Tried deliberately on the bench, with the motor turning
at 2690 rpm under an unchanged 1200 us command, the Avian simply changed
direction - one telemetry sample caught it passing through zero rpm, and the next
had it back at the same speed the other way. No stall, no cutout, and no current
step large enough to read at that load.

That is the ESC behaving well, not a licence to do it. With a propeller loaded in
flight the same reversal has to absorb the airflow driving the blades, which is
a different question from a bare motor on a bench, and it is the reason Spektrum
put reverse on a switch the pilot has to mean to throw.

### What reverse cannot do

Reverse is a manual, stick-and-switch capability. INAV's automatic throttle paths —
RTH, autoland, failsafe, launch — all clamp throttle to at least idle, so none of
them can call for reverse thrust. An automatic landing will not use it.

## If the motor does not come back after a reboot

An Avian announces itself for about a third of a second after it powers up - six
handshakes in 300 milliseconds, measured on a 70 A - and then never speaks again
unless it is asked something it recognises. A flight controller that starts while
the ESC is already running has missed that window, and the ESC will not answer it
afterwards: polled by name, broadcast to, addressed on every ID from 0x40 to
0x4F, or spoken to as though the link already existed, it stayed silent through
every one.

So the link is made at power-up or not at all. Connecting the battery powers both
together and the announcement lands while the flight controller is listening,
which is the normal case and needs nothing. The cases that bite are the other
ones:

* the flight controller reboots - a firmware update, a brownout, the Configurator
  asking for a restart - while the battery stays connected. **Unplug the battery
  and plug it in again**, or the motor will not respond.
* bench work on USB with the ESC powered from a separate supply. Power the ESC
  after the board has booted, not before.

Nothing in the firmware can work around this, so it refuses to hide it instead:
arming is blocked while an SRXL2 link is missing, and the OSD says the hardware
is not there.

What the firmware does not do is give up on an ESC that has gone quiet while it
is being flown. Telemetry has nothing to do with the throttle on these ESCs:
measured with a bench supply as the witness, an Avian held 0.30 A through ten
seconds with no telemetry requested at all. What stops the motor is the absence
of control frames - the current falls to the ESC's own 58 mA within about half a
second - and it takes the throttle back up by itself when frames return, with no
re-arm and no power cycle. So a silent ESC keeps being commanded, and only the
telemetry goes stale. Where ESC and board come up together the block clears in about a
second and is never seen; where it does not clear, the throttle would have done
nothing anyway.

## Settings

| Setting | Meaning |
|---|---|
| `motor_pwm_protocol = SRXL2` | drive motors over SRXL2 |
| `esc_srxl2_reverse_channel` | SRXL2 channel the ESC watches for reverse, 5 to 9; Spektrum default 7, 0 disables |
| `esc_srxl2_telemetry` | read telemetry from the SRXL2 link |
| `esc_srxl2_telemetry_rate` | how often telemetry arrives: 3, 2 or 1 Hz |
| `motor_poles` | required for correct rpm, see above |

## Reference

Protocol: [Specification for Spektrum SRXL2](https://github.com/SpektrumRC/SRXL2),
published by Horizon Hobby under the MIT licence. The library in that repository
implements the device side; the bus master side is not part of the open release, so
the master in INAV is written from the specification.

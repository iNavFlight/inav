# Board - DAKEFPV F405 WING

## Specification:

* STM32F405 CPU
* 6 UART serial ports (UART1-6)
* 10 motor/servo outputs (S1-S10)
* WS2811/WS2812 LED strip output
* Board identifier: `DK4W`

## Details

* By default UART1 is set up for a serial receiver (SBUS/CRSF/etc.), UART3 for GPS, and UART6 for MSP.
* Out of the box the board is configured for 2 motors, on S1 and S2, with the LED strip working.
* S3 through S10 default to servo outputs. Most of them can be switched over to additional DSHOT-capable motor outputs — see below.
* **S4 and S5 can never run DSHOT**, regardless of configuration. They are always plain PWM/servo outputs.

## Setting up more than 2 motors

Adding motors is done by changing the per-timer output mode, found in the Mixer
tab of INAV Configurator (also available from the CLI as `timer_output_mode`).

How many motors you can have, and which outputs they land on, depends on whether
you need the addressable LED strip.

### Outputs that must be changed together

Some outputs share an internal timer and can only be switched between motor and
servo mode as a group. You cannot make one a motor while its partner stays a servo:

| Outputs | Notes |
|---|---|
| S2 + S3 | Always switch together |
| S6 + S7 | Always switch together |
| S8 + S9 + S10 | Always switch together (group of three) |
| S4 + S5 | Always servos — cannot run DSHOT under any configuration |

### With the LED strip: up to 6 motors

S3 and the LED strip output cannot both be active at the same time. Because S2
and S3 share a timer, this means **S2 cannot be used as a motor in any setup
that has more than 2 motors and keeps the LED strip** — as soon as a third motor
exists, S3 is pulled in as a motor as well, and the LED strip stops working.

To run 3 or more motors with the LED strip still working:

1. In the Mixer tab, set the output mode for the **S2/S3 timer to Servo**.
2. Set your mixer's motor count as usual.

Motors are then assigned in this order:

| Motor | Output |
|---|---|
| 1 | S1 |
| 2 | S6 |
| 3 | S7 |
| 4 | S8 |
| 5 | S9 |
| 6 | S10 |

**Maximum 6 motors** in this configuration. S2, S3, S4 and S5 remain available
as servos.

Note that motor 2 is **S6**, not S2 — the motor outputs are not numbered
contiguously once S2/S3 have been reserved for servos.

### Without the LED strip: up to 8 motors

If you do not need the addressable LED strip, leave all output modes on Auto and
just set your motor count. Motors are assigned in this order:

| Motor | Output |
|---|---|
| 1 | S1 |
| 2 | S2 |
| 3 | S3 |
| 4 | S6 |
| 5 | S7 |
| 6 | S8 |
| 7 | S9 |
| 8 | S10 |

**Maximum 8 motors.** S4 and S5 remain available as servos. The LED strip will
not work once you have 3 or more motors.

### Cautions

* **Do not set a motor count higher than the number of motor outputs actually
  available** — 6 with the LED strip, 8 without. If you do, motor initialization
  fails completely and *no* motors will work, not just the extra ones.
* Prefer setting a timer to **Servo** to keep its outputs away from motors,
  rather than forcing timers to **Motor**. Forcing Motor mode can consume more
  output slots than you expect, and can silently leave an output you wanted as a
  motor configured as a servo instead.

After changing any output mode, save and let the board reboot, then set up your
motors in the Mixer tab. **Before flying, verify each motor output individually**
— spin each motor briefly at low throttle, one at a time, and confirm it is the
one you expect — rather than assuming outputs map to motor numbers in a
particular order.

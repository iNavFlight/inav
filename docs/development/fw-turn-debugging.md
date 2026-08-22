# Fixed-wing coordinated turn debugging (`DEBUG_FW_TURN`)

`set debug_mode = FW_TURN` exposes the fixed-wing coordinated WP turn system: the arc turn
coordinator, the S sequencer (COORD_FLYOVER with path tracking / COORD_FLYINTO), the loiter
radius stabiliser, the turn/loiter feed-forward and the energy bank guard. Values are available
via the CLI `debug` command, the `OSD_DEBUG` element, MSP `DEBUGMSG`/`DEBUG` telemetry and
blackbox logging.

## Channels

| debug[] | Value | Unit | Written by |
|---|---|---|---|
| 0 | Active turn/loiter radius | cm | Loiter stabiliser every cycle; overridden by the COORD_FLYBY planning radius while a corner approach is active, and by the engaged arc's radius while the coordinator flies the turn (latest writer wins) |
| 1 | Coordinator state (see below) | – | Arc coordinator |
| 2 | Exit course of the active arc | centideg | Arc coordinator, only while engaged |
| 3 | Remaining heading to the exit course | centideg | Arc coordinator, only while engaged |
| 4 | Arc bank command | centideg | Arc coordinator, only while engaged; clamped to the effective bank ceiling |
| 5 | Turn/loiter roll feed-forward | centideg | Feed-forward (0 when disabled, not established on the loiter circle, or inside the heading deadband) |
| 6 | Energy-guard bank ceiling | deg | Energy bank guard (sits at `max_angle_inclination_rll` unless the guard is reducing it) |
| 7 | Roll ease time | ms | Arc coordinator, only while engaged (sizes the entry/exit ramps and the turn-start lead) |

Channels 2, 3, 4 and 7 hold their last value after the arc disengages; check channel 1 to know
whether the coordinator is active.

## Channel 1: coordinator state

While the arc coordinator is engaged the value is `(arc phase + 1) * 10 + S stage`; while idle
it is the S stage alone.

Arc phase (tens digit):

| Digit | Phase | Meaning |
|---|---|---|
| 1 | RAMP_IN | Smoothstep bank ramp onto the pre-placed circle |
| 2 | STEADY | Coordinated arc: live feed-forward bank + radius/tangent feedback |
| 3 | CAPTURE | Predictive roll-out onto the exit course |

S stage (ones digit; 0 outside the S modes):

| Digit | Stage | Meaning |
|---|---|---|
| 0 | – | Plain single-arc turn (COORD_FLYBY, COORD_FLYOVER tangent exit) |
| 1 | AWAY | First arc of the S; the second arc is staged |
| 2 | MAIN | Second arc of the S (COORD_FLYINTO: the aligned WP crossing; COORD_FLYOVER + tracking: the corner-cut onto the leg) |
| 3 | DONE | S completed for this leg, waiting for the next leg switch |

Examples: `20` = flying a plain coordinated turn; `21` = steady on the first S arc; `32` =
rolling out of the S's second arc; `3` (idle) = S finished, coordinator handed back.

## Reading a turn

A healthy COORD_FLYBY corner shows: channel 1 stepping `10 → 20 → 30 → 0`, channel 4 ramping to
the nominal bank, holding, then collapsing as channel 3 converges to 0 without overshoot. The
hand-back happens aligned (|ch3| small) and nearly level (|ch4| < 10% of nominal). Channel 4
pegged at channel 6 × 100 indicates the command is saturated at the flyable ceiling — expected
briefly downwind, a problem if sustained. Channel 6 dropping below `max_angle_inclination_rll`
means the energy guard is trading bank for climb capability.

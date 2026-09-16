# When to Add a New Setting

Before adding a settings.yaml entry, ask: can this value be computed instead
of asked for?

## Add a setting when...

A setting is warranted when *either* of these holds:

1. **Aircraft-specific and uncomputable** — different aircraft genuinely
   need different values for it, **and** the correct value can't be
   computed from existing settings or sensor readings. It directly
   describes a fact about the aircraft/pilot the firmware has no other way
   to know.
2. **Genuine user preference** — it's a real preference question the user
   can reasonably answer on its own terms (the setting makes sense *to the
   user*). If it could instead be computed from other known preferences,
   compute it — don't ask twice for the same fact.

## Don't add a setting when...

- The value can be computed or derived from something already known
  (existing settings, sensor state, calibration state, etc.) — compute it
  instead.
- The user can't actually give a meaningful answer (a purely internal
  algorithm-tuning parameter, e.g. a statistical confidence ratio). If
  neither we can compute the value nor the user can answer it, look for an
  indirect signal — another setting, a sensor reading, a calibration-state
  flag — that lets us derive it, rather than adding a setting nobody can
  fill in well.

## Prefer settings that describe the aircraft or user, not a function's parameter

A setting should describe a *fact*, not a *parameter to one computation*.
`hover_throttle` is a good setting: it's a fact about the aircraft ("this
much throttle is needed to hover") that's independently useful — position
hold, altitude hold, any future feature needing an initial throttle guess
can all reuse it. `initial_throttle_to_be_used_for_position_mode` would be a
bad setting: it's really `hover_throttle` wearing a disguise, named after
one call site, so it can't be reused elsewhere and invites a near-duplicate
setting every time another feature wants the same fact.

## Worked example: compass auto-orientation-detection (proposed feature)

This walks through the settings design for a proposed (not yet merged)
compass auto-orientation-detection feature. Three candidate settings came up
during design and all three were dropped:

1. `align_mag_auto` (enable/disable flag) — dropped. The right gating
   signal already existed: `!STATE(COMPASS_CALIBRATED) &&
   !STATE(ACCELEROMETER_CALIBRATED)`, i.e. "has this board ever been
   calibrated." That's computable from existing calibration state, and it's
   a *better* signal than a manual flag — a flag risks being left on and
   silently re-triggering on a routine future recalibration months later.
2. `align_mag_auto_confidence_min` — dropped in favor of a fixed
   compile-time constant. This one looked like a legitimate safety knob at
   first, but fails the "can the user reasonably answer this" test: nobody
   picking a value between 1.0 and 10.0 for a statistical variance ratio has
   a principled basis to know their choice is better than the default.
   Unlike `mag_calibration_time` (a user can reason about "how long did I
   spin the aircraft"), this parameter only makes sense inside the
   algorithm — exposing it just invites a worse-than-default guess.
3. A settings-based enable path was the *first* idea, before checking
   whether existing calibration state could be used instead. That's the
   ordering to prefer generally: look for a computable/derivable signal
   before reaching for a new setting.

Net result: as designed, the feature needs zero new settings.yaml entries.

## See also

[README.md](README.md), [versioning-rules.md](versioning-rules.md),
[registration-guide.md](registration-guide.md)

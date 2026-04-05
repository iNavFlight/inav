# INS_RC_ORIG

Custom INS fork feature for preserving original RX channel values when `MSP RC Override` is active.

This branch is intended for local INS builds and flight testing. It is not an upstream INAV feature at this stage.

## Why this exists

With standard `MSP RC Override`, the final RC channels seen by the flight controller are the overridden ones. That is fine for control, but it makes it hard for an external computer to also read the pilot's original stick input at the same time.

`INS_RC_ORIG` solves that by mirroring selected **pre-override** RX channels into spare destination channels.

Typical use case:

- channels `1..4` are used for flight control and may be overridden by MSP
- spare channels such as `18`, `19`, `20`, `21` carry the original RX values
- an external controller can read both the overridden controls and the pilot's original inputs over MSP

## What it does

The feature adds four configurable copy rules.

For each rule:

1. INAV reads and processes normal RX input.
2. A snapshot of the original processed RX channels is stored.
3. `MSP RC Override` is applied normally.
4. The selected original channel is copied into the selected destination channel.

The destination channel therefore always contains the **original value from before MSP override**.

## Requirements

- firmware must be built with `USE_MSP_RC_OVERRIDE`
- `MSP RC Override` must be configured in the normal way if you want channels to be overridden
- destination channels should be spare channels that are not already used for important functions

## CLI settings

Four source/destination pairs are available:

- `rc_orig_src_1` and `rc_orig_dst_1`
- `rc_orig_src_2` and `rc_orig_dst_2`
- `rc_orig_src_3` and `rc_orig_dst_3`
- `rc_orig_src_4` and `rc_orig_dst_4`

Rules are **1-based** channel numbers:

- `0` disables the source or destination side of that rule
- valid channel range is `1..34`

## Recommended setup

For the common Raspberry Pi use case:

- keep MSP override on channels `1..4`
- mirror original pilot input to high spare channels

Example:

```text
set msp_override_channels = 15
set rc_orig_src_1 = 1
set rc_orig_dst_1 = 18
set rc_orig_src_2 = 2
set rc_orig_dst_2 = 19
set rc_orig_src_3 = 3
set rc_orig_dst_3 = 20
set rc_orig_src_4 = 4
set rc_orig_dst_4 = 21
save
```

Meaning:

- channel `1` may be overridden by MSP
- channel `18` always carries the original value of channel `1`
- channel `2` may be overridden by MSP
- channel `19` always carries the original value of channel `2`
- channel `3` may be overridden by MSP
- channel `20` always carries the original value of channel `3`
- channel `4` may be overridden by MSP
- channel `21` always carries the original value of channel `4`

## Example with only roll and pitch

If you only need two original channels:

```text
set rc_orig_src_1 = 1
set rc_orig_dst_1 = 18
set rc_orig_src_2 = 2
set rc_orig_dst_2 = 19
set rc_orig_src_3 = 0
set rc_orig_dst_3 = 0
set rc_orig_src_4 = 0
set rc_orig_dst_4 = 0
save
```

## MSP behavior

`MSP_RC` is extended up to the highest configured destination channel so the mirrored channels are visible to external consumers.

Example:

- if your receiver normally exposes 16 channels
- and you mirror to channels `18` and `19`
- `MSP_RC` will return channels up to `19`

This allows the external controller to read:

- final overridden channels in their normal positions
- original mirrored channels in the configured destination positions

## Important behavior notes

- Destination channels are written **after** MSP override.
- Destination channels contain the original RX value, not the MSP value.
- If `src == dst`, that channel is effectively restored to the original pre-override value after MSP override.
- If a source channel is outside the active RX channel count, that rule is ignored.
- If a destination channel is used for arming, modes, RSSI, gimbal control, or anything else important, the copied value will overwrite that destination every cycle.

## Recommended safety rules

- Use only spare AUX channels as destinations.
- Do not map mirrored originals into channels that drive flight modes unless you explicitly want that behavior.
- Bench test with props removed before first flight.
- Verify channel behavior in Configurator and over MSP before airborne testing.

## Quick verification checklist

1. Build and flash firmware with `USE_MSP_RC_OVERRIDE`.
2. Set up your normal `MSP RC Override` mode and `msp_override_channels`.
3. Configure the `rc_orig_src_*` and `rc_orig_dst_*` rules.
4. Move the sticks with MSP override disabled and confirm destination channels mirror the source channels.
5. Enable MSP override and confirm:
   - source control channels follow MSP override
   - destination channels still follow the pilot's original RX input

## Implementation summary

Main changes for this feature live in:

- [src/main/rx/rx.c](src/main/rx/rx.c)
- [src/main/rx/rx.h](src/main/rx/rx.h)
- [src/main/fc/settings.yaml](src/main/fc/settings.yaml)
- [src/main/fc/fc_msp.c](src/main/fc/fc_msp.c)

## Status

Current intent:

- custom INS branch feature
- useful for local builds and Pi-assisted control experiments
- kept separate from upstream until the behavior is proven in real use

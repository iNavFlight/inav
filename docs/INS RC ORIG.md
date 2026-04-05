# INS RC ORIG

`INS RC ORIG` adds configurable pre-override RC channel copies.

The goal is to preserve original RX values even when `MSP RC Override` is active. The firmware stores a snapshot of the processed RX channels before MSP override is applied, then copies selected original channels into selected destination channels.

## How it works

- Source channels are read from the normal RX path before `MSP RC Override`.
- `MSP RC Override` is applied as usual to the configured override mask.
- After that, the configured copy rules write the original pre-override values into the destination channels.
- Destination channels therefore always contain the original RX value, not the overridden MSP value.

This is intended for routing original stick data into spare AUX channels such as `18`, `19`, `20`, `21` so an external controller can read both:

- overridden flight channels `1..4`
- original RX channels mirrored into spare channels

## CLI settings

Four copy rules are available:

- `rc_orig_src_1` / `rc_orig_dst_1`
- `rc_orig_src_2` / `rc_orig_dst_2`
- `rc_orig_src_3` / `rc_orig_dst_3`
- `rc_orig_src_4` / `rc_orig_dst_4`

Rules use 1-based RC channel numbers.

- Set either side to `0` to disable that rule.
- Valid channel range is `1..34`.

## Example

Mirror original roll and pitch into channels `18` and `19`:

```text
set rc_orig_src_1 = 1
set rc_orig_dst_1 = 18
set rc_orig_src_2 = 2
set rc_orig_dst_2 = 19
save
```

With `MSP RC Override` active:

- channel `1` can still be overridden by MSP
- channel `2` can still be overridden by MSP
- channel `18` contains the original value of channel `1`
- channel `19` contains the original value of channel `2`

## Notes

- Destination channels should normally be unused spare channels.
- If a destination channel is also used for flight control, mode switching, or MSP override, the copy rule will overwrite that destination with the original RX value.
- If `src == dst`, that channel is effectively restored to its original pre-override value after MSP override.
- If the source channel is outside the active RX channel count, that rule is ignored.
- `MSP_RC` output is extended up to the highest configured destination channel so channels like `18` and `19` are visible to external consumers.

## Typical workflow

1. Build firmware with `USE_MSP_RC_OVERRIDE`.
2. Configure `MSP RC Override` mode and `msp_override_channels` as usual.
3. Choose spare destination channels for the mirrored originals.
4. Configure the `rc_orig_src_*` and `rc_orig_dst_*` rules.
5. Read the mirrored original values from the destination channels over MSP.

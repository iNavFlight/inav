## VTx Setup

### IRC Tramp

#### Matek 1G3SE Setup

To use the Matek 1G3SE with IRC Tramp. You will need to enter the CLI command `set vtx_frequency_group = FREQUENCYGROUP_1G3`. You must also make sure that the initial VTx settings in the configuration tab are in a valid range. They are: 
- `vtx_band` 1 or 2
- `vtx_channel` between 1 and 8

Note: The frequencies required by the US version of the VTx are on `vtx_band` 2 (BAND B) only.

Power levels are: 
- `1` 25mW
- `2` 200mW
- `3` 800 mW

##### Matek 1G3SE frequency chart

| Band | 1    | 2    | 3    | 4    | 5    | 6    | 7    | 8    |
|------|------|------|------|------|------|------|------|------|
| A    | 1080 | 1120 | 1160 | 1200 | 1240 | 1280 | 1320 | 1360 |
| B    | 1080 | 1120 | 1160 | 1200 | 1258 | 1280 | 1320 | 1360 |

### Team BlackSheep SmartAudio

If you have problems getting SmartAudio working. There are a couple of CLI parameters you can try changing to see if they help.

- There is a workaround for early AKK VTx modules. This is enabled by default. You could try disabling this setting [`vtx_smartaudio_early_akk_workaround`](https://github.com/iNavFlight/inav/blob/master/docs/Settings.md#vtx_smartaudio_early_akk_workaround) to OFF.

- If you are using softserial, you can try using the alternate method by setting [`vtx_smartaudio_alternate_softserial_method`](https://github.com/iNavFlight/inav/blob/master/docs/Settings.md#vtx_smartaudio_alternate_softserial_method) to OFF.

- If you are using TBS Sixty9 VTX you may consider to set count of stop bits to 1, using [`set vtx_smartaudio_stopbits = 1`](https://github.com/iNavFlight/inav/blob/master/docs/Settings.md#vtx_smartaudio_stopbits)

### Custom IRC Tramp power levels

Tramp normally selects a built-in power table from the maximum power reported by
its device. For devices with different levels, configure up to five ascending
power values in milliwatts with `vtx_tramp_power_a` through `vtx_tramp_power_e`.
Use consecutive entries followed by zeros. All zeros retain automatic selection;
an invalid table (gaps, duplicates, descending values) also falls back to automatic
selection. Reboot after changing the table.

For a BLITZ Whoop 2.5W:

```
set vtx_tramp_power_a = 25
set vtx_tramp_power_b = 400
set vtx_tramp_power_c = 1000
set vtx_tramp_power_d = 2500
set vtx_tramp_power_e = 0
set vtx_power = 1
save
```

This changes the power values and their labels, not the VTX's reported maximum.
The driver continues to clamp requests to that maximum. Only if the hardware's
maximum is independently confirmed and the device reports it incorrectly, use
`vtx_max_power_override` to provide the correct maximum. It does not unlock the
VTX or verify actual RF power. Power selection remains one-based: this example
maps levels 1–4 to 25, 400, 1000 and 2500 mW.

The VTX settings parameter-group version changes from 2 to 3. Save `diff all`
before upgrading and restore the VTX settings afterwards; the new table defaults
to automatic selection.

### Tramp pit mode on an AUX switch

Assign **VTX PIT MODE** in Modes (permanent mode ID 69). For example, with the
first two mode-condition slots already in use, this assigns AUX5 / channel 9:

```
aux 2 69 4 1800 2100
save
```

The assigned range requests pit mode while disarmed. Arming exits pit mode and
prevents entering it in flight. The switch is ignored without a valid receiver
signal. Without a mode assignment, existing hardware-button/MSP control is left
alone. Pit mode changes use the Tramp `I` command (0 = enter, 1 = exit), and the
driver retries if subsequent status reports do not match the request. Verify the
VTX's own pit indicator before relying on it; driver support is not a guarantee
that every Tramp-compatible device implements the command.

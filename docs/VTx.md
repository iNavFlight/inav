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

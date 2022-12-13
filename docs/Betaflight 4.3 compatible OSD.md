# Betaflight 4.3 compatible MSP DisplayPort OSD (DJI O3 "Canvas Mode")

INAV 6.0 includes a special mode for MSP DisplayPort that supports incomplete implementations of MSP DisplayPort that only support BetaFlight, like the DJI O3 Air Unit.

Different flight controllers have different OSD symbols and elements and require different fonts. BetaFlight's font is a single page and supports a maximum of 256 glyphs, INAV's font is currently 2 pages and supports up to 512 different glyphs.

While there is some overlap between the glyphs in BetaFlight and INAV, it is not possible to perform a 1 to 1 mapping for all the them. In cases where there is no suitable glyph in the BetaFlight font, a question mark `?` will be displayed.

This mode can be enabled by selecting BF43COMPAT as video format in the OSD tab of the configurator or by typing the following command on the CLI:

`set osd_video_system = BF43COMPAT`

## Limitations

* Canvas size is limited to PAL's canvas size.
* Unsupported Glyphs show up as `?`

## FAQ

### I see a lot of `?` on my OSD.

That is expected, when your INAV OSD widgets use glyphs that don't have a suitable mapping in BetaFlight's font.

### Does it work with the G2 and Original Air Unit/Vista?

Yes.

### Is this a replacement for WTFOS?

Not exactly. WTFOS is a full implementation of MSP-Displayport for rooted Air Unit/Vista/Googles V2 and actually works much better than BetaFlight compatibility mode, being able to display all INAV's glyphs.

### Can INAV fix DJI's product?

No. OSD renderinng happens on the googles/air unit side of things. Please ask DJI to fix their incomplete MSP DisplayPort implemenation. You can probably request it in [DJI's forum](https://forum.dji.com/forum.php?mod=forumdisplay&fid=129&filter=typeid&typeid=767).

### BetaFlight X.Y now has more symbols, can you update INAV?

Maybe. If a future version of BetaFlight includes more Glyphs that can be mapped into INAV it is fairly simple to add the mapping, but the problem with DJI's implemenation persists. Even if we update the mapping, if DJI does not update the fonts on their side the problem will persist.

### Can you replace glyph `X` with text `x description`?

While it might technically be possible to replace some glyphs with text in multiple cells, it will introduce a lot of complexity in the OSD rendering and configuration for something we hope is a temporary workaround.

### Does DJI support Canvas Mode?

Actually, no. What DJI calls Canvas Mode is actually MSP DisplayPort and is a character based OSD.
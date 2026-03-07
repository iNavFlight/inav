# DJI compatible MSP DisplayPort OSD (DJI O3 "Canvas Mode")

INAV 6.0 includes a special mode for MSP DisplayPort that supports DJI's incomplete implementations of MSP DisplayPort. This can be found on products like the DJI O3 Air Unit. INAV 6.1 expands this to include HD canvas sizes from BetaFlight 4.4.

Different flight controller firmware have different OSD symbols and elements and require different fonts. BetaFlight's font is a single page and supports a maximum of 256 glyphs, INAV's font is currently 2 pages and supports up to 512 different glyphs. DJI's font is single page and based, but not the same as, BetaFlight's font.

While there is some overlap between the glyphs in DJI and INAV, it is not possible to perform a 1 to 1 mapping for all the them. In cases where there is no suitable glyph in the DJI font, a question mark `?` will be displayed.

This mode can be enabled by selecting DJI43COMPAT or DJIHDCOMPAT as video format in the OSD tab of the configurator or by typing the following command on the CLI:

`set osd_video_system = DJI43COMPAT`

or

`set osd_video_system = DJIHDCOMPAT`

## Limitations

* Canvas size needs to be manually changed to HD on the Display menu in DJI's goggles (you may need a firmware update) and set as DJIHDCOMPAT in the OSD tab of the configurator.
* Unsupported Glyphs show up as `?`

## FAQ

### I see a lot of `?` on my OSD.

That is expected. When your INAV OSD widgets use glyphs that don't have a suitable mapping in DJI's font.

### Does it work with the G2 and Original Air Unit/Vista?

Yes.

### Is this a replacement for WTFOS?

Not exactly. WTFOS is a full implementation of MSP-Displayport for rooted Air Unit/Vista/Googles V2 and actually works much better than DJI compatibility mode. It can use all of INAV's OSD elements as intended. If you have the option of WTFOS or DJI compatability mode. WTFOS is the best option.

### Can INAV fix DJI's product?

No. OSD renderinng happens on the googles/air unit side of things. Please ask DJI to fix their incomplete MSP DisplayPort implemenation. You can probably request it in [DJI's forum](https://forum.dji.com/forum.php?mod=forumdisplay&fid=129&filter=typeid&typeid=767). To see what you're missing out on with O3. Check out what WTFOS did with the original system. Not only could the pilots upload the fonts of their choosing (who doesn't want a cool SneakyFPV font on their OSD). But there were no problems supporting and firmware. Plus, there was even an option to save the OSD to a file and overlay that over your DVR video. If you're reading this far. Please recommend to DJI that they fix their product, to at least what was possible with WTFOS.

### DJI's font now has more symbols, can you update INAV?

Maybe. If a future version of DJI's font includes more Glyphs that can be mapped into INAV. It is fairly simple to add the mapping. However, the best solution would be full support of MSP DisplayPort by DJI. Then there will never be an issue with missing icons. As the latest INAV font would be able to be uploaded on to the goggles.

### Can you replace glyph `X` with text `x description`?

While it might technically be possible to replace some glyphs with text in multiple cells, it will introduce a lot of complexity in the OSD rendering and configuration for something we hope is a temporary workaround.

### Does DJI support Canvas Mode?

Actually, no. What DJI calls Canvas Mode is actually MSP DisplayPort and is a character based OSD. Currently, the only true implementaion of Canvas Mode is with FrSKY PixelOSD. This was found on some F722 flight controllers from Matek.

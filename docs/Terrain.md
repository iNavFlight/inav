# Terrain
```
┌─────────────────────────────────────────────────┐
│  ===Experimental===                             │
│  This feature is experimental. Use it with      │
│  caution.                                       │
└─────────────────────────────────────────────────┘
```

This feature is available **only on H7-based and F4-based flight controllers with an SD card** (preferably SDIO). Due to the high CPU load
and SD card read speed requirements of terrain data processing, it is not supported on weaker hardware.

This feature in iNav determines the model’s altitude above ground level using preloaded elevation maps
(terrain / SRTM data) stored on an SD card, without requiring a physical rangefinder. Based on the current GPS position, the
flight controller identifies the corresponding point in the terrain map, calculates the ground elevation, and derives the
altitude above terrain.

In this first implementation, the calculated value is used **only for informational display in the OSD**. It is indicative
only, does not yet behave as a true virtual rangefinder, and is not used for navigation or automatic altitude control. If the
terrain data are unavailable or a read error occurs, the feature is automatically disabled.

# SD Card Preparation

For proper operation, the SD card must be prepared in advance. It is recommended to create a **partition with a maximum size
of 4 GB** and format it. Formatting should be done using the official 
[**SD Memory Card Formatter**](https://www.sdcard.org/downloads/formatter/sd-memory-card-formatter-for-windows-download/) tool from the SD Association,
which ensures correct alignment and a compatible file system structure. Using this tool minimizes the risk of terrain data
read issues during flight and is considered the recommended method for preparing an SD card for iNav.

# Data Generation and Copying

To generate elevation maps, use the terrain generator web tool available at https://terrain.ardupilot.org/

In iNav, **only 30 m resolution (SRTM1)** is currently supported, so this option must be selected during data generation.
The generated files are then copied to the SD card into the appropriate directory structure.

Copying can be done via **iNav MSC (Mass Storage Class)**, but this method is very slow, so using an **external SD card reader**
is strongly recommended. Before copying the data, the file **`FREESPAC.E`** must be deleted from the root directory of the SD
card. iNav uses this file to track available disk space, and without deleting it, the card may appear to be full. After the
copying process is complete, iNav will automatically recreate this file on the next startup.

# Enabling and Displaying Terrain Data

To display altitude above terrain in iNav, the OSD element **“Rangefinder distance”** must be enabled. If terrain data are
available on the SD card and no valid data are available from a dedicated rangefinder, the value calculated by the terrain
system will be displayed. If a rangefinder is present and providing valid data, its measurements always take priority and the
actual distance to the ground will be shown.

Loading terrain data from the SD card is enabled via the CLI using the following command:

```text
set terrain_enabled = ON
save
```

After restarting the flight controller, iNav will automatically start loading terrain data and, when conditions are met, use
them to display altitude above terrain.

Finally, it is **strongly recommended to use only high-quality, branded SD cards** from reputable manufacturers. The terrain
system is sensitive to SD card read speed and reliability, and low-quality or counterfeit cards may cause read errors,
display dropouts, or automatic disabling of the feature during flight. Using a quality SD card significantly improves the
stability and reliability of the terrain feature.

# TL;DR

- **H7/F4 flight controller** with SD card required
- Format SD card using [SD Memory Card Formatter](https://www.sdcard.org/downloads/formatter/sd-memory-card-formatter-for-windows-download/) (max 4 GB partition)
- Generate terrain data at https://terrain.ardupilot.org/ — select **SRTM1 (30 m)**
- Delete `FREESPAC.E` from SD card root before copying files (use external card reader, not MSC)
- Enable via CLI: `set terrain_enabled = ON` + `save`
- Enable **Rangefinder distance** OSD element to see altitude above terrain
- ⚠️ Displayed value is **informational only** — not used for navigation or altitude control
- Use only **high-quality branded SD cards**
# Terrain
```
┌─────────────────────────────────────────────────┐
│  ===Experimental===                             │
│  This feature is experimental. Use it with      │
│  caution.                                       │
└─────────────────────────────────────────────────┘
```

This feature is available **only on H7-based and F4-based flight controllers with an SD card**.

This feature in iNav determines the model’s altitude above ground level using preloaded elevation maps
(terrain / SRTM data) stored on an SD card, without requiring a physical rangefinder. Based on the current GPS position, the
flight controller identifies the corresponding point in the terrain map, calculates the ground elevation, and derives the
altitude above terrain.

In this first implementation, the calculated value is used **only for informational display in the OSD**. It is indicative
only, does not yet behave as a true virtual rangefinder, and is not used for navigation or automatic altitude control. If the
terrain data are unavailable or a read error occurs, the feature is automatically disabled.

# SD Card Preparation

Use only quality SD cards from reputable brands. Note that some combinations of flight controllers and SD cards can cause issues, 
so if you experience any problems, try a different card before troubleshooting further. Compatibility problems have been 
observed especially with F4-based flight controllers.

For proper operation, the SD card must be prepared in advance. It is recommended to create a **partition with a maximum size
of 4 GB** and format it to FAT32. 

# Data Generation and Copying

To generate elevation maps, use the terrain generator web tool available at https://martinovem.github.io/High-Resolution-Map-Generator/ or https://terrain.ardupilot.org/

**Before copying any terrain data files, the SD card must be formatted. Always format the card before each new terrain data
installation to avoid file system errors.**

In iNav, **only 30 m resolution (SRTM1)** is currently supported, so this option must be selected during data generation.
The generated files are then copied to the SD card into the root directory structure.

For example
```
SDCARD:\
├── N47E014.DAT
├── N47E015.DAT
├── N47E016.DAT
├── N49E015.DAT
├── N49E016.DAT
├── N49E017.DAT
└── N50E016.DAT
```

Copying can be done via **iNav MSC (Mass Storage Class)** is not recommended.

> **Important:** The `.DAT` file covering your **home position** (the location where you arm) must always be present on the SD
> card. The terrain system first reads the ground elevation at the home position to establish an altitude reference. Until this
> succeeds, terrain data will not be displayed for any position — even if tiles for your entire flight area are present. Make
> sure the tile for your take-off site is included when generating and copying terrain files.

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

# Troubleshooting
- OSD shows no terrain value / "Rangefinder distance" is dash:
  - Verify `terrain_enabled = ON` is set and saved (`set terrain_enabled = ON` → `save` → reboot)
  - Confirm the SD card is mounted: check `status` in the CLI for SD card state
  - Confirm a valid GPS fix is present before expecting any terrain value to appear
  - Terrain data is only loaded **after the first valid GPS fix**. If the fix is acquired after a long wait, allow a few seconds for the first read to complete
  - Check that the `.DAT` file covering your **home position** exists on the SD card — see note in *Data Generation and Copying* above
- Feature disables itself mid-flight:
  - The terrain subsystem disables itself on any SD card read failure. This is most commonly caused by a slow or low-quality SD card. Replace the card with a branded, high-speed card
  - On F4-based flight controllers, SD card compatibility issues are more common. If problems persist, test with a different card model or switch to an H7-based flight controller
  - Ensure the SD card partition is ≤ 4 GB and formatted FAT32. Larger partitions or exFAT formatting can cause intermittent read failures
- Value appears but is clearly wrong:
  - Confirm you selected **30 m resolution (SRTM1)** when generating data at https://terrain.ardupilot.org/. Other resolutions are not supported and will produce incorrect values
  - Reformat the SD card and recopy the terrain files. A corrupted or partially overwritten FAT32 filesystem can produce plausible but incorrect altitude values
  - The displayed value is altitude **above terrain at the current GPS position**, not distance to the nearest object below the aircraft. Trees, buildings, and local obstacles are not accounted for
- Terrain data was not loaded before arming:
  - If the flight controller is armed before terrain data for the home position is successfully read from the SD card, the system will not attempt to load data during the flight (to avoid SD card access latency while airborne). Disarm, wait for the OSD value to appear, then arm

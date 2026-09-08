# In-Flight OSD Menu

The In-Flight OSD Menu allows pilots to adjust flight parameters directly through the OSD while the aircraft is airborne and under autonomous navigation control. This eliminates the need to land and disarm for common tuning tasks such as PID adjustments, rate changes, VTX power/channel changes, and battery threshold corrections.

> **Warning:** Modifying parameters during flight carries inherent risk. Only make adjustments while in a stable cruise/loiter phase with plenty of altitude and airspace. Changes to PIDs or rates can affect flight behaviour immediately.

## Overview

The feature works by assigning a dedicated **IN FLIGHT MENU** flight mode switch in the Configurator. When the switch is activated and held for 3 seconds while the aircraft is armed and a navigation mode (Position Hold, Cruise, Loiter, etc.) is engaged, the OSD menu opens and stick inputs are redirected from flight control to menu navigation.


## Requirements

- Aircraft must be **armed**
- A **navigation mode** must be active (e.g. POSHOLD, NAV CRUISE, NAV WP, RTH)
- **Failsafe** must not be active
- The **IN FLIGHT MENU** RC mode switch must be assigned and held active

If any of these conditions are lost while the menu is open, the menu closes automatically and full stick control is restored immediately.

## Setup

### 1. Assign the IN FLIGHT MENU mode in the Configurator

1. Open the INAV Configurator and go to the **Modes** tab.
2. In the **OSD Modes** section, find **IN FLIGHT MENU**.
3. Assign it to a dedicated **latching (two-position) switch** on an AUX channel.

> **Important:** A **latching switch** is required. The menu stays open only while the switch is held in the active position. Toggling the switch off closes the menu immediately and restores full stick control.

### 2. Assign a navigation mode

At least one navigation mode (POSHOLD, NAV CRUISE, etc.) must be configured and active before the in-flight menu can open.

## Opening the Menu

1. Fly to a stable cruise altitude with a navigation mode active.
2. **Flip the `IN FLIGHT MENU` switch to the active position**.
3. The OSD will display a `MENU IN X.X` countdown notification while waiting to open.
4. After 3 seconds, the menu appears on the OSD.

> **Note:** The 3-second hold delay is intentional — it prevents accidental menu activation from brief switch contact.

## Closing the Menu

The menu can be closed in several ways:

| Method                        | Description                                               |
|-------------------------------|-----------------------------------------------------------|
| Toggle the switch off         | Flip the `IN FLIGHT MENU` switch back to the inactive position — closes immediately |
| Select EXIT                   | Navigate to the EXIT entry in the main menu               |
| 15-second inactivity timeout  | Menu closes automatically after 15 s with no input        |
| Safety auto-close             | Menu closes if any safety condition is lost (see below)   |

## Safety Auto-Close

The menu will close immediately and restore full stick control if any of the following occur:

- The `IN FLIGHT MENU` switch is toggled off
- The aircraft disarms
- Failsafe is triggered
- The active navigation mode is lost
- **Panic stick movement detected**: simultaneous Roll and Pitch deflection greater than 100 PWM from centre, sustained for approximately 150 ms (3 consecutive samples at 50 ms intervals). This is designed to detect a pilot grabbing the sticks instinctively during an emergency without triggering on normal single-axis menu navigation inputs.

## Important Behaviour Notes

- **Changes are not saved to EEPROM during flight.** All adjustments are applied to RAM only. To make changes permanent, land, disarm, and save from the main menu of the ground CMS.
- **Battery thresholds** are applied live so the OSD low-battery warnings reflect the new values immediately without a reboot.


## OSD Status Messages

While the switch is held active but the menu has not yet opened, the OSD shows a countdown:

```
MENU IN 3.0
MENU IN 2.5
...
MENU IN 0.0
```

If a navigation mode is not active when the switch is held, the OSD shows:

```
USE NAV MODE FOR MENU
```



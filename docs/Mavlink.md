# MAVLink INAV Implementation

INAV has a partial implementation of MAVLink that is intended primarily for simple telemetry and operation. It supports RC, missions, telemetry and some features such as Guided mode; but it is very different from a compliant MAVLink spec vehicle such as Pixhawk or Ardupilot and important differences exist, as such it is not 100% compatible and cannot be expected to work the same way. The standard MAVLink header library is used in compilation.

## Fundamental differences from ArduPilot/PX4

- **No MAVLink parameter API**: INAV sends a single stub parameter and otherwise ignores parameter traffic. Configure the aircraft through the INAV Configurator or CLI instead.
- **Limited command support**: only a subset of commands is implemented; unsupported commands are `COMMAND_ACK`ed as `UNSUPPORTED`.
- **Mission handling**: uploads are rejected while armed (except legacy guided waypoint writes). Mission frames are validated per command and unsupported frames are rejected.
- **Mode reporting**: `custom_mode` approximates ArduPilot modes and may not match all INAV states. 
- **Flow control expectations**: INAV honours `RADIO_STATUS.txbuf` to avoid overrunning radios; without it, packets are simply paced at 20 ms intervals.
- **Half‑duplex etiquette**: when half‑duplex is enabled, INAV waits one telemetry tick after any received frame before transmitting to reduce collisions.

### Relevant CLI options

- `mavlink_sysid` – system ID used in every outbound packet (default 1); most inbound handlers only act on packets targeted to this system ID.
- `mavlink_autopilot_type` – heartbeat autopilot ID (`GENERIC` or `ARDUPILOT`).
- `mavlink_version` – force MAVLink v1 when set to 1.
- Stream rates (Hz): `mavlink_ext_status_rate`, `mavlink_rc_chan_rate`, `mavlink_pos_rate`, `mavlink_extra1_rate`, `mavlink_extra2_rate`, `mavlink_extra3_rate`. Each group is polled up to 50 Hz; a rate of 0 disables the group.
- `mavlink_min_txbuffer` – minimum remote TX buffer level before sending when `RADIO_STATUS` provides flow control.
- `mavlink_radio_type` – scales `RADIO_STATUS` RSSI/SNR for **generic**, **ELRS**, or **SiK** links.

## Supported Outgoing Messages

Messages are organized into MAVLink datastream groups. Each group sends **one message per trigger** at the configured rate:

- `SYS_STATUS`: Advertises detected sensors (gyro/accel/compass, baro, pitot, GPS, optical flow, rangefinder, RC, blackbox) and whether they are healthy. Includes main loop load, battery voltage/current/percentage, and logging capability.
- `RC_CHANNELS_RAW`: for v1, `RC_CHANNELS`: for v2. Up to 18 input channels plus RSSI mapped to MAVLink units.
- `GPS_RAW_INT`: for GNSS fix quality, HDOP/VDOP, velocity, and satellite count when a fix (or estimated fix) exists.
- `GLOBAL_POSITION_INT`: couples GPS position with INAV's altitude and velocity estimates.
- `GPS_GLOBAL_ORIGIN`: broadcasts the current home position.
- `ATTITUDE`: Roll, pitch, yaw, and angular rates.
- `VFR_HUD`: with airspeed (if a healthy pitot is available), ground speed, throttle, altitude, and climb rate.
- `HEARTBEAT`: encodes arming state and maps INAV flight modes onto ArduPilot-style `custom_mode`: values (see mappings below).
- `EXTENDED_SYS_STATE`: publishes landed state.
- `BATTERY_STATUS`: with per-cell voltages (cells 11‑14 in `voltages_ext`), current draw, consumed mAh/Wh, and remaining percentage when available.
- `SCALED_PRESSURE`: for IMU/baro temperature.
- `STATUSTEXT`: when the OSD has a pending system message; severity follows OSD attributes (notice/warning/critical).
- On-demand (command-triggered) messages: `AUTOPILOT_VERSION`, `PROTOCOL_VERSION`, `MESSAGE_INTERVAL`, `HOME_POSITION`, `AVAILABLE_MODES`, and `CURRENT_MODE`.

## Supported Incoming Messages

- `HEARTBEAT`: used to detect ADS‑B participants when `type` is `MAV_TYPE_ADSB`.
- `MISSION_COUNT`: starts an upload transaction (capped at `NAV_MAX_WAYPOINTS`).
- `MISSION_ITEM` / `MISSION_ITEM_INT`: stores mission waypoints; rejects unsupported frames/sequence errors. Upload while armed is rejected except legacy guided waypoint writes.
- `MISSION_REQUEST_LIST`, `MISSION_REQUEST`, `MISSION_REQUEST_INT`: downloads active mission items; returns `MISSION_ACK` on bad sequence.
- `MISSION_CLEAR_ALL`: clears stored mission.
- `COMMAND_LONG` / `COMMAND_INT`: command transport for supported `MAV_CMD_*` handlers.
- `REQUEST_DATA_STREAM`: legacy stream-rate control per stream group.
- `SET_POSITION_TARGET_GLOBAL_INT`: writes the GCS-guided waypoint when the frame is supported.
- `RC_CHANNELS_OVERRIDE` passes channel values to the MAVLink serial receiver backend.
- `RADIO_STATUS` updates remote TX buffer level and scales RSSI/SNR according to `mavlink_radio_type` (also feeds link stats for MAVLink RX receivers).
- `ADSB_VEHICLE` populates the internal traffic list when ADS‑B is enabled.
- `PARAM_REQUEST_LIST` elicits a stub `PARAM_VALUE` response so ground stations stop requesting parameters (INAV does not expose parameters over MAVLink).


## Supported Commands

Limited implementation of the Command protocol.

- `MAV_CMD_DO_REPOSITION`: sets the Follow Me/GCS Nav waypoint when GCS NAV is valid. Accepts `MAV_FRAME_GLOBAL`, `MAV_FRAME_GLOBAL_INT`, `MAV_FRAME_GLOBAL_RELATIVE_ALT`, `MAV_FRAME_GLOBAL_RELATIVE_ALT_INT`; otherwise `UNSUPPORTED`.
- `MAV_CMD_SET_MESSAGE_INTERVAL` / `MAV_CMD_GET_MESSAGE_INTERVAL`: adjust or query telemetry stream output for supported message IDs (streamed messages only; intervals slower than 1 Hz are not accepted).
- `MAV_CMD_GET_HOME_POSITION`: replies with `HOME_POSITION` when home fix exists.
- `MAV_CMD_REQUEST_MESSAGE`: emits one selected message (`HEARTBEAT`, `SYS_STATUS`, `ATTITUDE`, `VFR_HUD`, `AVAILABLE_MODES`, `CURRENT_MODE`, `EXTENDED_SYS_STATE`, RC channels, GPS/global/origin, battery/pressure, and `HOME_POSITION` when available) or `UNSUPPORTED`.
- `MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES`: returns stub `AUTOPILOT_VERSION` (v2 only; v1 returns `UNSUPPORTED`).
- `MAV_CMD_REQUEST_PROTOCOL_VERSION`: returns stub `PROTOCOL_VERSION` (v2 only; v1 returns `UNSUPPORTED`).

## Mode mappings (INAV → MAVLink/ArduPilot)

`custom_mode` is derived from active INAV telemetry flight mode (`getFlightModeForTelemetry()`), then mapped per vehicle type.

- **Multirotor profiles**
  - ACRO / ACRO AIR → **ACRO**
  - ANGLE / HORIZON / ANGLE HOLD → **STABILIZE**
  - ALT HOLD → **ALT_HOLD**
  - POS HOLD → **GUIDED** (if GCS valid), otherwise **POSHOLD**
  - RTH → **RTL**
  - MISSION → **AUTO**
  - LAUNCH → **THROW**
  - FAILSAFE → **RTL** (RTH/other phases) or **LAND** (landing phase)
  - Any other unmapped mode falls back to **STABILIZE**
- **Fixed-wing profiles**
  - MANUAL → **MANUAL**
  - ACRO / ACRO AIR → **ACRO**
  - ANGLE → **FBWA**
  - HORIZON / ANGLE HOLD → **STABILIZE**
  - ALT HOLD → **FBWB**
  - POS HOLD → **GUIDED** (if GCS valid), otherwise **LOITER**
  - RTH → **RTL**
  - MISSION → **AUTO**
  - CRUISE → **CRUISE**
  - LAUNCH → **TAKEOFF**
  - FAILSAFE → **RTL** (RTH/other phases) or **AUTOLAND** (landing phase)
  - Any other unmapped mode falls back to **MANUAL**

## Datastream groups and defaults

Default rates (Hz) are shown; adjust with the CLI keys above.

| Datastream group | Messages | Default rate |
| --- | --- | --- |
| `EXTENDED_STATUS` | `SYS_STATUS` | 2 Hz |
| `RC_CHANNELS` | `RC_CHANNELS_RAW` (v1) / `RC_CHANNELS` (v2) | 1 Hz |
| `POSITION` | `GPS_RAW_INT`, `GLOBAL_POSITION_INT`, `GPS_GLOBAL_ORIGIN` | 2 Hz |
| `EXTRA1` | `ATTITUDE` | 3 Hz |
| `EXTRA2` | `VFR_HUD`, `HEARTBEAT` | 2 Hz |
| `EXT_SYS_STATE` | `EXTENDED_SYS_STATE` | 1 Hz (defaults to `mavlink_extra3_rate`) |
| `EXTRA3` | `BATTERY_STATUS`, `SCALED_PRESSURE`, `STATUSTEXT` (when present) | 1 Hz |

## Operating tips

- Set `mavlink_radio_type` to **ELRS** or **SiK** if you use those links to get accurate link quality scaling in `RADIO_STATUS`.
- If you rely on RC override via MAVLink, ensure the serial receiver type is set to `SERIALRX_MAVLINK` and consider enabling `telemetry_halfduplex` when RX shares the port.
- To reduce bandwidth, lower the stream rates for groups you do not need, or disable them entirely by setting the rate to 0.


## MAVLink Missions

Partial compatibility with MAVLink mission planners such as QGC is implemented, however the differences between the two protocols means INAV cannot be programmed to it's full potential using only the MAVLINK mission protocol; only simple missions are possible. It is recommended to use MultiWii Planner or the INAV Configurator to program missions.

## MSP mission parity gaps (MAV ↔ MSP)

- WAYPOINT: MSP→MAV sends lat/lon/alt but drops leg speed `p1` and all user-action bits in `p3` (only alt-mode bit drives frame). MAV→MSP stores lat/lon/alt but forces `p1=0`, `p2=0`, keeps only alt-mode bit in `p3`; leg speed and user bits are lost.
- POSHOLD_TIME / LOITER_TIME: loiter time `p1` OK; leg speed `p2` and user-action bits in `p3` are discarded both directions.
- LAND: lat/lon/alt OK; leg speed `p1`, ground elevation `p2`, and user-action bits in `p3` are cleared in both directions (only alt-mode bit is retained from frame on upload).
- RTH: RTH land-if-nonzero flag in `p1` is ignored both ways (always zeroed); user-action bits dropped; alt is sent only if the MAVLink frame is coordinate and returns with alt-mode bit set on upload.
- JUMP: target and repeat count OK
- SET_POI: lat/lon/alt OK; `param1` is fixed to `MAV_ROI_LOCATION`; user-action bits in `p3` are dropped (alt-mode bit respected on upload).
- SET_HEAD: heading `p1` OK; user-action bits in `p3` are not represented.
- Net effect: actions and positions OK, but MSP-specific fields (leg speed, LAND elevation adjustment, RTH land flag, user-action bits in `p3`) are lost, so MAVLink missions cannot fully conform to `MSP_navigation_messages.md`.

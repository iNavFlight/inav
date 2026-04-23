# MAVLink INAV Implementation

INAV on the `mavlink_multiport2` branch has a selective but broad MAVLink implementation. It is still not a drop-in MAVLink autopilot stack like ArduPilot or PX4, but it now covers much more than simple telemetry: multiport routing, mission upload/download, waypoint reached notifications, guided/navigation control, request-response services, RC override for MAVLink serial RX, high-latency mode, MSP-over-MAVLink tunnel, and native MLRS receiver integration.

INAV supports up to 4 concurrent MAVLink telemetry ports (`MAX_MAVLINK_PORTS`), one endpoint per serial port configured with `FUNCTION_TELEMETRY_MAVLINK`.

This branch builds against the checked-in generated `storm32` MAVLink headers/dialect bundle, which includes the native MLRS messages used by the current implementation.

## What INAV currently supports

- Multiport MAVLink telemetry on up to 4 serial ports, with per-port stream rates, radio type, and high-latency mode.
- Learned route discovery and forwarding between MAVLink ports based on `(sysid, compid) -> ingress port`.
- Mission upload, download, clear, and `MISSION_ITEM_REACHED` notifications.
- Guided / GCS-nav control via `MAV_CMD_DO_REPOSITION`, altitude changes, yaw changes, and selected `SET_POSITION_TARGET_*` messages.
- Request-response services including `REQUEST_DATA_STREAM`, `MAV_CMD_SET_MESSAGE_INTERVAL`, `MAV_CMD_GET_MESSAGE_INTERVAL`, `MAV_CMD_REQUEST_MESSAGE`, `MAV_CMD_GET_HOME_POSITION`, `AUTOPILOT_VERSION`, `PROTOCOL_VERSION`, `AVAILABLE_MODES`, and `CURRENT_MODE`.
- `RC_CHANNELS_OVERRIDE` for MAVLink serial RX setups.
- Per-port `HIGH_LATENCY2` mode.
- MSP-over-MAVLink tunnel using `TUNNEL` payload type `0x8001`.
- Native MLRS receiver integration using `MLRS_RADIO_LINK_STATS`, `MLRS_RADIO_LINK_INFORMATION`, and `MLRS_RADIO_LINK_FLOW_CONTROL`.

## Important differences and omissions

- **No MAVLink parameter API**: INAV sends a single stub parameter and otherwise ignores parameter traffic. Configure the aircraft through the INAV Configurator or CLI instead.
- **Selective command support**: INAV implements a useful subset of MAVLink commands and ACKs unsupported commands as `UNSUPPORTED`.
- **Mission handling is partial**: uploads are rejected while armed except for legacy guided waypoint writes, mission frames are validated per command, and MSP mission parity gaps remain.
- **Mode reporting is approximate**: `custom_mode` is mapped to ArduPilot-style modes for compatibility and does not represent every INAV state exactly.
- **Single local component identity**: INAV always originates as `MAV_COMP_ID_AUTOPILOT1`; attached radios, GCSes, and companions are always remote components, never local per-port FC identities.
- **Flow control is per-port and opportunistic**: INAV uses remote TX buffer information from `RADIO_STATUS.txbuf`, or from `MLRS_RADIO_LINK_FLOW_CONTROL.txbuf` on MLRS links. Without flow-control input it falls back to blind 20 ms pacing.
- **Half-duplex etiquette still applies**: on a MAVLink serial RX port configured for `serialrx_halfduplex`, INAV waits one telemetry tick after a received frame before transmitting.

### Usage guidance

- If you rely on RC via MAVLink, set the serial receiver type to `SERIALRX_MAVLINK`.
- If MAVLink RX and telemetry intentionally share one half-duplex wire, enable `serialrx_halfduplex` for that setup.
- Leave `mavlink_version = 2` unless you intentionally need MAVLink1 compatibility. Native MLRS messages, MSP tunnel, high-latency mode, `AUTOPILOT_VERSION`, and `PROTOCOL_VERSION` all depend on MAVLink2 behavior in this branch.
- To reduce bandwidth, lower stream rates for groups you do not need, or disable them entirely by setting the rate to 0.
- If a GCS or companion needs telemetry on ports 2..4, explicitly request streams (`REQUEST_DATA_STREAM` or `MAV_CMD_SET_MESSAGE_INTERVAL`) because only heartbeat is enabled by default.
- If you depend on directed forwarding between links, ensure each remote endpoint transmits at least one frame early so route learning is populated.

### Relevant CLI settings

- `mavlink_sysid` - system ID used in every outbound packet (default 1); most inbound handlers only act on packets targeted to this system ID.
- `mavlink_autopilot_type` - heartbeat autopilot ID (`GENERIC` or `ARDUPILOT`).
- `mavlink_version` - MAVLink version to use (`2` by default, `1` only when forced for compatibility).
- Stream rates (Hz): each group is polled up to 50 Hz; a rate of 0 disables the group.
  - `mavlink_port{1-4}_ext_status_rate`
  - `mavlink_port{1-4}_rc_chan_rate`
  - `mavlink_port{1-4}_pos_rate`
  - `mavlink_port{1-4}_extra1_rate`
  - `mavlink_port{1-4}_extra2_rate`
  - `mavlink_port{1-4}_extra3_rate`
- Port 1 uses configured CLI rates (`mavlink_port1_*_rate`).
- Ports 2..4 start with heartbeat only (1 Hz), all other streams disabled.
- `mavlink_port{1-4}_min_txbuffer` - minimum remote TX buffer level before sending when per-port flow-control information is available.
- `mavlink_port{1-4}_radio_type` - selects `GENERIC`, `ELRS`, `SIK`, or `MLRS`. `GENERIC` / `ELRS` / `SIK` use `RADIO_STATUS` interpretation; `MLRS` uses native `MLRS_RADIO_LINK_*` traffic on the RX-sharing MAVLink port.
- `mavlink_port{1-4}_high_latency` - turns on MAVLink `HIGH_LATENCY2` mode on that port.

## Local identity, compatibility, and advertised capabilities

- INAV always transmits as MAVLink component `MAV_COMP_ID_AUTOPILOT1`.
- The FC does not create local per-port MAVLink identities. Attached devices are learned as remote components from incoming traffic.
- `AUTOPILOT_VERSION` advertises ArduPilot-compatible flight software version `4.7.0`.
- When MAVLink2 is active, `AUTOPILOT_VERSION` advertises these protocol capabilities:
  - `MAV_PROTOCOL_CAPABILITY_MAVLINK2`
  - `MAV_PROTOCOL_CAPABILITY_MISSION_FLOAT`
  - `MAV_PROTOCOL_CAPABILITY_MISSION_INT`
  - `MAV_PROTOCOL_CAPABILITY_COMMAND_INT`
  - `MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_LOCAL_NED`
  - `MAV_PROTOCOL_CAPABILITY_SET_POSITION_TARGET_GLOBAL_INT`

## Datastream groups and defaults

Default rates (Hz) are shown; adjust with the CLI keys above for port 1.
Ports 2..N use a secondary startup profile (heartbeat at 1 Hz, other streams disabled).

| Datastream group | Messages | Default rate |
| --- | --- | --- |
| `EXTENDED_STATUS` | `SYS_STATUS` | 2 Hz |
| `RC_CHANNELS` | `RC_CHANNELS_RAW` (v1) / `RC_CHANNELS` (v2) | 1 Hz |
| `POSITION` | `GPS_RAW_INT`, `GLOBAL_POSITION_INT`, `GPS_GLOBAL_ORIGIN` | 2 Hz |
| `EXTRA1` | `ATTITUDE` | 2 Hz |
| `EXTRA2` | `VFR_HUD` | 2 Hz |
| `HEARTBEAT` | `HEARTBEAT` | 1 Hz (independent of stream groups) |
| `EXT_SYS_STATE` | `EXTENDED_SYS_STATE` | 1 Hz (defaults to `mavlink_port1_extra3_rate`) |
| `EXTRA3` | `BATTERY_STATUS`, `SCALED_PRESSURE`, `SYSTEM_TIME`, `STATUSTEXT` (when present) | 1 Hz |

### Routing, forwarding, and local handling

- INAV learns routes from incoming traffic as `(sysid, compid) -> ingress port`.
- Broadcast messages are forwarded to all other MAVLink ports except `RADIO_STATUS` and `MLRS_RADIO_LINK_FLOW_CONTROL`, which stay local to the ingress port.
- Targeted messages are forwarded only to ports with a learned route for that target.
- Practical caveat: the first targeted message to a never-seen endpoint may not forward until that endpoint has sent at least one MAVLink frame.
- INAV's local FC identity is always `(mavlink_sysid, MAV_COMP_ID_AUTOPILOT1)`.
- Traffic from the local system ID but a different component ID is treated as a remote component and can be learned into the route table.
- Local/system broadcasts (`target_system = 0` or local system ID with `target_component = 0`) are fanned out to all local ports only for:
  - `REQUEST_DATA_STREAM`
  - `MAV_CMD_SET_MESSAGE_INTERVAL`
  - `MAV_CMD_CONTROL_HIGH_LATENCY`
- Other incoming commands/messages are handled once on the resolved local ingress path, but broadcast-targeted control requests still execute locally.

## Native MLRS receiver integration

When `mavlink_port{1-4}_radio_type = MLRS`, INAV uses native receiver-emitted MLRS messages rather than treating `RADIO_STATUS` as the real MLRS data source.

- INAV expects `MLRS_RADIO_LINK_STATS`, `MLRS_RADIO_LINK_INFORMATION`, and `MLRS_RADIO_LINK_FLOW_CONTROL`.
- These messages are accepted only from `MAV_COMP_ID_TELEMETRY_RADIO`.
- `MLRS_RADIO_LINK_STATS` and `MLRS_RADIO_LINK_INFORMATION` must target the local system. `target_component` may be `0` or `MAV_COMP_ID_AUTOPILOT1`.
- INAV stores MLRS runtime state per ingress MAVLink port and clears that state with the normal port lifecycle.
- Only the ingress port that is also the active MAVLink serial RX port may update the global `rxLinkStatistics`.
- `MLRS_RADIO_LINK_STATS` feeds:
  - RC link quality into `uplinkLQ`
  - serial link quality into `downlinkLQ`
  - RSSI, SNR, and active antenna into the receiver stats model
- `MLRS_RADIO_LINK_INFORMATION` feeds:
  - mode and band strings
  - TX/RX power
  - receive sensitivity metadata
- `MLRS_RADIO_LINK_FLOW_CONTROL` applies only to the ingress port's pacing / TX-buffer runtime state and is not forwarded to other MAVLink ports.
- `RADIO_STATUS` remains the generic / legacy radio path for non-MLRS links (`GENERIC`, `ELRS`, `SIK`) and should not be treated as the real MLRS integration path.

## Supported outgoing messages

Messages are organized into MAVLink datastream groups. Each group sends one message per trigger at the configured rate.

- `SYS_STATUS`: advertises detected sensors (gyro/accel/compass, baro, pitot, GPS, optical flow, rangefinder, RC, blackbox) and whether they are healthy. Includes main loop load, battery voltage/current/percentage, and logging capability.
- `RC_CHANNELS_RAW` (v1) / `RC_CHANNELS` (v2): up to 18 input channels plus RSSI mapped to MAVLink units.
- `GPS_RAW_INT`: GNSS fix quality, HDOP/VDOP, velocity, and satellite count when a fix (or estimated fix) exists.
- `GLOBAL_POSITION_INT`: GPS position plus INAV altitude and velocity estimates.
- `GPS_GLOBAL_ORIGIN`: current home position.
- `ATTITUDE`: roll, pitch, yaw, and angular rates.
- `VFR_HUD`: airspeed (if a healthy pitot is available), ground speed, throttle, altitude, and climb rate.
- `HEARTBEAT`: arming state plus ArduPilot-style `custom_mode` mapping from INAV flight modes.
- `EXTENDED_SYS_STATE`: landed-state reporting.
- `BATTERY_STATUS`: per-cell voltages (cells 11-14 in `voltages_ext`), current draw, consumed mAh/Wh, and remaining percentage when available.
- `SCALED_PRESSURE`: baro pressure and temperature data.
- `SYSTEM_TIME`: `time_boot_ms = millis()` and `time_unix_usec = 0`.
- `STATUSTEXT`: pending OSD/system messages with mapped severity.
- `MISSION_ITEM_REACHED`: sent when navigation reports a mission waypoint reached and broadcast to all enabled MAVLink ports.
- `AUTOPILOT_VERSION`: on request, advertises ArduPilot-compatible version `4.7.0` and the capabilities listed above (MAVLink2 only).
- `PROTOCOL_VERSION`: on request, reports the configured MAVLink protocol version (MAVLink2 only).
- `MESSAGE_INTERVAL`: reply payload for `MAV_CMD_GET_MESSAGE_INTERVAL`.
- `HOME_POSITION`: on request, when a home fix exists.
- `AVAILABLE_MODES`: on request, for the current vehicle type / mode family.
- `CURRENT_MODE`: on request, for the current selected mode view.

## Supported incoming messages

- `HEARTBEAT`: used to detect ADS-B participants when `type` is `MAV_TYPE_ADSB`.
- `MISSION_COUNT`: starts an upload transaction (capped at `NAV_MAX_WAYPOINTS`).
- `MISSION_ITEM` / `MISSION_ITEM_INT`: stores mission waypoints; rejects unsupported frames / sequence errors. Upload while armed is rejected except legacy guided waypoint writes.
- `MISSION_REQUEST_LIST`, `MISSION_REQUEST`, `MISSION_REQUEST_INT`: downloads active mission items; returns `MISSION_ACK` on bad sequence.
- `MISSION_CLEAR_ALL`: clears stored mission.
- `COMMAND_LONG` / `COMMAND_INT`: command transport for supported `MAV_CMD_*` handlers.
- `REQUEST_DATA_STREAM`: legacy stream-rate control per stream group.
- `SET_POSITION_TARGET_GLOBAL_INT`: writes the GCS-guided waypoint when the frame is supported; altitude-only requests are also accepted when X/Y are masked out and GCS navigation is valid.
- `SET_POSITION_TARGET_LOCAL_NED`: accepts altitude-only requests in `MAV_FRAME_LOCAL_OFFSET_NED` when X/Y are zero or ignored and GCS navigation is valid.
- `RC_CHANNELS_OVERRIDE`: passes channel values to the MAVLink serial receiver backend.
- `MLRS_RADIO_LINK_STATS`: native MLRS per-port link stats from `MAV_COMP_ID_TELEMETRY_RADIO`, with local target-system / target-component checks.
- `MLRS_RADIO_LINK_INFORMATION`: native MLRS per-port link metadata from `MAV_COMP_ID_TELEMETRY_RADIO`, with local target-system / target-component checks.
- `MLRS_RADIO_LINK_FLOW_CONTROL`: native MLRS per-port flow-control input from `MAV_COMP_ID_TELEMETRY_RADIO`; applied only to the ingress port runtime.
- `RADIO_STATUS`: updates remote TX buffer level and generic / legacy link stats according to `mavlink_port{1-4}_radio_type`. This remains the non-MLRS radio path.
- `ADSB_VEHICLE`: populates the internal traffic list when ADS-B is enabled.
- `PARAM_REQUEST_LIST`: elicits a stub `PARAM_VALUE` response so ground stations stop requesting parameters.
- `TUNNEL`: accepts private payload type `0x8001` for MSP-over-MAVLink on MAVLink2 links.

## Supported commands

INAV implements a selective but useful subset of the MAVLink Command protocol. Unsupported commands are ACKed as `UNSUPPORTED`.

- `MAV_CMD_DO_REPOSITION`: sets the Follow Me / GCS-nav waypoint when GCS nav is valid. Accepts `MAV_FRAME_GLOBAL`, `MAV_FRAME_GLOBAL_INT`, `MAV_FRAME_GLOBAL_RELATIVE_ALT`, and `MAV_FRAME_GLOBAL_RELATIVE_ALT_INT`; otherwise `UNSUPPORTED`.
- `MAV_CMD_DO_CHANGE_ALTITUDE`: changes the current altitude target. `param1` is the target altitude in meters and `param2` is interpreted as the MAVLink frame (`MAV_FRAME_GLOBAL`, `MAV_FRAME_GLOBAL_INT`, `MAV_FRAME_GLOBAL_RELATIVE_ALT`, `MAV_FRAME_GLOBAL_RELATIVE_ALT_INT`); unsupported frames are rejected.
- `MAV_CMD_CONDITION_YAW`: changes the current heading target when the active navigation state has yaw control. Accepts absolute heading (`param4 = 0`) and relative turns (`param4 != 0`); turn rate is ignored.
- `MAV_CMD_SET_MESSAGE_INTERVAL` / `MAV_CMD_GET_MESSAGE_INTERVAL`: adjust or query per-message periodic output for `HEARTBEAT`, `SYS_STATUS`, `EXTENDED_SYS_STATE`, RC channels, `GPS_RAW_INT`, `GLOBAL_POSITION_INT`, `GPS_GLOBAL_ORIGIN`, `ATTITUDE`, `VFR_HUD`, `BATTERY_STATUS`, `SCALED_PRESSURE`, and `SYSTEM_TIME`. `REQUEST_DATA_STREAM` still controls the legacy base stream groups; `SET_MESSAGE_INTERVAL` overrides individual messages on top.
- `MAV_CMD_GET_HOME_POSITION`: replies with `HOME_POSITION` when a home fix exists.
- `MAV_CMD_REQUEST_MESSAGE`: emits one selected message (`HEARTBEAT`, `AUTOPILOT_VERSION`, `PROTOCOL_VERSION`, `SYS_STATUS`, `ATTITUDE`, `VFR_HUD`, `AVAILABLE_MODES`, `CURRENT_MODE`, `EXTENDED_SYS_STATE`, RC channels, `GPS_RAW_INT`, `GLOBAL_POSITION_INT`, `GPS_GLOBAL_ORIGIN`, `BATTERY_STATUS`, `SCALED_PRESSURE`, `SYSTEM_TIME`, `STATUSTEXT`, and `HOME_POSITION` when available) or `UNSUPPORTED`.
- `MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES`: returns `AUTOPILOT_VERSION` (MAVLink2 only; MAVLink1 returns `UNSUPPORTED`) advertising ArduPilot-compatible version `4.7.0` and the capabilities listed above.
- `MAV_CMD_REQUEST_PROTOCOL_VERSION`: returns `PROTOCOL_VERSION` (MAVLink2 only; MAVLink1 returns `UNSUPPORTED`).
- `MAV_CMD_CONTROL_HIGH_LATENCY`: enables or disables `HIGH_LATENCY2` scheduling on the ingress MAVLink port (`param1 = 0` or `1`). Enabling is rejected on MAVLink1 links.

## Mode mappings (INAV -> MAVLink / ArduPilot)

`custom_mode` is derived from active INAV telemetry flight mode (`getFlightModeForTelemetry()`), then mapped per vehicle type.

- **Multirotor profiles**
  - ACRO / ACRO AIR -> **ACRO**
  - ANGLE / HORIZON / ANGLE HOLD -> **STABILIZE**
  - ALT HOLD -> **ALT_HOLD**
  - POS HOLD -> **GUIDED** (if GCS valid), otherwise **POSHOLD**
  - RTH -> **RTL**
  - MISSION -> **AUTO**
  - LAUNCH -> **THROW**
  - FAILSAFE -> **RTL** (RTH / other phases) or **LAND** (landing phase)
  - Any other unmapped mode falls back to **STABILIZE**
- **Fixed-wing profiles**
  - MANUAL -> **MANUAL**
  - ACRO / ACRO AIR -> **ACRO**
  - ANGLE -> **FBWA**
  - HORIZON / ANGLE HOLD -> **STABILIZE**
  - ALT HOLD -> **FBWB**
  - POS HOLD -> **GUIDED** (if GCS valid), otherwise **LOITER**
  - RTH -> **RTL**
  - MISSION -> **AUTO**
  - CRUISE -> **CRUISE**
  - LAUNCH -> **TAKEOFF**
  - FAILSAFE -> **RTL** (RTH / other phases) or **AUTOLAND** (landing phase)
  - Any other unmapped mode falls back to **MANUAL**

## MAVLink missions

INAV supports MAVLink mission upload, download, clear, and waypoint-reached notifications, and works with common MAVLink mission planners such as QGC for simple mission flows. However, the differences between MAVLink missions and INAV's fuller MSP navigation model mean MAVLink still cannot represent every INAV mission feature. Use MultiWii Planner or the INAV Configurator when you need full MSP mission semantics.

## MSP mission parity gaps (MAV <-> MSP)

- WAYPOINT: MSP->MAV sends lat/lon/alt but drops leg speed `p1` and all user-action bits in `p3` (only alt-mode bit drives frame). MAV->MSP stores lat/lon/alt but forces `p1 = 0`, `p2 = 0`, keeps only alt-mode bit in `p3`; leg speed and user bits are lost.
- POSHOLD_TIME / LOITER_TIME: loiter time `p1` OK; leg speed `p2` and user-action bits in `p3` are discarded both directions.
- LAND: lat/lon/alt OK; leg speed `p1`, ground elevation `p2`, and user-action bits in `p3` are cleared in both directions (only alt-mode bit is retained from frame on upload).
- RTH: RTH land-if-nonzero flag in `p1` is ignored both ways (always zeroed); user-action bits dropped; alt is sent only if the MAVLink frame is coordinate and returns with alt-mode bit set on upload.
- JUMP: target and repeat count OK.
- SET_POI: lat/lon/alt OK; `param1` is fixed to `MAV_ROI_LOCATION`; user-action bits in `p3` are dropped (alt-mode bit respected on upload).
- SET_HEAD: heading `p1` OK; user-action bits in `p3` are not represented.
- Net effect: actions and positions OK, but MSP-specific fields (leg speed, LAND elevation adjustment, RTH land flag, user-action bits in `p3`) are lost, so MAVLink missions cannot fully conform to `MSP_navigation_messages.md`.

## MSP over MAVLink tunnel

This feature uses the MAVLink [Tunnel service](https://mavlink.io/en/services/tunnel.html) to let the INAV Configurator use MSP over an existing MAVLink telemetry link, typically a radio link where there is no separate wireless MSP device.
**It is not intended as a general-purpose serial tunnel, and it is not a replacement for normal MAVLink control / telemetry traffic.**
CLI mode is unavailable in MSP-over-MAVLink.

- INAV accepts `TUNNEL` messages with private payload type `0x8001` as an MSP byte stream carried over MAVLink2.
- `target_system` must match `mavlink_sysid`.
- `target_component` may be `0` or `MAV_COMP_ID_AUTOPILOT1`.
- `target_component = 0` is handled on the ingress MAVLink port only; it is not fanned out to other local MAVLink ports.
- MSP replies are sent back to the requester as one or more `TUNNEL` messages on that same ingress port.
- MSP framing is preserved end-to-end: MSPv1 requests get MSPv1 replies, and MSPv2 requests get MSPv2 replies.
- Reboot (`MSP_REBOOT`) is supported over the tunnel. Serial passthrough and ESC 4way passthrough are rejected before execution.

## High latency mode

High-latency mode uses the MAVLink [High Latency service](https://mavlink.io/en/services/high_latency.html) to replace normal scheduled telemetry on one port with periodic `HIGH_LATENCY2` summaries for very low-bandwidth or intermittent links.

- High latency mode is per-port, controlled by `mavlink_port{1-4}_high_latency` or by `MAV_CMD_CONTROL_HIGH_LATENCY` received on that port.
- It requires MAVLink2. MAVLink1 cannot enable or carry `HIGH_LATENCY2`.
- When enabled on a port, normal stream scheduling on that port is replaced by `HIGH_LATENCY2` at 5 second intervals.
- This is intended for slow and high-latency telemetry such as cellular, satellite, or LoRa, not for normal rich telemetry, mission planning, or configurator use.

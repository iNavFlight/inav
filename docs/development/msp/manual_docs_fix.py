MSP_SET_VTX_CONFIG = """**Temporary definition**
*   **Direction:** In
*   **Description:** Sets the VTX configuration (band, channel, power, pit mode). Supports multiple protocol versions/extensions based on payload size.
*   **Payload (Minimum):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `bandChannelEncoded` | `uint16_t` | 2 | Encoded band/channel value: `(band-1)*8 + (channel-1)`. If <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL` |
*   **Payload (Extended):** (Fields added sequentially based on size)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `power` | `uint8_t` | 1 | Power level index to set (`vtxSettingsConfigMutable()->power`) |
    | `pitMode` | `uint8_t` | 1 | Pit mode state to set (0=off, 1=on). Directly calls `vtxCommonSetPitMode` |
    | `lowPowerDisarm` | `uint8_t` | 1 | Low power on disarm setting (`vtxSettingsConfigMutable()->lowPowerDisarm`) |
    | `pitModeFreq` | `uint16_t` | 2 | *Ignored*. Betaflight extension |
    | `band` | `uint8_t` | 1 | Explicit band number to set (`vtxSettingsConfigMutable()->band`). Overrides encoded value if present |
    | `channel` | `uint8_t` | 1 | Explicit channel number to set (`vtxSettingsConfigMutable()->channel`). Overrides encoded value if present |
    | `frequency` | `uint16_t` | 2 | *Ignored*. Betaflight extension |
    | `bandCount` | `uint8_t` | 1 | *Ignored*. Betaflight extension |
    | `channelCount` | `uint8_t` | 1 | *Ignored*. Betaflight extension |
    | `powerCount` | `uint8_t` | 1 | *Ignored*. Betaflight extension (can potentially reduce reported power count if valid) |
*   **Notes:** Requires `USE_VTX_CONTROL`. Minimum size 2 bytes. Applies settings to `vtxSettingsConfig` and potentially directly to the device (pit mode).
"""
MSP2_INAV_SET_GEOZONE_VERTEX = """**Temporary definition**
*   **Direction:** In
*   **Description:** Sets the main configuration for a specific Geozone (type, shape, altitude, action). **This command resets (clears) all vertices associated with the zone.**
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |
    | `type` | `uint8_t` | 1 | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
    | `shape` | `uint8_t` | 1 | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGHON`): Zone shape (Polygon/Circular) |
    | `minAltitude` | `uint32_t` | 4 | Minimum allowed altitude (cm) |
    | `maxAltitude` | `uint32_t` | 4 | Maximum allowed altitude (cm) |
    | `isSeaLevelRef` | `uint8_t` | 1 | Boolean: Altitude reference |
    | `fenceAction` | `uint8_t` | 1 | Enum (`geozoneActionState_e`): Action to take upon boundary violation |
    | `vertexCount` | `uint8_t` | 1 | Number of vertices to be defined (used for validation later) |
*   **Notes:** Requires `USE_GEOZONE`. Expects 14 bytes. Returns error if index invalid. Calls `geozoneResetVertices()`. Vertices must be set subsequently using `MSP2_INAV_SET_GEOZONE_VERTEX`.""" 
MSP2_COMMON_SET_SETTING = """**Temporary definition**
*   **Direction:** In
*   **Description:** Sets the value of a specific configuration setting, identified by name or index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `settingIdentifier` | Varies | Variable | Setting name (null-terminated string) OR Index (0x00 followed by `uint16_t` index) |
    | `settingValue` | `uint8_t[]` | Variable | Raw byte value to set for the setting. Size must match the setting's type |
*   **Notes:** Performs type checking and range validation (min/max). Returns error if setting not found, value size mismatch, or value out of range. Handles different data types (`uint8`, `int16`, `float`, `string`, etc.) internally.
"""
MSP2_SENSOR_HEADTRACKER = """**Temporary definition**
*   **Direction:** In
*   **Description:** Provides head tracker orientation data.
*   **Payload:** (Structure not defined in provided headers, but likely Roll, Pitch, Yaw angles)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `...` | Varies | Variable | Head tracker angles (e.g., int16 Roll, Pitch, Yaw in deci-degrees) |
*   **Notes:** Requires `USE_HEADTRACKER` and `USE_HEADTRACKER_MSP`. Calls `mspHeadTrackerReceiverNewData()`. Payload structure needs verification from `mspHeadTrackerReceiverNewData` implementation.
"""
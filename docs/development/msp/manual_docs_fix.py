MSP2_COMMON_SET_SETTING = """**Temporary definition**
*   **Description:** Sets the value of a specific configuration setting, identified by name or index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `settingIdentifier` | Varies | Variable | Setting name (null-terminated string) OR Index (0x00 followed by `uint16_t` index) |
    | `settingValue` | `uint8_t[]` | Variable | Raw byte value to set for the setting. Size must match the setting's type |
*   **Notes:** Performs type checking and range validation (min/max). Returns error if setting not found, value size mismatch, or value out of range. Handles different data types (`uint8`, `int16`, `float`, `string`, etc.) internally.
"""
MSP2_SENSOR_HEADTRACKER = """**Temporary definition**
*   **Description:** Provides head tracker orientation data.
*   **Payload:** (Structure not defined in provided headers, but likely Roll, Pitch, Yaw angles)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `...` | Varies | Variable | Head tracker angles (e.g., int16 Roll, Pitch, Yaw in deci-degrees) |
*   **Notes:** Requires `USE_HEADTRACKER` and `USE_HEADTRACKER_MSP`. Calls `mspHeadTrackerReceiverNewData()`. Payload structure needs verification from `mspHeadTrackerReceiverNewData` implementation.
"""
# MSP Message Routing Architecture in fc_msp.c

## Overview

This document explains how MSP messages are routed through different handler functions in `src/main/fc/fc_msp.c`. Understanding this routing is critical when adding new MSP commands or debugging message flow.

---

## The Four Handler Functions

### 1. mspFcProcessCommand() - Main Router

**File:** `src/main/fc/fc_msp.c`

**Signature:**
```c
mspResult_e mspFcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply, mspPostProcessFnPtr *mspPostProcessFn)
```

**Purpose:** Top-level router that delegates to specialized handlers

**Routing Logic (in order of priority):**

```c
if (MSP2_IS_SENSOR_MESSAGE(cmdMSP)) {
    // Route to sensor handler
    ret = mspProcessSensorCommand(cmdMSP, src);

} else if (mspFcProcessOutCommand(cmdMSP, dst, mspPostProcessFn)) {
    // Route to OUT command handler (read-only queries)
    ret = MSP_RESULT_ACK;

} else if (cmdMSP == MSP_SET_PASSTHROUGH) {
    // Handle passthrough directly
    mspFcSetPassthroughCommand(dst, src, mspPostProcessFn);
    ret = MSP_RESULT_ACK;

} else if (cmdMSP == MSP_REBOOT) {
    // Handle reboot directly (MUST NOT be armed)
    if (!ARMING_FLAG(ARMED)) {
        ret = mspFcRebootCommand(src, mspPostProcessFn);
    } else {
        ret = MSP_RESULT_ERROR;
    }

} else {
    // Try INOUT handler first, then IN handler as fallback
    if (!mspFCProcessInOutCommand(cmdMSP, dst, src, &ret)) {
        ret = mspFcProcessInCommand(cmdMSP, src);
    }
}
```

**Commands Handled Directly:**
- MSP sensor messages (MSP2_IS_SENSOR_MESSAGE macro)
- MSP_SET_PASSTHROUGH
- MSP_REBOOT

---

### 2. mspFcProcessOutCommand() - Read-Only Queries

**File:** `src/main/fc/fc_msp.c`

**Signature:**
```c
static bool mspFcProcessOutCommand(uint16_t cmdMSP, sbuf_t *dst, mspPostProcessFnPtr *mspPostProcessFn)
```

**Purpose:** Handle commands that only READ data (no configuration changes)

**Parameters:**
- `cmdMSP` - Command ID
- `dst` - Destination buffer (reply payload)
- `mspPostProcessFn` - Optional post-process function pointer

**Return Value:**
- `true` - Command was handled
- `false` - Command not recognized (falls through to next handler)

**Message Direction:** FC → Configurator (OUT)

**Typical Commands:**
- MSP_API_VERSION - Protocol/API version info
- MSP_FC_VARIANT - "INAV" identifier
- MSP_FC_VERSION - Firmware version
- MSP_BOARD_INFO - Hardware identification
- MSP_STATUS / MSP_STATUS_EX - Flight controller status
- MSP_SENSOR_STATUS - Sensor health status
- MSP2_INAV_ANALOG - Battery/voltage readings
- MSP_ATTITUDE - Current attitude (roll/pitch/yaw)
- MSP_ALTITUDE - Altitude readings
- MSP_RAW_GPS - GPS data
- MSP_NAV_STATUS - Navigation state

**Characteristics:**
- Only writes to `dst` buffer (never reads `src`)
- Always returns `true` for handled commands
- Falls through to `default: return false;` for unknown commands
- Does NOT modify flight controller configuration

---

### 3. mspFcProcessInCommand() - Configuration Commands

**File:** `src/main/fc/fc_msp.c`

**Signature:**
```c
static mspResult_e mspFcProcessInCommand(uint16_t cmdMSP, sbuf_t *src)
```

**Purpose:** Handle commands that WRITE configuration (set/modify state)

**Parameters:**
- `cmdMSP` - Command ID
- `src` - Source buffer (request payload)

**Return Value:**
- `MSP_RESULT_ACK` - Command successful
- `MSP_RESULT_ERROR` - Command failed (validation error, armed state, etc.)

**Message Direction:** Configurator → FC (IN)

**Typical Commands:**
- MSP_SELECT_SETTING - Change active profile
- MSP_SET_HEAD - Set heading hold target
- MSP_SET_RAW_RC - RC channel override
- MSP_SET_LOOP_TIME - Set gyro loop time
- MSP2_SET_PID - Update PID values
- MSP_SET_MODE_RANGE - Configure flight modes
- MSP_SET_ADJUSTMENT_RANGE - Configure in-flight adjustments
- MSP_SET_RC_TUNING - Update RC rates/expo
- MSP_SET_MOTOR - Set motor test values
- MSP_EEPROM_WRITE - Save configuration to EEPROM

**Characteristics:**
- Only reads from `src` buffer (never writes `dst`)
- Never sends reply payload (ACK/ERROR only)
- Often checks `ARMING_FLAG(ARMED)` before allowing changes
- Validates input data before applying
- Returns `MSP_RESULT_ERROR` on validation failures

---

### 4. mspFCProcessInOutCommand() - Bidirectional Commands

**File:** `src/main/fc/fc_msp.c`

**Signature:**
```c
bool mspFCProcessInOutCommand(uint16_t cmdMSP, sbuf_t *dst, sbuf_t *src, mspResult_e *ret)
```

**Purpose:** Handle commands that both READ input AND WRITE output

**Parameters:**
- `cmdMSP` - Command ID
- `dst` - Destination buffer (reply payload)
- `src` - Source buffer (request payload)
- `ret` - Output parameter for result code

**Return Value:**
- `true` - Command was handled (check `*ret` for ACK/ERROR)
- `false` - Command not recognized (falls through to mspFcProcessInCommand)

**Message Direction:** Bidirectional (IN + OUT)

**Typical Commands:**
- MSP_WP - Get/Set waypoint (index in request, waypoint data in reply)
- MSP_DATAFLASH_READ - Read blackbox data (address in request, data in reply)
- MSP2_COMMON_SETTING - Get setting value by name
- MSP2_COMMON_SET_SETTING - Set setting value by name
- MSP2_COMMON_SETTING_INFO - Get setting metadata
- MSP2_COMMON_PG_LIST - List parameter groups
- MSP2_INAV_OSD_LAYOUTS - Get/Set OSD layout (layout/item ID in request, position in reply)

**Characteristics:**
- Reads from `src` AND writes to `dst`
- Sets `*ret` to MSP_RESULT_ACK or MSP_RESULT_ERROR
- Returns `false` on unknown command (fallback to IN handler)
- Used for complex queries with parameters
- Often used for indexed data access (waypoints, settings, etc.)

---

## Message Direction Terminology

### IN (Configurator → FC)
- **Write-only** commands
- No reply payload (ACK/ERROR only)
- Examples: MSP_SET_PID, MSP_EEPROM_WRITE
- Handler: `mspFcProcessInCommand()`

### OUT (FC → Configurator)
- **Read-only** queries
- Reply contains data
- No request payload
- Examples: MSP_STATUS, MSP_ATTITUDE, MSP_RAW_GPS
- Handler: `mspFcProcessOutCommand()`

### INOUT (Bidirectional)
- **Read AND write** commands
- Request contains parameters (e.g., index, name)
- Reply contains queried data
- Examples: MSP_WP, MSP2_COMMON_SETTING
- Handler: `mspFCProcessInOutCommand()`

**Note:** This terminology comes from the MSP packet direction byte:
- `$M<` = request TO FC (IN)
- `$M>` = response FROM FC (OUT)

---

## Routing Decision Tree

```
mspFcProcessCommand() receives packet
│
├─ Is sensor message? → mspProcessSensorCommand()
│
├─ Handled by mspFcProcessOutCommand()? → Return ACK
│  └─ Returns true if recognized
│
├─ Is MSP_SET_PASSTHROUGH? → Handle directly → Return ACK
│
├─ Is MSP_REBOOT?
│  ├─ Armed? → Return ERROR
│  └─ Not armed? → mspFcRebootCommand() → Return ACK/ERROR
│
└─ Try mspFCProcessInOutCommand()
   ├─ Returns true? → Return result from handler
   └─ Returns false? → mspFcProcessInCommand() → Return ACK/ERROR
```

---

## When to Use Each Handler

### Add to mspFcProcessOutCommand() when:
- Command only reads current state
- No parameters needed (or very simple ones)
- Reply is always the same structure
- Examples: status, version info, sensor readings

### Add to mspFcProcessInCommand() when:
- Command only writes configuration
- No reply data needed (just ACK/ERROR)
- May need to check arming state
- Examples: setting PID values, saving EEPROM

### Add to mspFCProcessInOutCommand() when:
- Command needs input parameters AND returns data
- Indexed/named data access
- Complex queries (e.g., get setting by name)
- Examples: waypoint operations, setting queries

### Handle directly in mspFcProcessCommand() when:
- **Post-processing required** - Command must execute *after* reply is sent
  - `mspPostProcessFn` is ONLY available in main router
  - IN/OUT/INOUT handlers cannot access post-process function
  - Examples: MSP_REBOOT (must reply before rebooting), MSP_SET_PASSTHROUGH
- **Special routing** - Sensors (handled by dedicated sensor subsystem)

**Note:** Security checks (armed state) and payload parsing can be done in any handler - only post-processing capability requires direct handling.

---

**Why MSP_REBOOT must be handled directly:**

MSP_REBOOT **cannot** be handled in `mspFcProcessInCommand()` because that function lacks access to `mspPostProcessFn`:

```c
// Main router - HAS mspPostProcessFn parameter
mspResult_e mspFcProcessCommand(mspPacket_t *cmd, mspPacket_t *reply,
                                 mspPostProcessFnPtr *mspPostProcessFn)

// IN handler - NO mspPostProcessFn parameter
static mspResult_e mspFcProcessInCommand(uint16_t cmdMSP, sbuf_t *src)
```

**Why post-processing is required:**
- The FC must send the MSP ACK reply **before** rebooting

## Code Examples

### OUT Command (Read-Only)
```c
case MSP_API_VERSION:
    sbufWriteU8(dst, MSP_PROTOCOL_VERSION);
    sbufWriteU8(dst, API_VERSION_MAJOR);
    sbufWriteU8(dst, API_VERSION_MINOR);
    break;
```

### IN Command (Write-Only)
```c
case MSP_SET_HEAD:
    if (sbufReadU16Safe(&tmp_u16, src)) {
        const int32_t headingCentidegrees = wrap_36000(DEGREES_TO_CENTIDEGREES(tmp_u16));
        updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(headingCentidegrees));

        if (navGetCurrentStateFlags() & NAV_CTL_YAW) {
            posControl.desiredState.yaw = headingCentidegrees;
            posControl.cruise.course = headingCentidegrees;
            posControl.cruise.previousCourse = headingCentidegrees;
        }
    } else
        return MSP_RESULT_ERROR;
    break;
```

### INOUT Command (Bidirectional)
```c
case MSP_WP:
    mspFcWaypointOutCommand(dst, src);  // Reads index from src, writes waypoint to dst
    *ret = MSP_RESULT_ACK;
    break;
```

### Direct Handling (Special Cases)
```c
else if (cmdMSP == MSP_REBOOT) {
    if (!ARMING_FLAG(ARMED)) {
        ret = mspFcRebootCommand(src, mspPostProcessFn);
    } else {
        ret = MSP_RESULT_ERROR;
    }
}
```

---

## Key Takeaways

1. **OUT = Read-only queries** (no state changes)
2. **IN = Write-only commands** (no reply data)
3. **INOUT = Bidirectional** (parameters + reply data)
4. **Direct handling is rare** - ONLY needed when `mspPostProcessFn` required
   - IN/OUT/INOUT handlers cannot access post-process function
   - Commands needing delayed execution must be handled directly
   - Examples: sensors, passthrough, reboot
5. **INOUT tries first, then falls back to IN**
6. **Return values matter:**
   - OUT: `true/false` (handled/not handled)
   - IN: `MSP_RESULT_ACK/ERROR`
   - INOUT: `true/false` + sets `*ret`
7. **Function signatures determine capabilities:**
   - Only `mspFcProcessCommand()` has `mspPostProcessFn` parameter
   - Security checks and payload parsing can be done in any handler

---

## Related Files

- **Main router:** `src/main/fc/fc_msp.c`
- **Command definitions:** `src/main/msp/msp_protocol.h`
- **MSP V2 commands:** `src/main/msp/msp_protocol_v2_inav.h`
- **Serial layer:** `src/main/msp/msp_serial.c`
- **Schema:** `docs/development/msp/msp_messages.json`

---

## References

- MSP Protocol Documentation: [format.md](format.md)
- MSP Message Reference: [README.md](README.md)

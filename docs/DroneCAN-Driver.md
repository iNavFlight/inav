# DroneCAN Driver Documentation

**Last Updated:** 2026-02-16
**Status:** Complete
**Branch:** feature-dronecan-sitl

---

## Overview

The DroneCAN driver (`dronecan.c/dronecan.h`) provides CAN bus communication for INAV using the DroneCAN protocol (UAVCAN v1). It enables INAV to act as a CAN bus master, receiving sensor data (GPS, barometer, magnetometer, battery) from DroneCAN peripherals and sharing INAV's node status with the network.

### Key Capabilities

- **CAN Bus Communication:** Support for 125 kbps, 250 kbps, 500 kbps, and 1000 kbps bitrates
- **Message Reception:** Receive GPS fixes, battery info, node status, and GNSS auxiliary data
- **Message Transmission:** Broadcast node status at 1 Hz showing health, mode, and uptime
- **Service Handling:** Respond to GetNodeInfo requests from other nodes
- **Bus Recovery:** Automatic recovery from CAN bus-off errors
- **Integration:** Direct integration with GPS provider and battery sensor systems

---

## Architecture

### High-Level Overview

```
DroneCAN Driver Flow:
┌─────────────────────────────────────────────────────────────┐
│ dronecanInit() at startup                                   │
│ - Initialize CAN bus (STM32 hardware)                       │
│ - Initialize libcanard (message codec)                      │
│ - Register callbacks (shouldAcceptTransfer, onTransferReceived)
│ - Set local node ID from configuration                      │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ dronecanUpdate() called repeatedly from scheduler           │
│ - Process TX queue (send pending messages)                  │
│ - Read RX FIFO and process incoming frames                  │
│ - Run 1Hz tasks (send node status, cleanup stale transfers) │
│ - Handle CAN bus-off recovery                               │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ├─ Receive path:
                 │  1. canardSTM32Receive() gets frame from CAN hardware
                 │  2. canardHandleRxFrame() processes via libcanard
                 │  3. shouldAcceptTransfer() decides if we care about this message
                 │  4. onTransferReceived() dispatches to handler function
                 │  5. Handler decodes message and integrates with INAV
                 │
                 └─ Transmit path:
                    1. send_NodeStatus() creates and encodes message
                    2. canardBroadcast() queues in libcanard TX buffer
                    3. processCanardTxQueue() sends queued frames via CAN hardware
```

### Initialization Sequence

**Phase 1: Hardware Setup** (in `dronecanInit()`)
1. Select bitrate from configuration (125/250/500/1000 kbps)
2. Call `canardSTM32CAN1_Init(bitrate)` - initializes STM32 CAN peripheral
3. Enable CAN interrupts for RX/TX

**Phase 2: Libcanard Setup**
1. Call `canardInit()` with:
   - Memory pool (1024 bytes for message codec state)
   - Callback: `onTransferReceived` - invoked when a message arrives
   - Callback: `shouldAcceptTransfer` - invoked to filter messages
2. Set local node ID via `canardSetLocalNodeID()` from configuration
3. Node is now ready to send/receive DroneCAN messages

**Phase 3: Runtime** (in `dronecanUpdate()`)
- State machine with 3 states: INIT → NORMAL → BUS_OFF (recovery) → NORMAL
- Continuously process incoming messages and transmit queued messages
- 1Hz timer: cleanup stale transfers and broadcast node status

### Message Reception Flow

```
CAN Bus ──[CAN interrupt]──> canardSTM32Receive() ──> shouldAcceptTransfer()
                                                              │
                                            ┌─────────────────┴─────────────────┐
                                            │                                   │
                                       Returns true                        Returns false
                                            │                                   │
                                            ▼                                   ▼
                                  onTransferReceived()                    Message ignored
                                            │
                                    Switch on data_type_id
                                            │
                        ┌───────────────────┼───────────────────┐
                        │                   │                   │
                   GNSS_FIX_ID          BATTERY_ID          NODESTATUS_ID
                        │                   │                   │
                        ▼                   ▼                   ▼
              handle_GNSSFix()      handle_BatteryInfo()  handle_NodeStatus()
                        │                   │                   │
                  uavcan_equipment_     dronecanBattery    Log node status
                  gnss_Fix_decode()     SensorReceiveInfo()
                        │
              dronecanGPSReceive...()
```

### Supported Message Types

| Data Type ID | Handler Function | Message | Format | Rate | Notes |
|---|---|---|---|---|---|
| `UAVCAN_PROTOCOL_NODESTATUS_ID` | `handle_NodeStatus()` | Node Status | Broadcast | 1 Hz | Received from other nodes |
| `UAVCAN_EQUIPMENT_GNSS_FIX_ID` | `handle_GNSSFix()` | GPS Fix | Broadcast | 5-10 Hz | Integrated with GPS provider |
| `UAVCAN_EQUIPMENT_GNSS_FIX2_ID` | `handle_GNSSFix2()` | GPS Fix2 | Broadcast | 5-10 Hz | Extended GPS data |
| `UAVCAN_EQUIPMENT_GNSS_AUXILIARY_ID` | `handle_GNSSAuxiliary()` | GPS Auxiliary | Broadcast | 5-10 Hz | Satellite count, HDOP |
| `UAVCAN_EQUIPMENT_GNSS_RTCMSTREAM_ID` | `handle_GNSSRCTMStream()` | RTCM Stream | Broadcast | Variable | RTK correction data |
| `UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID` | `handle_BatteryInfo()` | Battery Info | Broadcast | 1-10 Hz | Integrated with battery system |
| `UAVCAN_PROTOCOL_GETNODEINFO_ID` | `handle_GetNodeInfo()` | GetNodeInfo | Request/Response | On demand | Responds with FC firmware version |

### Handler-Based Architecture

The driver uses a handler callback pattern. Each supported message type has:

1. **Handler Function:** Receives the message, decodes it, and integrates with INAV systems
   ```c
   void handle_GNSSFix(CanardInstance *ins, CanardRxTransfer *transfer) {
       struct uavcan_equipment_gnss_Fix gnssFix;
       if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix)) {
           return;  // Decode error
       }
       dronecanGPSReceiveGNSSFix(&gnssFix);  // Integrate with GPS
   }
   ```

2. **Filter Function:** `shouldAcceptTransfer()` returns `true` to receive, `false` to ignore
   ```c
   case UAVCAN_EQUIPMENT_GNSS_FIX_ID: {
       *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE;
       return true;  // Yes, we want this message
   }
   ```

3. **Dispatcher:** `onTransferReceived()` routes message to correct handler
   ```c
   case UAVCAN_EQUIPMENT_GNSS_FIX_ID:
       handle_GNSSFix(ins, transfer);
       break;
   ```

### Output: Node Status Broadcast

The driver broadcasts INAV's node status at 1 Hz via `send_NodeStatus()`:

```c
struct uavcan_protocol_NodeStatus {
    uint32_t uptime_sec;              // Seconds since boot
    uint8_t health;                   // HEALTH_OK, WARNING, ERROR, CRITICAL
    uint8_t mode;                     // OPERATIONAL, INITIALIZATION, MAINTENANCE, etc.
    uint8_t sub_mode;                 // Reserved, usually 0
    uint16_t vendor_specific_status_code;  // Custom status (currently set to 1234)
};
```

This message allows INAV to appear in DroneCAN GUI tools and be monitored by other nodes.

---

## Public API

### Functions

#### `void dronecanInit(void)`

**Purpose:** Initialize the DroneCAN driver at startup

**Behavior:**
1. Configures CAN bus bitrate based on `dronecanConfig()->bitRateKbps`
2. Initializes STM32 CAN hardware via `canardSTM32CAN1_Init()`
3. Initializes libcanard message codec
4. Registers message handlers via `shouldAcceptTransfer` and `onTransferReceived` callbacks
5. Sets INAV's node ID from configuration (must be > 0 for transmit)

**Parameters:** None

**Returns:** void

**Error Handling:**
- If CAN hardware init fails, no recovery is attempted (returns normally but CAN is non-functional)
- If node ID is 0, logs warning and node cannot transmit

**Example:**
```c
// In board initialization
dronecanInit();

// INAV node is now ready to send/receive DroneCAN messages
```

---

#### `void dronecanUpdate(timeUs_t currentTimeUs)`

**Purpose:** Process DroneCAN messages and transmit queued messages

**Called By:** Main scheduler loop (typically at 1000 Hz)

**Behavior:**
1. **State Machine Management:** Tracks initialization, normal operation, and bus-off recovery
2. **Process TX Queue:** Sends any messages queued by `canardBroadcast()`
3. **Process RX FIFO:** Reads incoming CAN frames from hardware
4. **Run 1Hz Tasks:** Every 1 second, sends node status and cleans up stale transfers
5. **Bus-Off Recovery:** If CAN bus-off error detected, wait 100ms before recovery attempt

**Parameters:**
- `currentTimeUs` - Current time in microseconds (used for timing 1Hz tasks and cleanup)

**Returns:** void

**State Transitions:**
```
STATE_DRONECAN_INIT
    ├─ First call: initialize timer for 1Hz service
    └─ Switch to STATE_DRONECAN_NORMAL

STATE_DRONECAN_NORMAL
    ├─ Process all queued messages (RX/TX)
    ├─ Every 1 second: send NodeStatus, cleanup transfers
    └─ If BusOff detected: switch to STATE_DRONECAN_BUS_OFF

STATE_DRONECAN_BUS_OFF
    ├─ Wait 100ms
    └─ Attempt recovery via canardSTM32RecoverFromBusOff()
        └─ If successful: return to STATE_DRONECAN_NORMAL
```

**Example:**
```c
// In main scheduler loop
dronecanUpdate(micros());  // Called repeatedly

// Messages are processed automatically and integrated into GPS/battery systems
```

---

### Configuration

#### Settings

The driver uses the following settings from `settings.yaml`:

| Setting | Description | Type | Valid Range | Default |
|---|---|---|---|---|
| `dronecan_mode` | Enable/disable DroneCAN | bool | 0/1 | 0 |
| `dronecan_node_id` | This FC's CAN node ID | uint8 | 1-125 | 0 |
| `dronecan_baudrate` | CAN bus bitrate | enum | 0-3 | 2 (500kbps) |

**Bitrate Mapping:**
- 0 = 125 kbps
- 1 = 250 kbps
- 2 = 500 kbps (default)
- 3 = 1000 kbps

#### Accessing Configuration

```c
// Read current configuration
uint8_t nodeID = dronecanConfig()->nodeID;
dronecanBitrate_e bitrate = dronecanConfig()->bitRateKbps;
```

---

## Message Handlers

The driver includes 7 built-in message handlers. Each handler decodes a message type and integrates with INAV systems.

### GPS Message Handlers

#### `handle_GNSSFix()`
**Receives:** `uavcan_equipment_gnss_Fix`
**Integrates:** Calls `dronecanGPSReceiveGNSSFix()` to update INAV GPS system
**Status:** Logs "GNSS Fix received" on successful decode

#### `handle_GNSSFix2()`
**Receives:** `uavcan_equipment_gnss_Fix2`
**Integrates:** Calls `dronecanGPSReceiveGNSSFix2()` to update INAV GPS system
**Status:** Logs "GNSS Fix2 received" on successful decode

#### `handle_GNSSAuxiliary()`
**Receives:** `uavcan_equipment_gnss_Auxiliary`
**Data Extracted:** Satellite count (`sats_used`), HDOP
**Status:** Logs "GNSS Auxiliary: Sats=X HDOP=Y.Z"
**Note:** Currently logs data but does not integrate with GPS system

#### `handle_GNSSRCTMStream()`
**Receives:** `uavcan_equipment_gnss_RTCMStream`
**Status:** Currently logs receipt only, no integration

### Power & System Handlers

#### `handle_BatteryInfo()`
**Receives:** `uavcan_equipment_power_BatteryInfo`
**Integrates:** Calls `dronecanBatterySensorReceiveInfo()` to update INAV battery system
**Status:** Logs "Battery Info"

#### `handle_NodeStatus()`
**Receives:** `uavcan_protocol_NodeStatus` (from other nodes)
**Processing:** Decodes and logs node health (OK/WARNING/ERROR/CRITICAL) and mode (OPERATIONAL/INITIALIZATION/MAINTENANCE/SOFTWARE_UPDATE/OFFLINE)
**Status:** Logs health and mode status

### Service Handlers

#### `handle_GetNodeInfo()`
**Type:** Service Request Handler
**Receives:** GetNodeInfo request from another node
**Response Contains:**
- FC firmware version (major, minor, patch)
- Hardware version (set to 1.0)
- Unique hardware ID
- Node uptime
- Current node status (health, mode)
- Firmware name

**Implementation Note:** Currently returns hardcoded hardware version; version info populated from build system

---

## Handler Registration

### The `shouldAcceptTransfer()` Callback

This function is called by libcanard when a new CAN message begins arriving. It decides whether INAV should receive the message.

**Returns:**
- `true` - Accept this message (call `onTransferReceived` when complete)
- `false` - Ignore this message (save processing power and RAM)

**Must fill in:** `*out_data_type_signature` - The DSDL signature for this message type

**Implementation:** Switch on `transfer_type` and `data_type_id`:

```c
bool shouldAcceptTransfer(..., uint16_t data_type_id, CanardTransferType transfer_type, ...) {
    if (transfer_type == CanardTransferTypeRequest) {
        switch (data_type_id) {
            case UAVCAN_PROTOCOL_GETNODEINFO_ID:
                *out_data_type_signature = UAVCAN_PROTOCOL_GETNODEINFO_REQUEST_SIGNATURE;
                return true;  // Yes, handle GetNodeInfo requests
        }
    }
    if (transfer_type == CanardTransferTypeBroadcast) {
        switch (data_type_id) {
            case UAVCAN_EQUIPMENT_GNSS_FIX_ID:
                *out_data_type_signature = UAVCAN_EQUIPMENT_GNSS_FIX_SIGNATURE;
                return true;  // Yes, receive GPS fix messages
        }
    }
    return false;  // Don't care about this message
}
```

### The `onTransferReceived()` Callback

This function is called by libcanard when a complete message has been received and decoded.

**Implementation:** Switch on `transfer->data_type_id` to dispatch to appropriate handler:

```c
void onTransferReceived(CanardInstance *ins, CanardRxTransfer *transfer) {
    if (transfer->transfer_type == CanardTransferTypeBroadcast) {
        switch (transfer->data_type_id) {
            case UAVCAN_EQUIPMENT_GNSS_FIX_ID:
                handle_GNSSFix(ins, transfer);
                break;
            case UAVCAN_EQUIPMENT_POWER_BATTERYINFO_ID:
                handle_BatteryInfo(ins, transfer);
                break;
            // ... more handlers ...
        }
    }
}
```

---

## Message Transmission

### Sending Node Status

INAV broadcasts its node status at 1 Hz via `send_NodeStatus()`:

```c
void send_NodeStatus(void) {
    uint8_t buffer[UAVCAN_PROTOCOL_NODESTATUS_MAX_SIZE];

    // Update status from current FC state
    node_status.uptime_sec = millis() / 1000UL;
    node_status.health = UAVCAN_PROTOCOL_NODESTATUS_HEALTH_OK;
    node_status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;
    node_status.vendor_specific_status_code = 1234;

    // Encode into binary format
    uint32_t len = uavcan_protocol_NodeStatus_encode(&node_status, buffer);

    // Broadcast via CAN (queues in libcanard)
    static uint8_t transfer_id;
    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}
```

### TX Queue Processing

Messages sent via `canardBroadcast()` are queued in libcanard's internal TX buffer. The driver transmits queued messages via `processCanardTxQueue()`:

```c
void processCanardTxQueue(void) {
    for (const CanardCANFrame *tx_frame;
         (tx_frame = canardPeekTxQueue(&canard)) != NULL;) {
        int16_t tx_res = canardSTM32Transmit(tx_frame);

        if (tx_res < 0) {
            // Error - discard frame
            canardPopTxQueue(&canard);
        } else if (tx_res > 0) {
            // Success - remove from queue
            canardPopTxQueue(&canard);
        } else {
            // TX FIFO full - retry later
            break;
        }
    }
}
```

Called from `dronecanUpdate()` every scheduler cycle. Messages are transmitted in order via STM32 CAN hardware.

---

## Integration Points

The DroneCAN driver integrates with other INAV subsystems:

### GPS Integration

When GPS messages arrive, handlers call:
- `dronecanGPSReceiveGNSSFix()` - Process standard GPS fix
- `dronecanGPSReceiveGNSSFix2()` - Process extended GPS fix

These functions are defined in a GPS provider implementation (e.g., `gps_dronecan.c`) and update INAV's GPS state.

### Battery Integration

When battery messages arrive, the handler calls:
- `dronecanBatterySensorReceiveInfo()` - Process battery status

This function is defined in battery sensor driver and updates battery voltage/current estimates.

### Common Integration Pattern

```c
// In message handler
if (uavcan_equipment_gnss_Fix_decode(transfer, &gnssFix) == 0) {
    // Successful decode
    dronecanGPSReceiveGNSSFix(&gnssFix);  // Hand off to GPS system
    LOG_DEBUG(CAN, "GNSS Fix received");
} else {
    // Decode error
    LOG_DEBUG(CAN, "GNSSFix decode failed");
}
```

---

## Adding New Message Types

To add support for a new DroneCAN message type:

### 1. Define DSDL Message Format

Add message definition to `src/main/drivers/dronecan/dronecan_msgs.h` or generate from DSDL compiler if not already present.

### 2. Create Handler Function

```c
void handle_NewMessage(CanardInstance *ins, CanardRxTransfer *transfer) {
    UNUSED(ins);
    struct uavcan_message_Type newMessage;

    if (uavcan_message_Type_decode(transfer, &newMessage)) {
        LOG_DEBUG(CAN, "NewMessage decode failed");
        return;
    }

    // Process message
    // Integrate with appropriate INAV subsystem
    LOG_DEBUG(CAN, "NewMessage received");
}
```

### 3. Register in `shouldAcceptTransfer()`

Add case to filter function:

```c
if (transfer_type == CanardTransferTypeBroadcast) {
    switch (data_type_id) {
        case UAVCAN_MESSAGE_TYPE_ID: {
            *out_data_type_signature = UAVCAN_MESSAGE_TYPE_SIGNATURE;
            return true;  // Accept this message
        }
    }
}
```

### 4. Register in `onTransferReceived()`

Add dispatcher entry:

```c
if (transfer->transfer_type == CanardTransferTypeBroadcast) {
    switch (transfer->data_type_id) {
        case UAVCAN_MESSAGE_TYPE_ID:
            handle_NewMessage(ins, transfer);
            break;
    }
}
```

### 5. Verify Message Constants

Ensure message type has constants defined:
- `UAVCAN_MESSAGE_TYPE_ID` - The data type ID for filtering
- `UAVCAN_MESSAGE_TYPE_SIGNATURE` - The DSDL signature for encoding/decoding

---

## CAN Bus Configuration

### Hardware Setup

The driver uses STM32 CAN1 interface. Requires:

**STM32F7xx:**
- CAN1 interface (PA11/PA12 or PB8/PB9 depending on board)
- CAN transceiver (e.g., TJA1050)
- 120Ω termination resistors at each end of CAN bus

**STM32H7xx:**
- Same as F7xx but verify pin assignments

### Bitrate Selection

Supported bitrates: 125, 250, 500, 1000 kbps

**Recommended:** 500 kbps (good compromise between bandwidth and noise immunity)

### Bus Termination

CAN bus must have 120Ω termination at each end. Without proper termination:
- Messages fail to decode
- CAN bus enters "bus off" state
- Driver cannot recover

### Transceiver Requirements

- Logic level: 3.3V (STM32 native)
- Bus: Standard ISO CAN
- Examples: TJA1050, MCP2551, SN65HVD230

---

## Troubleshooting

### CAN Bus Not Initializing

**Symptoms:** DroneCAN messages not received, no CAN activity
**Possible Causes:**
- Node ID = 0 (required > 0 to transmit)
- CAN hardware not enabled in firmware build
- CAN transceiver not powered
- CAN bus wiring incorrect

**Debug Steps:**
1. Verify `dronecan_node_id` setting is > 0
2. Check CAN bus voltage (should be ~2.5V differential)
3. Verify CAN transceiver power and wiring
4. Check STM32 pin configuration for CAN1

### Bus-Off / Frequent Recovery

**Symptoms:** CAN bus repeatedly enters bus-off and recovers
**Possible Causes:**
- CAN transceiver not powered or damaged
- CAN bus impedance mismatch (missing 120Ω terminators)
- Bit timing configuration error
- CAN message collision/overload

**Debug Steps:**
1. Check CAN bus voltage with oscilloscope
2. Verify 120Ω termination resistors installed
3. Check for electrical noise on CAN bus
4. Reduce CAN message rate (GPS update rate, etc.)

### GPS Messages Not Received

**Symptoms:** DroneCAN GPS available in settings but no fix received
**Possible Causes:**
- GPS DroneCAN node not on bus or not transmitting
- GNSS_FIX_ID message not being sent by GPS node
- GPS provider not selected in INAV settings
- GPS messages filtered out (shouldn't happen with default config)

**Debug Steps:**
1. Verify GPS node is powered and appears on CAN bus
2. Check GPS node is sending at correct data type ID
3. Verify `gps_provider` setting includes DroneCAN option
4. Monitor CAN traffic for GNSS_FIX_ID frames (should arrive at ~10 Hz)

### Battery Info Not Received

**Symptoms:** Battery sensor shows no data from DroneCAN source
**Possible Causes:**
- Battery sensor DroneCAN node not on bus
- Battery node using wrong message type
- Battery messages not being received/decoded

**Debug Steps:**
1. Verify battery sensor node is powered
2. Check battery node transmits at correct data type ID
3. Monitor CAN traffic for BATTERY_INFO frames

---

## Error Recovery and Graceful Disable

### Safe Initialization

The DroneCAN driver initialization sequence in `dronecanInit()` and `dronecanSTM32Initializexx()` has been designed with failure safety in mind:

**Initialization Order:**
1. Memory pool allocation
2. CAN hardware setup (GPIO, clock, HAL initialization)
3. CAN filter configuration
4. RX message queue setup
5. **Interrupt enable** ← Moved to END of init sequence

**Critical Safety Fix:** The CAN interrupt (e.g., `CAN1_RX0_IRQn`) is enabled ONLY after all initialization steps complete successfully. If any step fails (steps 1-4), the function returns an error code without enabling interrupts, preventing spurious interrupts on unconfigured hardware.

### Graceful Disable Behavior

If DroneCAN is disabled via CLI (`set dronecan_enabled = 0`), the driver cleanly disables the CAN interface:

```c
void dronecanDisable(void) {
    // Disable interrupt first (prevents spurious interrupts)
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);

    // Clean up TX queue (discard pending messages)
    while (canardPeekTxQueue(&canard) != NULL) {
        canardPopTxQueue(&canard);
    }

    // Stop CAN hardware (enters sleep mode)
    HAL_CAN_Stop(&hcan1);

    // Mark as disabled
    dronecanInitialized = false;
}
```

**Benefits:**
- No messages stuck in TX queue
- No sporadic CAN activity after disable
- CAN bus remains stable for other potential users
- Can re-enable without hardware issues

### Error Recovery

The driver implements automatic error recovery for common CAN bus faults:

#### Bus-Off Recovery

When the CAN bus enters "bus-off" state (due to excessive errors), the STM32 CAN controller automatically recovers after transmitting 128 error frames. The driver detects this:

```c
void dronecanUpdate(void) {
    // Check for bus-off condition
    if (hcan1.State == HAL_CAN_STATE_BUS_OFF) {
        // Bus-off detected - wait for automatic recovery
        // No manual intervention needed
        LOG_INFO(CAN, "Bus-off detected, recovering...");
    }
}
```

**Typical Causes:**
- Wiring problems (CAN_H/CAN_L swapped)
- Termination resistor issues
- Transceiver failures
- High electrical noise

**Prevention:**
1. Verify CAN bus wiring and termination
2. Check transceiver power supply (3.3V)
3. Reduce message rates if experiencing high error rates
4. Electrically isolate CAN bus from noisy power supplies

#### Incomplete Initialization Handling

If hardware initialization fails partway through (e.g., GPIO allocation fails, clock not enabled), the function safely aborts:

```c
int8_t dronecanInit(...) {
    // Setup step 1
    if (setupGPIO() != 0) {
        return DRONECAN_INIT_FAILED;  // Abort - no interrupt enabled
    }

    // Setup step 2
    if (setupHALCAN() != 0) {
        return DRONECAN_INIT_FAILED;  // Abort - no interrupt enabled
    }

    // ... more setup steps ...

    // Enable interrupt ONLY after all steps succeed
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);

    return DRONECAN_INIT_OK;
}
```

**Why This Matters:**
- If enabled early, interrupt fires on unconfigured hardware → crash
- If enabled late (after failure), flight controller continues safely
- Previous versions had race condition → now fixed

#### Runtime Error Handling

The `dronecanUpdate()` function handles runtime errors gracefully:

```c
void dronecanUpdate(void) {
    if (!dronecanInitialized) {
        return;  // Not initialized, skip
    }

    // Process RX messages
    canardUpdate(&canard);  // May encounter decode errors

    // Process TX queue
    processCanardTxQueue();

    // If bus goes offline, driver continues
    // Waits for automatic recovery or manual restart
}
```

**Fault Tolerance:**
- Single message decode error doesn't affect others
- TX queue backpressure handled (graceful wait)
- Bus-off state doesn't crash flight controller
- Can survive transceiver power loss and recovery

### Node Status Broadcasting

The driver broadcasts periodic node status to other CAN nodes, enabling network monitoring:

```c
void broadcastNodeStatus(void) {
    struct uavcan_protocol_NodeStatus status;
    status.uptime_sec = ++uptime;
    status.health = getHealthStatus();
    status.mode = UAVCAN_PROTOCOL_NODESTATUS_MODE_OPERATIONAL;

    // Broadcast every 1 second
    canardBroadcast(&canard,
                    UAVCAN_PROTOCOL_NODESTATUS_SIGNATURE,
                    UAVCAN_PROTOCOL_NODESTATUS_ID,
                    &transfer_id,
                    CANARD_TRANSFER_PRIORITY_LOW,
                    buffer,
                    len);
}
```

**Enables:**
- Network topology visualization
- Health monitoring of flight controller
- DroneCAN GUI integration
- Future dynamic node ID assignment

---

## References

### UAVCAN Protocol Resources

- [UAVCAN Specification](https://uavcan.org/)
- [DroneCAN Implementation](https://dronecan.github.io/)
- [libcanard Library](https://github.com/dronecan/libcanard)

### Related INAV Documentation

- **CAN Hardware Setup:** See board target configuration (e.g., `src/main/target/*/target.h`)
- **GPS Provider:** `src/main/io/gps_dronecan.c`
- **Battery Sensor:** `src/main/sensors/battery_sensor_dronecan.c`
- **Settings Reference:** `src/main/fc/settings.yaml` - search for `dronecan`

### Code Locations

**Driver Implementation:**
- `src/main/drivers/dronecan/dronecan.c` - Main driver (512 lines)
- `src/main/drivers/dronecan/dronecan.h` - Public API and configuration struct

**Message Codec Generated Files:**
- `src/main/drivers/dronecan/dronecan_msgs.h` - Message definitions and codecs
- Generated by DSDL compiler from UAVCAN specification

**Integration:**
- `src/main/io/gps_dronecan.c` - GPS provider integration
- `src/main/sensors/battery_sensor_dronecan.c` - Battery sensor integration

**Tests:**
- `src/test/unit/dronecan/*` - Unit tests for message handlers and integration

---

## Document History

| Date | Version | Changes |
|---|---|---|
| 2026-02-18 | 1.1 | Added error recovery, graceful disable behavior, and safe initialization documentation |
| 2026-02-16 | 1.0 | Initial version - handler-based architecture documentation |

---

## Questions & Feedback

For questions about this documentation or to report inaccuracies, contact the development team.

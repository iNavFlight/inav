# INAV JavaScript Programming - Quick Reference

## Relationship to Logic Conditions

This JavaScript programming interface is built on top of INAV's traditional
[Logic Conditions](../../Programming%20Framework.md) system. The JavaScript code you
write is transpiled (converted) into logic conditions that run on the flight controller.

If you're familiar with the traditional logic conditions interface, you can think of
JavaScript programming as a more user-friendly syntax that generates the same logic
conditions behind the scenes.

**See also:**
- [Programming Framework documentation](../../Programming%20Framework.md) - Details about the underlying logic conditions system
- [Operations Reference](OPERATIONS_REFERENCE.md) - Complete reference for all supported operations

---

## Pattern Guide

### Continuous Conditions (if statements)
Use `if` statements for conditions that should check and execute **every cycle**:

```javascript
const { flight, override } = inav;

// Checks every cycle - adjusts VTX power continuously
if (flight.homeDistance > 100) {
  override.vtx.power = 3;
}
```

**Use when:** You want the action to happen continuously while the condition is true.

---

### One-Time Execution (edge)
Use `edge()` for actions that should execute **only once** when a condition becomes true:

```javascript
const { flight, gvar, edge } = inav;

// Executes ONCE when armTimer reaches 1000ms
edge(() => flight.armTimer > 1000, { duration: 0 }, () => {
  gvar[0] = flight.yaw;  // Save initial heading
  gvar[1] = 0;           // Initialize counter
});
```

**Parameters:**
- **condition**: Function returning boolean
- **duration**: Minimum duration in ms (0 = instant, >0 = debounce)
- **action**: Function to execute once

**Use when:** 
- Initializing on arm
- Detecting events (first time RSSI drops)
- Counting discrete occurrences
- Debouncing noisy signals

---

### Latching/Sticky Conditions
Use `sticky()` for conditions that latch ON and stay ON until reset.

**Option 1: Variable assignment syntax** (recommended when you need to reference the latch state):

```javascript
const { flight, gvar, sticky, override } = inav;

// Create a latch: ON when RSSI < 30, OFF when RSSI > 70
var rssiWarning = sticky({
  on: () => flight.rssi < 30,
  off: () => flight.rssi > 70
});

// Use the latch to control actions
if (rssiWarning) {
  override.vtx.power = 4;  // Max power while latched
}
```

**Option 2: Callback syntax** (simpler when actions are self-contained):

```javascript
const { flight, sticky, override } = inav;

// Latch ON when RSSI < 30, OFF when RSSI > 70
sticky(
  () => flight.rssi < 30,  // ON condition
  () => flight.rssi > 70,  // OFF condition
  () => {
    override.vtx.power = 4;  // Executes while latched
  }
);
```

**Parameters (variable syntax):**
- **on**: Condition that latches ON
- **off**: Condition that latches OFF

**Parameters (callback syntax):**
- **onCondition**: When to latch ON
- **offCondition**: When to latch OFF
- **action**: What to do while latched

**Use when:**
- Warning states that need manual reset
- Hysteresis/deadband behavior
- Failsafe conditions

---

### Delayed Execution (delay)
Use `delay()` to execute after a condition has been true for a duration:

```javascript
const { flight, gvar, delay } = inav;

// Executes only if RSSI < 30 for 2 seconds continuously
delay(() => flight.rssi < 30, { duration: 2000 }, () => {
  gvar[0] = 1;  // Set failsafe flag
});
```

**Parameters:**
- **condition**: Condition that must remain true
- **duration**: How long condition must be true (ms)
- **action**: Action to execute after delay

**Use when:**
- Avoiding false triggers
- Requiring sustained conditions
- Timeouts and delays

---

## Common Patterns

### Initialize on Arm
```javascript
const { flight, gvar, edge } = inav;

edge(() => flight.armTimer > 1000, { duration: 0 }, () => {
  gvar[0] = 0;              // Reset counter
  gvar[1] = flight.yaw;     // Save heading
  gvar[2] = flight.altitude; // Save starting altitude
});
```

### Count Events
```javascript
const { flight, gvar, edge } = inav;

// Initialize
edge(() => flight.armTimer > 1000, { duration: 0 }, () => {
  gvar[0] = 0;
});

// Count each time RSSI drops below 30 (counts transitions, not duration)
edge(() => flight.rssi < 30, { duration: 100 }, () => {
  gvar[0] = gvar[0] + 1;
});
```

### Debounce Noisy Signals
```javascript
const { flight, override, edge } = inav;

// Only trigger if RSSI < 30 for at least 500ms
edge(() => flight.rssi < 30, { duration: 500 }, () => {
  override.vtx.power = 4;
});
```

### Multi-Stage Logic
```javascript
const { flight, override } = inav;

// Stage 1: Far away
if (flight.homeDistance > 500) {
  override.vtx.power = 4;
}

// Stage 2: Medium distance  
if (flight.homeDistance > 200 && flight.homeDistance <= 500) {
  override.vtx.power = 3;
}

// Stage 3: Close to home
if (flight.homeDistance <= 200) {
  override.vtx.power = 2;
}
```

### Hysteresis/Deadband
```javascript
const { flight, gvar, sticky, override } = inav;

// Turn ON at low voltage, turn OFF when recovered
var lowVoltageWarning = sticky({
  on: () => flight.cellVoltage < 330,   // Warning threshold
  off: () => flight.cellVoltage > 350   // Recovery threshold
});

if (lowVoltageWarning) {
  override.throttleScale = 50;   // Reduce throttle while in warning
  gvar[0] = 1;                   // Warning flag
}
```

---

## Key Differences

| Pattern | Executes | Reset | Use Case |
|---------|----------|-------|----------|
| `if` | Every cycle while true | N/A | Continuous control |
| `edge()` | Once per transition | When condition becomes false | Events, initialization |
| `sticky()` | Continuous while latched | When OFF condition met | Warnings, hysteresis |
| `delay()` | Once after duration | When condition becomes false | Timeouts, debouncing |

---

## Available Objects

```javascript
const {
  flight,      // Flight telemetry (including flight.mode.*)
  override,    // Override flight parameters
  rc,          // RC channels
  gvar,        // Global variables (0-7)
  pid,         // Programming PID outputs (pid[0-3].output)
  waypoint,    // Waypoint navigation
  edge,        // Edge detection
  sticky,      // Latching conditions
  delay        // Delayed execution
} = inav;
```

### Flight Mode Detection

Check which flight modes are currently active via `flight.mode.*`:

```javascript
const { flight, gvar, override } = inav;

if (flight.mode.poshold === 1) {
  gvar[0] = 1;  // Flag: in position hold
}

if (flight.mode.rth === 1) {
  override.vtx.power = 4;  // Max power during RTH
}
```

**Available modes:** `failsafe`, `manual`, `rth`, `poshold`, `cruise`, `althold`, `angle`, `horizon`, `air`, `acro`, `courseHold`, `waypointMission`, `user1` through `user4`

### PID Controller Outputs

Read output values from the 4 programming PID controllers (configured in Programming PID tab):

```javascript
const { pid, gvar, override } = inav;

if (pid[0].output > 500) {
  override.throttle = 1600;
}

gvar[0] = pid[0].output;  // Store for OSD display
```

**Available:** `pid[0].output` through `pid[3].output`

---

## Tips

1. **Initialize variables on arm** using `edge()` with `flight.armTimer > 1000`
2. **Use gvars for state** - they persist between logic condition evaluations
3. **edge() duration = 0** means instant trigger on condition becoming true
4. **edge() duration > 0** adds debounce time
5. **if statements are continuous** - they execute every cycle
6. **sticky() provides hysteresis** - prevents rapid ON/OFF switching
7. **Use Math functions** - `Math.abs()`, `Math.min()`, `Math.max()` are available

---

## Debugging

Use global variables to track state:
```javascript
const { flight, gvar, edge } = inav;

// Debug counter
edge(() => flight.armTimer > 1000, { duration: 0 }, () => {
  gvar[7] = 0; // Use gvar[7] as debug counter
});

// Increment on each event
edge(() => flight.rssi < 30, { duration: 0 }, () => {
  gvar[7] = gvar[7] + 1;
});

// Check gvar[7] value in OSD or Configurator to see event count
```

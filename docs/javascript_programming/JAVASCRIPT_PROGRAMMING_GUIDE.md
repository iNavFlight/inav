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

// Checks every cycle - adjusts VTX power continuously
if (inav.flight.homeDistance > 100) {
  inav.override.vtx.power = 3;
}

```

**Use when:** You want the action to happen continuously while the condition is true.

---

### One-Time Execution (edge)
Use `edge()` for actions that should execute **only once** when a condition becomes true:

```javascript

// Executes ONCE when armTimer reaches 1000ms
inav.events.edge(() => inav.flight.armTimer > 1000, { duration: 0 }, () => {
  inav.gvar[0] = inav.flight.yaw;  // Save initial heading
  inav.gvar[1] = 0;           // Initialize counter
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
Use `sticky()` for conditions that latch ON and stay ON until reset:

```javascript

// Latches ON when RSSI < 30, stays ON until RSSI > 70
inav.events.sticky(
  () => inav.flight.rssi < 30,  // ON condition
  () => inav.flight.rssi > 70,  // OFF condition  
  () => {
    inav.override.vtx.power = 4;  // Executes while latched
  }
);

```

**Parameters:**
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

// Executes only if RSSI < 30 for 2 seconds continuously
inav.events.delay(() => inav.flight.rssi < 30, { duration: 2000 }, () => {
  inav.gvar[0] = 1;  // Set failsafe flag
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

inav.events.edge(() => inav.flight.armTimer > 1000, { duration: 0 }, () => {
  inav.gvar[0] = 0;              // Reset counter
  inav.gvar[1] = inav.flight.yaw;     // Save heading
  inav.gvar[2] = inav.flight.altitude; // Save starting altitude
});

```

### Count Events
```javascript

// Initialize
inav.events.edge(() => inav.flight.armTimer > 1000, { duration: 0 }, () => {
  inav.gvar[0] = 0;
});

// Count each time RSSI drops below 30 (counts transitions, not duration)
inav.events.edge(() => inav.flight.rssi < 30, { duration: 100 }, () => {
  inav.gvar[0] = inav.gvar[0] + 1;
});

```

### Debounce Noisy Signals
```javascript

// Only trigger if RSSI < 30 for at least 500ms
inav.events.edge(() => inav.flight.rssi < 30, { duration: 500 }, () => {
  inav.override.vtx.power = 4;
});

```

### Multi-Stage Logic
```javascript

// Stage 1: Far away
if (inav.flight.homeDistance > 500) {
  inav.override.vtx.power = 4;
}

// Stage 2: Medium distance  
if (inav.flight.homeDistance > 200 && inav.flight.homeDistance <= 500) {
  inav.override.vtx.power = 3;
}

// Stage 3: Close to home
if (inav.flight.homeDistance <= 200) {
  inav.override.vtx.power = 2;
}

```

### Hysteresis/Deadband
```javascript

// Turn ON at low voltage, turn OFF when recovered
inav.events.sticky(
  () => inav.flight.cellVoltage < 330,  // Warning threshold
  () => inav.flight.cellVoltage > 350,  // Recovery threshold
  () => {
    inav.override.throttleScale = 50;   // Reduce throttle while in warning
    inav.gvar[0] = 1;                   // Warning flag
  }
);

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

## Variables

### Let/Const Variables

Use `let` or `const` to define reusable expressions that are compiled into the logic:

```javascript
// Define reusable calculations
let distanceThreshold = 500;
let altitudeLimit = 100;
let combinedCondition = inav.flight.homeDistance > distanceThreshold && inav.flight.altitude > altitudeLimit;

// Use in conditions
if (combinedCondition) {
  inav.override.vtx.power = 4;
}
```

**Benefits:**
- Makes code more readable with named values
- Compiler automatically optimizes duplicate expressions
- Variables preserve their custom names through compile/decompile cycles

**Important:** `let`/`const` variables are **compile-time substituted**, not runtime variables. For runtime state, use `inav.gvar[]`.

### Ternary Operator

Use ternary expressions for conditional values:

```javascript
// Assign based on condition
let throttleLimit = inav.flight.cellVoltage < 330 ? 25 : 50;

if (inav.flight.cellVoltage < 350) {
  inav.override.throttleScale = throttleLimit;
}

// Inline in expressions
inav.override.vtx.power = inav.flight.homeDistance > 500 ? 4 : 2;
```

**Use when:** You need conditional value assignment in a single expression.

---

## Available Objects

The `inav` namespace provides access to all flight controller data and control functions:

- `inav.flight` - Flight telemetry (including `flight.mode.*`)
- `inav.override` - Override flight parameters
- `inav.rc` - RC channels
- `inav.gvar` - Global variables (0-7)
- `inav.pid` - Programming PID outputs (`pid[0-3].output`)
- `inav.waypoint` - Waypoint navigation
- `inav.events.edge` - Edge detection
- `inav.events.sticky` - Latching conditions
- `inav.events.delay` - Delayed execution

### Flight Mode Detection

Check which flight modes are currently active via `inav.flight.mode.*`:

```javascript
if (inav.flight.mode.poshold === 1) {
  inav.gvar[0] = 1;  // Flag: in position hold
}

if (inav.flight.mode.rth === 1) {
  inav.override.vtx.power = 4;  // Max power during RTH
}
```

**Available modes:** `failsafe`, `manual`, `rth`, `poshold`, `cruise`, `althold`, `angle`, `horizon`, `air`, `acro`, `courseHold`, `waypointMission`, `user1` through `user4`

### PID Controller Outputs

Read output values from the 4 programming PID controllers (configured in Programming PID tab):

```javascript
if (inav.pid[0].output > 500) {
  inav.override.throttle = 1600;
}

inav.gvar[0] = inav.pid[0].output;  // Store for OSD display
```

**Available:** `inav.pid[0].output` through `inav.pid[3].output`

---

## Tips

1. **Initialize variables on arm** using `inav.events.edge()` with `inav.flight.armTimer > 1000`
2. **Use inav.gvar for state** - they persist between logic condition evaluations
3. **edge() duration = 0** means instant trigger on condition becoming true
4. **edge() duration > 0** adds debounce time
5. **if statements are continuous** - they execute every cycle
6. **sticky() provides hysteresis** - prevents rapid ON/OFF switching
7. **Use Math functions** - `Math.abs()`, `Math.min()`, `Math.max()` are available

---

## Debugging

Use global variables to track state:
```javascript

// Debug counter
inav.events.edge(() => inav.flight.armTimer > 1000, { duration: 0 }, () => {
  inav.gvar[7] = 0; // Use inav.gvar[7] as debug counter
});

// Increment on each event
inav.events.edge(() => inav.flight.rssi < 30, { duration: 0 }, () => {
  inav.gvar[7] = inav.gvar[7] + 1;
});

// Check inav.gvar[7] value in OSD or Configurator to see event count

```

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

## Available Objects

```javascript
const { 
  flight,      // Flight telemetry
  override,    // Override flight parameters
  rc,          // RC channels
  gvar,        // Global variables (0-7)
  waypoint,    // Waypoint navigation
  edge,        // Edge detection
  sticky,      // Latching conditions
  delay        // Delayed execution
} = inav;

```

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

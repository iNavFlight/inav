# timer() and whenChanged() Examples

## timer() Examples

### Example 1: Periodic VTX Power Boost
```javascript

// Boost VTX power to maximum for 1 second every 5 seconds
inav.events.timer(1000, 5000, () => {
  inav.override.vtx.power = 4;
});

```

### Example 2: Flashing OSD Layout
```javascript

// Alternate between OSD layout 0 and 1 every second
inav.events.timer(1000, 1000, () => {
  inav.override.osdLayout = 1;
});

```

### Example 3: Periodic Status Check
```javascript

// Record battery voltage every 10 seconds for 100ms
inav.events.timer(100, 10000, () => {
  inav.gvar[0] = inav.flight.vbat;
  inav.gvar[1] = inav.flight.current;
});

```

### Example 4: Warning Beep Pattern
```javascript

// Beep pattern: 200ms on, 200ms off, 200ms on, 5s off
// (Would need multiple timers or different approach for complex patterns)
inav.events.timer(200, 200, () => {
  inav.override.rcChannel(8, 2000); // Beeper channel
});

```

## whenChanged() Examples

### Example 1: Altitude Change Logger
```javascript

// Log altitude whenever it changes by 50cm or more
inav.events.whenChanged(inav.flight.altitude, 50, () => {
  inav.gvar[0] = inav.flight.altitude;
});

```

### Example 2: RSSI Drop Detection
```javascript

// Boost VTX power when RSSI drops by 10 or more
inav.events.whenChanged(inav.flight.rssi, 10, () => {
  if (inav.flight.rssi < 50) {
    inav.override.vtx.power = 4;
  }
});

```

### Example 3: Speed Change Tracker
```javascript

// Track ground speed changes of 100cm/s or more
inav.events.whenChanged(inav.flight.groundSpeed, 100, () => {
  inav.gvar[1] = inav.flight.groundSpeed;
});

```

### Example 4: Battery Voltage Monitor
```javascript

// Record voltage whenever it changes by 1 unit (0.1V)
inav.events.whenChanged(inav.flight.vbat, 1, () => {
  inav.gvar[2] = inav.flight.vbat;
  inav.gvar[3] = inav.flight.mahDrawn;
});

```

### Example 5: Climb Rate Detection
```javascript

// Detect rapid climbs (>50cm/s change in vertical speed)
inav.events.whenChanged(inav.flight.verticalSpeed, 50, () => {
  if (inav.flight.verticalSpeed > 200) {
    // Climbing fast - reduce throttle scale
    inav.override.throttleScale = 80;
  }
});

```

## Combined Examples

### Example 1: Timer + WhenChanged
```javascript

// Periodic VTX boost
inav.events.timer(1000, 5000, () => {
  inav.override.vtx.power = 4;
});

// Log altitude changes
inav.events.whenChanged(inav.flight.altitude, 100, () => {
  inav.gvar[0] = inav.flight.altitude;
});

```

### Example 2: Conditional Timer
```javascript

// Only run timer when armed
if (inav.flight.isArmed) {
  inav.events.timer(500, 500, () => {
    inav.override.osdLayout = 2;
  });
}

```

### Example 3: Emergency Response System
```javascript

// Monitor RSSI drops
inav.events.whenChanged(inav.flight.rssi, 5, () => {
  if (inav.flight.rssi < 30) {
    // RSSI critical - max VTX power
    inav.override.vtx.power = 4;
    inav.gvar[7] = 1; // Set emergency flag
  }
});

// Monitor altitude changes
inav.events.whenChanged(inav.flight.altitude, 200, () => {
  if (inav.flight.altitude > 10000) {
    // Too high - warn
    inav.gvar[6] = inav.flight.altitude;
  }
});

```

### Example 4: Data Logging System
```javascript

// Periodic logging every 5 seconds
inav.events.timer(100, 5000, () => {
  inav.gvar[0] = inav.flight.vbat;
  inav.gvar[1] = inav.flight.current;
  inav.gvar[2] = inav.flight.altitude;
});

// Event-based logging on significant changes
inav.events.whenChanged(inav.flight.altitude, 500, () => {
  inav.gvar[3] = inav.flight.altitude;
  inav.gvar[4] = inav.flight.verticalSpeed;
});

inav.events.whenChanged(inav.flight.groundSpeed, 200, () => {
  inav.gvar[5] = inav.flight.groundSpeed;
});

```

## Logic Condition Output Examples

### timer() Output
```javascript
inav.events.timer(1000, 2000, () => {
  inav.gvar[0] = 1;
});

```
Generates:
```
logic 0 1 -1 49 0 1000 0 2000 0    # TIMER: 1000ms ON, 2000ms OFF
logic 1 1 0 18 0 0 0 0 1 0         # gvar[0] = 1
```

### whenChanged() Output
```javascript
inav.events.whenChanged(inav.flight.altitude, 50, () => {
  inav.gvar[0] = inav.flight.altitude;
});

```
Generates:
```
logic 0 1 -1 50 2 12 0 50 0        # DELTA: altitude, threshold 50
logic 1 1 0 18 0 0 2 12 0          # gvar[0] = altitude
```

## Use Cases

### timer() Use Cases
1. **Periodic Tasks** - Execute actions at regular intervals
2. **Flashing Indicators** - Toggle states on/off
3. **Sampling** - Collect data periodically
4. **Timeouts** - Implement time-based state machines
5. **Warning Systems** - Beep or flash warnings

### whenChanged() Use Cases
1. **Event Detection** - React to significant changes
2. **Data Logging** - Record values when they change
3. **Thresholds** - Trigger actions on large changes
4. **Rate Limiting** - Avoid excessive updates (100ms window)
5. **Monitoring** - Track important parameter changes

## Best Practices

### timer()
- ✅ Use for periodic, time-based actions
- ✅ Keep durations reasonable (>100ms)
- ✅ Avoid very short ON times (may miss cycles)
- ❌ Don't use for one-time delays (use delay() instead)
- ❌ Don't nest timers (creates complex behavior)

### whenChanged()
- ✅ Use for event-driven responses
- ✅ Set thresholds appropriate to your data
- ✅ Monitor slowly-changing values
- ❌ Don't use for rapidly-changing values (may trigger constantly)
- ❌ Don't set threshold too low (noise will trigger it)
- ⚠️ Remember: 100ms detection window (built into DELTA operation)

## Common Patterns

### Pattern 1: Status Monitor
```javascript
// Log key parameters when they change significantly
inav.events.whenChanged(inav.flight.vbat, 2, () => { inav.gvar[0] = inav.flight.vbat; });
inav.events.whenChanged(inav.flight.rssi, 10, () => { inav.gvar[1] = inav.flight.rssi; });
inav.events.whenChanged(inav.flight.altitude, 100, () => { inav.gvar[2] = inav.flight.altitude; });

```

### Pattern 2: Periodic Beacon
```javascript
// Boost VTX power briefly every 10 seconds
inav.events.timer(500, 10000, () => {
  inav.override.vtx.power = 4;
});

```

### Pattern 3: Adaptive Response
```javascript
// React to rapid altitude changes
inav.events.whenChanged(inav.flight.altitude, 200, () => {
  if (inav.flight.altitude < 1000) {
    inav.override.throttleScale = 120; // Boost
  } else {
    inav.override.throttleScale = 80; // Reduce
  }
});

```

## Troubleshooting

### timer() Issues
**Problem:** Timer doesn't seem to trigger
- Check that ON duration > 0
- Check that OFF duration > 0
- Verify actions are valid

**Problem:** Timer seems irregular
- INAV runs at different rates depending on mode
- Timer precision is limited by LC execution rate

### whenChanged() Issues
**Problem:** Never triggers
- Check threshold isn't too high
- Verify value actually changes
- Remember: 100ms detection window

**Problem:** Triggers too often
- Increase threshold
- Value may be noisy
- Consider using timer() for periodic sampling instead

**Problem:** Doesn't track fast changes
- Built-in 100ms window
- Use timer() for faster sampling if needed

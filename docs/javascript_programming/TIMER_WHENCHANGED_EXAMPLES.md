# timer() and whenChanged() Examples

## timer() Examples

### Example 1: Periodic VTX Power Boost
```javascript
const { override, timer } = inav;

// Boost VTX power to maximum for 1 second every 5 seconds
timer(1000, 5000, () => {
  override.vtx.power = 4;
});
```

### Example 2: Flashing OSD Layout
```javascript
const { override, timer } = inav;

// Alternate between OSD layout 0 and 1 every second
timer(1000, 1000, () => {
  override.osdLayout = 1;
});
```

### Example 3: Periodic Status Check
```javascript
const { flight, gvar, timer } = inav;

// Record battery voltage every 10 seconds for 100ms
timer(100, 10000, () => {
  gvar[0] = flight.vbat;
  gvar[1] = flight.current;
});
```

### Example 4: Warning Beep Pattern
```javascript
const { override, timer } = inav;

// Beep pattern: 200ms on, 200ms off, 200ms on, 5s off
// (Would need multiple timers or different approach for complex patterns)
timer(200, 200, () => {
  override.rcChannel(8, 2000); // Beeper channel
});
```

## whenChanged() Examples

### Example 1: Altitude Change Logger
```javascript
const { flight, gvar, whenChanged } = inav;

// Log altitude whenever it changes by 50cm or more
whenChanged(flight.altitude, 50, () => {
  gvar[0] = flight.altitude;
});
```

### Example 2: RSSI Drop Detection
```javascript
const { flight, override, whenChanged } = inav;

// Boost VTX power when RSSI drops by 10 or more
whenChanged(flight.rssi, 10, () => {
  if (flight.rssi < 50) {
    override.vtx.power = 4;
  }
});
```

### Example 3: Speed Change Tracker
```javascript
const { flight, gvar, whenChanged } = inav;

// Track ground speed changes of 100cm/s or more
whenChanged(flight.groundSpeed, 100, () => {
  gvar[1] = flight.groundSpeed;
});
```

### Example 4: Battery Voltage Monitor
```javascript
const { flight, gvar, whenChanged } = inav;

// Record voltage whenever it changes by 1 unit (0.1V)
whenChanged(flight.vbat, 1, () => {
  gvar[2] = flight.vbat;
  gvar[3] = flight.mahDrawn;
});
```

### Example 5: Climb Rate Detection
```javascript
const { flight, override, whenChanged } = inav;

// Detect rapid climbs (>50cm/s change in vertical speed)
whenChanged(flight.verticalSpeed, 50, () => {
  if (flight.verticalSpeed > 200) {
    // Climbing fast - reduce throttle scale
    override.throttleScale = 80;
  }
});
```

## Combined Examples

### Example 1: Timer + WhenChanged
```javascript
const { flight, override, gvar, timer, whenChanged } = inav;

// Periodic VTX boost
timer(1000, 5000, () => {
  override.vtx.power = 4;
});

// Log altitude changes
whenChanged(flight.altitude, 100, () => {
  gvar[0] = flight.altitude;
});
```

### Example 2: Conditional Timer
```javascript
const { flight, override, timer } = inav;

// Only run timer when armed
if (flight.isArmed) {
  timer(500, 500, () => {
    override.osdLayout = 2;
  });
}
```

### Example 3: Emergency Response System
```javascript
const { flight, override, gvar, whenChanged } = inav;

// Monitor RSSI drops
whenChanged(flight.rssi, 5, () => {
  if (flight.rssi < 30) {
    // RSSI critical - max VTX power
    override.vtx.power = 4;
    gvar[7] = 1; // Set emergency flag
  }
});

// Monitor altitude changes
whenChanged(flight.altitude, 200, () => {
  if (flight.altitude > 10000) {
    // Too high - warn
    gvar[6] = flight.altitude;
  }
});
```

### Example 4: Data Logging System
```javascript
const { flight, gvar, timer, whenChanged } = inav;

// Periodic logging every 5 seconds
timer(100, 5000, () => {
  gvar[0] = flight.vbat;
  gvar[1] = flight.current;
  gvar[2] = flight.altitude;
});

// Event-based logging on significant changes
whenChanged(flight.altitude, 500, () => {
  gvar[3] = flight.altitude;
  gvar[4] = flight.verticalSpeed;
});

whenChanged(flight.groundSpeed, 200, () => {
  gvar[5] = flight.groundSpeed;
});
```

## Logic Condition Output Examples

### timer() Output
```javascript
timer(1000, 2000, () => {
  gvar[0] = 1;
});
```
Generates:
```
logic 0 1 -1 49 0 1000 0 2000 0    # TIMER: 1000ms ON, 2000ms OFF
logic 1 1 0 18 0 0 0 0 1 0         # gvar[0] = 1
```

### whenChanged() Output
```javascript
whenChanged(flight.altitude, 50, () => {
  gvar[0] = flight.altitude;
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
whenChanged(flight.vbat, 2, () => { gvar[0] = flight.vbat; });
whenChanged(flight.rssi, 10, () => { gvar[1] = flight.rssi; });
whenChanged(flight.altitude, 100, () => { gvar[2] = flight.altitude; });
```

### Pattern 2: Periodic Beacon
```javascript
// Boost VTX power briefly every 10 seconds
timer(500, 10000, () => {
  override.vtx.power = 4;
});
```

### Pattern 3: Adaptive Response
```javascript
// React to rapid altitude changes
whenChanged(flight.altitude, 200, () => {
  if (flight.altitude < 1000) {
    override.throttleScale = 120; // Boost
  } else {
    override.throttleScale = 80; // Reduce
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

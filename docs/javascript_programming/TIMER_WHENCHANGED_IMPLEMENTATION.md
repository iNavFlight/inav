# timer() and whenChanged() Functions

## Overview

The `timer()` and `whenChanged()` functions provide advanced timing and change-detection capabilities for INAV logic programming.

## timer() Function

### Syntax
```javascript
inav.events.timer(onMs, offMs, () => {
  // actions
});

```

### Description
Execute actions on a periodic timer with on/off cycling. The action executes during the "on" period and pauses during the "off" period.

### Parameters
- `onMs`: Duration in milliseconds to run the action
- `offMs`: Duration in milliseconds to wait between executions
- `action`: Arrow function containing actions to execute during on-time

### Example
```javascript

// Flash VTX power: ON for 1 second, OFF for 2 seconds, repeat
inav.events.timer(1000, 2000, () => {
  inav.override.vtx.power = 4;
});

```

**Generated Logic Conditions:**
```
logic 0 1 -1 49 0 1000 0 2000 0    # TIMER: ON 1000ms, OFF 2000ms
logic 1 1 0 25 0 0 0 0 4 0         # Set VTX power = 4
```

### Validation Rules
- Requires exactly 3 arguments
- `onMs` must be numeric literal > 0
- `offMs` must be numeric literal > 0
- Third argument must be arrow function with actions

## whenChanged() Function

### Syntax
```javascript
inav.events.whenChanged(value, threshold, () => {
  // actions
});

```

### Description
Execute actions when a monitored value changes by more than the specified threshold. Uses a built-in 100ms detection window.

### Parameters
- `value`: Flight parameter or global variable to monitor
- `threshold`: Minimum change required to trigger (numeric literal)
- `action`: Arrow function containing actions to execute on change

### Example
```javascript

// Log altitude whenever it changes by 50cm or more
inav.events.whenChanged(inav.flight.altitude, 50, () => {
  inav.gvar[0] = inav.flight.altitude;
});

```

**Generated Logic Conditions:**
```
logic 0 1 -1 50 2 12 0 50 0        # DELTA: altitude, threshold 50
logic 1 1 0 18 0 0 2 12 0          # gvar[0] = altitude
```

### Validation Rules
- Requires exactly 3 arguments
- `value` must be a valid flight parameter or gvar
- `threshold` must be numeric literal > 0
- Third argument must be arrow function with actions

## Round-Trip Support

Both functions support perfect round-trip transpilation/decompilation:

### timer() Round-Trip
```javascript
// Original JavaScript
inav.events.timer(1000, 2000, () => { inav.gvar[0] = 1; });

// Transpiled to logic conditions
logic 0 1 -1 49 0 1000 0 2000 0
logic 1 1 0 18 0 0 0 0 1 0

// Decompiled back to JavaScript
inav.events.timer(1000, 2000, () => {
  inav.gvar[0] = 1;
});

```

### whenChanged() Round-Trip
```javascript
// Original JavaScript
inav.events.whenChanged(inav.flight.altitude, 50, () => { inav.gvar[0] = inav.flight.altitude; });

// Transpiled to logic conditions
logic 0 1 -1 50 2 12 0 50 0
logic 1 1 0 18 0 0 2 12 0

// Decompiled back to JavaScript
inav.events.whenChanged(inav.flight.altitude, 50, () => {
  inav.gvar[0] = inav.flight.altitude;
});

```

## API Definitions

Both functions are defined in `api/definitions/events.js`:

```javascript
timer: {
  type: 'function',
  desc: 'Execute action on a periodic timer (on/off cycling)',
  params: {
    onMs: { type: 'number', unit: 'ms', desc: 'Duration to run action' },
    offMs: { type: 'number', unit: 'ms', desc: 'Duration to wait between executions' },
    action: { type: 'function', desc: 'Action to execute during on-time' }
  },
  example: 'inav.events.timer(1000, 5000, () => { inav.override.vtx.power = 4; })'
},

whenChanged: {
  type: 'function',
  desc: 'Execute when value changes by more than threshold',
  params: {
    value: { type: 'number', desc: 'Value to monitor' },
    threshold: { type: 'number', desc: 'Change threshold' },
    action: { type: 'function', desc: 'Action to execute on change' }
  },
  example: 'inav.events.whenChanged(inav.flight.altitude, 100, () => { inav.gvar[0] = inav.flight.altitude; })'
}

```

## See Also

- **TIMER_WHENCHANGED_EXAMPLES.md** - More usage examples and patterns
- **JAVASCRIPT_PROGRAMMING_GUIDE.md** - Complete programming guide
- **events.js** - Full API definitions

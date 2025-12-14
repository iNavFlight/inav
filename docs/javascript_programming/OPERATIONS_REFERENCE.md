# INAV Logic Condition Operations Reference

This document provides a complete reference for all INAV logic condition operations and their JavaScript transpiler implementations.

## All INAV Operations Now Implemented! 🎉

All INAV logic condition operations supported by the firmware are now fully implemented in the JavaScript transpiler.

## Already Implemented

✅ **Arithmetic**: ADD, SUB, MUL, DIV, MODULUS (via +, -, *, /, %)
✅ **Comparisons**: EQUAL, GREATER_THAN, LOWER_THAN, APPROX_EQUAL (via ===, >, <, approxEqual())
✅ **Logical**: AND, OR, NOT, XOR, NAND, NOR (via &&, ||, !, xor(), nand(), nor())
✅ **Math**: MIN, MAX, SIN, COS, TAN, ACOS, ASIN, ATAN2, ABS (via Math.min/max/sin/cos/tan/acos/asin/atan2/abs)
✅ **Scaling**: MAP_INPUT, MAP_OUTPUT (via mapInput(), mapOutput())
✅ **Flow control**: STICKY, EDGE, DELAY, TIMER, DELTA (via on.* and helper functions)
✅ **Variables**: GVAR_SET, GVAR_INC, GVAR_DEC (via assignments, ++, --)
✅ **Overrides**: VTX, throttle, arming, etc. (via override.* properties)
✅ **RC Channel States**: LOW, MID, HIGH (via rc[n].low, rc[n].mid, rc[n].high)

## Usage Examples

### Logical Operations

```javascript
// XOR - true when exactly one condition is true
if (xor(flight.armed, flight.mode.failsafe)) {
  // One or the other, but not both
}

// NAND - false only when both are true
if (nand(flight.armed, gvar[0] > 100)) {
  // Not both conditions true
}

// NOR - true only when both are false
if (nor(flight.mode.failsafe, flight.mode.rth)) {
  // Neither failsafe nor RTH active
}

// Approximate equality with tolerance
if (approxEqual(flight.altitude, 1000, 50)) {
  // Altitude is 1000 ± 50
}
```

### Scaling/Mapping Operations

```javascript
// mapInput: Scale from [0:maxValue] to [0:1000]
// Example: RC value (1000-2000) to normalized (0-1000)
const normalizedThrottle = mapInput(rc[3].value - 1000, 1000);

// mapOutput: Scale from [0:1000] to [0:maxValue]
// Example: normalized (0-1000) to servo angle (0-180)
const servoAngle = mapOutput(normalizedThrottle, 180);

// Chaining for full range mapping
// Map altitude (0-5000m) to percentage (0-100)
const altitudePercent = mapOutput(mapInput(flight.altitude, 5000), 100);
```

### RC Channel State Detection

```javascript
// LOW state - RC value < 1333us
if (rc[0].low) {
  // Roll stick is in low position
  gvar[0] = 1;
}

// MID state - RC value between 1333-1666us
if (rc[1].mid) {
  // Pitch stick is centered
  gvar[1] = 1;
}

// HIGH state - RC value > 1666us
if (rc[2].high) {
  // Throttle stick is in high position
  override.vtx.power = 4;
}

// Access raw RC value
if (rc[3].value > 1700) {
  // Custom threshold on yaw channel
  gvar[2] = 1;
}

// Combined with logical operations
if (rc[0].low && rc[1].mid && rc[2].high) {
  // Specific stick combination detected
  override.armSafety = 1;
}
```

## Notes

- RC channels: `rc[0]` through `rc[17]` (18 channels)
- RC channel properties: `.value` (1000-2000us), `.low` (<1333us), `.mid` (1333-1666us), `.high` (>1666us)
- Trig functions: sin/cos/tan take degrees; acos/asin use ratios (-1..1) and atan2 returns degrees from y/x inputs
- MODULUS operator bug fixed: `%` now correctly generates MODULUS operation
- MAP_INPUT normalizes to [0:1000], MAP_OUTPUT scales from [0:1000]

## Recent Changes

**2025-11-25 (Session 3)**:
- ✅ Implemented RC channel LOW/MID/HIGH state detection (rc[n].low, rc[n].mid, rc[n].high)
- ✅ Fixed analyzer to validate RC channel array access patterns
- ✅ Updated codegen to generate LOW/MID/HIGH operations (operations 4, 5, 6)
- 🎉 ALL INAV logic condition operations now fully implemented and tested!

**2025-11-25 (Session 2)**:
- ✅ Implemented XOR, NAND, NOR logical operations (xor(), nand(), nor())
- ✅ Implemented APPROX_EQUAL comparison (approxEqual())
- ✅ Implemented MAP_INPUT and MAP_OUTPUT scaling (mapInput(), mapOutput())

**2025-11-25 (Session 1)**:
- ✅ Implemented Math.min(), Math.max(), Math.sin(), Math.cos(), Math.tan()
- ✅ Fixed MODULUS operator (was using wrong constant name)
- 📝 Documented unimplemented operations for future work

**Last Updated**: 2025-11-25

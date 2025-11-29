# INAV Constants Generator

## Purpose

Generates `inav_constants.js` from the INAV firmware header file (`logic_condition.h`) to ensure the transpiler/decompiler constants exactly match the actual firmware.

## Single Source of Truth Architecture

**Firmware Header** (`logic_condition.h`) 
  ↓ (parse)
**Constants File** (`inav_constants.js`) - AUTO-GENERATED
  ↓ (import)
**API Definitions** (`flight.js`, etc.) - Reference constants
  ↓ (use)
**Transpiler & Decompiler** - Use constants

## Usage

### Generate Constants

```bash
node scripts/generate-constants.js <path-to-logic_condition.h> [output-path]
```

### Example

```bash
# From INAV configurator root
node scripts/generate-constants.js \
  ../inav/src/main/programming/logic_condition.h \
  js/transpiler/transpiler/inav_constants.js
```

### Default Output

If output path is not specified, defaults to:
```
js/transpiler/transpiler/inav_constants.js
```

## What It Parses

The parser extracts these enums from `logic_condition.h`:

1. **logicOperation_e** → `OPERATION`
   - TRUE, EQUAL, GREATER_THAN, etc.
   
2. **logicOperandType_s** → `OPERAND_TYPE`
   - VALUE, RC_CHANNEL, FLIGHT, etc.
   
3. **logicFlightOperands_e** → `FLIGHT_PARAM`
   - ARM_TIMER, HOME_DISTANCE, RSSI, YAW, etc.
   
4. **logicFlightModeOperands_e** → `FLIGHT_MODE`
   - FAILSAFE, MANUAL, RTH, etc.
   
5. **logicWaypointOperands_e** → `WAYPOINT_PARAM`
   - IS_WP, WAYPOINT_INDEX, etc.

## Generated File Format

```javascript
/**
 * AUTO-GENERATED from firmware header files
 * DO NOT EDIT MANUALLY
 */

const OPERAND_TYPE = {
  VALUE: 0,
  RC_CHANNEL: 1,
  FLIGHT: 2,
  // ...
};

const OPERATION = {
  TRUE: 0,
  EQUAL: 1,
  // ...
};

const FLIGHT_PARAM = {
  ARM_TIMER: 0,
  HOME_DISTANCE: 1,
  // ...
  YAW: 40,  // ← Correct value from firmware!
  // ...
};

// ... exports
```

## Build Integration

Add to package.json scripts:

```json
{
  "scripts": {
    "generate-constants": "node scripts/generate-constants.js ../inav/src/main/programming/logic_condition.h",
    "prebuild": "npm run generate-constants"
  }
}
```

This ensures constants are regenerated before each build.

## Next Steps: Update API Definitions

Currently, API definitions have hardcoded values:

```javascript
// flight.js - WRONG (hardcoded)
yaw: {
  inavOperand: { type: 2, value: 17 }  // Wrong value!
}
```

Change to reference constants:

```javascript
// flight.js - CORRECT (references constants)
const { OPERAND_TYPE, FLIGHT_PARAM } = require('../../transpiler/inav_constants.js');

yaw: {
  inavOperand: { type: OPERAND_TYPE.FLIGHT, value: FLIGHT_PARAM.ATTITUDE_YAW }
}
```

Benefits:
- ✅ Single source of truth (firmware)
- ✅ Type-safe references
- ✅ Compile-time errors if constants missing
- ✅ Auto-update when firmware changes

## Verification

After generating, verify key values:

```bash
grep "ATTITUDE_YAW" js/transpiler/transpiler/inav_constants.js
# Should show: ATTITUDE_YAW: 40,

grep "IS_ARMED" js/transpiler/transpiler/inav_constants.js  
# Should show: IS_ARMED: 17,
```

## Error Handling

The parser handles:
- ✅ Both `typedef enum { } name_e;` and `typedef enum name { }` formats
- ✅ Explicit values (`= 40`)
- ✅ Auto-incrementing values
- ✅ Hex values (`= 0x10`)
- ✅ C-style comments (`//` and `/* */`)
- ✅ Missing enums (warns but continues)

## Maintenance

**When INAV firmware updates:**
1. Get new `logic_condition.h` from firmware repo
2. Run `npm run generate-constants`
3. Review diff in `inav_constants.js`
4. Test transpiler/decompiler
5. Commit updated constants file

**DO NOT manually edit `inav_constants.js`** - all changes will be overwritten!

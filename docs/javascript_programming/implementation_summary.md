# INAV JavaScript Transpiler - Technical Overview

## Overview

The INAV JavaScript Transpiler is a bidirectional JavaScript ↔ INAV Logic Conditions system that allows users to write flight controller logic in JavaScript instead of raw logic condition commands.

**System Components:**
- **Transpiler**: JavaScript → INAV Logic Conditions
- **Decompiler**: INAV Logic Conditions → JavaScript
- **Semantic Analysis**: Full validation and error checking
- **Parser**: Production-grade using Acorn
- **Code Generation**: Optimized INAV CLI commands
- **Integration**: Monaco Editor with IntelliSense

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        User Interface                        │
│              (Monaco Editor + Event Handlers)                │
└───────────────┬───────────────────────┬─────────────────────┘
                │                       │
                │ Transpile             │ Load from FC
                ▼                       ▼
┌───────────────────────────┐ ┌──────────────────────────────┐
│       TRANSPILER          │ │        DECOMPILER            │
│  (JavaScript → INAV)      │ │    (INAV → JavaScript)       │
└───────────────────────────┘ └──────────────────────────────┘
        │                                  │
        ▼                                  ▼
┌──────────────────┐              ┌──────────────────┐
│   Parser (Acorn) │              │ Analyze & Group  │
└────────┬─────────┘              └────────┬─────────┘
         │                                 │
         ▼                                 ▼
┌──────────────────┐              ┌──────────────────┐
│ Semantic Analyzer│              │  Generate Code   │
└────────┬─────────┘              └────────┬─────────┘
         │                                 │
         ▼                                 │
┌──────────────────┐                      │
│    Optimizer     │                      │
└────────┬─────────┘                      │
         │                                 │
         ▼                                 │
┌──────────────────┐                      │
│  Code Generator  │                      │
└────────┬─────────┘                      │
         │                                 │
         ▼                                 ▼
┌─────────────────────────────────────────────────────────────┐
│                   INAV Logic Conditions                      │
│                  (Flight Controller MSP)                     │
└─────────────────────────────────────────────────────────────┘
```

## Key Features

### Transpiler (JavaScript → INAV)

✅ **Robust Parsing**
- Uses Acorn for production-grade JavaScript parsing
- Handles all edge cases correctly
- Proper error messages with line/column numbers

✅ **Comprehensive Validation**
- Variable scope checking
- Property access validation
- Range checking (gvar indices, heading values, etc.)
- Dead code detection
- Conflict detection
- Uninitialized variable detection

✅ **Smart Code Generation**
- Optimized logic condition output
- Efficient operand usage
- Proper activator chaining

✅ **Developer Experience**
- Monaco Editor integration
- Real-time syntax highlighting
- IntelliSense autocomplete
- Lint mode for fast feedback
- Detailed error messages with code context

### Decompiler (INAV → JavaScript)

✅ **Intelligent Reconstruction**
- Pattern recognition for handler types
- Smart grouping of related conditions
- Preserves logical structure

✅ **Comprehensive Coverage**
- All INAV operations supported
- Flight parameters
- Global variables
- Override operations
- Arithmetic operations

✅ **Warning System**
- Alerts about lossy conversions
- Flags unsupported features
- Suggests manual review where needed

✅ **Documentation**
- Inline comments in generated code
- Warning annotations
- Original logic condition references

## Usage Examples

### Example 1: Transpilation

**Input JavaScript:**
```javascript
const { flight, override } = inav;

if (flight.homeDistance > 100) {
  override.vtx.power = 3;
}
```

**Output INAV Commands:**
```
logic 0 1 -1 2 2 1 0 100 0
logic 1 1 0 27 0 0 0 3 0
```

### Example 2: Decompilation

**Input INAV Commands:**
```
logic 0 1 -1 2 2 5 0 350 0
logic 1 1 0 25 0 0 0 50 0
```

**Output JavaScript:**
```javascript
const { flight, override } = inav;

if (flight.cellVoltage < 350) {
  override.throttleScale = 50;
}
```

### Example 3: Full Round-Trip

**Original Code:**
```javascript
on.arm({ delay: 1 }, () => {
  gvar[0] = flight.yaw;
});

if (flight.homeDistance > 500) {
  override.vtx.power = 4;
  override.throttleScale = 75;
}
```

**Transpiled → Saved to FC → Loaded from FC:**
```javascript
// INAV Logic Conditions - Decompiled to JavaScript
// Note: Comments, variable names, and some structure may be lost

const { flight, override, rc, gvar, on } = inav;

on.arm({ delay: 1 }, () => {
  gvar[0] = flight.yaw;
});

if (flight.homeDistance > 500) {
  override.vtx.power = 4;
  override.throttleScale = 75;
}
```

## Known Limitations

### Transpiler

1. **Subset of JavaScript**: Only supports INAV-specific syntax
2. **No complex expressions**: Nested function calls not supported
3. **Limited control flow**: Only if/else supported, no loops or complex functions

### Decompiler

1. **Lossy conversion**: Comments and variable names lost
2. **Structure changes**: Optimizations may alter original code
3. **Complex conditions**: May not perfectly reconstruct nested logic
4. **LC references**: References between logic conditions flagged for review

## Testing

### Running Tests

```bash
npm test parser.test.js
npm test analyzer.test.js
npm test decompiler.test.js
npm test integration.test.js
```

### Test Coverage

- Parser: Empty input, syntax errors, edge cases
- Analyzer: Validation, dead code, conflicts, ranges
- Transpiler: Full pipeline, error handling
- Decompiler: All operations, grouping, warnings
- Integration: Monaco editor, UI events, MSP communication

## Further Documentation

- **User Guide**: See `JAVASCRIPT_PROGRAMMING_GUIDE.md` for usage patterns
- **API Reference**: See `api_definitions_summary.md` for complete API
- **Maintenance**: See `api_maintenance_guide.md` for adding new features
- **Timer/WhenChanged**: See `TIMER_WHENCHANGED_EXAMPLES.md` for advanced patterns

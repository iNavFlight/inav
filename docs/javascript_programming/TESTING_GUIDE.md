# Transpiler Testing Guide

This guide explains how to test changes to the INAV JavaScript transpiler to ensure all components work together correctly.

## Overview

The transpiler has 4 main components that must stay in sync:

1. **Parser** (`transpiler/parser.js`) - Parses JavaScript code into AST
2. **Analyzer** (`transpiler/analyzer.js`) - Validates syntax and checks for errors
3. **Codegen** (`transpiler/codegen.js`) - Generates INAV CLI commands from AST
4. **Decompiler** (`transpiler/decompiler.js`) - Converts CLI commands back to JavaScript

## When Adding a New Feature

When implementing a new operation, function, or syntax feature, you must update:

### 1. Analyzer (transpiler/analyzer.js)
- Add validation for new syntax patterns
- Update property access validation if needed
- Add warnings for unsupported variations

Example: For RC channel states, added validation for `rc[0-17]` with `.low/.mid/.high` properties

### 2. Codegen (transpiler/codegen.js)
- Add code generation for the new feature
- Handle the new AST node types
- Generate correct INAV operation codes

Example: For RC channel states, added handler in `generateCondition()` to detect `rc[n].low` and generate LOW operation (4)

### 3. Decompiler (transpiler/decompiler.js)
- Add reverse mapping from operation codes to JavaScript
- Ensure generated code matches expected syntax

Example: For RC channel states, map operations 4/5/6 to `.low/.mid/.high` properties

### 4. Diagnostics (optional, editor/diagnostics.js)
- Add helpful warnings for common mistakes
- Suggest correct syntax alternatives

## Round-Trip Testing

The most reliable way to test transpiler changes is round-trip testing:
JavaScript → CLI commands → JavaScript

### Basic Test Template

```javascript
import { Transpiler } from './transpiler/index.js';
import { Decompiler } from './transpiler/decompiler.js';

const testCode = `
const { flight, gvar } = inav;

// Your test code here
if (flight.altitude > 100) {
  gvar[0] = 1;
}
`;

console.log('=== Original JavaScript ===\n');
console.log(testCode);

// Transpile
const transpiler = new Transpiler();
const transpileResult = transpiler.transpile(testCode);

console.log('\n=== Generated CLI Commands ===\n');
transpileResult.commands.forEach(cmd => console.log(cmd));

// Parse commands to LC format for decompiler
const logicConditions = transpileResult.commands.map((cmd) => {
  const parts = cmd.split(/\s+/);
  return {
    index: parseInt(parts[1]),
    enabled: parseInt(parts[2]),
    activatorId: parseInt(parts[3]),
    operation: parseInt(parts[4]),
    operandAType: parseInt(parts[5]),
    operandAValue: parseInt(parts[6]),
    operandBType: parseInt(parts[7]),
    operandBValue: parseInt(parts[8]),
    flags: parseInt(parts[9])
  };
});

// Decompile
const decompiler = new Decompiler();
const decompileResult = decompiler.decompile(logicConditions);

console.log('\n=== Decompiled JavaScript ===\n');
console.log(decompileResult.code);

if (decompileResult.success) {
  console.log('\n✅ Round-trip successful!');
} else {
  console.error('\n❌ Decompilation failed:', decompileResult.error);
}
```

### Running Tests

```bash
cd /path/to/inav-configurator/js/transpiler

# Create your test file
nano test_feature.js

# Run the test
node test_feature.js

# Clean up after testing
rm test_feature.js
```

## Test Cases to Verify

When making changes, test these scenarios:

### 1. Basic Functionality
```javascript
const { flight, gvar } = inav;

if (flight.altitude > 100) {
  gvar[0] = 1;
}
```
Expected: 2 commands (condition + action)

### 2. Error Handling
```javascript
// Invalid syntax - should produce helpful error
if (flight.invalidProperty > 100) {
  gvar[0] = 1;
}
```
Expected: Error with suggestion for correct property

### 3. Edge Cases
```javascript
// Test boundary values
if (rc[0].low) {  // Channel 0 (first)
  gvar[0] = 1;
}

if (rc[17].high) {  // Channel 17 (last valid)
  gvar[1] = 1;
}
```
Expected: Valid generation for both

### 4. Complex Combinations
```javascript
// Test multiple operations together
if (xor(rc[0].low, flight.armed)) {
  gvar[0] = Math.max(100, flight.altitude);
}
```
Expected: Proper nesting of operations

## Common Issues

### Analyzer Rejects Valid Syntax
**Problem**: Analyzer validation is too strict
**Solution**: Update `checkPropertyAccess()` or relevant validation method
**Example**: RC channels needed special handling for array syntax `rc[n]`

### Codegen Produces Wrong Operation Code
**Problem**: Operation constant name incorrect or not imported
**Solution**: Check `OPERATION.*` constants match `inav_constants.js`
**Example**: MODULUS was incorrectly `OPERATION.MOD` instead of `OPERATION.MODULUS`

### Decompiler Output Doesn't Match Input
**Problem**: Decompiler reverse mapping incomplete
**Solution**: Add case for new operation in decompiler
**Example**: LOW/MID/HIGH operations needed to map to `.low/.mid/.high` properties

### Line Numbers Off in Errors
**Problem**: Auto-import adds lines before parsing
**Solution**: Track lineOffset and adjust all error/warning line numbers
**Fixed**: Session 1 of 2025-11-25

## Verification Checklist

Before committing changes:

- [ ] Analyzer validates new syntax without false positives
- [ ] Codegen generates correct operation codes
- [ ] Decompiler correctly reverses the operation
- [ ] Round-trip test passes (JS → CLI → JS)
- [ ] Error messages are helpful and accurate
- [ ] Edge cases are handled (boundary values, empty inputs)
- [ ] Documentation updated (OPERATIONS_REFERENCE.md, API definitions)

## Example: Adding a New Operation

Let's say you want to add support for a new INAV operation `FOOBAR (operation 99)`:

**1. Check if operation exists in firmware**
```javascript
// Look in transpiler/inav_constants.js
const OPERATION = {
  // ...
  FOOBAR: 99,  // Check this exists
  // ...
};
```

**2. Update Codegen**
```javascript
// In transpiler/codegen.js
case 'CallExpression': {
  const funcName = condition.callee?.name;

  if (funcName === 'foobar') {
    // Generate FOOBAR operation
    const resultIndex = this.lcIndex;
    this.commands.push(
      `logic ${this.lcIndex} 1 ${activatorId} ${OPERATION.FOOBAR} ...`
    );
    this.lcIndex++;
    return resultIndex;
  }
}
```

**3. Update Decompiler**
```javascript
// In transpiler/decompiler.js
case OPERATION.FOOBAR:
  return 'foobar()';
```

**4. Update Analyzer (if needed)**
```javascript
// In transpiler/analyzer.js
// Add validation for foobar() usage
```

**5. Test Round-Trip**
```javascript
const testCode = `
const { flight } = inav;

if (foobar()) {
  gvar[0] = 1;
}
`;
// Run round-trip test...
```

**6. Update Documentation**
- Add to OPERATIONS_REFERENCE.md
- Add usage examples
- Update API definitions if needed

## Last Updated

2025-11-25 - Created after implementing RC channel state detection and completing all INAV operations

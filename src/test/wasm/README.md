# WASM SITL Test Suite

This directory contains test utilities for INAV WASM SITL builds.

---

## MSP Test Harness

**File:** `msp_test_harness.html`

Browser-based test interface for validating MSP communication between JavaScript and WASM SITL.

### Running the Tests

1. **Build WASM SITL:**
   ```bash
   cd inav
   mkdir -p build_wasm && cd build_wasm
   source ~/emsdk/emsdk_env.sh
   cmake .. -DTOOLCHAIN=wasm
   make SITL
   ```

2. **Copy test harness to build directory:**
   ```bash
   cp ../src/test/wasm/msp_test_harness.html .
   ```

3. **Start HTTP server:**
   ```bash
   python3 -m http.server 8082
   ```

4. **Open in browser:**
   ```
   http://localhost:8082/msp_test_harness.html
   ```

### Available Tests

1. **MSP_API_VERSION** - Convenience function test
   - Validates basic WASM function export
   - Returns API version (e.g., "2.5")

2. **MSP_FC_VARIANT** - String return test
   - Validates UTF8 string handling
   - Returns "INAV"

3. **General MSP Handler** - Memory management test
   - Tests `wasm_msp_process_command()` with malloc/free
   - Validates `getValue()` for reading WASM memory
   - Parses MSP_API_VERSION binary response

4. **MSP_STATUS** - Real flight controller data
   - Tests complex binary data parsing
   - Returns cycle time, sensors, flight mode, profile
   - Validates multi-byte integer parsing (uint16, uint32)

### Expected Results

All tests should show:
- ✅ Green success messages
- Parsed data values
- No JavaScript errors

### Troubleshooting

**"Module._wasm_msp_process_command is not a function"**
- Check EXPORTED_FUNCTIONS in cmake/sitl.cmake
- Ensure build completed successfully

**"SharedArrayBuffer error"**
- pthreads are disabled in Phase 5 MVP
- This error should not occur

**"Module.getValue is not a function"**
- Check EXPORTED_RUNTIME_METHODS includes getValue,setValue
- Rebuild WASM

---

## Architecture

The test harness validates this communication flow:

```
JavaScript (Browser)
    ↓ Module._wasm_msp_process_command()
WASM Bridge (wasm_msp_bridge.c)
    ↓ mspFcProcessCommand()
INAV MSP Handler (fc_msp.c)
    ↓
Flight Controller Logic
```

---

## Related Documentation

- Phase 5 Implementation: `/.claude/projects/wasm-sitl-phase5-msp-integration.md`
- MSP Bridge Source: `/src/main/target/SITL/wasm_msp_bridge.c`
- Build Configuration: `/cmake/sitl.cmake`

# INAV WebAssembly Build Guide

This document describes how to build INAV as WebAssembly (WASM) using Emscripten.

## Overview

INAV can be compiled to WebAssembly (WASM), allowing the flight controller firmware to run in web browsers. This enables:

- Browser-based flight simulator
- Web-based configuration and tuning tools
- Integration with online ground stations
- Educational simulation environments

## Prerequisites

### System Requirements

- **Emscripten SDK**: https://emscripten.org/docs/getting_started/downloads.html
- **CMake** 3.13 or later
- **Build tools**:
  - Unix/Linux/macOS: `make` or `ninja`
  - Windows: `ninja` or `make` (via MSYS2/WSL)

### Setting Up Emscripten

1. Install Emscripten SDK following official instructions
2. Activate the environment:

```bash
# Linux/macOS
source ~/emsdk/emsdk_env.sh

# Windows (PowerShell)
& "C:\emsdk\emsdk_env.ps1"
```

3. Verify installation:
```bash
emcc --version
```

## Building WASM

```bash
# Create build directory
mkdir -p build/wasm
cd build/wasm

# Configure for WASM
cmake ../.. -DWASM=ON -DCMAKE_BUILD_TYPE=Release

# Build
make WASM
# or with Ninja
ninja WASM
```

## Build Configuration Options

### Basic Options

| Option | Default | Description |
|--------|---------|-------------|
| `WASM` | OFF | Enable WebAssembly build |
| `CMAKE_BUILD_TYPE` | Release | Release or Debug |
| `WARNINGS_AS_ERRORS` | ON | Treat warnings as errors |

### Memory Options

```bash
# Custom memory allocation (in bytes)
cmake ../.. -DWASM=ON -DWASM_CUSTOM_MEMORY=33554432  # 32MB
```

Default: 16MB (16777216 bytes)

## Output Files

The build generates:

- **`inav_X.X.X_WASM.js`** - JavaScript loader and interface
- **`inav_X.X.X_WASM.wasm`** - WebAssembly binary module
- **`index.html`** - Simple Debugger HTML to test and debug without INAV-Configurator (Web-Version)

## Project Structure

### WASM-Specific Files

```
cmake/
├── emscripten.cmake    # Emscripten toolchain
└── wasm.cmake          # WASM-specific build rules

src/main/target/WASM/
├── CMakeLists.txt      # WASM target definition
├── target.h            # Target configuration
├── config.c            # Default settings
├── README.md           # Target documentation
└── wasm/               # Optional WASM-specific code
```

### Key CMake Integrations

#### Main CMakeLists.txt

- Added `WASM` option
- Updated `TOOLCHAIN_OPTIONS` to include `emscripten`
- Conditional ASM language enablement (not for WASM)
- Conditional OpenOCD/SVD inclusion

#### cmake/emscripten.cmake

- Emscripten compiler detection
- Compilation flags for different build types
- Linker options and WASM module settings

#### cmake/wasm.cmake

- WASM-specific source management
- `target_wasm()` function for building WASM targets
- Memory configuration
- Exported functions definition

## Compilation Flags

### Emscripten Flags

```cmake
-s WASM=1                          # Enable WebAssembly output
-s ALLOW_MEMORY_GROWTH=1           # Allow heap growth
-s ASSERTIONS=0                    # Disable assertions in release builds
-s TOTAL_MEMORY=16777216          # Initial heap size
```

### C Compiler Flags

- **Debug**: `-O0 -g -DDEBUG`
- **Release**: `-O3 -DNDEBUG`

## JavaScript Integration

### Embedding WASM in HTML

```html
<!DOCTYPE html>
<html>
<head>
    <title>INAV Flight Simulator</title>
</head>
<body>
    <script src="inav_9.0.0_WASM.js"></script>
    <script>
        Module = {
            onRuntimeInitialized: function() {
                console.log("INAV WASM module loaded");
                // Initialize your flight simulator
            }
        };
    </script>
</body>
</html>
```

### Calling WASM Functions from JavaScript

```javascript
// Call exported C functions
Module.ccall('navigationUpdate', null, [], []);

// Use memory access
var ptr = Module._malloc(100);
// ... work with memory ...
Module._free(ptr);

// Access WASM memory directly
var data = new Uint8Array(Module.HEAPU8.buffer, ptr, 100);
```

## Debugging

### Enable Debug Symbols

```bash
cmake ../.. -DWASM=ON -DCMAKE_BUILD_TYPE=Debug
make WASM
```

### Browser Debugging

To debug the webassembly using Chome Dev-Tools: https://developer.chrome.com/docs/devtools/wasm?hl=de

**Tip for Windows users:**
As recommended by INAV and SITL, it is best to use WSL. If a test server is running in WSL, it can be accessed normally via the browser in Windows on localhost, only the DWARF/Webassembly debugger cannot access the source maps.
As a workaround, mount a folder in WSL as a network drive in Windows (e.g. WSL installation name: Ubuntu -> `\wsl$\Ubuntu` as network path, drive X:) and then set a path replacement in the settings of the "C/C++ Dev Tools Support extension": ´/home/user´ -> `X:\home\user`

1. Open DevTools (F12)
2. Go to Sources tab
3. WASM debugging available with debug builds
4. Set breakpoints in C code (source maps needed)

### Console Output

C `printf()` statements go to browser console:

```c
printf("Debug message: %d\n", value);  // Appears in console
```

## Size Optimization

For production deployments, reduce WASM binary size:

```bash
cmake ../.. -DWASM=ON -DCMAKE_BUILD_TYPE=Release \
    -DSTRIP_SYMBOLS=ON
```

Typical binary sizes:
- Debug: ~8-12 MB
- Release: ~2-4 MB (with optimization)
- Compressed (gzip): ~400-800 KB

## Continuous Integration

The WASM build can be integrated into CI/CD:

```bash
#!/bin/bash
set -e

# Activate Emscripten
source ~/emsdk/emsdk_env.sh

# Build
mkdir -p build/wasm
cd build/wasm
cmake ../.. -DWASM=ON -DCMAKE_BUILD_TYPE=Release -GNinja
ninja WASM

# Optionally: compress for deployment
gzip -k inav_*.wasm
```

## Deployment

### Web Server Setup

1. Copy `.js` and `.wasm` files to web server
2. Configure proper MIME types:
   ```
   application/wasm  wasm
   application/javascript  js
   ```
3. Enable CORS if necessary
4. Optional: Compress with gzip

## Troubleshooting

### "emcc not found"

Ensure Emscripten environment is activated:
```bash
source ~/emsdk/emsdk_env.sh  # Linux/macOS
```

### Memory Issues

Increase `WASM_CUSTOM_MEMORY`:
```bash
cmake ../.. -DWASM=ON -DWASM_CUSTOM_MEMORY=67108864  # 64MB
```

### Build Failures

1. Check CMake version: `cmake --version` (needs 3.13+)
2. Clear CMake cache: `rm -rf build/wasm && mkdir build/wasm`
3. Check compiler: `emcc --version`

## Performance Considerations

- WASM runs at near-native speed (~70-80% of C on modern browsers)
- Initial module load: 1-3 seconds (depends on compression and connection)
- Simulation loop: Achievable at 4kHz with proper tuning
- Memory: Configurable but limited to available RAM

## Advanced Features

### Extending WASM Target

Add custom WASM-specific code in `src/main/target/WASM/wasm/`:

```c
// src/main/target/WASM/wasm/my_interface.c
#include "my_interface.h"

// Called from JavaScript
void EMSCRIPTEN_KEEPALIVE wasmSimulationStep(void) {
    // Custom simulation logic
}
```

Then expose in JavaScript via `ccall`:

```javascript
Module.ccall('wasmSimulationStep', null, [], []);
```

### Export More Functions

In `cmake/wasm.cmake`, modify exported functions:

```cmake
-s EXPORTED_FUNCTIONS="['_main','_malloc','_free','_myCustomFunction']"
```

## References

- [Emscripten Documentation](https://emscripten.org/docs/)
- [WebAssembly Specification](https://webassembly.org/)
- [Emscripten Best Practices](https://emscripten.org/docs/optimizing/Optimizing-Code.html)

#!/bin/bash
# Build script for INAV SITL on macOS

set -e

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build/build_SITL"

echo "Building INAV SITL for macOS..."
echo "Project root: $SCRIPT_DIR"
echo "Build directory: $BUILD_DIR"

# Create/clean build directory
if [ -d "$BUILD_DIR" ]; then
    echo "Cleaning previous build..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cd "$BUILD_DIR"
cmake -DSITL=ON -DDEBUG=ON -DWARNINGS_AS_ERRORS=OFF -GNinja ../..

# Build
echo "Building with Ninja..."
ninja

# Check if build was successful
BINARY="$BUILD_DIR/inav_8.0.1_SITL"
if [ -f "$BINARY" ]; then
    echo ""
    echo "✓ Build successful!"
    echo "Binary location: $BINARY"
    echo ""
    echo "To run the simulator:"
    echo "  ./build/inav_8.0.1_SITL"
    echo ""
else
    echo "✗ Build failed - binary not found at $BINARY"
    exit 1
fi

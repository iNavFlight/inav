#!/bin/bash
# This script is for Docker builds (Linux). For macOS builds, use build.sh or native CMake.
# To build for macOS natively, use:
#   cd /path/to/inav/build
#   cmake -DSITL=ON -DDEBUG=ON -DWARNINGS_AS_ERRORS=OFF ..
#   make

rm -r build_SITL
mkdir -p build_SITL
#cmake -DSITL=ON -DWARNINGS_AS_ERRORS=ON -GNinja -B build_SITL ..
cmake -DSITL=ON -DDEBUG=ON -DWARNINGS_AS_ERRORS=OFF -GNinja -B build_SITL ..
cd build_SITL
ninja
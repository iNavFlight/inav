#!/bin/bash
rm -r build_SITL
mkdir -p build_SITL
#cmake -DSITL=ON -DWARNINGS_AS_ERRORS=ON -GNinja -B build_SITL ..
cmake -DSITL=ON -DDEBUG=ON -DWARNINGS_AS_ERRORS=ON -GNinja -B build_SITL ..
cd build_SITL
ninja
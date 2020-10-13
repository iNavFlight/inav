#!/bin/bash
set -e

LAST_CMAKE_AT_REV_FILE="docker_cmake.rev"
CURR_REV="$(git rev-parse HEAD)"

initialize_cmake() {
    echo -e "*** CMake was not initialized yet, doing it now.\n"
    cmake ..
    echo "$CURR_REV" > "$LAST_CMAKE_AT_REV_FILE"
}

# Check if CMake has never been initialized
if [ ! -f Makefile ]; then
    initialize_cmake
fi

# Check if CMake was initialized for a different Git revision (new targets may have been added)
if [ -f "$LAST_CMAKE_AT_REV_FILE" ]; then
    LAST_CMAKE_AT_REV="$(cat $LAST_CMAKE_AT_REV_FILE)"
    if [[ "$LAST_CMAKE_AT_REV" != "SKIP" ]] && [[ "$LAST_CMAKE_AT_REV" != "$CURR_REV" ]]; then
        initialize_cmake
    fi
else
    initialize_cmake
fi

# Let Make handle the arguments coming from the build script
make "$@"

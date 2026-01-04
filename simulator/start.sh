#!/usr/bin/env bash

set -Eeuo pipefail

echo "Script PID: $$"

cleanup() {
    echo
    echo "Stopping everything (process group $$)..."
    # Kill entire process group
    kill -- -$$ 2>/dev/null || true
}

trap cleanup INT TERM EXIT

echo "Starting Python..."
rotenv/bin/python3.12 main.py --sim=inav &

echo "Starting INAV SITL..."
../build/build_SITL/inav_8.0.1_SITL \
    --path=./eeprom.bin \
    --sim=adum \
    --chanmap=M01-01,M02-02,M03-03,M04-04 \
    > simulator.log 2>&1 &

echo "Waiting for INAV to initialize..."
sleep 5

echo "Starting socat..."
socat -d -d \
  pty,raw,echo=0,link=/tmp/inav_uart6 \
  tcp:localhost:5765 &

echo
echo "All components started."
echo "PTY available at /tmp/inav_uart6"
echo "Press Ctrl+C to stop everything."

# Keep script in foreground & process group alive
wait

pkill -f SITL
pkill -f socat
pkill -f main.py

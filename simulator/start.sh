#!/usr/bin/env bash

set -Eeuo pipefail

# Determine OS and set Python executable path
OS="$(uname -s)"
if [[ "$OS" == "Linux" ]]; then
    PYTHON_BIN="rotenv/bin/python3"
elif [[ "$OS" == "Darwin" ]]; then
    PYTHON_BIN="/Users/mitjastrakl/miniconda3/envs/inavsim-py312/bin/python3"
else
    echo "Unsupported OS: $OS"
    exit 1
fi

# Setup cleanup handler
cleanup() {
    echo ""
    echo "Shutting down..."
    # Kill all background processes in this script's process group
    kill -- -$$ 2>/dev/null || true
}

trap cleanup INT TERM EXIT

# Start Python simulator
echo "Starting Python simulator..."
$PYTHON_BIN main.py --sim=inav &
PYTHON_PID=$!

# Wait for Python socket to be ready
echo "Waiting for Python to bind socket on port 2323..."
for i in {1..30}; do
    if netstat -an 2>/dev/null | grep -q "2323.*LISTEN" || lsof -i :2323 >/dev/null 2>&1; then
        echo "✓ Python ready"
        break
    fi
    if ! kill -0 $PYTHON_PID 2>/dev/null; then
        echo "✗ Python process died"
        exit 1
    fi
    sleep 0.5
done

sleep 2  # Extra time for full initialization

# Start INAV SITL simulator
echo "Starting INAV SITL..."
BINARY="../build/inav_8.0.1_SITL"
if [ ! -f "$BINARY" ]; then
    echo "✗ Error: SITL binary not found at $BINARY"
    echo "  Run: bash ../build_sitl_mac.sh (macOS) or bash ../build.sh (Linux)"
    exit 1
fi

$BINARY \
    --path=./eeprom.bin \
    --sim=adum \
    --chanmap=M01-01,M02-02,M03-03,M04-04 &
INAV_PID=$!

echo "Waiting for INAV to initialize..."
sleep 2

# Start socat to expose UART6 on a PTY
echo "Starting socat PTY bridge..."
socat -d -d \
  pty,raw,echo=0,link=/tmp/inav_uart6 \
  tcp:localhost:5765 &
SOCAT_PID=$!

echo ""
echo "✓ All components started"
echo "  Python simulator:  localhost:2323"
echo "  INAV SITL:         TCP sockets on various ports"
echo "  UART6 PTY:         /tmp/inav_uart6"
echo ""
echo "Press Ctrl+C to stop all components."
echo ""

# Keep script alive and wait for all background processes
wait

#!/usr/bin/env bash

echo "Removing old library..."
for dir in ASLUAV AVSSUAS all ardupilotmega common csAirLink cubepilot development icarous loweheiser marsh minimal python_array_test standard stemstudios storm32 test ualberta uAvionix; do
    rm -rf "$dir"
done
rm -f checksum.h
rm -f mavlink_*
rm -f protocol.h

echo "Downloading or updating MAVLink sources..."

if [ -d "mavlink-src/.git" ]; then
    cd mavlink-src || return 1
    git fetch
    git checkout origin/master
    cd ../
else
    git clone https://github.com/mavlink/mavlink.git --recursive mavlink-src
fi

export PYTHONPATH="$(pwd)/mavlink-src"

echo "Running MAVLink generator..."
python mavlink-src/pymavlink/tools/mavgen.py --lang=C --wire-protocol=2.0 --output=. mavlink-src/message_definitions/v1.0/storm32.xml --no-validate

echo "Removing inlines..."
sed -i 's/ inline//' protocol.h

echo "Removing trailing whitespace..."
sed -i 's/[[:space:]]*$//' common/testsuite.h

@echo off

echo Removing old library...
for %%D in (ASLUAV AVSSUAS all ardupilotmega common csAirLink cubepilot development icarous loweheiser marsh minimal python_array_test standard stemstudios storm32 test ualberta uAvionix) do if exist %%D\ rmdir /s /q %%D
del /q checksum.h 2>nul
del /q mavlink_* 2>nul
del /q protocol.h 2>nul

echo Downloading or updating MAVLink sources...

if exist mavlink-src\.git\ (
    cd mavlink-src
    git fetch
    git checkout origin/master
    cd ../
) else (
    git clone https://github.com/mavlink/mavlink.git --recursive mavlink-src
)

set PYTHONPATH=%CD%\mavlink-src

echo Running MAVLink generator...
python -m pymavlink.tools.mavgen --lang=C --wire-protocol=2.0 --output=. mavlink-src/message_definitions/v1.0/storm32.xml --no-validate

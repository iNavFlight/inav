

rotenv/bin/python3.12 main.py --sim=inav & 

../build/build_SITL/inav_8.0.1_SITL \
    --path=./eeprom.bin \
    --sim=adum  \
    --chanmap=M01-01,M02-02,M03-03,M04-04
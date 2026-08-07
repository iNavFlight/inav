#!/bin/bash
cd build_SITL

#Lauch SITL - configurator only mode
./inav_7.0.0_SITL

#Launch SITL - connect to X-Plane. IP address should be host IP address, not 127.0.0.1. Can be found in X-Plane "Network" tab.
#./inav_7.0.0_SITL --sim=xp --simip=192.168.2.105 --simport=49000
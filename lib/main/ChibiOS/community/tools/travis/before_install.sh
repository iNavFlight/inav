#!/bin/sh

set -ex

cd /tmp

sudo apt-get install lib32z1
wget https://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q3-update/+download/gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
tar xjf gcc-arm-none-eabi-4_9-2015q3-20150921-linux.tar.bz2
export PATH=/tmp/gcc-arm-none-eabi-4_9-2015q3/bin:$PATH
arm-none-eabi-gcc --version

cd -
cd ..

mkdir ChibiOS-RT
cd ChibiOS-RT
git clone https://github.com/ChibiOS/ChibiOS.git .

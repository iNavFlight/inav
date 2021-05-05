#!/bin/bash

if [ ! -d $PWD/gcc-arm-none-eabi-8-2018-q4-major/bin ] ; then
    curl --retry 10 --retry-max-time 120 -L "https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/8-2018q4/gcc-arm-none-eabi-8-2018-q4-major-linux.tar.bz2" | tar xfj -
fi

#!/bin/bash

if [ ! -d $PWD/gcc-arm-none-eabi-7-2018-q2-update/bin ] ; then
    curl --retry 10 --retry-max-time 120 -L "https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/7-2018q2/gcc-arm-none-eabi-7-2018-q2-update-linux.tar.bz2" | tar xfj -
fi

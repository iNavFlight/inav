#!/bin/bash

if [ ! -d $PWD/tools/gcc-arm-none-eabi-10-2020-q4-major/bin ] ; then
    curl --retry 10 --retry-max-time 120 -L "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2" | tar xfj - -C $PWD/tools/
fi

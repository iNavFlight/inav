#!/bin/bash

targets=("PUBLISHMETA=True" \
    "RUNTESTS=True" \
    "TARGET=SPRACINGF3" \
    "TARGET=SPRACINGF3EVO" \
    "TARGET=LUX_RACE" \
    "TARGET=MOTOLAB" \
    "TARGET=RMDO" \
    "TARGET=SPARKY" \
    "TARGET=STM32F3DISCOVERY" \
    "TARGET=RCEXPLORERF3" )
#fake a travis build environment
export TRAVIS_BUILD_NUMBER=$(date +%s)
export BUILDNAME=${BUILDNAME:=fake_travis}
export TRAVIS_REPO_SLUG=${TRAVIS_REPO_SLUG:=$USER/simulated}

for target in "${targets[@]}"
do
	unset RUNTESTS PUBLISHMETA TARGET
	eval "export $target"
	make clean
	./.travis.sh
done

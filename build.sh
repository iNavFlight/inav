#!/usr/bin/env bash
set -e

if [[ $# == 0 ]]; then
  echo -e "\
Usage syntax: ./build.sh <TARGET>

Notes:
  * You can specify multiple targets.
    ./build.sh <TARGET_1> <TARGET_2> <TARGET_N>
  * To get a list of all targets use \"help\". Hint: pipe the output through a pager.
    ./build.sh help | less
  * To build all targets use \"all\"
    ./build.sh all
  * To clean a target prefix it with \"clean_\".
    ./build.sh clean_MATEKF405SE
  * To clean all targets just use \"clean\".
    ./build.sh clean"
  exit 1
fi

if [ -z "$(docker images -q inav-build)" ]; then
  echo -e "*** Building image\n"
  docker build -t inav-build .
  echo -ne "\n"
fi

if [ ! -d ./build ]; then
  echo -e "*** Creating build directory\n"
  mkdir ./build
fi

echo -e "*** Building targets [$@]\n"
docker run --rm -it -v "$(pwd)":/src inav-build $@

if ls ./build/*.hex &> /dev/null; then
  echo -e "\n*** Built targets in ./build:"
  stat -c "%n (%.19y)" ./build/*.hex
fi

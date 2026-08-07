#!/usr/bin/env bash
set -euo pipefail

if [[ $# == 0 ]]; then
  echo -e "\
Usage syntax: ./build.sh <TARGET>

Notes:
  * You can specify multiple targets.
    ./build.sh <TARGET_1> <TARGET_2> <TARGET_N>
  * To get a list of release targets use \"release_targets\"
    ./build.sh release_targets
  * To get a list of valid targets use \"valid_targets\"
    ./build.sh valid_targets
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

run_docker() {
    docker run --rm -it -v "$(pwd)":/src inav-build "$@"
}

if [ -z "$(docker images -q inav-build)" ]; then
    echo "*** Building Docker image"
    docker build -t inav-build \
                 --build-arg USER_ID="$(id -u)" \
                 --build-arg GROUP_ID="$(id -g)" .
else
    docker build -q -t inav-build \
                 --build-arg USER_ID="$(id -u)" \
                 --build-arg GROUP_ID="$(id -g)" . >/dev/null ||
    { echo "*** Building Docker image: ERROR"; exit 1; }
fi

if [ ! -d ./build ]; then
  echo -e "*** Creating build directory\n"
  mkdir ./build && chmod 777 ./build
fi

if [ ! -d ./downloads ]; then
  echo -e "*** Creating downloads directory\n"
  mkdir ./downloads && chmod 777 ./downloads
fi

if [ ! -d ./tools ]; then
  echo -e "*** Creating tools directory\n"
  mkdir ./tools && chmod 777 ./tools
fi

case "$1" in
    release_targets)
        run_docker targets | sed -n 's/^Release targets: \(.*\)/\1/p'|tr ' ' '\n'
    ;;
    valid_targets)
        run_docker targets | sed -n 's/^Valid targets: \(.*\)/\1/p'|tr ' ' '\n'
    ;;
    *)
        echo -e "*** Building targets [$@]\n"
        run_docker "$@"
        if ls ./build/*.hex &> /dev/null; then
            echo -e "\n*** Built targets in ./build:"
            stat -c "%n (%.19y)" ./build/*.hex
        fi
    ;;
esac


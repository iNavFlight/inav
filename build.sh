set -e

if [[ $# == 0 ]]; then
  echo -e "\
Usage syntax: ./build.sh <TARGET>

Notes:
  * You can specify multiple targets.
  * If no targets are specified, *all* of them will be built.
  * To clean a target prefix it with \"clean_\".
  * To clean all targets just use \"clean\"."
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

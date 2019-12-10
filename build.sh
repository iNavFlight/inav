if [ -z "$1" ]; then
  echo "Usage syntax: ./build.sh <TARGET>"
  exit 1
fi

if [ -z "$(docker images -q inav-build)" ]; then
  echo -e "*** Building image\n"
  docker build -t inav-build .
fi

echo -e "*** Building target $1\n"
docker run --rm -v "$(pwd)":/home/src/ inav-build make TARGET="$1"

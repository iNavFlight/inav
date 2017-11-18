echo "Building target" $1
docker run --rm -v `pwd`:/home/src/ flyandi/docker-inav make TARGET=$1
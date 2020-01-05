# Building with Docker

Building in Docker is remarkably easy.

## Build a container with toolchain

```
docker build -t inav-build .
```

## Building firmware with Docker on Ubuntu

Build specified target
```
sh build.sh SPRACINGF3
```

## Building firmware with Docker on Windows 10

Path in Docker on Windows works in a _strange_ way, so you have to provide full path for `docker run` command. For example:

`docker run --rm -v //c/Users/pspyc/Documents/Projects/inav:/home/src/ inav-build make TARGET=AIRBOTF4`

So, `c:\Users\pspyc\Documents\Projects\inav` becomes `//c/Users/pspyc/Documents/Projects/inav`

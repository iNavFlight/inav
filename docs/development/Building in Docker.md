# Building with Docker

Building with [Docker](https://www.docker.com/) is remarkably easy: an isolated container will hold all the needed compilation tools so that they won't interfere with your system and you won't need to install and manage them by yourself. You'll only need to have Docker itself [installed](https://docs.docker.com/install/).

The first time that you'll run a build it will take a little more time than following executions since it will be building its base image first. Once this initial process is completed, the firmware will be always built immediately.

If you want to start from scratch - _even if that's rarely needed_ - delete the `inav-build` image on your system (`docker image rm inav-build`).

## Linux

In the repo's root, run:

```
./build.sh <TARGET>
```

Where `<TARGET>` must be replaced with the name of the target that you want to build. For example:

```
./build.sh MATEKF405SE
```

## Windows 10

Docker on Windows requires full paths for mounting volumes in `docker run` commands. For example: `c:\Users\pspyc\Documents\Projects\inav` becomes `//c/Users/pspyc/Documents/Projects/inav` .

You'll have to manually execute the same steps that the build script does:

1. `docker build -t inav-build .`
   + This step is only needed the first time.
2. `docker run --rm -it -v <PATH_TO_REPO>:/src inav-build <TARGET>`
   + Where `<PATH_TO_REPO>` must be replaced with the absolute path of where you cloned this repo (see above), and `<TARGET>` with the name of the target that you want to build.

Refer to the [Linux](#Linux) instructions or the [build script](/build.sh) for more details.
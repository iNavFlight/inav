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

Run the script with no arguments to get more details on its usage:

```
./build.sh
```

## Windows 10

Docker on Windows requires full paths for mounting volumes in `docker run` commands. For example: `c:\Users\pspyc\Documents\Projects\inav` becomes `//c/Users/pspyc/Documents/Projects/inav` .
If you are getting error "standard_init_linux.go:219: exec user process caused: no such file or directory", make sure `\cmake\docker.sh` has lf (not crlf) line endings.

You'll have to manually execute the same steps that the build script does:

1. `docker build --build-arg USER_ID=1000 --build-arg GROUP_ID=1000 -t inav-build .`
   + This step is only needed the first time.
   + If GDB should be installed in the image, add argument '--build-arg GDB=yes'
2. `docker run --rm -it -v <PATH_TO_REPO>:/src inav-build <TARGET>`
   + Where `<PATH_TO_REPO>` must be replaced with the absolute path of where you cloned this repo (see above), and `<TARGET>` with the name of the target that you want to build.

3. If you need to update `Settings.md`, run:

`docker run --entrypoint /src/cmake/docker_docs.sh --rm -it -v <PATH_TO_REPO>:/src inav-build`

4. Building SITL: 

`docker run --rm --entrypoint /src/cmake/docker_build_sitl.sh -it -v <PATH_TO_REPO>:/src inav-build`

5. Running SITL: 

`docker run -p 5760:5760 -p 5761:5761 -p 5762:5762 -p 5763:5763 -p 5764:5764 -p 5765:5765 -p 5766:5766 -p 5767:5767 --entrypoint /src/cmake/docker_run_sitl.sh --rm -it -v <PATH_TO_REPO>:/src inav-build`.
   + SITL command line parameters can be adjusted in `cmake/docker_run_sitl.sh`.

Refer to the [Linux](#Linux) instructions or the [build script](/build.sh) for more details.

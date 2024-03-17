## Building SITL

### Linux and FreeBSD:
Almost like normal, ruby, cmake and make are also required.
With cmake, the option "-DSITL=ON" must be specified.

```
mkdir build_SITL
cd build_SITL
cmake -DSITL=ON ..
make
```

### Windows:
Compile under cygwin, then as in Linux.
Copy cygwin1.dll into the directory, or include cygwin's /bin/ directory in the environment variable PATH.

If the build fails (segfault, possibly out of memory), adding `-DCMAKE_BUILD_TYPE=MinRelSize` to the `cmake` command may help.

#### Build manager

`ninja` may also be used (parallel builds without `-j $(nproc)`):

```
cmake -GNinja -DSITL=ON ..
ninja
```

### Compiler requirements

* Modern GCC. Must be a *real* GCC, macOS faking it with clang will not work. GCC 10 to GCC 13 are known to work.
* Unix sockets networking. Cygwin is required on Windows (vice `winsock`).
* Pthreads

## Supported environments

* Linux on x86_64, ia-32, Aarch64 (e.g. Rpi4), RISCV64 (e.g. VisionFive2)
* Windows on x86_64
* FreeBSD (x86_64 at least).

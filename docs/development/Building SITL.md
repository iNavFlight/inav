## Building SITL

### Compiler requirements

* Modern GCC. Must be a *real* GCC, unless you're using MacOS; faking it with clang will not work. gcc13 or later is recommended.
* Unix sockets networking. Cygwin is required on Windows (vice `winsock`).
* Pthreads


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

Compile under cygwin, using the Linux instructions. Note that depending on the Cygwin packaging _du jour_ of `ruby` it _may_ also be necessary to:

* Install the `rubygems` package.
* run `gem install getoptlong`

Copy cygwin1.dll into the directory, or include cygwin's /bin/ directory in the environment variable PATH.

If the build fails (segfault, possibly out of memory), adding `-DCMAKE_BUILD_TYPE=MinRelSize` to the `cmake` command may help.

### Build manager

`ninja` may also be used (parallel builds without `-j $(nproc)`):

```
cmake -GNinja -DSITL=ON ..
ninja
```

### Supported environments

* Linux on x86_64, ia-32, Aarch64 (e.g. Rpi), RISCV64 (e.g. VisionFive2)
* Windows on x86_64
* FreeBSD x86_64 (at least).
* MacOS on x86_64 and Aarch64

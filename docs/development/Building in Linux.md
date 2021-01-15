# Generic Linux development tools
## Overview

This article endeavours to provide a generic guide for compiling inav on Linux for inav 2.6 and later.

inav requires a reasonably modern `gcc-arm-none-eabi` cross-compiler. Different Linux distros will provide different versions of the cross-compiler. This will range from obsolete versions (e.g. Debian, Ubuntu LTS) to the latest stable release (Arch Linux).

In order to provide a uniform and reasonably modern cross compiler, inav provides for the installation of a "known good / working" cross compiler, as well as a mechanism to override this if your distro provides a more modern option (e.g Arch Linux). In general, from a security perspective, Linux distros discourage the installation of software from sources other than the official distribution repositories and  'approved' sources (Ubuntu PPA, Arch AUR). The inav approach of providing a recommended compiler is however both sound and justified:

* The cross-compiler is installed from a reputable source (ARM, the company that makes the CPUs used in our flight controllers)
* Disto cross-compilers are often older than the recommended inav compiler
* The installed cross-compiler is only used to build inav and it not obviously / generally available outside of the inav build environment.

There are a however some specific cases for using the distro cross-compiler in preference to that installed by inav:

* You are using a distro that installs a more modern compiler (Arch)
* You are using a host platform for which ARM does not provide a compiler (e.g. Linux ia32).

## Prerequisites

In addition to a cross-compiler, it is necessary to install some other tools:

* `git`  : clone and manage the inav code repository
* `cmake` : generate the build environment
* `make` : run the firmware compilation
* `ruby` : build some generated source files from JSON definitions
* `gcc` : native compiler used to generate settings and run tests

Note that inav requires `cmake` version 3.13 or later; any distro that provides `cmake` 3.13 will also provide adequate versions of the other tools.

Note also that Ubuntu 18.04 LTS does NOT provide a modern enough `cmake`; it is recommended that you upgrade to Ubuntu 20.04 LTS which does.

### Ubuntu / Debian
```
# make sure the system is updated first
sudo apt update && sudo apt upgrade
sudo apt install git make ruby cmake gcc
```

### Fedora
```
# make sure the system is updated first
sudo dnf -y update
sudo dnf install git make ruby cmake gcc
```

### Arch
```
# make sure the system is updated first
sudo pacman -Syu
sudo pacman -S git make ruby cmake gcc
```

Once these prerequisites are installed, we can clone the repository to provide a local instance of the inav source code.

## Cloning the repository
```
git clone https://github.com/iNavFlight/inav.git
```

Note: If you have a Github account with registered ssh key you can replace the `git clone` command with  `git clone git@github.com:iNavFlight/inav.git` instead of the https link.

The `git clone` creates an `inav` directory; we can enter this directory, configure the build environment and build firmware.


## Build tooling

For 2.6 and later, inav uses `cmake` as its primary build tool. `cmake` simplies various platform and hardware dependencies required to cross compile multiple targets. `cmake` still uses GNU `make` to invoke the actual compiler. It is necessary to configure the build enviroment with `cmake` before we can build any firmware.

## Using `cmake`

The canonanical method of using `cmake` is to create a `build` directory and run the `cmake` and `make` commands from within the `build` directory. So, assuming we've cloned the firmware repository into an `inav` directory, we can issue the following commands to set up the build environment.

```
cd inav
# first time only, create the build directory
mkdir build
cd build
cmake ..
# note the "..", this is required as it tells cmake where to find its ruleset
```

`cmake` will check for the presence of an inav-embedded cross-compiler; if this cross-compiler is not found it will attempt to download the vendor (ARM) GCC cross-compiler.

Note. If you want to use your own cross-compiler, either because you're running a distro (e.g. Arch Linux) that ships a more recent cross-compiler, or you're on a platform for which ARM doesn't provide a cross-compiler (e.g. 32bit Linux), the you should run the `cmake` command as:

```
cmake -DCOMPILER_VERSION_CHECK=OFF ..
```

`cmake` will generate a number of files in your `build` directory, including a cache of generated build settings `CMakeCache.txt` and a `Makefile`.

## Bulding the firmware

Once `cmake` has generated the `build/Makefile`, this `Makfile` (with `make`) is used to build the firmware, again from the `build` directory. It is not necessary to re-run `cmake` unless the inav cmake configuration is changed (i.e. a new release) or you wish to swap between the ARM SDK compiler and a distro or other external compiler.

The generated `Makefile` uses different a target selection mechanism from the older (pre 2.6) top level `Makefile`; you can generate a list of targets with `make help` (or, as the list is extremely long), pipe this into a pager, e.g. `make help | less`.

Typically, to build a single target, just pass the target name to `make`; note that unlike eariler releases, `make` without a target specified will build **all** targets.

```
# Build the MATEKF405 firmware
make MATEKF405
```

One can also build multiple targets from a sinlge `make` command:

```
make MATEKF405 MATEKF722
```

The resultant hex file are in the `build` directory.

You can then use the INAV Configurator to flash the local `build/inav_x.y.z_TARGET.hex` file, or use `stm32flash` or `dfu-util` directly from the command line.

[msp-tool](https://github.com/fiam/msp-tool) and [flash.sh](https://github.com/stronnag/mwptools/blob/master/docs/MiscTools.asciidoc#flashsh) provide / describe 3rd party helper tools for command line flashing.

### Cleaning

You can clean out the built files, either for all targets or selectively; a selective clean target is simply defined by prefixing the target name with `clean_`:

```
# clean out every thing
make clean
# clean out single target
make clean_MATEKF405
# or multiple targets
make clean_MATEKF405  clean_MATEKF722
```

### `cmake` cache maintainance

`cmake` caches the build environment, so you don't need to rerun `cmake` each time you build a target. Two `make` options are provided to maintain the `cmake` cache.

* `make edit_cache`
* `make rebuild_cache`

It is unlikely that the typical user will need to employ these options, other than perhaps to change between the embedded ARM and distro compilers.

## Updating and rebuilding

In order to update your local firmware build:

* Navigate to the local iNav repository
* Use the following steps to pull the latest changes and rebuild your local version of inav firmware from the `build` directory:

```
$ cd inav
$ git pull
$ cd build
$ make <TARGET>
```

## Advanced Usage

For more advanced development information and `git` usage, please refer to the [development guide](https://github.com/iNavFlight/inav/blob/master/docs/development/Development.md).

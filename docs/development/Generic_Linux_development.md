# Generic Linux development tools
## Overview

This article endeavours to provide a generic guide for compiling iNav on Linux for iNav after iNav 2.2.1.

iNav requires a reasonably modern `gcc-arm-none-eabi` cross-compiler. Different Linux distros will provide different versions of the cross-compiler. This will range from obsolete versions (e.g. Debian, Ubuntu LTS) to the latest stable release (Arch Linux). 

In order to provide a uniform and reasonably modern cross compiler, after release iNav 2.2.1, iNav provides for the installation of a "known good / working" cross compiler, as well as a mechanism to override this if your distro provides a more modern option (e.g Arch Linux). In general, from a security perspective, Linux distros discourage the installation of software from sources other than the official distribution repositories and  'approved' sources (Ubuntu PPA, Arch AUR). The iNav approach of providing a recommended compiler is however both sound and justified:

* The cross-compiler is installed from a reputable source (ARM, the company that makes the CPUs used in our UAS)
* Disto cross-compiler are often older than the recommended iNav compiler
* The installed cross-compiler is only used to build iNav and it not obviously / generally available outside of the iNav build environment.

There are a however some specific cases for using the distro cross-compiler in preference to that installed by iNav:

* You are using a distro that installs a more modern compiler (Arch)
* You are using a host platform other than x86_64 (e.g. ia32, AArch64).

However, before we consider the compiler options, it is necessary to install some other dependencies.

## Other Prerequisites

In addition to a cross-compiler, it is necessary to install some other tools:

### Ubuntu / Debian
```
$ sudo apt install gcc git make ruby
```

### Fedora
```
$ sudo dnf install gcc git make ruby
```

### Arch
```
$ sudo pacman -S gcc git make ruby  
```

Once these prerequisites are installed, we can clone the repository to provide a local instance of the iNav source code.

## Cloning the repository
```
$ git clone https://github.com/iNavFlight/inav.git
``` 

Note: If you have a Github account with registered ssh key you can replace the `git clone` command with  `git clone git@github.com:iNavFlight/inav.git` instead of the https link.

The `git clone` creates an `inav` directory; we can enter this directory and try and build the firmware. Initially, will probably fail as we have not yet defined the cross-compiler. The following example is from Ubuntu 19.04:

```
$ cd inav
$ make TARGET=MATEKF405
make/tools.mk:78: *** **ERROR** your arm-none-eabi-gcc is '7.3.1', but '8.2.1' is expected. Override with 'GCC_REQUIRED_VERSION' in make/local.mk or run 'make arm_sdk_install' to install the right version automatically in the tools folder of this repo. Stop.
```

We must  either install the iNav toolchain cross-compiler (preferred option) or define the distro cross-compiler.

## Compiler choices

### Installing the iNav preferred cross compiler.

In the iNav directory, issue the command `make arm_sdk_install`. The output will be something like the following (only hopefully you do not suffer from "rural broadband"):

```
$ make arm_sdk_install
mkdir -p tools
mkdir -p downloads
Warning: Illegal date format for -z, --time-cond (and not a file name). 
Warning: Disabling time condition. See curl_getdate(3) for valid date syntax.
  % Total    % Received % Xferd  Average Speed   Time    Time     Time  Current
                                 Dload  Upload   Total   Spent    Left  Speed
100   244  100   244    0     0    645      0 --:--:-- --:--:-- --:--:--   643
100  102M  100  102M    0     0  1385k      0  0:01:15  0:01:15 --:--:-- 1392k

```

Note that the `curl` warning will only occur the first time (as there is no pre-existing iNav-specific cross-compiler installed).

### Using the Distro compiler.

If your distro provides a more recent cross compiler, or you're using a host platform other than x86_64 you may choose to use your distro compiler (or have no choice in the case of "not x86_64").

You must first install the distro compiler: Arch is the most likely to provide a more modern cross compiler, which would need to be installed as `sudo pacman -S arm-none-eabi-gcc arm-none-eabi-binutils arm-none-eabi-newlib`.

There are two options to define the distro compiler version:

* Invoke `make` with the variable `GCC_REQUIRED_VERSION` set to the distro version. This can be done as:

  ```
  $ make GCC_REQUIRED_VERSION=$(arm-none-eabi-gcc -dumpversion)
  ```
  For convenience, you can create an `alias` or define a shell function. The shell function will be evaluated each time it is invoked, while the alias will be evaluated once (typically at login), for example:

  ```
  # bash function, typically in ~/.bashrc
  $ makefc() { make GCC_REQUIRED_VERSION=$(arm-none-eabi-gcc -dumpversion) $@; }
  export -f makefc

  # or, bash alias, typically in ~/.bash_aliases
  $ alias makefc="make GCC_REQUIRED_VERSION=$(arm-none-eabi-gcc -dumpversion) $@"
  ```

  then e.g `$ makefc TARGET=MATEKF405`

* or, create the file `make/local.mk` defining the local distro compiler version.

   ```
   $ echo GCC_REQUIRED_VERSION=$(arm-none-eabi-gcc -dumpversion) > make/local.mk
   ```
   then use `make` normally, `$ make TARGET=MATEKF405`

If you define the distro cross-compiler version in `make/local.mk`, you will need to update `make/local.mk` whenever your disto updates the cross-compiler.

## Building the firmware

By default, `make` builds the REVO target, to build another target, specify the target name to the make command, for example:
```
make TARGET=MATEKF405
```
The resultant hex file are in the `obj/` directory.

You can use the INAV Configurator to flash the local ```obj/inav_TARGET.hex``` file, or use `stm32flash` or `dfu-util` directly from the command line.

[msp-tool](https://github.com/fiam/msp-tool) and [flash.sh](https://github.com/stronnag/mwptools/blob/master/docs/MiscTools.asciidoc#flashsh) provide / describe 3rd party helper tools for command line flashing.

## Updating and rebuilding

In order to update your local firmware build:

* Navigate to the local iNav repository
* Use the following steps to pull the latest changes and rebuild your local version of iNav:

```
$ cd inav
$ git pull
$ make TARGET=<TARGET>
```

## Advanced Usage

For more advanced development information and `git` usage, please refer to the [development guide](https://github.com/iNavFlight/inav/blob/master/docs/development/Development.md).


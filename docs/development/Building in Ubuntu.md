# Building in Ubuntu

iNav requires a reasonably modern `gcc-arm-none-eabi` compiler. As a consequence of the long term support options in Ubuntu, it is possible that the distribution compiler will be too old to build iNav firmware. For example, Ubuntu 16.04 LTS ships with version 4.9.3 which cannot compile contemporary iNav.

As of August 2018, the recommendation for Ubuntu releases is:

| Release | Compiler Source |
| ------- | --------------- |
| 16.04 or earlier | Use the 'official' PPA |
| 17.10 | Use the 'official' PPA as the distro compiler (5.4) *may* be too old |
| 18.04 | Use the 'official' PPA, as the distro compiler (6.3) was broken when last tested |

For Ubuntu derivatives (ElementaryOS, Mint etc.), please check the distro provided version, and if it's lower than 6, use the PPA.

e.g. ElementaryOS

```
$ apt show gcc-arm-none-eabi
...
Version: 15:4.9.3+svn231177-1
```

This 4.9.3 and will not build iNav, so we need the PPA.

## Installer commands

Older versions of Debian / Ubuntu and derivatives use the `apt-get` command; newer versions use `apt`. Use the appropriate command for your release.

# Prerequisites

Regardless of the cross-compiler version, it is necessary to install some other tools:

```
sudo apt install git
sudo apt install gcc
sudo apt install ruby
```

A ruby release of at least 2 or later is recommended, if your release only provides 1.9, it is necessary to install a later version:

```
sudo apt-get remove ruby
sudo apt-add-repository ppa:brightbox/ruby-ng
sudo apt-get update
sudo apt-get install ruby2.4 ruby2.4-dev
```

# Using the Distro compiler

In case Ubuntu ever provides a modern compiler (as of August 2018, not recommended):

```
sudo apt install gcc-arm-none-eabi
```

# Using the PPA compiler

The PPA compiler is recommended for all cases:

```
sudo apt-get remove binutils-arm-none-eabi gcc-arm-none-eabi
sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install gcc-arm-embedded
```

After these steps, on Ubuntu 16.04, (at least of March 2018) you should now have:

```
$ arm-none-eabi-gcc -dumpversion
7.2.1
```

Which is more than adequate for our purposes.

# Building from the Github repository

After the ARM cross-compiler toolchain from is installed, you should be able to build from source.

```
mkdir src
cd src
git clone https://github.com/iNavFlight/inav.git
cd inav
make
```

If you have a github account with registered ssh key you can replace the `git clone` command with  `git clone git@github.com:iNavFlight/inav.git` instead of the https link.

By default, this builds the REVO target, to build another target, specify the target name to the make command, for example:
```
make TARGET=MATEKF405
```
The resultant hex file are in the `obj/` directory.

You can use the INAV Configurator to flash the local ```obj/inav_TARGET.hex``` file, or use `stm32flash` or `dfu-util` directly from the command line.

[msp-tool](https://github.com/fiam/msp-tool) and [flash.sh](https://github.com/stronnag/mwptools/blob/master/docs/MiscTools.asciidoc#flashsh) provide / describe 3rd party helper tools for command line flashing.

# Updating and rebuilding

In order to update your local firmware build:

* Navigate to the local iNav repository
* Use the following steps to pull the latest changes and rebuild your local version of iNav:

```
cd src/inav
git pull
make TARGET=<TARGET>
```

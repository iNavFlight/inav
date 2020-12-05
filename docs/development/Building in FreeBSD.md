# Building in FreeBSD

In order to build the inav firmware in FreeBSD, it is recommended to install [Linux Binary Emulation](https://www.freebsd.org/doc/handbook/linuxemu.html). This will enable you to use the project recommended ARM cross-compiler. The cross-compiler available in Ports tends to be too old to build inav firmware.

* Install Linux binary emulation
* Install the following packages (`pkg` provides suitable versions)
 - `git`
 - `cmake`
 - `make`
 - `ruby`

* Follow the [Building in Linux](Building%20in%20Linux.md) guide.

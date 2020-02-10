# Hardware Debugging

Hardware debugging allows debugging the firmware with GDB, including most of its
features that you can find while debugging software for a computer like setting
breakpoins or printing variables or stepping through the code.

Additionally, firmware can also be flashed directly either from the IDE or from GDB,
significanly reducing the time required for the compile/flash/test cycle.

## Required Hardware

Although more complex and expensive solutions exists, an STLink V2 clone will let you
use all the features of hardware debugging. They can be purchased on any of the typical
Chinese sites.

[ST Link V2 Clone](https://inavflight.com/shop/s/bg/1177014)
[Original ST Link V2](https://inavflight.com/shop/s/bg/1099119)

Additionally, most nucleo boards from ST come with a brekable part that contains an
STLink V2.1 or V3. These can also be used to debug an FC, but can be more difficult to
source.

To connect it a flight controller, you need to locate the SWDIO and SWCLK pins from the
MCU. These correspond to PA13 (SWDIO) and PA14 (SWCLK). Be aware that not all manufacturers
break out these pins, but a lot of them put them in small pads available somewhere.
Connect SWDIO, SWCLK and GND from the FC to pins with the FC

TODO: Add pictures of several FCs with SWDIO and SWCLK highlighted.

## Required software

Besides an ARM toolchain, [OpenOCD](http://openocd.org) is required. Note that at the
time of this writing, OpenOCD hasn't had a release in almost 3 years, so you might
need to look for unofficial releases or compile from source.

[stlink](https://github.com/texane/stlink), while not strictly required, can be handy
for quickly testing the SWD connection or flashing or erasing. To avoid ambiguities
between the hardware and the software, the former will be referred as `ST Link` while
we'll use `stlink` for the latter.

Please, follow the installation instructions for your operating system.

### Windows

Install the Windows Subsystem for Linux, then follow the Linux instructions.

### macOS

Install [Homebrew](https://brew.sh) (a package manager) first.

To install OpenOCD type `brew install open-ocd --HEAD` in a terminal. Note the `--HEAD`
command line switch.

For stlink, use `brew install stlink`.

### Linux

Install [Homebrew for Linux](https://docs.brew.sh/Homebrew-on-Linux), since versions
provided by your distro's package manager might be out of date. Homebrew can cohexist
with your existing package manager without any problems.

Then, follow the same instructions for installing OpenOCD and stlink for macOS.

## Hardware setup

Connect SWDIO and SWCLK from the FC to pins with the same label on the ST Link. You must
also connect one of the GND from FC to any of the GND pins to the ST Link. Note the
following caveats:

- There are several ST Link clone types with different pinouts. Pay attention to the pin
labels.
- In some ST Link clones, some GND pins are actually floating and not connected to
- anything. Use a multimeter to check the GND pins and use any of the valid ones.
- Even if you're powering everything from the same computer, make sure to directly connect
the grounds from the FC to the ST Link. Some FC/stlink combinations have a 0.1-0.2V
difference between their grounds and if you don't connect them, stlink won't work.

The FC can be powered by any power source that it supports (battery, USB, etc...), just
make sure to not connect power from the ST Link (the pins labelled as 3.3V and 5V) to the
FC if something else is powering it.

Once you're wired everything, test the connections with a DMM before applying power. Then
power both the FC and the stlink (the order doesn't matter) and run `st-info --probe`
You should see something like:

```
Found 1 stlink programmers
 serial: 0d0d09002a12354d314b4e00
openocd: "\x0d\x0d\x09\x00\x2a\x12\x35\x4d\x31\x4b\x4e\x00"
  flash: 524288 (pagesize: 16384)
   sram: 131072
 chipid: 0x0431
  descr: F4 device (low power) - stm32f411re
```

## Compilation options

INAV is compiled with debug symbols by default, since they're only stored in the locally
generated `.elf` file and they never use flash space in the target. However, some
optimizations like inlining and LTO might rearrange some sections of the code enough
to interfere with debugging. All compile time optimizations can be disabled by
using `DEBUG=GDB` when calling `make`.

You may find that if you compile all the files without optimizations the program might
too big to fit on the target device. In that case, one of the possible solutions is
compiling all files with optimization (`make clean`, `make ...`) first, then re-save
or `touch` the files you want to be able to step though and then run `make DEBUG=GDB`.
This will then re-compile the files you're interested in debugging with debugging symbols and you will get a smaller binary file which should then fit on the device.

## Debugging

To run a debug session, you will need two terminal windows. One will run OpenOCD, while
the other one will run gdb.

Although not strictly required, it is recommended to set the target you're working on
in `make/local.mk` (create it if it doesn't exist), by adding a line like e.g.
`TARGET ?= SOME_VALID_TARGET`. This way you won't need to specify the target name in
all commands.

From one of the terminals, type `make openocd-run`. This will start OpenOCD and connect
to the MCU. Leave OpenOCD running in this terminal.

From another terminal, type `make gdb-openocd`. This will compile the `.elf` binary for
the current target and start `gdb`. From there you will usually want to execute the gdb
`load` command first, which will flash the binary to the target. Once it finishes, start
running it by executing the `continue` command.

For conveniency, you can invoke `make gdb-openocd` with the environment variable `$LOAD`
set to a non-empty string (e.g. `LOAD=1 make gdb-openocd`), which will run the `load`
command and flash the target as soon as gdb connects to it.

From there on, you can use any gdb commands like `b` for setting breakpoints, `p` for
printing, etc... Check a gdb tutorial for more details if you're not already familiar
with it.

### Rebuilding and reflashing

To rebuild, flash and rerun the binary after doing any modifications, recompile it
with `make`, then press `control+c` to interrupt gdb. Halt the target by entering the
gdb command `monitor reset halt` and then type `load` to flash it. gdb will notice the
binary has changed and re-read the debug symbols. Then you can restart the firmware with
`continue`. This way, you can very quickly flash, upload and test since neither OpenOCD
nor gdb need to be restarted.

### ST Link versions

By default, the Makefiles will assume an ST Link v2, which is the version found in the
popular and cheap clones. However, other versions are also supported. Just set the
`STLINK` environment variable (either via command line or either via `local.mk`) to
`1` or `2` or `2.1`, according to your hardware.

### Semihosting

Semihosting is an ARM feature that allows printing messages via the SWD connection.
The logging framework inside INAV can output its messages via semihosting. To enable
it, make sure you've deleted all generated files (e.g. `make clean`) and set the
environment variable `$SEMIHOSTING` to a non-empty string, either via command line
or via `local.mk`. Once you start the target, log messages will appear on the openocd
terminal. Note that even with semihosting enabled, logging has be explicitely enabled
via settings.

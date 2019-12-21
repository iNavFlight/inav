## Overview

iNav (after 2.3.0) offers USB MSC (mass storage device class) SD card and internal flash access, meaning you can mount the FC (SD card / internal flash) as an OS file system via USB to read BB logs (and delete them from an SD card).

When MSC mode is used with **internal flash** there are a few differences compared to **SD card** as it's a virtual file system:

* The file system is read-only. In order to delete logs it is necessary to erase the flash as usual (configurator, CLI or other tool).
* The logs are presented as a single, consolidated file (`inav_all.bbl`) as well as individual logs (`inav_001.bbl` etc.).
* Other informative files (e.g. `readme.txt`) may also exist in the virtual file system.


## Usage

To put the FC in MSC mode:

* Enter the CLI
* Enter the CLI command `msc` ; the FC will reboot
* Close the CLI tool (`cliterm`, configurator etc.)
* Wait for the device to be recognised as USB storage device by the operating system (may take some time, 10-15 seconds perhaps).
* Copy files off the MSC mounted FC (sd card) (`cp`, file manager)
* Dismount / eject the FC (sd card) card using the standard OS method
* Power-cycle the FC to exit MSC mode.

## Performance

Internal flash is quite fast.

For an SD card, reading is quite slow, typically c. 340kBs, for example:

```
####################
## Using MSC mode ##
####################
# FC is automounted to /run/media/jrh/BBOX-QUAD by the OS
$ rsync -P /run/media/jrh/BBOX-QUAD/LOGS/LOG00035.TXT /tmp/msclogs/
LOG00035.TXT
     55,856,827 100%  339.15kB/s    0:02:40 (xfr#1, to-chk=0/1)
```

```
#########################
## Using a card-reader ##
#########################
# SD Card is automounted to /run/media/jrh/BBOX-QUAD by the OS
$ rsync -P /run/media/jrh/BBOX-QUAD/LOGS/LOG00035.TXT /tmp/sdclogs/
LOG00035.TXT
     55,856,827 100%   19.26MB/s    0:00:02 (xfr#1, to-chk=0/1)
```
i.e c. 2.5 seconds for the card reader, 2 minutes 40 seconds for MSC (60 times slower). However, if the card is relatively inaccessible, this is a reasonable trade-off

## Comparison and Integrity

The same file (`LOG00035.TXT`, c 55MB) is copied by MSC to `/tmp/msclogs` and directly (SD Card Reader) to `/tmp/sdclogs`.

```
$ cmp /tmp/{msc,sdc}logs/LOG00035.TXT
# no differences reported ...
```

```
$ md5sum /tmp/{msc,sdc}logs/LOG00035.TXT
7cd259777ba4f29ecbde2f76882b1840  /tmp/msclogs/LOG00035.TXT
7cd259777ba4f29ecbde2f76882b1840  /tmp/sdclogs/LOG00035.TXT
```
You should also be able to run blackbox utilities (e.g. the iNav specific `blackbox_decode`) without errors on the files, e.g.

```
$ blackbox_decode --stdout --merge-gps > /dev/null /tmp/msclogs/LOG00035.TXT
Log 1 of 1, start 36:00.888, end 62:00.851, duration 25:59.963

Statistics
Looptime           1006 avg            9.2 std dev (0.9%)
I frames   48405  104.6 bytes avg  5062215 bytes total
P frames  726064   69.2 bytes avg 50246994 bytes total
H frames     380   10.0 bytes avg     3800 bytes total
G frames   15674   21.4 bytes avg   334701 bytes total
S frames    6198   33.0 bytes avg   204534 bytes total
Frames    774469   71.4 bytes avg 55309209 bytes total
Data rate  496Hz  35806 bytes/s     358100 baud

3 frames failed to decode, rendering 4 loop iterations unreadable. 4 iterations are missing in total (4ms, 0.00%)
774472 loop iterations weren't logged because of your blackbox_rate settings (779980ms, 50.00%)
```
## Developer Notes

Providing MSC for a target requires that the `.mk` file includes in `FEATURES` the key `MSC` and at least one of `ONBOARDFLASH` and /or `SDCARD`.

For F4 and F7 targets, `USE_USB_MSC` is set unconditionally in `common.h`; if your target does not support blackbox logging to either SD card or internal flash, you should over-ride this in `target.h`
```
#ifdef USE_USB_MSC
# undef USE_USB_MSC
#endif
```
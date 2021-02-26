# Building in Windows 10 with Linux subsystem [Recommended]

Linux subsystem for Windows 10 is probably the simplest way of building INAV under Windows 10.

## Setting up the environment

Enable WSL:
run `windows features`
enable `windows subsytem for linux`
reboot


Install Ubuntu:
1.  Go to Microsoft store https://www.microsoft.com/en-gb/store/b/home
1.  Search and install most recent Ubuntu LTS version
1.  When download completed, select `Launch Ubuntu`
1.  When prompted enter a user name and password which you will need to remember
1.  When complete, the linux command prompt will be displayed

NOTE: from this point all commands are entered into the Ubunto shell command window

Update the repo packages:
1.  `sudo apt update`

Install Git, Make, gcc and Ruby
1.  `sudo apt-get install git`
1.  `sudo apt-get install make`
1.  `sudo apt-get install cmake`
1.  `sudo apt-get install ruby`

### CMAKE and Ubuntu 18_04

To run  `cmake` in the latest version you will need to update from Ubuntu `18_04` to `20_04`. The fastest way to do it is to uninstall current version and install `20_04` for Microsoft Store [https://www.microsoft.com/store/productId/9N6SVWS3RX71](https://www.microsoft.com/store/productId/9N6SVWS3RX71) 

## Downloading the iNav repository (example):

Mount MS windows C drive and clone iNav
1.   `cd /mnt/c`
1.   `git clone https://github.com/iNavFlight/inav.git`

You are ready!
You now have a folder called inav in the root of C drive that you can edit in windows

### If you get a cloning error

On some installations, you may see the following error:
```
Cloning into 'inav'...
error: chmod on /mnt/c/inav/.git/config.lock failed: Operation not permitted
fatal: could not set 'core.filemode' to 'false'
```

You can fix this with by remounting the drive using the following commands
1. `sudo umount /mnt/c`
2. `sudo mount -t drvfs C: /mnt/c -o metadata`

## Building (example):

For detailed build instrusctions see [Building in Linux](Building%20in%20Linux.md)

Launch Ubuntu:
Click Windows Start button then scroll and lauch "Ubuntu"

Building from Ubuntu command line

`cd /mnt/c/inav`

Do it onece to prepare build environment
```
mkdir build
cd build
cmake ..
```

Then to build
```
cd build
make MATEKF722
```

## Flashing:
Launch windows configurator GUI and from within the firmware flasher select `Load firmware[Local]`
Hex files can be found in the folder `c:\inav\build`

## Troubleshooting

### Syntax error: "(" unexpected

```
dzikuvx@BerlinAtHome:/mnt/c/Users/pspyc/Documents/Projects/inav/build$ make MATEKF722SE
Generating MATEKF722SE/settings_generated.h, MATEKF722SE/settings_generated.c
/bin/sh: 1: Syntax error: "(" unexpected
make[3]: *** [src/main/target/MATEKF722SE/CMakeFiles/MATEKF722SE.elf.dir/build.make:63: src/main/target/MATEKF722SE/MATEKF722SE/settings_generated.h] Error 2
make[2]: *** [CMakeFiles/Makefile2:33607: src/main/target/MATEKF722SE/CMakeFiles/MATEKF722SE.elf.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:33290: src/main/target/MATEKF722SE/CMakeFiles/MATEKF722SE.dir/rule] Error 2
make: *** [Makefile:13703: MATEKF722SE] Error 2
```

This error can be triggered by a Windows PATHs included in the Linux Subsystem. The solution is:

#### For WSL V1 - Flags set as 7 by default

1. Open Windows RegEdit tool
1. Find `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{GUID}\Flags`
1. Change `Flags` from `7` to `5`
1. Restart WSL and Windows preferably
1. `cd build`
1. `cmake ..`
1. `make {TARGET}` should be working again 

#### For WSL V2 - Flags set as 0x0000000f (15) by default
1. Open Windows RegEdit tool
1. Find `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Lxss\{GUID}\Flags`
1. Change `Flags` from `f` to `d`, it is stored as Base Hexadecimal
1. Restart WSL and Windows preferably
1. `cd build`
1. `cmake ..`
1. `make {TARGET}` should be working again 

#### Or, for either version
1. In the Linux Subsystem, `cd /etc/`
2. Create a new file with `sudo nano wsl.conf`
3. Enter the following in to the new file:
```
[Interop]
appendWindowsPath=false
```
4. Save the file by holding `Ctrl` and pressing `o`
5. Press `Enter` to confirm the wsl.conf filename.
6. Hit `Ctrl`+`x` to exit nano
7. Restart WSL and Windows preferably
8. `cd build`
9. `cmake ..`
9. `make {TARGET}` should be working again 

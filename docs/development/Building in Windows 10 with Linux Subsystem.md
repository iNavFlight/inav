# Building in Windows 10 with Linux subsystem

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
1.  `sudo apt-get install gcc-arm-none-eabi`
1.  `sudo apt-get install ruby`

## Downloading the iNav repository (example):

Mount MS windows C drive and clone iNav
1.   `cd /mnt/c`
1.   `git clone https://github.com/iNavFlight/inav.git`

You are ready!
You now have a folder called inav in the root of C drive that you can edit in windows


## Building (example):

Launch Ubuntu:
Click Windows Start button then scroll and lauch "Ubuntu"

Building from Ubunto command line
`cd /mnt/c/inav`
`make clean TARGET=OMNIBUSF4PRO` (as an example)
`make TARGET=MATEKF405`  (as an example)


## Flashing:
Launch windows configurator GUI and from within the firmware flasher select `Load firmware[Local]`
Hex files can be found in the folder `c:\inav\obj`

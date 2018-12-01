# Building in Windows 10 with Linux subsystem

Linux subsystem for Windows 10 is probably the simplest way of building INAV under Windows 10.

1. Enable WSL (_Windows Subsystem for Linux) using any guide from internet. [This](https://winaero.com/blog/enable-wsl-windows-10-fall-creators-update/) is up to date step by step (January 2018)
1. From _Windows Store_ install `Ubuntu`
1. Start `Ubuntu` and run:
1. `sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa`
1. `sudo apt-get update`
1. `sudo apt-get install gcc-arm-embedded make ruby`

At the moment (January 2018) it will install `gcc-arm-none-eabi` in version _7 q4_

From this moment INAV can be build using the following command

`make TARGET={TARGET_NAME}`

Of course, replace `{TARGET_NAME}` with a target you want to compile
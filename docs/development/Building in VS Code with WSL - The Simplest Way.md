# Building in VS Code with WSL [The Simplest Way]

Linux subsystem for Windows 10 is probably the simplest way of building INAV under Windows 10. It's so simple, that any lay user who is not used to the development environment can do it!

## Setting up the environment

1. Open the VS Code
2. Find and open the "Extensions" tab (Or from a Ctrl + Shift + X)
3. Look for and install the "Remote-WSL" extension
4. Open a new terminal (Ctrl + Shift + ' ) and type the command 'wsl'.

## Downloading the iNav repository:

Mount MS windows C drive and clone iNav (Yes, it has to be direct on disk C, or D, or E, or others...). You will only do this part once!
One command at a time (not all at once).
1. `cd /mnt/c`
2. `git clone https://github.com/iNavFlight/inav.git`
3. `cd /mnt/c/inav`
4. `mkdir build`
5. `cd build`
6. `cmake ..`

## Building:

To compile for a target:

1. `cd build`
2. `make MATEKF405SE`

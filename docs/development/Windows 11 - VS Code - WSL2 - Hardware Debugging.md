# Environment prerequistes
- Windows 10/11
- WSL2 (Ubuntu 20.04), [how to install WSL2](https://docs.microsoft.com/en-us/windows/wsl/install)
  - **ATTENTION:** Works ONLY on WSL2 Kernel version 5.10.60.1 or later, check it with command `uname -a` and if it is lower - update WSL2
    - Using Windows Update: Start > Settings > Windows Update > Advanced Options > Receive updates for other Microsoft products SHOULD be Turned ON
      - Start > Settings > Windows Update > Check for updates
    - Using Elevated Command Prompt: wsl --update
    - NOTE: If your WSL is V1 -> Switch it to V2 or install new WSL (Ubuntu 20.04) instance (switching are done with command `wsl --set-default-version 2`)
- ST-Link V2 Debugger
- INAV Project are Cloned from GitHub to WSL2 storage space, look at:
  - [IDE - Visual Studio Code with Windows 10](https://github.com/iNavFlight/inav/blob/master/docs/development/IDE%20-%20Visual%20Studio%20Code%20with%20Windows%2010.md)
  - [Building in Windows 10 or 11 with Linux Subsystem](https://github.com/iNavFlight/inav/blob/master/docs/development/Building%20in%20Windows%2010%20or%2011%20with%20Linux%20Subsystem.md)
  - [Hardware Debugging](https://github.com/iNavFlight/inav/blob/master/docs/development/Hardware%20Debugging.md)

# Installation
## WSL2 Setup
### Install prerequistes
- `sudo apt install linux-tools-5.4.0-77-generic hwdata`
- `sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/5.4.0-77-generic/usbip 20`
- `sudo apt install libncurses5`
### Adjust rules to allow connecting with non-root
The following script creates access rules for ST-LINK V2, V2.1 and V3 and for some USB-to-serial converters, download it to new file under you home folder, save, change attributes and run
- `nano createrules.sh`
- Copy and Paste script below
- Ctrl-X
- Y
- Enter
- `sudo chmod +x createrules.sh`
- `./createrules.sh`
```
#!/bin/bash

sudo tee /etc/udev/rules.d/70-st-link.rules > /dev/null <<'EOF'
# ST-LINK V2
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3748", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv2_%n"

# ST-LINK V2.1
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374b", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv2-1_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3752", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv2-1_%n"

# ST-LINK V3
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374d", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3loader_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374e", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="374f", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3_%n"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="3753", MODE="600", TAG+="uaccess", SYMLINK+="stlinkv3_%n"
EOF

sudo tee /etc/udev/rules.d/70-usb-to-serial.rules > /dev/null <<'EOF'
# CP2101 - CP 2104
SUBSYSTEMS=="usb", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", MODE="600", TAG+="uaccess", SYMLINK+="usb2ser_%n"

# ATEN UC-232A
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0557", ATTRS{idProduct}=="2008", MODE="600", TAG+="uaccess", SYMLINK+="usb2ser_aten_%n"
EOF

sudo udevadm control --reload-rules
```
#### In case you want reload services manually later
- `sudo service udev restart`
- `udevadm control --reload`

## Windows Setup
- Download last usbipd from [here](https://github.com/dorssel/usbipd-win/releases)
- Install it

## VS Code Setup
- INAV project is on WSL drive
- Folder opened using WSL Remote extension
- Cortex Debug extension installed
- CMake will install all dependencies after first build automatically
- Use configuration files provided below, place them to `.vscode` subfolder (create it if required)

# Debugging
## Preparing for debugging (before each new debug session)
### Open WSL2 prompt
- Go to project folder
- `lsusb`
- `sudo /lib/systemd/systemd-udevd --daemon`
- `sudo udevadm control --reload-rules && udevadm trigger`
- REPLUG USB
### Open Elevated Command Prompt (Run as Administrator)
- `usbipd wsl list` - shows you plugged and accessible USB devices
- `usbipd wsl attach --busid ID_OF_DEVICE_FROM_FIRST_COMMAND` - will attach USB device to WSL
- Just for info: `usbipd detach --busid ID_OF_DEVICE_FROM_FIRST_COMMAND` - will deattach USB device from WSL
### Back to WSL2 prompt
- `lsusb` - should show you just attached USB device
- `st-info -probe` - should "see" ST-Link and MCU

#### Leave Command Prompt and WSL Prompt minimized (for later usage)
#### **NOTE:** Due to some USB reconnect issues, sometimes, is need to execute `usbipd wsl list` and `usbipd wsl attach...` commands again, to reconnect ST-Link to WSL

## Debugging
- Connect SWD from ST-Link to FC board (at least GND, SWDIO and SWCLK should be connected, but connecting Vref to +3.3V pad and RESET accordingly will improve debugging stability a lot!)
- Power FC (can be powered from USB)
- Use VS Code Run -> Start Debugging (F5) menu

# Troubleshooting
- OpenOCD shows Permission denied during "Flashing":
  - Go to WSL, INAV Project folder and run `cd src/utils` and `sudo chmod +x *`
- If running from WSL itself and get errors:
  - Go to WSL and run
    - `sudo apt install gdb-multiarch`
    - `sudo ln -s /usr/bin/gdb-multiarch /usr/bin/arm-none-eabi-gdb`

# References
- https://github.com/dorssel/usbipd-win/wiki/WSL-support
- https://devblogs.microsoft.com/commandline/connecting-usb-devices-to-wsl/#:~:text=Select%20the%20bus%20ID%20of,to%20run%20a%20sudo%20command.&text=From%20within%20WSL%2C%20run%20lsusb,it%20using%20normal%20Linux%20tools.
- https://calinradoni.github.io/pages/200616-non-root-access-usb.html

# VS Code Example configurations
`launch.json`
```
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387

    // ******* INAV ********
    // Define the following values in settings.json
    //      - BUILD_DIR: Relative path to the build directory
    //      - TARGET: Target name that you want to launch

    "version": "0.2.0",
    "configurations": [
        {
            "name": "Cortex Debug",
            "cwd": "${workspaceRoot}",
            "executable": "${config:BUILD_DIR}/bin/${config:TARGET}.elf",
            "request": "launch",
            "type": "cortex-debug",
            "servertype": "openocd",
            "device": "${config:TARGET}",
            "configFiles": [
                "${config:BUILD_DIR}/openocd/${config:TARGET}.cfg"
            ],
            "preLaunchTask": "openocd-debug-prepare",
            "svdFile": "${config:BUILD_DIR}/svd/${config:TARGET}.svd",
        }
    ]
}
```

`settings.json`
```
{
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "files.associations": {
        "general_settings.h": "c",
        "parameter_group.h": "c"
    },
    "BUILD_DIR": "build",
    "TARGET": "MAMBAF405"
}
```

`tasks.json`
```
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "options": {
        "env": {
            "TARGET": "${config:TARGET}",
        }
    },
    "tasks": [
        {
            "label": "target",
            "type": "shell",
            "command": "make", "args": ["-C", "${config:BUILD_DIR}", "${config:TARGET}"],
            "problemMatcher": "$gcc",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
            }
        },
        {
            "label": "flash",
            "type": "shell",
            "command": "make", "args":  ["-C", "${config:BUILD_DIR}", "openocd_flash_${config:TARGET}"],
            "dependsOn": "elf"
        },
        {
            "label": "svd",
            "type": "shell",
            "command": "make", "args":  ["-C", "${config:BUILD_DIR}", "svd_${config:TARGET}"],
            "problemMatcher": []
        },
        {
            "label": "openocd-cfg",
            "type": "shell",
            "command": "make", "args":  ["-C", "${config:BUILD_DIR}", "openocd_cfg_${config:TARGET}"],
            "problemMatcher": []
        },
        {
            "label": "openocd-debug-prepare",
            "type": "shell",
//            "dependsOn": ["svd", "openocd-cfg", "flash"],
            "dependsOn": ["svd", "openocd-cfg"],
            "problemMatcher": []
        }
    ]
}
```

`cpp_properties.json`
```
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceRoot}",
                "${workspaceRoot}/src/main/**"
            ],
            "browse": {
                "limitSymbolsToIncludedHeaders": false,
                "path": [
                    "${workspaceRoot}/**"
                ]
            },
            "intelliSenseMode": "msvc-x64",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "defines": [
                "NAV_FIXED_WING_LANDING",
                "USE_OSD",
                "USE_GYRO_NOTCH_1",
                "USE_GYRO_NOTCH_2",
                "USE_DTERM_NOTCH",
                "USE_ACC_NOTCH",
                "USE_GYRO_BIQUAD_RC_FIR2",
                "USE_D_BOOST",
                "USE_SERIALSHOT",
                "USE_ANTIGRAVITY",
                "USE_ASYNC_GYRO_PROCESSING",
                "USE_RPM_FILTER",
                "USE_GLOBAL_FUNCTIONS",
                "USE_DYNAMIC_FILTERS",
                "USE_IMU_BNO055",
                "USE_SECONDARY_IMU",
                "USE_DSHOT",
                "FLASH_SIZE 480",
                "USE_I2C_IO_EXPANDER",
                "USE_PCF8574",
                "USE_ESC_SENSOR"
            ],
            "configurationProvider": "ms-vscode.cmake-tools"
        }
    ],
    "version": 4
}
```

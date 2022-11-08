# Responsibility and resource
This is the unofficial target for INAV( Original source: https://github.com/iNavFlight/inav). Provides support for flight control boards that do not have official INAV support for the STM32F411CEU6 board. 
Whether or not to use this firmware is the user's responsibility and is free to do so. INAV developer link : https://github.com/iNavFlight/inav/tree/master/docs/development You can find all the details for firmware development here.

# STM32F411CEU6 INAV Firmware
STM32F411CEU6 Board Firmware
First, let's connect the board and the computer.
![image](https://user-images.githubusercontent.com/19993109/139479391-49dafee0-a7da-49ae-9196-10a578d4ac55.png)

# Download INAV Configurator
https://github.com/iNavFlight/inav-configurator/releases

## Windows
Download Configurator for Windows platform (win32 or win64 is present)
Extract ZIP archive
Run INAV Configurator app from unpacked folder
Configurator is not signed, so you have to allow Windows to run untrusted application. There might be a monit for it during first run

# Install DFU Drivers (DFU mode)
## ImpulseRC Driver Fixer
https://impulserc.blob.core.windows.net/utilities/ImpulseRC_Driver_Fixer.exe
* Start ImpluseRC Driver Fixer
* Connect the FC USB to the PC While holding the boot button in. (DO NOT power on FC via external 5V or Vbat)
* The ImpulseRC Driver Fixer should then see and load the proper driver
* Start INAV configurator
* Connect the FC USB to the PC while holding the boot button in.
* INAV configurator should show it’s connected in DFU mode in the top right corner (DO NOT click the CONNECT button)
* Choose the latest hex file for your FC and then “Load Firmware local”. Once loaded, click “Flash Firmware”.

# Setup sensors video
https://youtu.be/HbSUMauSkiw

[![sensors setup](https://user-images.githubusercontent.com/19993109/143448588-6d599bb3-b89d-479a-997a-c6c4c3e21fea.png)](https://youtu.be/HbSUMauSkiw "sensors setup")


# STM32F411CEU6 PINOUT Connection
![image](https://user-images.githubusercontent.com/19993109/139479854-9793e17c-1e2a-4ccc-8b5f-ec23026710fd.png)

# GY-87 10DOF Module MPU6050 HMC5883L BMP180
![image](https://user-images.githubusercontent.com/19993109/139479938-a1166d41-17c8-41a2-8903-195406ecd020.png)

# Module Connection
![image](https://user-images.githubusercontent.com/19993109/139481190-abcfcc7e-f293-4a88-a734-72a122daef17.png)

# Receiver Connection
![image](https://user-images.githubusercontent.com/19993109/139479978-5e0735b5-34c6-4752-b130-f54a79ec9ce5.png)

![image](https://user-images.githubusercontent.com/19993109/139480054-d270bc46-24c8-4c49-a4b3-eb6e3ae2ea32.png)

# INAV Configurator IBUS Telemetry Setup

![image](https://user-images.githubusercontent.com/19993109/148845453-e8c84093-4cae-4f5f-9f57-10febcd7216a.png)

# IBUS Receiver Pins and IBUS Telemetry input Receiver
![image](https://user-images.githubusercontent.com/19993109/148845532-a4476a7b-ea24-4711-903e-6e2f61280f9a.png)

## SOFTSERIAL_1_TX_PIN     PB6
![image](https://user-images.githubusercontent.com/19993109/148846827-12eb543c-f4cb-4ab8-be8f-1b9b10d9a25b.png)

# OSD Connection
![image](https://user-images.githubusercontent.com/19993109/148816930-2e3b88e6-693b-42f9-bf57-801aac6cbe31.png)

# Motors Connection
![image](https://user-images.githubusercontent.com/19993109/139480115-09056969-39fd-42c4-a09e-0201845f48fa.png)

# Hexacopter Drone Frame
![image](https://user-images.githubusercontent.com/19993109/139480161-2b3f7512-6f71-4ec0-8eb6-8fe174400366.png)


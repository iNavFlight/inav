# RealFlight

Supported are RealFlight 9.5S and RealFlight Evolution, NOT RealFlight-X.

RealFlight is very well suited to simulate the model flight specific aspects. Autolaunch and the mixers can be used.

The RealFlight 3D sceneries are based on real topographic data of the Sierra Nevada in Southern Spain.
INAV uses as reference the scenery "RealFlight Ranch" which is located at the coordinates Lat: 37.118949°, Lon: -2.772960.
Use these scenery to use the mission planner and other GPS features.

> [!CAUTION]:
> The immediate surroundings of the airfield have been levelled in the scenery.  If, for example, Autoland is to be tested here, do not use "Sea level ref" and the automatically determined heights of the Configurator. 
> Either use relarive elevations or correct the elevation manually.
> The altitude of the airfield is exactly 1300 metres. 

## Joystick 
In the settings, calibrate the joystick, set it up and assign the axes in the same order as in INAV.
Channel 1 (Aileron) in RealFlight is Cannel 1 (Aileron in INAV) and so on. 

## General settings
Under Settings / Physics / Quality Switch on "RealFlight Link enabled".
As a command line option for SITL, the port does not need to be specified, the port is fixed.
For better results, set the difficulty level to "Realistic". 

## Prepare the models
All mixer and servo influencing settings should be deactivated.
In the model editor under "Electronis" all mixers should be deleted and the servos should be connected directly to the virtual receiver output.
In the "Radio" tab also deactivate Expo and low rates: "Activadd when: Never".
Configure the model in the same way as a real model would be set up in INAV including Mixer, Expo, etc. depending on the selected model in RealFlight.

Then adjust the channelmap im the Configurator or via command line accordingly. 
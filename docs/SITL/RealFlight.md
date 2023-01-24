# RealFlight

Supported are RealFlight 9.5S and RealFlight Evolution, NOT RealFlight-X.

RealFlight is very well suited to simulate the model flight specific aspects. Autolaunch and the mixers can be set in INAV.
However, since the sceneries do not correspond to a real environment, the GPS data must be "faked". The position is always shown somewhere in southern Nevada ;).
GPS data and flight modes work fine though, only for missions with waypoints it is of course not ideal. 

## General settings
Under Settings / Physics / Quality Switch on "RealFlight Link enabled".
As a command line option for SITL, the port does not need to be specified, the port is fixed.
For better results, set the difficulty level to "Realistic". 

## Prepare the models
All mixer and servo influencing settings should be deactivated.
In the model editor under "Electronis" all mixers should be deleted and the servos should be connected directly to the virtual receiver output.
In the "Radio" tab also deactivate Expo and low rates: "Activadd when: Never".
Configure the model in the same way as a real model would be set up in INAV including Mixer, Expo, etc. depending on the selected model in RealFlight.

Then adjust the channelmap (see command line option) accordingly. 
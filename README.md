Project


Goal: 
Add a feature to create a radar map to see others wings and LOW Cost

Material: (<10$)
RF Transmitter (test in progress, Lora, HC12, NRF..)
Arduino
FC with inav :)

Workflow


How to ?
Arduino/RF:
Get FC data over MSP protocol 
BroadCast data to other clients (done)
Reply Data if not concerned (mesh network) (evolution)

Second version (i hope) : without arduino, integrate wireless management in iNav (thx to Jelle)

FC:
Retrieve MSP datas sent by arduino in iNav
Format: 
OSD (see examples below)
First step: Use actual Radar/map osd view and add waypoint of other planes
Arrow white or black if wing is up to you or down to you
2nd Version: create custom OSD view

And now ? Whats DONE ?
I’m actually near finish the electronics/wireless parts, Next : Custom inav !

OSD EXAMPLE
First dev : OSD base on Radar/map current inav view (adding waypoint of others planes)

Nb: white is up to you, black is down to you

OSD futur version (i hope!)

Cross: plane in front of you (you can see it on your camera) 
Circles: planes behind you !

## Features


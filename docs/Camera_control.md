# Camera control

This feature allows the control of cameras such as Caddx Turtle, Rucam models such as Split, Box etc.
It's built on BetaFlights runcam impementation. See https://github.com/betaflight/betaflight/wiki/RunCam-Device-Protocol

## Additional features

This feature ads support for Caddx Turtle v2
- Start and Stop for record
- Photo
- Stick control from transmitter for settings of Caddx Turtle v2

## Hardware setup
Runcam: See the Betaflight link above
Caddx: Connect Only Rx Pad at Caddx Turtle v2 to an unused Tx Pad on your FC.

## Configuration
Select "Runcam" in the configurator for the port that you would like to use.
in CLI: set cam_provider = CADDX_V2
in modes tab assign the function you want to channels/switches:
CAMERA POWER Caddx: Photo shot
CAMERA CHANGE MODE Caddx: Stop recording
CAMERA WI-FI Caddx: Start recording

## Usage
Stick contol/menu control - see the linked Wiki on betaflight above

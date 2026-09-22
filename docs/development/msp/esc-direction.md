# Per-motor ESC direction configuration

Paired firmware/Configurator change for `maintenance-10.x`. Betaflight's Motor
Direction Wizard inspired the workflow; no wizard source was copied.

## Scope

The Outputs tab shows the integrated dialog only for INAV multirotor/tricopter
platforms (`FC.isMultirotor()`), not based on motor count. Airplanes, including
multi-engine models, rovers, boats and unsupported platforms keep the whole panel
hidden and do not start direction polling.

The wizard initializes the selected ESC to Normal, offers a bounded hold-to-test
pulse, reverse and next-motor confirmation. Individual mode changes nothing on
selection; Normal/Reverse are explicit writes. The Quad X diagram uses the existing
INAV mixer image and motor-rule numbering. Other mixers use numbered controls.

## Protocol and ESC storage

Provisional MSP2 codes require maintainer agreement:

- `0x2235` status, ten U8 fields: version=2, available motor count, direction phase,
  motor index, reverse (0/1), direction token, simulated (0/1), test motor index,
  test active (0/1), test token. Count=0 means unavailable. Direction phases are
  0 idle/cancelled, 1 stopping, 2 direction, 3 gap, 4 save, 5 settling, 6 sent.
- `0x2236` set direction: motor, reverse, nonzero token. Repeating the current
  token does not repeat the operation, including after cancellation.
- `0x2237` test: motor, run (0/1), token. Run=1 requests DShot120 for at most
  1.5 seconds. Run=0 stops unconditionally. Repeated tokens cannot extend or
  restart the pulse after stop, expiry or cancellation.

The driver holds unselected motors at zero during the operation, first sends
zero for 1 second, then command 7/8 ten times at >=1ms intervals, waits 10ms,
sends SAVE_SETTINGS (12) ten times and waits >=35ms. Zero frames are suppressed
between repetitions so they cannot reset the ESC's repeat counter.

Requires initialized/enabled STM32/AT32 DShot outputs, no reversible/3D mode and
compatible ESC firmware. RP2350 is unsupported. Direction/save support and
persistent storage cannot be read back; phase 6 means commands sent, not confirmed
ESC storage. Settings live in the ESC, not FC EEPROM. Normal/Reverse are relative
to ESC configuration and wiring, not absolute clockwise/counterclockwise.

## Safety without changing existing arming behavior

All propellers must be removed before powering ESCs and acknowledging the dialog.
Direction writes and tests are refused while armed or ordinary motor testing is
active. Only this operation's active phase temporarily owns the outputs.

**No new arming flag or persistent arming lock is introduced.** The existing arming
rules remain unchanged. If the FC arms, the output driver cancels the operation
before preparing the next frame and returns control to the normal motor path.
Cancellation retains request tokens so delayed retries cannot restart an operation.
An interrupted direction/save sequence has an uncertain result and needs checking.

Ordinary MSP motor-test writes are rejected only during an active direction
sequence or pulse; they work normally afterward. Reboot, EEPROM writes, reset and
passthrough are rejected during active operations because they interrupt the
sequence. Normal DShot/turtle-mode behavior resumes after completion/cancellation.

Release, pointer cancellation/leave, keyboard release, blur, dialog close and tab
cleanup request pulse stop. Release racing with a queued start sends another stop
after that start completes. Firmware expiry does not depend on the UI or USB.

## Validation and manual checks

C tests cover sequence order/repetitions/timing, clock wrap, pulse expiry, explicit
stop, cancellation and token retention. Firmware builds: SITL, SPEEDYBEEF405V4,
IFLIGHT_BLITZ_ATF435. Configurator tests cover parser validity, freshness/safety
checks, tokens and simulated status. Native Electron/SITL checks cover wizard and
individual mode, release/timeout, unchanged ordinary motor-test availability and
platform visibility.

SITL uses the same sequencer and publishes isolated test pulses in the simulator
motor-value range. It reports simulation explicitly and has no physical ESC storage. Hardware validation has not been performed. Before merging:

1. Remove all propellers. Verify mapped output isolation and command timing at
   DShot150/300/600, with burst and non-burst DMA, on STM32 and AT32.
2. Verify requests are refused while armed or during existing motor tests.
   Arm during a direction sequence/pulse: verify cancellation and normal motor
   output ownership without introducing any new arming restriction.
3. Release, close, disconnect USB and hold past the deadline; verify pulse stops.
   Inject duplicate/delayed requests after cancellation: no restart or extra save.
4. Power-cycle ESCs and FC; verify selected direction persisted and other motors
   are unchanged. Check normal motor testing and turtle mode afterward.
5. Check older firmware, disabled outputs, analog PWM, 3D, missing ESC power and
   ESC firmware without direction/save support; do not claim confirmed storage.

The protocol timings follow https://betaflight.com/docs/development/API/Dshot .
Upstream Configurator currently accepts 9.x only while the firmware development
branch now reports 10.0.0. Version-policy changes are deliberately outside this
feature PR. The local native validation package temporarily accepts 10.x; that
packaging-only override is not included in the PR source.

## Review regression coverage

Configurator probes capability once on mounting. Periodic status requests run only
while the dialog is open or an operation needs tracking; unsupported firmware stops
polling. Write acknowledgements have separate nullable state fields and never imply
ESC persistence. Queue drops and MSP errors are failures; unconditional stops retry
up to three times, including after cleanup. The firmware deadline remains the final
guarantee after a total connection loss.

Quad diagrams are bundled directly, including reversed mixer direction, without
copying the asynchronously loaded Outputs preview. Dialog/map sizing is fluid and
text uses relative units; the icon close control has a localized accessible name.

Firmware prepares configuration frames as an overlay without replacing cached
normal outputs. Stop/arming cancellation therefore cannot replay a previous test
value. Ordinary DShot commands remain queued and execute after configuration ends
or arming cancels it, including turtle-mode direction commands.

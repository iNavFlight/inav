# ESC 4-Way Passthrough: Known Issues

This covers two confirmed issues in INAV's ESC 4-way (BLHeli bootloader)
passthrough implementation, along with the reasoning behind the "sends data
before init" behavior some users observe on an oscilloscope, which turns out
to be expected protocol behavior rather than a bug in itself.

## Background: Why It "Sends Data Before Init"

**Files:** `src/main/io/serial_4way.c`, `src/main/io/serial_4way_avrootloader.c`

When a passthrough connection attempt starts, `BL_ConnectEx()` transmits a
17-byte BLHeli bootloader init sequence immediately, without first
handshaking to confirm the ESC is ready:

```c
uint8_t BootInit[] = {0,0,0,0,0,0,0,0,0x0D,'B','L','H','e','l','i',0xF4,0x7D};
BL_SendBuf(BootInit, 17);
```

(An alternate 21-byte form exists behind `USE_SERIAL_4WAY_SK_BOOTLOADER`, used
by some derivative bootloaders — the 17-byte form above is what ships by
default.)

This is by design in the BLHeli bootloader protocol: the FC optimistically
transmits the init sequence and expects the ESC to respond with a "471"
signature + device info. If the ESC wasn't ready, isn't in bootloader mode,
or runs different firmware with a different init handshake (e.g. some
Bluejay/AM32 configurations), there's no acknowledgment — and that's where
the real problems below start.

## Issue 1: Unbounded Blocking in `ReadByte()`

**File:** `src/main/io/serial_4way.c`, function `ReadByte()`

```c
static uint8_t ReadByte(void)
{
    // need timeout?
    while (!serialRxBytesWaiting(port));
    return serialRead(port);
}
```

There's no timeout — if the ESC never responds, this loops forever. The
`// need timeout?` comment suggests the author was already aware of the gap.

**Call path when an ESC doesn't respond to the init sequence:**

```
esc4wayProcess()
  └─> do { ESC = ReadByteCrc(); } while (ESC != cmd_Local_Escape);
      └─> ReadByteCrc()
          └─> ReadByte()
              └─> while (!serialRxBytesWaiting(port));  ← blocks indefinitely
```

Once this path is hit, the firmware blocks, the configurator hangs, USB
becomes unresponsive, and the user has to power-cycle the FC.

Betaflight's starting point here is different from INAV's — its `ReadByte()`
already takes a `timeoutUs` parameter and already has working timeouts on
its general reads (command/address/length/data/CRC bytes), unlike INAV's
`ReadByte(void)`, which has no timeout mechanism anywhere.
[PR #13287](https://github.com/betaflight/betaflight/pull/13287) (merged
2024-01-13) didn't add timeout handling — it changed how one specific loop
uses it. Before the PR, Betaflight's escape-sequence-detection loop had a
3-second timeout (`ESC_TIMEOUT_US`); the PR *removed* that timeout by adding
a `timeoutUs &&` guard to `ReadByte()` (so passing `timeoutUs = 0` means
"wait forever" instead of timing out almost immediately) and then changed
the escape loop specifically to pass `0`, per the PR's own added comment:
*"No timeout as BLHeliSuite32 has this loop sitting indefinitely waiting for
input."* The already-working timeouts on the other reads were only
refactored (nested `if`s flattened into one condition), not newly fixed.

None of that timeout infrastructure exists in INAV's version at all — the
useful takeaway isn't "copy Betaflight's PR #13287 diff," it's that INAV
needs a `timeoutUs` parameter on `ReadByte()` in the first place, with a
`0 = wait forever` convention reserved deliberately for the
BLHeliSuite32-compatible escape loop, mirroring where Betaflight ended up
rather than the specific diff that got it there.

## Issue 2: PWM-Specific Motor I/O Access

**File:** `src/main/io/serial_4way.c`

```c
for (int idx = 0; idx < getMotorCount(); idx++) {
    ioTag_t tag = pwmGetMotorPinTag(idx);
    if (tag != IOTAG_NONE) {
        escHardware[escCount].io = IOGetByTag(tag);
        ...
```

`pwmGetMotorPinTag()` is PWM-protocol-specific — passthrough's motor
detection goes through the PWM motor-pin table directly rather than a
protocol-agnostic motor I/O accessor, which can affect detection on setups
using non-PWM digital ESC protocols.

Betaflight addressed the equivalent code in
[PR #14214](https://github.com/betaflight/betaflight/pull/14214) (merged
2025-01-29) by routing motor I/O access through abstracted accessors
(`motorGetIo()`, `motorIsMotorEnabled()`) instead of reading the PWM motor
struct directly. INAV doesn't currently have an equivalent motor I/O
abstraction layer for this code path.

## Priority

1. **`ReadByte()` timeout** — this is the one that hangs the firmware and
   requires a power cycle; highest priority to fix.
2. **Motor I/O abstraction** — affects passthrough detection reliability on
   non-PWM-protocol setups; lower urgency than a firmware hang, but blocks
   full digital-ESC-protocol support for passthrough.

## References

- Betaflight PR #13287 — https://github.com/betaflight/betaflight/pull/13287
- Betaflight PR #14214 — https://github.com/betaflight/betaflight/pull/14214
- BLHeli bootloader protocol reference (AVRootloader by Hagen), cited in
  `serial_4way_avrootloader.c`

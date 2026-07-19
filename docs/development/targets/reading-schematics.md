# Reading Manufacturer Schematics for New Target Creation

Workflow and checklist for building an INAV target from raw schematic files
(KiCad `.kicad_sch`, or Altium exports) before hardware exists, or to verify
an existing target against the real design. Written from experience turning
a multi-file KiCad schematic (MCU sheet + interfaces sheet + sensor sheet,
~13k lines total) into a verified pin map and a working target.

## 0. Figure out what format you actually have

- Manufacturers often hand over PDFs "for convenience" alongside the native
  CAD files. **PDFs exported from EDA tools (KiCad, Altium) with outlined/
  curve fonts have no extractable text layer** — `pdftotext` returns
  nothing useful. Check for this before assuming the PDF is usable:
  `pdftotext -layout file.pdf - | head` — if it's empty or garbage, fall
  back to the native `.kicad_sch` / `.SchDoc` text files if present.
- `.kicad_sch` files are plain S-expression text — fully `grep`-able. Even
  Altium-native schematics are sometimes distributed as KiCad-imported
  `.kicad_sch` alongside the original `.SchDoc` — prefer the KiCad text
  form for searching, it's much faster than parsing binary/proprietary
  formats.
- Multi-sheet designs are sometimes exported as **separate top-level
  `.kicad_sch` files per functional block** (e.g. `master-controller`,
  `interfaces`, `sensor`) rather than one hierarchical sheet tree. In that
  case there's no `(sheet ...)` linkage between files — **nets with the
  same label text in different files are the same net** (this is how
  Altium's flat multi-sheet-per-project net model gets flattened on
  export). Confirm this assumption by cross-checking: pick a label you're
  fairly sure about (e.g. a UART pin name) and verify it appears with
  matching text in both files at a point that makes physical sense.

## 1. Build the pin → function map

- Find the MCU symbol definition (`(symbol "...:0_<PARTNUMBER>" ...)`) and
  read its full pin list — each `(pin ...)` block gives you `(name "PXn")`
  and `(number "...")`. This is your ground truth for "this schematic pin
  number = this GPIO name," independent of whatever grid/BGA-style numbering
  the symbol author used (it may not match the real physical package pinout
  at all — that's fine, you only care about GPIO name, not physical pin
  number, for target.h purposes).
- **Best case:** the schematic author already embedded the function in the
  net label text, e.g. `(label "PA5（GYRO1_SPI1_SCK）")`. If you see this
  pattern, grep all `(label "..."` lines across every schematic file and
  dedupe — it hands you almost the entire pin map for free:
  ```bash
  grep -ohE '\(label "[^"]*"' *.kicad_sch | sed 's/(label "//;s/"$//' | sort -u
  ```
- **When a label has no embedded pin name** (e.g. just `"CAN_RX"`, `"D+"`,
  `"BEEPER"`), you have to trace it back to the MCU pin manually:
  1. Find the MCU symbol's placement: `(symbol (lib_id "...") (at X Y angle))`.
  2. For each candidate MCU pin, compute its placed position. At `angle=0`
     with no mirroring, KiCad's convention is typically
     `final_x = instance_x + pin_rel_x`, `final_y = instance_y - pin_rel_y`
     (Y inverts because symbol-editor coordinates are Y-up, page coordinates
     are Y-down). **Verify this transform** against a pin you already know
     from an embedded label before trusting it on the unknown ones — pick a
     nearby pin with an embedded name, confirm the math reproduces its
     known wire endpoint, then apply the same transform to the unlabeled
     net.
  3. Cross-check the result against the datasheet AF table if the resulting
     assignment is safety-critical (e.g. it claims to be a CAN or USB pin).

## 2. Never trust a preliminary/manager scan at face value

Re-derive every claim from the schematic text directly, even for things
that "sound right." In this project, a preliminary pass claimed a dual-gyro
IMU was on SPI3 with specific CS pins — but the real schematic showed the
two gyro chips on SPI1 and SPI4, and the SPI3 net (with what looked like
two CS pins) actually terminated on bare test pads with no chip attached.
The giveaway was searching for the actual gyro chip's reference designator
(`ICM-42688-P`) and reading which SPI bus's pins its instance-specific wires
actually reached — not just trusting a label that happened to say "SPI3."

**Checklist of things to independently verify, don't assume:**
- [ ] Which physical chip (part number, reference designator) is on each
      sensor bus — search for the chip's part number/reference directly
      (`grep -n 'reference "U'` / grep the exact part number), don't infer
      it from a bus label alone.
- [ ] Whether a "bus" (SPI/I2C/UART broken out with clean labels) actually
      terminates on a populated chip, an external connector, or a bare test
      pad. Test pad footprints are often named descriptively in the
      manufacturer's language (e.g. `焊盘` = "solder pad" in Chinese CAD
      libraries) — grep for the footprint name once you spot the pattern,
      it'll show you every other test-pad-only net at once.
- [ ] Whether a chip described generically (e.g. "baro," "mag") is actually
      present on the board at all, or whether that bus is only broken out
      externally (e.g. compass-on-GPS-module is extremely common — many FCs
      have zero onboard magnetometer and rely entirely on an external GPS+
      compass module via I2C1).
- [ ] CS/interrupt pin assignments — re-derive per revision. Manufacturers
      reuse a schematic family across product SKUs and change individual
      pins (e.g. a second gyro's CS pin) between revisions without renaming
      the product family internally.

## 3. Identify every sensor chip and confirm INAV driver support

- Grep for `(reference "U` and `(value "...")` pairs — this dumps the full
  BOM-ish reference→part-number list in one shot, usually faster than
  hunting symbol-by-symbol:
  ```bash
  grep -n '(reference "U' sensor.kicad_sch
  ```
- For each sensor part number found, check INAV has a driver:
  `grep -rl "<PARTNUMBER>" inav/src/main/drivers/` (or the closest known
  family name — e.g. `DEVHW_ICM42605` is the correct driver identifier for
  both ICM-42605 **and** ICM-42688-P, since the driver detects the exact
  chip via its WHO_AM_I register. Don't assume "no driver named exactly
  this part number" means unsupported — check the family/WHO_AM_I behavior
  first).
- Baro chip CS-pin-tied-high (or SDO/SA0 strapping) tells you I2C vs SPI
  mode even without simulation — e.g. an SPL06 with its `CSB` pin wired to
  VDD is running in I2C mode regardless of the CSB pin existing on the
  footprint.

## 4. Cross-reference against existing INAV targets before inventing anything

This is usually the single biggest time-saver. Before writing target.h from
scratch:
```bash
ls inav/src/main/target/ | grep -i <mcu-family>   # e.g. H743
grep -n "DEF_TIM" inav/src/main/target/*<family>*/target.c | grep -E "PA0|PB0|..."
```
If your new board's motor/servo pins match an existing target's `DEF_TIM`
table pin-for-pin, that target has almost certainly already solved your
timer/DMA layout, and probably your whole sensor bus layout too — many
manufacturers reuse the same ODM reference PCB across brands. In this
project, a "new" board turned out to match an existing target
(`AETH743Basic`) on *every single pin* except two (a gyro CS pin and the
two status LED pins) — finding that early turned a from-scratch target
build into a diff-and-verify exercise, and gave high confidence in the
config (proven working DMA layout, no fresh conflicts to hunt for).

When you find a near-exact match, don't blindly copy it — check every
differing pin explicitly (ADC channel count, GYRO2 CS, LED pins, whether
SPI3/CAN/etc are populated) against the new schematic; if you don't want to
assume the whole family is identical, resolve differences.

## 5. DMA analysis

- **F4/F7:** DMA streams are fixed per timer-channel, from a small pool of
  1-3 named alternatives per channel — real conflicts happen when two
  requested channels only have one shared option. Cross-reference the exact
  `DEF_TIM` table against the MCU's DMA request mapping table (its
  reference manual, or STM32CubeMX's pinout view) before finalizing.
- **H7 (DMAMUX):** any of DMA1/DMA2's 16 streams can serve any peripheral —
  conflicts are rare, generally only when total simultaneous DMA-users
  exceeds 16. Still check for outright *missing* entries — e.g. `TIM15_CH2`
  has **no** DMA request line at all on H7 (only `TIM15_CH1` does, confirmed
  in `drivers/timer_def_stm32h7xx.h`), which is a real hardware constraint,
  not a target.h bug, and explains why some existing targets mark that
  channel `DMA_NONE` (PWM-only, no DSHOT) rather than a conflict resolution
  choice.
- Count total simultaneous DMA-hungry peripherals (motor/servo DSHOT
  channels + LED strip + gyro SPI + OSD SPI + SDIO + any UART DMA) against
  the stream budget as a sanity check even when using DMAMUX.
- If your timer table is identical to an already-shipping target's, that's
  strong practical evidence of "no conflicts" — note this in your writeup,
  but still do the table lookup for anything new/different.

## 6. Things that "look wired" but do nothing under INAV

Don't assume every trace on the schematic maps to firmware behavior. A
recurring example:
- **UART RTS/CTS pins wired in hardware** (e.g. for a DJI HD air unit) —
  INAV's UART driver hardcodes `USART_HardwareFlowControl_None`
  (`drivers/serial_uart.c`), so those pins are just inert GPIO from
  firmware's perspective even though the schematic (and the other
  firmware this board also targets) uses them.

Conversely, don't assume a peripheral is unsupported just because it's
unfamiliar — INAV has real CAN bus support via DroneCAN
(`grep -rl "USE_DRONECAN" inav/src/main/target/*/target.h` to see which
shipping targets already use it), enabled per-target with `USE_DRONECAN`
plus `CAN1_RX`/`CAN1_TX`/`CAN1_STANDBY` pin defines. Check
`grep -rln "USE_DRONECAN\|FDCAN" inav/src/main/` before assuming any CAN
support is missing.

## 7. Writing the manufacturer feedback doc

Split into three sections, always:
1. **Correct / no action needed** — confirm the things they got right
   (builds trust, and confirms your understanding matches their intent).
2. **Corrected from any preliminary/internal scan** — say plainly what was
   wrong in any earlier assumption and how the schematic actually reads.
3. **Issues / flags** — anything that needs their attention or a design
   decision (unpopulated buses, unverifiable orientation, INAV feature gaps).

Also flag (per team policy) any `config.c` containing
`beeperConfigMutable()->pwmMode = true;` for review — it's often correct
(matches a PWM-driven passive buzzer + `BEEPER_PWM_FREQUENCY`), but should
always be called out explicitly rather than silently carried over from a
template.

## Related Documentation

- `creating-targets.md` — general target.h/target.c/config.c authoring steps
- `timer-dma-conflicts.md` — DMA conflict theory and resolution strategies
- `at32f435-mux-defaults.md` — AT32F435's MUX-based alternate-function model, if working on an AT32 target

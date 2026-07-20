# mwptools for INAV Development

[mwptools](https://codeberg.org/stronnag/mwptools) (author: Jonathan Hudson)
is a suite of tools for INAV mission planning, ground control, and
development/debugging, useful alongside the configurator for ground-testing
flight controllers.

Building the full GUI application requires a recent Go (1.19+) and the Vala
compiler; the CLI tools below only need Go and/or Ruby depending on the
tool. See the repository for build instructions
(`meson setup _build --prefix=... --strip && ninja -C _build install`, with
`valac`, `libgtk-4-dev`, `libshumate-dev`, and Go 1.19+ as dependencies for
the full GUI).

## Key CLI Tools

### fc-get / fc-set — CLI Settings Backup/Restore

Backup and restore INAV CLI settings as diff files.

```bash
fc-get /tmp/my-settings.txt   # backup (auto-detects FC)
fc-set /tmp/my-settings.txt   # restore
```

Options: `-b/--baud` (default 115200), `-d/--device` (auto-detected if
omitted), `-n/--no-back` (skip backup when restoring).

Typical workflow: backup before a firmware update, flash, then restore —
the restore step backs up the current diff with a timestamp before applying
the old one.

### cliterm — Interactive CLI Terminal

Auto-detects the FC and enters CLI mode.

```bash
cliterm                    # auto-detect device
cliterm -d /dev/ttyACM0    # specific device
cliterm -g                 # GPS passthrough (for u-center or ublox-cli)
cliterm -m                 # MSC mode (USB mass storage for SD card access)
cliterm -s                 # SITL, connects to localhost:5670
```

Options: `-b/--baud`, `-d/--device`, `-n/--noinit` (don't auto-enter CLI),
`-f/--file` (read commands from file). `Ctrl-D` quits without saving,
`Ctrl-C` exits.

### flashgo — Blackbox Flash Downloader

Downloads blackbox logs from onboard flash, faster than the configurator.

```bash
flashgo -info          # check flash usage
flashgo                # download, auto-generated filename
flashgo -file log.TXT  # download to specific file
flashgo -erase         # download and erase
flashgo -only-erase    # erase only
```

Reported throughput ~52KB/s on VCP boards.

### fcflash — Firmware Flasher

Flashes INAV firmware from the command line via DFU or USB serial.

```bash
fcflash inav_9.0.0_MATEKF405SE.hex                       # auto-detect mode
fcflash rescue /dev/ttyACM0 inav_9.0.0_MATEKF405SE.hex    # bootloader already active
fcflash erase inav_9.0.0_MATEKF405SE.hex                  # full erase before flash
fcflash 921600 /dev/ttyUSB0 inav_9.0.0_MATEKF405SE.hex    # explicit baud, USB serial
```

Requires `dfu-util` (DFU mode), `stm32flash` (USB serial mode), and
`objcopy` (hex→bin conversion).

### mspmplex — MSP Multiplexer

Lets multiple MSP clients (mwp, configurator) access one FC simultaneously.
Requires INAV 8.0+ (uses MSPv2 flags); up to 64 clients; the configurator
must run on a different host than `mspmplex` itself.

```bash
mspmplex -verbose 1                       # on the machine connected to the FC
mwp -d udp://localhost:27072              # a client, e.g. mwp
```

## Blackbox Analysis Scripts

Ruby scripts for blackbox log analysis, generating CSV/SVG output:

- `inav-parse_bb_compass.rb` — compass/heading analysis with SVG graphs
- `inav_states.rb` — navigation state transition extraction
- `inav_gps_alt.rb` — GPS vs. baro vs. position-estimator altitude comparison
- `inav_hw_status.rb` — hardware status extraction
- `inav_modes.rb` — flight mode extraction
- `inav_sats.rb` — satellite count analysis
- `inav_heading.rb` — heading analysis
- `replay_bbox_ltm.rb` — replays blackbox data as LTM telemetry for mwp visualization

## CRSF Tools

`crsfparser` (Rust) parses and displays CRSF telemetry frames:

```bash
cargo build --release
./target/release/crsfparser capture.log     # from a file
crsfparser < /dev/ttyUSB0                    # from serial
nc -l -k -u -p 42042 | crsfparser            # from UDP
```

## Other Tools

- **dbg-tool** — displays `MSP_DEBUGMSG` output in a terminal, useful for
  serial printf-style debugging. See
  [codeberg.org/stronnag/dbg-tool](https://codeberg.org/stronnag/dbg-tool).
- **ublox-cli / ublox-test** — GPS testing/configuration for u-blox receivers.
- **mwp-log-replay** — replays mwp log files for analysis.

## Quick Reference

| Tool | Purpose | Key Command |
|------|---------|--------------|
| `fc-get` | Backup CLI settings | `fc-get backup.txt` |
| `fc-set` | Restore CLI settings | `fc-set backup.txt` |
| `cliterm` | Interactive CLI | `cliterm` |
| `flashgo` | Download blackbox logs | `flashgo -info` |
| `fcflash` | Flash firmware | `fcflash firmware.hex` |
| `mspmplex` | MSP multiplexer | `mspmplex -verbose 1` |

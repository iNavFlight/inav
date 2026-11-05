# inav-msp for Python

Python library for INAV's MultiWii Serial Protocol (MSP), one of the INAV MSP libraries generated from `msp/schema/msp_v2.yaml` in the INAV repository (see [`msp/README.md`](../README.md)). The message table, enums and constants are generated from the schema and INAV's source; a typed API on top converts units and wraps common commands, so you never hand-craft binary payloads.

Install name `inav-msp`, import name `inav_msp`.

## Features

* **`MSPApi`** — typed, high-level API mirroring MSP messages: FC info,
  telemetry (attitude, altitude, IMU, GPS, battery/analog, link stats),
  RC channels and AUX mapping, mode ranges and active modes, missions
  (waypoint get/set), flight commands (`set_armed`, `activate_rth`,
  `activate_landing`), guided/offboard control (flight-axis overrides,
  altitude/local/global targets), logic conditions, DroneCAN node
  inspection, and SITL simulator state injection.
* **`MSPCodec`** — packs/unpacks payloads from the generated schema tables; usable
  standalone when you only need binary ↔ Python conversion.
* **`MSPSerial`** — MSP v1/v2 framing over serial, TCP, or UDP, with a
  background reader thread, per-message queues, retries, reconnect, and
  frame logging.
* **`inav-msp-shell`** — interactive shell that exposes every `MSPApi` method.
* **`inav_msp.lib`** — generated data: `InavMSP` (message enum), `InavEnums`
  (INAV enums), `InavConstants` (protocol constants), `boxes` (mode boxes),
  `inav_version` (INAV snapshot version).

## Install

```bash
pip install ./msp/python       # from the INAV repository root
pip install -e ./msp/python    # editable/development install
```

## Quick start

```python
from inav_msp import MSPApi

with MSPApi(port="/dev/ttyACM0", baudrate=115200) as api:
    print(api.get_api_version())
    print(api.get_inav_status())
    api.set_rc_channels([1500, 1500, 1500, 1500])

    # Per-request metadata after each call:
    print(f"latency: {api.info['latency_ms']:.1f} ms")
```

All `get_*`/`set_*` methods return plain dicts/lists; per-request metadata
(`code`, `latency_ms`, `transport`, `attempt`, `timestamp`) is available as
`api.info` after each call.

### Transports

```python
MSPApi(port="/dev/ttyACM0", baudrate=115200)   # serial (default)
MSPApi(tcp_endpoint="127.0.0.1:5760")          # TCP (e.g. SITL, wifi bridge)
MSPApi(udp_endpoint="127.0.0.1:27072")         # UDP (implies MSP v2)
```

Extra options: `force_msp_v2=True` forces v2 framing on serial/TCP;
`max_retries` controls retransmission of unanswered requests.

### Interactive shell

```bash
inav-msp-shell --tcp 127.0.0.1:5760
```

Every `MSPApi` method is callable by name with `key=value` arguments:

```
> get_raw_gps
> set_rc_channels channels=[1500,1500,1500,1500]
> set_flight_axis_angle_override pitch_deg=10
> set_altitude_target altitude_datum=1 altitude_m=50
```

## Schema and generated data

`src/inav_msp/generated/` holds the generated protocol code — `messages.py`
(every payload resolved to struct format strings, plus `InavMSP`) and
`enums.py` (`InavEnums`) and `constants.py` (`InavConstants`).
`src/inav_msp/lib/` re-exports them and keeps `inav_version.py`. Nothing reads a JSON schema at runtime; the codec executes
the generated table.

**The schema must match the firmware you talk to.** The codec trusts the
schema completely: decoding against a stale schema produces wrong values or
errors. Generate from the INAV version that matches your target, and check
`api.get_inav_version()` (the packaged snapshot version) against
`api.get_fc_version()` (the firmware).

## Examples

`examples/example_api.py` is the "how to do everything" tour — reads a broad
set of MSP data and performs RC/waypoint writes:

```bash
python examples/example_api.py --tcp 127.0.0.1:5760
```

Also included: `basic_usage.py` (connection, FC info, sensors),
`flight_computer.py` (companion-computer telemetry loop), `introspection.py`
(schema discovery), `logic_conditions.py` (custom messages without helper
methods), `msp_joystick.py` (gamepad as MSP RC input). See
`examples/README.md`.

## Tests

```bash
PYTHONPATH=src python -m unittest discover tests
```

(or `./msp/build.sh test` from the INAV repository root, which also runs the C and JavaScript checks.)

## Error handling philosophy

Exceptions are allowed to surface: timeouts, decode failures, and transport
errors raise rather than being silently swallowed. A crash is preferable to
masking protocol drift.

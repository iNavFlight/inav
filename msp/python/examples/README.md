# Examples

Practical examples showing how to use inav_msp. They accept `--port` for a
serial connection and, where applicable, `--tcp`/`--udp` endpoints (e.g.
SITL or a wifi bridge):

```bash
# Serial (direct connection)
python examples/basic_usage.py --port /dev/ttyACM0

# TCP
python examples/basic_usage.py --tcp 127.0.0.1:5760
```

## example_api.py

The comprehensive "how to do everything" tour: reads a broad set of MSP data
(FC info, status, sensors, GPS, modes) and performs RC/waypoint writes.

```bash
python examples/example_api.py --tcp 127.0.0.1:5760
```

## basic_usage.py

Fundamental operations: connecting, reading FC info, reading sensors.

```bash
python examples/basic_usage.py --port /dev/ttyACM0
```

Shows connection setup (serial and TCP), reading FC info (version, board,
variant), reading sensors (attitude, altitude, GPS, battery), error handling,
and context-manager usage.

## flight_computer.py

Complete flight computer example for Raspberry Pi/companion computers.

```bash
python examples/flight_computer.py --port /dev/ttyAMA0 --rate 0.1
```

Shows continuous telemetry monitoring, safety checks (battery, GPS, altitude),
setting waypoints programmatically, error handling and reconnection, and
logging for post-flight analysis. A starting point for obstacle avoidance,
follow-me, autonomous missions, or monitoring/alerting.

## introspection.py

Helpers for discovering message structure from the schema.

```bash
python examples/introspection.py
```

**Use in your code:**

```python
from examples.introspection import print_message_info
from inav_msp import InavMSP

print_message_info(InavMSP.MSP2_INAV_STATUS)
```

**Functions provided:**

- `get_message_info(code)` — message details as a dict
- `print_message_info(code)` — pretty-print message structure
- `list_all_messages(filter)` — list available messages
- `print_all_messages(filter)` — pretty-print message list

## logic_conditions.py

Using messages that don't have convenience methods.

```bash
python examples/logic_conditions.py --port /dev/ttyACM0 --index 0
```

Shows the 3-step pattern that works for any message in the schema:

1. Pack request data with `api._pack_request(code, {...})`
2. Send it with `api._request(code, payload)`
3. Process the returned `(info, reply)`

## msp_joystick.py

Use a gamepad as an MSP RC input:

```bash
python examples/msp_joystick.py --tcp 127.0.0.1:5762 --rate 50 --joystick-index 0
```

## Creating your own

### Simple template

```python
from inav_msp import MSPApi

with MSPApi(port="/dev/ttyACM0") as api:
    attitude = api.get_attitude()
    print(f"Roll: {attitude['roll']}°  (latency {api.info['latency_ms']:.1f} ms)")
```

### Custom message template

```python
from inav_msp import MSPApi, InavMSP
from examples.introspection import print_message_info

# First, find out what fields the message has
print_message_info(InavMSP.MSP2_INAV_GVAR_STATUS)

# Then use it
with MSPApi(port="/dev/ttyACM0") as api:
    payload = api._pack_request(InavMSP.MSP2_INAV_GVAR_STATUS, {})
    info, reply = api._request(InavMSP.MSP2_INAV_GVAR_STATUS, payload)
    print(reply)
```

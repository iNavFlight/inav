# ADSB Vehicle MAVLink Simulator

A Python tool for simulating ADS-B aircraft traffic over a serial connection using the MAVLink protocol. Useful for testing ADS-B receivers, ground station software, or flight controller integrations without real aircraft.

---

## Requirements
```bash
pip install pymavlink pyserial
```

---

## Usage
```bash
python adsb_sim.py <com_port> <json_file> [--baud BAUD] [--rate RATE]
```

### Arguments

| Argument | Required | Default | Description |
|---|---|---|---|
| `com_port` | ✅ | — | Serial port (e.g. `COM3`, `/dev/ttyUSB0`) |
| `json_file` | ✅ | — | Path to JSON file with aircraft definitions |
| `--baud` | ❌ | `115200` | Serial baud rate |
| `--rate` | ❌ | `1.0` | Update rate in Hz |

### Examples
```bash
# Windows
python adsb_sim.py COM3 aircraft.json

# Linux
python adsb_sim.py /dev/ttyUSB0 aircraft.json --baud 57600 --rate 2.0
```

---

## Aircraft JSON Format

Each aircraft is defined as an object in a JSON array:
```json
[
  {
    "icao_address": 1001,
    "lat": 49.2344299,
    "lon": 16.5610206,
    "altitude": 300,
    "heading": 237,
    "hor_velocity": 30,
    "ver_velocity": 0,
    "callsign": "V250"
  }
]
```

| Field | Type | Unit | Description |
|---|---|---|---|
| `icao_address` | int | — | Unique ICAO identifier |
| `lat` | float | degrees | Latitude |
| `lon` | float | degrees | Longitude |
| `altitude` | float | meters ASL | Altitude above sea level |
| `heading` | float | degrees (0–360) | Track heading |
| `hor_velocity` | float | m/s | Horizontal speed |
| `ver_velocity` | float | m/s | Vertical speed (positive = climb) |
| `callsign` | string | — | Aircraft callsign (max 8 chars) |

---

## How It Works

The simulator connects to a serial port and continuously transmits two MAVLink message types:

- **HEARTBEAT** — sent once per second to identify the component as an ADS-B device
- **ADSB_VEHICLE** — sent for each aircraft at the configured rate, containing position, velocity, heading and identification data

All aircraft from the JSON file are broadcast in every update cycle. The positions are static — aircraft do not move between updates.

---

## Notes

- Altitude is transmitted in millimeters internally (`altitude * 1000`) as required by the MAVLink `ADSB_VEHICLE` message spec
- Heading is transmitted in centidegrees (`heading * 100`)
- Callsigns are ASCII-encoded and null-padded to 9 bytes
- `flags` is set to `3` (lat/lon and altitude valid)
- `tslc` (time since last communication) is hardcoded to `1` second
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Generate Markdown documentation from an MSP message definitions JSON.

Strict + Index:
- Builds an Index grouped as MSPv1 (0–254) and MSPv2 (4096–20000) using lib.msp_enum.MSPCodes.
- STRICT: If a code exists in one (MSPCodes vs JSON) but not the other, crash with details.
- Index items link to headings via GitHub-style auto-anchors.
- Tight layout; identical Request/Reply tables; skip complex=true with a stub.
- Default input: lib/msp_messages.json ; default output: MSP_Doc.md
"""

import sys
import json
import re
import unicodedata
import manual_docs_fix
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple


import enum

class MSPCodes(enum.IntEnum):
    MSP_API_VERSION = 1
    MSP_FC_VARIANT = 2
    MSP_FC_VERSION = 3
    MSP_BOARD_INFO = 4
    MSP_BUILD_INFO = 5
    MSP_INAV_PID = 6
    MSP_SET_INAV_PID = 7
    MSP_NAME = 10
    MSP_SET_NAME = 11
    MSP_NAV_POSHOLD = 12
    MSP_SET_NAV_POSHOLD = 13
    MSP_CALIBRATION_DATA = 14
    MSP_SET_CALIBRATION_DATA = 15
    MSP_POSITION_ESTIMATION_CONFIG = 16
    MSP_SET_POSITION_ESTIMATION_CONFIG = 17
    MSP_WP_MISSION_LOAD = 18
    MSP_WP_MISSION_SAVE = 19
    MSP_WP_GETINFO = 20
    MSP_RTH_AND_LAND_CONFIG = 21
    MSP_SET_RTH_AND_LAND_CONFIG = 22
    MSP_FW_CONFIG = 23
    MSP_SET_FW_CONFIG = 24
    MSP_MODE_RANGES = 34
    MSP_SET_MODE_RANGE = 35
    MSP_FEATURE = 36
    MSP_SET_FEATURE = 37
    MSP_BOARD_ALIGNMENT = 38
    MSP_SET_BOARD_ALIGNMENT = 39
    MSP_CURRENT_METER_CONFIG = 40
    MSP_SET_CURRENT_METER_CONFIG = 41
    MSP_MIXER = 42
    MSP_SET_MIXER = 43
    MSP_RX_CONFIG = 44
    MSP_SET_RX_CONFIG = 45
    MSP_LED_COLORS = 46
    MSP_SET_LED_COLORS = 47
    MSP_LED_STRIP_CONFIG = 48
    MSP_SET_LED_STRIP_CONFIG = 49
    MSP_RSSI_CONFIG = 50
    MSP_SET_RSSI_CONFIG = 51
    MSP_ADJUSTMENT_RANGES = 52
    MSP_SET_ADJUSTMENT_RANGE = 53
    MSP_CF_SERIAL_CONFIG = 54
    MSP_SET_CF_SERIAL_CONFIG = 55
    MSP_VOLTAGE_METER_CONFIG = 56
    MSP_SET_VOLTAGE_METER_CONFIG = 57
    MSP_SONAR_ALTITUDE = 58
    MSP_RX_MAP = 64
    MSP_SET_RX_MAP = 65
    MSP_REBOOT = 68
    MSP_DATAFLASH_SUMMARY = 70
    MSP_DATAFLASH_READ = 71
    MSP_DATAFLASH_ERASE = 72
    MSP_LOOP_TIME = 73
    MSP_SET_LOOP_TIME = 74
    MSP_FAILSAFE_CONFIG = 75
    MSP_SET_FAILSAFE_CONFIG = 76
    MSP_SDCARD_SUMMARY = 79
    MSP_BLACKBOX_CONFIG = 80
    MSP_SET_BLACKBOX_CONFIG = 81
    MSP_TRANSPONDER_CONFIG = 82
    MSP_SET_TRANSPONDER_CONFIG = 83
    MSP_OSD_CONFIG = 84
    MSP_SET_OSD_CONFIG = 85
    MSP_OSD_CHAR_READ = 86
    MSP_OSD_CHAR_WRITE = 87
    MSP_VTX_CONFIG = 88
    MSP_SET_VTX_CONFIG = 89
    MSP_ADVANCED_CONFIG = 90
    MSP_SET_ADVANCED_CONFIG = 91
    MSP_FILTER_CONFIG = 92
    MSP_SET_FILTER_CONFIG = 93
    MSP_PID_ADVANCED = 94
    MSP_SET_PID_ADVANCED = 95
    MSP_SENSOR_CONFIG = 96
    MSP_SET_SENSOR_CONFIG = 97
    MSP_SPECIAL_PARAMETERS = 98
    MSP_SET_SPECIAL_PARAMETERS = 99
    MSP_IDENT = 100
    MSP_STATUS = 101
    MSP_RAW_IMU = 102
    MSP_SERVO = 103
    MSP_MOTOR = 104
    MSP_RC = 105
    MSP_RAW_GPS = 106
    MSP_COMP_GPS = 107
    MSP_ATTITUDE = 108
    MSP_ALTITUDE = 109
    MSP_ANALOG = 110
    MSP_RC_TUNING = 111
    MSP_ACTIVEBOXES = 113
    MSP_MISC = 114
    MSP_BOXNAMES = 116
    MSP_PIDNAMES = 117
    MSP_WP = 118
    MSP_BOXIDS = 119
    MSP_SERVO_CONFIGURATIONS = 120
    MSP_NAV_STATUS = 121
    MSP_NAV_CONFIG = 122
    MSP_3D = 124
    MSP_RC_DEADBAND = 125
    MSP_SENSOR_ALIGNMENT = 126
    MSP_LED_STRIP_MODECOLOR = 127
    MSP_BATTERY_STATE = 130
    MSP_VTXTABLE_BAND = 137
    MSP_VTXTABLE_POWERLEVEL = 138
    MSP_STATUS_EX = 150
    MSP_SENSOR_STATUS = 151
    MSP_UID = 160
    MSP_GPSSVINFO = 164
    MSP_GPSSTATISTICS = 166
    MSP_OSD_VIDEO_CONFIG = 180
    MSP_SET_OSD_VIDEO_CONFIG = 181
    MSP_DISPLAYPORT = 182
    MSP_SET_TX_INFO = 186
    MSP_TX_INFO = 187
    MSP_SET_RAW_RC = 200
    MSP_SET_RAW_GPS = 201
    MSP_SET_BOX = 203
    MSP_SET_RC_TUNING = 204
    MSP_ACC_CALIBRATION = 205
    MSP_MAG_CALIBRATION = 206
    MSP_SET_MISC = 207
    MSP_RESET_CONF = 208
    MSP_SET_WP = 209
    MSP_SELECT_SETTING = 210
    MSP_SET_HEAD = 211
    MSP_SET_SERVO_CONFIGURATION = 212
    MSP_SET_MOTOR = 214
    MSP_SET_NAV_CONFIG = 215
    MSP_SET_3D = 217
    MSP_SET_RC_DEADBAND = 218
    MSP_SET_RESET_CURR_PID = 219
    MSP_SET_SENSOR_ALIGNMENT = 220
    MSP_SET_LED_STRIP_MODECOLOR = 221
    MSP_SET_ACC_TRIM = 239
    MSP_ACC_TRIM = 240
    MSP_SERVO_MIX_RULES = 241
    MSP_SET_SERVO_MIX_RULE = 242
    MSP_SET_PASSTHROUGH = 245
    MSP_RTC = 246
    MSP_SET_RTC = 247
    MSP_EEPROM_WRITE = 250
    MSP_RESERVE_1 = 251
    MSP_RESERVE_2 = 252
    MSP_DEBUGMSG = 253
    MSP_DEBUG = 254
    MSP_V2_FRAME = 255
    MSP2_COMMON_TZ = 4097
    MSP2_COMMON_SET_TZ = 4098
    MSP2_COMMON_SETTING = 4099
    MSP2_COMMON_SET_SETTING = 4100
    MSP2_COMMON_MOTOR_MIXER = 4101
    MSP2_COMMON_SET_MOTOR_MIXER = 4102
    MSP2_COMMON_SETTING_INFO = 4103
    MSP2_COMMON_PG_LIST = 4104
    MSP2_COMMON_SERIAL_CONFIG = 4105
    MSP2_COMMON_SET_SERIAL_CONFIG = 4106
    MSP2_COMMON_SET_RADAR_POS = 4107
    MSP2_COMMON_SET_RADAR_ITD = 4108
    MSP2_COMMON_SET_MSP_RC_LINK_STATS = 4109
    MSP2_COMMON_SET_MSP_RC_INFO = 4110
    MSP2_COMMON_GET_RADAR_GPS = 4111
    MSP2_SENSOR_RANGEFINDER = 7937
    MSP2_SENSOR_OPTIC_FLOW = 7938
    MSP2_SENSOR_GPS = 7939
    MSP2_SENSOR_COMPASS = 7940
    MSP2_SENSOR_BAROMETER = 7941
    MSP2_SENSOR_AIRSPEED = 7942
    MSP2_SENSOR_HEADTRACKER = 7943
    MSP2_INAV_STATUS = 8192
    MSP2_INAV_OPTICAL_FLOW = 8193
    MSP2_INAV_ANALOG = 8194
    MSP2_INAV_MISC = 8195
    MSP2_INAV_SET_MISC = 8196
    MSP2_INAV_BATTERY_CONFIG = 8197
    MSP2_INAV_SET_BATTERY_CONFIG = 8198
    MSP2_INAV_RATE_PROFILE = 8199
    MSP2_INAV_SET_RATE_PROFILE = 8200
    MSP2_INAV_AIR_SPEED = 8201
    MSP2_INAV_OUTPUT_MAPPING = 8202
    MSP2_INAV_MC_BRAKING = 8203
    MSP2_INAV_SET_MC_BRAKING = 8204
    MSP2_INAV_OUTPUT_MAPPING_EXT = 8205
    MSP2_INAV_TIMER_OUTPUT_MODE = 8206
    MSP2_INAV_SET_TIMER_OUTPUT_MODE = 8207
    MSP2_INAV_MIXER = 8208
    MSP2_INAV_SET_MIXER = 8209
    MSP2_INAV_OSD_LAYOUTS = 8210
    MSP2_INAV_OSD_SET_LAYOUT_ITEM = 8211
    MSP2_INAV_OSD_ALARMS = 8212
    MSP2_INAV_OSD_SET_ALARMS = 8213
    MSP2_INAV_OSD_PREFERENCES = 8214
    MSP2_INAV_OSD_SET_PREFERENCES = 8215
    MSP2_INAV_SELECT_BATTERY_PROFILE = 8216
    MSP2_INAV_DEBUG = 8217
    MSP2_BLACKBOX_CONFIG = 8218
    MSP2_SET_BLACKBOX_CONFIG = 8219
    MSP2_INAV_TEMP_SENSOR_CONFIG = 8220
    MSP2_INAV_SET_TEMP_SENSOR_CONFIG = 8221
    MSP2_INAV_TEMPERATURES = 8222
    MSP_SIMULATOR = 8223
    MSP2_INAV_SERVO_MIXER = 8224
    MSP2_INAV_SET_SERVO_MIXER = 8225
    MSP2_INAV_LOGIC_CONDITIONS = 8226
    MSP2_INAV_SET_LOGIC_CONDITIONS = 8227
    MSP2_INAV_GLOBAL_FUNCTIONS = 8228
    MSP2_INAV_SET_GLOBAL_FUNCTIONS = 8229
    MSP2_INAV_LOGIC_CONDITIONS_STATUS = 8230
    MSP2_INAV_GVAR_STATUS = 8231
    MSP2_INAV_PROGRAMMING_PID = 8232
    MSP2_INAV_SET_PROGRAMMING_PID = 8233
    MSP2_INAV_PROGRAMMING_PID_STATUS = 8234
    MSP2_PID = 8240
    MSP2_SET_PID = 8241
    MSP2_INAV_OPFLOW_CALIBRATION = 8242
    MSP2_INAV_FWUPDT_PREPARE = 8243
    MSP2_INAV_FWUPDT_STORE = 8244
    MSP2_INAV_FWUPDT_EXEC = 8245
    MSP2_INAV_FWUPDT_ROLLBACK_PREPARE = 8246
    MSP2_INAV_FWUPDT_ROLLBACK_EXEC = 8247
    MSP2_INAV_SAFEHOME = 8248
    MSP2_INAV_SET_SAFEHOME = 8249
    MSP2_INAV_MISC2 = 8250
    MSP2_INAV_LOGIC_CONDITIONS_SINGLE = 8251
    MSP2_INAV_ESC_RPM = 8256
    MSP2_INAV_ESC_TELEM = 8257
    MSP2_INAV_LED_STRIP_CONFIG_EX = 8264
    MSP2_INAV_SET_LED_STRIP_CONFIG_EX = 8265
    MSP2_INAV_FW_APPROACH = 8266
    MSP2_INAV_SET_FW_APPROACH = 8267
    MSP2_INAV_GPS_UBLOX_COMMAND = 8272
    MSP2_INAV_RATE_DYNAMICS = 8288
    MSP2_INAV_SET_RATE_DYNAMICS = 8289
    MSP2_INAV_EZ_TUNE = 8304
    MSP2_INAV_EZ_TUNE_SET = 8305
    MSP2_INAV_SELECT_MIXER_PROFILE = 8320
    MSP2_ADSB_VEHICLE_LIST = 8336
    MSP2_INAV_CUSTOM_OSD_ELEMENTS = 8448
    MSP2_INAV_CUSTOM_OSD_ELEMENT = 8449
    MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS = 8450
    MSP2_INAV_OUTPUT_MAPPING_EXT2 = 8461
    MSP2_INAV_SERVO_CONFIG = 8704
    MSP2_INAV_SET_SERVO_CONFIG = 8705
    MSP2_INAV_GEOZONE = 8720
    MSP2_INAV_SET_GEOZONE = 8721
    MSP2_INAV_GEOZONE_VERTEX = 8722
    MSP2_INAV_SET_GEOZONE_VERTEX = 8723
    MSP2_BETAFLIGHT_BIND = 12288


# ---- C type size helpers ----------------------------------------------------

BASE_SIZES = {
    "uint8_t": 1, "int8_t": 1, "char": 1,
    "uint16_t": 2, "int16_t": 2,
    "uint32_t": 4, "int32_t": 4,
    "uint64_t": 8, "int64_t": 8,
    "float": 4, "double": 8,
}

array_brackets_re = re.compile(r"^(?P<base>[A-Za-z_0-9]+)\[(?P<size>.*)\]$")

def parse_ctype(ctype: str) -> Tuple[str, Optional[str]]:
    m = array_brackets_re.match(ctype.strip())
    if not m:
        return ctype.strip(), None
    return m.group("base").strip(), m.group("size").strip()

def sizeof_entry(field: Dict[str, Any]) -> str:
    ctype = field.get("ctype", "").strip()
    base, bracket = parse_ctype(ctype)

    is_array = bool(field.get("array", False))
    array_size_meta = field.get("array_size", None)
    array_ctype = field.get("array_ctype", base)

    if is_array or bracket is not None:
        base_for_size = array_ctype if is_array else base
        base_bytes = BASE_SIZES.get(base_for_size, None)

        if isinstance(array_size_meta, int):
            return str(array_size_meta * base_bytes) if base_bytes is not None else str(array_size_meta)

        if isinstance(array_size_meta, str) and array_size_meta:
            if base_bytes is None or base_for_size == "char":
                return array_size_meta
            return f"{array_size_meta} * {base_bytes}"

        if bracket is not None:
            if bracket == "":
                return "array"
            if bracket.isdigit():
                n = int(bracket)
                return str(n * base_bytes) if base_bytes is not None else str(n)
            if base_bytes is None or base == "char":
                return bracket
            return f"{bracket} * {base_bytes}"

        return "array"

    base_bytes = BASE_SIZES.get(base, None)
    return str(base_bytes) if base_bytes is not None else "-"


# ---- Markdown rendering -----------------------------------------------------

#inav_wiki_url = "https://github.com/xznhj8129/msp_documentation/blob/master/docs/"
inav_wiki_url = "https://github.com/iNavFlight/inav/wiki/"

def units_cell(field: Dict[str, Any]) -> str:
    if "enum" in field:
        if field["enum"]=="?_e":
            return "[ENUM_NAME](LINK_TO_ENUM)"
        else:
            return f"[{field['enum']}]({inav_wiki_url}inav_enums_ref.md#enum-{field['enum'].lower()})"
    u = (field.get("units") or "").strip()
    return u if u else "-"

def has_fields(section: Any) -> bool:
    if not isinstance(section, dict):
        return False
    payload = section.get("payload")
    return isinstance(payload, list) and len(payload) > 0

def get_fields(section: Any) -> List[Dict[str, Any]]:
    if not isinstance(section, dict):
        return []
    payload = section.get("payload")
    return payload if isinstance(payload, list) else []

def table_with_units(fields: List[Dict[str, Any]], label: str) -> str:
    header = (
        f"  \n**{label}:**\n"
        "| Field | C Type | Size (Bytes) | Units | Description |\n"
        "|---|---|---|---|---|\n"
    )
    rows = []
    for f in fields:
        name = f.get("name", "")
        ctype = f.get("ctype", "")
        size = sizeof_entry(f)
        if size == "0":
            size = "-"
        units = units_cell(f)
        desc = (f.get("desc") or "").strip()
        rows.append(f"| `{name}` | `{ctype}` | {size} | {units} | {desc} |")
    return header + "\n".join(rows) + "\n"

def render_message(name: str, msg: Dict[str, Any]) -> Tuple[str, str]:
    """
    Returns (section_markdown, heading_text_for_anchor)
    """
    code = msg.get("code", 0)
    hex_str = msg.get("hex", hex(code))
    description = (msg.get("description") or "").strip()
    notes = (msg.get("notes") or "").strip()
    complex_flag = bool(msg.get("complex", False))

    heading = f'## <a id="{name.lower()}"></a>`{name} ({code} / {hex_str})`'
    #heading = f"### `{name}` ({code} / {hex_str})"
    out = [heading + "\n"]

    #out.append("\n**Request Payload:** **None**  \n")
    if description:
        out.append(f"**Description:** {description}  \n")

    if complex_flag:
        out.append("**Special case, skipped for now**\n\n")
        return "".join(out), heading

    req = msg.get("request", None)
    rep = msg.get("reply", None)

    if has_fields(req):
        out.append(table_with_units(get_fields(req), "Request Payload"))
    else:
        out.append("\n**Request Payload:** **None**  \n")

    if has_fields(rep):
        out.append(table_with_units(get_fields(rep), "Reply Payload"))
    else:
        out.append("\n**Reply Payload:** **None**  \n")

    if notes:
        out.append(f"\n**Notes:** {notes}\n")

    out.append("\n")
    return "".join(out), heading

# ---- Index + strict consistency --------------------------------------------

def build_maps(defs: Dict[str, Any]) -> Tuple[Dict[int, str], Dict[int, str]]:
    """
    Returns:
      json_by_code: {code -> message_name_from_json}
      mw_by_code:   {code -> enum_name_from_MSPCodes}
    Only for codes in the enforced ranges (v1 and v2).
    """
    v1_range = range(0, 255)
    v2_range = range(4096, 20001)

    # JSON: build by code (restrict to ranges)
    json_by_code: Dict[int, str] = {}
    for name, body in defs.items():
        code = int(body.get("code", -1))
        if code in v1_range or code in v2_range:
            json_by_code[code] = name

    # MSPCodes: probe the same ranges
    mw_by_code: Dict[int, str] = {}
    def try_get(code: int) -> Optional[str]:
        try:
            e = MSPCodes(code)
            return e.name
        except Exception:
            return None

    for code in list(v1_range) + list(v2_range):
        ename = try_get(code)
        if ename is not None:
            mw_by_code[code] = ename

    return json_by_code, mw_by_code

def enforce_strict_match(json_by_code: Dict[int, str], mw_by_code: Dict[int, str]) -> None:
    json_codes = set(json_by_code.keys())
    mw_codes = set(mw_by_code.keys())

    only_in_json = sorted(json_codes - mw_codes)
    only_in_mw   = sorted(mw_codes - json_codes)

    if only_in_json or only_in_mw:
        lines = ["MSP code mismatch detected:"]
        if only_in_json:
            lines.append("  Present in JSON but missing in MSPCodes:")
            for c in only_in_json:
                lines.append(f"    {c}\t{json_by_code[c]}")
        if only_in_mw:
            lines.append("  Present in MSPCodes but missing in JSON:")
            for c in only_in_mw:
                lines.append(f"    {c}\t{mw_by_code[c]}")
        raise SystemExit("\n".join(lines))

def build_index(json_by_code: Dict[int, str]) -> str:
    """
    Build a compact index linking to each heading.
    """
    v1 = []
    v2 = []
    for code, name in sorted(json_by_code.items()):
        hex_str = hex(code)
        item = f"[{code} - {name}](#{name.lower()})  "
        if 0 <= code <= 255:
            v1.append(item)
        elif 4096 <= code <= 20000:
            v2.append(item)

    parts = ["## Index", "### MSPv1"]
    parts.extend(v1)
    parts.append("\n### MSPv2")
    parts.extend(v2)
    parts.append("")  # trailing newline
    return "\n".join(parts)

# ---- Orchestration ----------------------------------------------------------

def generate_markdown(defs: Dict[str, Any]) -> str:
    # Strict maps & check
    json_by_code, mw_by_code = build_maps(defs)
    enforce_strict_match(json_by_code, mw_by_code)

    # Build sections, remembering headings for slugging (already handled in index)
    items = sorted(((int(body.get("code", 0)), name, body) for name, body in defs.items()),
                   key=lambda t: t[0])

    sections = []
    for _, name, body in items:
        sec, _heading = render_message(name, body)

        if name == "MSP_SET_VTX_CONFIG":
            sections.append(sec.split('\n')[0]+'\n')
            sec = manual_docs_fix.MSP_SET_VTX_CONFIG + '\n\n'
        if name == "MSP2_COMMON_SET_SETTING":
            sections.append(sec.split('\n')[0]+'\n')
            sec = manual_docs_fix.MSP2_COMMON_SET_SETTING + '\n\n'
        if name == "MSP2_INAV_SET_GEOZONE_VERTEX":
            sections.append(sec.split('\n')[0]+'\n')
            sec = manual_docs_fix.MSP2_INAV_SET_GEOZONE_VERTEX + '\n\n'
        if name == "MSP2_SENSOR_HEADTRACKER": 
            sections.append(sec.split('\n')[0]+'\n')
            sec = manual_docs_fix.MSP2_SENSOR_HEADTRACKER + '\n\n'
        sections.append(sec)

    with open("docs_v2_header.md", "r", encoding="utf-8") as f:
        header = f.read()

    index_md = build_index(json_by_code)
    return header + "\n" + index_md + "\n" + "".join(sections)

def main():
    in_path = Path(sys.argv[1]) if len(sys.argv) >= 2 else Path("msp_messages.json")
    out_path = Path(sys.argv[2]) if len(sys.argv) >= 3 else Path("msp_ref.md")

    with in_path.open("r", encoding="utf-8") as f:
        defs = json.load(f)

    md = generate_markdown(defs)
    out_path.write_text(md, encoding="utf-8")
    print(f"Wrote {out_path}")

if __name__ == "__main__":
    main()

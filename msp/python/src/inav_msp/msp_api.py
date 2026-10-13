from __future__ import annotations

import struct
from pathlib import Path
from typing import Any, Dict, List, Mapping, Optional, Sequence, Union, Tuple

from .generated.constants import InavConstants
from .lib import InavEnums, InavMSP
from .lib.inav_version import MAJOR as INAV_VERSION_MAJOR, MINOR as INAV_VERSION_MINOR, PATCH as INAV_VERSION_PATCH
from .lib import boxes
from .mspcodec import MSPCodec
from .msp_serial import MSPSerial

__all__ = ["MSPApi"]

AUX_RC_RESOLUTION_MODES = {
    2: 0,
    4: 1,
    8: 2,
    16: 3,
}


def _scale(value: float, scale: float) -> int:
    return int(round(value * scale))

def _vec3(vec: Sequence[float], scale: float) -> tuple[int, int, int]:
    min_val = -32768
    max_val = 32767
    return (
        max(min(int(round(vec[0] * scale)), max_val), min_val),
        max(min(int(round(vec[1] * scale)), max_val), min_val),
        max(min(int(round(vec[2] * scale)), max_val), min_val),
    )

class MSPApi:
    """
    High-level MSP helper backed by MSPCodec + MSPSerial.
    Provides typed / unit-converted accessors that mirror test_msp expectations.
    """

    def __init__(
        self,
        port: Optional[str] = "/dev/ttyACM0",
        baudrate: int = 115200,
        *,
        # Match MSPSerial's own defaults. These were 1.0 ms, which is shorter
        # than the 0.52 ms it takes to clock a 6-byte request out at 115200 --
        # close enough to the wire time that writes failed intermittently.
        read_timeout_ms: float = 50.0,
        write_timeout_ms: float = 200.0,
        tcp_endpoint: Optional[str] = None,
        udp_endpoint: Optional[str] = None,
        force_msp_v2: bool = False,
        max_retries: int = 1,
        serial_transport: Optional[Any] = None,
    ) -> None:
        self._codec = MSPCodec()
        endpoint = tcp_endpoint.strip() if tcp_endpoint else None
        udp_target = udp_endpoint.strip() if udp_endpoint else None
        if endpoint and udp_target:
            raise ValueError("Specify only one of tcp_endpoint or udp_endpoint")
        if serial_transport is not None:
            self._serial = serial_transport
        else:
            if endpoint:
                if ":" not in endpoint:
                    raise ValueError("tcp_endpoint must be in HOST:PORT format")
                self._serial = MSPSerial(
                    endpoint,
                    baudrate,
                    read_timeout=read_timeout_ms / 1000.0,
                    write_timeout=write_timeout_ms / 1000.0,
                    tcp=True,
                    force_msp_v2=force_msp_v2,
                    max_retries=max_retries,
                )
            elif udp_target:
                if ":" not in udp_target:
                    raise ValueError("udp_endpoint must be in HOST:PORT format")
                self._serial = MSPSerial(
                    udp_target,
                    baudrate,
                    read_timeout=read_timeout_ms / 1000.0,
                    write_timeout=write_timeout_ms / 1000.0,
                    udp=True,
                    force_msp_v2=True,
                    max_retries=max_retries,
                )
            else:
                if not port:
                    raise ValueError("Serial port must be provided when tcp_endpoint is not set")
                self._serial = MSPSerial(
                    port,
                    baudrate,
                    read_timeout=read_timeout_ms / 1000.0,
                    write_timeout=write_timeout_ms / 1000.0,
                    force_msp_v2=force_msp_v2,
                    max_retries=max_retries,
                )


        self.box_ids = None
        self.chmap = [
            "roll",
            "pitch",
            "throttle",
            "yaw",
            "ch5",
            "ch6",
            "ch7",
            "ch8",
            "ch9",
            "ch10",
            "ch11",
            "ch12",
            "ch13",
            "ch14",
            "ch15",
            "ch16",
            "ch17",
            "ch18",
        ]
        self.rxmap = None
        self.diag: Optional[Dict[str, Any]] = None
        self._last_code: Optional[int] = None
        self.info = None

    def open(self) -> None:
        self._serial.open()

    def close(self) -> None:
        self._serial.close()

    def __enter__(self) -> "MSPApi":
        self.open()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    # ----- helpers -----

    def _build_info(self, diag: Optional[Dict[str, Any]], code: Optional[Union[InavMSP, int]]) -> Dict[str, Any]:
        code_int = None if code is None else int(code)
        info: Dict[str, Any] = {
            "code": code_int,
            "latency_ms": None,
            "transport": None,
            "attempt": None,
            "timestamp": None
        }
        if not diag:
            return info
        info["latency_ms"] = diag.get("duration_ms")
        info["transport"] = diag.get("transport")
        info["attempt"] = diag.get("attempt")
        info["timestamp"] = diag.get("timestamp")
        return info

    def _capture_info(self, code: Optional[InavMSP]) -> Dict[str, Any]:
        diag = getattr(self._serial, "last_diag", None)
        info = self._build_info(diag, code)
        self.diag = info
        self._last_code = info.get("code")
        return info

    def _request_raw(
        self,
        code: InavMSP,
        payload: bytes = b"",
        *,
        timeout: float = 1.0,
    ) -> Tuple[Dict[str, Any], bytes]:

        self.open()
        rsp_code, rsp_payload = self._serial.request(int(code), payload, timeout=timeout)
        info = self._capture_info(code)
        if rsp_code != int(code):
            raise RuntimeError(f"Unexpected MSP response code {rsp_code} for request {int(code)}")
        return info, rsp_payload

    def _request(
        self,
        code: InavMSP,
        payload: bytes = b"",
        *,
        timeout: float = 1.0,
    ) -> Tuple[Dict[str, Any], Union[Mapping[str, Any], List[Mapping[str, Any]]]]:

        info, raw = self._request_raw(code, payload, timeout=timeout)
        self.info = info
        return info, self._codec.unpack_reply(code, raw)

    def _pack_request(self, code: InavMSP, data: Mapping[str, Any]) -> bytes:
        return self._codec.pack_request(code, data)

    def _ensure_box_ids_cached(self) -> List[int]:
        if self.box_ids is None:
            self.get_box_ids()
        if self.box_ids is None:
            raise RuntimeError("BOX IDs could not be retrieved")
        return self.box_ids

    def _ensure_rx_map_cached(self) -> Dict[int, Dict[str, Any]]:
        if self.rxmap is None:
            self.get_rx_map()
        if self.rxmap is None:
            raise RuntimeError("RX map could not be retrieved")
        return self.rxmap

    def _resolve_channel_index(self, channel: Union[int, str]) -> int:
        if isinstance(channel, str):
            rxmap = self._ensure_rx_map_cached()
            target = channel.strip().lower()
            for entry in rxmap.values():
                name = str(entry.get("name", "")).lower()
                if name == target:
                    mapped = entry.get("mappedTo")
                    if mapped is None:
                        break
                    return int(mapped)
            raise KeyError(f"Channel '{channel}' not present in RX map")
        try:
            idx = int(channel)
        except (TypeError, ValueError) as exc:
            raise ValueError(f"Invalid channel index {channel!r}") from exc
        if idx < 0:
            raise ValueError("Channel index must be non-negative")
        return idx

    def _active_mode_mask_from_raw(self, raw_modes: Any) -> int:
        if raw_modes is None:
            return 0
        if isinstance(raw_modes, int):
            return raw_modes
        mask = 0
        if isinstance(raw_modes, list):
            box_ids_map = {
                pid: idx
                for idx, pid in enumerate(self._ensure_box_ids_cached())
            }
            for mode in raw_modes:
                if not isinstance(mode, Mapping):
                    try:
                        permanent_id = int(mode)
                    except (TypeError, ValueError):
                        continue
                    idx = box_ids_map.get(permanent_id)
                    if idx is None:
                        continue
                    mask |= 1 << idx
                    continue
                box_index = mode.get("boxIndex")
                if box_index is None:
                    continue
                mask |= 1 << int(box_index)
        return mask

    def _decode_active_modes_mask(
        self, mask: Union[int, bytes, None]
    ) -> List[Union[boxes.BoxEnum, int]]:
        """Walk the active-mode bits.

        boxBitmask_t is INAV's bitarray, so the codec hands back its raw bytes
        rather than a width this side has guessed at. Bit n lives in byte n//8,
        little-endian, exactly as BITARRAY_DECLARE lays it out on the wire.
        """
        if not mask:
            return []
        if isinstance(mask, (bytes, bytearray)):
            mask = int.from_bytes(bytes(mask), "little")
        box_ids = self._ensure_box_ids_cached()
        active_modes: List[Union[boxes.BoxEnum, int]] = []
        for idx, permanent_id in enumerate(box_ids):
            if not (mask & (1 << idx)):
                continue
            active_modes.append(boxes.BoxEnum._value2member_map_.get(permanent_id, permanent_id))
        return active_modes

    def get_inav_version(self) -> Dict[str, int]:
        return {
            "major": INAV_VERSION_MAJOR,
            "minor": INAV_VERSION_MINOR,
            "patch": INAV_VERSION_PATCH,
        }

    # ----- API surface -----

    def get_api_version(self) -> Dict[str, int]:
        self.info, rep = self._request(InavMSP.MSP_API_VERSION)
        return {
            "mspProtocolVersion": rep["mspProtocolVersion"],
            "apiVersionMajor": rep["apiVersionMajor"],
            "apiVersionMinor": rep["apiVersionMinor"],
        }

    def get_fc_version(self) -> Dict[str, int]:
        self.info, rep = self._request(InavMSP.MSP_FC_VERSION)
        return {
            "major": rep["fcVersionMajor"],
            "minor": rep["fcVersionMinor"],
            "patch": rep["fcVersionPatch"],
        }

    def get_fc_variant(self) -> Dict[str, str]:
        self.info, rep = self._request(InavMSP.MSP_FC_VARIANT)
        identifier = rep["fcVariantIdentifier"].rstrip("\x00")
        return {"fcVariantIdentifier": identifier}

    def get_board_info(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP_BOARD_INFO)
        board_identifier = rep["boardIdentifier"].rstrip("\x00")
        target_name = rep["targetName"].rstrip("\x00")
        comm_capabilities = rep["commCapabilities"]
        return {
            "boardIdentifier": board_identifier,
            "hardwareRevision": rep["hardwareRevision"],
            "osdSupport": rep["osdSupport"],
            "commCapabilities": {
                "vcp": bool(comm_capabilities & 0x1),
                "softSerial": bool(comm_capabilities & 0x2),
            },
            "targetName": target_name,
        }

    def get_sensor_config(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP_SENSOR_CONFIG)
        sensor_enums = {
            "accHardware": InavEnums.accelerationSensor_e,
            "baroHardware": InavEnums.baroSensor_e,
            "magHardware": InavEnums.magSensor_e,
            "pitotHardware": InavEnums.pitotSensor_e,
            "rangefinderHardware": InavEnums.rangefinderType_e,
            "opflowHardware": InavEnums.opticalFlowSensor_e,
        }
        converted: Dict[str, InavEnums] = {}
        for key, enum_cls in sensor_enums.items():
            if key in rep:
                converted[key] = enum_cls(rep[key])
        return converted

    def get_box_ids(self) -> List[int]:
        self.info, rep = self._request(InavMSP.MSP_BOXIDS)
        self.box_ids = list(rep["boxIds"])
        return list(self.box_ids)

    def get_mode_ranges(self) -> List[Dict[str, Any]]:
        box_ids = self._ensure_box_ids_cached()
        self.info, entries = self._request(InavMSP.MSP_MODE_RANGES)
        min_pwm = InavConstants.CHANNEL_RANGE_MIN
        step_width = InavConstants.CHANNEL_RANGE_STEP_WIDTH
        summary: List[Dict[str, Any]] = []
        armfound = False
        for entry in entries:
            permanent_id = entry["modePermanentId"]
            if permanent_id == 0:
                if armfound:
                    continue
                armfound = True
            aux_index = entry["auxChannelIndex"]
            start_step = entry["rangeStartStep"]
            end_step = entry["rangeEndStep"]
            box_info = boxes.MODEBOXES.get(permanent_id, {})
            box_name = box_info.get("boxName", f"UNKNOWN_{permanent_id}")
            pwm_start = min_pwm + start_step * step_width
            pwm_end = min_pwm + end_step * step_width
            summary.append(
                {
                    "mode": box_name,
                    "boxIndex": box_ids.index(permanent_id) if permanent_id in box_ids else None,
                    "permanentId": permanent_id,
                    "auxChannelIndex": aux_index,
                    "pwmRange": (pwm_start, pwm_end),
                }
            )
        return summary

    def get_rx_map(self) -> Dict[int, Dict[str, Any]]:
        self.info, rep = self._request(InavMSP.MSP_RX_MAP)
        rc_map = list(rep.get("rcMap"))
        decoded= {}
        for idx, mapped_idx in enumerate(rc_map):
            source_name = self.chmap[idx] if idx < len(self.chmap) else f"aux{idx + 1}"
            target_name = self.chmap[mapped_idx] if mapped_idx < len(self.chmap) else f"aux{mapped_idx + 1}"
            decoded[idx] = {
                    "name": source_name,
                    "mappedTo": mapped_idx
                }
        self.rxmap = decoded
        return decoded

    def get_inav_status(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP2_INAV_STATUS)
        active_modes = self._decode_active_modes_mask(rep.get("activeModes"))
        arming_flags_raw = rep["armingFlags"]
        arming_flags = [
            flag
            for flag in InavEnums.armingFlag_e
            if flag is not InavEnums.armingFlag_e.ARMING_DISABLED_ALL_FLAGS and (arming_flags_raw & flag.value)
        ]
        sensor_status_raw = rep["sensorStatus"]
        sensor_status = [sensor for sensor in InavEnums.sensors_e if sensor_status_raw & sensor.value]
        return {
            "cycleTime": rep["cycleTime"],
            "i2cErrors": rep["i2cErrors"],
            "sensorStatus": sensor_status,
            "cpuLoad": rep["cpuLoad"],
            "profileAndBattProfile": rep["profileAndBattProfile"],
            "armingFlags": arming_flags,
            "activeModes": active_modes,
            "mixerProfile": rep["mixerProfile"],
        }

    def get_inav_analog(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP2_INAV_ANALOG)
        battery_flags_raw = rep["batteryFlags"]
        battery_state = InavEnums.batteryState_e((battery_flags_raw >> 2) & 0x3)
        return {
            "batteryFlags": {
                "fullOnPlugIn": bool(battery_flags_raw & 0x1),
                "useCapacityThreshold": bool(battery_flags_raw & 0x2),
                "state": battery_state,
                "cellCount": battery_flags_raw >> 4,
            },
            "vbat": rep["vbat"] / 100.0,
            "amperage": rep["amperage"] / 100.0,
            "powerDraw": rep["powerDraw"] / 1000.0,
            "mAhDrawn": rep["mAhDrawn"],
            "mWhDrawn": rep["mWhDrawn"],
            "remainingCapacity": rep["remainingCapacity"],
            "percentageRemaining": rep["percentageRemaining"],
            "rssi": rep["rssi"],
        }

    def get_rx_config(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP_RX_CONFIG)
        return {
            "serialRxProvider": InavEnums.rxSerialReceiverType_e(rep["serialRxProvider"]),
            "maxCheck": rep["maxCheck"],
            "midRc": rep["midRc"],
            "minCheck": rep["minCheck"],
            "spektrumSatBind": rep["spektrumSatBind"],
            "rxMinUsec": rep["rxMinUsec"],
            "rxMaxUsec": rep["rxMaxUsec"],
            "bfCompatRcInterpolation": rep["bfCompatRcInterpolation"],
            "bfCompatRcInterpolationInt": rep["bfCompatRcInterpolationInt"],
            "bfCompatAirModeThreshold": rep["bfCompatAirModeThreshold"],
            "reserved1": rep["reserved1"],
            "reserved2": rep["reserved2"],
            "reserved3": rep["reserved3"],
            "bfCompatFpvCamAngle": rep["bfCompatFpvCamAngle"],
            "receiverType": InavEnums.rxReceiverType_e(rep["receiverType"]),
        }

    def get_logic_condition(self, condition_index: int) -> Dict[str, Any]:
        payload = self._pack_request(
            InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE, {"conditionIndex": condition_index}
        )
        self.info, rep = self._request(InavMSP.MSP2_INAV_LOGIC_CONDITIONS_SINGLE, payload)
        flags_raw = rep["flags"]
        return {
            "enabled": bool(rep["enabled"]),
            "activatorId": None if rep["activatorId"] in (-1, 0xFF) else rep["activatorId"],
            "operation": InavEnums.logicOperation_e(rep["operation"]),
            "operandAType": InavEnums.logicOperandType_e(rep["operandAType"]),
            "operandAValue": rep["operandAValue"],
            "operandBType": InavEnums.logicOperandType_e(rep["operandBType"]),
            "operandBValue": rep["operandBValue"],
            "flags": [
                flag for flag in InavEnums.logicConditionFlags_e if flags_raw & flag.value
            ],
        }

    def get_attitude(self) -> Dict[str, float]:
        self.info, rep = self._request(InavMSP.MSP_ATTITUDE)
        return {
            "roll": rep["roll"] / 10.0,
            "pitch": rep["pitch"] / 10.0,
            "yaw": float(rep["yaw"]),
        }

    def get_altitude(self) -> Dict[str, float]:
        self.info, rep = self._request(InavMSP.MSP_ALTITUDE)
        return {
            "estimatedAltitude": rep["estimatedAltitude"] / 100.0,
            "variometer": rep["variometer"] / 100.0,
            "baroAltitude": rep["baroAltitude"] / 100.0,
        }

    def get_imu(self) -> Dict[str, Dict[str, float]]:
        self.info, rep = self._request(InavMSP.MSP_RAW_IMU)
        axes = ("X", "Y", "Z")
        return {
            "acc": {axis: rep[f"acc{axis}"] / 512.0 for axis in axes},
            "gyro": {axis: rep[f"gyro{axis}"] for axis in axes},
            "mag": {axis: rep[f"mag{axis}"] for axis in axes},
        }

    def get_rc_channels(self) -> List[int]:
        self.info, payload = self._request_raw(InavMSP.MSP_RC)
        channel_width = 2
        if len(payload) % channel_width:
            raise ValueError("RC payload not aligned to 16-bit channel width")
        channel_count = len(payload) // channel_width
        values = list(struct.unpack(f"<{channel_count}H", payload)) if channel_count else []
        return values

    def get_ch(self, channel: Union[int, str]) -> int:
        """
        Return the current value for a channel referenced either by numeric index
        or by the friendly name defined in the RX map (e.g. 'pitch').
        """
        idx = self._resolve_channel_index(channel)
        channels = self.get_rc_channels()
        if idx >= len(channels):
            raise IndexError(f"Channel index {idx} is out of range for RC payload of size {len(channels)}")
        return channels[idx]
        
    def set_rc_channels(self, channels: Union[Sequence[int], Mapping[Union[int, str], int]]) -> Mapping[str, Any]:
        if isinstance(channels, Mapping):
            resolved = self._build_channel_frame(channels)
        else:
            resolved = [int(value) for value in channels]
        if not resolved:
            raise ValueError("channels must not be empty")
        payload = struct.pack(f"<{len(resolved)}H", *resolved)
        self.info, rep = self._request(InavMSP.MSP_SET_RAW_RC, payload)
        return rep

    def _build_channel_frame(self, overrides: Mapping[Union[int, str], int]) -> List[int]:
        if not overrides:
            raise ValueError("channels must not be empty")
        resolved: List[Tuple[int, int]] = []
        max_idx = -1
        for key, value in overrides.items():
            idx = self._resolve_channel_index(key)
            resolved.append((idx, int(value)))
            if idx > max_idx:
                max_idx = idx
        if max_idx < 0:
            raise ValueError("channels must contain at least one override")
        values = self.get_rc_channels()
        if not values:
            raise RuntimeError("Cannot preserve unspecified channels because MSP_RC returned no channel data")
        if len(values) <= max_idx:
            raise IndexError(f"Channel index {max_idx} is out of range for RC payload of size {len(values)}")
        for idx, value in resolved:
            values[idx] = value
        return values

    @staticmethod
    def _axis_mask_from_names(axes: Sequence[str]) -> int:
        if not axes:
            raise ValueError("axes must not be empty")
        mask = 0
        for axis in axes:
            name = str(axis).strip().lower()
            if name == "roll":
                mask |= 0x01
            elif name == "pitch":
                mask |= 0x02
            elif name == "yaw":
                mask |= 0x04
            else:
                raise ValueError(f"Unsupported axis '{axis}' (use roll/pitch/yaw)")
        return mask

    def get_battery_config(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP2_INAV_BATTERY_CONFIG)
        return {
            "vbatScale": rep["vbatScale"],
            "vbatSource": InavEnums.batVoltageSource_e(rep["vbatSource"]),
            "cellCount": rep["cellCount"],
            "vbatCellDetect": rep["vbatCellDetect"] / 100.0,
            "vbatMinCell": rep["vbatMinCell"] / 100.0,
            "vbatMaxCell": rep["vbatMaxCell"] / 100.0,
            "vbatWarningCell": rep["vbatWarningCell"] / 100.0,
            "currentOffset": rep["currentOffset"],
            "currentScale": rep["currentScale"],
            "capacityValue": rep["capacityValue"],
            "capacityWarning": rep["capacityWarning"],
            "capacityCritical": rep["capacityCritical"],
            "capacityUnit": InavEnums.batCapacityUnit_e(rep["capacityUnit"]),
        }

    def get_gps_statistics(self) -> Dict[str, float]:
        self.info, rep = self._request(InavMSP.MSP_GPSSTATISTICS)
        packet_count = max(rep["packetCount"], 1)
        return {
            "lastMessageDt": rep["lastMessageDt"] / 1000.0,
            "errors": rep["errors"] / packet_count,
            "timeouts": rep["timeouts"] / packet_count,
            "hdop": rep["hdop"] / 100.0,
            "eph": rep["eph"] / 100.0,
            "epv": rep["epv"] / 100.0,
        }

    def get_link_stats(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP2_INAV_GET_LINK_STATS)
        return {
            "uplinkRSSI_dBm": rep["uplinkRSSI_dBm"],
            "uplinkLQ": rep["uplinkLQ"],
            "uplinkSNR": rep["uplinkSNR"],
        }

    def get_dronecan_nodes(self) -> List[Dict[str, int]]:
        """Detected DroneCAN nodes, one dict per node.

        The schema models the reply as nodeCount followed by nodeCount 13-byte
        records, so the codec returns them already decoded.
        """
        self.info, rep = self._request(InavMSP.MSP2_INAV_DRONECAN_NODES)
        return rep["items"]

    def get_waypoint_info(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP_WP_GETINFO)
        mission_valid = bool(rep["missionValid"])
        payload = {
            "wpCapabilities": rep["wpCapabilities"],
            "maxWaypoints": rep["maxWaypoints"],
            "missionValid": mission_valid,
            "waypointCount": rep["waypointCount"],
        }
        if mission_valid:
            remaining = rep["maxWaypoints"] - rep["waypointCount"]
            payload["waypointsRemaining"] = max(remaining, 0)
        return payload

    def get_raw_gps(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP_RAW_GPS)
        return {
            "fixType": InavEnums.gpsFixType_e(rep["fixType"]),
            "numSat": rep["numSat"],
            "latitude": rep["latitude"] / 1e7,
            "longitude": rep["longitude"] / 1e7,
            "altitude": rep["altitude"] / 100.0,
            "speed": rep["speed"] / 100.0,
            "groundCourse": rep["groundCourse"] / 10.0,
        }

    def set_waypoint(
        self,
        *,
        waypointIndex: int,
        action: InavEnums.navWaypointActions_e,
        latitude: float,
        longitude: float,
        altitude: float,
        param1: int,
        param2: int,
        param3: int,
        flag: int,
    ) -> Mapping[str, Any]:
        payload = self._pack_request(
            InavMSP.MSP_SET_WP,
            {
                "waypointIndex": waypointIndex,
                "action": int(action),
                "latitude": int(round(latitude * 1e7)),
                "longitude": int(round(longitude * 1e7)),
                "altitude": int(round(altitude * 100.0)),
                "param1": param1,
                "param2": param2,
                "param3": param3,
                "flag": flag,
            },
        )
        self.info, rep = self._request(InavMSP.MSP_SET_WP, payload)
        return rep

    def get_waypoint(self, waypoint_index: int) -> Dict[str, Any]:
        payload = self._pack_request(InavMSP.MSP_WP, {"waypointIndex": waypoint_index})
        self.info, rep = self._request(InavMSP.MSP_WP, payload)
        return {
            "waypointIndex": rep["waypointIndex"],
            "action": InavEnums.navWaypointActions_e(rep["action"]),
            "latitude": rep["latitude"] / 1e7,
            "longitude": rep["longitude"] / 1e7,
            "altitude": rep["altitude"] / 100.0,
            "param1": rep["param1"],
            "param2": rep["param2"],
            "param3": rep["param3"],
            "flag": rep["flag"],
        }

    def get_nav_status(self) -> Dict[str, Any]:
        self.info, rep = self._request(InavMSP.MSP_NAV_STATUS)
        active_wp_action = rep["activeWpAction"]
        active_wp_action_enum = InavEnums.navWaypointActions_e._value2member_map_.get(active_wp_action, active_wp_action)
        return {
            "navMode": InavEnums.navSystemStatus_Mode_e(rep["navMode"]),
            "navState": InavEnums.navigationFSMState_t(rep["navState"]),
            "activeWaypoint": {
                "action": active_wp_action_enum,
                "number": rep["activeWpNumber"],
            },
            "navError": InavEnums.navSystemStatus_Error_e(rep["navError"]),
            "targetHeading": rep["targetHeading"],
        }

    def set_waypoint_index(self, waypoint_index: int) -> Mapping[str, Any]:
        payload = self._pack_request(
            InavMSP.MSP2_INAV_SET_WP_INDEX,
            {"wp_index": waypoint_index},
        )
        self.info, rep = self._request(InavMSP.MSP2_INAV_SET_WP_INDEX, payload)
        return rep

    def set_cruise_heading(self, heading_deg: float) -> Mapping[str, Any]:
        payload = self._pack_request(
            InavMSP.MSP2_INAV_SET_CRUISE_HEADING,
            {"heading_centidegrees": int(round(heading_deg * 100.0))},
        )
        self.info, rep = self._request(InavMSP.MSP2_INAV_SET_CRUISE_HEADING, payload)
        return rep

    def set_armed(self, arm: bool) -> Mapping[str, Any]:
        payload = self._pack_request(InavMSP.MSP2_INAV_ARM_DISARM, {"arm": int(arm)})
        self.info, rep = self._request(InavMSP.MSP2_INAV_ARM_DISARM, payload)
        return rep

    def activate_rth(self) -> Mapping[str, Any]:
        self.info, rep = self._request(InavMSP.MSP2_INAV_ACTIVATE_RTH)
        return rep

    def activate_landing(self) -> Mapping[str, Any]:
        self.info, rep = self._request(InavMSP.MSP2_INAV_ACTIVATE_LANDING)
        return rep

    def get_timesync_ns(self) -> int:
        self.info, rep = self._request(InavMSP.MSP2_INAV_TIMESYNC)
        return rep["timeNs"]

    def set_aux_rc(
        self,
        start_channel_index: int,
        channel_values: Sequence[int],
        *,
        resolution_bits: int = 16,
    ) -> Mapping[str, Any]:
        resolution_mode = AUX_RC_RESOLUTION_MODES[resolution_bits]
        channel_data: List[int] = []

        if resolution_bits == 16:
            for value in channel_values:
                channel_data.extend(struct.pack("<H", value))
        else:
            raw_values = [
                0 if value == 0 else 1 + int(round((value - 1000) * ((1 << resolution_bits) - 2) / 1000.0))
                for value in channel_values
            ]
            if resolution_bits == 8:
                channel_data = raw_values
            else:
                channels_per_byte = 8 // resolution_bits
                for offset in range(0, len(raw_values), channels_per_byte):
                    packed = 0
                    for subchannel in range(channels_per_byte):
                        raw_index = offset + subchannel
                        raw_value = raw_values[raw_index] if raw_index < len(raw_values) else 0
                        shift = (channels_per_byte - subchannel - 1) * resolution_bits
                        packed |= raw_value << shift
                    channel_data.append(packed)

        payload = self._pack_request(
            InavMSP.MSP2_INAV_SET_AUX_RC,
            {
                "definitionByte": (start_channel_index << 3) | resolution_mode,
                "channelData": channel_data,
            },
        )
        self.info, rep = self._request(InavMSP.MSP2_INAV_SET_AUX_RC, payload)
        return rep


    def set_heading(self, heading_deg: int) -> Mapping[str, Any]:
        payload = self._pack_request(
            InavMSP.MSP_SET_HEAD,
            {
                "heading": int(heading_deg),
            },
        )
        self.info, rep = self._request(InavMSP.MSP_SET_HEAD, payload)
        return rep

    def get_active_modes(self) -> List[Union[boxes.BoxEnum, int]]:
        """
        Returns a decoded list of currently active BoxEnum values by combining NAV/INAV status data with cached box IDs.
        Falls back to MSP2_INAV_STATUS or MSP_ACTIVEBOXES if MSP_NAV_STATUS does not provide the bitmask.
        """
        self._ensure_box_ids_cached()
        nav_status = self.get_nav_status()
        raw_modes = nav_status.get("activeModes")
        has_raw_modes = "activeModes" in nav_status
        mask = self._active_mode_mask_from_raw(raw_modes)

        if not has_raw_modes:
            status = self.get_inav_status()
            raw_modes = status.get("activeModes")
            has_raw_modes = "activeModes" in status
            mask = self._active_mode_mask_from_raw(raw_modes)

        if not has_raw_modes:
            self.info, rep = self._request(InavMSP.MSP_ACTIVEBOXES)
            mask = self._active_mode_mask_from_raw(rep.get("activeModes"))

        return self._decode_active_modes_mask(mask)

    def set_simulator(
        self,
        simulator_version: int,
        flags: int,
        *,
        gps: Mapping[str, Any],
        attitude: Mapping[str, float],
        acc: Sequence[float],
        gyro: Sequence[float],
        baro_pressure: float,
        mag: Sequence[int],
        battery_voltage: float,
        airspeed: float,
        ext_flags: int,
    ) -> Mapping[str, Any]:
        gps_fix_type = int(gps["fix_type"])
        gps_num_sat = int(gps["num_sat"])
        gps_lat = _scale(gps["lat"], 1e7)
        gps_lon = _scale(gps["lon"], 1e7)
        gps_alt = _scale(gps["alt"], 100.0)
        gps_speed = _scale(gps["speed"], 100.0)
        gps_course = _scale(gps["course"], 10.0)
        gps_vel_n = _scale(gps["vel_n"], 100.0)
        gps_vel_e = _scale(gps["vel_e"], 100.0)
        gps_vel_d = _scale(gps["vel_d"], 100.0)

        imu_roll = _scale(attitude["roll"], 10.0)
        imu_pitch = _scale(attitude["pitch"], 10.0)
        imu_yaw = _scale(attitude["yaw"], 10.0)

        acc_x, acc_y, acc_z = _vec3(acc, 1000.0)
        gyro_x, gyro_y, gyro_z = _vec3(gyro, 16.0)
        mag_x, mag_y, mag_z = _vec3(mag, 1.0)

        payload = self._pack_request(
            InavMSP.MSP_SIMULATOR,
            {
                "simulatorVersion": simulator_version,
                "simulatorFlags_t": flags,
                "gpsFixType": gps_fix_type,
                "gpsNumSat": gps_num_sat,
                "gpsLat": gps_lat,
                "gpsLon": gps_lon,
                "gpsAlt": gps_alt,
                "gpsSpeed": gps_speed,
                "gpsCourse": gps_course,
                "gpsVelN": gps_vel_n,
                "gpsVelE": gps_vel_e,
                "gpsVelD": gps_vel_d,
                "imuRoll": imu_roll,
                "imuPitch": imu_pitch,
                "imuYaw": imu_yaw,
                "accX": acc_x,
                "accY": acc_y,
                "accZ": acc_z,
                "gyroX": gyro_x,
                "gyroY": gyro_y,
                "gyroZ": gyro_z,
                "baroPressure": _scale(baro_pressure, 1.0),
                "magX": mag_x,
                "magY": mag_y,
                "magZ": mag_z,
                "vbat": _scale(battery_voltage, 10.0),
                "airspeed": _scale(airspeed, 100.0),
                "extFlags": ext_flags,
            },
        )
        self.info, raw_reply = self._request_raw(InavMSP.MSP_SIMULATOR, payload)
        if not raw_reply:
            return {}
        return self._codec.unpack_reply(InavMSP.MSP_SIMULATOR, raw_reply)

    if hasattr(InavMSP, "MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE"):
        def set_flight_axis_angle_override(
            self,
            *,
            roll_deg: Optional[float] = None,
            pitch_deg: Optional[float] = None,
            yaw_deg: Optional[float] = None,
        ) -> Mapping[str, Any]:
            if roll_deg is None and pitch_deg is None and yaw_deg is None:
                payload = self._pack_request(
                    InavMSP.MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE,
                    {
                        "overrideMask": 0,
                        "angleTargetRoll": 0,
                        "angleTargetPitch": 0,
                        "angleTargetYaw": 0,
                    },
                )
                self.info, rep = self._request(InavMSP.MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE, payload)
                return rep

            mask = 0
            roll_target = 0
            pitch_target = 0
            yaw_target = 0

            if roll_deg is not None:
                mask |= 0x01
                roll_target = int(round(roll_deg * 10.0))
            if pitch_deg is not None:
                mask |= 0x02
                pitch_target = int(round(pitch_deg * 10.0))
            if yaw_deg is not None:
                mask |= 0x04
                yaw_target = int(round(yaw_deg * 10.0))

            if roll_target < -32768:
                roll_target = -32768
            if roll_target > 32767:
                roll_target = 32767

            if pitch_target < -32768:
                pitch_target = -32768
            if pitch_target > 32767:
                pitch_target = 32767

            if yaw_target < 0:
                yaw_target = 0
            if yaw_target > 3600:
                yaw_target = 3600

            payload = self._pack_request(
                InavMSP.MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE,
                {
                    "overrideMask": mask,
                    "angleTargetRoll": roll_target,
                    "angleTargetPitch": pitch_target,
                    "angleTargetYaw": yaw_target,
                },
            )
            self.info, rep = self._request(InavMSP.MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE, payload)
            return rep

        def set_flight_axis_rate_override(
            self,
            *,
            roll_dps: Optional[float] = None,
            pitch_dps: Optional[float] = None,
            yaw_dps: Optional[float] = None,
        ) -> Mapping[str, Any]:
            if roll_dps is None and pitch_dps is None and yaw_dps is None:
                payload = self._pack_request(
                    InavMSP.MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE,
                    {
                        "overrideMask": 0,
                        "rateTargetRoll": 0,
                        "rateTargetPitch": 0,
                        "rateTargetYaw": 0,
                    },
                )
                self.info, rep = self._request(InavMSP.MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE, payload)
                return rep

            mask = 0
            roll_target = 0
            pitch_target = 0
            yaw_target = 0

            if roll_dps is not None:
                mask |= 0x01
                roll_target = int(round(roll_dps))
            if pitch_dps is not None:
                mask |= 0x02
                pitch_target = int(round(pitch_dps))
            if yaw_dps is not None:
                mask |= 0x04
                yaw_target = int(round(yaw_dps))

            if roll_target < -2000:
                roll_target = -2000
            if roll_target > 2000:
                roll_target = 2000

            if pitch_target < -2000:
                pitch_target = -2000
            if pitch_target > 2000:
                pitch_target = 2000

            if yaw_target < -2000:
                yaw_target = -2000
            if yaw_target > 2000:
                yaw_target = 2000

            payload = self._pack_request(
                InavMSP.MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE,
                {
                    "overrideMask": mask,
                    "rateTargetRoll": roll_target,
                    "rateTargetPitch": pitch_target,
                    "rateTargetYaw": yaw_target,
                },
            )
            self.info, rep = self._request(InavMSP.MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE, payload)
            return rep

        def set_altitude_target(
            self,
            *,            
            altitude_m: float,
            altitude_datum: Union[int, "InavEnums.geoAltitudeDatumFlag_e"] = InavEnums.geoAltitudeDatumFlag_e.NAV_WP_TAKEOFF_DATUM,
        ) -> Mapping[str, Any]:
            payload = self._pack_request(
                InavMSP.MSP2_INAV_SET_ALT_TARGET,
                {
                    "altitudeDatum": int(altitude_datum),
                    "altitudeTarget": int(round(altitude_m * 100.0)),
                },
            )
            self.info, rep = self._request(InavMSP.MSP2_INAV_SET_ALT_TARGET, payload)
            return rep

        def set_local_target(
            self,
            *,
            x_cm: float,
            y_cm: float,
            z_cm: float,
        ) -> Mapping[str, Any]:
            payload_map = {
                "posX": int(round(x_cm)),
                "posY": int(round(y_cm)),
                "posZ": int(round(z_cm)),
            }
            payload = self._pack_request(InavMSP.MSP2_INAV_SET_LOCAL_TARGET, payload_map)
            self.info, rep = self._request(InavMSP.MSP2_INAV_SET_LOCAL_TARGET, payload)
            return rep

        def get_local_target(self) -> Dict[str, Any]:
            self.info, rep = self._request(InavMSP.MSP2_INAV_LOCAL_TARGET)
            return {
                "pos": {
                    "x_cm": rep["posX"],
                    "y_cm": rep["posY"],
                    "z_cm": rep["posZ"],
                },
                "vel": {
                    "x_cm_s": rep["velX"],
                    "y_cm_s": rep["velY"],
                    "z_cm_s": rep["velZ"],
                },
                "yaw_deg": rep["yaw"] / 100.0,
                "climb_rate_ms": rep["climbRate"] / 100.0,
            }

        def set_global_target(
            self,
            *,
            latitude_deg: float,
            longitude_deg: float,
            altitude_m: Optional[float],
            altitude_datum: Union[int, "InavEnums.geoAltitudeDatumFlag_e"] = InavEnums.geoAltitudeDatumFlag_e.NAV_WP_TAKEOFF_DATUM,
            loiter_radius_m: Optional[float] = None,
        ) -> Mapping[str, Any]:
            altitude_cm = 0 if altitude_m is None else int(round(altitude_m * 100.0))
            payload_values = {
                "latitude": int(round(latitude_deg * 1e7)),
                "longitude": int(round(longitude_deg * 1e7)),
                "altitudeTarget": altitude_cm,
                "altitudeDatum": int(altitude_datum),
            }
            if loiter_radius_m is not None:
                payload_values["loiterRadius"] = int(round(loiter_radius_m * 100.0))
            payload = self._pack_request(
                InavMSP.MSP2_INAV_SET_GLOBAL_TARGET,
                payload_values,
            )
            self.info, rep = self._request(InavMSP.MSP2_INAV_SET_GLOBAL_TARGET, payload)
            return rep

        def get_nav_target(self) -> Dict[str, Any]:
            self.info, rep = self._request(InavMSP.MSP2_INAV_NAV_TARGET)
            result = {
                "latitude": rep["latTarget"] / 1e7,
                "longitude": rep["lonTarget"] / 1e7,
                "altitude_m": rep["altitudeTarget"] / 100.0,
                "heading_deg": rep["headingTarget"] / 1.0,
                "climb_rate_ms": rep["climbRate"] / 100.0,
            }
            if "loiterRadius" in rep:
                result["loiter_radius_m"] = rep["loiterRadius"] / 100.0
            return result
    else:
        def set_flight_axis_angle_override(self, *, roll_deg: Optional[float] = None, pitch_deg: Optional[float] = None, yaw_deg: Optional[float] = None) -> Mapping[str, Any]:
            raise RuntimeError("API version mismatch")

        def set_flight_axis_rate_override(self, *, roll_dps: Optional[float] = None, pitch_dps: Optional[float] = None, yaw_dps: Optional[float] = None) -> Mapping[str, Any]:
            raise RuntimeError("API version mismatch")

        def set_altitude_target(self, *, altitude_datum: Union[int, "InavEnums.geoAltitudeDatumFlag_e"], altitude_m: float) -> Mapping[str, Any]:
            raise RuntimeError("API version mismatch")

        def set_local_target(self, *, x_cm: float, y_cm: float, z_cm: float) -> Mapping[str, Any]:
            raise RuntimeError("API version mismatch")

        def get_local_target(self) -> Dict[str, Any]:
            raise RuntimeError("API version mismatch")

        def set_global_target(self, *, latitude_deg: float, longitude_deg: float, altitude_m: Optional[float], altitude_datum: Union[int, "InavEnums.geoAltitudeDatumFlag_e"]) -> Mapping[str, Any]:
            raise RuntimeError("API version mismatch")

        def get_nav_target(self) -> Dict[str, Any]:
            raise RuntimeError("API version mismatch")

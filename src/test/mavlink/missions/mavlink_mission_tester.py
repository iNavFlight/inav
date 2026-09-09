#!/usr/bin/env python3
"""
Usage:
  conda run -n drone python src/test/mavlink/missions/mavlink_mission_tester.py
  conda run -n drone python src/test/mavlink/missions/mavlink_mission_tester.py --config src/test/mavlink/missions/mavlink_mission_tester.ini

Expected external setup:
  ./cmake/build_SITL/inav_9.1.0_SITL --serialport=/dev/ttyUSB0 --serialuart=3 --baudrate=460800 --path="../mydev/branch/mavlink_multiport2/eeprom.bin" --chanmap=M01-01,S02-02,S01-03,S04-04
  cd src/test/mavlink/missions/results
  mavproxy.py --master=tcp:127.0.0.1:5763 --force-connected --nowait --daemon --out=udp:127.0.0.1:14550
"""

from __future__ import annotations

import argparse
import configparser
import json
import os
import sys
import time
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable

os.environ["MAVLINK20"] = "1"

from pymavlink import mavutil

try:
    from mspapi2.msp_api import MSPApi
    from mspapi2.lib import InavEnums
except ModuleNotFoundError:
    workspace_root_guess = Path(__file__).resolve().parents[5]
    mspapi2_repo = workspace_root_guess / "mspapi2"
    sys.path.insert(0, str(mspapi2_repo))
    from mspapi2.msp_api import MSPApi
    from mspapi2.lib import InavEnums


INAV_ROOT = Path(__file__).resolve().parents[4]
WORKSPACE_ROOT = INAV_ROOT.parent
DEFAULT_CONFIG_PATH = Path(__file__).resolve().with_suffix(".ini")

BASE_LAT_E7 = 365304400
BASE_LON_E7 = -832163830
LAT_STEP_E7 = 2500
LON_STEP_E7 = 3500

NAV_WP_ACTION_WAYPOINT = int(InavEnums.navWaypointActions_e.NAV_WP_ACTION_WAYPOINT)
NAV_WP_ACTION_HOLD_TIME = int(InavEnums.navWaypointActions_e.NAV_WP_ACTION_HOLD_TIME)
NAV_WP_ACTION_JUMP = int(InavEnums.navWaypointActions_e.NAV_WP_ACTION_JUMP)
NAV_WP_ACTION_LAND = int(InavEnums.navWaypointActions_e.NAV_WP_ACTION_LAND)
NAV_WP_FLAG_LAST = int(InavEnums.navWaypointFlags_e.NAV_WP_FLAG_LAST)
NAV_WP_ALTMODE = int(InavEnums.navWaypointP3Flags_e.NAV_WP_ALTMODE)

MAV_MISSION_ACCEPTED = int(mavutil.mavlink.MAV_MISSION_ACCEPTED)
MAV_MISSION_UNSUPPORTED = int(mavutil.mavlink.MAV_MISSION_UNSUPPORTED)
MAV_MISSION_TYPE_MISSION = int(mavutil.mavlink.MAV_MISSION_TYPE_MISSION)
MAV_AUTOPILOT_INVALID = int(mavutil.mavlink.MAV_AUTOPILOT_INVALID)
MAV_TYPE_GCS = int(mavutil.mavlink.MAV_TYPE_GCS)

LIVE_STATUS_ERROR_WORDS = (
    "error",
    "fail",
    "failed",
    "invalid",
    "unsupported",
    "denied",
    "mission",
    "waypoint",
)


@dataclass(frozen=True)
class RuntimeConfig:
    mavlink_endpoint: str
    msp_tcp_endpoint: str
    target_component: int
    source_system: int
    source_component: int
    log_dir: Path
    report_path: Path
    heartbeat_timeout_s: float
    mission_timeout_s: float
    clear_timeout_s: float
    msp_read_timeout_ms: float
    msp_write_timeout_ms: float
    msp_verify_settle_s: float


@dataclass(frozen=True)
class MissionItem:
    seq: int
    frame: int
    command: int
    current: int
    autocontinue: int
    param1: float
    param2: float
    param3: float
    param4: float
    x: int
    y: int
    z: float


@dataclass(frozen=True)
class ExpectedWaypoint:
    waypointIndex: int
    action: int
    latitudeE7: int
    longitudeE7: int
    altitudeCm: int
    param1: int
    param2: int
    param3: int
    flag: int


@dataclass(frozen=True)
class MissionCase:
    name: str
    description: str
    items: tuple[MissionItem, ...]
    expected_upload_result: int
    expected_waypoints: tuple[ExpectedWaypoint, ...]
    preload_items: tuple[MissionItem, ...] = ()
    verify_legacy_download: bool = False


def resolve_inav_path(path_text: str) -> Path:
    path = Path(path_text).expanduser()
    if path.is_absolute():
        return path
    return INAV_ROOT / path


def load_config(config_path: Path) -> RuntimeConfig:
    parser = configparser.ConfigParser()
    parser.read(config_path)
    return RuntimeConfig(
        mavlink_endpoint=parser["connection"]["mavlink_endpoint"],
        msp_tcp_endpoint=parser["connection"]["msp_tcp_endpoint"],
        target_component=parser["connection"].getint("target_component"),
        source_system=parser["connection"].getint("source_system"),
        source_component=parser["connection"].getint("source_component"),
        log_dir=resolve_inav_path(parser["paths"]["log_dir"]),
        report_path=resolve_inav_path(parser["paths"]["report_path"]),
        heartbeat_timeout_s=parser["timeouts"].getfloat("heartbeat_timeout_s"),
        mission_timeout_s=parser["timeouts"].getfloat("mission_timeout_s"),
        clear_timeout_s=parser["timeouts"].getfloat("clear_timeout_s"),
        msp_read_timeout_ms=parser["timeouts"].getfloat("msp_read_timeout_ms"),
        msp_write_timeout_ms=parser["timeouts"].getfloat("msp_write_timeout_ms"),
        msp_verify_settle_s=parser["timeouts"].getfloat("msp_verify_settle_s"),
    )


def lat_e7(offset: int) -> int:
    return BASE_LAT_E7 + offset * LAT_STEP_E7


def lon_e7(offset: int) -> int:
    return BASE_LON_E7 + offset * LON_STEP_E7


def meters_to_centimeters(meters: float) -> int:
    return int(round(meters * 100.0))


def mission_item(
    seq: int,
    frame: int,
    command: int,
    *,
    current: int = 0,
    autocontinue: int = 1,
    param1: float = 0.0,
    param2: float = 0.0,
    param3: float = 0.0,
    param4: float = 0.0,
    latitudeE7: int = 0,
    longitudeE7: int = 0,
    altitudeMeters: float = 0.0,
) -> MissionItem:
    return MissionItem(
        seq=seq,
        frame=frame,
        command=command,
        current=current,
        autocontinue=autocontinue,
        param1=param1,
        param2=param2,
        param3=param3,
        param4=param4,
        x=latitudeE7,
        y=longitudeE7,
        z=altitudeMeters,
    )


def expected_waypoint(
    waypoint_index: int,
    action: int,
    latitudeE7: int,
    longitudeE7: int,
    altitudeMeters: float,
    *,
    param1: int = 0,
    param2: int = 0,
    param3: int = 0,
    flag: int = 0,
) -> ExpectedWaypoint:
    return ExpectedWaypoint(
        waypointIndex=waypoint_index,
        action=action,
        latitudeE7=latitudeE7,
        longitudeE7=longitudeE7,
        altitudeCm=meters_to_centimeters(altitudeMeters),
        param1=param1,
        param2=param2,
        param3=param3,
        flag=flag,
    )


def mission_result_name(result: int) -> str:
    return mavutil.mavlink.enums["MAV_MISSION_RESULT"][int(result)].name


def command_name(command: int) -> str:
    return mavutil.mavlink.enums["MAV_CMD"][int(command)].name


def frame_name(frame: int) -> str:
    return mavutil.mavlink.enums["MAV_FRAME"][int(frame)].name


def waypoint_action_name(action: int) -> str:
    return InavEnums.navWaypointActions_e(int(action)).name


def make_cases() -> tuple[MissionCase, ...]:
    global_int = int(mavutil.mavlink.MAV_FRAME_GLOBAL_INT)
    global_relative_alt_int = int(mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT_INT)
    mission_frame = int(mavutil.mavlink.MAV_FRAME_MISSION)

    nav_waypoint = int(mavutil.mavlink.MAV_CMD_NAV_WAYPOINT)
    nav_loiter_time = int(mavutil.mavlink.MAV_CMD_NAV_LOITER_TIME)
    nav_land = int(mavutil.mavlink.MAV_CMD_NAV_LAND)
    nav_takeoff = int(mavutil.mavlink.MAV_CMD_NAV_TAKEOFF)
    do_jump = int(mavutil.mavlink.MAV_CMD_DO_JUMP)
    do_change_speed = int(mavutil.mavlink.MAV_CMD_DO_CHANGE_SPEED)
    condition_delay = int(mavutil.mavlink.MAV_CMD_CONDITION_DELAY)
    condition_change_alt = int(mavutil.mavlink.MAV_CMD_CONDITION_CHANGE_ALT)
    do_change_altitude = int(mavutil.mavlink.MAV_CMD_DO_CHANGE_ALTITUDE)

    return (
        MissionCase(
            name="qgc_planned_home_waypoints_land",
            description="QGC planned home item is skipped; MAVLink seq 1 becomes INAV WP1.",
            expected_upload_result=MAV_MISSION_ACCEPTED,
            items=(
                mission_item(0, global_int, nav_waypoint, latitudeE7=lat_e7(0), longitudeE7=lon_e7(0), altitudeMeters=0.0),
                mission_item(1, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(1), longitudeE7=lon_e7(1), altitudeMeters=50.0),
                mission_item(2, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(2), longitudeE7=lon_e7(2), altitudeMeters=70.0),
                mission_item(3, global_relative_alt_int, nav_land, latitudeE7=lat_e7(3), longitudeE7=lon_e7(3), altitudeMeters=0.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_WAYPOINT, lat_e7(1), lon_e7(1), 50.0),
                expected_waypoint(2, NAV_WP_ACTION_WAYPOINT, lat_e7(2), lon_e7(2), 70.0),
                expected_waypoint(3, NAV_WP_ACTION_LAND, lat_e7(3), lon_e7(3), 0.0, flag=NAV_WP_FLAG_LAST),
            ),
        ),
        MissionCase(
            name="waypoint_hold_time_and_condition_delay",
            description="Waypoint hold time plus CONDITION_DELAY folds into one INAV HOLD_TIME item.",
            expected_upload_result=MAV_MISSION_ACCEPTED,
            items=(
                mission_item(0, global_int, nav_waypoint, latitudeE7=lat_e7(0), longitudeE7=lon_e7(0), altitudeMeters=0.0),
                mission_item(1, global_relative_alt_int, nav_waypoint, param1=5.0, latitudeE7=lat_e7(4), longitudeE7=lon_e7(4), altitudeMeters=55.0),
                mission_item(2, mission_frame, condition_delay, param1=7.0),
                mission_item(3, global_relative_alt_int, nav_land, latitudeE7=lat_e7(5), longitudeE7=lon_e7(5), altitudeMeters=0.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_HOLD_TIME, lat_e7(4), lon_e7(4), 55.0, param1=12),
                expected_waypoint(2, NAV_WP_ACTION_LAND, lat_e7(5), lon_e7(5), 0.0, flag=NAV_WP_FLAG_LAST),
            ),
        ),
        MissionCase(
            name="absolute_first_waypoint_is_not_planned_home",
            description="A current first absolute waypoint is kept; only QGC-style non-current planned home is skipped.",
            expected_upload_result=MAV_MISSION_ACCEPTED,
            items=(
                mission_item(0, global_int, nav_waypoint, current=1, latitudeE7=lat_e7(16), longitudeE7=lon_e7(16), altitudeMeters=95.0),
                mission_item(1, global_relative_alt_int, nav_land, latitudeE7=lat_e7(17), longitudeE7=lon_e7(17), altitudeMeters=0.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_WAYPOINT, lat_e7(16), lon_e7(16), 95.0, param3=NAV_WP_ALTMODE),
                expected_waypoint(2, NAV_WP_ACTION_LAND, lat_e7(17), lon_e7(17), 0.0, flag=NAV_WP_FLAG_LAST),
            ),
        ),
        MissionCase(
            name="speed_change_applies_to_following_legs",
            description="DO_CHANGE_SPEED is a pending modifier applied to later geographic legs.",
            expected_upload_result=MAV_MISSION_ACCEPTED,
            items=(
                mission_item(0, global_int, nav_waypoint, latitudeE7=lat_e7(0), longitudeE7=lon_e7(0), altitudeMeters=0.0),
                mission_item(1, mission_frame, do_change_speed, param1=1.0, param2=12.5),
                mission_item(2, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(6), longitudeE7=lon_e7(6), altitudeMeters=60.0),
                mission_item(3, global_relative_alt_int, nav_loiter_time, param1=4.0, latitudeE7=lat_e7(7), longitudeE7=lon_e7(7), altitudeMeters=60.0),
                mission_item(4, global_relative_alt_int, nav_land, latitudeE7=lat_e7(8), longitudeE7=lon_e7(8), altitudeMeters=0.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_WAYPOINT, lat_e7(6), lon_e7(6), 60.0, param1=1250),
                expected_waypoint(2, NAV_WP_ACTION_HOLD_TIME, lat_e7(7), lon_e7(7), 60.0, param1=4, param2=1250),
                expected_waypoint(3, NAV_WP_ACTION_LAND, lat_e7(8), lon_e7(8), 0.0, param1=1250, flag=NAV_WP_FLAG_LAST),
            ),
        ),
        MissionCase(
            name="altitude_modifiers_preserve_p3_bits",
            description="Altitude modifier items update the previous geographic waypoint and preserve P3 semantics.",
            expected_upload_result=MAV_MISSION_ACCEPTED,
            items=(
                mission_item(0, global_int, nav_waypoint, latitudeE7=lat_e7(0), longitudeE7=lon_e7(0), altitudeMeters=0.0),
                mission_item(1, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(9), longitudeE7=lon_e7(9), altitudeMeters=40.0),
                mission_item(2, mission_frame, do_change_altitude, param1=65.0, param2=float(global_relative_alt_int)),
                mission_item(3, global_int, condition_change_alt, latitudeE7=0, longitudeE7=0, altitudeMeters=80.0),
                mission_item(4, global_relative_alt_int, nav_land, latitudeE7=lat_e7(10), longitudeE7=lon_e7(10), altitudeMeters=0.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_WAYPOINT, lat_e7(9), lon_e7(9), 80.0, param3=NAV_WP_ALTMODE),
                expected_waypoint(2, NAV_WP_ACTION_LAND, lat_e7(10), lon_e7(10), 0.0, flag=NAV_WP_FLAG_LAST),
            ),
        ),
        MissionCase(
            name="jump_target_remap_after_home_skip",
            description="DO_JUMP target seq is remapped after QGC planned-home skip.",
            expected_upload_result=MAV_MISSION_ACCEPTED,
            items=(
                mission_item(0, global_int, nav_waypoint, latitudeE7=lat_e7(0), longitudeE7=lon_e7(0), altitudeMeters=0.0),
                mission_item(1, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(11), longitudeE7=lon_e7(11), altitudeMeters=45.0),
                mission_item(2, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(12), longitudeE7=lon_e7(12), altitudeMeters=45.0),
                mission_item(3, mission_frame, do_jump, param1=1.0, param2=2.0),
                mission_item(4, global_relative_alt_int, nav_land, latitudeE7=lat_e7(13), longitudeE7=lon_e7(13), altitudeMeters=0.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_WAYPOINT, lat_e7(11), lon_e7(11), 45.0),
                expected_waypoint(2, NAV_WP_ACTION_WAYPOINT, lat_e7(12), lon_e7(12), 45.0),
                expected_waypoint(3, NAV_WP_ACTION_JUMP, 0, 0, 0.0, param1=1, param2=2),
                expected_waypoint(4, NAV_WP_ACTION_LAND, lat_e7(13), lon_e7(13), 0.0, flag=NAV_WP_FLAG_LAST),
            ),
            verify_legacy_download=True,
        ),
        MissionCase(
            name="takeoff_is_rejected_without_partial_commit",
            description="MAV_CMD_NAV_TAKEOFF remains unsupported and must not half-write a mission.",
            expected_upload_result=MAV_MISSION_UNSUPPORTED,
            items=(
                mission_item(0, global_relative_alt_int, nav_takeoff, latitudeE7=lat_e7(14), longitudeE7=lon_e7(14), altitudeMeters=20.0),
                mission_item(1, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(15), longitudeE7=lon_e7(15), altitudeMeters=50.0),
            ),
            expected_waypoints=(),
        ),
        MissionCase(
            name="failed_upload_preserves_existing_mission",
            description="A rejected upload after a valid mission leaves the stored INAV mission unchanged.",
            expected_upload_result=MAV_MISSION_UNSUPPORTED,
            preload_items=(
                mission_item(0, global_relative_alt_int, nav_waypoint, current=1, latitudeE7=lat_e7(18), longitudeE7=lon_e7(18), altitudeMeters=55.0),
                mission_item(1, global_relative_alt_int, nav_land, latitudeE7=lat_e7(19), longitudeE7=lon_e7(19), altitudeMeters=0.0),
            ),
            items=(
                mission_item(0, global_relative_alt_int, nav_takeoff, latitudeE7=lat_e7(20), longitudeE7=lon_e7(20), altitudeMeters=20.0),
                mission_item(1, global_relative_alt_int, nav_waypoint, latitudeE7=lat_e7(21), longitudeE7=lon_e7(21), altitudeMeters=50.0),
            ),
            expected_waypoints=(
                expected_waypoint(1, NAV_WP_ACTION_WAYPOINT, lat_e7(18), lon_e7(18), 55.0),
                expected_waypoint(2, NAV_WP_ACTION_LAND, lat_e7(19), lon_e7(19), 0.0, flag=NAV_WP_FLAG_LAST),
            ),
        ),
    )


class MissionTester:
    def __init__(self, config: RuntimeConfig) -> None:
        self.config = config
        self.connection = mavutil.mavlink_connection(
            config.mavlink_endpoint,
            source_system=config.source_system,
            source_component=config.source_component,
            dialect="common",
            autoreconnect=True,
        )
        self.connection.mav.srcSystem = config.source_system
        self.connection.mav.srcComponent = config.source_component
        self.target_system = 0
        self.target_component = config.target_component
        self.next_heartbeat_at = 0.0
        self.case_statustext: list[dict[str, Any]] = []

    def close(self) -> None:
        self.connection.close()

    def send_heartbeat(self) -> None:
        self.connection.mav.heartbeat_send(
            MAV_TYPE_GCS,
            MAV_AUTOPILOT_INVALID,
            0,
            0,
            mavutil.mavlink.MAV_STATE_ACTIVE,
        )

    def maybe_send_heartbeat(self) -> None:
        now = time.monotonic()
        if now >= self.next_heartbeat_at:
            self.send_heartbeat()
            self.next_heartbeat_at = now + 1.0

    def learn_target_from_heartbeat(self, message: Any) -> None:
        if int(message.autopilot) == MAV_AUTOPILOT_INVALID:
            return
        self.target_system = int(message.get_srcSystem())
        if self.target_component == 0:
            self.target_component = int(message.get_srcComponent())

    def wait_for_target(self) -> None:
        deadline = time.monotonic() + self.config.heartbeat_timeout_s
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=True, timeout=0.1)
            if message is None:
                continue
            if message.get_type() == "HEARTBEAT":
                self.learn_target_from_heartbeat(message)
                if self.target_system != 0:
                    print(
                        f"target_discovered system={self.target_system} component={self.target_component}",
                        flush=True,
                    )
                    return
        raise TimeoutError(f"mavlink_endpoint={self.config.mavlink_endpoint} heartbeat_timeout_s={self.config.heartbeat_timeout_s}")

    def handle_background_message(self, message: Any) -> None:
        message_type = message.get_type()
        if message_type == "HEARTBEAT":
            self.learn_target_from_heartbeat(message)
        elif message_type == "STATUSTEXT":
            text = bytes(message.text).decode("utf-8", errors="ignore").rstrip("\x00") if isinstance(message.text, list) else str(message.text).rstrip("\x00")
            self.case_statustext.append(
                {
                    "severity": int(message.severity),
                    "text": text,
                }
            )

    def drain(self, duration_s: float) -> None:
        deadline = time.monotonic() + duration_s
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=False)
            if message is None:
                time.sleep(0.01)
                continue
            self.handle_background_message(message)

    def clear_mission(self) -> int:
        self.drain(0.1)
        self.connection.mav.mission_clear_all_send(
            self.target_system,
            self.target_component,
        )
        deadline = time.monotonic() + self.config.clear_timeout_s
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=True, timeout=0.1)
            if message is None:
                continue
            self.handle_background_message(message)
            if message.get_type() == "MISSION_ACK" and int(message.get_srcSystem()) == self.target_system:
                return int(message.type)
        raise TimeoutError(f"mission_clear_timeout_s={self.config.clear_timeout_s}")

    def send_mission_item(self, item: MissionItem) -> None:
        self.connection.mav.mission_item_int_send(
            self.target_system,
            self.target_component,
            item.seq,
            item.frame,
            item.command,
            item.current,
            item.autocontinue,
            item.param1,
            item.param2,
            item.param3,
            item.param4,
            item.x,
            item.y,
            item.z,
        )

    def upload_mission(self, mission_items: tuple[MissionItem, ...]) -> int:
        self.connection.mav.mission_count_send(
            self.target_system,
            self.target_component,
            len(mission_items),
        )

        items_by_seq = {item.seq: item for item in mission_items}
        deadline = time.monotonic() + self.config.mission_timeout_s
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=True, timeout=0.1)
            if message is None:
                continue
            self.handle_background_message(message)
            if int(message.get_srcSystem()) != self.target_system:
                continue

            message_type = message.get_type()
            if message_type in ("MISSION_REQUEST", "MISSION_REQUEST_INT"):
                seq = int(message.seq)
                self.send_mission_item(items_by_seq[seq])
            elif message_type == "MISSION_ACK":
                return int(message.type)

        raise TimeoutError(f"mission_upload_timeout_s={self.config.mission_timeout_s}")

    def verify_legacy_download_uses_item_int(self, expected_count: int) -> dict[str, Any]:
        self.drain(0.1)
        self.connection.mav.mission_request_list_send(
            self.target_system,
            self.target_component,
        )

        mismatches = []
        downloaded_items = []
        deadline = time.monotonic() + self.config.mission_timeout_s
        count = None
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=True, timeout=0.1)
            if message is None:
                continue
            self.handle_background_message(message)
            if int(message.get_srcSystem()) != self.target_system:
                continue
            if message.get_type() == "MISSION_COUNT":
                count = int(message.count)
                break

        if count is None:
            raise TimeoutError(f"mission_download_count_timeout_s={self.config.mission_timeout_s}")
        if count != expected_count:
            mismatches.append(f"download_count: expected={expected_count} actual={count}")

        for seq in range(count):
            self.connection.mav.mission_request_send(
                self.target_system,
                self.target_component,
                seq,
            )
            item_deadline = time.monotonic() + self.config.mission_timeout_s
            item_message = None
            while time.monotonic() < item_deadline:
                self.maybe_send_heartbeat()
                message = self.connection.recv_match(blocking=True, timeout=0.1)
                if message is None:
                    continue
                self.handle_background_message(message)
                if int(message.get_srcSystem()) != self.target_system:
                    continue
                if message.get_type() in ("MISSION_ITEM", "MISSION_ITEM_INT", "MISSION_ACK"):
                    item_message = message
                    break

            if item_message is None:
                raise TimeoutError(f"mission_download_item_timeout_s={self.config.mission_timeout_s} seq={seq}")

            message_type = item_message.get_type()
            downloaded_items.append(
                {
                    "seq": int(getattr(item_message, "seq", -1)),
                    "message_type": message_type,
                }
            )
            if message_type != "MISSION_ITEM_INT":
                mismatches.append(f"download_seq={seq}: expected MISSION_ITEM_INT actual={message_type}")
                continue
            if int(item_message.seq) != seq:
                mismatches.append(f"download_seq: expected={seq} actual={int(item_message.seq)}")

        self.connection.mav.mission_ack_send(
            self.target_system,
            self.target_component,
            MAV_MISSION_ACCEPTED,
        )

        return {
            "items": downloaded_items,
            "mismatches": mismatches,
        }

    def upload_case(self, case: MissionCase) -> dict[str, Any]:
        self.case_statustext = []
        clear_result = self.clear_mission()
        preload_result = None
        if case.preload_items:
            preload_result = self.upload_mission(case.preload_items)
        upload_result = self.upload_mission(case.items)
        return {
            "clear_result": clear_result,
            "clear_result_name": mission_result_name(clear_result),
            "preload_result": preload_result,
            "preload_result_name": mission_result_name(preload_result) if preload_result is not None else None,
            "upload_result": upload_result,
            "upload_result_name": mission_result_name(upload_result),
            "statustext": self.case_statustext,
        }


def actual_waypoint_from_msp(waypoint: dict[str, Any]) -> ExpectedWaypoint:
    return ExpectedWaypoint(
        waypointIndex=int(waypoint["waypointIndex"]),
        action=int(waypoint["action"]),
        latitudeE7=int(round(float(waypoint["latitude"]) * 1e7)),
        longitudeE7=int(round(float(waypoint["longitude"]) * 1e7)),
        altitudeCm=int(round(float(waypoint["altitude"]) * 100.0)),
        param1=int(waypoint["param1"]),
        param2=int(waypoint["param2"]),
        param3=int(waypoint["param3"]),
        flag=int(waypoint["flag"]),
    )


def compare_waypoints(expected: ExpectedWaypoint, actual: ExpectedWaypoint) -> list[str]:
    mismatches = []
    for field_name in expected.__dataclass_fields__:
        expected_value = getattr(expected, field_name)
        actual_value = getattr(actual, field_name)
        if actual_value != expected_value:
            mismatches.append(f"{field_name}: expected={expected_value} actual={actual_value}")
    return mismatches


def verify_msp_mission(config: RuntimeConfig, case: MissionCase) -> dict[str, Any]:
    with MSPApi(
        tcp_endpoint=config.msp_tcp_endpoint,
        read_timeout_ms=config.msp_read_timeout_ms,
        write_timeout_ms=config.msp_write_timeout_ms,
    ) as api:
        waypoint_info = api.get_waypoint_info()
        actual_count = int(waypoint_info["waypointCount"])
        actual_waypoints = [
            actual_waypoint_from_msp(api.get_waypoint(waypoint_index))
            for waypoint_index in range(1, actual_count + 1)
        ]

    expected_count = len(case.expected_waypoints)
    mismatches = []
    if actual_count != expected_count:
        mismatches.append(f"waypointCount: expected={expected_count} actual={actual_count}")

    for expected, actual in zip(case.expected_waypoints, actual_waypoints):
        waypoint_mismatches = compare_waypoints(expected, actual)
        if waypoint_mismatches:
            mismatches.append(f"WP{expected.waypointIndex}: " + "; ".join(waypoint_mismatches))

    return {
        "waypoint_info": waypoint_info,
        "expected_waypoints": [asdict(waypoint) for waypoint in case.expected_waypoints],
        "actual_waypoints": [asdict(waypoint) for waypoint in actual_waypoints],
        "mismatches": mismatches,
    }


def interesting_statustext(messages: Iterable[dict[str, Any]]) -> list[dict[str, Any]]:
    interesting = []
    for message in messages:
        lower_text = str(message["text"]).lower()
        if any(word in lower_text for word in LIVE_STATUS_ERROR_WORDS):
            interesting.append(message)
    return interesting


def find_latest_mavproxy_log(log_dir: Path) -> Path | None:
    candidates = list(log_dir.glob("*.tlog")) + list(log_dir.glob("**/*.tlog"))
    if not candidates:
        return None
    return max(candidates, key=lambda path: path.stat().st_mtime)


def scan_mavproxy_log(log_dir: Path) -> dict[str, Any]:
    log_path = find_latest_mavproxy_log(log_dir)
    if log_path is None:
        return {
            "path": None,
            "interesting_statustext": [],
        }

    connection = mavutil.mavlink_connection(str(log_path), dialect="common")
    messages = []
    while True:
        message = connection.recv_match(type="STATUSTEXT", blocking=False)
        if message is None:
            break
        text = bytes(message.text).decode("utf-8", errors="ignore").rstrip("\x00") if isinstance(message.text, list) else str(message.text).rstrip("\x00")
        messages.append(
            {
                "severity": int(message.severity),
                "text": text,
            }
        )
    connection.close()

    return {
        "path": str(log_path.relative_to(INAV_ROOT)),
        "interesting_statustext": interesting_statustext(messages),
    }


def mission_item_report(item: MissionItem) -> dict[str, Any]:
    payload = asdict(item)
    payload["frame_name"] = frame_name(item.frame)
    payload["command_name"] = command_name(item.command)
    return payload


def expected_waypoint_report(waypoint: ExpectedWaypoint) -> dict[str, Any]:
    payload = asdict(waypoint)
    payload["action_name"] = waypoint_action_name(waypoint.action)
    return payload


def run_cases(config: RuntimeConfig, cases: tuple[MissionCase, ...]) -> dict[str, Any]:
    tester = MissionTester(config)
    results = []
    try:
        tester.wait_for_target()
        for case in cases:
            print(f"case_start name={case.name} items={len(case.items)}", flush=True)
            upload = tester.upload_case(case)
            preload_matches = upload["preload_result"] is None or int(upload["preload_result"]) == MAV_MISSION_ACCEPTED
            upload_matches = preload_matches and int(upload["upload_result"]) == case.expected_upload_result
            msp_report = None
            legacy_download_report = None
            if upload_matches:
                time.sleep(config.msp_verify_settle_s)
                msp_report = verify_msp_mission(config, case)
                passed = not msp_report["mismatches"]
                if case.verify_legacy_download:
                    legacy_download_report = tester.verify_legacy_download_uses_item_int(len(case.expected_waypoints))
                    passed = passed and not legacy_download_report["mismatches"]
            else:
                passed = False

            case_report = {
                "name": case.name,
                "description": case.description,
                "passed": passed,
                "expected_upload_result": case.expected_upload_result,
                "expected_upload_result_name": mission_result_name(case.expected_upload_result),
                "upload": upload,
                "mission_items": [mission_item_report(item) for item in case.items],
                "preload_items": [mission_item_report(item) for item in case.preload_items],
                "expected_waypoints": [expected_waypoint_report(waypoint) for waypoint in case.expected_waypoints],
                "msp": msp_report,
                "legacy_download": legacy_download_report,
                "interesting_statustext": interesting_statustext(upload["statustext"]),
            }
            results.append(case_report)
            print(
                f"case_done name={case.name} passed={int(passed)} "
                f"upload_result={upload['upload_result_name']}",
                flush=True,
            )
    finally:
        tester.close()

    return {
        "config": {
            "mavlink_endpoint": config.mavlink_endpoint,
            "msp_tcp_endpoint": config.msp_tcp_endpoint,
            "log_dir": str(config.log_dir.relative_to(INAV_ROOT)),
            "report_path": str(config.report_path.relative_to(INAV_ROOT)),
        },
        "cases": results,
        "mavproxy_log": scan_mavproxy_log(config.log_dir),
        "passed": all(case["passed"] for case in results),
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Upload MAVLink mission cases and verify INAV's stored MSP mission.")
    parser.add_argument("--config", default=str(DEFAULT_CONFIG_PATH), help="INI config path")
    parser.add_argument("--case", action="append", dest="case_names", help="Run one case name; may be supplied more than once")
    parser.add_argument("--list-cases", action="store_true", help="Print available mission cases without connecting")
    args = parser.parse_args()

    cases = make_cases()
    if args.list_cases:
        for case in cases:
            print(f"{case.name}: {case.description}")
        return

    if args.case_names:
        selected_names = set(args.case_names)
        cases = tuple(case for case in cases if case.name in selected_names)
        missing_names = selected_names - {case.name for case in cases}
        if missing_names:
            raise ValueError(f"unknown_case_names={sorted(missing_names)}")

    config = load_config(resolve_inav_path(args.config))
    report = run_cases(config, cases)
    config.report_path.parent.mkdir(parents=True, exist_ok=True)
    config.report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"report_path={config.report_path.relative_to(INAV_ROOT)} passed={int(report['passed'])}", flush=True)
    print("mission_test_summary_start", flush=True)
    for case_report in report["cases"]:
        print(
            f"case={case_report['name']} passed={int(case_report['passed'])} "
            f"upload={case_report['upload']['upload_result_name']} "
            f"expected_upload={case_report['expected_upload_result_name']}",
            flush=True,
        )
        msp_report = case_report["msp"]
        if msp_report is not None:
            print(
                f"case={case_report['name']} msp_mismatches={len(msp_report['mismatches'])} "
                f"waypoint_info={msp_report['waypoint_info']}",
                flush=True,
            )
            for mismatch in msp_report["mismatches"]:
                print(f"case={case_report['name']} mismatch={mismatch}", flush=True)
        for message in case_report["interesting_statustext"]:
            print(
                f"case={case_report['name']} statustext_severity={message['severity']} "
                f"statustext={message['text']}",
                flush=True,
            )
        legacy_download_report = case_report["legacy_download"]
        if legacy_download_report is not None:
            print(
                f"case={case_report['name']} legacy_download_mismatches={len(legacy_download_report['mismatches'])}",
                flush=True,
            )
            for mismatch in legacy_download_report["mismatches"]:
                print(f"case={case_report['name']} legacy_download_mismatch={mismatch}", flush=True)
    print("mission_test_summary_end", flush=True)
    raise SystemExit(0 if report["passed"] else 1)


if __name__ == "__main__":
    main()

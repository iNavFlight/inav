#!/usr/bin/env python3
"""
Usage:
  conda run -n drone python tools/mavlink_sitl_sanity.py
  conda run -n drone python tools/mavlink_sitl_sanity.py --config tools/mavlink_sitl_sanity.yaml
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import queue
import re
import shutil
import signal
import socket
import subprocess
import sys
import threading
import time

from pymavlink import mavutil
import yaml

try:
    from mspapi2.msp_api import MSPApi
except ModuleNotFoundError:
    SCRIPT_DIR = Path(__file__).resolve().parent
    WORKSPACE_ROOT = SCRIPT_DIR.parents[3]
    MSPAPI2_REPO = WORKSPACE_ROOT / "mspapi2"
    if MSPAPI2_REPO.exists():
        sys.path.insert(0, str(MSPAPI2_REPO))
    from mspapi2.msp_api import MSPApi


SCRIPT_DIR = Path(__file__).resolve().parent
WORKSPACE_ROOT = SCRIPT_DIR.parents[3]
DEFAULT_CONFIG_PATH = SCRIPT_DIR / "mavlink_sitl_sanity.yaml"
AUTOPILOT_INVALID = int(mavutil.mavlink.MAV_AUTOPILOT_INVALID)
MAV_TYPE_GCS = int(mavutil.mavlink.MAV_TYPE_GCS)


class ProcessOutput:
    def __init__(self, process: subprocess.Popen[str], log_path: Path):
        self.process = process
        self.log_path = log_path
        self.lines: list[str] = []
        self.events: queue.Queue[str] = queue.Queue()
        self.log_file = log_path.open("w", encoding="utf-8")
        self.thread = threading.Thread(target=self._reader, daemon=True)
        self.thread.start()

    def _reader(self) -> None:
        try:
            for line in self.process.stdout:
                self.lines.append(line)
                self.log_file.write(line)
                self.log_file.flush()
                self.events.put(line)
        finally:
            self.log_file.flush()

    def mark(self) -> int:
        return len(self.lines)

    def wait_for(self, pattern: str, timeout_s: float, start_index: int = 0) -> str:
        compiled = re.compile(pattern)
        deadline = time.monotonic() + timeout_s
        while True:
            for line in self.lines[start_index:]:
                if compiled.search(line):
                    return line.strip()
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break
            try:
                line = self.events.get(timeout=min(remaining, 0.5))
                if compiled.search(line):
                    return line.strip()
            except queue.Empty:
                continue
        recent = "".join(self.lines[max(0, len(self.lines) - 20):]).strip()
        raise TimeoutError(f'Pattern "{pattern}" not found in {self.log_path}: {recent}')

    def close(self) -> None:
        self.thread.join(timeout=1.0)
        self.log_file.close()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a small SITL sanity check across MSP, MAVLink RC, and MAVLink GCS links.")
    parser.add_argument("--config", default=str(DEFAULT_CONFIG_PATH), help="YAML config path")
    return parser.parse_args()


def load_config(config_path: Path) -> dict:
    return yaml.safe_load(config_path.read_text(encoding="utf-8"))


def relpath(path: Path) -> str:
    return path.resolve().relative_to(WORKSPACE_ROOT.resolve()).as_posix()


def wait_for_tcp_port(host: str, port: int, timeout_s: float) -> None:
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1.0):
                return
        except (ConnectionRefusedError, socket.timeout, OSError):
            time.sleep(0.2)
    raise TimeoutError(f"TCP port {host}:{port} did not become available within {timeout_s:.1f}s")


def wait_for_tcp_port_cycle(host: str, port: int, timeout_s: float) -> None:
    deadline = time.monotonic() + timeout_s
    saw_closed = False
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1.0):
                if saw_closed:
                    return
        except (ConnectionRefusedError, socket.timeout, OSError):
            saw_closed = True
        time.sleep(0.2)
    if saw_closed:
        raise TimeoutError(f"TCP port {host}:{port} did not return after reboot within {timeout_s:.1f}s")
    wait_for_tcp_port(host, port, timeout_s)


def wait_for_configured_ports(config: dict) -> None:
    tests = config["tests"]
    ports = config["ports"]
    time.sleep(1.0)
    wait_for_tcp_port("127.0.0.1", int(ports["msp"]), float(tests["save_reboot_timeout_s"]))
    wait_for_tcp_port("127.0.0.1", int(ports["rc"]), float(tests["save_reboot_timeout_s"]))
    wait_for_tcp_port("127.0.0.1", int(ports["gcs"]), float(tests["save_reboot_timeout_s"]))


def cli_read_until_prompt(cli_socket: socket.socket) -> str:
    data = b""
    while b"\n# " not in data:
        chunk = cli_socket.recv(65536)
        if not chunk:
            raise ConnectionError("CLI socket closed before prompt")
        data += chunk
    return data.decode("utf-8", errors="replace")


def run_cli_commands(host: str, port: int, commands: list[str]) -> bool:
    sent_save = False
    with socket.create_connection((host, port), timeout=5.0) as cli_socket:
        cli_socket.settimeout(3.0)
        cli_socket.sendall(b"#\n")
        cli_read_until_prompt(cli_socket)
        for command in commands:
            cli_socket.sendall(command.encode("utf-8") + b"\n")
            if command == "save":
                sent_save = True
                break
            cli_read_until_prompt(cli_socket)
    return sent_save


def build_serial_command(index: int, function_mask: int, baud: int) -> str:
    return f"serial {index} {function_mask} {baud} {baud} 0 {baud}"


def load_cli_batch_commands(path: Path) -> list[str]:
    commands: list[str] = []
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if line in ("batch start", "batch end"):
            continue
        commands.append(line)
    return commands


def build_cli_config_commands(config: dict) -> list[str]:
    cli_cfg = config["cli"]
    if "diff_path" in cli_cfg:
        return load_cli_batch_commands(WORKSPACE_ROOT / cli_cfg["diff_path"])
    rc_baud = int(cli_cfg["rc_baud"])
    telemetry_baud = int(cli_cfg["telemetry_baud"])
    commands = [
        "feature TELEMETRY",
        "set receiver_type = SERIAL",
        "set serialrx_provider = MAVLINK",
        "set mavlink_version = 2",
        "set mavlink_port1_high_latency = OFF",
        "set mavlink_port2_high_latency = OFF",
        "set mavlink_port3_high_latency = OFF",
        "set mavlink_port4_high_latency = OFF",
        build_serial_command(0, 1, 115200),
        build_serial_command(1, 320, rc_baud),
        build_serial_command(2, 256, telemetry_baud),
        build_serial_command(3, 0, telemetry_baud),
        build_serial_command(4, 0, telemetry_baud),
    ]
    commands.extend(cli_cfg["mode_ranges"])
    commands.append("save")
    return commands


def start_process(command: list[str], cwd: Path, log_path: Path) -> tuple[subprocess.Popen[str], ProcessOutput]:
    log_path.parent.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.pop("LD_LIBRARY_PATH", None)
    env["PYTHONUNBUFFERED"] = "1"
    process = subprocess.Popen(
        command,
        cwd=str(cwd),
        env=env,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
        start_new_session=True,
    )
    if process.stdout is None:
        raise RuntimeError("stdout pipe was not created")
    return process, ProcessOutput(process, log_path)


def stop_process(process: subprocess.Popen[str] | None) -> None:
    if process is None or process.poll() is not None:
        return
    os.killpg(process.pid, signal.SIGINT)
    try:
        process.wait(timeout=10.0)
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGTERM)
        try:
            process.wait(timeout=5.0)
        except subprocess.TimeoutExpired:
            os.killpg(process.pid, signal.SIGKILL)
            process.wait(timeout=5.0)


def open_mavlink_tcp_connection(port: int, source_system: int, source_component: int, timeout_s: float):
    deadline = time.monotonic() + timeout_s
    while True:
        try:
            return mavutil.mavlink_connection(
                f"tcp:127.0.0.1:{port}",
                source_system=source_system,
                source_component=source_component,
                autoreconnect=True,
            )
        except OSError:
            if time.monotonic() >= deadline:
                raise
            time.sleep(0.2)


def open_mavlink_udp_listener(port: int, source_system: int, source_component: int):
    return mavutil.mavlink_connection(
        f"udpin:127.0.0.1:{port}",
        source_system=source_system,
        source_component=source_component,
        autoreconnect=True,
    )


def is_fc_heartbeat(message) -> bool:
    return int(message.autopilot) != AUTOPILOT_INVALID and int(message.type) != MAV_TYPE_GCS


def wait_for_fc_heartbeat(master, timeout_s: float):
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        message = master.recv_match(type="HEARTBEAT", blocking=True, timeout=min(1.0, deadline - time.monotonic()))
        if message is None:
            continue
        if is_fc_heartbeat(message):
            return message
    raise TimeoutError(f"No FC heartbeat received within {timeout_s:.1f}s")


def wait_for_message(master, message_type: str, timeout_s: float):
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        message = master.recv_match(blocking=True, timeout=min(1.0, deadline - time.monotonic()))
        if message is None:
            continue
        if message.get_type() == message_type:
            return message
    raise TimeoutError(f"No {message_type} received within {timeout_s:.1f}s")


def wait_for_command_ack(master, command: int, timeout_s: float):
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        message = master.recv_match(blocking=True, timeout=min(1.0, deadline - time.monotonic()))
        if message is None:
            continue
        if message.get_type() == "COMMAND_ACK" and int(message.command) == command:
            return message
    raise TimeoutError(f"No COMMAND_ACK for {command} within {timeout_s:.1f}s")


def wait_for_fc_mode(master, custom_mode: int, armed: bool | None, timeout_s: float):
    deadline = time.monotonic() + timeout_s
    observed_modes: list[str] = []
    while time.monotonic() < deadline:
        heartbeat = master.recv_match(type="HEARTBEAT", blocking=True, timeout=min(1.0, deadline - time.monotonic()))
        if heartbeat is None or not is_fc_heartbeat(heartbeat):
            continue
        is_armed = (int(heartbeat.base_mode) & int(mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED)) != 0
        observed_modes.append(f"{int(heartbeat.custom_mode)}:{int(is_armed)}")
        if len(observed_modes) > 8:
            observed_modes.pop(0)
        if int(heartbeat.custom_mode) != custom_mode:
            continue
        if armed is None:
            return heartbeat
        if is_armed == armed:
            return heartbeat
    raise TimeoutError(
        f"No FC heartbeat for custom_mode={custom_mode} armed={armed} within {timeout_s:.1f}s "
        f"observed={observed_modes}"
    )


def wait_for_fc_armed_state(master, armed: bool, timeout_s: float):
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        heartbeat = master.recv_match(type="HEARTBEAT", blocking=True, timeout=min(1.0, deadline - time.monotonic()))
        if heartbeat is None or not is_fc_heartbeat(heartbeat):
            continue
        is_armed = (int(heartbeat.base_mode) & int(mavutil.mavlink.MAV_MODE_FLAG_SAFETY_ARMED)) != 0
        if is_armed == armed:
            return heartbeat
    raise TimeoutError(f"No FC heartbeat for armed={armed} within {timeout_s:.1f}s")


def measure_message_rate(master, message_type: str, window_s: float) -> float:
    deadline = time.monotonic() + window_s
    count = 0
    while time.monotonic() < deadline:
        message = master.recv_match(blocking=True, timeout=min(0.5, deadline - time.monotonic()))
        if message is None:
            continue
        if message.get_type() == message_type:
            count += 1
    return count / window_s


def drain_messages(master, drain_s: float) -> None:
    deadline = time.monotonic() + drain_s
    while time.monotonic() < deadline:
        timeout_s = deadline - time.monotonic()
        if timeout_s <= 0:
            return
        message = master.recv_match(blocking=True, timeout=min(0.1, timeout_s))
        if message is None:
            continue


def mavproxy_send(process: subprocess.Popen[str], command: str) -> None:
    if process.stdin is None:
        raise RuntimeError("MAVProxy stdin pipe was not created")
    process.stdin.write(command + "\n")
    process.stdin.flush()


def readback_waypoint_count(path: Path) -> int:
    lines = [line for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]
    if not lines or lines[0] != "QGC WPL 110":
        raise ValueError(f"Unexpected waypoint file header in {path}")
    return len(lines) - 1


def msp_check(config: dict) -> str:
    tests = config["tests"]
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        status = api.get_inav_status()
        attitude = api.get_attitude()
        rx_config = api.get_rx_config()
    receiver_type = rx_config["receiverType"].name
    serial_provider = rx_config["serialRxProvider"].name
    return (
        f"cycleTime={status['cycleTime']} cpuLoad={status['cpuLoad']} "
        f"roll={attitude['roll']:.1f} pitch={attitude['pitch']:.1f} "
        f"receiverType={receiver_type} serialRxProvider={serial_provider}"
    )


def msp_land_command_check(config: dict) -> str:
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        status = api.get_inav_status()
        try:
            api.set_land()
            result = "ack=ok"
        except Exception as exc:
            detail = str(exc)
            if "unsupported (! response)" not in detail:
                raise
            result = f"expected_unarmed_error={detail}"
    active_modes = [mode.name for mode in status.get("activeModes", [])]
    return f"{result} activeModes={active_modes}"


def msp_rth_command_check(config: dict) -> str:
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        status = api.get_inav_status()
        try:
            api.set_rth()
            result = "ack=ok"
        except Exception as exc:
            detail = str(exc)
            if "unsupported (! response)" not in detail:
                raise
            result = f"expected_idle_error={detail}"
    active_modes = [mode.name for mode in status.get("activeModes", [])]
    return f"{result} activeModes={active_modes}"


def msp_set_home_check(config: dict) -> str:
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        status = api.get_inav_status()
        try:
            api.set_home(
                latitude_deg=37.5001,
                longitude_deg=-122.2499,
                altitude_m=12.34,
                altitude_datum=0,
            )
            result = "ack=ok"
        except Exception as exc:
            detail = str(exc)
            if "unsupported (! response)" not in detail:
                raise
            result = f"expected_gate_error={detail}"
    active_modes = [mode.name for mode in status.get("activeModes", [])]
    return f"{result} activeModes={active_modes}"


def msp_arm_disarm_check(config: dict) -> str:
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        api.set_arm_state(False)
        status = api.get_inav_status()
        try:
            api.set_arm_state(True)
            arm_result = "arm_ack=ok"
        except Exception as exc:
            detail = str(exc)
            if "unsupported (! response)" not in detail:
                raise
            arm_result = f"expected_arm_error={detail}"
    arming_flags = [flag.name for flag in status["armingFlags"]]
    active_modes = [mode.name for mode in status.get("activeModes", [])]
    return f"disarm_ack=ok {arm_result} armingFlags={arming_flags} activeModes={active_modes}"


def msp_nav_roi_check(config: dict) -> str:
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        api.set_nav_roi(
            latitude_deg=37.5001,
            longitude_deg=-122.2499,
            altitude_m=12.34,
            p1=123,
            p2=-45,
            alt_datum=0,
            action=7,
            flag=1,
        )
        roi = api.get_nav_roi()
        api.set_nav_roi(
            latitude_deg=0.0,
            longitude_deg=0.0,
            altitude_m=0.0,
            p1=0,
            p2=0,
            alt_datum=0,
            action=0,
            flag=0,
        )
        cleared = api.get_nav_roi()
    if abs(roi["latitude_deg"] - 37.5001) > 1e-7 or abs(roi["longitude_deg"] + 122.2499) > 1e-7:
        raise RuntimeError(f"ROI readback mismatch lat={roi['latitude_deg']} lon={roi['longitude_deg']}")
    if abs(roi["altitude_m"] - 12.34) > 0.01 or roi["p1"] != 123 or roi["p2"] != -45:
        raise RuntimeError(f"ROI payload mismatch altitude_m={roi['altitude_m']} p1={roi['p1']} p2={roi['p2']}")
    if roi["alt_datum"] != 0 or roi["action"] != 7 or roi["flag"] != 1:
        raise RuntimeError(f"ROI metadata mismatch alt_datum={roi['alt_datum']} action={roi['action']} flag={roi['flag']}")
    if (
        cleared["latitude_deg"] != 0.0
        or cleared["longitude_deg"] != 0.0
        or cleared["altitude_m"] != 0.0
        or cleared["p1"] != 0
        or cleared["p2"] != 0
        or cleared["alt_datum"] != 0
        or cleared["action"] != 0
        or cleared["flag"] != 0
    ):
        raise RuntimeError(f"ROI clear mismatch cleared={cleared}")
    return (
        f"lat={roi['latitude_deg']:.7f} lon={roi['longitude_deg']:.7f} "
        f"altitude_m={roi['altitude_m']:.2f} action={roi['action']} cleared_flag={cleared['flag']}"
    )


def read_arming_status(config: dict) -> str:
    ports = config["ports"]
    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(ports['msp'])}") as api:
        status = api.get_inav_status()
        nav_status = api.get_nav_status()
    arming_flags = [flag.name for flag in status["armingFlags"]]
    if "activeModes" in nav_status:
        active_modes = [mode.name for mode in nav_status["activeModes"]]
    else:
        active_modes = [mode.name for mode in status.get("activeModes", [])]
    return f"armingFlags={arming_flags} activeModes={active_modes}"


def receiver_telemetry_check(config: dict) -> str:
    tests = config["tests"]
    ports = config["ports"]
    master = open_mavlink_tcp_connection(int(ports["rc"]), 241, 191, float(tests["port_ready_timeout_s"]))
    try:
        heartbeat = wait_for_fc_heartbeat(master, float(tests["heartbeat_timeout_s"]))
        target_system = heartbeat.get_srcSystem()
        target_component = heartbeat.get_srcComponent()
        master.mav.request_data_stream_send(
            target_system,
            target_component,
            mavutil.mavlink.MAV_DATA_STREAM_ALL,
            int(tests["receiver_request_rate_hz"]),
            1,
        )
        wait_for_message(master, "ATTITUDE", float(tests["telemetry_timeout_s"]))
        return f"heartbeat_sysid={target_system} heartbeat_compid={target_component} attitude=seen"
    finally:
        master.close()


def run_rc_script(config: dict) -> str:
    tests = config["tests"]
    paths = config["paths"]
    rc_script = WORKSPACE_ROOT / paths["rc_script"]
    command = build_rc_script_command(config, float(tests["rc_duration_s"]), "MID")
    env = os.environ.copy()
    env.pop("LD_LIBRARY_PATH", None)
    completed = subprocess.run(command, cwd=str(WORKSPACE_ROOT), env=env, capture_output=True, text=True, check=True)
    summary_lines = [line for line in completed.stdout.splitlines() if line.startswith("summary ")]
    if not summary_lines:
        raise RuntimeError(f"No summary line from {relpath(rc_script)}:\n{completed.stdout}")
    summary = summary_lines[-1]
    match = re.search(r"rx=(\d+).*mismatch_count=(\d+).*rx_hz=([0-9.]+)", summary)
    if match is None:
        raise ValueError(f"Unexpected RC summary format: {summary}")
    rx_count = int(match.group(1))
    mismatch_count = int(match.group(2))
    rx_hz = float(match.group(3))
    if rx_count < int(tests["rc_min_rx_count"]):
        raise RuntimeError(f"RC rx count too low: {rx_count}")
    if mismatch_count != 0:
        raise RuntimeError(f"RC mismatch count is not zero: {mismatch_count}")
    return f"rx={rx_count} mismatch_count={mismatch_count} rx_hz={rx_hz:.2f}"


def build_rc_script_command(config: dict, duration_s: float, throttle_level: str) -> list[str]:
    paths = config["paths"]
    rc_script = WORKSPACE_ROOT / paths["rc_script"]
    return [
        sys.executable,
        str(rc_script),
        "--master",
        f"tcp:127.0.0.1:{int(config['ports']['rc'])}",
        "--duration",
        str(duration_s),
        "--tx-hz",
        str(float(config["tests"]["rc_tx_hz"])),
        "--roll",
        "MID",
        "--pitch",
        "MID",
        "--yaw",
        "MID",
        "--throttle",
        throttle_level,
    ]


def start_rc_stream(config: dict, duration_s: float) -> tuple[subprocess.Popen[str], ProcessOutput]:
    paths = config["paths"]
    return start_process(
        build_rc_script_command(config, duration_s, "LOW"),
        WORKSPACE_ROOT,
        WORKSPACE_ROOT / paths["control_plane_rc_log"],
    )


def start_mavproxy(config: dict) -> tuple[subprocess.Popen[str], ProcessOutput]:
    paths = config["paths"]
    ports = config["ports"]
    tests = config["tests"]
    mavproxy = shutil.which("mavproxy.py")
    if mavproxy is None:
        raise FileNotFoundError("mavproxy.py not found in PATH")
    state_dir = WORKSPACE_ROOT / paths["mavproxy_state_dir"]
    state_dir.mkdir(parents=True, exist_ok=True)
    command = [
        mavproxy,
        f"--master=tcp:127.0.0.1:{int(ports['gcs'])}",
        f"--out=udp:127.0.0.1:{int(ports['mavproxy_out'])}",
        "--streamrate=-1",
        "--state-basedir",
        str(state_dir),
        "--aircraft",
        "mavlink_sitl_sanity",
        "--target-system",
        str(int(tests["target_system"])),
        "--target-component",
        str(int(tests["target_component"])),
    ]
    return start_process(command, WORKSPACE_ROOT, WORKSPACE_ROOT / paths["mavproxy_log"])


def gcs_heartbeat_check(config: dict, observer) -> str:
    tests = config["tests"]
    heartbeat = wait_for_fc_heartbeat(observer, float(tests["heartbeat_timeout_s"]))
    return f"heartbeat_sysid={heartbeat.get_srcSystem()} heartbeat_compid={heartbeat.get_srcComponent()}"


def send_gcs_heartbeat(master) -> None:
    master.mav.heartbeat_send(
        mavutil.mavlink.MAV_TYPE_GCS,
        mavutil.mavlink.MAV_AUTOPILOT_INVALID,
        0,
        0,
        mavutil.mavlink.MAV_STATE_ACTIVE,
    )


def control_plane_check(config: dict) -> str:
    tests = config["tests"]
    ports = config["ports"]
    control_cfg = config["control_plane"]
    rc_process = None
    rc_output = None
    master = open_mavlink_tcp_connection(
        int(ports["gcs"]),
        int(control_cfg["source_system"]),
        int(control_cfg["source_component"]),
        float(tests["port_ready_timeout_s"]),
    )
    try:
        rc_process, rc_output = start_rc_stream(config, float(control_cfg["rc_hold_duration_s"]))
        time.sleep(float(control_cfg["rc_warmup_s"]))
        heartbeat = wait_for_fc_heartbeat(master, float(control_cfg["heartbeat_timeout_s"]))
        target_system = heartbeat.get_srcSystem()
        target_component = heartbeat.get_srcComponent()
        send_gcs_heartbeat(master)
        drain_messages(master, 0.5)

        requester_ts = time.time_ns()
        master.mav.timesync_send(0, requester_ts)
        timesync = wait_for_message(master, "TIMESYNC", float(control_cfg["timesync_timeout_s"]))
        if int(timesync.ts1) != requester_ts:
            raise RuntimeError(f"TIMESYNC ts1 mismatch: expected {requester_ts} got {int(timesync.ts1)}")
        if int(timesync.tc1) <= 0:
            raise RuntimeError(f"TIMESYNC tc1 not populated: {int(timesync.tc1)}")

        send_gcs_heartbeat(master)
        master.mav.set_mode_send(
            target_system,
            int(mavutil.mavlink.MAV_MODE_FLAG_CUSTOM_MODE_ENABLED),
            int(control_cfg["fbwa_custom_mode"]),
        )
        try:
            wait_for_fc_mode(master, int(control_cfg["fbwa_custom_mode"]), False, float(control_cfg["mode_timeout_s"]))
            mode_probe = f"fbwa_mode={int(control_cfg['fbwa_custom_mode'])}"
        except TimeoutError as exc:
            mode_probe = f"fbwa_headless_masked={exc}"

        send_gcs_heartbeat(master)
        master.mav.set_mode_send(
            target_system,
            int(mavutil.mavlink.MAV_MODE_FLAG_CUSTOM_MODE_ENABLED),
            int(control_cfg["manual_custom_mode"]),
        )
        wait_for_fc_mode(master, int(control_cfg["manual_custom_mode"]), False, float(control_cfg["mode_timeout_s"]))

        send_gcs_heartbeat(master)
        master.mav.command_long_send(
            target_system,
            target_component,
            mavutil.mavlink.MAV_CMD_COMPONENT_ARM_DISARM,
            0,
            1.0,
            0,
            0,
            0,
            0,
            0,
            0,
        )
        arm_ack = wait_for_command_ack(master, int(mavutil.mavlink.MAV_CMD_COMPONENT_ARM_DISARM), float(control_cfg["ack_timeout_s"]))
        if int(arm_ack.result) != int(mavutil.mavlink.MAV_RESULT_DENIED):
            raise RuntimeError(f"Headless arm expected DENIED but got result {int(arm_ack.result)}")
        arming_status = read_arming_status(config)
        if "ARMING_DISABLED_SENSORS_CALIBRATING" not in arming_status or \
            "ARMING_DISABLED_NAVIGATION_UNSAFE" not in arming_status or \
            "ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED" not in arming_status:
            raise RuntimeError(f"Headless arm deny did not expose expected blockers: {arming_status}")

        return (
            f"timesync_tc1={int(timesync.tc1)} {mode_probe} "
            f"manual_mode={int(control_cfg['manual_custom_mode'])} arm_result={int(arm_ack.result)} "
            f"{arming_status}"
        )
    finally:
        master.close()
        stop_process(rc_process)
        if rc_output is not None:
            rc_output.close()


def mission_upload_check(config: dict, mavproxy_process: subprocess.Popen[str], mavproxy_output: ProcessOutput) -> str:
    tests = config["tests"]
    paths = config["paths"]
    mission_path = WORKSPACE_ROOT / paths["mission_file"]
    start_index = mavproxy_output.mark()
    mavproxy_send(mavproxy_process, f"wp load {mission_path}")
    line = mavproxy_output.wait_for(rf"Loaded {int(tests['mission_waypoint_count'])} waypoints in ", float(tests["mission_timeout_s"]), start_index)
    return f"mission={relpath(mission_path)} {line}"


def mission_readback_check(config: dict, mavproxy_process: subprocess.Popen[str], mavproxy_output: ProcessOutput) -> str:
    tests = config["tests"]
    paths = config["paths"]
    readback_path = WORKSPACE_ROOT / paths["mission_readback_file"]
    if readback_path.exists():
        readback_path.unlink()
    start_index = mavproxy_output.mark()
    mavproxy_send(mavproxy_process, f"wp save {readback_path}")
    mavproxy_output.wait_for(rf"Saved {int(tests['mission_waypoint_count'])} waypoints to ", float(tests["mission_timeout_s"]), start_index)
    count = readback_waypoint_count(readback_path)
    if count != int(tests["mission_waypoint_count"]):
        raise RuntimeError(f"Readback waypoint count mismatch: expected {tests['mission_waypoint_count']} got {count}")
    return f"readback={relpath(readback_path)} waypoint_count={count}"


def stream_rate_check(config: dict, observer, mavproxy_process: subprocess.Popen[str]) -> str:
    tests = config["tests"]
    mavproxy_send(mavproxy_process, f"set streamrate {int(tests['streamrate_hz'])}")
    wait_for_message(observer, "ATTITUDE", float(tests["telemetry_timeout_s"]))
    drain_messages(observer, 0.5)
    rate_hz = measure_message_rate(observer, "ATTITUDE", float(tests["rate_measure_window_s"]))
    if rate_hz < float(tests["streamrate_min_hz"]):
        raise RuntimeError(f"ATTITUDE rate too low after streamrate set: {rate_hz:.2f}Hz")
    return f"attitude_rate_hz={rate_hz:.2f}"


def message_rate_check(config: dict, observer, mavproxy_process: subprocess.Popen[str], mavproxy_output: ProcessOutput) -> str:
    tests = config["tests"]
    target_rate_hz = float(tests["message_rate_hz"])
    start_index = mavproxy_output.mark()
    mavproxy_send(mavproxy_process, "module load messagerate")
    mavproxy_send(mavproxy_process, f"messagerate set ATTITUDE {target_rate_hz}")
    time.sleep(1.0)
    mavproxy_send(mavproxy_process, "messagerate get ATTITUDE")
    line = mavproxy_output.wait_for(r"Msg:ATTITUDE  rate:([0-9.]+)Hz", float(tests["telemetry_timeout_s"]), start_index)
    reported_match = re.search(r"rate:([0-9.]+)Hz", line)
    if reported_match is None:
        raise RuntimeError(f"Unexpected messagerate output: {line}")
    reported_rate_hz = float(reported_match.group(1))
    if abs(reported_rate_hz - target_rate_hz) > float(tests["message_rate_report_tolerance_hz"]):
        raise RuntimeError(f"Reported ATTITUDE rate {reported_rate_hz:.2f}Hz does not match target {target_rate_hz:.2f}Hz")
    drain_messages(observer, 0.5)
    observed_rate_hz = measure_message_rate(observer, "ATTITUDE", float(tests["rate_measure_window_s"]))
    if abs(observed_rate_hz - target_rate_hz) > float(tests["message_rate_observed_tolerance_hz"]):
        raise RuntimeError(f"Observed ATTITUDE rate {observed_rate_hz:.2f}Hz does not match target {target_rate_hz:.2f}Hz")
    return f"reported_rate_hz={reported_rate_hz:.2f} observed_rate_hz={observed_rate_hz:.2f}"


def print_report(results: list[tuple[str, bool, str]]) -> None:
    for name, ok, detail in results:
        status = "PASS" if ok else "FAIL"
        print(f"{name}: {status} {detail}", flush=True)
    overall = all(ok for _, ok, _ in results)
    print(f"OVERALL: {'PASS' if overall else 'FAIL'}", flush=True)


def main() -> int:
    args = parse_args()
    config_path = Path(args.config).resolve()
    config = load_config(config_path)
    sitl_process = None
    sitl_output = None
    mavproxy_process = None
    mavproxy_output = None
    observer = None
    results: list[tuple[str, bool, str]] = []

    def record(name: str, action) -> None:
        try:
            detail = action()
            results.append((name, True, detail))
        except Exception as exc:
            results.append((name, False, str(exc)))
            raise

    try:
        sitl_cfg = config["sitl"]
        tests = config["tests"]
        eeprom_path = WORKSPACE_ROOT / sitl_cfg["eeprom_path"]
        if eeprom_path.exists():
            eeprom_path.unlink()
        sitl_command = [str(WORKSPACE_ROOT / sitl_cfg["binary"]), "--path", str(WORKSPACE_ROOT / sitl_cfg["eeprom_path"])]
        sitl_process, sitl_output = start_process(sitl_command, WORKSPACE_ROOT / sitl_cfg["workdir"], WORKSPACE_ROOT / sitl_cfg["runtime_log"])
        wait_for_tcp_port("127.0.0.1", int(config["ports"]["msp"]), float(tests["port_ready_timeout_s"]))
        for _ in range(int(config["cli"]["apply_count"])):
            run_cli_commands("127.0.0.1", int(config["ports"]["msp"]), build_cli_config_commands(config))
            wait_for_configured_ports(config)
        wait_for_tcp_port("127.0.0.1", int(config["ports"]["rc"]), float(tests["port_ready_timeout_s"]))
        wait_for_tcp_port("127.0.0.1", int(config["ports"]["gcs"]), float(tests["port_ready_timeout_s"]))

        record("MSP_UART1", lambda: msp_check(config))
        record("MSP_ARM_DISARM_UART1", lambda: msp_arm_disarm_check(config))
        record("MSP_LAND_UART1", lambda: msp_land_command_check(config))
        record("MSP_RTH_UART1", lambda: msp_rth_command_check(config))
        record("MSP_SET_HOME_UART1", lambda: msp_set_home_check(config))
        record("MSP_NAV_ROI_UART1", lambda: msp_nav_roi_check(config))
        record("MAVLINK_TELEMETRY_UART2", lambda: receiver_telemetry_check(config))
        record("RC_QUICK_CHECK_UART2", lambda: run_rc_script(config))
        record("CONTROL_PLANE_UART3", lambda: control_plane_check(config))

        mavproxy_process, mavproxy_output = start_mavproxy(config)
        observer = open_mavlink_udp_listener(int(config["ports"]["mavproxy_out"]), 244, 191)
        record("MAVLINK_GCS_UART3", lambda: gcs_heartbeat_check(config, observer))
        record("MISSION_UPLOAD_UART3", lambda: mission_upload_check(config, mavproxy_process, mavproxy_output))
        record("MISSION_READBACK_UART3", lambda: mission_readback_check(config, mavproxy_process, mavproxy_output))
        record("STREAM_RATE_UART3", lambda: stream_rate_check(config, observer, mavproxy_process))
        record("MESSAGE_RATE_UART3", lambda: message_rate_check(config, observer, mavproxy_process, mavproxy_output))
    except Exception:
        exc = sys.exc_info()[1]
        if exc is not None and (not results or results[-1][1]):
            results.append(("UNHANDLED", False, str(exc)))
        print_report(results)
        return 1
    finally:
        if observer is not None:
            observer.close()
        stop_process(mavproxy_process)
        if mavproxy_output is not None:
            mavproxy_output.close()
        stop_process(sitl_process)
        if sitl_output is not None:
            sitl_output.close()

    print_report(results)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

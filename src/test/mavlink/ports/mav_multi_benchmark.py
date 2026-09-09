#!/usr/bin/env python3
"""
Usage:
  conda run -n drone python src/test/mavlink/ports/mav_multi_benchmark.py
  conda run -n drone python src/test/mavlink/ports/mav_multi_benchmark.py --config src/test/mavlink/ports/test_config.yaml
"""

from __future__ import annotations

import argparse
import asyncio
import socket
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Tuple

import yaml
try:
    from mavsdk import System
except ModuleNotFoundError:
    System = None
from pymavlink import mavutil
from serial.serialutil import SerialException

try:
    from mspapi2 import MSPApi, InavMSP
except ModuleNotFoundError:
    repo_root_guess = Path(__file__).resolve().parents[4]
    mspapi2_repo = repo_root_guess / "mspapi2"
    if mspapi2_repo.exists():
        import sys

        sys.path.insert(0, str(mspapi2_repo))
    from mspapi2 import MSPApi, InavMSP


DEFAULT_CONFIG_PATH = Path(__file__).resolve().parent / "test_config_multiport4_sweep_rc460800_tele115200.yaml"
CHANNELS = [1500, 1500, 900, 1500] + [900] * 14
MAV_CMD_REQUEST_MESSAGE = int(mavutil.mavlink.MAV_CMD_REQUEST_MESSAGE)
MAV_CMD_SET_MESSAGE_INTERVAL = int(mavutil.mavlink.MAV_CMD_SET_MESSAGE_INTERVAL)
MAVLINK_MSG_ID_HEARTBEAT = int(mavutil.mavlink.MAVLINK_MSG_ID_HEARTBEAT)
MAV_AUTOPILOT_INVALID = int(mavutil.mavlink.MAV_AUTOPILOT_INVALID)
MAV_TYPE_GCS = int(mavutil.mavlink.MAV_TYPE_GCS)


def load_config(config_path: Path) -> Dict[str, Any]:
    return yaml.safe_load(config_path.read_text(encoding="utf-8"))


def wait_for_tcp_port(host: str, port: int, timeout_s: float) -> None:
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1.0):
                return
        except ConnectionRefusedError:
            time.sleep(0.2)
            continue
        except socket.timeout:
            time.sleep(0.2)
            continue
    raise TimeoutError(f"TCP port {host}:{port} did not become available within {timeout_s}s")


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
        raise TimeoutError(f"TCP port {host}:{port} did not return after reboot within {timeout_s}s")

    # If no close window was observed, treat it as already rebooted and just ensure the port is reachable.
    wait_for_tcp_port(host, port, timeout_s)


def cli_read_until_prompt(cli_socket: socket.socket) -> str:
    data = b""
    while b"\n# " not in data:
        chunk = cli_socket.recv(65536)
        if not chunk:
            raise ConnectionError("CLI socket closed before prompt")
        data += chunk
    return data.decode("utf-8", errors="replace")


def wait_for_cli_ready(host: str, port: int, timeout_s: float) -> None:
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=1.0) as cli_socket:
                cli_socket.settimeout(1.0)
                cli_socket.sendall(b"#\n")
                cli_read_until_prompt(cli_socket)
                return
        except (ConnectionRefusedError, ConnectionError, socket.timeout, OSError):
            time.sleep(0.2)
    raise TimeoutError(f"CLI prompt did not become available on {host}:{port} within {timeout_s}s")


def run_cli_commands(host: str, port: int, commands: List[str], reboot_timeout_s: float) -> bool:
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
    if sent_save:
        # Save triggers reboot; do not send '#' again because that re-enters CLI mode.
        time.sleep(min(reboot_timeout_s, 1.0))
    return sent_save


def build_serial_command(index: int, function_mask: int, baud: int) -> str:
    return f"serial {index} {function_mask} {baud} {baud} 0 {baud}"


def build_cli_config_commands(config: Dict[str, Any], mavlink_port_count: int) -> List[str]:
    cli_cfg = config["cli"]
    rc_baud = int(cli_cfg["rc_baud"])
    telemetry_baud = int(cli_cfg["telemetry_baud"])
    uart2_mask = 320  # FUNCTION_RX_SERIAL + FUNCTION_TELEMETRY_MAVLINK
    uart3_mask = 256 if mavlink_port_count >= 2 else 0  # FUNCTION_TELEMETRY_MAVLINK
    uart4_mask = 256 if mavlink_port_count >= 3 else 0  # FUNCTION_TELEMETRY_MAVLINK
    uart5_mask = 256 if mavlink_port_count >= 4 else 0  # FUNCTION_TELEMETRY_MAVLINK
    commands = [
        "feature TELEMETRY",
        "set receiver_type = SERIAL",
        "set serialrx_provider = MAVLINK",
        build_serial_command(0, 1, 115200),  # MSP on UART1, always.
        build_serial_command(1, uart2_mask, rc_baud),
        build_serial_command(2, uart3_mask, telemetry_baud),
        build_serial_command(3, uart4_mask, telemetry_baud),
        build_serial_command(4, uart5_mask, telemetry_baud),
        "set mavlink_version = 2",
        "set mavlink_port1_high_latency = OFF",
        "set mavlink_port2_high_latency = OFF",
        "set mavlink_port3_high_latency = OFF",
        "set mavlink_port4_high_latency = OFF",
        "save",
    ]
    return commands


def apply_config_and_wait(config: Dict[str, Any], mavlink_port_count: int) -> None:
    ports = config["ports"]
    commands = build_cli_config_commands(config, mavlink_port_count)
    sent_save = run_cli_commands(
        "127.0.0.1",
        int(ports["msp"]),
        commands,
        reboot_timeout_s=float(config["tests"]["save_reboot_timeout_s"]),
    )
    if sent_save:
        wait_for_tcp_port_cycle("127.0.0.1", int(ports["msp"]), float(config["tests"]["save_reboot_timeout_s"]))
    wait_for_tcp_port("127.0.0.1", int(ports["msp"]), float(config["tests"]["port_ready_timeout_s"]))
    wait_for_tcp_port("127.0.0.1", int(ports["rc"]), float(config["tests"]["port_ready_timeout_s"]))
    if mavlink_port_count >= 2:
        wait_for_tcp_port("127.0.0.1", int(ports["telemetry"][0]), float(config["tests"]["port_ready_timeout_s"]))
    if mavlink_port_count >= 3:
        wait_for_tcp_port("127.0.0.1", int(ports["telemetry"][1]), float(config["tests"]["port_ready_timeout_s"]))
    if mavlink_port_count >= 4:
        wait_for_tcp_port("127.0.0.1", int(ports["telemetry"][2]), float(config["tests"]["port_ready_timeout_s"]))


async def mavsdk_probe(address: str, timeout_s: float) -> Dict[str, Any]:
    if System is None:
        raise RuntimeError("mavsdk is not installed")

    drone = System()
    result: Dict[str, Any] = {
        "address": address,
        "connected": False,
        "flight_mode": None,
    }
    await drone.connect(system_address=address)
    async with asyncio.timeout(timeout_s):
        async for state in drone.core.connection_state():
            if state.is_connected:
                result["connected"] = True
                break
    async with asyncio.timeout(timeout_s):
        async for flight_mode in drone.telemetry.flight_mode():
            result["flight_mode"] = str(flight_mode)
            break
    return result


def probe_mavsdk_sync(address: str, timeout_s: float) -> Dict[str, Any]:
    return asyncio.run(mavsdk_probe(address, timeout_s))


def open_mavlink_tcp_connection(port: int, source_system: int, source_component: int, timeout_s: float) -> Any:
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


def is_fc_heartbeat(msg: Any) -> bool:
    return int(msg.autopilot) != MAV_AUTOPILOT_INVALID and int(msg.type) != MAV_TYPE_GCS


def run_workload(
    config: Dict[str, Any],
    sitl_pid: int,
    telemetry_ports: List[int],
    rc_port: int,
    load_rates_hz: Dict[int, float],
    duration_s: float,
    rc_tx_hz_override: float | None = None,
) -> Dict[str, Any]:
    print(
        f"run_workload_start duration_s={duration_s} telemetry_ports={telemetry_ports} rc_port={rc_port} "
        f"load_rates_hz={load_rates_hz}",
        flush=True,
    )
    tests_cfg = config["tests"]
    rc_tx_hz = float(tests_cfg["rc_tx_hz"]) if rc_tx_hz_override is None else float(rc_tx_hz_override)
    msp_poll_hz = float(tests_cfg["msp_poll_hz"])
    inav_status_hz = float(tests_cfg.get("inav_status_hz", 1.0))
    msp_request_timeout_s = float(tests_cfg["msp_request_timeout_s"])
    command_message_id = int(tests_cfg["stress_command_message_id"])
    rc_target_system = int(tests_cfg["rc_target_system"])
    rc_target_component = int(tests_cfg["rc_target_component"])
    heartbeat_expected_hz = float(tests_cfg["heartbeat_expected_hz"])
    stream_cfg = config["cli"]
    port_ready_timeout_s = float(tests_cfg["port_ready_timeout_s"])
    warmup_s = float(tests_cfg.get("warmup_s", 3.0))
    warmup_heartbeat_count = int(tests_cfg.get("warmup_heartbeat_count", 3))
    warmup_timeout_s = float(tests_cfg.get("warmup_timeout_s", 30.0))

    listeners: Dict[int, Any] = {}
    targets: Dict[int, Tuple[int, int]] = {}
    port_stats: Dict[int, Dict[str, Any]] = {}
    load_senders: Dict[int, Any] = {}
    load_sent: Dict[int, int] = {}
    load_next_send: Dict[int, float] = {}
    msp_success = 0
    msp_fail = 0
    rc_sent = 0
    fc_cpu_load_samples: List[float] = []
    fc_cycle_time_samples: List[float] = []
    fc_status_fail = 0
    fc_status_last_error = ""

    source_base = 170
    ports_to_open = set(telemetry_ports)
    ports_to_open.update(load_rates_hz.keys())
    ports_to_open.add(rc_port)
    for port in sorted(ports_to_open):
        wait_for_tcp_port("127.0.0.1", int(port), port_ready_timeout_s)

    for idx, port in enumerate(telemetry_ports):
        conn = open_mavlink_tcp_connection(
            port=port,
            source_system=source_base + idx,
            source_component=190,
            timeout_s=port_ready_timeout_s,
        )
        listeners[port] = conn
        targets[port] = (rc_target_system, rc_target_component)
        port_stats[port] = {
            "heartbeat_count": 0,
            "rc_count": 0,
            "command_ack_total": 0,
            "command_ack_ok": 0,
            "mav_msg_count": 0,
            "mav_seq_lost": 0,
            "last_seq_by_source": {},
            "seq_track_source": None,
        }

    for idx, port in enumerate(sorted(load_rates_hz.keys())):
        if port in listeners:
            sender = listeners[port]
        else:
            sender = open_mavlink_tcp_connection(
                port=port,
                source_system=210 + idx,
                source_component=191,
                timeout_s=port_ready_timeout_s,
            )
        load_senders[port] = sender
        load_sent[port] = 0
        load_next_send[port] = time.monotonic()
        if port not in targets:
            targets[port] = (rc_target_system, rc_target_component)

    if rc_port in listeners:
        rc_sender = listeners[rc_port]
    else:
        rc_sender = open_mavlink_tcp_connection(
            port=rc_port,
            source_system=239,
            source_component=191,
            timeout_s=port_ready_timeout_s,
        )
    if telemetry_ports:
        telemetry_target = targets[telemetry_ports[0]]
        rc_target_system, rc_target_component = telemetry_target

    rc_period_s = 1.0 / rc_tx_hz
    next_rc_send_t = time.monotonic()
    heartbeat_period_s = 1.0
    next_heartbeat_send_t = next_rc_send_t

    warmup_hb_seen: Dict[int, int] = {port: 0 for port in telemetry_ports}
    warmup_start_t = time.monotonic()
    warmup_deadline_t = warmup_start_t + warmup_timeout_s

    while True:
        now = time.monotonic()

        if now >= next_heartbeat_send_t:
            heartbeat_conns: Dict[int, Any] = {}
            for conn in listeners.values():
                heartbeat_conns[id(conn)] = conn
            for conn in load_senders.values():
                heartbeat_conns[id(conn)] = conn
            heartbeat_conns[id(rc_sender)] = rc_sender
            for conn in heartbeat_conns.values():
                try:
                    conn.mav.heartbeat_send(
                        mavutil.mavlink.MAV_TYPE_GCS,
                        mavutil.mavlink.MAV_AUTOPILOT_INVALID,
                        0,
                        0,
                        0,
                    )
                except (ConnectionRefusedError, OSError):
                    continue
            next_heartbeat_send_t += heartbeat_period_s
            if next_heartbeat_send_t < now:
                next_heartbeat_send_t = now + heartbeat_period_s

        if now >= next_rc_send_t:
            rc_target = targets.get(rc_port)
            if rc_target is not None:
                rc_target_system, rc_target_component = rc_target
            try:
                rc_sender.mav.rc_channels_override_send(rc_target_system, rc_target_component, *CHANNELS[:8])
            except (ConnectionRefusedError, OSError):
                pass
            next_rc_send_t += rc_period_s
            if next_rc_send_t < now:
                next_rc_send_t = now + rc_period_s

        for port, conn in listeners.items():
            while True:
                try:
                    msg = conn.recv_match(blocking=False)
                except (ConnectionRefusedError, OSError):
                    break
                if msg is None:
                    break
                msg_type = msg.get_type()
                if msg_type == "HEARTBEAT":
                    src_system = int(msg.get_srcSystem())
                    src_component = int(msg.get_srcComponent())
                    if is_fc_heartbeat(msg):
                        targets[port] = (src_system, src_component)
                        warmup_hb_seen[port] += 1

        if telemetry_ports and all(count >= warmup_heartbeat_count for count in warmup_hb_seen.values()) and (now - warmup_start_t) >= warmup_s:
            break
        if (not telemetry_ports) and (now - warmup_start_t) >= warmup_s:
            break
        if now >= warmup_deadline_t:
            raise TimeoutError(
                f"Warmup failed: heartbeat_counts={warmup_hb_seen} required={warmup_heartbeat_count} timeout_s={warmup_timeout_s}"
            )

        time.sleep(0.001)

    for port, conn in listeners.items():
        target_system, target_component = targets[port]
        port_stats[port]["seq_track_source"] = (target_system, target_component)
        conn.mav.request_data_stream_send(
            target_system,
            target_component,
            int(mavutil.mavlink.MAV_DATA_STREAM_EXTENDED_STATUS),
            int(stream_cfg["ext_status_rate_hz"]),
            1 if int(stream_cfg["ext_status_rate_hz"]) > 0 else 0,
        )
        conn.mav.request_data_stream_send(
            target_system,
            target_component,
            int(mavutil.mavlink.MAV_DATA_STREAM_RC_CHANNELS),
            int(stream_cfg["rc_chan_rate_hz"]),
            1 if int(stream_cfg["rc_chan_rate_hz"]) > 0 else 0,
        )
        conn.mav.request_data_stream_send(
            target_system,
            target_component,
            int(mavutil.mavlink.MAV_DATA_STREAM_POSITION),
            int(stream_cfg["pos_rate_hz"]),
            1 if int(stream_cfg["pos_rate_hz"]) > 0 else 0,
        )
        conn.mav.request_data_stream_send(
            target_system,
            target_component,
            int(mavutil.mavlink.MAV_DATA_STREAM_EXTRA1),
            int(stream_cfg["extra1_rate_hz"]),
            1 if int(stream_cfg["extra1_rate_hz"]) > 0 else 0,
        )
        conn.mav.request_data_stream_send(
            target_system,
            target_component,
            int(mavutil.mavlink.MAV_DATA_STREAM_EXTRA2),
            int(stream_cfg["extra2_rate_hz"]),
            1 if int(stream_cfg["extra2_rate_hz"]) > 0 else 0,
        )
        conn.mav.request_data_stream_send(
            target_system,
            target_component,
            int(mavutil.mavlink.MAV_DATA_STREAM_EXTRA3),
            int(stream_cfg["extra3_rate_hz"]),
            1 if int(stream_cfg["extra3_rate_hz"]) > 0 else 0,
        )
        conn.mav.command_long_send(
            target_system,
            target_component,
            MAV_CMD_SET_MESSAGE_INTERVAL,
            0,
            float(MAVLINK_MSG_ID_HEARTBEAT),
            float(int(1_000_000.0 / heartbeat_expected_hz)),
            0,
            0,
            0,
            0,
            0,
        )
    time.sleep(0.1)

    for stats in port_stats.values():
        stats["heartbeat_count"] = 0
        stats["rc_count"] = 0
        stats["command_ack_total"] = 0
        stats["command_ack_ok"] = 0
        stats["mav_msg_count"] = 0
        stats["mav_seq_lost"] = 0
        stats["last_seq_by_source"] = {}

    rc_sent = 0
    measurement_start_t = time.monotonic()
    next_rc_send_t = measurement_start_t
    next_heartbeat_send_t = measurement_start_t
    for port in load_next_send:
        load_next_send[port] = measurement_start_t
    next_resource_sample_t = measurement_start_t + (1.0 / inav_status_hz)
    next_msp_poll_t = measurement_start_t + (1.0 / msp_poll_hz)
    workload_end_t = measurement_start_t + duration_s

    with MSPApi(tcp_endpoint=f"127.0.0.1:{int(config['ports']['msp'])}") as msp_api:
        msp_api._serial._max_retries = 1
        msp_api._serial._reconnect_delay = 0.05
        while time.monotonic() < workload_end_t:
            now = time.monotonic()

            if now >= next_heartbeat_send_t:
                heartbeat_conns: Dict[int, Any] = {}
                for conn in listeners.values():
                    heartbeat_conns[id(conn)] = conn
                for conn in load_senders.values():
                    heartbeat_conns[id(conn)] = conn
                heartbeat_conns[id(rc_sender)] = rc_sender
                for conn in heartbeat_conns.values():
                    try:
                        conn.mav.heartbeat_send(
                            mavutil.mavlink.MAV_TYPE_GCS,
                            mavutil.mavlink.MAV_AUTOPILOT_INVALID,
                            0,
                            0,
                            0,
                        )
                    except (ConnectionRefusedError, OSError):
                        continue
                next_heartbeat_send_t += heartbeat_period_s
                if next_heartbeat_send_t < now:
                    next_heartbeat_send_t = now + heartbeat_period_s

            if now >= next_rc_send_t:
                rc_target = targets.get(rc_port)
                if rc_target is not None:
                    rc_target_system, rc_target_component = rc_target
                try:
                    rc_sender.mav.rc_channels_override_send(rc_target_system, rc_target_component, *CHANNELS[:8])
                except (ConnectionRefusedError, OSError):
                    pass
                rc_sent += 1
                next_rc_send_t += rc_period_s
                if next_rc_send_t < now:
                    next_rc_send_t = now + rc_period_s

            for port, rate_hz in load_rates_hz.items():
                next_send_t = load_next_send[port]
                if now >= next_send_t:
                    target_system, target_component = targets[port]
                    try:
                        load_senders[port].mav.command_long_send(
                            target_system,
                            target_component,
                            MAV_CMD_REQUEST_MESSAGE,
                            0,
                            float(command_message_id),
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                        )
                    except (ConnectionRefusedError, OSError):
                        next_send_t = now + (1.0 / rate_hz)
                        load_next_send[port] = next_send_t
                        continue
                    load_sent[port] += 1
                    next_send_t += 1.0 / rate_hz
                    if next_send_t < now:
                        next_send_t = now + (1.0 / rate_hz)
                    load_next_send[port] = next_send_t

            for port, conn in listeners.items():
                while True:
                    try:
                        msg = conn.recv_match(blocking=False)
                    except (ConnectionRefusedError, OSError):
                        break
                    if msg is None:
                        break
                    msg_type = msg.get_type()
                    stats = port_stats[port]
                    seq = None
                    if hasattr(msg, "get_seq"):
                        try:
                            seq = int(msg.get_seq())
                        except (TypeError, ValueError):
                            seq = None
                    if seq is None:
                        header = getattr(msg, "_header", None)
                        if header is not None and hasattr(header, "seq"):
                            seq = int(header.seq)
                    if seq is not None:
                        src_key = (int(msg.get_srcSystem()), int(msg.get_srcComponent()))
                        if src_key == stats["seq_track_source"]:
                            last_seq = stats["last_seq_by_source"].get(src_key)
                            if last_seq is not None:
                                seq_delta = (seq - int(last_seq)) & 0xFF
                                if seq_delta > 0:
                                    stats["mav_seq_lost"] += max(seq_delta - 1, 0)
                            stats["last_seq_by_source"][src_key] = seq
                            stats["mav_msg_count"] += 1

                    if msg_type == "HEARTBEAT":
                        src_system = int(msg.get_srcSystem())
                        src_component = int(msg.get_srcComponent())
                        if is_fc_heartbeat(msg):
                            targets[port] = (src_system, src_component)
                        stats["heartbeat_count"] += 1
                    elif msg_type == "RC_CHANNELS":
                        stats["rc_count"] += 1
                    elif msg_type == "COMMAND_ACK":
                        if int(msg.command) == MAV_CMD_REQUEST_MESSAGE:
                            stats["command_ack_total"] += 1
                            if int(msg.result) == int(mavutil.mavlink.MAV_RESULT_ACCEPTED):
                                stats["command_ack_ok"] += 1

            if now >= next_msp_poll_t:
                try:
                    msp_api._request_raw(InavMSP.MSP_RC, timeout=msp_request_timeout_s)
                    msp_success += 1
                except (SerialException, RuntimeError, TimeoutError, ConnectionRefusedError, OSError):
                    msp_fail += 1
                next_msp_poll_t += 1.0 / msp_poll_hz

            if now >= next_resource_sample_t:
                try:
                    status = msp_api.get_inav_status()
                    fc_cpu_load_samples.append(float(status["cpuLoad"]))
                    fc_cycle_time_samples.append(float(status["cycleTime"]))
                except (SerialException, RuntimeError, TimeoutError, ConnectionRefusedError, OSError, KeyError, ValueError) as exc:
                    fc_status_fail += 1
                    fc_status_last_error = str(exc)
                next_resource_sample_t += 1.0 / inav_status_hz

            time.sleep(0.001)

    all_connections: Dict[int, Any] = {}
    for conn in listeners.values():
        all_connections[id(conn)] = conn
    for conn in load_senders.values():
        all_connections[id(conn)] = conn
    all_connections[id(rc_sender)] = rc_sender
    for conn in all_connections.values():
        conn.close()

    if not fc_cpu_load_samples:
        raise RuntimeError(f"No MSP2_INAV_STATUS samples collected; last_error={fc_status_last_error} fail_count={fc_status_fail}")

    summarized_ports: Dict[int, Dict[str, Any]] = {}
    for port in telemetry_ports:
        stats = port_stats[port]
        mav_total = int(stats["mav_msg_count"])
        mav_lost = int(stats["mav_seq_lost"])
        mav_expected = mav_total + mav_lost
        sent = load_sent.get(port, 0)
        ack_total = stats["command_ack_total"]
        ack_ok = stats["command_ack_ok"]
        ack_missing = max(sent - ack_total, 0)
        command_failed_total = max(sent - ack_ok, 0)
        summarized_ports[port] = {
            "heartbeat_count": stats["heartbeat_count"],
            "rc_count": stats["rc_count"],
            "mav_msg_count": mav_total,
            "mav_seq_lost": mav_lost,
            "mav_seq_loss_rate": (mav_lost / mav_expected) if mav_expected > 0 else 0.0,
            "command_sent": sent,
            "command_ack_total": ack_total,
            "command_ack_ok": ack_ok,
            "command_failed_total": command_failed_total,
            "command_ack_missing": ack_missing,
            "command_ack_failure_rate": (command_failed_total / sent) if sent > 0 else 0.0,
        }

    result = {
        "duration_s": duration_s,
        "ports": summarized_ports,
        "msp_success": msp_success,
        "msp_fail": msp_fail,
        "rc_sent": rc_sent,
        "rc_sent_hz": (rc_sent / max(duration_s, 1e-6)),
        "msp_failure_rate": (msp_fail / max(msp_success + msp_fail, 1)),
        "fc_cpu_load_avg_pct": (sum(fc_cpu_load_samples) / len(fc_cpu_load_samples)) if fc_cpu_load_samples else 0.0,
        "fc_cpu_load_max_pct": max(fc_cpu_load_samples) if fc_cpu_load_samples else 0.0,
        "fc_cycle_time_avg_us": (sum(fc_cycle_time_samples) / len(fc_cycle_time_samples)) if fc_cycle_time_samples else 0.0,
        "fc_cycle_time_max_us": max(fc_cycle_time_samples) if fc_cycle_time_samples else 0.0,
        "fc_status_samples": len(fc_cpu_load_samples),
    }
    print(
        f"run_workload_done duration_s={duration_s} fc_cpu_load_avg_pct={result['fc_cpu_load_avg_pct']:.2f} "
        f"msp_fail_rate={result['msp_failure_rate']:.4f}",
        flush=True,
    )
    return result


def format_port_table(port_result: Dict[int, Dict[str, Any]]) -> str:
    if not port_result:
        return "_No MAVLink telemetry ports active in this scenario._"

    lines = [
        "| Port | MAV Msg Count | MAV Seq Lost | MAV Seq Loss % | HB Count | RC_CHANNELS Count | Cmd Sent | Cmd Ack OK | Cmd Failed Total | Cmd Missing Ack |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
    ]
    for port in sorted(port_result.keys()):
        data = port_result[port]
        lines.append(
            "| "
            f"{port} | {data['mav_msg_count']} | {data['mav_seq_lost']} | {(data['mav_seq_loss_rate'] * 100.0):.2f}% | "
            f"{data['heartbeat_count']} | {data['rc_count']} | {data['command_sent']} | "
            f"{data['command_ack_ok']} | {data['command_failed_total']} | {data['command_ack_missing']} |"
        )
    return "\n".join(lines)


def summarize_command_failures(port_result: Dict[int, Dict[str, Any]]) -> Dict[str, float]:
    sent_total = sum(int(item["command_sent"]) for item in port_result.values())
    failed_total = sum(int(item["command_failed_total"]) for item in port_result.values())
    missing_ack_total = sum(int(item["command_ack_missing"]) for item in port_result.values())
    mav_seq_lost_total = sum(int(item["mav_seq_lost"]) for item in port_result.values())
    mav_msg_total = sum(int(item["mav_msg_count"]) for item in port_result.values())
    mav_seq_loss_pct = (mav_seq_lost_total / max(mav_seq_lost_total + mav_msg_total, 1)) * 100.0
    return {
        "sent_total": float(sent_total),
        "failed_total": float(failed_total),
        "failed_pct": (failed_total / max(sent_total, 1)) * 100.0,
        "missing_ack_total": float(missing_ack_total),
        "mav_seq_loss_pct": mav_seq_loss_pct,
    }


def write_testing_report(config: Dict[str, Any], report: Dict[str, Any]) -> None:
    out_path = Path(config["output"]["testing_md"])
    timestamp = datetime.now(timezone.utc).isoformat()
    lines: List[str] = []
    lines.append("# MAVLink Multiport Testing")
    lines.append("")
    lines.append(f"- Generated: `{timestamp}`")
    lines.append(f"- Conda env: `drone`")
    lines.append(f"- SITL binary: `{config['sitl']['binary']}`")
    lines.append(f"- EEPROM: `{config['sitl']['eeprom_path']}`")
    lines.append("- MSP kept on UART1 by design (`serial 0 ... functionMask=1`).")
    lines.append("- CLI under test sets `receiver_type=SERIAL` and `serialrx_provider=MAVLINK`.")
    lines.append("- Stream rates are configured at runtime via MAVLink `REQUEST_DATA_STREAM` and `MAV_CMD_SET_MESSAGE_INTERVAL`.")
    lines.append("- `RC_CHANNELS` metrics below are MAVLink telemetry stream metrics, not RX input ownership.")
    lines.append("")
    lines.append("## MAVSDK Probe")
    lines.append("")
    lines.append("| Address | Connected | Flight Mode |")
    lines.append("| --- | --- | --- |")
    if report["mavsdk_probes"]:
        for probe in report["mavsdk_probes"]:
            lines.append(f"| {probe['address']} | {probe['connected']} | {probe['flight_mode']} |")
    else:
        lines.append("| _skipped_ | _skipped_ | _skipped_ |")
    lines.append("")
    lines.append("## Port Count Baseline (1 RC, then +Telemetry ports)")
    lines.append("")
    for label, result in report["baseline"].items():
        cmd_summary = summarize_command_failures(result["ports"])
        lines.append(f"### {label}")
        lines.append("")
        lines.append(
            f"- Duration: `{result['duration_s']}s` | FC cpuLoad avg/max: `{result['fc_cpu_load_avg_pct']:.2f}% / {result['fc_cpu_load_max_pct']:.2f}%` | "
            f"FC cycleTime avg/max: `{result['fc_cycle_time_avg_us']:.1f} / {result['fc_cycle_time_max_us']:.1f}` us | "
            f"Status samples: `{result['fc_status_samples']}`"
        )
        lines.append(
            f"- MSP success/fail: `{result['msp_success']}/{result['msp_fail']}` | MSP failure rate: `{(result['msp_failure_rate'] * 100.0):.2f}%`"
        )
        lines.append(
            f"- Commands sent/failed: `{int(cmd_summary['sent_total'])}/{int(cmd_summary['failed_total'])}` (`{cmd_summary['failed_pct']:.2f}%`) | "
            f"Missing ACK: `{int(cmd_summary['missing_ack_total'])}` | MAVLink seq loss: `{cmd_summary['mav_seq_loss_pct']:.2f}%`"
        )
        lines.append(f"- RC override send rate: `{result['rc_sent_hz']:.2f} Hz` (`{result['rc_sent']}` packets)")
        lines.append("")
        lines.append(format_port_table(result["ports"]))
        lines.append("")
    lines.append("## Stress Test: Progressive Overload On One MAVLink Port")
    lines.append("")
    for item in report["single_port_stress"]:
        rate = item["rate_hz"]
        result = item["result"]
        cmd_summary = summarize_command_failures(result["ports"])
        lines.append(f"### Load Rate `{rate} req/s` on telemetry port")
        lines.append("")
        lines.append(
            f"- Duration: `{result['duration_s']}s` | FC cpuLoad avg/max: `{result['fc_cpu_load_avg_pct']:.2f}% / {result['fc_cpu_load_max_pct']:.2f}%` | "
            f"FC cycleTime avg/max: `{result['fc_cycle_time_avg_us']:.1f} / {result['fc_cycle_time_max_us']:.1f}` us | "
            f"Status samples: `{result['fc_status_samples']}`"
        )
        lines.append(
            f"- MSP success/fail: `{result['msp_success']}/{result['msp_fail']}` | MSP failure rate: `{(result['msp_failure_rate'] * 100.0):.2f}%`"
        )
        lines.append(
            f"- Commands sent/failed: `{int(cmd_summary['sent_total'])}/{int(cmd_summary['failed_total'])}` (`{cmd_summary['failed_pct']:.2f}%`) | "
            f"Missing ACK: `{int(cmd_summary['missing_ack_total'])}` | MAVLink seq loss: `{cmd_summary['mav_seq_loss_pct']:.2f}%`"
        )
        lines.append(f"- RC override send rate: `{result['rc_sent_hz']:.2f} Hz` (`{result['rc_sent']}` packets)")
        lines.append("")
        lines.append(format_port_table(result["ports"]))
        lines.append("")
    max_result = report["all_ports_max"]
    cmd_summary = summarize_command_failures(max_result["ports"])
    max_ports = len(max_result["ports"])
    lines.append(f"## Stress Test: All {max_ports} MAVLink Ports At Max Load")
    lines.append("")
    lines.append(
        f"- Duration: `{max_result['duration_s']}s` | FC cpuLoad avg/max: `{max_result['fc_cpu_load_avg_pct']:.2f}% / {max_result['fc_cpu_load_max_pct']:.2f}%` | "
        f"FC cycleTime avg/max: `{max_result['fc_cycle_time_avg_us']:.1f} / {max_result['fc_cycle_time_max_us']:.1f}` us | "
        f"Status samples: `{max_result['fc_status_samples']}`"
    )
    lines.append(
        f"- MSP success/fail: `{max_result['msp_success']}/{max_result['msp_fail']}` | MSP failure rate: `{(max_result['msp_failure_rate'] * 100.0):.2f}%`"
    )
    lines.append(
        f"- Commands sent/failed: `{int(cmd_summary['sent_total'])}/{int(cmd_summary['failed_total'])}` (`{cmd_summary['failed_pct']:.2f}%`) | "
        f"Missing ACK: `{int(cmd_summary['missing_ack_total'])}` | MAVLink seq loss: `{cmd_summary['mav_seq_loss_pct']:.2f}%`"
    )
    lines.append(f"- RC override send rate: `{max_result['rc_sent_hz']:.2f} Hz` (`{max_result['rc_sent']}` packets)")
    lines.append("")
    lines.append(format_port_table(max_result["ports"]))
    lines.append("")
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run MAVLink multiport RC/telemetry/stress benchmark and write TESTING.md")
    parser.add_argument("--config", default=str(DEFAULT_CONFIG_PATH), help="YAML configuration path")
    args = parser.parse_args()

    config_path = Path(args.config)
    config = load_config(config_path)

    sitl_bin = Path(config["sitl"]["binary"])
    sitl_workdir = Path(config["sitl"]["workdir"])
    sitl_eeprom = str(config["sitl"]["eeprom_path"])
    sitl_log = Path(config["sitl"]["runtime_log"])
    sitl_log.parent.mkdir(parents=True, exist_ok=True)
    log_handle = sitl_log.open("w", encoding="utf-8")
    sitl_proc = subprocess.Popen(
        [str(sitl_bin), f"--path={sitl_eeprom}"],
        cwd=str(sitl_workdir),
        stdout=log_handle,
        stderr=subprocess.STDOUT,
        text=True,
    )

    report: Dict[str, Any] = {
        "mavsdk_probes": [],
        "baseline": {},
        "single_port_stress": [],
        "all_ports_max": {},
    }

    try:
        wait_for_tcp_port("127.0.0.1", int(config["ports"]["msp"]), float(config["tests"]["port_ready_timeout_s"]))

        configured_port_count = 1 + len(config["ports"]["telemetry"])
        print(f"single_config_start count={configured_port_count}", flush=True)
        apply_config_and_wait(config, configured_port_count)
        print(f"single_config_done count={configured_port_count}", flush=True)

        baseline_scenarios: List[Tuple[str, List[int]]] = []
        rc_port = int(config["ports"]["rc"])
        telemetry_ports = [int(port) for port in config["ports"]["telemetry"]]
        for count in range(1, configured_port_count + 1):
            baseline_scenarios.append((f"{count} MAVLink port(s)", [rc_port] + telemetry_ports[: max(0, count - 1)]))
        for label, active_telemetry_ports in baseline_scenarios:
            print(f"baseline_run_start label={label} ports={active_telemetry_ports}", flush=True)
            result = run_workload(
                config=config,
                sitl_pid=sitl_proc.pid,
                telemetry_ports=active_telemetry_ports,
                rc_port=int(config["ports"]["rc"]),
                load_rates_hz={},
                duration_s=float(config["tests"]["baseline_duration_s"]),
            )
            report["baseline"][label] = result
            print(f"baseline_run_done label={label}", flush=True)

        if bool(config["tests"].get("run_mavsdk_probe", True)) and System is not None:
            print("mavsdk_probe_start", flush=True)
            mavsdk_probe_ports = [int(config["ports"]["rc"])] + [int(p) for p in config["ports"]["telemetry"]]
            for port in mavsdk_probe_ports:
                probe = probe_mavsdk_sync(f"tcp://127.0.0.1:{port}", float(config["tests"]["mavsdk_probe_timeout_s"]))
                report["mavsdk_probes"].append(probe)
            print("mavsdk_probe_done", flush=True)
        else:
            print("mavsdk_probe_skipped", flush=True)

        stress_port = int(config["ports"]["telemetry"][0])
        active_telemetry_ports = [int(config["ports"]["rc"])] + [int(port) for port in config["ports"]["telemetry"]]
        for rate in config["tests"]["progressive_rates_hz"]:
            print(f"single_port_stress_start rate={rate}", flush=True)
            result = run_workload(
                config=config,
                sitl_pid=sitl_proc.pid,
                telemetry_ports=active_telemetry_ports,
                rc_port=int(config["ports"]["rc"]),
                load_rates_hz={stress_port: float(rate)},
                duration_s=float(config["tests"]["stress_duration_s"]),
            )
            report["single_port_stress"].append({"rate_hz": rate, "result": result})

        print("all_ports_max_start", flush=True)
        max_rate = float(config["tests"]["max_rate_hz"])
        max_loads = {port: max_rate for port in active_telemetry_ports}
        result = run_workload(
            config=config,
            sitl_pid=sitl_proc.pid,
            telemetry_ports=active_telemetry_ports,
            rc_port=int(config["ports"]["rc"]),
            load_rates_hz=max_loads,
            duration_s=float(config["tests"]["all_ports_max_duration_s"]),
        )
        report["all_ports_max"] = result
        print("all_ports_max_done", flush=True)

        write_testing_report(config, report)
        print(f"report_written={config['output']['testing_md']}")
    finally:
        sitl_proc.terminate()
        try:
            sitl_proc.wait(timeout=5.0)
        except subprocess.TimeoutExpired:
            sitl_proc.kill()
            sitl_proc.wait(timeout=5.0)
        log_handle.close()

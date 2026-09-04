#!/usr/bin/env python3
"""Deterministic Dual RX integration test against INAV SITL.

No RF hardware, serial adapters, pymavlink, mspapi2, or Configurator required.
The harness launches SITL, configures two CRSF receivers on UART3/UART4,
feeds real CRSF RC/link-statistics frames into the SITL TCP UARTs, and uses
MSP on UART2 as the test oracle.

Typical use from the INAV repository root:

    python3 src/test/dualrx/dualrx_sitl_test.py

or as a standalone file:

    python3 dualrx_sitl_test.py --repo /path/to/inav \
        --sitl /path/to/inav/cmake/build_SITL/inav_10.0.0_SITL

The test uses a fresh temporary EEPROM by default and does not touch hardware.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import glob
import os
from pathlib import Path
import random
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time
from typing import Callable


# SITL defaults.
DEFAULT_TCP_BASE = 5760
UART_MSP = 2
UART_RX1 = 3
UART_RX2 = 4

# Serial function masks.
FUNCTION_MSP = 1
FUNCTION_RX_SERIAL = 64
FUNCTION_RX_SERIAL_SECONDARY = 8192

# MSP commands used by this test.
MSP_RC = 105
MSP_ACTIVEBOXES = 113
MSP_BOXIDS = 119
MSP_SET_RAW_RC = 200
MSP2_INAV_GET_LINK_STATS = 0x2103
MSP2_INAV_SET_RX_LINK = 0x2232

# Permanent mode IDs returned through MSP_BOXIDS.
BOX_BEEPER = 13
BOX_FAILSAFE = 27

# Dual RX enums in the recovered/current implementation.
RX1 = 0
RX2 = 1
RX_DUAL_STATUS_OK = 1
RX_LINK_SWITCH_BOOT = 0
RX_LINK_SWITCH_LINK_LOSS = 1
RX_LINK_SWITCH_HANDOVER_MSP = 3

# CRSF framing.
CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8
CRSF_FRAMETYPE_LINK_STATISTICS = 0x14
CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16
CRSF_CHANNEL_COUNT = 16


class TestFailure(RuntimeError):
    pass


class MspError(RuntimeError):
    def __init__(self, cmd: int, payload: bytes = b"") -> None:
        super().__init__(f"MSP command 0x{cmd:04X} returned error payload={payload.hex()}")
        self.cmd = cmd
        self.payload = payload


def crc8_dvb_s2(data: bytes, crc: int = 0) -> int:
    """CRC-8/DVB-S2, polynomial 0xD5. Used by CRSF and MSPv2."""
    for value in data:
        crc ^= value
        for _ in range(8):
            crc = ((crc << 1) ^ 0xD5) & 0xFF if (crc & 0x80) else (crc << 1) & 0xFF
    return crc


def crsf_frame(frame_type: int, payload: bytes) -> bytes:
    body = bytes([frame_type]) + payload
    return bytes([CRSF_ADDRESS_FLIGHT_CONTROLLER, len(body) + 1]) + body + bytes([crc8_dvb_s2(body)])


def us_to_crsf(value_us: int) -> int:
    value_us = max(1000, min(2000, int(value_us)))
    return 172 + round((value_us - 1000) * 1639 / 1000)


def crsf_value_to_inav_us(value: int) -> int:
    # Matches crsfReadRawRC() in src/main/rx/crsf.c.
    return (value * 1024 // 1639) + 881


def expected_inav_us(value_us: int) -> int:
    return crsf_value_to_inav_us(us_to_crsf(value_us))


def crsf_rc_frame(channels_us: list[int]) -> bytes:
    if len(channels_us) != CRSF_CHANNEL_COUNT:
        raise ValueError(f"expected {CRSF_CHANNEL_COUNT} channels, got {len(channels_us)}")
    packed = 0
    for index, value_us in enumerate(channels_us):
        packed |= (us_to_crsf(value_us) & 0x7FF) << (index * 11)
    return crsf_frame(CRSF_FRAMETYPE_RC_CHANNELS_PACKED, packed.to_bytes(22, "little"))


def crsf_link_stats_frame(*, rssi_dbm: int, lq: int, snr_db: int, rf_mode: int = 2, tx_power_index: int = 3) -> bytes:
    # CRSF link-statistics RSSI is an unsigned magnitude; INAV stores it negative.
    rssi_mag = max(0, min(255, abs(int(rssi_dbm))))
    lq = max(0, min(100, int(lq)))
    snr = int(snr_db) & 0xFF
    payload = bytes([
        rssi_mag,          # uplink RSSI antenna 1
        rssi_mag,          # uplink RSSI antenna 2
        lq,                # uplink LQ
        snr,               # uplink SNR, int8
        0,                 # active antenna
        rf_mode & 0xFF,
        tx_power_index & 0xFF,
        rssi_mag,          # downlink RSSI
        lq,                # downlink LQ
        snr,               # downlink SNR
    ])
    return crsf_frame(CRSF_FRAMETYPE_LINK_STATISTICS, payload)


def msp_v2_request(cmd: int, payload: bytes = b"") -> bytes:
    header = struct.pack("<BHH", 0, cmd, len(payload))
    crc = crc8_dvb_s2(header + payload)
    return b"$X<" + header + payload + bytes([crc])


class MspClient:
    def __init__(self, host: str, port: int, timeout_s: float = 2.0) -> None:
        self.sock = socket.create_connection((host, port), timeout=timeout_s)
        self.sock.settimeout(timeout_s)
        self.buffer = bytearray()
        self._box_ids: list[int] | None = None
        self._request_lock = threading.Lock()

    def close(self) -> None:
        try:
            self.sock.close()
        except OSError:
            pass

    def __enter__(self) -> "MspClient":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def _recv_into_buffer(self) -> None:
        chunk = self.sock.recv(65536)
        if not chunk:
            raise ConnectionError("MSP TCP connection closed")
        self.buffer.extend(chunk)

    def _read_frame(self, deadline: float) -> tuple[str, int, bytes]:
        while time.monotonic() < deadline:
            # Find a native MSPv2 magic prefix.
            pos = self.buffer.find(b"$X")
            if pos < 0:
                if len(self.buffer) > 1:
                    del self.buffer[:-1]
                self._recv_into_buffer()
                continue
            if pos:
                del self.buffer[:pos]

            while len(self.buffer) < 8:
                self._recv_into_buffer()
            direction = chr(self.buffer[2])
            flags, cmd, size = struct.unpack_from("<BHH", self.buffer, 3)
            del flags
            total = 3 + 5 + size + 1
            while len(self.buffer) < total:
                self._recv_into_buffer()
            frame = bytes(self.buffer[:total])
            del self.buffer[:total]
            payload = frame[8:-1]
            expected_crc = crc8_dvb_s2(frame[3:-1])
            if frame[-1] != expected_crc:
                # Resynchronize instead of trusting a damaged frame.
                continue
            return direction, cmd, payload
        raise TimeoutError("timed out waiting for MSP response")

    def request(self, cmd: int, payload: bytes = b"", timeout_s: float = 2.0) -> bytes:
        with self._request_lock:
            self.sock.sendall(msp_v2_request(cmd, payload))
            deadline = time.monotonic() + timeout_s
            while time.monotonic() < deadline:
                direction, response_cmd, response_payload = self._read_frame(deadline)
                if response_cmd != cmd:
                    continue
                if direction == "!":
                    raise MspError(cmd, response_payload)
                if direction != ">":
                    continue
                return response_payload
            raise TimeoutError(f"timed out waiting for MSP response 0x{cmd:04X}")

    def link_status(self) -> "DualRxStatus":
        return parse_link_status(self.request(MSP2_INAV_GET_LINK_STATS))

    def rc_channels(self) -> list[int]:
        payload = self.request(MSP_RC)
        if len(payload) % 2:
            raise TestFailure(f"MSP_RC odd payload length {len(payload)}")
        return list(struct.unpack("<" + "H" * (len(payload) // 2), payload))

    def handover(self, link: int) -> None:
        self.request(MSP2_INAV_SET_RX_LINK, bytes([link]))

    def set_raw_rc(self, channels: list[int]) -> None:
        payload = struct.pack("<" + "H" * len(channels), *channels)
        self.request(MSP_SET_RAW_RC, payload)

    def active_modes(self) -> set[int]:
        if self._box_ids is None:
            self._box_ids = list(self.request(MSP_BOXIDS))
        active = self.request(MSP_ACTIVEBOXES)
        return {
            permanent_id
            for index, permanent_id in enumerate(self._box_ids)
            if index // 8 < len(active) and active[index // 8] & (1 << (index % 8))
        }

    def mode_active(self, permanent_id: int) -> bool:
        return permanent_id in self.active_modes()


@dataclass
class LinkStatistics:
    valid_fields: int
    quality: int
    uplink_rssi: int
    uplink_lq: int
    uplink_snr: int
    downlink_lq: int
    rf_mode: int
    uplink_tx_power: int
    downlink_tx_power: int
    active_antenna: int
    band: str
    mode: str


@dataclass
class DualRxStatus:
    legacy_rssi: int
    legacy_lq: int
    legacy_snr: int
    extension_version: int
    active: int
    configured_mask: int
    initialized_mask: int
    valid_mask: int
    stats_valid_mask: int
    dual_status: int
    switch_reason: int
    switch_time_ms: int
    links: tuple[LinkStatistics, LinkStatistics]


def _signed8(value: int) -> int:
    return value - 256 if value >= 128 else value


def _signed16(value: int) -> int:
    return value - 65536 if value >= 32768 else value


def _cstring_fixed(data: bytes) -> str:
    return data.split(b"\0", 1)[0].decode("ascii", errors="replace")


def parse_link_status(payload: bytes) -> DualRxStatus:
    # 3 legacy bytes + 12-byte extension header + 25 bytes per link.
    if len(payload) < 65:
        raise TestFailure(f"GET_LINK_STATS payload too short: {len(payload)} bytes, expected >=65")
    off = 0
    legacy_rssi = payload[off]
    legacy_lq = payload[off + 1]
    legacy_snr = _signed8(payload[off + 2])
    off += 3

    extension_version = payload[off]
    active = payload[off + 1]
    configured_mask = payload[off + 2]
    initialized_mask = payload[off + 3]
    valid_mask = payload[off + 4]
    stats_valid_mask = payload[off + 5]
    dual_status = payload[off + 6]
    switch_reason = payload[off + 7]
    switch_time_ms = struct.unpack_from("<I", payload, off + 8)[0]
    off += 12

    links: list[LinkStatistics] = []
    for _ in range(2):
        valid_fields, quality, raw_rssi = struct.unpack_from("<HHH", payload, off)
        off += 6
        uplink_lq = payload[off]
        uplink_snr = _signed8(payload[off + 1])
        downlink_lq = payload[off + 2]
        rf_mode = payload[off + 3]
        off += 4
        uplink_tx_power, downlink_tx_power = struct.unpack_from("<HH", payload, off)
        off += 4
        active_antenna = payload[off]
        off += 1
        band = _cstring_fixed(payload[off:off + 4])
        off += 4
        mode = _cstring_fixed(payload[off:off + 6])
        off += 6
        links.append(LinkStatistics(
            valid_fields=valid_fields,
            quality=quality,
            uplink_rssi=_signed16(raw_rssi),
            uplink_lq=uplink_lq,
            uplink_snr=uplink_snr,
            downlink_lq=downlink_lq,
            rf_mode=rf_mode,
            uplink_tx_power=uplink_tx_power,
            downlink_tx_power=downlink_tx_power,
            active_antenna=active_antenna,
            band=band,
            mode=mode,
        ))

    return DualRxStatus(
        legacy_rssi=legacy_rssi,
        legacy_lq=legacy_lq,
        legacy_snr=legacy_snr,
        extension_version=extension_version,
        active=active,
        configured_mask=configured_mask,
        initialized_mask=initialized_mask,
        valid_mask=valid_mask,
        stats_valid_mask=stats_valid_mask,
        dual_status=dual_status,
        switch_reason=switch_reason,
        switch_time_ms=switch_time_ms,
        links=(links[0], links[1]),
    )


class VirtualCrsfReceiver:
    def __init__(self, name: str, host: str, port: int, channels_us: list[int]) -> None:
        self.name = name
        self.host = host
        self.port = port
        self.channels_us = list(channels_us)
        self.rc_enabled = False
        self.stats_enabled = False
        self.rssi_dbm = -80
        self.lq = 75
        self.snr_db = 5
        self.rc_period_s = 1.0 / 50.0
        self.stats_period_s = 0.2
        self._lock = threading.Lock()
        self._send_lock = threading.Lock()
        self._receive_lock = threading.Lock()
        self._received = bytearray()
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None
        self._sock: socket.socket | None = None
        self.error: BaseException | None = None

    def connect(self, timeout_s: float = 5.0) -> None:
        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            try:
                self._sock = socket.create_connection((self.host, self.port), timeout=1.0)
                self._sock.setblocking(False)
                self._thread = threading.Thread(target=self._run, name=self.name, daemon=True)
                self._thread.start()
                return
            except OSError:
                time.sleep(0.05)
        raise TimeoutError(f"{self.name}: could not connect to {self.host}:{self.port}")

    def close(self) -> None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=1.0)
        if self._sock is not None:
            try:
                self._sock.close()
            except OSError:
                pass

    def set_rc(self, enabled: bool) -> None:
        with self._lock:
            self.rc_enabled = enabled

    def set_stats(self, enabled: bool) -> None:
        with self._lock:
            self.stats_enabled = enabled

    def set_channel(self, index: int, value_us: int) -> None:
        with self._lock:
            self.channels_us[index] = value_us

    def set_stats_values(self, *, rssi_dbm: int, lq: int, snr_db: int) -> None:
        with self._lock:
            self.rssi_dbm = rssi_dbm
            self.lq = lq
            self.snr_db = snr_db

    def send_fragmented(self, data: bytes, *, chunk_size: int = 1, inter_chunk_s: float = 0.002) -> None:
        if chunk_size <= 0:
            raise ValueError("chunk_size must be positive")
        if self._sock is None:
            raise RuntimeError(f"{self.name}: not connected")
        with self._send_lock:
            for offset in range(0, len(data), chunk_size):
                self._sock.sendall(data[offset:offset + chunk_size])
                if inter_chunk_s:
                    time.sleep(inter_chunk_s)

    def _send(self, data: bytes) -> None:
        assert self._sock is not None
        with self._send_lock:
            self._sock.sendall(data)

    def take_received(self) -> bytes:
        with self._receive_lock:
            data = bytes(self._received)
            self._received.clear()
            return data

    def _run(self) -> None:
        assert self._sock is not None
        next_rc = time.monotonic()
        next_stats = time.monotonic()
        try:
            while not self._stop.is_set():
                now = time.monotonic()
                with self._lock:
                    rc_enabled = self.rc_enabled
                    stats_enabled = self.stats_enabled
                    channels = list(self.channels_us)
                    rssi_dbm = self.rssi_dbm
                    lq = self.lq
                    snr_db = self.snr_db

                if rc_enabled and now >= next_rc:
                    self._send(crsf_rc_frame(channels))
                    next_rc = now + self.rc_period_s
                elif not rc_enabled:
                    next_rc = now

                if stats_enabled and now >= next_stats:
                    self._send(crsf_link_stats_frame(rssi_dbm=rssi_dbm, lq=lq, snr_db=snr_db))
                    next_stats = now + self.stats_period_s
                elif not stats_enabled:
                    next_stats = now

                # Drain FC -> RX bytes if a SITL build has CRSF telemetry enabled.
                try:
                    while chunk := self._sock.recv(4096):
                        with self._receive_lock:
                            self._received.extend(chunk)
                except BlockingIOError:
                    pass
                except OSError:
                    raise

                time.sleep(0.002)
        except BaseException as exc:  # Preserve background failures for the main thread.
            if not self._stop.is_set():
                self.error = exc
                self._stop.set()

    def check(self) -> None:
        if self.error is not None:
            raise TestFailure(f"{self.name} background thread failed: {self.error}")


class SitlProcess:
    def __init__(self, binary: Path, repo: Path, eeprom: Path, tcp_base: int, log_path: Path) -> None:
        self.binary = binary
        self.repo = repo
        self.eeprom = eeprom
        self.tcp_base = tcp_base
        self.log_path = log_path
        self.process: subprocess.Popen[str] | None = None
        self._log_file = None

    def start(self) -> None:
        self.log_path.parent.mkdir(parents=True, exist_ok=True)
        self._log_file = self.log_path.open("w", encoding="utf-8")
        env = os.environ.copy()
        env.pop("LD_LIBRARY_PATH", None)
        command = [
            str(self.binary),
            f"--path={self.eeprom}",
            f"--tcpbaseport={self.tcp_base}",
        ]
        self.process = subprocess.Popen(
            command,
            cwd=str(self.repo),
            stdout=self._log_file,
            stderr=subprocess.STDOUT,
            text=True,
            start_new_session=True,
            env=env,
        )

    def check_alive(self) -> None:
        if self.process is None:
            raise RuntimeError("SITL not started")
        code = self.process.poll()
        if code is not None:
            raise TestFailure(f"SITL exited with code {code}\n{self.tail_log()}")

    def stop(self) -> None:
        if self.process is not None and self.process.poll() is None:
            try:
                os.killpg(self.process.pid, signal.SIGINT)
                self.process.wait(timeout=5.0)
            except (ProcessLookupError, subprocess.TimeoutExpired):
                try:
                    os.killpg(self.process.pid, signal.SIGTERM)
                    self.process.wait(timeout=2.0)
                except (ProcessLookupError, subprocess.TimeoutExpired):
                    try:
                        os.killpg(self.process.pid, signal.SIGKILL)
                    except ProcessLookupError:
                        pass
        if self._log_file is not None:
            self._log_file.close()

    def tail_log(self, lines: int = 30) -> str:
        try:
            content = self.log_path.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            return "<no SITL log>"
        return "\n".join(content[-lines:])


def tcp_port(tcp_base: int, uart: int) -> int:
    return tcp_base + uart - 1


def wait_tcp(host: str, port: int, timeout_s: float, process: SitlProcess | None = None) -> None:
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        if process is not None:
            process.check_alive()
        try:
            with socket.create_connection((host, port), timeout=0.25):
                return
        except OSError:
            time.sleep(0.05)
    raise TimeoutError(f"TCP port {host}:{port} did not open within {timeout_s:.1f}s")


def cli_read_until_prompt(sock: socket.socket, timeout_s: float = 3.0) -> str:
    sock.settimeout(timeout_s)
    data = bytearray()
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        chunk = sock.recv(65536)
        if not chunk:
            raise ConnectionError("CLI socket closed before prompt")
        data.extend(chunk)
        if b"\n# " in data or data.endswith(b"# "):
            return data.decode("utf-8", errors="replace")
    raise TimeoutError("CLI prompt not received")


def configure_cli(host: str, msp_port: int, commands: list[str]) -> None:
    # The first non-MSP '#' enters CLI after the normal MSP guard interval.
    with socket.create_connection((host, msp_port), timeout=3.0) as sock:
        sock.sendall(b"#\n")
        cli_read_until_prompt(sock)
        for command in commands:
            sock.sendall(command.encode("ascii") + b"\n")
            response = cli_read_until_prompt(sock)
            lowered = response.lower()
            if "invalid" in lowered or "error" in lowered:
                raise TestFailure(f"CLI command failed: {command}\n{response}")
        sock.sendall(b"save\n")
        # save reboots SITL; disconnect is expected.


def configure_dual_crsf(host: str, msp_port: int) -> None:
    configure_cli(host, msp_port, [
        "set receiver_type = SERIAL",
        "set serialrx_provider = CRSF",
        "set dual_rx_enabled = ON",
        "set receiver_type_rx2 = SERIAL",
        "set serialrx_provider_rx2 = CRSF",
        "aux 0 13 0 1700 2100",
        f"serial 2 {FUNCTION_RX_SERIAL}",
        f"serial 3 {FUNCTION_RX_SERIAL_SECONDARY}",
    ])


def discover_repo(script_path: Path, explicit_repo: str | None) -> Path:
    if explicit_repo:
        return Path(explicit_repo).expanduser().resolve()
    # If installed under src/test/dualrx, this lands at repository root.
    if len(script_path.parents) >= 4:
        candidate = script_path.parents[3]
        if (candidate / "cmake").exists() and (candidate / "src").exists():
            return candidate
    return Path.cwd().resolve()


def discover_sitl(repo: Path, explicit_binary: str | None) -> Path:
    if explicit_binary:
        path = Path(explicit_binary).expanduser().resolve()
        if not path.is_file():
            raise FileNotFoundError(path)
        return path
    candidates = [Path(p) for p in glob.glob(str(repo / "cmake" / "build_SITL" / "inav_*_SITL"))]
    candidates = [p for p in candidates if p.is_file() and os.access(p, os.X_OK)]
    if not candidates:
        raise FileNotFoundError(
            "No SITL binary found under cmake/build_SITL/inav_*_SITL. "
            "Build SITL first or pass --sitl PATH."
        )
    return max(candidates, key=lambda p: p.stat().st_mtime)


def wait_status(
    msp: MspClient,
    predicate: Callable[[DualRxStatus], bool],
    description: str,
    receivers: tuple[VirtualCrsfReceiver, VirtualCrsfReceiver],
    timeout_s: float = 1.5,
) -> DualRxStatus:
    deadline = time.monotonic() + timeout_s
    last: DualRxStatus | None = None
    while time.monotonic() < deadline:
        for receiver in receivers:
            receiver.check()
        last = msp.link_status()
        if predicate(last):
            return last
        time.sleep(0.02)
    raise TestFailure(f"timeout waiting for {description}; last={last}")


def assert_rc_signature(msp: MspClient, channel_index: int, requested_us: int, label: str, tolerance_us: int = 4) -> None:
    channels = msp.rc_channels()
    if channel_index >= len(channels):
        raise TestFailure(f"{label}: MSP_RC only returned {len(channels)} channels")
    expected = expected_inav_us(requested_us)
    actual = channels[channel_index]
    if abs(actual - expected) > tolerance_us:
        raise TestFailure(f"{label}: channel {channel_index} expected ~{expected}, got {actual}")


def wait_mode(
    msp: MspClient,
    permanent_id: int,
    expected: bool,
    description: str,
    receivers: tuple[VirtualCrsfReceiver, VirtualCrsfReceiver],
    timeout_s: float = 1.5,
) -> None:
    deadline = time.monotonic() + timeout_s
    last = False
    while time.monotonic() < deadline:
        for receiver in receivers:
            receiver.check()
        last = msp.mode_active(permanent_id)
        if last == expected:
            return
        time.sleep(0.02)
    raise TestFailure(f"timeout waiting for {description}; mode {permanent_id} active={last}")


def print_pass(message: str) -> None:
    print(f"[PASS] {message}")


def deterministic_suite(msp: MspClient, rx1: VirtualCrsfReceiver, rx2: VirtualCrsfReceiver) -> None:
    receivers = (rx1, rx2)
    rx1_sig = rx1.channels_us[0]
    rx2_sig = rx2.channels_us[0]

    status = msp.link_status()
    if status.extension_version != 1:
        raise TestFailure(f"unexpected Dual RX stats extension version {status.extension_version}")
    if status.configured_mask != 0x03 or status.initialized_mask != 0x03:
        raise TestFailure(f"configured/initialized masks wrong: {status}")
    if status.dual_status != RX_DUAL_STATUS_OK:
        raise TestFailure(f"dual_rx_status={status.dual_status}, expected OK({RX_DUAL_STATUS_OK})")
    print_pass("Dual RX configured and both receiver instances initialized")

    # Start RX1 first so the boot default is not obscured by a deliberate RX2-first race.
    rx1.set_rc(True)
    rx1.set_stats(True)
    wait_status(msp, lambda s: s.valid_mask == 0x01 and s.active == RX1, "RX1 valid/active", receivers)
    assert_rc_signature(msp, 0, rx1_sig, "RX1 boot source")
    print_pass("RX1 becomes the live source from the boot default")

    rx2.set_rc(True)
    rx2.set_stats(True)
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX1, "both valid with RX1 latched", receivers)
    assert_rc_signature(msp, 0, rx1_sig, "both healthy, RX1 latched")
    print_pass("Both links healthy, RX1 remains latched")

    # Standby traffic must not leak into the live RC stream.
    rx2.set_channel(0, rx2_sig)
    time.sleep(0.15)
    assert_rc_signature(msp, 0, rx1_sig, "standby isolation")
    print_pass("Standby RX2 traffic does not publish into live MSP_RC")

    # AUX1 is configured as the BEEPER mode. RX1 holds it low while RX2 holds
    # it high, so this observes the downstream mode pipeline rather than only
    # the legacy channel array.
    wait_mode(msp, BOX_BEEPER, False, "RX1 low AUX keeps beeper inactive", receivers)
    for value in (1100, 1900, 1100, 1900):
        rx2.set_channel(4, value)
        time.sleep(0.06)
        if msp.mode_active(BOX_BEEPER):
            raise TestFailure("standby RX2 AUX traffic activated the BEEPER mode")
    print_pass("Standby RX2 AUX traffic does not advance downstream mode state")

    # Failsafe monitoring begins five seconds after boot. Keep both links live
    # through that boundary so subsequent loss assertions exercise the actual
    # FC failsafe state machine rather than only the selector's valid mask.
    time.sleep(5.1)
    wait_mode(msp, BOX_FAILSAFE, False, "no failsafe with both links valid", receivers)

    rx1.set_rc(False)
    status = wait_status(
        msp,
        lambda s: s.valid_mask == 0x02 and s.active == RX2 and s.switch_reason == RX_LINK_SWITCH_LINK_LOSS,
        "RX1 loss -> RX2",
        receivers,
    )
    del status
    assert_rc_signature(msp, 0, rx2_sig, "RX1 loss failover")
    wait_mode(msp, BOX_FAILSAFE, False, "no FC failsafe while RX2 remains valid", receivers)
    wait_mode(msp, BOX_BEEPER, True, "RX2 high AUX activates beeper after failover", receivers)
    print_pass("RX1 loss switches once to RX2")

    rx1.set_rc(True)
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX2, "RX1 recovery without failback", receivers)
    assert_rc_signature(msp, 0, rx2_sig, "RX1 recovery")
    rx1.set_channel(4, 1900)
    time.sleep(0.08)
    if not msp.mode_active(BOX_BEEPER):
        raise TestFailure("inactive RX1 AUX traffic changed the active RX2 BEEPER mode")
    rx1.set_channel(4, 1100)
    print_pass("Recovered inactive RX1 does not preempt valid RX2")

    rx2.set_rc(False)
    wait_status(
        msp,
        lambda s: s.valid_mask == 0x01 and s.active == RX1 and s.switch_reason == RX_LINK_SWITCH_LINK_LOSS,
        "RX2 loss -> RX1",
        receivers,
    )
    assert_rc_signature(msp, 0, rx1_sig, "RX2 loss failover")
    wait_mode(msp, BOX_FAILSAFE, False, "no FC failsafe while RX1 remains valid", receivers)
    wait_mode(msp, BOX_BEEPER, False, "RX1 low AUX applies after reverse failover", receivers)
    print_pass("RX2 loss switches to valid RX1")

    rx1.set_rc(False)
    wait_status(msp, lambda s: s.valid_mask == 0x00 and s.active == RX1, "both links down with RX1 identity latched", receivers)
    wait_mode(msp, BOX_FAILSAFE, True, "FC failsafe after both links are lost", receivers)
    print_pass("Both links down enters FC failsafe and keeps active identity latched")

    rx2.set_rc(True)
    wait_status(
        msp,
        lambda s: s.valid_mask == 0x02 and s.active == RX2 and s.switch_reason == RX_LINK_SWITCH_LINK_LOSS,
        "other-link recovery after total loss",
        receivers,
    )
    assert_rc_signature(msp, 0, rx2_sig, "other-link recovery")
    wait_mode(msp, BOX_FAILSAFE, False, "FC failsafe clears after RX2 recovery", receivers)
    print_pass("After total loss, sole recovered RX2 becomes the control source")

    rx1.set_rc(True)
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX2, "both recovered with RX2 latched", receivers)
    print_pass("RX1 recovery after total loss does not steal control back from valid RX2")

    # MSP handover is a queued one-shot event.
    msp.handover(RX1)
    wait_status(
        msp,
        lambda s: s.active == RX1 and s.switch_reason == RX_LINK_SWITCH_HANDOVER_MSP,
        "MSP handover to RX1",
        receivers,
    )
    assert_rc_signature(msp, 0, rx1_sig, "MSP handover RX1")
    msp.handover(RX2)
    wait_status(
        msp,
        lambda s: s.active == RX2 and s.switch_reason == RX_LINK_SWITCH_HANDOVER_MSP,
        "MSP handover to RX2",
        receivers,
    )
    assert_rc_signature(msp, 0, rx2_sig, "MSP handover RX2")
    print_pass("Explicit MSP handover works in both directions")

    # Invalid handover target must be rejected.
    rx1.set_rc(False)
    wait_status(msp, lambda s: s.valid_mask == 0x02 and s.active == RX2, "RX1 invalid before rejected handover", receivers)
    try:
        msp.handover(RX1)
    except MspError:
        pass
    else:
        raise TestFailure("handover to invalid RX1 was accepted")
    status = msp.link_status()
    if status.active != RX2:
        raise TestFailure(f"invalid handover changed active link: {status.active}")
    print_pass("Handover to an invalid target is rejected")
    rx1.set_rc(True)
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX2, "RX1 restored after rejected handover", receivers)

    # Regression: stale structured stats must be invalidated on signal loss and
    # must not reappear merely because RC frames recover before new stats arrive.
    rx2.set_stats_values(rssi_dbm=-81, lq=72, snr_db=7)
    rx2.set_stats(True)
    status = wait_status(
        msp,
        lambda s: (s.stats_valid_mask & 0x02) != 0 and s.links[RX2].uplink_lq == 72,
        "fresh RX2 statistics",
        receivers,
    )
    if status.links[RX2].uplink_rssi != -81:
        raise TestFailure(f"RX2 stats seed RSSI mismatch: {status.links[RX2].uplink_rssi}")

    rx2.set_stats(False)
    time.sleep(0.25)  # Ensure no fresh statistics frame is queued.
    rx2.set_rc(False)
    status = wait_status(
        msp,
        lambda s: (s.valid_mask & 0x02) == 0 and (s.stats_valid_mask & 0x02) == 0,
        "RX2 stats invalidated on signal loss",
        receivers,
    )
    if status.links[RX2].valid_fields != 0 or status.links[RX2].uplink_lq != 0 or status.links[RX2].uplink_rssi != 0:
        raise TestFailure(f"RX2 structured stats not cleared on loss: {status.links[RX2]}")

    rx2.set_rc(True)
    status = wait_status(
        msp,
        lambda s: (s.valid_mask & 0x02) != 0,
        "RX2 RC recovery without stats",
        receivers,
    )
    if (status.stats_valid_mask & 0x02) != 0 or status.links[RX2].valid_fields != 0:
        raise TestFailure(f"RX2 stale stats became valid again on RC-only recovery: {status.links[RX2]}")
    print_pass("Structured link stats stay invalid across loss -> RC-only recovery")

    rx2.set_stats(True)
    wait_status(
        msp,
        lambda s: (s.stats_valid_mask & 0x02) != 0 and s.links[RX2].uplink_lq == 72,
        "RX2 fresh stats after recovery",
        receivers,
    )
    print_pass("Fresh RX2 statistics become valid again after recovery")


def parser_abuse_suite(msp: MspClient, rx1: VirtualCrsfReceiver, rx2: VirtualCrsfReceiver) -> None:
    receivers = (rx1, rx2)
    rx1.set_stats(False)
    rx2.set_stats(False)
    rx1.set_rc(True)
    rx2.set_rc(True)
    wait_status(msp, lambda s: s.valid_mask == 0x03, "parser test initial both-valid", receivers)

    if msp.link_status().active != RX2:
        msp.handover(RX2)
        wait_status(msp, lambda s: s.active == RX2, "parser test handover to RX2", receivers)

    rx1.set_rc(False)
    wait_status(msp, lambda s: s.valid_mask == 0x02 and s.active == RX2, "RX1 invalid before parser abuse", receivers)

    valid = crsf_rc_frame(rx1.channels_us)
    bad_crc = valid[:-1] + bytes([valid[-1] ^ 0xFF])
    rx1.send_fragmented(b"\x00\xFFgarbage" + bad_crc, chunk_size=2, inter_chunk_s=0.001)
    time.sleep(0.15)
    status = msp.link_status()
    if status.valid_mask != 0x02 or status.active != RX2:
        raise TestFailure(f"garbage/bad-CRC RX1 traffic disturbed selector state: {status}")
    assert_rc_signature(msp, 0, rx2.channels_us[0], "parser corruption isolation")
    print_pass("Garbage and bad-CRC RX1 traffic cannot disturb healthy RX2 control")

    # CRSF is a 420-kbaud wire protocol: split the TCP writes, but do not add
    # millisecond-scale gaps that correctly look like separate incomplete
    # frames to the firmware parser.
    rx1.send_fragmented(valid, chunk_size=1, inter_chunk_s=0)
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX2, "byte-fragmented RX1 recovery", receivers)
    assert_rc_signature(msp, 0, rx2.channels_us[0], "fragmented standby isolation")
    print_pass("Byte-at-a-time CRSF recovery parses correctly without preempting active RX2")

    rx1.set_rc(True)


def boot_order_probe(
    binary: Path,
    repo: Path,
    eeprom: Path,
    tcp_base: int,
    log_path: Path,
    order: str,
) -> None:
    sitl = SitlProcess(binary, repo, eeprom, tcp_base, log_path)
    rx1: VirtualCrsfReceiver | None = None
    rx2: VirtualCrsfReceiver | None = None
    msp: MspClient | None = None
    try:
        sitl.start()
        msp_port = tcp_port(tcp_base, UART_MSP)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(tcp_base, UART_RX1), 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(tcp_base, UART_RX2), 8.0, sitl)

        rx1_channels = [1300, 1400, 1200, 1500] + [1100] * 12
        rx2_channels = [1800, 1600, 1700, 1500] + [1900] * 12
        rx1 = VirtualCrsfReceiver("RX1", "127.0.0.1", tcp_port(tcp_base, UART_RX1), rx1_channels)
        rx2 = VirtualCrsfReceiver("RX2", "127.0.0.1", tcp_port(tcp_base, UART_RX2), rx2_channels)
        rx1.connect()
        rx2.connect()
        msp = MspClient("127.0.0.1", msp_port, timeout_s=2.0)
        receivers = (rx1, rx2)

        if order == "rx1-first":
            rx1.set_rc(True)
            wait_status(msp, lambda s: s.valid_mask == 0x01 and s.active == RX1, "boot RX1-first", receivers)
            rx2.set_rc(True)
            wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX1, "boot RX1-first both-valid", receivers)
            assert_rc_signature(msp, 0, rx1.channels_us[0], "boot RX1-first source")
            print_pass("Fresh boot with RX1 first selects RX1 and remains latched")
            return

        if order == "rx2-first":
            rx2.set_rc(True)
            wait_status(msp, lambda s: s.valid_mask == 0x02 and s.active == RX2, "boot RX2-first", receivers)
            rx1.set_rc(True)
            wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX2, "boot RX2-first both-valid", receivers)
            assert_rc_signature(msp, 0, rx2.channels_us[0], "boot RX2-first source")
            print_pass("Fresh boot with RX2 first selects RX2 and RX1 recovery does not preempt it")
            return

        if order != "together":
            raise ValueError(f"unknown boot probe order {order}")

        rx1.set_rc(True)
        rx2.set_rc(True)
        status = wait_status(msp, lambda s: s.valid_mask == 0x03, "near-simultaneous boot", receivers)
        selected = status.active
        selected_receiver = rx1 if selected == RX1 else rx2
        assert_rc_signature(msp, 0, selected_receiver.channels_us[0], "near-simultaneous boot source")
        time.sleep(0.25)
        status = msp.link_status()
        if status.active != selected or status.valid_mask != 0x03:
            raise TestFailure(f"near-simultaneous boot source was not stable: {status}")

        rx1.set_rc(False)
        rx2.set_rc(False)
        wait_status(msp, lambda s: s.valid_mask == 0, "near-simultaneous total loss", receivers)
        time.sleep(0.2)  # Let already-buffered UART frames expire before recovery.
        status = msp.link_status()
        if status.valid_mask != 0:
            raise TestFailure(f"near-simultaneous total loss did not settle: {status}")
        rx1.set_rc(True)
        rx2.set_rc(True)
        status = wait_status(msp, lambda s: s.valid_mask == 0x03, "near-simultaneous recovery", receivers)
        recovered = status.active
        recovered_receiver = rx1 if recovered == RX1 else rx2
        assert_rc_signature(msp, 0, recovered_receiver.channels_us[0], "near-simultaneous recovery source")
        time.sleep(0.25)
        status = msp.link_status()
        if status.valid_mask != 0x03 or status.active != recovered:
            raise TestFailure(f"near-simultaneous recovery source was not stable: {status}")
        print_pass(
            f"Near-simultaneous boot is stable on RX{selected + 1}; "
            f"loss/recovery settles without oscillation on RX{recovered + 1}"
        )
    finally:
        if msp is not None:
            msp.close()
        if rx1 is not None:
            rx1.close()
        if rx2 is not None:
            rx2.close()
        sitl.stop()


def stress_suite(msp: MspClient, rx1: VirtualCrsfReceiver, rx2: VirtualCrsfReceiver, steps: int, seed: int) -> None:
    if steps <= 0:
        return
    receivers = (rx1, rx2)
    rng = random.Random(seed)
    rx1.set_stats(False)
    rx2.set_stats(False)
    rx1.set_rc(True)
    rx2.set_rc(True)
    status = wait_status(msp, lambda s: s.valid_mask == 0x03, "stress initial both-valid", receivers)
    model_active = status.active
    model_valid = [True, True]

    for step in range(steps):
        link = rng.randrange(2)
        new_state = not model_valid[link]
        (rx1 if link == RX1 else rx2).set_rc(new_state)
        model_valid[link] = new_state

        expected_mask = (1 if model_valid[0] else 0) | (2 if model_valid[1] else 0)
        if not model_valid[model_active] and model_valid[1 - model_active]:
            model_active = 1 - model_active

        status = wait_status(
            msp,
            lambda s, em=expected_mask, ea=model_active: s.valid_mask == em and s.active == ea,
            f"stress step {step + 1}/{steps}",
            receivers,
            timeout_s=1.2,
        )
        if status.valid_mask:
            requested = rx1.channels_us[0] if status.active == RX1 else rx2.channels_us[0]
            assert_rc_signature(msp, 0, requested, f"stress step {step + 1}")

    print_pass(f"Deterministic selector model matched {steps} randomized loss/recovery transitions (seed={seed})")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Exercise INAV Dual RX end-to-end in SITL with two virtual CRSF receivers.")
    parser.add_argument("--repo", help="INAV repository root (default: infer from script location/current directory)")
    parser.add_argument("--sitl", help="SITL executable (default: newest cmake/build_SITL/inav_*_SITL)")
    parser.add_argument("--tcp-base", type=int, default=DEFAULT_TCP_BASE, help=f"SITL TCP base port (default {DEFAULT_TCP_BASE})")
    parser.add_argument("--eeprom", help="EEPROM file path (default: temporary file)")
    parser.add_argument("--keep-eeprom", action="store_true", help="Do not delete the temporary EEPROM after the test")
    parser.add_argument("--stress", type=int, default=40, help="Randomized loss/recovery transitions after deterministic suite (default 40; 0 disables)")
    parser.add_argument("--seed", type=int, default=0xD0A1, help="Random stress seed")
    parser.add_argument("--skip-boot-matrix", action="store_true", help="Skip isolated RX1-first/RX2-first/simultaneous boot probes")
    parser.add_argument("--log", help="SITL log path (default: temporary file)")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    script_path = Path(__file__).resolve()
    repo = discover_repo(script_path, args.repo)
    sitl_binary = discover_sitl(repo, args.sitl)

    temp_dir_obj = tempfile.TemporaryDirectory(prefix="inav-dualrx-sitl-")
    temp_dir = Path(temp_dir_obj.name)
    eeprom = Path(args.eeprom).expanduser().resolve() if args.eeprom else temp_dir / "eeprom.bin"
    log_path = Path(args.log).expanduser().resolve() if args.log else temp_dir / "sitl.log"
    if eeprom.exists():
        eeprom.unlink()

    sitl = SitlProcess(sitl_binary, repo, eeprom, args.tcp_base, log_path)
    rx1: VirtualCrsfReceiver | None = None
    rx2: VirtualCrsfReceiver | None = None
    msp: MspClient | None = None

    print("Dual RX SITL integration test")
    print(f"  repo:   {repo}")
    print(f"  SITL:   {sitl_binary}")
    print(f"  EEPROM: {eeprom}")
    print(f"  TCP:    UART2={tcp_port(args.tcp_base, UART_MSP)} UART3={tcp_port(args.tcp_base, UART_RX1)} UART4={tcp_port(args.tcp_base, UART_RX2)}")
    print()

    try:
        sitl.start()
        msp_port = tcp_port(args.tcp_base, UART_MSP)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        time.sleep(0.15)
        configure_dual_crsf("127.0.0.1", msp_port)

        # save causes SITL to reboot/re-exec. Give it time to close old sockets,
        # then wait for all three configured UART listeners.
        time.sleep(1.0)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(args.tcp_base, UART_RX1), 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(args.tcp_base, UART_RX2), 8.0, sitl)
        time.sleep(0.15)

        rx1_channels = [1300, 1400, 1200, 1500] + [1100] * 12
        rx2_channels = [1800, 1600, 1700, 1500] + [1900] * 12
        rx1 = VirtualCrsfReceiver("RX1", "127.0.0.1", tcp_port(args.tcp_base, UART_RX1), rx1_channels)
        rx2 = VirtualCrsfReceiver("RX2", "127.0.0.1", tcp_port(args.tcp_base, UART_RX2), rx2_channels)
        rx1.set_stats_values(rssi_dbm=-70, lq=91, snr_db=10)
        rx2.set_stats_values(rssi_dbm=-80, lq=77, snr_db=6)
        rx1.connect()
        rx2.connect()
        time.sleep(0.1)

        msp = MspClient("127.0.0.1", msp_port, timeout_s=2.0)
        deterministic_suite(msp, rx1, rx2)
        parser_abuse_suite(msp, rx1, rx2)
        stress_suite(msp, rx1, rx2, args.stress, args.seed)

        if not args.skip_boot_matrix:
            msp.close()
            msp = None
            rx1.close()
            rx1 = None
            rx2.close()
            rx2 = None
            sitl.stop()
            for order in ("rx1-first", "rx2-first", "together"):
                boot_order_probe(
                    sitl_binary,
                    repo,
                    eeprom,
                    args.tcp_base,
                    temp_dir / f"sitl-{order}.log",
                    order,
                )

        print()
        print("ALL DUAL RX SITL TESTS PASSED")
        return 0

    except BaseException as exc:
        print()
        print(f"[FAIL] {exc}", file=sys.stderr)
        print("\n--- SITL log tail ---", file=sys.stderr)
        print(sitl.tail_log(), file=sys.stderr)
        return 1
    finally:
        if msp is not None:
            msp.close()
        if rx1 is not None:
            rx1.close()
        if rx2 is not None:
            rx2.close()
        sitl.stop()
        if args.keep_eeprom and not args.eeprom:
            kept = Path.cwd() / "dualrx-sitl-eeprom.bin"
            try:
                kept.write_bytes(eeprom.read_bytes())
                print(f"Kept EEPROM: {kept}")
            except OSError:
                pass
        temp_dir_obj.cleanup()


if __name__ == "__main__":
    raise SystemExit(main())

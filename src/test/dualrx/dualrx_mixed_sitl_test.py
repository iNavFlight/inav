#!/usr/bin/env python3
"""Exercise live mixed Dual RX ingress paths against INAV SITL."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import socket
import struct
import tempfile
import threading
import time
import traceback

from dualrx_sitl_test import (
    BOX_BEEPER,
    DEFAULT_TCP_BASE,
    FUNCTION_RX_SERIAL,
    FUNCTION_RX_SERIAL_SECONDARY,
    MspClient,
    RX1,
    RX2,
    RX_DUAL_STATUS_OK,
    SitlProcess,
    TestFailure,
    UART_MSP,
    VirtualCrsfReceiver,
    assert_rc_signature,
    configure_cli,
    discover_repo,
    discover_sitl,
    tcp_port,
    wait_mode,
    wait_status,
    wait_tcp,
)


FUNCTION_TELEMETRY_MAVLINK = 1 << 8
UART_RX1 = 3
UART_RX2 = 4
BOX_MSP_RC_OVERRIDE = 50

MAVLINK_V1_MAGIC = 0xFE
MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE = 70
MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE_CRC = 124
MAVLINK_TARGET_SYSTEM = 1
MAVLINK_TARGET_COMPONENT = 1
MAVLINK_SOURCE_COMPONENT = 191


class PeriodicIngress:
    def __init__(self, name: str, channels: list[int], period_s: float = 0.02) -> None:
        self.name = name
        self.channels = list(channels)
        self.period_s = period_s
        self.mode = "off"
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None
        self.error: Exception | None = None

    def start(self) -> None:
        self._thread = threading.Thread(target=self._run_guarded, name=self.name, daemon=True)
        self._thread.start()

    def close(self) -> None:
        self._stop.set()
        if self._thread is not None:
            self._thread.join(timeout=2.0)

    def set_mode(self, mode: str) -> None:
        if mode not in ("off", "full", "partial"):
            raise ValueError(mode)
        with self._lock:
            self.mode = mode

    def check(self) -> None:
        if self.error is not None:
            raise TestFailure(f"{self.name} background thread failed: {type(self.error).__name__}: {self.error}") from self.error

    def _run_guarded(self) -> None:
        try:
            self._run()
        except Exception as exc:
            if not self._stop.is_set():
                self.error = exc
                self._stop.set()

    def _run(self) -> None:
        raise NotImplementedError


def mavlink_x25_crc(data: bytes, crc: int = 0xFFFF) -> int:
    for value in data:
        tmp = value ^ (crc & 0xFF)
        tmp ^= (tmp << 4) & 0xFF
        crc = ((crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)) & 0xFFFF
    return crc


def mavlink_v1_rc_override(seq: int, source_system: int, channels: list[int]) -> bytes:
    if len(channels) < 8:
        raise ValueError("RC_CHANNELS_OVERRIDE test frame needs at least 8 channels")
    payload = struct.pack(
        "<8HBB",
        *[int(value) & 0xFFFF for value in channels[:8]],
        MAVLINK_TARGET_SYSTEM,
        MAVLINK_TARGET_COMPONENT,
    )
    header = bytes([
        len(payload),
        seq & 0xFF,
        source_system & 0xFF,
        MAVLINK_SOURCE_COMPONENT,
        MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE,
    ])
    crc = mavlink_x25_crc(header + payload)
    crc = mavlink_x25_crc(bytes([MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE_CRC]), crc)
    return bytes([MAVLINK_V1_MAGIC]) + header + payload + struct.pack("<H", crc)


class MavlinkIngress(PeriodicIngress):
    def __init__(self, name: str, host: str, port: int, channels: list[int], source_system: int) -> None:
        super().__init__(name, channels)
        self.source_system = source_system
        self.sequence = 0
        self.sock: socket.socket | None = None
        deadline = time.monotonic() + 5.0
        last_error: OSError | None = None
        while time.monotonic() < deadline:
            try:
                self.sock = socket.create_connection((host, port), timeout=1.0)
                self.sock.settimeout(None)
                break
            except OSError as exc:
                last_error = exc
                time.sleep(0.05)
        if self.sock is None:
            raise TimeoutError(f"{name}: could not connect to {host}:{port}: {last_error}")

    def close(self) -> None:
        super().close()
        if self.sock is not None:
            try:
                self.sock.close()
            except OSError:
                pass

    def _run(self) -> None:
        assert self.sock is not None
        next_send = time.monotonic()
        while not self._stop.is_set():
            now = time.monotonic()
            with self._lock:
                mode = self.mode
                channels = list(self.channels)
            if mode != "off" and now >= next_send:
                if mode == "partial":
                    channels[:4] = [0xFFFF] * 4
                frame = mavlink_v1_rc_override(self.sequence, self.source_system, channels)
                self.sock.sendall(frame)
                self.sequence = (self.sequence + 1) & 0xFF
                next_send = now + self.period_s
            elif mode == "off":
                next_send = now
            time.sleep(0.002)


class MspIngress(PeriodicIngress):
    def __init__(self, name: str, client: MspClient, channels: list[int]) -> None:
        super().__init__(name, channels, period_s=0.05)
        self.client = client

    def _run(self) -> None:
        next_send = time.monotonic()
        while not self._stop.is_set():
            now = time.monotonic()
            with self._lock:
                mode = self.mode
                channels = list(self.channels)
            if mode != "off" and now >= next_send:
                self.client.set_raw_rc(channels if mode == "full" else channels[:2])
                next_send = now + self.period_s
            elif mode == "off":
                next_send = now
            time.sleep(0.002)


class CrsfIngress:
    def __init__(self, receiver: VirtualCrsfReceiver) -> None:
        self.receiver = receiver
        self.name = receiver.name
        self.channels = receiver.channels_us

    def set_mode(self, mode: str) -> None:
        if mode == "partial":
            raise ValueError("CRSF partial mode is not supported")
        self.receiver.set_rc(mode == "full")

    def check(self) -> None:
        self.receiver.check()

    def close(self) -> None:
        self.receiver.close()


def assert_direct_rc(msp: MspClient, channel_index: int, expected: int, label: str, tolerance: int = 3) -> None:
    channels = msp.rc_channels()
    actual = channels[channel_index]
    if abs(actual - expected) > tolerance:
        raise TestFailure(f"{label}: channel {channel_index} expected ~{expected}, got {actual}")


def wait_direct_rc(msp: MspClient, channel_index: int, expected: int, label: str, timeout_s: float = 1.0) -> None:
    deadline = time.monotonic() + timeout_s
    actual = -1
    while time.monotonic() < deadline:
        channels = msp.rc_channels()
        actual = channels[channel_index]
        if abs(actual - expected) <= 3:
            return
        time.sleep(0.02)
    raise TestFailure(f"{label}: channel {channel_index} expected ~{expected}, got {actual}")


def assert_ingress_signature(msp: MspClient, ingress, label: str) -> None:
    if isinstance(ingress, CrsfIngress):
        assert_rc_signature(msp, 0, ingress.channels[0], label)
    else:
        assert_direct_rc(msp, 0, ingress.channels[0], label)


def exercise_selector(msp: MspClient, rx1, rx2, name: str) -> None:
    receivers = (rx1, rx2)
    rx1.set_mode("full")
    wait_status(msp, lambda s: s.valid_mask == 0x01 and s.active == RX1, f"{name} RX1 initial", receivers)
    assert_ingress_signature(msp, rx1, f"{name} RX1 source")

    rx2.set_mode("full")
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX1, f"{name} both valid", receivers)
    assert_ingress_signature(msp, rx1, f"{name} inactive RX2 isolation")
    wait_mode(msp, BOX_BEEPER, False, f"{name} inactive RX2 AUX isolation", receivers)

    rx1.set_mode("off")
    wait_status(msp, lambda s: s.valid_mask == 0x02 and s.active == RX2, f"{name} failover to RX2", receivers)
    assert_ingress_signature(msp, rx2, f"{name} RX2 source")
    wait_mode(msp, BOX_BEEPER, True, f"{name} RX2 AUX after failover", receivers)

    rx1.set_mode("full")
    wait_status(msp, lambda s: s.valid_mask == 0x03 and s.active == RX2, f"{name} RX1 recovery", receivers)
    assert_ingress_signature(msp, rx2, f"{name} no failback")

    rx2.set_mode("off")
    wait_status(msp, lambda s: s.valid_mask == 0x01 and s.active == RX1, f"{name} reverse failover", receivers)
    assert_ingress_signature(msp, rx1, f"{name} reverse source")
    wait_mode(msp, BOX_BEEPER, False, f"{name} RX1 AUX after reverse failover", receivers)
    print(f"[PASS] {name} preserves selector, publication, recovery, and AUX authority semantics")


def exercise_partial_liveness(msp: MspClient, active, tested, name: str) -> None:
    receivers = (active, tested)
    active.set_mode("full")
    tested.set_mode("full")
    wait_status(msp, lambda s: s.valid_mask == 0x03, f"{name} full frame establishes liveness", receivers)

    tested.set_mode("off")
    expected_mask = 0x01 if tested is not active else 0x02
    wait_status(msp, lambda s: s.valid_mask == expected_mask, f"{name} full-frame liveness expires", receivers)
    tested.set_mode("partial")
    time.sleep(0.4)
    status = msp.link_status()
    if status.valid_mask != expected_mask:
        raise TestFailure(f"{name}: partial/AUX-only traffic kept flight axes alive: {status}")
    assert_ingress_signature(msp, active, f"{name} partial inactive publication isolation")
    wait_mode(msp, BOX_BEEPER, False, f"{name} partial inactive AUX isolation", receivers)
    print(f"[PASS] {name} partial/AUX-only messages do not establish or preserve RX liveness")
    tested.set_mode("off")


def exercise_legacy_msp_override(msp: MspClient, rx1, rx2) -> None:
    receivers = (rx1, rx2)
    rx1.set_mode("full")
    rx2.set_mode("off")
    wait_status(msp, lambda s: s.valid_mask == 0x01 and s.active == RX1, "legacy MSP override RX1 baseline", receivers)
    wait_mode(msp, BOX_MSP_RC_OVERRIDE, True, "legacy MSP override mode active", receivers)
    assert_ingress_signature(msp, rx1, "legacy MSP override baseline source")

    override_channels = [1950, 1500, 1200, 1500] + [1100] * 12
    override = MspIngress("legacy-MSP-override", msp, override_channels)
    override.start()
    try:
        override.set_mode("full")
        wait_direct_rc(msp, 0, override_channels[0], "legacy MSP override applies configured channel")
        override.set_mode("off")
        # Legacy MSP override uses PERIOD_RXDATA_FAILURE plus the configured
        # failsafe_delay (700 ms with defaults) before relinquishing authority.
        time.sleep(0.9)
        assert_ingress_signature(msp, rx1, "legacy MSP override expiry restores receiver")
    finally:
        override.close()
    print("[PASS] Legacy MSP override remains mode/mask-scoped when MSP is not a configured receiver")


def configure_case(kind: str) -> list[str]:
    commands = [
        "set dual_rx_enabled = ON",
        "set mavlink_sysid = 1",
        "aux 0 13 0 1700 2100",
        "feature TELEMETRY",
    ]
    if kind == "crsf-mavlink":
        commands += [
            "set receiver_type = SERIAL",
            "set serialrx_provider = CRSF",
            "set receiver_type_rx2 = SERIAL",
            "set serialrx_provider_rx2 = MAVLINK",
            f"serial 2 {FUNCTION_RX_SERIAL}",
            f"serial 3 {FUNCTION_RX_SERIAL_SECONDARY | FUNCTION_TELEMETRY_MAVLINK}",
            "set msp_override_channels = 1",
            "aux 1 50 1 1700 2100",
        ]
    elif kind == "crsf-msp":
        commands += [
            "set receiver_type = SERIAL",
            "set serialrx_provider = CRSF",
            "set receiver_type_rx2 = MSP",
            f"serial 2 {FUNCTION_RX_SERIAL}",
        ]
    elif kind == "mavlink-mavlink":
        commands += [
            "set receiver_type = SERIAL",
            "set serialrx_provider = MAVLINK",
            "set receiver_type_rx2 = SERIAL",
            "set serialrx_provider_rx2 = MAVLINK",
            f"serial 2 {FUNCTION_RX_SERIAL | FUNCTION_TELEMETRY_MAVLINK}",
            f"serial 3 {FUNCTION_RX_SERIAL_SECONDARY | FUNCTION_TELEMETRY_MAVLINK}",
        ]
    else:
        raise ValueError(kind)
    return commands


def sitl_log_count(sitl: SitlProcess, marker: str) -> int:
    try:
        text = sitl.log_path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return 0
    return text.count(marker)


def wait_for_sitl_reset(sitl: SitlProcess, reset_count_before: int, timeout_s: float = 8.0) -> None:
    """Wait until CLI `save` has persisted configuration and entered reset."""
    deadline = time.monotonic() + timeout_s
    last_reset_count = reset_count_before
    while time.monotonic() < deadline:
        sitl.check_alive()
        last_reset_count = sitl_log_count(sitl, "[SYSTEM] Reset")
        if last_reset_count > reset_count_before:
            return
        time.sleep(0.02)
    raise TestFailure(
        "SITL did not enter CLI save/reset cycle within "
        f"{timeout_s:.1f}s (reset {reset_count_before}->{last_reset_count})\n"
        f"{sitl.tail_log(lines=80)}"
    )


def hard_restart_sitl_after_configuration(sitl: SitlProcess, reset_count_before: int) -> None:
    """Replace SITL's in-process exec reboot with a clean harness restart.

    The mixed serial layout opens UARTs lazily. During the exec-based reboot a
    TCP listener can survive long enough for the new instance to hit EADDRINUSE,
    leaving a receiver connected to the dying listener. Once `save` has reached
    systemReset(), terminate that process completely and start a fresh one from
    the saved EEPROM so every UART listener is recreated from a clean process.
    """
    wait_for_sitl_reset(sitl, reset_count_before)
    sitl.stop()
    time.sleep(0.05)
    sitl.start()


def run_case(kind: str, binary: Path, repo: Path, tcp_base: int, temp_dir: Path) -> None:
    eeprom = temp_dir / f"{kind}.bin"
    log_path = temp_dir / f"{kind}.log"
    sitl = SitlProcess(binary, repo, eeprom, tcp_base, log_path)
    msp: MspClient | None = None
    ingresses: list = []
    print(f"\n[CASE] {kind}")
    try:
        sitl.start()
        msp_port = tcp_port(tcp_base, UART_MSP)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        time.sleep(0.15)
        reset_count = sitl_log_count(sitl, "[SYSTEM] Reset")
        configure_cli("127.0.0.1", msp_port, configure_case(kind))
        hard_restart_sitl_after_configuration(sitl, reset_count)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)

        msp = MspClient("127.0.0.1", msp_port, timeout_s=2.0)
        status = msp.link_status()
        if status.dual_status != RX_DUAL_STATUS_OK or status.initialized_mask != 0x03:
            raise TestFailure(f"{kind}: pair did not initialize: {status}")

        rx1_channels = [1300, 1400, 1200, 1500] + [1100] * 12
        rx2_channels = [1800, 1600, 1700, 1500] + [1900] * 12
        if kind == "crsf-mavlink":
            rx1_channels[5] = 1900
            rx2_channels[5] = 1900

        if kind.startswith("crsf-"):
            crsf = VirtualCrsfReceiver("RX1-CRSF", "127.0.0.1", tcp_port(tcp_base, UART_RX1), rx1_channels)
            crsf.connect()
            rx1 = CrsfIngress(crsf)
        else:
            rx1 = MavlinkIngress("RX1-MAVLink", "127.0.0.1", tcp_port(tcp_base, UART_RX1), rx1_channels, 241)
            rx1.start()

        if kind == "crsf-msp":
            rx2 = MspIngress("RX2-MSP", msp, rx2_channels)
            rx2.start()
        else:
            rx2 = MavlinkIngress("RX2-MAVLink", "127.0.0.1", tcp_port(tcp_base, UART_RX2), rx2_channels, 242)
            rx2.start()
        ingresses = [rx1, rx2]

        if isinstance(rx2, (MavlinkIngress, MspIngress)):
            exercise_partial_liveness(msp, rx1, rx2, f"{rx2.name} partial-frame safety")
        if kind == "crsf-mavlink":
            exercise_legacy_msp_override(msp, rx1, rx2)
        exercise_selector(msp, rx1, rx2, kind.replace("-", " + ").upper())
    except Exception as exc:
        process_code = sitl.process.poll() if sitl.process is not None else None
        sitl_state = "still running" if sitl.process is not None and process_code is None else f"exited with code {process_code}"
        log_tail = sitl.tail_log(lines=120)
        raise TestFailure(
            f"{kind} failed: {type(exc).__name__}: {exc}\n"
            f"SITL state: {sitl_state}\n"
            f"SITL log: {log_path}\n"
            f"EEPROM: {eeprom}\n"
            f"--- SITL LOG TAIL ---\n{log_tail}\n--- END SITL LOG TAIL ---"
        ) from exc
    finally:
        for ingress in reversed(ingresses):
            ingress.close()
        if msp is not None:
            msp.close()
        sitl.stop()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Exercise mixed INAV Dual RX ingress paths in SITL.")
    parser.add_argument("--repo", help="INAV repository root")
    parser.add_argument("--sitl", help="SITL executable")
    parser.add_argument("--tcp-base", type=int, default=DEFAULT_TCP_BASE, help=f"SITL TCP base port (default {DEFAULT_TCP_BASE})")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    script_path = Path(__file__).resolve()
    repo = discover_repo(script_path, args.repo)
    binary = discover_sitl(repo, args.sitl)
    print("Dual RX mixed-ingress SITL test")
    print(f"  repo: {repo}")
    print(f"  SITL: {binary}\n")

    temp_dir = Path(tempfile.mkdtemp(prefix="inav-dualrx-mixed-"))
    try:
        for kind in ("crsf-mavlink", "crsf-msp", "mavlink-mavlink"):
            run_case(kind, binary, repo, args.tcp_base, temp_dir)
    except Exception:
        print(f"\n[FAIL] Full diagnostics retained in: {temp_dir}")
        traceback.print_exc()
        return 1

    shutil.rmtree(temp_dir, ignore_errors=True)
    print("\nALL DUAL RX MIXED-INGRESS SITL TESTS PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

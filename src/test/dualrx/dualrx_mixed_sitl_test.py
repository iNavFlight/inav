#!/usr/bin/env python3
"""Exercise live mixed Dual RX ingress paths against INAV SITL."""

from __future__ import annotations

import argparse
from pathlib import Path
import tempfile
import threading
import time

from pymavlink import mavutil

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


class PeriodicIngress:
    def __init__(self, name: str, channels: list[int], period_s: float = 0.02) -> None:
        self.name = name
        self.channels = list(channels)
        self.period_s = period_s
        self.mode = "off"
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None
        self.error: BaseException | None = None

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
            raise TestFailure(f"{self.name} background thread failed: {self.error}")

    def _run_guarded(self) -> None:
        try:
            self._run()
        except BaseException as exc:
            if not self._stop.is_set():
                self.error = exc
                self._stop.set()

    def _run(self) -> None:
        raise NotImplementedError


class MavlinkIngress(PeriodicIngress):
    def __init__(self, name: str, host: str, port: int, channels: list[int], source_system: int) -> None:
        super().__init__(name, channels)
        self.connection = mavutil.mavlink_connection(
            f"tcp:{host}:{port}",
            source_system=source_system,
            source_component=191,
            autoreconnect=False,
        )

    def close(self) -> None:
        super().close()
        self.connection.close()

    def _run(self) -> None:
        next_send = time.monotonic()
        while not self._stop.is_set():
            now = time.monotonic()
            with self._lock:
                mode = self.mode
                channels = list(self.channels)
            if mode != "off" and now >= next_send:
                if mode == "partial":
                    channels[:4] = [0xFFFF] * 4
                self.connection.mav.rc_channels_override_send(1, 1, *channels[:8])
                next_send = now + self.period_s
            elif mode == "off":
                next_send = now
            self.connection.recv_match(blocking=False)
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
    commands = ["set dual_rx_enabled = ON", "aux 0 13 0 1700 2100", "feature TELEMETRY"]
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


def run_case(kind: str, binary: Path, repo: Path, tcp_base: int, temp_dir: Path) -> None:
    eeprom = temp_dir / f"{kind}.bin"
    sitl = SitlProcess(binary, repo, eeprom, tcp_base, temp_dir / f"{kind}.log")
    msp: MspClient | None = None
    ingresses: list = []
    try:
        sitl.start()
        msp_port = tcp_port(tcp_base, UART_MSP)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        time.sleep(0.15)
        configure_cli("127.0.0.1", msp_port, configure_case(kind))
        time.sleep(1.0)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(tcp_base, UART_RX1), 8.0, sitl)
        if kind != "crsf-msp":
            wait_tcp("127.0.0.1", tcp_port(tcp_base, UART_RX2), 8.0, sitl)

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
    try:
        with tempfile.TemporaryDirectory(prefix="inav-dualrx-mixed-") as temp_name:
            temp_dir = Path(temp_name)
            for kind in ("crsf-mavlink", "crsf-msp", "mavlink-mavlink"):
                run_case(kind, binary, repo, args.tcp_base, temp_dir)
    except BaseException as exc:
        print(f"\n[FAIL] {exc}")
        return 1
    print("\nALL DUAL RX MIXED-INGRESS SITL TESTS PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Exercise Dual RX pair capability gating against a real INAV SITL boot."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import signal
import tempfile
import time

from dualrx_sitl_test import (
    DEFAULT_TCP_BASE,
    FUNCTION_MSP,
    FUNCTION_RX_SERIAL,
    FUNCTION_RX_SERIAL_SECONDARY,
    MspClient,
    RX_DUAL_STATUS_OK,
    SitlProcess,
    TestFailure,
    configure_cli,
    discover_repo,
    discover_sitl,
    tcp_port,
    wait_tcp,
    UART_MSP,
)


FUNCTION_TELEMETRY_MAVLINK = 1 << 8
RX_DUAL_STATUS_UNSUPPORTED_PAIR = 4


@dataclass(frozen=True)
class PairCase:
    name: str
    receiver_type_1: str
    receiver_type_2: str
    provider_1: str | None
    provider_2: str | None
    accepted: bool


PAIR_CASES = (
    PairCase("CRSF + CRSF", "SERIAL", "SERIAL", "CRSF", "CRSF", True),
    PairCase("SBUS + SBUS2", "SERIAL", "SERIAL", "SBUS", "SBUS2", True),
    PairCase("MAVLink + MAVLink", "SERIAL", "SERIAL", "MAVLINK", "MAVLINK", True),
    PairCase("MSP + MSP", "MSP", "MSP", None, None, False),
    PairCase("SIM + SIM", "SIM (SITL)", "SIM (SITL)", None, None, False),
    PairCase("FPORT + FPORT", "SERIAL", "SERIAL", "FPORT", "FPORT", False),
    PairCase("FPORT + FPORT2", "SERIAL", "SERIAL", "FPORT", "FPORT2", False),
    PairCase("FPORT2 + FBUS", "SERIAL", "SERIAL", "FPORT2", "FBUS", False),
    PairCase("CRSF + SBUS", "SERIAL", "SERIAL", "CRSF", "SBUS", True),
)


def serial_mask(provider: str | None, secondary: bool) -> int:
    mask = FUNCTION_RX_SERIAL_SECONDARY if secondary else FUNCTION_RX_SERIAL
    if provider == "MAVLINK":
        mask |= FUNCTION_TELEMETRY_MAVLINK
    return mask


def case_commands(case: PairCase) -> list[str]:
    commands = [
        "set dual_rx_enabled = ON",
        f"set receiver_type = {case.receiver_type_1}",
        f"set receiver_type_rx2 = {case.receiver_type_2}",
    ]
    if case.provider_1 is not None:
        commands.append(f"set serialrx_provider = {case.provider_1}")
    if case.provider_2 is not None:
        commands.append(f"set serialrx_provider_rx2 = {case.provider_2}")
    if case.receiver_type_1 == "SERIAL":
        commands.append(f"serial 2 {serial_mask(case.provider_1, False)}")
    if case.receiver_type_2 == "SERIAL":
        commands.append(f"serial 3 {serial_mask(case.provider_2, True)}")
    return commands


def run_case(case: PairCase, binary: Path, repo: Path, tcp_base: int, temp_dir: Path) -> None:
    slug = case.name.lower().replace(" + ", "-").replace(" ", "-")
    eeprom = temp_dir / f"{slug}.bin"
    log_path = temp_dir / f"{slug}.log"
    sitl = SitlProcess(binary, repo, eeprom, tcp_base, log_path)
    try:
        sitl.start()
        msp_port = tcp_port(tcp_base, UART_MSP)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        time.sleep(0.15)
        configure_cli("127.0.0.1", msp_port, case_commands(case))

        time.sleep(1.0)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        time.sleep(0.1)
        with MspClient("127.0.0.1", msp_port, timeout_s=2.0) as msp:
            status = msp.link_status()

        if status.configured_mask != 0x03:
            raise TestFailure(f"{case.name}: configured mask {status.configured_mask:#04x}, expected 0x03")

        if case.accepted:
            if status.dual_status != RX_DUAL_STATUS_OK or status.initialized_mask != 0x03:
                raise TestFailure(
                    f"{case.name}: expected accepted/initialized pair, "
                    f"status={status.dual_status} initialized={status.initialized_mask:#04x}"
                )
            print(f"[PASS] {case.name} accepted and both receiver instances initialized")
        else:
            if status.dual_status != RX_DUAL_STATUS_UNSUPPORTED_PAIR:
                raise TestFailure(
                    f"{case.name}: expected UNSUPPORTED_PAIR({RX_DUAL_STATUS_UNSUPPORTED_PAIR}), "
                    f"got {status.dual_status}"
                )
            print(f"[PASS] {case.name} rejected as unsupported while configuration remains visible")
    finally:
        sitl.stop()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Exercise INAV Dual RX pair acceptance/rejection in SITL.")
    parser.add_argument("--repo", help="INAV repository root")
    parser.add_argument("--sitl", help="SITL executable")
    parser.add_argument("--tcp-base", type=int, default=DEFAULT_TCP_BASE, help=f"SITL TCP base port (default {DEFAULT_TCP_BASE})")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    script_path = Path(__file__).resolve()
    repo = discover_repo(script_path, args.repo)
    binary = discover_sitl(repo, args.sitl)

    print("Dual RX SITL configuration matrix")
    print(f"  repo: {repo}")
    print(f"  SITL: {binary}")
    print()

    signal.signal(signal.SIGINT, signal.default_int_handler)
    try:
        with tempfile.TemporaryDirectory(prefix="inav-dualrx-config-") as temp_name:
            temp_dir = Path(temp_name)
            for case in PAIR_CASES:
                run_case(case, binary, repo, args.tcp_base, temp_dir)
    except BaseException as exc:
        print(f"\n[FAIL] {exc}")
        return 1

    print("\nALL DUAL RX CONFIGURATION MATRIX TESTS PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

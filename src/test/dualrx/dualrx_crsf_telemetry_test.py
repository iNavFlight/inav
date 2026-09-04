#!/usr/bin/env python3
"""Verify per-link CRSF telemetry and MSP-over-CRSF reply ownership in SITL."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import tempfile
import time

from dualrx_sitl_test import (
    DEFAULT_TCP_BASE,
    FUNCTION_RX_SERIAL,
    FUNCTION_RX_SERIAL_SECONDARY,
    MSP2_INAV_GET_LINK_STATS,
    RX1,
    RX2,
    RX_DUAL_STATUS_OK,
    SitlProcess,
    TestFailure,
    UART_MSP,
    UART_RX1,
    UART_RX2,
    VirtualCrsfReceiver,
    configure_cli,
    crsf_frame,
    crc8_dvb_s2,
    discover_repo,
    discover_sitl,
    tcp_port,
    wait_status,
    wait_tcp,
    MspClient,
)


CRSF_FRAMETYPE_MSP_REQ = 0x7A
CRSF_FRAMETYPE_MSP_RESP = 0x7B
CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8
RX1_ORIGIN = 0xEA
RX2_ORIGIN = 0xEE


class CrsfStreamDecoder:
    def __init__(self) -> None:
        self.buffer = bytearray()

    def feed(self, data: bytes) -> list[bytes]:
        self.buffer.extend(data)
        frames: list[bytes] = []
        while True:
            try:
                start = self.buffer.index(CRSF_ADDRESS_FLIGHT_CONTROLLER)
            except ValueError:
                self.buffer.clear()
                break
            if start:
                del self.buffer[:start]
            if len(self.buffer) < 2:
                break
            total = self.buffer[1] + 2
            if total < 4 or total > 64:
                del self.buffer[0]
                continue
            if len(self.buffer) < total:
                break
            frame = bytes(self.buffer[:total])
            del self.buffer[:total]
            if crc8_dvb_s2(frame[2:-1]) != frame[-1]:
                continue
            frames.append(frame)
        return frames


@dataclass
class MspTelemetryResponse:
    destination: int
    origin: int
    command: int
    payload: bytes


class MspResponseAssembler:
    def __init__(self) -> None:
        self.expected_size: int | None = None
        self.command = 0
        self.destination = 0
        self.origin = 0
        self.payload = bytearray()
        self.last_sequence: int | None = None

    def feed(self, frame: bytes) -> MspTelemetryResponse | None:
        if frame[2] != CRSF_FRAMETYPE_MSP_RESP:
            return None
        destination = frame[3]
        origin = frame[4]
        chunk = frame[5:-1]
        if not chunk:
            return None
        status = chunk[0]
        sequence = status & 0x0F
        if status & 0x10:
            if len(chunk) < 6:
                raise TestFailure(f"short MSP-over-CRSF response header: {chunk.hex()}")
            version = (status & 0x60) >> 5
            if version != 2:
                raise TestFailure(f"unexpected MSP-over-CRSF version {version}")
            self.destination = destination
            self.origin = origin
            self.command = chunk[2] | (chunk[3] << 8)
            self.expected_size = chunk[4] | (chunk[5] << 8)
            self.payload = bytearray(chunk[6:])
        else:
            if self.expected_size is None or self.last_sequence is None:
                raise TestFailure("MSP-over-CRSF continuation arrived without a response start")
            if sequence != ((self.last_sequence + 1) & 0x0F):
                raise TestFailure(f"MSP-over-CRSF response sequence jumped {self.last_sequence}->{sequence}")
            self.payload.extend(chunk[1:])
        self.last_sequence = sequence

        if self.expected_size is None or len(self.payload) < self.expected_size:
            return None
        response = MspTelemetryResponse(
            destination=self.destination,
            origin=self.origin,
            command=self.command,
            payload=bytes(self.payload[:self.expected_size]),
        )
        self.expected_size = None
        self.payload.clear()
        self.last_sequence = None
        return response


def msp_v2_crsf_request(command: int, origin: int, payload: bytes = b"", sequence: int = 0) -> bytes:
    status = 0x10 | (2 << 5) | (sequence & 0x0F)
    chunk = bytes([
        status,
        0,
        command & 0xFF,
        (command >> 8) & 0xFF,
        len(payload) & 0xFF,
        (len(payload) >> 8) & 0xFF,
    ]) + payload
    return crsf_frame(
        CRSF_FRAMETYPE_MSP_REQ,
        bytes([CRSF_ADDRESS_FLIGHT_CONTROLLER, origin]) + chunk,
    )


def incomplete_msp_v2_crsf_request(command: int, origin: int, declared_size: int) -> bytes:
    status = 0x10 | (2 << 5)
    chunk = bytes([
        status,
        0,
        command & 0xFF,
        (command >> 8) & 0xFF,
        declared_size & 0xFF,
        (declared_size >> 8) & 0xFF,
    ])
    return crsf_frame(
        CRSF_FRAMETYPE_MSP_REQ,
        bytes([CRSF_ADDRESS_FLIGHT_CONTROLLER, origin]) + chunk,
    )


def clear_received(receivers: tuple[VirtualCrsfReceiver, VirtualCrsfReceiver]) -> None:
    for receiver in receivers:
        receiver.take_received()


def collect_responses(
    receivers: tuple[VirtualCrsfReceiver, VirtualCrsfReceiver],
    expected_links: set[int],
    timeout_s: float = 2.0,
) -> tuple[dict[int, MspTelemetryResponse], list[int]]:
    decoders = (CrsfStreamDecoder(), CrsfStreamDecoder())
    assemblers = (MspResponseAssembler(), MspResponseAssembler())
    completed: dict[int, MspTelemetryResponse] = {}
    response_frame_links: list[int] = []
    deadline = time.monotonic() + timeout_s
    while time.monotonic() < deadline:
        for link, receiver in enumerate(receivers):
            receiver.check()
            for frame in decoders[link].feed(receiver.take_received()):
                if frame[2] != CRSF_FRAMETYPE_MSP_RESP:
                    continue
                response_frame_links.append(link)
                response = assemblers[link].feed(frame)
                if response is not None:
                    completed[link] = response
        if expected_links.issubset(completed):
            # Observe another telemetry interval so a misrouted duplicate is
            # not hidden merely because the expected response completed first.
            time.sleep(0.15)
            for link, receiver in enumerate(receivers):
                for frame in decoders[link].feed(receiver.take_received()):
                    if frame[2] == CRSF_FRAMETYPE_MSP_RESP:
                        response_frame_links.append(link)
            return completed, response_frame_links
        time.sleep(0.005)
    raise TestFailure(f"timeout waiting for MSP-over-CRSF responses on links {sorted(expected_links)}; got {completed}")


def assert_response(response: MspTelemetryResponse, destination: int) -> None:
    if response.destination != destination or response.origin != CRSF_ADDRESS_FLIGHT_CONTROLLER:
        raise TestFailure(f"wrong CRSF response routing header: {response}")
    if response.command != MSP2_INAV_GET_LINK_STATS or len(response.payload) < 65:
        raise TestFailure(f"wrong MSP response command/size: command={response.command:#06x} size={len(response.payload)}")


def run_test(binary: Path, repo: Path, tcp_base: int, temp_dir: Path) -> None:
    eeprom = temp_dir / "crsf-telemetry.bin"
    sitl = SitlProcess(binary, repo, eeprom, tcp_base, temp_dir / "crsf-telemetry.log")
    msp: MspClient | None = None
    rx1: VirtualCrsfReceiver | None = None
    rx2: VirtualCrsfReceiver | None = None
    try:
        sitl.start()
        msp_port = tcp_port(tcp_base, UART_MSP)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        time.sleep(0.15)
        configure_cli("127.0.0.1", msp_port, [
            "feature TELEMETRY",
            "set receiver_type = SERIAL",
            "set serialrx_provider = CRSF",
            "set serialrx_halfduplex = OFF",
            "set dual_rx_enabled = ON",
            "set receiver_type_rx2 = SERIAL",
            "set serialrx_provider_rx2 = CRSF",
            "set serialrx_halfduplex_rx2 = OFF",
            f"serial 2 {FUNCTION_RX_SERIAL}",
            f"serial 3 {FUNCTION_RX_SERIAL_SECONDARY}",
        ])
        time.sleep(1.0)
        wait_tcp("127.0.0.1", msp_port, 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(tcp_base, UART_RX1), 8.0, sitl)
        wait_tcp("127.0.0.1", tcp_port(tcp_base, UART_RX2), 8.0, sitl)

        channels1 = [1300, 1400, 1200, 1500] + [1100] * 12
        channels2 = [1800, 1600, 1700, 1500] + [1900] * 12
        rx1 = VirtualCrsfReceiver("RX1", "127.0.0.1", tcp_port(tcp_base, UART_RX1), channels1)
        rx2 = VirtualCrsfReceiver("RX2", "127.0.0.1", tcp_port(tcp_base, UART_RX2), channels2)
        rx1.connect()
        rx2.connect()
        rx1.set_rc(True)
        rx2.set_rc(True)
        receivers = (rx1, rx2)

        msp = MspClient("127.0.0.1", msp_port, timeout_s=2.0)
        status = wait_status(msp, lambda s: s.valid_mask == 0x03, "both CRSF links valid", receivers)
        if status.dual_status != RX_DUAL_STATUS_OK or status.initialized_mask != 0x03:
            raise TestFailure(f"dual CRSF telemetry pair did not initialize: {status}")

        time.sleep(0.2)
        clear_received(receivers)
        rx1.send_fragmented(msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX1_ORIGIN), chunk_size=64, inter_chunk_s=0)
        responses, links = collect_responses(receivers, {RX1})
        assert_response(responses[RX1], RX1_ORIGIN)
        if RX2 in links:
            raise TestFailure("RX1-originated MSP response appeared on RX2")
        print("[PASS] RX1 MSP-over-CRSF request replies only through RX1")

        clear_received(receivers)
        rx2.send_fragmented(msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX2_ORIGIN), chunk_size=64, inter_chunk_s=0)
        responses, links = collect_responses(receivers, {RX2})
        assert_response(responses[RX2], RX2_ORIGIN)
        if RX1 in links:
            raise TestFailure("RX2-originated MSP response appeared on RX1")
        print("[PASS] RX2 MSP-over-CRSF request replies only through RX2")

        clear_received(receivers)
        rx1.send_fragmented(msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX1_ORIGIN), chunk_size=64, inter_chunk_s=0)
        rx2.send_fragmented(msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX2_ORIGIN), chunk_size=64, inter_chunk_s=0)
        responses, _ = collect_responses(receivers, {RX1, RX2})
        assert_response(responses[RX1], RX1_ORIGIN)
        assert_response(responses[RX2], RX2_ORIGIN)
        print("[PASS] Simultaneous MSP-over-CRSF requests retain independent endpoint ownership")

        clear_received(receivers)
        rx1.send_fragmented(incomplete_msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX1_ORIGIN, 1), chunk_size=64, inter_chunk_s=0)
        rx2.send_fragmented(msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX2_ORIGIN), chunk_size=64, inter_chunk_s=0)
        responses, links = collect_responses(receivers, {RX2})
        assert_response(responses[RX2], RX2_ORIGIN)
        if RX1 in links:
            raise TestFailure("incomplete RX1 transaction emitted an MSP response")
        print("[PASS] Interrupted RX1 request cannot block or steal RX2 reply routing")

        clear_received(receivers)
        rx1.send_fragmented(msp_v2_crsf_request(MSP2_INAV_GET_LINK_STATS, RX1_ORIGIN), chunk_size=64, inter_chunk_s=0)
        responses, _ = collect_responses(receivers, {RX1})
        assert_response(responses[RX1], RX1_ORIGIN)
        print("[PASS] RX1 accepts a fresh request after an interrupted transaction")
    finally:
        if msp is not None:
            msp.close()
        if rx1 is not None:
            rx1.close()
        if rx2 is not None:
            rx2.close()
        sitl.stop()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Exercise Dual RX CRSF telemetry endpoint ownership in SITL.")
    parser.add_argument("--repo", help="INAV repository root")
    parser.add_argument("--sitl", help="telemetry-enabled SITL executable")
    parser.add_argument("--tcp-base", type=int, default=DEFAULT_TCP_BASE, help=f"SITL TCP base port (default {DEFAULT_TCP_BASE})")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    script_path = Path(__file__).resolve()
    repo = discover_repo(script_path, args.repo)
    binary = discover_sitl(repo, args.sitl)
    print("Dual RX CRSF telemetry ownership test")
    print(f"  repo: {repo}")
    print(f"  SITL: {binary}\n")
    try:
        with tempfile.TemporaryDirectory(prefix="inav-dualrx-crsf-telemetry-") as temp_name:
            run_test(binary, repo, args.tcp_base, Path(temp_name))
    except BaseException as exc:
        print(f"\n[FAIL] {exc}")
        return 1
    print("\nALL DUAL RX CRSF TELEMETRY OWNERSHIP TESTS PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

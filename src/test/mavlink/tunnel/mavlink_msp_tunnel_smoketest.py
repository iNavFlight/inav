#!/usr/bin/env python3
"""
Usage:
  python src/test/mavlink/tunnel/mavlink_msp_tunnel_smoketest.py --mavlink-endpoint tcp:127.0.0.1:5761
  python src/test/mavlink/tunnel/mavlink_msp_tunnel_smoketest.py --mavlink-endpoint tcp:127.0.0.1:5761 --target-system 1 --post-reboot-timeout 20
"""

from __future__ import annotations

import argparse
import os
import struct
import time
from dataclasses import dataclass
from typing import Iterable, List, Optional, Sequence

os.environ["MAVLINK20"] = "1"

from pymavlink import mavutil


MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP = 0x8001
MAVLINK_CONFIGURATOR_SYSTEM_ID = 253
MAVLINK_CONFIGURATOR_COMPONENT_ID = 25
MAVLINK_TUNNEL_CHUNK_SIZE = 128

MSP_API_VERSION = 1
MSP_FC_VARIANT = 2
MSP_FC_VERSION = 3
MSP_BUILD_INFO = 5
MSP_EEPROM_WRITE = 250
MSP_REBOOT = 68

MSP_DIRECTION_TO_FC = ord("<")
MSP_DIRECTION_FROM_FC = ord(">")
MSP_DIRECTION_ERROR = ord("!")
MSP_V1_JUMBO_PAYLOAD_MARKER = 255


def msp_v1_checksum(data: bytes) -> int:
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum


def build_msp_v1_request(cmd: int, payload: bytes = b"") -> bytes:
    payload_length = len(payload)
    header = bytearray(b"$M<")
    if payload_length >= MSP_V1_JUMBO_PAYLOAD_MARKER:
        header.extend((MSP_V1_JUMBO_PAYLOAD_MARKER, cmd))
        header.extend(struct.pack("<H", payload_length))
    else:
        header.extend((payload_length, cmd))
    checksum = msp_v1_checksum(bytes(header[3:]) + payload)
    return bytes(header) + payload + bytes((checksum,))


@dataclass
class MspV1Reply:
    cmd: int
    payload: bytes
    is_error: bool


class MspV1Parser:
    def __init__(self) -> None:
        self.buffer = bytearray()

    def clear(self) -> None:
        self.buffer.clear()

    def feed(self, data: bytes) -> List[MspV1Reply]:
        self.buffer.extend(data)
        replies: List[MspV1Reply] = []

        while True:
            frame_start = self.buffer.find(b"$M")
            if frame_start < 0:
                self.buffer.clear()
                return replies
            if frame_start > 0:
                del self.buffer[:frame_start]

            if len(self.buffer) < 6:
                return replies

            direction = self.buffer[2]
            if direction not in (MSP_DIRECTION_FROM_FC, MSP_DIRECTION_ERROR):
                del self.buffer[:2]
                continue

            payload_length = self.buffer[3]
            cmd = self.buffer[4]
            frame_header_length = 5

            if payload_length == MSP_V1_JUMBO_PAYLOAD_MARKER:
                if len(self.buffer) < 8:
                    return replies
                payload_length = struct.unpack_from("<H", self.buffer, 5)[0]
                frame_header_length = 7

            frame_length = frame_header_length + payload_length + 1
            if len(self.buffer) < frame_length:
                return replies

            frame = bytes(self.buffer[:frame_length])
            payload = frame[frame_header_length:-1]
            expected_checksum = frame[-1]
            actual_checksum = msp_v1_checksum(frame[3:-1])
            del self.buffer[:frame_length]

            if actual_checksum != expected_checksum:
                continue

            replies.append(
                MspV1Reply(
                    cmd=cmd,
                    payload=payload,
                    is_error=direction == MSP_DIRECTION_ERROR,
                )
            )


def decode_api_version(payload: bytes) -> str:
    msp_protocol_version, api_major, api_minor = struct.unpack("<BBB", payload)
    return (
        f"msp_protocol_version={msp_protocol_version} "
        f"api_version={api_major}.{api_minor}"
    )


def decode_fc_variant(payload: bytes) -> str:
    variant = payload.rstrip(b"\x00").decode("ascii", errors="ignore")
    return f"fc_variant={variant}"


def decode_fc_version(payload: bytes) -> str:
    major, minor, patch = struct.unpack("<BBB", payload)
    return f"fc_version={major}.{minor}.{patch}"


def decode_build_info(payload: bytes) -> str:
    build_date, build_time, git_revision = struct.unpack("<11s8s8s", payload)
    build_date_text = build_date.rstrip(b" ").decode("ascii", errors="ignore")
    build_time_text = build_time.rstrip(b" ").decode("ascii", errors="ignore")
    git_revision_text = git_revision.rstrip(b"\x00").decode("ascii", errors="ignore")
    return (
        f"build_date={build_date_text} "
        f"build_time={build_time_text} "
        f"git_revision={git_revision_text}"
    )


def split_frame(frame: bytes, chunk_sizes: Sequence[int]) -> List[bytes]:
    chunks: List[bytes] = []
    offset = 0
    for chunk_size in chunk_sizes:
        if offset >= len(frame):
            break
        next_offset = min(offset + chunk_size, len(frame))
        chunks.append(frame[offset:next_offset])
        offset = next_offset
    if offset < len(frame):
        chunks.extend(split_frame_default(frame[offset:]))
    return chunks


def split_frame_default(frame: bytes) -> List[bytes]:
    return [
        frame[offset:offset + MAVLINK_TUNNEL_CHUNK_SIZE]
        for offset in range(0, len(frame), MAVLINK_TUNNEL_CHUNK_SIZE)
    ]


def normalize_mavlink_endpoint(endpoint: str) -> str:
    if endpoint.startswith("tcp://"):
        return "tcp:" + endpoint[len("tcp://"):]
    if endpoint.startswith("udp://"):
        return "udp:" + endpoint[len("udp://"):]
    if endpoint.startswith("udpin://"):
        return "udpin:" + endpoint[len("udpin://"):]
    if endpoint.startswith("udpout://"):
        return "udpout:" + endpoint[len("udpout://"):]
    return endpoint


class MavlinkMspTunnelSmokeTest:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.endpoint = normalize_mavlink_endpoint(args.mavlink_endpoint)
        self.connection = mavutil.mavlink_connection(
            self.endpoint,
            source_system=args.source_system,
            source_component=args.source_component,
            dialect="common",
            autoreconnect=True,
        )
        self.connection.mav.srcSystem = args.source_system
        self.connection.mav.srcComponent = args.source_component
        self.target_system = 0
        self.target_component = args.target_component
        self.parser = MspV1Parser()
        self.next_heartbeat_at = time.monotonic()
        self.last_target_heartbeat_at = 0.0

    def close(self) -> None:
        self.connection.close()

    def send_heartbeat(self) -> None:
        self.connection.mav.heartbeat_send(
            mavutil.mavlink.MAV_TYPE_GCS,
            mavutil.mavlink.MAV_AUTOPILOT_INVALID,
            mavutil.mavlink.MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
            0,
            mavutil.mavlink.MAV_STATE_ACTIVE,
        )

    def maybe_send_heartbeat(self) -> None:
        now = time.monotonic()
        if now >= self.next_heartbeat_at:
            self.send_heartbeat()
            self.next_heartbeat_at = now + 1.0 / self.args.heartbeat_hz

    def learn_target_from_heartbeat(self, message: object) -> None:
        if int(message.autopilot) == int(mavutil.mavlink.MAV_AUTOPILOT_INVALID):
            return

        src_system = int(message.get_srcSystem())
        src_component = int(message.get_srcComponent())
        if self.args.target_system and src_system != self.args.target_system:
            return

        self.last_target_heartbeat_at = time.monotonic()

        if self.target_system == 0:
            self.target_system = src_system
            if self.target_component == 0:
                self.target_component = 0
            print(
                f"target_discovered system_id={self.target_system} component_id={src_component} "
                f"target_component_for_tunnel={self.target_component}",
                flush=True,
            )

    def handle_incoming_message(self, message: object) -> List[MspV1Reply]:
        message_type = message.get_type()
        if message_type == "HEARTBEAT":
            self.learn_target_from_heartbeat(message)
            return []

        if message_type != "TUNNEL":
            return []

        src_system = int(message.get_srcSystem())
        src_component = int(message.get_srcComponent())
        if self.target_system and src_system != self.target_system:
            return []

        if int(message.payload_type) != MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP:
            return []

        if int(message.target_system) != self.args.source_system:
            return []

        if int(message.target_component) != self.args.source_component:
            return []

        tunnel_bytes = bytes(message.payload[:int(message.payload_length)])
        print(
            f"rx_tunnel src_system={src_system} src_component={src_component} payload_length={len(tunnel_bytes)}",
            flush=True,
        )
        return self.parser.feed(tunnel_bytes)

    def poll(self, deadline: float) -> List[MspV1Reply]:
        replies: List[MspV1Reply] = []
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=False)
            if message is None:
                time.sleep(0.01)
                continue
            replies.extend(self.handle_incoming_message(message))
            if replies:
                return replies
        return replies

    def wait_for_target(self, timeout_s: float) -> None:
        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(type="HEARTBEAT", blocking=False)
            if message is None:
                time.sleep(0.01)
                continue
            self.learn_target_from_heartbeat(message)
            if self.target_system != 0:
                return
        raise TimeoutError(
            f"mavlink_endpoint={self.endpoint} requested_target_system={self.args.target_system} timeout_s={timeout_s}"
        )

    def drain(self, duration_s: float) -> None:
        deadline = time.monotonic() + duration_s
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(blocking=False)
            if message is None:
                time.sleep(0.01)
                continue
            self.handle_incoming_message(message)

    def send_tunnel_chunks(self, chunks: Iterable[bytes]) -> None:
        for chunk in chunks:
            if len(chunk) > MAVLINK_TUNNEL_CHUNK_SIZE:
                raise ValueError(f"chunk_length={len(chunk)} exceeds tunnel_limit={MAVLINK_TUNNEL_CHUNK_SIZE}")
            print(
                f"tx_tunnel target_system={self.target_system} target_component={self.target_component} payload_length={len(chunk)}",
                flush=True,
            )
            padded_chunk = bytes(chunk) + bytes(MAVLINK_TUNNEL_CHUNK_SIZE - len(chunk))
            self.connection.mav.tunnel_send(
                self.target_system,
                self.target_component,
                MAVLINK_TUNNEL_PAYLOAD_TYPE_INAV_MSP,
                len(chunk),
                padded_chunk,
            )

    def request(self, cmd: int, payload: bytes, timeout_s: float, split_chunks: Optional[Sequence[int]] = None) -> MspV1Reply:
        self.parser.clear()
        self.drain(0.1)
        frame = build_msp_v1_request(cmd, payload)
        chunks = split_frame(frame, split_chunks) if split_chunks else split_frame_default(frame)
        print(
            f"tx_msp cmd={cmd} msp_length={len(frame)} tunnel_chunks={len(chunks)} timeout_s={timeout_s}",
            flush=True,
        )
        self.send_tunnel_chunks(chunks)

        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            replies = self.poll(deadline)
            for reply in replies:
                print(
                    f"rx_msp cmd={reply.cmd} payload_length={len(reply.payload)} is_error={int(reply.is_error)}",
                    flush=True,
                )
                if reply.cmd == cmd:
                    return reply
        raise TimeoutError(f"cmd={cmd} timeout_s={timeout_s}")

    def wait_for_reboot_reconnect(self, timeout_s: float) -> None:
        self.parser.clear()
        self.drain(0.2)
        deadline = time.monotonic() + timeout_s
        last_seen = self.last_target_heartbeat_at
        while time.monotonic() < deadline:
            self.maybe_send_heartbeat()
            message = self.connection.recv_match(type="HEARTBEAT", blocking=False)
            if message is None:
                time.sleep(0.05)
                continue
            self.learn_target_from_heartbeat(message)
            if self.last_target_heartbeat_at > last_seen:
                print(
                    f"post_reboot_heartbeat system_id={self.target_system} last_target_heartbeat_at={self.last_target_heartbeat_at:.3f}",
                    flush=True,
                )
                return
        raise TimeoutError(f"post_reboot_timeout_s={timeout_s} target_system={self.target_system}")


def verify_reply_ok(reply: MspV1Reply, cmd: int) -> None:
    if reply.cmd != cmd:
        raise ValueError(f"expected_cmd={cmd} actual_cmd={reply.cmd}")
    if reply.is_error:
        raise RuntimeError(f"cmd={cmd} returned_error=1")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Headless MSP-over-MAVLink tunnel smoke test with configurator-like heartbeat, save, and reboot flow."
    )
    parser.add_argument("--mavlink-endpoint", required=True, help='Existing SITL or FC MAVLink listener, e.g. "tcp:127.0.0.1:5761"')
    parser.add_argument("--source-system", type=int, default=MAVLINK_CONFIGURATOR_SYSTEM_ID, help="MAVLink source system ID")
    parser.add_argument("--source-component", type=int, default=MAVLINK_CONFIGURATOR_COMPONENT_ID, help="MAVLink source component ID")
    parser.add_argument("--target-system", type=int, default=0, help="Expected FC system ID, 0 accepts the first autopilot heartbeat")
    parser.add_argument("--target-component", type=int, default=0, help="Tunnel target component, 0 matches configurator behavior")
    parser.add_argument("--connect-timeout", type=float, default=10.0, help="Seconds to wait for FC heartbeat discovery")
    parser.add_argument("--request-timeout", type=float, default=2.0, help="Seconds to wait for normal MSP replies")
    parser.add_argument("--save-timeout", type=float, default=5.0, help="Seconds to wait for MSP_EEPROM_WRITE reply")
    parser.add_argument("--reboot-timeout", type=float, default=5.0, help="Seconds to wait for MSP_REBOOT reply")
    parser.add_argument("--post-reboot-timeout", type=float, default=15.0, help="Seconds to wait for heartbeat and tunnel recovery after reboot")
    parser.add_argument("--heartbeat-hz", type=float, default=1.0, help="Outbound GCS heartbeat rate")
    args = parser.parse_args()

    tester = MavlinkMspTunnelSmokeTest(args)
    try:
        print(
            f"connect mavlink_endpoint={tester.endpoint} source_system={args.source_system} source_component={args.source_component} "
            f"target_system={args.target_system} target_component={args.target_component}",
            flush=True,
        )
        tester.wait_for_target(args.connect_timeout)

        api_reply = tester.request(MSP_API_VERSION, b"", args.request_timeout, split_chunks=(2,))
        verify_reply_ok(api_reply, MSP_API_VERSION)
        print(decode_api_version(api_reply.payload), flush=True)

        fc_variant_reply = tester.request(MSP_FC_VARIANT, b"", args.request_timeout)
        verify_reply_ok(fc_variant_reply, MSP_FC_VARIANT)
        print(decode_fc_variant(fc_variant_reply.payload), flush=True)

        fc_version_reply = tester.request(MSP_FC_VERSION, b"", args.request_timeout)
        verify_reply_ok(fc_version_reply, MSP_FC_VERSION)
        print(decode_fc_version(fc_version_reply.payload), flush=True)

        build_info_reply = tester.request(MSP_BUILD_INFO, b"", args.request_timeout)
        verify_reply_ok(build_info_reply, MSP_BUILD_INFO)
        print(decode_build_info(build_info_reply.payload), flush=True)

        save_reply = tester.request(MSP_EEPROM_WRITE, b"", args.save_timeout)
        verify_reply_ok(save_reply, MSP_EEPROM_WRITE)
        print("save_ok cmd=250", flush=True)

        reboot_reply = tester.request(MSP_REBOOT, b"", args.reboot_timeout)
        verify_reply_ok(reboot_reply, MSP_REBOOT)
        print("reboot_ack cmd=68", flush=True)

        tester.wait_for_reboot_reconnect(args.post_reboot_timeout)
        post_reboot_api_reply = tester.request(MSP_API_VERSION, b"", args.request_timeout)
        verify_reply_ok(post_reboot_api_reply, MSP_API_VERSION)
        print("post_reboot_tunnel_ok cmd=1", flush=True)
        print(decode_api_version(post_reboot_api_reply.payload), flush=True)
    finally:
        tester.close()


if __name__ == "__main__":
    main()

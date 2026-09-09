#!/usr/bin/env python3
"""
Usage:
  conda run -n drone python src/utils/mavlink_tests/mavlink_routing_compliance_test.py --config src/utils/mavlink_tests/routing_test_config.yaml
"""

from __future__ import annotations

import argparse
import math
import time
from pathlib import Path
from typing import Any, Dict, List

import yaml
from pymavlink import mavutil


MAV_AUTOPILOT_INVALID = int(mavutil.mavlink.MAV_AUTOPILOT_INVALID)
MAV_MODE_FLAG_CUSTOM_MODE_ENABLED = int(mavutil.mavlink.MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)
MAV_STATE_ACTIVE = int(mavutil.mavlink.MAV_STATE_ACTIVE)
MAV_TYPE_GCS = int(mavutil.mavlink.MAV_TYPE_GCS)

ROUTING_RULE_REFERENCES = {
    "broadcast_forward": "routing.md:25-27,48-49",
    "known_target_forward": "routing.md:49-50; MAVLink_routing.cpp:66-80",
    "unknown_target_blocked": "routing.md:26-27,53",
    "no_ingress_loop": "MAVLink_routing.cpp:197-209",
    "no_repack": "routing.md:29-31",
}


def load_config(config_path: Path) -> Dict[str, Any]:
    return yaml.safe_load(config_path.read_text(encoding="utf-8"))


def open_mavlink_connection(endpoint: str, source_system: int, source_component: int, timeout_s: float) -> Any:
    deadline = time.monotonic() + timeout_s
    while True:
        try:
            return mavutil.mavlink_connection(
                endpoint,
                source_system=source_system,
                source_component=source_component,
                autoreconnect=True,
            )
        except OSError as error:
            if time.monotonic() >= deadline:
                raise TimeoutError(
                    f"endpoint={endpoint} source_system={source_system} source_component={source_component} timeout_s={timeout_s}"
                ) from error
            time.sleep(0.2)


def set_source_identity(connection: Any, system_id: int, component_id: int) -> None:
    connection.mav.srcSystem = system_id
    connection.mav.srcComponent = component_id


def send_heartbeat(connection: Any, system_id: int, component_id: int, mav_type: int) -> None:
    set_source_identity(connection, system_id, component_id)
    connection.mav.heartbeat_send(
        mav_type,
        MAV_AUTOPILOT_INVALID,
        MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
        0,
        MAV_STATE_ACTIVE,
    )


def drain_link(connection: Any, duration_s: float) -> None:
    deadline = time.monotonic() + duration_s
    while time.monotonic() < deadline:
        message = connection.recv_match(blocking=False)
        if message is None:
            time.sleep(0.002)


def drain_all_links(connections: Dict[str, Any], duration_s: float) -> None:
    for connection in connections.values():
        drain_link(connection, duration_s)


def collect_command_observations(connections: Dict[str, Any], command_id: int, observation_window_s: float) -> Dict[str, List[Dict[str, Any]]]:
    observations: Dict[str, List[Dict[str, Any]]] = {link_name: [] for link_name in connections}
    deadline = time.monotonic() + observation_window_s
    while time.monotonic() < deadline:
        saw_message = False
        for link_name, connection in connections.items():
            while True:
                message = connection.recv_match(type=["COMMAND_LONG"], blocking=False)
                if message is None:
                    break
                if int(message.command) != command_id:
                    continue
                observations[link_name].append(
                    {
                        "src_system": int(message.get_srcSystem()),
                        "src_component": int(message.get_srcComponent()),
                        "target_system": int(message.target_system),
                        "target_component": int(message.target_component),
                        "command": int(message.command),
                        "confirmation": int(message.confirmation),
                        "params": [
                            float(message.param1),
                            float(message.param2),
                            float(message.param3),
                            float(message.param4),
                            float(message.param5),
                            float(message.param6),
                            float(message.param7),
                        ],
                    }
                )
                saw_message = True
        if not saw_message:
            time.sleep(0.002)
    return observations


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run MAVLink routing compliance checks against INAV multiport routing.")
    parser.add_argument("--config", required=True, type=Path, help="YAML config path")
    args = parser.parse_args()

    config = load_config(args.config)
    timing = config["timing"]
    network = config["network"]
    tests = config["tests"]
    components_cfg = config["components"]
    vehicle = config["vehicle"]

    gcs_link_name = network["gcs_link"]
    fc_system_id = int(vehicle["fc_system_id"])
    unknown_component_id = int(tests["unknown_component_id"])
    unknown_system_id = int(tests["unknown_system_id"])
    marker_command_base = int(tests["marker_command_base"])
    report_path = Path(tests["report_markdown"])

    link_cfgs = network["links"]
    if gcs_link_name not in link_cfgs:
        raise ValueError(f"gcs_link={gcs_link_name} missing from network.links")

    components: List[Dict[str, Any]] = []
    for component in components_cfg:
        if component["link"] not in link_cfgs:
            raise ValueError(f"component={component['name']} link={component['link']} missing from network.links")
        components.append(
            {
                "name": component["name"],
                "link": component["link"],
                "system_id": int(component["system_id"]),
                "component_id": int(component["component_id"]),
                "mav_type": int(component["mav_type"]),
            }
        )

    component_ids = {component["component_id"] for component in components}
    component_system_ids = {component["system_id"] for component in components}
    if unknown_component_id in component_ids:
        raise ValueError(f"unknown_component_id={unknown_component_id} collides with configured component IDs")
    if unknown_system_id in component_system_ids or unknown_system_id == fc_system_id:
        raise ValueError(f"unknown_system_id={unknown_system_id} collides with configured systems")

    gcs_actor = {
        "name": "gcs",
        "link": gcs_link_name,
        "system_id": int(link_cfgs[gcs_link_name]["source_system"]),
        "component_id": int(link_cfgs[gcs_link_name]["source_component"]),
        "mav_type": MAV_TYPE_GCS,
    }

    inter_source = components[0]
    inter_target = None
    for component in components:
        if component["link"] != inter_source["link"]:
            inter_target = component
            break
    if inter_target is None:
        raise ValueError("Need at least one component on a different link for inter-component routing test")

    connections: Dict[str, Any] = {}
    fc_heartbeats: Dict[str, Dict[str, int]] = {}

    try:
        for link_name, link_cfg in link_cfgs.items():
            connection = open_mavlink_connection(
                endpoint=link_cfg["endpoint"],
                source_system=int(link_cfg["source_system"]),
                source_component=int(link_cfg["source_component"]),
                timeout_s=float(timing["connect_timeout_s"]),
            )
            connections[link_name] = connection

        for link_name, connection in connections.items():
            heartbeat = connection.wait_heartbeat(timeout=float(timing["connect_timeout_s"]))
            if heartbeat is None:
                raise TimeoutError(f"link={link_name} wait_heartbeat timed out")
            fc_heartbeats[link_name] = {
                "system_id": int(heartbeat.get_srcSystem()),
                "component_id": int(heartbeat.get_srcComponent()),
            }
            print(
                f"fc_heartbeat link={link_name} system_id={fc_heartbeats[link_name]['system_id']} "
                f"component_id={fc_heartbeats[link_name]['component_id']}",
                flush=True,
            )

        time.sleep(float(timing["settle_after_connect_s"]))
        drain_all_links(connections, float(timing["drain_window_s"]))

        send_heartbeat(
            connections[gcs_actor["link"]],
            gcs_actor["system_id"],
            gcs_actor["component_id"],
            gcs_actor["mav_type"],
        )
        for component in components:
            send_heartbeat(
                connections[component["link"]],
                component["system_id"],
                component["component_id"],
                component["mav_type"],
            )
            print(
                f"route_learn_heartbeat component={component['name']} link={component['link']} "
                f"system_id={component['system_id']} component_id={component['component_id']} mav_type={component['mav_type']}",
                flush=True,
            )
        time.sleep(float(timing["route_learn_settle_s"]))
        drain_all_links(connections, float(timing["drain_window_s"]))

        cases: List[Dict[str, Any]] = []
        all_other_than_gcs = sorted([link_name for link_name in connections if link_name != gcs_link_name])

        cases.append(
            {
                "name": "gcs_broadcast",
                "source": gcs_actor,
                "target_system": 0,
                "target_component": 0,
                "expected_links": all_other_than_gcs,
                "rules": [ROUTING_RULE_REFERENCES["broadcast_forward"], ROUTING_RULE_REFERENCES["no_ingress_loop"]],
            }
        )

        cases.append(
            {
                "name": "gcs_target_local_system_component_broadcast",
                "source": gcs_actor,
                "target_system": fc_system_id,
                "target_component": 0,
                "expected_links": all_other_than_gcs,
                "rules": [ROUTING_RULE_REFERENCES["broadcast_forward"], ROUTING_RULE_REFERENCES["no_ingress_loop"]],
            }
        )

        for component in components:
            expected_links = [component["link"]]
            if component["link"] == gcs_link_name:
                expected_links = []
            cases.append(
                {
                    "name": f"gcs_target_{component['name']}",
                    "source": gcs_actor,
                    "target_system": component["system_id"],
                    "target_component": component["component_id"],
                    "expected_links": sorted(expected_links),
                    "rules": [ROUTING_RULE_REFERENCES["known_target_forward"], ROUTING_RULE_REFERENCES["no_ingress_loop"]],
                }
            )

        cases.append(
            {
                "name": "gcs_target_unknown_component",
                "source": gcs_actor,
                "target_system": fc_system_id,
                "target_component": unknown_component_id,
                "expected_links": [],
                "rules": [ROUTING_RULE_REFERENCES["unknown_target_blocked"]],
            }
        )

        cases.append(
            {
                "name": "gcs_target_unknown_system",
                "source": gcs_actor,
                "target_system": unknown_system_id,
                "target_component": 1,
                "expected_links": [],
                "rules": [ROUTING_RULE_REFERENCES["unknown_target_blocked"]],
            }
        )

        component_broadcast_expected = sorted([link_name for link_name in connections if link_name != inter_source["link"]])
        cases.append(
            {
                "name": f"{inter_source['name']}_broadcast",
                "source": inter_source,
                "target_system": 0,
                "target_component": 0,
                "expected_links": component_broadcast_expected,
                "rules": [ROUTING_RULE_REFERENCES["broadcast_forward"], ROUTING_RULE_REFERENCES["no_ingress_loop"]],
            }
        )

        cases.append(
            {
                "name": f"{inter_source['name']}_to_{inter_target['name']}",
                "source": inter_source,
                "target_system": inter_target["system_id"],
                "target_component": inter_target["component_id"],
                "expected_links": [inter_target["link"]],
                "rules": [ROUTING_RULE_REFERENCES["known_target_forward"], ROUTING_RULE_REFERENCES["no_ingress_loop"]],
            }
        )

        cases.append(
            {
                "name": f"{inter_source['name']}_to_gcs",
                "source": inter_source,
                "target_system": gcs_actor["system_id"],
                "target_component": gcs_actor["component_id"],
                "expected_links": [gcs_actor["link"]],
                "rules": [ROUTING_RULE_REFERENCES["known_target_forward"], ROUTING_RULE_REFERENCES["no_ingress_loop"]],
            }
        )

        if marker_command_base + len(cases) > 65535:
            raise ValueError(f"marker_command_base={marker_command_base} too high for case_count={len(cases)}")

        results: List[Dict[str, Any]] = []
        for case_index, case in enumerate(cases, start=1):
            drain_all_links(connections, float(timing["drain_window_s"]))
            command_id = marker_command_base + case_index
            confirmation = case_index % 256
            params = [
                float(1000 + case_index),
                float(2000 + case_index),
                float(3000 + case_index),
                float(4000 + case_index),
                float(5000 + case_index),
                float(6000 + case_index),
                float(7000 + case_index),
            ]

            source = case["source"]
            source_connection = connections[source["link"]]
            set_source_identity(source_connection, source["system_id"], source["component_id"])
            source_connection.mav.command_long_send(
                int(case["target_system"]),
                int(case["target_component"]),
                command_id,
                confirmation,
                params[0],
                params[1],
                params[2],
                params[3],
                params[4],
                params[5],
                params[6],
            )

            observations = collect_command_observations(
                connections=connections,
                command_id=command_id,
                observation_window_s=float(timing["observation_window_s"]),
            )
            observed_links = sorted([link_name for link_name, msgs in observations.items() if len(msgs) > 0])
            expected_links = sorted(case["expected_links"])
            no_ingress_loop = source["link"] not in observed_links
            routing_match = observed_links == expected_links

            payload_intact = True
            payload_issues: List[str] = []
            for link_name, messages in observations.items():
                for message in messages:
                    if message["src_system"] != int(source["system_id"]) or message["src_component"] != int(source["component_id"]):
                        payload_intact = False
                        payload_issues.append(
                            f"link={link_name} src_mismatch expected=({source['system_id']},{source['component_id']}) "
                            f"observed=({message['src_system']},{message['src_component']})"
                        )
                    if message["target_system"] != int(case["target_system"]) or message["target_component"] != int(case["target_component"]):
                        payload_intact = False
                        payload_issues.append(
                            f"link={link_name} target_mismatch expected=({case['target_system']},{case['target_component']}) "
                            f"observed=({message['target_system']},{message['target_component']})"
                        )
                    if message["command"] != command_id or message["confirmation"] != confirmation:
                        payload_intact = False
                        payload_issues.append(
                            f"link={link_name} cmd_mismatch expected=(command={command_id},confirmation={confirmation}) "
                            f"observed=(command={message['command']},confirmation={message['confirmation']})"
                        )
                    for index, observed_value in enumerate(message["params"]):
                        expected_value = params[index]
                        if not math.isclose(observed_value, expected_value, rel_tol=0.0, abs_tol=1e-3):
                            payload_intact = False
                            payload_issues.append(
                                f"link={link_name} param{index + 1}_mismatch expected={expected_value} observed={observed_value}"
                            )

            case_pass = routing_match and no_ingress_loop and payload_intact
            results.append(
                {
                    "name": case["name"],
                    "source": source["name"],
                    "source_link": source["link"],
                    "target_system": int(case["target_system"]),
                    "target_component": int(case["target_component"]),
                    "command_id": command_id,
                    "expected_links": expected_links,
                    "observed_links": observed_links,
                    "routing_match": routing_match,
                    "no_ingress_loop": no_ingress_loop,
                    "payload_intact": payload_intact,
                    "pass": case_pass,
                    "rules": case["rules"],
                    "payload_issues": payload_issues,
                }
            )

            print(
                f"case={case['name']} command_id={command_id} source={source['name']} source_link={source['link']} "
                f"target=({case['target_system']},{case['target_component']}) expected_links={expected_links} observed_links={observed_links} "
                f"routing_match={routing_match} no_ingress_loop={no_ingress_loop} payload_intact={payload_intact} pass={case_pass}",
                flush=True,
            )
            time.sleep(float(timing["inter_case_pause_s"]))

    finally:
        for connection in connections.values():
            connection.close()

    pass_count = sum(1 for result in results if result["pass"])
    fail_count = len(results) - pass_count

    markdown_lines: List[str] = []
    markdown_lines.append("# MAVLink Routing Compliance Test")
    markdown_lines.append("")
    markdown_lines.append(
        f"- fc_system_id: `{fc_system_id}`"
    )
    markdown_lines.append(
        f"- links: `{sorted(list(link_cfgs.keys()))}`"
    )
    markdown_lines.append(
        f"- gcs_link: `{gcs_link_name}`"
    )
    markdown_lines.append(
        f"- components: `{[(component['name'], component['system_id'], component['component_id'], component['link']) for component in components]}`"
    )
    markdown_lines.append("")
    markdown_lines.append("## Rule References")
    markdown_lines.append("")
    markdown_lines.append(f"- broadcast_forward: `{ROUTING_RULE_REFERENCES['broadcast_forward']}`")
    markdown_lines.append(f"- known_target_forward: `{ROUTING_RULE_REFERENCES['known_target_forward']}`")
    markdown_lines.append(f"- unknown_target_blocked: `{ROUTING_RULE_REFERENCES['unknown_target_blocked']}`")
    markdown_lines.append(f"- no_ingress_loop: `{ROUTING_RULE_REFERENCES['no_ingress_loop']}`")
    markdown_lines.append(f"- no_repack: `{ROUTING_RULE_REFERENCES['no_repack']}`")
    markdown_lines.append("")
    markdown_lines.append("## Results")
    markdown_lines.append("")
    markdown_lines.append("| Case | Source(link) | Target(sys,comp) | Expected Links | Observed Links | Routing | No Ingress Loop | Payload Intact | Pass |")
    markdown_lines.append("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    for result in results:
        markdown_lines.append(
            f"| `{result['name']}` | `{result['source']} ({result['source_link']})` | "
            f"`({result['target_system']},{result['target_component']})` | "
            f"`{result['expected_links']}` | `{result['observed_links']}` | "
            f"`{result['routing_match']}` | `{result['no_ingress_loop']}` | `{result['payload_intact']}` | `{result['pass']}` |"
        )
    markdown_lines.append("")
    markdown_lines.append(f"summary pass_count={pass_count} fail_count={fail_count} total={len(results)}")
    markdown_lines.append("")

    failures = [result for result in results if not result["pass"]]
    if failures:
        markdown_lines.append("## Failure Details")
        markdown_lines.append("")
        for failure in failures:
            markdown_lines.append(f"### {failure['name']}")
            markdown_lines.append("")
            markdown_lines.append(
                f"- expected_links: `{failure['expected_links']}` observed_links: `{failure['observed_links']}` "
                f"routing_match: `{failure['routing_match']}` no_ingress_loop: `{failure['no_ingress_loop']}` "
                f"payload_intact: `{failure['payload_intact']}`"
            )
            if failure["payload_issues"]:
                markdown_lines.append(f"- payload_issues: `{failure['payload_issues']}`")
            markdown_lines.append(f"- rule_refs: `{failure['rules']}`")
            markdown_lines.append("")

    report_path.write_text("\n".join(markdown_lines) + "\n", encoding="utf-8")
    print(f"report_path={report_path} pass_count={pass_count} fail_count={fail_count} total={len(results)}", flush=True)

    if fail_count > 0:
        raise SystemExit(1)

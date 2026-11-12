#!/usr/bin/env python3
"""
Usage:
  conda run -n drone python src/utils/mavlink_tests/mav_multi_sweep.py
  conda run -n drone python src/utils/mavlink_tests/mav_multi_sweep.py --config src/utils/mavlink_tests/test_config_multiport4_sweep_rc460800_tele115200.yaml
"""

from __future__ import annotations

import argparse
import importlib.util
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Tuple

import yaml


TESTS_DIR = Path(__file__).resolve().parent
DEFAULT_CONFIG_PATH = TESTS_DIR / "test_config_multiport4_sweep_rc460800_tele115200.yaml"
BENCHMARK_MODULE_PATH = TESTS_DIR / "mav_multi_benchmark.py"

spec = importlib.util.spec_from_file_location("mav_multi_benchmark", BENCHMARK_MODULE_PATH)
benchmark = importlib.util.module_from_spec(spec)
assert spec is not None
assert spec.loader is not None
spec.loader.exec_module(benchmark)


parser = argparse.ArgumentParser(description="Run MAVLink total-load sweep table for 1..4 ports and [50,100,200,max] req/s total")
parser.add_argument("--config", default=str(DEFAULT_CONFIG_PATH), help="YAML configuration path")
args = parser.parse_args()

config_path = Path(args.config)
config: Dict[str, Any] = yaml.safe_load(config_path.read_text(encoding="utf-8"))

sitl_bin = Path(config["sitl"]["binary"])
sitl_workdir = Path(config["sitl"]["workdir"])
sitl_eeprom = str(config["sitl"]["eeprom_path"])
sitl_log = Path(config["sitl"]["runtime_log"])
sitl_log.parent.mkdir(parents=True, exist_ok=True)

rate_labels: List[Tuple[str, float]] = [
    ("50", 50.0),
    ("100", 100.0),
    ("200", 200.0),
    ("max", float(config["tests"]["max_rate_hz"])),
]

results: Dict[str, Dict[int, Dict[str, Any]]] = {}

rc_port = int(config["ports"]["rc"])
telemetry_ports = [int(port) for port in config["ports"]["telemetry"]]
sweep_duration_s = float(config["tests"]["sweep_duration_s"])

rate_cache: Dict[float, Dict[int, Dict[str, Any]]] = {}

for label, total_rate_hz in rate_labels:
    if total_rate_hz not in rate_cache:
        rate_cache[total_rate_hz] = {}
        for port_count in range(1, 5):
            scenario_log = sitl_log.with_name(f"{sitl_log.stem}_{label}_{port_count}{sitl_log.suffix}")
            log_handle = scenario_log.open("w", encoding="utf-8")
            sitl_proc = subprocess.Popen(
                [str(sitl_bin), f"--path={sitl_eeprom}"],
                cwd=str(sitl_workdir),
                stdout=log_handle,
                stderr=subprocess.STDOUT,
                text=True,
            )
            try:
                benchmark.wait_for_tcp_port("127.0.0.1", int(config["ports"]["msp"]), float(config["tests"]["port_ready_timeout_s"]))
                benchmark.apply_config_and_wait(config, 4)
                active_ports = [rc_port] + telemetry_ports[: max(0, port_count - 1)]
                per_port_rate_hz = total_rate_hz / float(port_count)
                load_rates_hz = {port: per_port_rate_hz for port in active_ports}
                print(
                    f"sweep_run_start label={label} total_rate={total_rate_hz} per_port_rate={per_port_rate_hz} ports={port_count}",
                    flush=True,
                )
                result = benchmark.run_workload(
                    config=config,
                    sitl_pid=sitl_proc.pid,
                    telemetry_ports=active_ports,
                    rc_port=rc_port,
                    load_rates_hz=load_rates_hz,
                    duration_s=sweep_duration_s,
                )
                rate_cache[total_rate_hz][port_count] = result
                print(
                    f"sweep_run_done label={label} total_rate={total_rate_hz} per_port_rate={per_port_rate_hz} ports={port_count}",
                    flush=True,
                )
            finally:
                sitl_proc.terminate()
                try:
                    sitl_proc.wait(timeout=5.0)
                except subprocess.TimeoutExpired:
                    sitl_proc.kill()
                    sitl_proc.wait(timeout=5.0)
                log_handle.close()
                scenario_log.unlink(missing_ok=True)
    results[label] = rate_cache[total_rate_hz]

output_path = Path(config["output"]["testing_md"])
lines: List[str] = [
    "# MAVLink Port Load Sweep",
    "",
    f"- Generated: `{datetime.now(timezone.utc).isoformat()}`",
    f"- SITL binary: `{config['sitl']['binary']}`",
    f"- EEPROM: `{config['sitl']['eeprom_path']}`",
    "- UART1 MSP only.",
    "- UART2 is MAVLink RC (460800), RC override target is 100 Hz.",
    "- Additional MAVLink telemetry ports are 115200 baud.",
    "- Total command rate is held constant per load label and split evenly across active MAVLink ports.",
    "",
    "## Comparison Table",
    "",
    "| Load Label | Total Msg/s | Active MAVLink ports | Msg/s per active port | FC cpuLoad avg/max | FC cycleTime avg/max (us) | MAV seq loss max | Cmd failed total | Cmd failed % | MSP fail % |",
    "| --- | ---: | ---: | ---: | --- | --- | ---: | ---: | ---: | ---: |",
]

for label, total_rate_hz in rate_labels:
    rate_results = results[label]
    for port_count in range(1, 5):
        result = rate_results[port_count]
        per_port = list(result["ports"].values())
        mav_seq_loss_max_pct = max(item["mav_seq_loss_rate"] * 100.0 for item in per_port)
        cmd_sent_total = sum(int(item["command_sent"]) for item in per_port)
        cmd_failed_total = sum(int(item["command_failed_total"]) for item in per_port)
        cmd_failed_pct = (cmd_failed_total / max(cmd_sent_total, 1)) * 100.0
        per_port_rate_hz = total_rate_hz / float(port_count)
        lines.append(
            f"| {label} | {total_rate_hz:.1f} | {port_count} | {per_port_rate_hz:.2f} | "
            f"{result['fc_cpu_load_avg_pct']:.2f}% / {result['fc_cpu_load_max_pct']:.2f}% | "
            f"{result['fc_cycle_time_avg_us']:.1f} / {result['fc_cycle_time_max_us']:.1f} | "
            f"{mav_seq_loss_max_pct:.2f}% | {cmd_failed_total} | {cmd_failed_pct:.2f}% | {(result['msp_failure_rate'] * 100.0):.2f}% |"
        )

lines.append("")
lines.append("## Raw Scenario Details")
lines.append("")
for label, total_rate_hz in rate_labels:
    lines.append(f"### {label} ({total_rate_hz:.1f} total msg/s)")
    lines.append("")
    for port_count in range(1, 5):
        result = results[label][port_count]
        per_port_rate_hz = total_rate_hz / float(port_count)
        lines.append(f"#### {port_count} active MAVLink port(s)")
        lines.append("")
        lines.append(f"- Total command rate: `{total_rate_hz:.1f} msg/s`")
        lines.append(f"- Per-port command rate: `{per_port_rate_hz:.2f} msg/s`")
        lines.append(
            f"- FC cpuLoad avg/max: `{result['fc_cpu_load_avg_pct']:.2f}% / {result['fc_cpu_load_max_pct']:.2f}%` | "
            f"FC cycleTime avg/max: `{result['fc_cycle_time_avg_us']:.1f} / {result['fc_cycle_time_max_us']:.1f}` us | "
            f"MSP fail rate: `{(result['msp_failure_rate'] * 100.0):.2f}%`"
        )
        lines.append("")
        lines.append(benchmark.format_port_table(result["ports"]))
        lines.append("")

output_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"report_written={output_path}", flush=True)

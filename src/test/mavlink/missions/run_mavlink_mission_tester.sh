#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
INAV_DIR="$(cd -- "${SCRIPT_DIR}/../../../.." && pwd)"
WORKSPACE_ROOT="$(cd -- "${INAV_DIR}/.." && pwd)"
BRANCH_DIR="${WORKSPACE_ROOT}/mydev/branch/mavlink_multiport2"
LOG_DIR="${SCRIPT_DIR}/results"

SITL_BINARY="${INAV_DIR}/cmake/build_SITL/inav_9.1.0_SITL"
EEPROM_PATH="../mydev/branch/mavlink_multiport2/eeprom.bin"
CONFIG_PATH="${SCRIPT_DIR}/mavlink_mission_tester.ini"
REPORT_PATH="${LOG_DIR}/mission_tester_report.json"
SITL_LOG="${LOG_DIR}/mission_tester_sitl.log"
MAVPROXY_LOG="${LOG_DIR}/mission_tester_mavproxy.log"
TESTER_LOG="${LOG_DIR}/mission_tester_stdout.log"

SITL_PID=""
MAVPROXY_PID=""

require_file() {
    local path="$1"
    local label="$2"
    if [[ ! -f "${path}" ]]; then
        echo "missing_${label}=${path}" >&2
        exit 1
    fi
}

wait_tcp() {
    local host="$1"
    local port="$2"
    local label="$3"
    local timeout_s="$4"
    local start_s
    start_s="$(date +%s)"

    while (( "$(date +%s)" - start_s < timeout_s )); do
        if (exec 3<>"/dev/tcp/${host}/${port}") 2>/dev/null; then
            exec 3<&-
            exec 3>&-
            echo "tcp_ready label=${label} endpoint=${host}:${port}"
            return 0
        fi
        sleep 0.2
    done

    echo "tcp_timeout label=${label} endpoint=${host}:${port} timeout_s=${timeout_s}" >&2
    return 1
}

stop_process_group() {
    local pid="$1"
    local label="$2"

    if [[ -z "${pid}" ]]; then
        return 0
    fi

    if ! kill -0 "${pid}" 2>/dev/null; then
        return 0
    fi

    echo "stopping_process label=${label} pid=${pid}"
    kill -TERM -- "-${pid}" 2>/dev/null || kill -TERM "${pid}" 2>/dev/null || true

    for _ in {1..30}; do
        if ! kill -0 "${pid}" 2>/dev/null; then
            wait "${pid}" 2>/dev/null || true
            return 0
        fi
        sleep 0.1
    done

    echo "killing_process label=${label} pid=${pid}"
    kill -KILL -- "-${pid}" 2>/dev/null || kill -KILL "${pid}" 2>/dev/null || true
    wait "${pid}" 2>/dev/null || true
}

tail_log() {
    local path="$1"
    local label="$2"
    if [[ -f "${path}" ]]; then
        echo "${label}_tail_start"
        tail -80 "${path}" || true
        echo "${label}_tail_end"
    fi
}

scan_log_errors() {
    local path="$1"
    local label="$2"
    if [[ -f "${path}" ]]; then
        echo "${label}_error_scan_start"
        grep -Ein "error|fail|failed|invalid|unsupported|denied|timeout|traceback" "${path}" || true
        echo "${label}_error_scan_end"
    fi
}

start_mavproxy_master() {
    local master_endpoint="$1"
    local port_label="${master_endpoint##*:}"
    local attempt_log="${LOG_DIR}/mission_tester_mavproxy_${port_label}.log"

    : > "${attempt_log}"
    (
        cd "${LOG_DIR}"
        exec setsid conda run --no-capture-output -n drone mavproxy.py \
            --master="${master_endpoint}" \
            --force-connected \
            --nowait \
            --daemon \
            --out=udp:127.0.0.1:14550
    ) >"${attempt_log}" 2>&1 &
    MAVPROXY_PID=$!
    MAVPROXY_LOG="${attempt_log}"
    echo "mavproxy_started pid=${MAVPROXY_PID} master=${master_endpoint} log=src/test/mavlink/missions/results/mission_tester_mavproxy_${port_label}.log"

    sleep 5
    if kill -0 "${MAVPROXY_PID}" 2>/dev/null; then
        echo "mavproxy_ready pid=${MAVPROXY_PID} master=${master_endpoint}"
        return 0
    fi

    echo "mavproxy_exited_early master=${master_endpoint}" >&2
    tail_log "${attempt_log}" "mavproxy_${port_label}"
    MAVPROXY_PID=""
    return 1
}

cleanup() {
    local status=$?
    set +e
    stop_process_group "${MAVPROXY_PID}" "mavproxy"
    stop_process_group "${SITL_PID}" "sitl"
    if [[ "${status}" -ne 0 ]]; then
        tail_log "${TESTER_LOG}" "tester"
        tail_log "${MAVPROXY_LOG}" "mavproxy"
        tail_log "${SITL_LOG}" "sitl"
    fi
    exit "${status}"
}

trap cleanup EXIT INT TERM

require_file "${SITL_BINARY}" "sitl_binary"
require_file "${CONFIG_PATH}" "config"
require_file "${BRANCH_DIR}/eeprom.bin" "eeprom"

mkdir -p "${LOG_DIR}"
rm -f "${REPORT_PATH}"
: > "${SITL_LOG}"
: > "${TESTER_LOG}"

echo "mission_tester_start"
echo "inav_dir=."
echo "sitl_binary=cmake/build_SITL/inav_9.1.0_SITL"
echo "results_dir=src/test/mavlink/missions/results"

(
    cd "${INAV_DIR}"
    exec setsid "${SITL_BINARY}" \
        --serialport=/dev/ttyUSB0 \
        --serialuart=3 \
        --baudrate=460800 \
        --path="${EEPROM_PATH}" \
        --chanmap=M01-01,S02-02,S01-03,S04-04
) >"${SITL_LOG}" 2>&1 &
SITL_PID=$!
echo "sitl_started pid=${SITL_PID} log=src/test/mavlink/missions/results/mission_tester_sitl.log"

wait_tcp 127.0.0.1 5760 "sitl_msp_uart1" 20
wait_tcp 127.0.0.1 5763 "sitl_mavlink_uart4" 20

start_mavproxy_master tcp:127.0.0.1:5763

set +e
(
    cd "${INAV_DIR}"
    conda run --no-capture-output -n drone python src/test/mavlink/missions/mavlink_mission_tester.py \
        --config src/test/mavlink/missions/mavlink_mission_tester.ini
) 2>&1 | tee "${TESTER_LOG}"
TEST_STATUS=${PIPESTATUS[0]}
set -e

echo "mission_tester_process_status=${TEST_STATUS}"
scan_log_errors "${TESTER_LOG}" "tester"
scan_log_errors "${MAVPROXY_LOG}" "mavproxy"
scan_log_errors "${SITL_LOG}" "sitl"

if [[ -f "${REPORT_PATH}" ]]; then
    echo "mission_tester_report=src/test/mavlink/missions/results/mission_tester_report.json"
fi

if [[ "${TEST_STATUS}" -ne 0 ]]; then
    if [[ -f "${REPORT_PATH}" ]]; then
        echo "mission_tester_report_json_start"
        cat "${REPORT_PATH}"
        echo "mission_tester_report_json_end"
    fi
    exit "${TEST_STATUS}"
fi

echo "mission_tester_passed"

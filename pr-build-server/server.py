#!/usr/bin/env python3
"""
server.py - HTTP server for the INAV PR build interface.

Serves the web UI and provides a REST-ish API for queuing and monitoring builds.

Usage:
    python3 server.py [--host 0.0.0.0] [--port 8080]

API endpoints:
    GET  /                          Redirect to /web/index.html
    GET  /web/<file>                Serve static web files
    GET  /api/prs                   Return current open PRs (from data/prs.json)
    GET  /api/targets               Return available targets (from data/targets.json)
    POST /api/build                 Queue a build {pr: <number>, target: <name>}
    GET  /api/status/<job_id>       Return build status
    GET  /api/download/<job_id>     Download the built hex file
    GET  /api/log/<job_id>          Return raw build log text
    GET  /api/queue                 Return current build queue

Requires: Python 3.6+, no extra packages.
"""

import argparse
import json
import mimetypes
import os
import queue
import re
import subprocess
import sys
import threading
import time
import uuid
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path
from urllib.parse import urlparse

SERVER_DIR = Path(__file__).parent.resolve()
INAV_DIR = SERVER_DIR.parent
DATA_DIR = SERVER_DIR / "data"
BUILDS_DIR = SERVER_DIR / "builds"
WEB_DIR = SERVER_DIR / "web"
BUILD_SCRIPT = SERVER_DIR / "scripts" / "build_pr.sh"

# Limit concurrent builds and queue depth
MAX_CONCURRENT_BUILDS = 1
MAX_QUEUE_DEPTH = 10

# How long to keep completed build artifacts (seconds) – 24 hours
BUILD_TTL = 86400

# ── Build queue ───────────────────────────────────────────────────────────────

_build_queue: queue.Queue = queue.Queue(maxsize=MAX_QUEUE_DEPTH)
_active_builds: dict = {}   # job_id -> {"pr", "target", "proc", "started_at"}
_queue_lock = threading.Lock()


def _sanitize(value: str, pattern: str = r"^[A-Za-z0-9_\-]+$") -> str:
    """Raise ValueError if value doesn't match the expected safe pattern."""
    if not re.match(pattern, value):
        raise ValueError(f"Invalid value: {value!r}")
    return value


def _run_builder():
    """Worker thread: pull jobs from the queue and execute them."""
    while True:
        job = _build_queue.get()
        if job is None:
            break
        job_id = job["job_id"]
        pr = job["pr"]
        target = job["target"]

        with _queue_lock:
            _active_builds[job_id] = {**job, "started_at": time.time()}

        print(f"[builder] Starting job {job_id}: PR#{pr} {target}")
        env = os.environ.copy()
        env["SERVER_DIR"] = str(SERVER_DIR)
        env["INAV_DIR"] = str(INAV_DIR)

        try:
            proc = subprocess.Popen(
                ["bash", str(BUILD_SCRIPT), str(pr), target, job_id],
                env=env,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            proc.wait()
        except Exception as exc:
            print(f"[builder] Job {job_id} crashed: {exc}", file=sys.stderr)
            status_file = BUILDS_DIR / job_id / "status.json"
            status_file.parent.mkdir(parents=True, exist_ok=True)
            status_file.write_text(json.dumps({
                "job_id": job_id, "pr": pr, "target": target,
                "phase": "failed", "message": str(exc),
                "updated_at": _now(),
            }))
        finally:
            with _queue_lock:
                _active_builds.pop(job_id, None)

        _build_queue.task_done()


def _now() -> str:
    import datetime
    return datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")


def _cleanup_old_builds():
    """Remove build directories older than BUILD_TTL."""
    if not BUILDS_DIR.exists():
        return
    cutoff = time.time() - BUILD_TTL
    for d in BUILDS_DIR.iterdir():
        if d.is_dir() and d.stat().st_mtime < cutoff:
            import shutil
            try:
                shutil.rmtree(d)
                print(f"[cleanup] Removed old build: {d.name}")
            except Exception as e:
                print(f"[cleanup] Failed to remove {d}: {e}", file=sys.stderr)


def _cleanup_loop():
    while True:
        time.sleep(3600)
        _cleanup_old_builds()


# ── HTTP handler ──────────────────────────────────────────────────────────────

class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        # Suppress default noisy access log; uncomment to re-enable
        # print(f"{self.address_string()} - {fmt % args}")
        pass

    def send_json(self, data, code=200):
        body = json.dumps(data, indent=2).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    def send_error_json(self, code, message):
        self.send_json({"error": message}, code)

    def read_json_body(self):
        length = int(self.headers.get("Content-Length", 0))
        if length == 0:
            return {}
        return json.loads(self.rfile.read(length).decode())

    # ── routing ──────────────────────────────────────────────────────────────

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path.rstrip("/") or "/"

        if path == "/":
            self.send_response(302)
            self.send_header("Location", "/web/index.html")
            self.end_headers()
        elif path.startswith("/web/"):
            self._serve_static(path[5:] or "index.html")
        elif path == "/api/prs":
            self._api_prs()
        elif path == "/api/targets":
            self._api_targets()
        elif path.startswith("/api/status/"):
            self._api_status(path[12:])
        elif path.startswith("/api/download/"):
            self._api_download(path[14:])
        elif path.startswith("/api/log/"):
            self._api_log(path[9:])
        elif path == "/api/queue":
            self._api_queue()
        else:
            self.send_error_json(404, "Not found")

    def do_POST(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path == "/api/build":
            self._api_build()
        else:
            self.send_error_json(404, "Not found")

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    # ── static files ─────────────────────────────────────────────────────────

    def _serve_static(self, filename):
        # Prevent directory traversal
        safe = Path(WEB_DIR / filename).resolve()
        if not str(safe).startswith(str(WEB_DIR)):
            self.send_error_json(403, "Forbidden")
            return
        if not safe.exists() or not safe.is_file():
            self.send_error_json(404, f"File not found: {filename}")
            return
        mime, _ = mimetypes.guess_type(str(safe))
        mime = mime or "application/octet-stream"
        body = safe.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", mime)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    # ── API: /api/prs ─────────────────────────────────────────────────────────

    def _api_prs(self):
        prs_file = DATA_DIR / "prs.json"
        if not prs_file.exists():
            self.send_json({"updated_at": None, "prs": []})
            return
        self.send_json(json.loads(prs_file.read_text()))

    # ── API: /api/targets ─────────────────────────────────────────────────────

    def _api_targets(self):
        targets_file = DATA_DIR / "targets.json"
        if not targets_file.exists():
            self.send_json([])
            return
        self.send_json(json.loads(targets_file.read_text()))

    # ── API: POST /api/build ──────────────────────────────────────────────────

    def _api_build(self):
        try:
            body = self.read_json_body()
            pr = str(int(body["pr"]))       # must be numeric
            target = _sanitize(str(body["target"]))
        except (KeyError, ValueError, TypeError) as e:
            self.send_error_json(400, f"Invalid request: {e}")
            return

        # Check target exists
        targets_file = DATA_DIR / "targets.json"
        if targets_file.exists():
            targets = json.loads(targets_file.read_text())
            valid = {t["name"] for t in targets}
            if target not in valid:
                self.send_error_json(400, f"Unknown target: {target}")
                return

        job_id = str(uuid.uuid4())
        job = {"job_id": job_id, "pr": pr, "target": target, "queued_at": _now()}

        # Write initial queued status
        job_dir = BUILDS_DIR / job_id
        job_dir.mkdir(parents=True, exist_ok=True)
        (job_dir / "status.json").write_text(json.dumps({
            **job, "phase": "queued", "message": "Waiting in queue",
        }))

        try:
            _build_queue.put_nowait(job)
        except queue.Full:
            import shutil; shutil.rmtree(job_dir, ignore_errors=True)
            self.send_error_json(503, "Build queue is full; please try again later")
            return

        print(f"[api] Queued job {job_id}: PR#{pr} {target}")
        self.send_json({"job_id": job_id, "pr": pr, "target": target}, 202)

    # ── API: /api/status/<job_id> ─────────────────────────────────────────────

    def _api_status(self, job_id):
        try:
            job_id = _sanitize(job_id, r"^[0-9a-f\-]+$")
        except ValueError:
            self.send_error_json(400, "Invalid job ID")
            return

        status_file = BUILDS_DIR / job_id / "status.json"
        if not status_file.exists():
            self.send_error_json(404, "Job not found")
            return
        self.send_json(json.loads(status_file.read_text()))

    # ── API: /api/download/<job_id> ───────────────────────────────────────────

    def _api_download(self, job_id):
        try:
            job_id = _sanitize(job_id, r"^[0-9a-f\-]+$")
        except ValueError:
            self.send_error_json(400, "Invalid job ID")
            return

        job_dir = BUILDS_DIR / job_id
        if not job_dir.exists():
            self.send_error_json(404, "Job not found")
            return

        status_file = job_dir / "status.json"
        if not status_file.exists():
            self.send_error_json(404, "Build not complete")
            return

        status = json.loads(status_file.read_text())
        if status.get("phase") != "done":
            self.send_error_json(409, f"Build not complete (phase: {status.get('phase')})")
            return

        hex_name = status.get("hex_file")
        if not hex_name:
            self.send_error_json(500, "Hex filename missing from status")
            return

        hex_path = (job_dir / hex_name).resolve()
        if not str(hex_path).startswith(str(job_dir)):
            self.send_error_json(403, "Forbidden")
            return
        if not hex_path.exists():
            self.send_error_json(404, "Hex file not found on server")
            return

        body = hex_path.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", "application/octet-stream")
        self.send_header("Content-Disposition", f'attachment; filename="{hex_name}"')
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    # ── API: /api/log/<job_id> ────────────────────────────────────────────────

    def _api_log(self, job_id):
        try:
            job_id = _sanitize(job_id, r"^[0-9a-f\-]+$")
        except ValueError:
            self.send_error_json(400, "Invalid job ID")
            return

        log_file = BUILDS_DIR / job_id / "build.log"
        if not log_file.exists():
            self.send_error_json(404, "Log not found")
            return

        body = log_file.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)

    # ── API: /api/queue ───────────────────────────────────────────────────────

    def _api_queue(self):
        with _queue_lock:
            active = [
                {"job_id": jid, "pr": v["pr"], "target": v["target"],
                 "started_at": v.get("started_at")}
                for jid, v in _active_builds.items()
            ]
        pending_size = _build_queue.qsize()
        self.send_json({"active": active, "pending_count": pending_size})


# ── main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="INAV PR Build Server")
    parser.add_argument("--host", default="0.0.0.0", help="Bind address")
    parser.add_argument("--port", type=int, default=8080, help="Listen port")
    args = parser.parse_args()

    BUILDS_DIR.mkdir(parents=True, exist_ok=True)

    # Start builder worker thread
    builder_thread = threading.Thread(target=_run_builder, daemon=True)
    builder_thread.start()

    # Start cleanup thread
    cleanup_thread = threading.Thread(target=_cleanup_loop, daemon=True)
    cleanup_thread.start()

    print(f"INAV PR Build Server listening on http://{args.host}:{args.port}")
    print(f"  INAV source: {INAV_DIR}")
    print(f"  Build script: {BUILD_SCRIPT}")
    print(f"  Data dir: {DATA_DIR}")
    print(f"  Builds dir: {BUILDS_DIR}")

    server = HTTPServer((args.host, args.port), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down...")
        _build_queue.put(None)  # Signal builder to stop
        server.server_close()


if __name__ == "__main__":
    main()

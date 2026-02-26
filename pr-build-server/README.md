# INAV PR Firmware Builder

A self-hosted web server that lets INAV community members compile and download
firmware from any open pull request for the target board they own – without
needing a local toolchain.

```
┌─────────────────────────────────────────────────────┐
│  Browser                                            │
│  ① Pick PR  ② Pick target  ③ Click "Build"         │
│  ④ Watch progress  ⑤ Download .hex                  │
└──────────────────┬──────────────────────────────────┘
                   │  HTTP
┌──────────────────▼──────────────────────────────────┐
│  server.py  (Python 3, stdlib only)                 │
│  ┌──────────────┐  ┌─────────────────────────────┐  │
│  │ Static files │  │ REST API                    │  │
│  │ web/         │  │ /api/prs  /api/targets      │  │
│  └──────────────┘  │ /api/build  /api/status/…   │  │
│                    │ /api/download/…  /api/log/… │  │
│                    └────────────┬────────────────┘  │
│                                 │ subprocess         │
│                    ┌────────────▼────────────────┐  │
│                    │ build_pr.sh                 │  │
│                    │  git worktree + merge PR    │  │
│                    │  cmake + ninja TARGET       │  │
│                    └─────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Requirements

| Component | Minimum version |
|-----------|----------------|
| Python    | 3.6 |
| bash      | 4.x |
| git       | 2.5 (worktree support) |
| cmake     | 3.13 |
| ninja     | 1.10 |
| arm-none-eabi-gcc | 10.x (matches INAV CI) |

On Ubuntu/Debian:

```bash
sudo apt-get update
sudo apt-get install -y cmake ninja-build gcc-arm-none-eabi python3
```

## Directory layout

```
pr-build-server/
├── server.py            # HTTP server (run this)
├── crontab.example      # Cron job setup
├── README.md
│
├── scripts/
│   ├── update_prs.py    # Cron: fetch open PRs from GitHub API
│   ├── prebuild.sh      # Cron: warm cmake/ninja object cache
│   └── build_pr.sh      # Called per-build: git worktree + cmake + ninja
│
├── web/                 # Static web UI
│   ├── index.html
│   ├── style.css
│   └── app.js
│
├── data/
│   ├── prs.json         # Written by update_prs.py, read by server
│   └── targets.json     # Static list of all 205 firmware targets
│
├── builds/              # Per-job directories (auto-created)
│   └── <job-uuid>/
│       ├── status.json
│       ├── build.log
│       ├── cmake_build/   # cmake + ninja working directory
│       └── *.hex          # Output firmware
│
├── worktrees/           # git worktrees for PRs (auto-created)
│   └── pr-<number>/
│
└── logs/                # Cron job logs (auto-created)
    ├── prebuild-*.log
    └── update_prs.log
```

## Quick start

### 1. Clone / copy the inav repo

The server expects to live inside the INAV git repository:

```bash
git clone https://github.com/iNavFlight/inav.git /opt/inav
cd /opt/inav
git checkout maintenance-9.x
```

### 2. Run the initial PR fetch

```bash
# Optional: set a GitHub personal access token to avoid rate limiting
export GITHUB_TOKEN=ghp_YOURTOKEN

python3 pr-build-server/scripts/update_prs.py
```

### 3. Run the prebuild (warm the cache)

This builds one seed target (`MATEKF405SE` by default) so subsequent PR builds
are faster.  It takes 5–15 minutes on the first run.

```bash
bash pr-build-server/scripts/prebuild.sh
```

You can change the seed target:

```bash
SEED_TARGET=KAKUTEH7 bash pr-build-server/scripts/prebuild.sh
```

### 4. Start the server

```bash
python3 pr-build-server/server.py --host 0.0.0.0 --port 8080
```

Open <http://your-server:8080> in a browser.

### 5. Set up cron jobs

```bash
crontab -e
```

Paste the entries from [`crontab.example`](crontab.example), adjusting paths
and your `GITHUB_TOKEN`.

## How it works

### PR list (update_prs.py)

Calls the GitHub REST API (`/repos/iNavFlight/inav/pulls`) to list all open
pull requests targeting `maintenance-9.x`.  Results are written to
`data/prs.json` and picked up immediately on the next browser page load.

### Pre-build (prebuild.sh)

1. Fetches and checks out the latest `maintenance-9.x`.
2. Runs `cmake -G Ninja` in `build_base/`.
3. Builds a single seed target (default: `MATEKF405SE`) to compile all common
   source files into `.o` object files.
4. Records the HEAD SHA so the script is a no-op when nothing has changed.

### Per-PR build (build_pr.sh)

When a user clicks **Build Firmware**:

1. `git fetch origin pull/<N>/head` – fetches the PR branch.
2. Creates (or updates) a git worktree at `worktrees/pr-<N>/` checked out to
   `maintenance-9.x`, then merges the PR into it.  If the merge fails due to
   conflicts the build is marked failed immediately.
3. If a matching pre-built base exists (`build_base/` at the same `maintenance-9.x`
   SHA), it is `rsync`-copied to `builds/<job-id>/cmake_build/` and cmake is
   re-pointed at the worktree.  Ninja then only recompiles object files whose
   source changed between the base and the PR.
4. `ninja <TARGET>` builds the requested target.
5. The resulting `.hex` is copied to `builds/<job-id>/` and the status file is
   updated to `done`.

### Server (server.py)

- Pure Python 3, no extra packages.
- A single background worker thread processes builds one at a time (configurable
  with `MAX_CONCURRENT_BUILDS`).
- Up to 10 builds can queue (`MAX_QUEUE_DEPTH`).
- Completed build artifacts are automatically cleaned up after 24 hours.

## Configuration

| Variable | Default | Where |
|----------|---------|-------|
| `BASE_BRANCH` | `maintenance-9.x` | `prebuild.sh`, `build_pr.sh`, `update_prs.py` |
| `SEED_TARGET` | `MATEKF405SE` | `prebuild.sh` |
| `NUM_CORES`   | `nproc` | `prebuild.sh`, `build_pr.sh` |
| `GITHUB_TOKEN` | *(none)* | `update_prs.py` (env) |
| `--host` / `--port` | `0.0.0.0:8080` | `server.py` CLI args |

## Security notes

- The server only accepts alphanumeric target names and numeric PR numbers.
- File paths are validated against the builds directory to prevent directory
  traversal.
- The server should be run behind a reverse proxy (nginx/caddy) if exposed to
  the internet.  Consider adding authentication (basic auth or OAuth) to
  prevent abuse of the build queue.
- GitHub token only needs read-only access to pull request metadata.

## Reverse proxy example (nginx)

```nginx
server {
    listen 80;
    server_name pr-builds.your-domain.example;

    location / {
        proxy_pass http://127.0.0.1:8080;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        # Increase timeout for long builds
        proxy_read_timeout 600;
    }
}
```

## Systemd service example

```ini
# /etc/systemd/system/inav-pr-builder.service
[Unit]
Description=INAV PR Firmware Builder
After=network.target

[Service]
Type=simple
User=inav
WorkingDirectory=/opt/inav
ExecStart=/usr/bin/python3 /opt/inav/pr-build-server/server.py --host 127.0.0.1 --port 8080
Restart=on-failure
RestartSec=10

[Install]
WantedBy=multi-user.target
```

```bash
sudo systemctl enable --now inav-pr-builder
```

#!/usr/bin/env python3
"""
OpenOCD telnet helper for RP2350 SWD debugging.

Usage examples:
    python3 openocd_helper.py sample_pc          # Sample PC 5 times
    python3 openocd_helper.py check_fault        # Read CFSR/BFAR/PC after halt
    python3 openocd_helper.py check_stack        # Halt + dump registers + stack
    python3 openocd_helper.py set_bp 0x10001234  # Set breakpoint + resume
    python3 openocd_helper.py flash <elf>        # Flash ELF via SWD (no BOOTSEL)

Requires OpenOCD already running, or launches it automatically.

Tested with:
    OOCD = ~/.pico-sdk/openocd/0.12.0+dev/openocd
    Target: RP2350 (Pico 2) via debugprobe (Pico as CMSIS-DAP, VID:PID 2e8a:000c)
    SWD wiring: debugprobe GP2->SWCLK, GP3->SWDIO on target
"""

import socket
import time
import subprocess
import sys
import os

OOCD = os.path.expanduser("~/.pico-sdk/openocd/0.12.0+dev/openocd")
SCRIPTS = os.path.expanduser("~/.pico-sdk/openocd/0.12.0+dev/scripts")
ELF = os.path.expanduser(
    "~/Documents/planes/inavflight/inav/build_rp2350/bin/RP2350_PICO.elf"
)


def start_openocd():
    proc = subprocess.Popen(
        [OOCD, "-s", SCRIPTS,
         "-f", "interface/cmsis-dap.cfg",
         "-f", "target/rp2350.cfg",
         "-c", "adapter speed 5000",
         "-c", "init"],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    time.sleep(3)
    return proc


def connect():
    s = socket.socket()
    s.connect(("127.0.0.1", 4444))
    time.sleep(0.5)
    s.recv(4096)  # discard banner
    return s


def cmd(s, c, delay=0.5):
    s.sendall((c + "\n").encode())
    time.sleep(delay)
    r = b""
    s.settimeout(0.4)
    try:
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            r += chunk
    except Exception:
        pass
    return r.decode(errors="replace")


def nm_lookup(addrs):
    """Return {addr: funcname} for a list of PC values."""
    result = {}
    try:
        syms = []
        out = subprocess.check_output(
            ["arm-none-eabi-nm", ELF], stderr=subprocess.DEVNULL
        ).decode()
        for line in out.splitlines():
            parts = line.strip().split()
            if len(parts) >= 3 and parts[1].upper() in ("T", "t"):
                syms.append((int(parts[0], 16), parts[2]))
        syms.sort()
        for t in addrs:
            for i, (addr, name) in enumerate(syms):
                if addr > t:
                    prev = syms[i - 1]
                    result[t] = f"{name} (+0x{t - prev[0]:x})"
                    break
    except Exception as e:
        print(f"nm lookup failed: {e}")
    return result


# ── commands ──────────────────────────────────────────────────────────────────

def sample_pc(n=5):
    proc = start_openocd()
    s = connect()
    pcs = []
    for _ in range(n):
        cmd(s, "halt", 0.3)
        r = cmd(s, "reg pc", 0.3)
        for line in r.splitlines():
            if "pc (/32)" in line:
                pcs.append(int(line.split(":")[1].strip(), 16))
        cmd(s, "resume", 0.3)
        time.sleep(0.5)
    cmd(s, "shutdown", 0.3)
    proc.wait(timeout=5)

    names = nm_lookup(pcs)
    for pc in pcs:
        print(f"  0x{pc:08x}  {names.get(pc, '?')}")


def check_fault():
    """Halt and read fault status registers + exception frame."""
    proc = start_openocd()
    s = connect()

    print(cmd(s, "halt"))
    print("PC:", cmd(s, "reg pc"))
    print("SP:", cmd(s, "reg sp"))
    print("LR:", cmd(s, "reg lr"))

    # Fault status
    cfsr_raw = cmd(s, "mdw 0xe000ed28")  # CFSR
    hfsr_raw = cmd(s, "mdw 0xe000ed2c")  # HFSR
    bfar_raw = cmd(s, "mdw 0xe000ed38")  # BFAR
    print("CFSR:", cfsr_raw)
    print("HFSR:", hfsr_raw)
    print("BFAR:", bfar_raw)

    # Exception frame: INAV HardFault handler pushes {r7,lr}+sub sp,#8 → frame at SP+16
    sp_r = cmd(s, "reg sp")
    for line in sp_r.splitlines():
        if "sp (/32)" in line:
            sp = int(line.split(":")[1].strip(), 16)
            frame = sp + 16
            print(f"\nException frame at 0x{frame:08x}:")
            print(cmd(s, f"mdw 0x{frame:08x} 8"))
            # frame+24 = faulting PC
            fpc_raw = cmd(s, f"mdw 0x{frame + 24:08x}")
            print("Faulting PC word:", fpc_raw)
            for w in fpc_raw.split():
                try:
                    fpc = int(w, 16)
                    if 0x10000000 <= fpc <= 0x10400000:
                        names = nm_lookup([fpc])
                        print(f"  -> {names.get(fpc, '?')}")
                except ValueError:
                    pass
            break

    cmd(s, "shutdown", 0.3)
    proc.wait(timeout=5)


def check_stack():
    """Halt and dump registers + top of stack."""
    proc = start_openocd()
    s = connect()

    print(cmd(s, "halt"))
    for reg in ("pc", "sp", "lr", "r0", "r1"):
        print(cmd(s, f"reg {reg}"))

    sp_r = cmd(s, "reg sp")
    for line in sp_r.splitlines():
        if "sp (/32)" in line:
            sp = int(line.split(":")[1].strip(), 16)
            print(f"Stack top (sp=0x{sp:08x}, 16 words):")
            print(cmd(s, f"mdw 0x{sp:08x} 16"))
            break

    # CFSR
    print("CFSR:", cmd(s, "mdw 0xe000ed28"))
    print("BFAR:", cmd(s, "mdw 0xe000ed38"))

    cmd(s, "shutdown", 0.3)
    proc.wait(timeout=5)


def flash_elf(elf_path=None):
    """Flash ELF via SWD — no BOOTSEL mode required."""
    elf = elf_path or ELF
    result = subprocess.run(
        [OOCD, "-s", SCRIPTS,
         "-f", "interface/cmsis-dap.cfg",
         "-f", "target/rp2350.cfg",
         "-c", "adapter speed 5000",
         "-c", f"program {elf} verify reset exit"],
    )
    return result.returncode == 0


# ── main ──────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    op = sys.argv[1] if len(sys.argv) > 1 else "sample_pc"
    if op == "sample_pc":
        sample_pc(int(sys.argv[2]) if len(sys.argv) > 2 else 5)
    elif op == "check_fault":
        check_fault()
    elif op == "check_stack":
        check_stack()
    elif op == "flash":
        ok = flash_elf(sys.argv[2] if len(sys.argv) > 2 else None)
        sys.exit(0 if ok else 1)
    else:
        print(__doc__)
        sys.exit(1)

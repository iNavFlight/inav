#!/usr/bin/env python

from __future__ import print_function

import os
import socket
import subprocess
import sys

def openocd_telnet_await_prompt(s):
    prev = None
    while True:
        b = s.recv(1)
        if b == '':
            # Closed
            return False
        if prev == '>' and b == ' ':
            # Prompt for next command
            return True
        prev = b
        print(b, end='')

def openocd_telnet_command(s, cmd):
    s.send(cmd + '\n')
    openocd_telnet_await_prompt(s)

def openocd_flash_telnet(port, filename):
    try:
        s = socket.create_connection(('localhost', port))
    except socket.error:
        return False

    openocd_telnet_await_prompt(s)
    openocd_telnet_command(s, 'halt')
    openocd_telnet_command(s, 'program {} verify reset\n'.format(filename))
    openocd_telnet_command(s, 'exit')
    s.close()
    return True

def openocd_flash_cmd(openocd, args, filename):
    cmd = [openocd]
    cmd.extend(args)
    cmd.extend(('-c', 'program {} verify reset exit'.format(filename)))
    status = subprocess.call(cmd)
    return status == 0

def usage():
    print('Usage: {} <openocd_args> <elf_file>'.format(sys.argv[0]))
    print('Environment variables: OPENOCD_CMD = path to openocd')
    sys.exit(1)

def main():
    import sys

    # Default openocd telnet port
    # TODO: Parse arguments and check if we
    # should use a non-default port
    port = 4444
    openocd = os.environ.get('OPENOCD_CMD') or 'openocd'

    openocd_args = []
    flag = None
    elf = None
    for arg in sys.argv[1:]:
        if flag:
            openocd_args.append(arg)
            flag = None
        else:
            if arg.startswith('-'):
                openocd_args.append(arg)
                flag = arg
            elif elf is None:
                elf = arg
            else:
                usage()

    if len(openocd_args) == 0 or elf is None:
        usage()

    if not openocd_flash_telnet(port, elf):
        if not openocd_flash_cmd(openocd, openocd_args, elf):
            print('could not flash')
            sys.exit(1)

if __name__ == '__main__':
    main()

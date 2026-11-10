#!/usr/bin/env python3
"""Serial to TCP bridge for bench testing the MassZero thermal camera driver.

Connects a camera on a local serial port to a SITL instance listening on TCP,
for example the UART the camera function is assigned to under
"serial port emulation". Run with --verbose for a hex dump of every transfer.
"""

import argparse
import socket
import sys
import threading

import serial  # pyserial


def hexdump(data, limit=None):
    shown = data if limit is None else data[:limit]
    text = ' '.join('%02X' % b for b in shown)
    if limit is not None and len(data) > limit:
        text += '...'
    return text


class Bridge:
    def __init__(self, ser, sock, verbose):
        self.ser = ser
        self.sock = sock
        self.verbose = verbose
        # One worker hitting EOF or an error has to stop the other, or the
        # bridge hangs half open and never becomes ready for a reconnect.
        self.shutdown = threading.Event()

    def stop(self):
        self.shutdown.set()
        # Unblock the workers that sit in recv() and read().
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        try:
            self.ser.cancel_read()
        except (AttributeError, OSError):
            pass

    def serial_to_tcp(self):
        try:
            while not self.shutdown.is_set():
                data = self.ser.read(1024)
                if not data:
                    continue
                # sendall retries partial writes. A bare send() under TCP
                # backpressure would silently drop the tail of a packet.
                self.sock.sendall(data)
                if self.verbose:
                    print('[SERIAL->TCP] %d bytes: %s' % (len(data), hexdump(data)))
        except OSError as e:
            if not self.shutdown.is_set():
                print('Serial->TCP: %s' % e)
        finally:
            self.stop()

    def tcp_to_serial(self):
        try:
            while not self.shutdown.is_set():
                data = self.sock.recv(1024)
                if not data:
                    print('TCP peer closed the connection')
                    break
                self.ser.write(data)
                self.ser.flush()
                if self.verbose:
                    print('[TCP->SERIAL] %d bytes: %s' % (len(data), hexdump(data)))
        except OSError as e:
            if not self.shutdown.is_set():
                print('TCP->Serial: %s' % e)
        finally:
            self.stop()

    def run(self):
        workers = [
            threading.Thread(target=self.serial_to_tcp, daemon=True),
            threading.Thread(target=self.tcp_to_serial, daemon=True),
        ]
        for w in workers:
            w.start()
        for w in workers:
            w.join()


def parse_args():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--serial', required=True,
                   help='serial port the camera is on, for example COM13 or /dev/ttyUSB0')
    p.add_argument('--baud', type=int, default=115200,
                   help='serial baud rate (default: 115200)')
    p.add_argument('--host', default='127.0.0.1',
                   help='SITL host to connect to (default: 127.0.0.1)')
    p.add_argument('--port', type=int, default=5762,
                   help='SITL TCP port to connect to (default: 5762)')
    p.add_argument('--verbose', action='store_true',
                   help='hex dump every transfer')
    return p.parse_args()


def main():
    args = parse_args()

    try:
        ser = serial.Serial(args.serial, args.baud, timeout=0.1)
    except (OSError, serial.SerialException) as e:
        print('Failed to open %s: %s' % (args.serial, e))
        return 1

    print('Serial: %s @ %d' % (args.serial, args.baud))

    try:
        while True:
            try:
                sock = socket.create_connection((args.host, args.port))
            except OSError as e:
                print('Failed to connect to %s:%d: %s' % (args.host, args.port, e))
                return 1

            print('Connected to SITL on %s:%d' % (args.host, args.port))
            try:
                Bridge(ser, sock, args.verbose).run()
            finally:
                sock.close()
            print('Link dropped. Reconnecting. Interrupt to stop.')
    except KeyboardInterrupt:
        print('Stopped')
        return 0
    finally:
        ser.close()


if __name__ == '__main__':
    sys.exit(main())

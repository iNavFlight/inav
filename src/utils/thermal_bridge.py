#!/usr/bin/env python3
"""
Simple TCP-Serial bridge for connecting Windows COM port to INAV SITL in WSL
"""

import serial
import socket
import threading
import sys

# Configuration
SERIAL_PORT = 'COM13'  # Your thermal camera port
SERIAL_BAUD = 115200
TCP_HOST = '172.18.102.124'  # Connect to localhost (SITL)
TCP_PORT = 5762

def forward_data(source, destination, name):
    """Forward data from source to destination"""
    try:
        while True:
            if hasattr(source, 'read'):  # Serial source
                data = source.read(1024)
            else:  # Socket source
                data = source.recv(1024)
                if not data:
                    print(f"{name}: Connection closed")
                    break
            
            if data:
                if hasattr(destination, 'write'):  # Serial destination
                    destination.write(data)
                else:  # Socket destination
                    destination.send(data)
                # Enhanced debugging with hex dump
                hex_str = ' '.join([f'{b:02X}' for b in data[:20]])
                if len(data) > 20:
                    hex_str += '...'
                print(f"{name}: {len(data)} bytes: {hex_str}")
    except Exception as e:
        print(f"{name}: Error - {e}")

def main():
    print(f"MassZero Thermal Camera Bridge")
    print(f"Serial: {SERIAL_PORT} @ {SERIAL_BAUD}")
    print(f"TCP: Connecting to {TCP_HOST}:{TCP_PORT}")
    
    # Open serial port
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=0.1)
        print(f"Opened {SERIAL_PORT}")
    except Exception as e:
        print(f"Failed to open {SERIAL_PORT}: {e}")
        sys.exit(1)
    
    # Connect to SITL's TCP server
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((TCP_HOST, TCP_PORT))
        print(f"Connected to SITL on {TCP_HOST}:{TCP_PORT}")
        
        # Create forwarding threads
        t1 = threading.Thread(target=forward_data, 
                             args=(ser, sock, "Serial->TCP"))
        t2 = threading.Thread(target=forward_data, 
                             args=(sock, ser, "TCP->Serial"))
        
        t1.daemon = True
        t2.daemon = True
        t1.start()
        t2.start()
        
        # Wait for threads to finish
        t1.join()
        t2.join()
        
    except Exception as e:
        print(f"Failed to connect to SITL: {e}")
    finally:
        ser.close()
        sock.close()

if __name__ == "__main__":
    main()
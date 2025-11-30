#!/usr/bin/env python3
"""
Debug version of TCP-Serial bridge with better logging
"""

import serial
import socket
import threading
import sys
import time

# Configuration
SERIAL_PORT = 'COM13'  # Your thermal camera port
SERIAL_BAUD = 115200
TCP_HOST = '172.18.102.124'  # Connect to localhost (SITL)
TCP_PORT = 5762

def forward_serial_to_tcp(ser, sock):
    """Forward data from serial to TCP with detailed logging"""
    print("Serial->TCP thread started")
    while True:
        try:
            # Read with timeout
            data = ser.read(1024)
            if data:
                # Log the actual data received
                hex_str = ' '.join([f'{b:02X}' for b in data])
                print(f"[SERIAL RX] {len(data)} bytes: {hex_str}")
                
                # Send to TCP
                sent = sock.send(data)
                print(f"[TCP TX] Sent {sent} bytes")
        except Exception as e:
            print(f"Serial->TCP Error: {e}")
            break

def forward_tcp_to_serial(sock, ser):
    """Forward data from TCP to serial with detailed logging"""
    print("TCP->Serial thread started")
    while True:
        try:
            data = sock.recv(1024)
            if not data:
                print("TCP connection closed")
                break
                
            # Log the data
            hex_str = ' '.join([f'{b:02X}' for b in data])
            print(f"[TCP RX] {len(data)} bytes: {hex_str}")
            
            # Send to serial
            written = ser.write(data)
            ser.flush()
            print(f"[SERIAL TX] Sent {written} bytes")
            
        except Exception as e:
            print(f"TCP->Serial Error: {e}")
            break

def main():
    print(f"MassZero Thermal Camera Bridge (DEBUG MODE)")
    print(f"Serial: {SERIAL_PORT} @ {SERIAL_BAUD}")
    print(f"TCP: Connecting to {TCP_HOST}:{TCP_PORT}")
    print("-" * 50)
    
    # Open serial port
    try:
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=0.1)
        print(f"✓ Opened {SERIAL_PORT}")
        
        # Clear any pending data
        ser.read_all()
        print("✓ Cleared serial buffer")
        
    except Exception as e:
        print(f"✗ Failed to open {SERIAL_PORT}: {e}")
        sys.exit(1)
    
    # Connect to SITL's TCP server
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((TCP_HOST, TCP_PORT))
        print(f"✓ Connected to SITL on {TCP_HOST}:{TCP_PORT}")
        
        # Test: Send a read model command to see if camera responds
        print("\nSending test command to camera...")
        test_cmd = bytearray([0xF0, 0x04, 0x36, 0x74, 0x02, 0x01, 0xAD, 0xFF])
        ser.write(test_cmd)
        ser.flush()
        time.sleep(0.2)
        
        response = ser.read(256)
        if response:
            hex_str = ' '.join([f'{b:02X}' for b in response])
            print(f"Camera test response: {hex_str}")
        else:
            print("No response from camera to test command")
        
        print("\nStarting bridge threads...")
        
        # Create forwarding threads
        t1 = threading.Thread(target=forward_serial_to_tcp, 
                             args=(ser, sock))
        t2 = threading.Thread(target=forward_tcp_to_serial, 
                             args=(sock, ser))
        
        t1.daemon = True
        t2.daemon = True
        t1.start()
        t2.start()
        
        print("Bridge is running. Press Ctrl+C to stop.\n")
        
        # Wait for threads
        t1.join()
        t2.join()
        
    except Exception as e:
        print(f"✗ Failed to connect to SITL: {e}")
    finally:
        ser.close()
        sock.close()

if __name__ == "__main__":
    main()


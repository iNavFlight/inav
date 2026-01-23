#!/usr/bin/env python3
"""
Test script to verify thermal camera communication on COM13
"""

import serial
import time

# Configuration
SERIAL_PORT = 'COM13'
SERIAL_BAUD = 115200

def send_command(ser, class_cmd, subclass_cmd, flags=0x01, data=None):
    """Send a command packet to the thermal camera"""
    if data is None:
        data = []
    
    # Build packet
    packet = bytearray()
    packet.append(0xF0)  # Start byte
    packet.append(len(data) + 4)  # Size
    packet.append(0x36)  # Device address
    packet.append(class_cmd)
    packet.append(subclass_cmd)
    packet.append(flags)
    packet.extend(data)
    
    # Calculate checksum
    checksum = 0x36 + class_cmd + subclass_cmd + flags + sum(data)
    packet.append(checksum & 0xFF)
    packet.append(0xFF)  # End byte
    
    # Send packet
    print(f"Sending: {' '.join([f'{b:02X}' for b in packet])}")
    ser.write(packet)
    
    # Wait for response
    time.sleep(0.1)
    response = ser.read(256)
    if response:
        print(f"Response: {' '.join([f'{b:02X}' for b in response])}")
        return response
    else:
        print("No response")
        return None

def main():
    print("Testing MassZero Thermal Camera on COM13")
    print("-" * 50)
    
    try:
        # Open serial port
        ser = serial.Serial(SERIAL_PORT, SERIAL_BAUD, timeout=1.0)
        print(f"Opened {SERIAL_PORT} at {SERIAL_BAUD} baud")
        
        # Clear any pending data
        ser.read_all()
        
        # Test 1: Read device model
        print("\n1. Reading device model...")
        send_command(ser, 0x74, 0x02)
        
        # Test 2: Read FPGA version
        print("\n2. Reading FPGA version...")
        send_command(ser, 0x74, 0x03)
        
        # Test 3: Read software version
        print("\n3. Reading software version...")
        send_command(ser, 0x74, 0x05)
        
        # Test 4: Read initialization status
        print("\n4. Reading initialization status...")
        send_command(ser, 0x7C, 0x14)
        
        # Test 5: Try to set brightness (write command)
        print("\n5. Setting brightness to 50...")
        send_command(ser, 0x78, 0x02, flags=0x00, data=[50])
        
        ser.close()
        
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()


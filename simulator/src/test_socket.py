import socket
import time
import math

HOST = "127.0.0.1"
PORT = 2323
RATE_HZ = 60

import random

# Amplitude in degrees
AMPLITUDE = 30.0

freq_roll  = random.uniform(0.1, 0.5)
freq_pitch = random.uniform(0.1, 0.5)
freq_yaw   = random.uniform(0.1, 0.5)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    server.bind((HOST, PORT))
    server.listen(1)
    print(f"Listening on {HOST}:{PORT}")

    conn, addr = server.accept()
    with conn:
        print(f"Client connected from {addr}")

        period = 1.0 / RATE_HZ
        
        start_time = time.time()

        try:
            while True:
                t = time.time() - start_time  # relative time

                # Roll, pitch, yaw oscillating sine wave
                roll  = AMPLITUDE * math.sin(2 * math.pi * 0.20 * t)
                pitch = AMPLITUDE * math.sin(2 * math.pi * -0.05 * t)
                yaw   = AMPLITUDE * math.sin(2 * math.pi * 0.10 * t)

                # Send semicolon-separated string in degrees
                msg = f"{t:.6f};{roll:.6f};{pitch:.6f};{yaw:.6f}\n"
                conn.sendall(msg.encode("utf-8"))

                time.sleep(period)
        except KeyboardInterrupt:
            print("\nCtrl+C pressed! Exiting gracefully...")
            
        print("Done sending, closing connection.")

    
    server.close()
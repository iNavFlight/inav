import socket
import time
import math
import argparse

from src.inav_interface import InavSimulate

HOST = "127.0.0.1"
PORT = 2323
RATE_HZ = 60


def main(args):
    
    inav_connect = False
    
    if args.sim == "inav":
        inav_connect = True
    
    if inav_connect:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        server.bind((HOST, PORT))
        server.listen(1)
        print(f"Listening on {HOST}:{PORT}")

        conn, addr = server.accept()
        print(f"Client connected from {addr}")
            
        
    Inav = InavSimulate(run_isolated=(not inav_connect))
     
    dt = 1.0 / RATE_HZ
    start_time = time.time()

    try:
        while True:
            t = time.time() - start_time  # relative time
            
            if inav_connect:
                Inav.rx(conn)
            
            Inav.update(t)
            
            if inav_connect:
                Inav.tx(conn)

            time.sleep(dt)
    except KeyboardInterrupt:
        print("\nCtrl+C pressed! Exiting gracefully...")
    
    if inav_connect:
        server.close()

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description="Simulator entry point")
    parser.add_argument(
        "--sim",
        type=str,
        default=None,
        choices=["inav"],
        help="Select simulator backend (e.g. inav)"
    )

    args = parser.parse_args()
    
    main(args)
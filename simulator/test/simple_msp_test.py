
import os, sys, time, struct
import numpy as np
from unavlib import MSPy
from unavlib.enums.msp_codes import MSPCodes 


#
# https://github.com/iNavFlight/inav/wiki/MSP-V2
# https://github.com/iNavFlight/inav/blob/master/docs/development/msp/msp_ref.md
#

def main():
    
    run({}, serial_port="/dev/pts/16")
    

def run(mdct, serial_port="/dev/serial0"):
    
    LOOP_DT = 0.046
    
    with MSPy(device=serial_port, baudrate=230400, logfilename='mspout.log', logfilemode='w', loglevel='ERROR') as fc:
    
        time.sleep(0.05)
        
        if fc==1:
            raise RuntimeError(f"Cannot read from serial port {serial_port}")
        
        data = {
            "ch1": 1010,
            "ch2": 1020,
            "ch3": 1030,
            "ch4": 1040,
            "ch5": 1050,
            "ch6": 1060,
            "ch7": 1070,
            "ch8": 1080,
        }
        
    
        format = '<hhhhhhhh'
        bytes = struct.pack(format, *[int(i) for i in data.values()])
        fc.send_RAW_msg(MSPCodes['MSP_SET_RAW_RC'], data=bytes)
        
        


if __name__ == "__main__":
    main()
    
# ------------------------------ #
#   Python struct formats
# ------------------------------ #
#    b   signed char
#    B   unsigned char
#    h   signed short
#    H   unsigned shot
#    i   int32
#    I   unsigned int32
#    l   long
#    L   unsigned long
#    f   float
# ------------------------------ #
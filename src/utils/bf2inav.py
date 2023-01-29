#!/usr/bin/python3
# This script will read a betaflight unified target config
# and generate an INAV target.
#
# The generated target can be used as starting point to a new target.
#
# The generated target will not include any servo outputs, or FW output
# configurations.
#
# It will setup timers, DMA, IMU hardware, I2C and SPI busses and  Blackbox interfaces.
import sys
import os
import io
import getopt
import re
import json

version = '0.1'

def translateFunctionName(bffunction, index):
    return bffunction + '_' + index

def translatePin(bfpin):
    pin = re.sub("^([A-Z])0*(\d+)$", r'P\1\2', bfpin)
    return pin

def buildMap(inputFile):
    map = { 'defines': [], 'features': [], 'pins': {}, 'serial': {}, 'variables': {}}


    f = open(inputFile, 'r')
    while True:
        l = f.readline()
        if not l:
            break
        m = re.search(r'^#mcu\s+([0-9A-Za-z]+)$', l)
        if m:
            map['mcu'] = {'type': m.group(1)}
        
        m = re.search(r'^#define\s+(\w+)$', l)
        if m:
            map['defines'].append(m.group(1))

        m = re.search(r'^feature\s+(-?\w+)$', l)
        if m:
            map['features'].append(m.group(1))


        m = re.search(r'^resource\s+(-?\w+)\s+(\d+)\s+(\w+)$', l)
        if m:
            resource_type = m.group(1)
            resource_index = m.group(2)
            pin = translatePin(m.group(3))
            if not map['pins'].get(pin):
                map['pins'][pin] = {}

            map['pins'][pin]['function'] = translateFunctionName(resource_type, resource_index)

        m = re.search(r'^timer\s+(\w+)\s+AF(\d+)$', l)
        if m:
            pin = translatePin(m.group(1))
            if not map['pins'].get(pin):
                map['pins'][pin] = {}
            
            map['pins'][pin]['AF'] = m.group(2)

        m = re.search(r'^#\s*pin\s+(\w+):\s*(TIM\d+)\s+(CH\d+).+$', l)
        if m:
            pin = translatePin(m.group(1))
            if not map['pins'].get(pin):
                map['pins'][pin] = {}
            
            map['pins'][pin]['TIM'] = m.group(2)
            map['pins'][pin]['CH'] = m.group(3)
        
        m = re.search(r'^dma\s+([A-Za-z0-9]+)\s+([A-Za-z0-9]+)\s+(\d+).*$', l)
        if m:

            if(m.group(1) == 'ADC'):
                pin = 'ADC' + m.group(2)
            else:
                pin = translatePin(m.group(2))
            if not map['pins'].get(pin):
                map['pins'][pin] = {}
            
            map['pins'][pin]['DMA'] = m.group(3)

#      1     2         3         4
# pin B04: DMA1 Stream 4 Channel 5
    
        m = re.search(r'^#\s+pin\s+(\w+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Channel\s+(\d+)\s*$', l)
        if m:
            pin = translatePin(m.group(1))
            if not map['pins'].get(pin):
                map['pins'][pin] = {}
            
            map['pins'][pin]['DMA_STREAM'] = m.group(3)
            map['pins'][pin]['DMA_CHANNEL'] = m.group(4)
        
        m = re.search(r'^#\s+ADC\s+(\d+):\s+(DMA\d+)\s+Stream\s+(\d+)\s+Channel\s+(\d+)\s*$', l)
        if m:
            pin = 'ADC' + m.group(1)
            if not map['pins'].get(pin):
                map['pins'][pin] = {}
            
            map['pins'][pin]['DMA_STREAM'] = m.group(3)
            map['pins'][pin]['DMA_CHANNEL'] = m.group(4)
 
#serial 0 64 115200 57600 0 115200
        m = re.search(r'^serial\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$', l)
        if m:
            idx = m.group(1)
            if not map['serial'].get(idx):
                map['serial'][idx] = {}
            
            map['serial'][idx]['FUNCTION'] = m.group(2)
            map['serial'][idx]['MSP_BAUD'] = m.group(3)
            map['serial'][idx]['GPS_BAUD'] = m.group(4)
            map['serial'][idx]['TELEMETRY_BAUD'] = m.group(5)
            map['serial'][idx]['BLACKBOX_BAUD'] = m.group(6)

        m = re.search(r'^set\s+(\w+)\s*=\s*(\w+)$', l)
        if m:
            map['variables'][m.group(1)] = m.group(2)
 

    return map

def printHelp():
    print ("%s -i bf-target.config -o output-directory" % (sys.argv[0]))
    print ("    -i | --input-config=<file>     -- print this help")
    print ("    -o | --output-dir=<targetdir>  -- print this help")
    print ("    -h | --help                    -- print this help")
    print ("    -v | --version                 -- print version")
    return

def main(argv):
    inputfile = ''
    outputdir = '.'
    global version

    try:
        opts, args = getopt.getopt(argv,"hvi:o:", ["input-config=", "output-dir=", 'version', 'help'])
    except getopt.GeoptError:
        printHelp()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            printHelp()
            sys.exit(1)
        elif opt in ('-i', '--input-config'):
            inputfile = arg
        elif opt in ('-o', '--output-dir'):
            outputdir = arg
        elif opt in ('-v', '--version'):
            print ("%s: %s" % (sys.argv[0], version))
            sys.exit(0)

    if (os.path.exists(inputfile) and os.path.isdir(outputdir)):
        targetDefinition = buildMap(inputfile)
    else:
        printHelp()
        sys.exit(2)


    map = buildMap(inputfile)

    print (json.dumps(map, indent=4))

    sys.exit(0)


if( __name__ == "__main__"):
    main(sys.argv[1:])



#!/usr/bin/env python3
# https://developer.thingstream.io/guides/location-services/assistnow-user-guide
#
# Create a file named tokens.yaml
# Add your assist now tokens
# assistnow_online: XXXXXXXXXXX
# assistnow_offline: XXXXXXXXXXX

import requests
import yaml
import sys
import socket
import selectors
import types
import serial
import getopt
import io
import time

online_token = ""
offline_token = ""
passthrough = False
token_file = "tokens.yaml"
serial_port = None
dry_run = False


# UBX frame
# 0xB5
# 0x62
# 0x?? class
# 0x?? id
# 0x?? len low
# 0x?? len high
# 0x?? len bytes payload
# 0x?? crc1
# 0x?? crc2
# Total len = 8 + len  

hasFirstHeader = False
hasSecondHeader = False
ubxClass = False
ubxId = False
lenLow = False
lenHigh = False
payloadLen = 0
skipped = 0
currentCommand = []

def resetUbloxState():
    global hasFirstHeader
    global hasSecondHeader
    global ubxClass
    global ubxId
    global lenLow
    global lenHigh
    global payloadLen
    global skipped
    global currentCommand

    hasFirstHeader = False
    hasSecondHeader = False
    ubxClass = False
    ubxId = False
    lenLow = False
    lenHigh = False
    payloadLen = 0
    skipped = 0
    currentCommand = []

def splitUbloxCommands(ubxBytes):
    ubxCommands = []
    global hasFirstHeader
    global hasSecondHeader
    global ubxClass
    global ubxId
    global lenLow
    global lenHigh
    global payloadLen
    global skipped
    global currentCommand

    resetUbloxState()

    #print("%s" % (type(ubxBytes)))
    #print("len: %i" % (len(ubxBytes)))

    for i in range(len(ubxBytes)):
        if not hasFirstHeader:
            if ubxBytes[i] == 0xb5:
                hasFirstHeader = True
                currentCommand.append(ubxBytes[i])
                continue
            else:
                resetUbloxState()
                continue
        if not hasSecondHeader:
            if ubxBytes[i] == 0x62:
                hasSecondHeader = True
                currentCommand.append(ubxBytes[i])
                continue
            else:
                resetUbloxState()
                continue
        if not ubxClass:
            ubxClass = True
            #print ("ubxClass: %02x" % (ubxBytes[i]))
            currentCommand.append(ubxBytes[i])
            continue
        if not ubxId:
            ubxId = True
            #print ("ubxId: %02x" % (ubxBytes[i]))
            currentCommand.append(ubxBytes[i])
            continue
        if not lenLow:
            lenLow = True
            payloadLen = int(ubxBytes[i])
            currentCommand.append(ubxBytes[i])
            continue
        if not lenHigh:
            lenHigh = True
            payloadLen = (int(ubxBytes[i]) << 8) | payloadLen
            #print ("Payload len %i" % (payloadLen))
            payloadLen += 2 # add crc bytes
            currentCommand.append(ubxBytes[i])
            continue
        if skipped < payloadLen - 1:
            skipped = skipped + 1
            currentCommand.append(ubxBytes[i])
            continue
        if skipped == payloadLen - 1:
            skipped = skipped + 1
            currentCommand.append(ubxBytes[i])
            ubxCommands.append(currentCommand)
            resetUbloxState()
            continue;
    
    return ubxCommands

def loadTokens(file):
    global online_token
    global offline_token
    yaml_config = {}

    with open(file, "r") as stream:
        try:
            yaml_config = yaml.safe_load(stream)
            online_token = yaml_config['assistnow_online']
            offline_token = yaml_config['assistnow_online']
        except yaml.YAMLError as exc:
            print(exc)
        stream.close()

def crc8_dvb_s2( crc:int,  b:int) -> int:
    crc ^= b
    for ii in range(8):
        if (crc & 0x80) == 0x80:
            crc = ((crc << 1) ^ 0xD5) & 0xFF
        else:
            crc = (crc << 1) & 0xFF

    return int(crc & 0xFF)

def ubloxToMsp(ubxCmd):
    ubloxLen = len(ubxCmd)
    #msp = bytearray(b"$X<\x00d\x00\x00\x00\x8F")
    crc = 0
    msp = bytearray(b"$X<\x00\x50\x20")
    crc = crc8_dvb_s2(crc, 0x00)
    crc = crc8_dvb_s2(crc, 0x50)
    crc = crc8_dvb_s2(crc, 0x20)
    msp.append(ubloxLen & 0xFF)
    crc = crc8_dvb_s2(crc, ubloxLen & 0xFF)
    msp.append((ubloxLen >> 8) & 0xFF)
    crc = crc8_dvb_s2(crc, (ubloxLen >> 8) & 0xFF)

    if(len(msp) != 8):
        print ("Wrong size")

    for i in range(ubloxLen):
        msp.append(ubxCmd[i])
        crc = crc8_dvb_s2(crc, ubxCmd[i])
    
    #print ("msp: %s" % (bytes(msp)))

    msp.append(crc & 0xFF)
    print ("CRC: %i" % (crc))

    return bytes(msp)


def usage():
    print ("assistnow.py -s /dev/ttyS0 [-t tokens.yaml]")

def sendUbxMessages(s, ubxMessages):
    printed = 0
    for cmd in ubxMessages:
        printed += 1
        if(len(msp) > 8):
            print ("%i/%i ubx: %i" % (printed, len(ubxMessages), len(cmd)))
            try:
                s.write(cmd)
            except serial.SerialException as err:
                print (err)
                print (cmd)
                break
            #time.sleep(0.1)


def sendMspMessages(s, ubxMessages):
    printed = 0
    for cmd in ubxMessages:
        msp = ubloxToMsp(cmd)
        #msp = bytearray(b"$X<\x00d\x00\x00\x00\x8F")
        printed += 1
        if(len(msp) > 8):
            print ("%i/%i msp: %i ubx: %i" % (printed, len(ubxMessages), len(msp), len(cmd)))
            try:
                s.write(msp)
                #s.sendall(msp)
            except serial.SerialException as err:
                print (err)
                print (cmd)
                print (msp)
                break
            #time.sleep(1.0)

try:
    opts, args = getopt.getopt(sys.argv[1:], "s:t:pd", ["serial=", "tokens=", "passthrough", "dry-run"])
except getopt.GetoptError as err:
    # print help information and exit:
    print(err)  # will print something like "option -a not recognized"
    usage()
    sys.exit(2)

for o, a in opts:
    if o in ("-s", "--serial="):
        serial_port = a
    elif o in ("-t", "--tokens="):
        token_file = a
    elif o in ("-p", "--passthrough"):
        passthrough = True
    elif o in ("-d", "--dry-run"):
        dry_run = True
    else:
        usage()
        sys.exit(2)

#if serial_port == None and not dry_run:
#    usage()
#    sys.exit(2)

loadTokens(token_file)


#m8 only
fmt="mga"
gnss="gps,gal,bds,glo,qzss"
offline_gnss="gps,gal,bds,glo"
#m8 only
alm="gps,qzss,gal,bds,glo"

online_servers = ['online-live1.services.u-blox.com', 'online-live2.services.u-blox.com']
#m8 format = mga
#m7 format = aid
url = "https://online-live1.services.u-blox.com/GetOnlineData.ashx?token=" + online_token + ";gnss=" + gnss + ";datatype=eph,alm,aux,pos;format=" + fmt + ";"
online_req = requests.get(url)

print (online_req)
#print (online_req.content)
print (len(online_req.content))
online = io.BytesIO(online_req.content)
online_bytes = online.read()
online_cmds = splitUbloxCommands(online_bytes)
print ("AssitnowOnline: %i" % (len(online_cmds)))

max_payload = 0
max_msp_payload = 0
for cmd in online_cmds:
    if len(cmd) > max_payload:
        max_payload = len(cmd)
        max_msp_payload = len(ubloxToMsp(cmd))
        print ("Max ubx payload: %i" % (max_payload))
        print ("Max msp payload: %i" % (max_msp_payload))

of = open("aon.ubx", "wb")
of.write(online_req.content)
of.close()

period = "5"

offline_servers = ['offline-live1.services.u-blox.com', 'offline-live2.services.u-blox.com']
offline_url = "https://offline-live1.services.u-blox.com/GetOfflineData.ashx?token=" + offline_token + ";gnss=" + offline_gnss + ";format=" + fmt + ";period=" + period + ";resolution=1;alm=" + alm + ";"
offline_req =  requests.get(offline_url)

print(offline_req)
#print(offline_req.content)
off = io.BytesIO(offline_req.content)
print(len(offline_req.content))
offline_bytes = off.read()
offline_cmds = splitUbloxCommands(offline_bytes)
print ("AssitnowOffline: %i" % (len(offline_cmds)))

for cmd in offline_cmds:
    if len(cmd) > max_payload:
        max_payload = len(cmd)
        print ("Max ubx payload: %i" % (max_payload))
        max_msp_payload = len(ubloxToMsp(cmd))
        print ("Max msp payload: %i" % (max_msp_payload))

of = open("aoff.ubx", "wb")
of.write(offline_req.content)
of.close()

print ("Connecting...")
s = serial.Serial(serial_port, 230400)
#s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#s.connect(('localhost', 5760))
print ("Connected.")

if not dry_run:
    if not passthrough:
        print ("Offline cmds...")
        sendMspMessages(s, offline_cmds)
        print ("Online cmds...")
        sendMspMessages(s, online_cmds)
    else:
        #serial.write('#\r\n')
        #serial.write('gpspassthrough\r\n')
        sendUbxMessages(s, offline_cmds)
        sendUbxMessages(s, online_cmds)

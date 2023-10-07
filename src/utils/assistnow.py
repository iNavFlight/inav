#!/usr/local/bin/python3
# https://developer.thingstream.io/guides/location-services/assistnow-user-guide
#
# Create a file named tokens.yaml
# Add your assist now tokens
# assistnow_online: XXXXXXXXXXX
# assistnow_offline: XXXXXXXXXXX

import requests
import yaml
import sys
import serial
import getopt
import io

online_token = ""
offline_token = ""
token_file = "tokens.yaml"
serial_port = None


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

    print("%s" % (type(ubxBytes)))
    print("len: %i" % (len(ubxBytes)))

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
            currentCommand.append(ubxBytes[i])
            continue
        if not ubxId:
            ubxId = True
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
            payloadLen += 2 # add crc bytes
            currentCommand.append(ubxBytes[i])
            continue
        if skipped < payloadLen:
            skipped = skipped + 1
            currentCommand.append(ubxBytes[i])
            continue
        ubxCommands.append(currentCommand)
        resetUbloxState()
    
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


def usage():
    print ("assistnow.py -s /dev/ttyS0 [-t tokens.yaml]")

try:
    opts, args = getopt.getopt(sys.argv[1:], "s:t:", ["serial=", "tokens="])
except getopt.GetoptError as err:
    # print help information and exit:
    print(err)  # will print something like "option -a not recognized"
    usage()
    sys.exit(2)

for o, a in opts:
    if o in ("-s", "--serial"):
        serial_port = a
    elif o in ("-t", "--tokens="):
        token_file = a
    else:
        usage()
        sys.exit(2)

if serial_port == None:
    usage()
    sys.exit(2)

loadTokens(token_file)

#s = serial.Serial(serial_port, 115200)

#s.write(b"#\r\n")
#s.write(b"gpspassrhtrough\r\n");

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
#print (len(online_req.content))
online = io.BytesIO(online_req.content)
online_bytes = online.read()
online_cmds = splitUbloxCommands(online_bytes)
print ("AssitnowOnline: %i" % (len(online_cmds)))
#s.write(online_req.content);
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

#s.write(offline_req.content);

of = open("aoff.ubx", "wb")
of.write(offline_req.content)
of.close()


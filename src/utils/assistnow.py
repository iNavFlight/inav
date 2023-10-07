#!/usr/bin/python3
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

online_token = ""
offline_token = ""
token_file = "tokens.yaml"
serial_port = None

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
    print ("assist-now.py -s /dev/ttyS0 [-t tokens.yaml]")

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
print (len(online_req.content))
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
print(len(offline_req.content))

#s.write(offline_req.content);

of = open("aoff.ubx", "wb")
of.write(offline_req.content)
of.close()


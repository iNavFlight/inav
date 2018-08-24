#!/usr/bin/python

# This script was used during the #define FEATURE -> #define USE_FEATURE
# migration. It's commited to the repo in case it will be useful again
# in the future.
# Conditional flags in the RENAMES list are renamed prepending USE_ to
# them unles a different renaming is found in the NEW_NAMES map.
#
# This script should be able to replace all ocurrences in all the source
# code for the project, including the settings.yaml file.

import os
import re

RENAMES = [
 'ACC',
 'GYRO',
 'BARO',
 'MAG',
 'LED_STRIP',
 'SPEKTRUM_BIND',
 'SERIAL_RX',
 'BLACKBOX',
 'GPS',
 'GPS_PROTO_UBLOX',
 'TELEMETRY',
 'TELEMETRY_LTM',
 'TELEMETRY_FRSKY',
 'CMS',
 'GPS_PROTO_NMEA',
 'GPS_PROTO_I2C_NAV',
 'GPS_PROTO_NAZA',
 'GPS_PROTO_UBLOX_NEO7PLUS',
 'GPS_PROTO_MTK',
 'TELEMETRY_HOTT',
 'TELEMETRY_IBUS',
 'TELEMETRY_MAVLINK',
 'TELEMETRY_SMARTPORT',
 'TELEMETRY_CRSF',
 'PWM_DRIVER_PCA9685',
 'PITOT',
 'OSD',
 'NAV',
 'ASYNC_GYRO_PROCESSING',
 'BOOTLOG',
 'STATS',
 'FIXED_WING_LANDING',
 'VTX_CONTROL',
 'VTX_SMARTAUDIO',
 'VTX_TRAMP',
 'VTX_RTC6705',
 'VTX_COMMON',
]

NEW_NAMES = {
    'FIXED_WING_LANDING': 'NAV_FIXED_WING_LANDING',
}

REPLS = [
    '(define ){0}(\W|$)',
    '(undef ){0}(\W|$)',
    '(ifdef ){0}(\W|$)',
    '(defined\\(){0}(\\)(\W|$))',
    '(condition: ){0}(\W|$)',
]


def replace_in_files(root):
    def visit(arg, dirname, names):
        for n in names:
            p = os.path.join(dirname, n)
            if os.path.isfile(p):
                with open(p) as f:
                    data = f.read()
                new_data = data
                for name in RENAMES:
                    new_name = NEW_NAMES.get(name, 'USE_' + name)
                    repl = '\\1' + new_name + '\\2'
                    for r in REPLS:
                        pattern = r.format(name)
                        new_data = re.sub(pattern, repl, new_data)
                if new_data != data:
                    with open(p, 'w') as f:
                        f.write(new_data)


    os.path.walk(root, visit, None)


replace_in_files('src')

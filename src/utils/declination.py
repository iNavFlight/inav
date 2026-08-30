#!/usr/bin/env python3
'''
generate field tables from IGRF13. Note that this requires python3

To run the generator you need the igrf module. Install like this:

    python3 -m pip install --user igrf

And run:

    python3 src/utils/declination.py

It will updates the navigation_declination_gen.c code
'''

import igrf
import numpy as np
import datetime
import pathlib
from math import sin, cos, sqrt
import math

class Vector3(object):
    '''a vector'''
    def __init__(self, x=None, y=None, z=None):
        if x is not None and y is not None and z is not None:
            self.x = float(x)
            self.y = float(y)
            self.z = float(z)
        elif x is not None and len(x) == 3:
            self.x = float(x[0])
            self.y = float(x[1])
            self.z = float(x[2])
        elif x is not None:
            raise ValueError('bad initializer')
        else:
            self.x = float(0)
            self.y = float(0)
            self.z = float(0)

    def length(self):
        return sqrt(self.x**2 + self.y**2 + self.z**2)

    def __sub__(self, other):
        return Vector3(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, other):
        if isinstance(other, (int, float)):
            return Vector3(self.x * other, self.y * other, self.z * other)
        elif isinstance(other, Vector3):
            return self.x * other.x + self.y * other.y + self.z * other.z
        else:
            raise ValueError('Multiplication with unsupported type')

class Matrix3(object):
    '''a 3x3 matrix, intended as a rotation matrix'''
    def __init__(self, a=None, b=None, c=None):
        if a is not None and b is not None and c is not None:
            self.a = a.copy()
            self.b = b.copy()
            self.c = c.copy()
        else:
            self.identity()

    def identity(self):
        self.a = Vector3(1,0,0)
        self.b = Vector3(0,1,0)
        self.c = Vector3(0,0,1)

    def from_euler(self, roll, pitch, yaw):
        '''fill the matrix from Euler angles in radians'''
        cp = cos(pitch)
        sp = sin(pitch)
        sr = sin(roll)
        cr = cos(roll)
        sy = sin(yaw)
        cy = cos(yaw)

        self.a.x = cp * cy
        self.a.y = (sr * sp * cy) - (cr * sy)
        self.a.z = (cr * sp * cy) + (sr * sy)
        self.b.x = cp * sy
        self.b.y = (sr * sp * sy) + (cr * cy)
        self.b.z = (cr * sp * sy) - (sr * cy)
        self.c.x = -sp
        self.c.y = sr * cp
        self.c.z = cr * cp

    def __mul__(self, vector):
        if isinstance(vector, Vector3):
            x = self.a.x * vector.x + self.a.y * vector.y + self.a.z * vector.z
            y = self.b.x * vector.x + self.b.y * vector.y + self.b.z * vector.z
            z = self.c.x * vector.x + self.c.y * vector.y + self.c.z * vector.z
            return Vector3(x, y, z)
        else:
            raise ValueError('Multiplication with unsupported type')

def write_table(f, name, table):
    '''write one table'''
    f.write("const float %s[%u][%u] = {\n" %
                (name, NUM_LAT, NUM_LON))
    for i in range(NUM_LAT):
        f.write("    {")
        for j in range(NUM_LON):
            f.write("%.5ff" % table[i][j])
            if j != NUM_LON-1:
                f.write(",")
        f.write("}")
        if i != NUM_LAT-1:
            f.write(",")
        f.write("\n")
    f.write("};\n\n")

date = datetime.datetime.now()

SAMPLING_RES = 10
SAMPLING_MIN_LAT = -90
SAMPLING_MAX_LAT = 90
SAMPLING_MIN_LON = -180
SAMPLING_MAX_LON = 180

lats = np.arange(SAMPLING_MIN_LAT, SAMPLING_MAX_LAT+SAMPLING_RES, SAMPLING_RES)
lons = np.arange(SAMPLING_MIN_LON, SAMPLING_MAX_LON+SAMPLING_RES, SAMPLING_RES)

NUM_LAT = lats.size
NUM_LON = lons.size

intensity_table = np.empty((NUM_LAT, NUM_LON))
inclination_table = np.empty((NUM_LAT, NUM_LON))
declination_table = np.empty((NUM_LAT, NUM_LON))

max_error = 0
max_error_pos = None
max_error_field = None

def get_igrf(lat, lon):
    '''return field as [declination_deg, inclination_deg, intensity_gauss]'''
    mag = igrf.igrf(date, glat=lat, glon=lon, alt_km=0., isv=0, itype=1)
    intensity = float(mag.total/1e5)
    inclination = float(mag.incl)
    declination = float(mag.decl)
    return [declination, inclination, intensity]

def interpolate_table(table, latitude_deg, longitude_deg):
    '''interpolate inside a table for a given lat/lon in degrees'''
    # round down to nearest sampling resolution
    min_lat = int(math.floor(latitude_deg / SAMPLING_RES) * SAMPLING_RES)
    min_lon = int(math.floor(longitude_deg / SAMPLING_RES) * SAMPLING_RES)

    # find index of nearest low sampling point
    min_lat_index = int(math.floor((min_lat - SAMPLING_MIN_LAT) / SAMPLING_RES))
    min_lon_index = int(math.floor((min_lon - SAMPLING_MIN_LON) / SAMPLING_RES))

    # calculate intensity
    data_sw = table[min_lat_index][min_lon_index]
    data_se = table[min_lat_index][min_lon_index + 1]
    data_ne = table[min_lat_index + 1][min_lon_index + 1]
    data_nw = table[min_lat_index + 1][min_lon_index]

    # perform bilinear interpolation on the four grid corners
    data_min = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_se - data_sw) + data_sw
    data_max = ((longitude_deg - min_lon) / SAMPLING_RES) * (data_ne - data_nw) + data_nw

    value = ((latitude_deg - min_lat) / SAMPLING_RES) * (data_max - data_min) + data_min
    return value

def interpolate_field(latitude_deg, longitude_deg):
    '''calculate magnetic field intensity and orientation, interpolating in tables

    returns array [declination_deg, inclination_deg, intensity] or None'''
    # limit to table bounds
    if latitude_deg < SAMPLING_MIN_LAT or latitude_deg >= SAMPLING_MAX_LAT:
        return None
    if longitude_deg < SAMPLING_MIN_LON or longitude_deg >= SAMPLING_MAX_LON:
        return None

    intensity_gauss = interpolate_table(intensity_table, latitude_deg, longitude_deg)
    declination_deg = interpolate_table(declination_table, latitude_deg, longitude_deg)
    inclination_deg = interpolate_table(inclination_table, latitude_deg, longitude_deg)

    return [declination_deg, inclination_deg, intensity_gauss]

def field_to_Vector3(mag):
    '''return mGauss field from dec, inc and intensity'''
    R = Matrix3()
    mag_ef = Vector3(mag[2]*1000.0, 0.0, 0.0)
    R.from_euler(0.0, -math.radians(mag[1]), math.radians(mag[0]))
    return R * mag_ef

def test_error(lat, lon):
    '''check for error from lat, lon'''
    global max_error, max_error_pos, max_error_field
    mag1 = get_igrf(lat, lon)
    mag2 = interpolate_field(lat, lon)
    if mag2 is None:
        return
    ef1 = field_to_Vector3(mag1)
    ef2 = field_to_Vector3(mag2)
    err = (ef1 - ef2).length()
    if err > max_error or err > 100:
        print(lat, lon, err, ef1, ef2)
        max_error = err
        max_error_pos = (lat, lon)
        max_error_field = ef1 - ef2

def test_max_error(lat, lon):
    '''check for maximum error from lat, lon over SAMPLING_RES range'''
    steps = 3
    delta = SAMPLING_RES/steps
    for i in range(steps):
        for j in range(steps):
            lat2 = lat + i * delta
            lon2 = lon + j * delta
            if lat2 > SAMPLING_MAX_LAT or lon2 > SAMPLING_MAX_LON:
                continue
            if lat2 < SAMPLING_MIN_LAT or lon2 < SAMPLING_MIN_LON:
                continue
            test_error(lat2, lon2)

for i, lat in enumerate(lats):
    for j, lon in enumerate(lons):
        mag = get_igrf(lat, lon)
        declination_table[i][j] = mag[0]
        inclination_table[i][j] = mag[1]
        intensity_table[i][j] = mag[2]

with open(pathlib.Path(__file__).parent / '..' / 'main' / 'navigation' / 'navigation_declination_gen.c', 'w') as f:
    f.write('/* this file is automatically generated by src/utils/declination.py - DO NOT EDIT! */\n\n\n')
    f.write('/* Updated on %s */\n\n\n' % date)

    f.write('''const float SAMPLING_RES = %u;
const float SAMPLING_MIN_LAT = %u;
const float SAMPLING_MAX_LAT = %u;
const float SAMPLING_MIN_LON = %u;
const float SAMPLING_MAX_LON = %u;

''' % (SAMPLING_RES,
           SAMPLING_MIN_LAT,
           SAMPLING_MAX_LAT,
           SAMPLING_MIN_LON,
           SAMPLING_MAX_LON))

    write_table(f, 'declination_table', declination_table)
    write_table(f, 'inclination_table', inclination_table)
    write_table(f, 'intensity_table', intensity_table)

print("Checking for maximum error")
for lat in range(-60, 60, 1):
    for lon in range(-180, 180, 1):
        test_max_error(lat, lon)
print("Generated with max error %.2f %s at (%.2f,%.2f)" % (
    max_error, max_error_field, max_error_pos[0], max_error_pos[1]))

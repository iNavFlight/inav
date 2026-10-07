
import enum

MODEBOXES = { # Key = permanentID
    0:  {"boxName": "ARM",                 "boxEnum": "BOXARM",              "boxId": 0},
    1:  {"boxName": "ANGLE",               "boxEnum": "BOXANGLE",            "boxId": 1},
    2:  {"boxName": "HORIZON",             "boxEnum": "BOXHORIZON",          "boxId": 2},
    3:  {"boxName": "NAV ALTHOLD",         "boxEnum": "BOXNAVALTHOLD",       "boxId": 3},
    5:  {"boxName": "HEADING HOLD",        "boxEnum": "BOXHEADINGHOLD",      "boxId": 4},
    6:  {"boxName": "HEADFREE",            "boxEnum": "BOXHEADFREE",         "boxId": 5},
    7:  {"boxName": "HEADADJ",             "boxEnum": "BOXHEADADJ",          "boxId": 6},
    8:  {"boxName": "CAMSTAB",             "boxEnum": "BOXCAMSTAB",          "boxId": 7},
    10: {"boxName": "NAV RTH",             "boxEnum": "BOXNAVRTH",           "boxId": 8},
    11: {"boxName": "NAV POSHOLD",         "boxEnum": "BOXNAVPOSHOLD",       "boxId": 9},
    12: {"boxName": "MANUAL",              "boxEnum": "BOXMANUAL",           "boxId": 10},
    13: {"boxName": "BEEPER",              "boxEnum": "BOXBEEPERON",         "boxId": 11},
    15: {"boxName": "LEDS OFF",            "boxEnum": "BOXLEDLOW",           "boxId": 12},
    16: {"boxName": "LIGHTS",              "boxEnum": "BOXLIGHTS",           "boxId": 13},
    19: {"boxName": "OSD OFF",             "boxEnum": "BOXOSD",              "boxId": 15},
    20: {"boxName": "TELEMETRY",           "boxEnum": "BOXTELEMETRY",        "boxId": 16},
    21: {"boxName": "AUTO TUNE",           "boxEnum": "BOXAUTOTUNE",         "boxId": 28},
    26: {"boxName": "BLACKBOX",            "boxEnum": "BOXBLACKBOX",         "boxId": 17},
    27: {"boxName": "FAILSAFE",            "boxEnum": "BOXFAILSAFE",         "boxId": 18},
    28: {"boxName": "NAV WP",              "boxEnum": "BOXNAVWP",            "boxId": 19},
    29: {"boxName": "AIR MODE",            "boxEnum": "BOXAIRMODE",          "boxId": 20},
    30: {"boxName": "HOME RESET",          "boxEnum": "BOXHOMERESET",        "boxId": 21},
    31: {"boxName": "GCS NAV",             "boxEnum": "BOXGCSNAV",           "boxId": 22},
    32: {"boxName": "FPV ANGLE MIX",       "boxEnum": "BOXFPVANGLEMIX",      "boxId": 39},
    33: {"boxName": "SURFACE",             "boxEnum": "BOXSURFACE",          "boxId": 24},
    34: {"boxName": "FLAPERON",            "boxEnum": "BOXFLAPERON",         "boxId": 25},
    35: {"boxName": "TURN ASSIST",         "boxEnum": "BOXTURNASSIST",       "boxId": 26},
    36: {"boxName": "NAV LAUNCH",          "boxEnum": "BOXNAVLAUNCH",        "boxId": 14},
    37: {"boxName": "SERVO AUTOTRIM",      "boxEnum": "BOXAUTOTRIM",         "boxId": 27},
    39: {"boxName": "CAMERA CONTROL 1",    "boxEnum": "BOXCAMERA1",          "boxId": 29},
    40: {"boxName": "CAMERA CONTROL 2",    "boxEnum": "BOXCAMERA2",          "boxId": 30},
    41: {"boxName": "CAMERA CONTROL 3",    "boxEnum": "BOXCAMERA3",          "boxId": 31},
    42: {"boxName": "OSD ALT 1",           "boxEnum": "BOXOSDALT1",          "boxId": 32},
    43: {"boxName": "OSD ALT 2",           "boxEnum": "BOXOSDALT2",          "boxId": 33},
    44: {"boxName": "OSD ALT 3",           "boxEnum": "BOXOSDALT3",          "boxId": 34},
    45: {"boxName": "NAV COURSE HOLD",     "boxEnum": "BOXNAVCOURSEHOLD",    "boxId": 35},
    46: {"boxName": "MC BRAKING",          "boxEnum": "BOXBRAKING",          "boxId": 36},
    47: {"boxName": "USER1",               "boxEnum": "BOXUSER1",            "boxId": 37},
    48: {"boxName": "USER2",               "boxEnum": "BOXUSER2",            "boxId": 38},
    49: {"boxName": "LOITER CHANGE",       "boxEnum": "BOXLOITERDIRCHN",     "boxId": 40},
    50: {"boxName": "MSP RC OVERRIDE",     "boxEnum": "BOXMSPRCOVERRIDE",    "boxId": 41},
    51: {"boxName": "PREARM",              "boxEnum": "BOXPREARM",           "boxId": 42},
    52: {"boxName": "TURTLE",              "boxEnum": "BOXTURTLE",           "boxId": 43},
    53: {"boxName": "NAV CRUISE",          "boxEnum": "BOXNAVCRUISE",        "boxId": 44},
    54: {"boxName": "AUTO LEVEL TRIM",     "boxEnum": "BOXAUTOLEVEL",        "boxId": 45},
    55: {"boxName": "WP PLANNER",          "boxEnum": "BOXPLANWPMISSION",    "boxId": 46},
    56: {"boxName": "SOARING",             "boxEnum": "BOXSOARING",          "boxId": 47},
    57: {"boxName": "USER3",               "boxEnum": "BOXUSER3",            "boxId": 48},
    58: {"boxName": "USER4",               "boxEnum": "BOXUSER4",            "boxId": 49},
    59: {"boxName": "MISSION CHANGE",      "boxEnum": "BOXCHANGEMISSION",    "boxId": 50},
    60: {"boxName": "BEEPER MUTE",         "boxEnum": "BOXBEEPERMUTE",       "boxId": 51},
    61: {"boxName": "MULTI FUNCTION",      "boxEnum": "BOXMULTIFUNCTION",    "boxId": 52},
    62: {"boxName": "MIXER PROFILE 2",     "boxEnum": "BOXMIXERPROFILE",     "boxId": 53},
    63: {"boxName": "MIXER TRANSITION",    "boxEnum": "BOXMIXERTRANSITION",  "boxId": 54},
    64: {"boxName": "ANGLE HOLD",          "boxEnum": "BOXANGLEHOLD",        "boxId": 55},
    65: {"boxName": "GIMBAL LEVEL TILT",   "boxEnum": "BOXGIMBALTLOCK",      "boxId": 56},
    66: {"boxName": "GIMBAL LEVEL ROLL",   "boxEnum": "BOXGIMBALRLOCK",      "boxId": 57},
    67: {"boxName": "GIMBAL CENTER",       "boxEnum": "BOXGIMBALCENTER",     "boxId": 58},
    68: {"boxName": "GIMBAL HEADTRACKER",  "boxEnum": "BOXGIMBALHTRK",       "boxId": 59},
    69: {"boxName": "AUTO SPEED",          "boxEnum": "BOXAUTOSPEED",        "boxId": 60},
    70: {"boxName": "TERRAIN AGL HOLD",    "boxEnum": "BOXTERRAINAGLHOLD",   "boxId": 61},
    71: {"boxName": "IN FLIGHT MENU",      "boxEnum": "BOXINFLIGHTMENU",     "boxId": 62},
    72: {"boxName": "THRUST REVERSE",      "boxEnum": "BOXTHRUSTREVERSE",    "boxId": 63},
}

class BoxEnum(enum.IntEnum):
    BOXARM = 0
    BOXANGLE = 1
    BOXHORIZON = 2
    BOXNAVALTHOLD = 3
    BOXHEADINGHOLD = 5
    BOXHEADFREE = 6
    BOXHEADADJ = 7
    BOXCAMSTAB = 8
    BOXNAVRTH = 10
    BOXNAVPOSHOLD = 11
    BOXMANUAL = 12
    BOXBEEPERON = 13
    BOXLEDLOW = 15
    BOXLIGHTS = 16
    BOXOSD = 19
    BOXTELEMETRY = 20
    BOXAUTOTUNE = 21
    BOXBLACKBOX = 26
    BOXFAILSAFE = 27
    BOXNAVWP = 28
    BOXAIRMODE = 29
    BOXHOMERESET = 30
    BOXGCSNAV = 31
    BOXFPVANGLEMIX = 32
    BOXSURFACE = 33
    BOXFLAPERON = 34
    BOXTURNASSIST = 35
    BOXNAVLAUNCH = 36
    BOXAUTOTRIM = 37
    BOXCAMERA1 = 39
    BOXCAMERA2 = 40
    BOXCAMERA3 = 41
    BOXOSDALT1 = 42
    BOXOSDALT2 = 43
    BOXOSDALT3 = 44
    BOXNAVCOURSEHOLD = 45
    BOXBRAKING = 46
    BOXUSER1 = 47
    BOXUSER2 = 48
    BOXLOITERDIRCHN = 49
    BOXMSPRCOVERRIDE = 50
    BOXPREARM = 51
    BOXTURTLE = 52
    BOXNAVCRUISE = 53
    BOXAUTOLEVEL = 54
    BOXPLANWPMISSION = 55
    BOXSOARING = 56
    BOXUSER3 = 57
    BOXUSER4 = 58
    BOXCHANGEMISSION = 59
    BOXBEEPERMUTE = 60
    BOXMULTIFUNCTION = 61
    BOXMIXERPROFILE = 62
    BOXMIXERTRANSITION = 63
    BOXANGLEHOLD = 64
    BOXGIMBALTLOCK = 65
    BOXGIMBALRLOCK = 66
    BOXGIMBALCENTER = 67
    BOXGIMBALHTRK = 68
    BOXAUTOSPEED = 69
    BOXTERRAINAGLHOLD = 70
    BOXINFLIGHTMENU = 71
    BOXTHRUSTREVERSE = 72

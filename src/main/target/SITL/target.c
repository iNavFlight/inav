/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include <platform.h>
#include "target.h"

#include "fc/runtime_config.h"
#include "common/utils.h"
#include "scheduler/scheduler.h"
#include "drivers/system.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/serial.h"
#include "config/config_streamer.h"

#include "target/SITL/sim/realFlight.h"
#include "target/SITL/sim/xplane.h"

// More dummys
const int timerHardwareCount = 0;
timerHardware_t timerHardware[1];
uint32_t SystemCoreClock = 500 * 1e6; // fake 500 MHz;
char _estack = 0 ;
char _Min_Stack_Size = 0;

static pthread_mutex_t mainLoopLock;
static SitlSim_e sitlSim = SITL_SIM_NONE;
static struct timespec start_time;
static uint8_t pwmMapping[MAX_MOTORS + MAX_SERVOS];
static uint8_t mappingCount = 0;
static bool useImu = false;
static char *simIp = NULL;
static int simPort = 0;

static char **c_argv;

void systemInit(void) {

    fprintf(stderr, "INAV %d.%d.%d SITL\n", FC_VERSION_MAJOR, FC_VERSION_MINOR, FC_VERSION_PATCH_LEVEL);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    fprintf(stderr, "[SYSTEM] Init...\n");

#if !defined(__FreeBSD__)   // maybe also || !defined(__APPLE__)
    pthread_attr_t thAttr;
    int policy = 0;

    pthread_attr_init(&thAttr);
    pthread_attr_getschedpolicy(&thAttr, &policy);
    pthread_setschedprio(pthread_self(), sched_get_priority_min(policy));
    pthread_attr_destroy(&thAttr);
#endif

    if (pthread_mutex_init(&mainLoopLock, NULL) != 0) {
        fprintf(stderr, "[SYSTEM] Unable to create mainLoop lock.\n");
        exit(1);
    }

    if (sitlSim != SITL_SIM_NONE) {
        fprintf(stderr, "[SIM] Waiting for connection...\n");
    }

    switch (sitlSim) {
        case SITL_SIM_REALFLIGHT:
            if (mappingCount > RF_MAX_PWM_OUTS) {
                fprintf(stderr, "[SIM] Mapping error. RealFligt supports a maximum of %i PWM outputs.", RF_MAX_PWM_OUTS);
                sitlSim = SITL_SIM_NONE;
                break;
            }
            if (simRealFlightInit(simIp, pwmMapping, mappingCount, useImu)) {
                fprintf(stderr, "[SIM] Connection with RealFlight successfully established.\n");
            } else {
                fprintf(stderr, "[SIM] Connection with RealFlight NOT established.\n");
            }
            break;
        case SITL_SIM_XPLANE:
            if (mappingCount > XP_MAX_PWM_OUTS) {
                fprintf(stderr, "[SIM] Mapping error. RealFligt supports a maximum of %i PWM outputs.", XP_MAX_PWM_OUTS);
                sitlSim = SITL_SIM_NONE;
                break;
            }
            if (simXPlaneInit(simIp, simPort, pwmMapping, mappingCount, useImu)) {
                fprintf(stderr, "[SIM] Connection with X-Plane successfully established.\n");
            } else {
                fprintf(stderr, "[SIM] Connection with X-PLane NOT established.\n");
            }
            break;
        default:
          fprintf(stderr, "[SIM] No interface specified. Configurator only.\n");
          break;
    }

    rescheduleTask(TASK_SERIAL, 1);
}

bool parseMapping(char* mapStr)
{
    char *split = strtok(mapStr, ",");
    char numBuf[2];
    while(split)
    {
        if (strlen(split) != 6) {
            return false;
        }

        if (split[0] == 'M' || split[0] == 'S') {
            memcpy(numBuf, &split[1], 2);
            int pwmOut = atoi(numBuf);
            memcpy(numBuf, &split[4], 2);
            int rOut = atoi(numBuf);
            if (pwmOut < 0 || rOut < 1) {
                return false;
            }
            if (split[0] == 'M') {
                pwmMapping[rOut - 1] = pwmOut - 1;
                pwmMapping[rOut - 1] |= 0x80;
                mappingCount++;
            } else if (split[0] == 'S') {
                pwmMapping[rOut - 1] = pwmOut;
                mappingCount++;
            }
        } else {
            return false;
        }
        split = strtok(NULL, ",");
    }

    return true;
}

void printCmdLineOptions(void)
{
    fprintf(stderr, "Avaiable options:\n");
    fprintf(stderr, "--path=[path]                        Path and filename of eeprom.bin. If not specified 'eeprom.bin' in program directory is used.\n");
    fprintf(stderr, "--sim=[rf|xp]                        Simulator interface: rf = RealFligt, xp = XPlane. Example: --sim=rf\n");
    fprintf(stderr, "--simip=[ip]                         IP-Address oft the simulator host. If not specified localhost (127.0.0.1) is used.\n");
    fprintf(stderr, "--simport=[port]                     Port oft the simulator host.\n");
    fprintf(stderr, "--useimu                             Use IMU sensor data from the simulator instead of using attitude data from the simulator directly (experimental, not recommended).\n");
    fprintf(stderr, "--chanmap=[mapstring]                Channel mapping. Maps INAVs motor and servo PWM outputs to the virtual receiver output in the simulator.\n");
    fprintf(stderr, "                                     The mapstring has the following format: M(otor)|S(servo)<INAV-OUT>-<RECEIVER-OUT>,... All numbers must have two digits\n");
    fprintf(stderr, "                                     For example: Map motor 1 to virtal receiver output 1, servo 1 to output 2 and servo 2 to output 3:\n");
    fprintf(stderr, "                                     --chanmap=M01-01,S01-02,S02-03\n");
}

void parseArguments(int argc, char *argv[])
{
    // Stash these so we can rexec on reboot, just like a FC does
    c_argv = calloc(argc+1, sizeof(char *));
    for (int i = 0; i < argc; i++) {
        c_argv[i] = strdup(argv[i]);
    }
    int c;
    while(true) {
        static struct option longOpt[] = {
            {"sim", required_argument, 0, 's'},
            {"useimu", no_argument, 0, 'u'},
            {"chanmap", required_argument, 0, 'c'},
            {"simip", required_argument, 0, 'i'},
            {"simport", required_argument, 0, 'p'},
            {"help", no_argument, 0, 'h'},
            {"path", required_argument, 0, 'e'},
            {NULL, 0, NULL, 0}
        };

        c = getopt_long_only(argc, argv, "s:c:h", longOpt, NULL);
        if (c == -1)
            break;

        switch (c) {
            case 's':
                if (strcmp(optarg, "rf") == 0) {
                    sitlSim = SITL_SIM_REALFLIGHT;
                } else if (strcmp(optarg, "xp") == 0){
                    sitlSim = SITL_SIM_XPLANE;
                } else {
                    fprintf(stderr, "[SIM] Unsupported simulator %s.\n", optarg);
                }
                break;

            case 'c':
                if (!parseMapping(optarg) && sitlSim != SITL_SIM_NONE) {
                    fprintf(stderr, "[SIM] Invalid channel mapping string.\n");
                    printCmdLineOptions();
                    exit(0);
                }
                break;
            case 'p':
                simPort = atoi(optarg);
                break;
            case 'u':
                useImu = true;
                break;
            case 'i':
                simIp = optarg;
                break;
            case 'e':
                if (!configFileSetPath(optarg)){
                    fprintf(stderr, "[EEPROM] Invalid path, using eeprom file in program directory\n.");
                }
                break;
            case 'h':
                printCmdLineOptions();
                exit(0);
                break;
        }
    }

    if (simIp == NULL) {
        simIp = malloc(10);
        strcpy(simIp, "127.0.0.1");
    }
}


bool lockMainPID(void) {
    return pthread_mutex_trylock(&mainLoopLock) == 0;
}

void unlockMainPID(void)
{
    pthread_mutex_unlock(&mainLoopLock);
}

// Replacements for system functions
timeUs_t micros(void) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (now.tv_sec - start_time.tv_sec) * 1000000 + (now.tv_nsec - start_time.tv_nsec) / 1000;
}

uint64_t microsISR(void)
{
    return micros();
}

uint32_t millis(void) {
    return (uint32_t)(micros() / 1000);
}

void delayMicroseconds(timeUs_t us)
{
    usleep(us);
}

void delay(timeMs_t ms)
{
    delayMicroseconds(ms * 1000UL);
}

void systemReset(void)
{
    fprintf(stderr, "[SYSTEM] Reset\n");
#if defined(__CYGWIN__) || defined(__APPLE__) || GCC_MAJOR < 12
    for(int j = 3; j < 1024; j++) {
        close(j);
    }
#else
    closefrom(3);
#endif
    execvp(c_argv[0], c_argv); // restart
}

void systemResetToBootloader(void)
{
    fprintf(stderr, "[SYSTEM] Reset to bootloader\n");
    exit(0);
}

void failureMode(failureMode_e mode) {
    fprintf(stderr, "[SYSTEM] Failure mode %d\n", mode);
    while (true) {
        delay(1000);
    };
}

// Even more dummys and stubs
uint32_t getEscUpdateFrequency(void)
{
    return 400;
}

pwmInitError_e getPwmInitError(void)
{
    return PWM_INIT_ERROR_NONE;
}

const char *getPwmInitErrorMessage(void)
{
    return "No error";
}

void IOConfigGPIO(IO_t io, ioConfig_t cfg)
{
    UNUSED(io);
    UNUSED(cfg);
}

void systemClockSetup(uint8_t cpuUnderclock)
{
    UNUSED(cpuUnderclock);
}

void timerInit(void) {
    // NOP
}

bool isMPUSoftReset(void)
{
    return false;
}

// Not in linux libs, but in arm-none-eabi ?!?
// https://github.com/lattera/freebsd/blob/master/lib/libc/string/strnstr.c
char * strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if (slen-- < 1 || (sc = *s++) == '\0')
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

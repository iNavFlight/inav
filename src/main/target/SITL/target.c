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

#include <platform.h>
#include "target.h"

#include "fc/runtime_config.h"
#include "common/utils.h"
#include "scheduler/scheduler.h"
#include "drivers/system.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/serial.h"

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

void systemInit(void) {

    printf("INAV %d.%d.%d SITL\n", FC_VERSION_MAJOR, FC_VERSION_MINOR, FC_VERSION_PATCH_LEVEL);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    printf("[SYSTEM] Init...\n");

    if (pthread_mutex_init(&mainLoopLock, NULL) != 0) {
        printf("[SYSTEM] Unable to create mainLoop lock.\n");
        exit(1);
    }

    if (sitlSim != SITL_SIM_NONE) {
        printf("[SIM] Waiting for connection...\n");
    }
    
    switch (sitlSim) {
        case SITL_SIM_REALFLIGHT:
            if (mappingCount > RF_MAX_PWM_OUTS) {
                printf("[SIM] Mapping error. RealFligt supports a maximum of %i PWM outputs.", RF_MAX_PWM_OUTS);
                sitlSim = SITL_SIM_NONE;
                break;
            }
            if (simRealFlightInit(simIp, pwmMapping, mappingCount, useImu)) {
                printf("[SIM] Connection with RealFlight (%s) successfully established. \n", simIp);
            } else {
                printf("[SIM] Connection with RealFlight (%s) NOT established. \n", simIp);
            }
            break;
        case SITL_SIM_XPLANE:
            if (mappingCount > XP_MAX_PWM_OUTS) {
                printf("[SIM] Mapping error. RealFligt supports a maximum of %i PWM outputs.", XP_MAX_PWM_OUTS);
                sitlSim = SITL_SIM_NONE;
                break;
            }            
            if (simXPlaneInit(simIp, simPort, pwmMapping, mappingCount, useImu)) {
                printf("[SIM] Connection with XPlane successfully established. \n");
            } else {
                printf("[SIM] Connection with XPLane NOT established. \n");
            }
            break;
        default:
          printf("[SIM] No interface specified. Configurator only.\n");
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
    printf("Avaiable options:\n");
    printf("--sim=[rf|xp]                        Simulator interface: rf = RealFligt, xp = XPlane. Example: --sim=rf\n");
    printf("--simip=[ip]                         IP-Address oft the simulator host. If not specified localhost (127.0.0.1) is used.");
    printf("--simport=[port]                     Port oft the simulator host.");
    printf("--useimu                             Use IMU sensor data from the simulator instead of using attitude data from the simulator directly (experimental, not recommended).");
    printf("--chanmap=[mapstring]                Channel mapping, Maps INAVs motor and servo PWM outputs to the virtual receiver output in the simulator.\n");
    printf("                                     The mapstring has the following format: M(otor)|S(servo)<INAV-OUT>-<RECEIVER_OUT>,)... All numbers must have two digits\n");
    printf("                                     For example map motor 1 to virtal receiver output 1, servo 1 to output 2 and servo 2 to output 3:\n");
    printf("                                     --chanmap=M01-01,S01-02,S02-03\n"); 
}

void parseArguments(int argc, char *argv[])
{
    int c;
    while(1) {
        static struct option longOpt[] = {
            {"sim", optional_argument, 0, 's'},
            {"useimu", optional_argument, 0, 'u'},
            {"chanmap", optional_argument, 0, 'c'},
            {"simip", optional_argument, 0, 'i'},
            {"simport", optional_argument, 0, 'p'},
            {"help", optional_argument, 0, 'h'},
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
                    printf("[SIM] Unsupported simulator %s.\n", optarg);
                }
                break;

            case 'c':
                if (!parseMapping(optarg) && sitlSim != SITL_SIM_NONE) {
                    printf("[SIM] Invalid channel mapping string.\n");
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
void microsleep(uint32_t usec) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = usec*1000UL;
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) ;
}

void delayMicroseconds_real(uint32_t us) {
    microsleep(us);
}

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
    timeUs_t now = micros();
    while (micros() - now < us);
}

void delay(timeMs_t ms)
{
    while (ms--)
        delayMicroseconds(1000);
}

void systemReset(void) 
{
    printf("[SYSTEM] Reset\n");
    exit(0);
}

void systemResetToBootloader(void)
{
    printf("[SYSTEM] Reset to bootloader\n");
    exit(0);
}

void failureMode(failureMode_e mode) {
    printf("[SYSTEM] Failure mode %d\n", mode);
    while (1);
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

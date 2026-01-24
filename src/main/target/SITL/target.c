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
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#if defined(WASM_BUILD)
#include <emscripten.h>
#include <emscripten/posix_socket.h>
#include <emscripten/threading.h>
#include <emscripten/wasmfs.h>
#endif

#include <platform.h>
#include "target.h"

#include "fc/runtime_config.h"
#include "common/utils.h"
#include "scheduler/scheduler.h"
#include "drivers/system.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"
#include "drivers/serial.h"
#include "drivers/serial_tcp.h"
#include "config/config_streamer.h"
#include "build/version.h"

#include "target/SITL/sim/realFlight.h"
#include "target/SITL/sim/xplane.h"

#include "target/SITL/serial_proxy.h"

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

#if defined(WASM_BUILD)
#define WASM_DEFAULT_PROXY_PORT 8081
#define WARM_PROXY_CONNECT_TIMEOUT_MS 5000
#define WEBSOCKET_READY_STATE_OPEN 1

static int wasmProxyPort = WASM_DEFAULT_PROXY_PORT;
static EMSCRIPTEN_WEBSOCKET_T bridgeSocket = 0;
bool wasmProxyConnected = false;

static pthread_t wasmMainThread;
static wasmMainThreadType wasmMainThreadWorker = NULL;
static bool wasmMainWorkerThreadStarted = false;

void wasmExit(void);
#endif

uint32_t millis(void);

static void printVersion(void) {
    
#if defined(WASM_BUILD)
    const char* const sitlVariant = "SITL Webassembly";
#else
    const char* const sitlVariant = "SITL";
#endif
    fprintf(stderr, "INAV %d.%d.%d %s (%s)\n", FC_VERSION_MAJOR, FC_VERSION_MINOR, FC_VERSION_PATCH_LEVEL, sitlVariant, shortGitRevision);
}

void systemInit(void) {
    printVersion();
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    fprintf(stderr, "[SYSTEM] Init...\n");

#if !defined(__FreeBSD__) && !defined(__APPLE__) && !defined(__EMSCRIPTEN__)
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

    
#if defined(WASM_BUILD)
    // Init emscripten socket bridge
    char url[64];
    snprintf(url, sizeof(url), "ws://localhost:%d", wasmProxyPort);
    bridgeSocket = emscripten_init_websocket_to_posix_socket_bridge(url);
    uint16_t readyState = 0;
    const uint32_t start = millis();
    do {
        emscripten_websocket_get_ready_state(bridgeSocket, &readyState);
        emscripten_thread_sleep(100);
    } while((readyState != WEBSOCKET_READY_STATE_OPEN && millis() - start < WARM_PROXY_CONNECT_TIMEOUT_MS));

    if (readyState != WEBSOCKET_READY_STATE_OPEN) {
        fprintf(stderr, "[SOCKET] Warning: Unable to connect to socket proxy on port %d. Network connection may be unavailable.\n", wasmProxyPort);
    } else {
        fprintf(stderr, "[SOCKET] Connected to socket proxy on port %d\n", wasmProxyPort);
        wasmProxyConnected = true;
    }

#endif

    if (sitlSim != SITL_SIM_NONE) {
#if defined(WASM_BUILD)
        if (!wasmProxyConnected) {
            fprintf(stderr, "[SIM] Simulator connection requires a working socket proxy. Simulator interface disabled.\n");
            sitlSim = SITL_SIM_NONE;
        } else {
#endif
        fprintf(stderr, "[SIM] Waiting for connection...\n");
#if defined(WASM_BUILD)
        }
#endif
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

    rescheduleTask(TASK_SERIAL, SITL_SERIAL_TASK_US);
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

#ifdef SITL_BUILD
OptSerialStopBits_e parseStopBits(const char* optarg){
    if ( strcmp(optarg, "One") == 0 ) {
        return OPT_SERIAL_STOP_BITS_ONE;
    } else if ( strcmp(optarg, "Two") == 0 ) {
        return OPT_SERIAL_STOP_BITS_TWO;
    } else  {
        return OPT_SERIAL_STOP_BITS_INVALID;
    }
}

OptSerialParity_e parseParity(const char* optarg){
    if ( strcmp(optarg, "Even") == 0 ) {
        return OPT_SERIAL_PARITY_EVEN;
    } else if ( strcmp(optarg, "None") == 0 ) {
        return OPT_SERIAL_PARITY_NONE;
    } else if ( strcmp(optarg, "Odd") == 0 ) {
        return OPT_SERIAL_PARITY_ODD;
    } else  {
        return OPT_SERIAL_PARITY_INVALID;
    }
}
#endif

void printCmdLineOptions(void)
{
    printVersion();
    fprintf(stderr, "Avaiable options:\n");
    fprintf(stderr, "--path=[path]                  Path and filename of eeprom.bin. If not specified 'eeprom.bin' in program directory is used.\n");
    fprintf(stderr, "--sim=[rf|xp]                  Simulator interface: rf = RealFligt, xp = XPlane. Example: --sim=rf\n");
    fprintf(stderr, "--simip=[ip]                   IP-Address oft the simulator host. If not specified localhost (127.0.0.1) is used.\n");
    fprintf(stderr, "--simport=[port]               Port oft the simulator host.\n");
    fprintf(stderr, "--useimu                       Use IMU sensor data from the simulator instead of using attitude data from the simulator directly (experimental, not recommended).\n");
#if defined(SITL_BUILD)
    fprintf(stderr, "--serialuart=[uart]            UART number on which serial receiver is configured in SITL, f.e. 3 for UART3\n");
    fprintf(stderr, "--serialport=[serialport]      Host's serial port to which serial receiver/proxy FC is connected, f.e. COM3, /dev/ttyACM3\n");
    fprintf(stderr, "--baudrate=[baudrate]          Serial receiver baudrate (default: 115200).\n");
    fprintf(stderr, "--stopbits=[None|One|Two]      Serial receiver stopbits (default: One).\n");
    fprintf(stderr, "--parity=[Even|None|Odd]       Serial receiver parity (default: None).\n");
    fprintf(stderr, "--fcproxy                      Use inav/betaflight FC as a proxy for serial receiver.\n");
    fprintf(stderr, "--tcpbaseport=[port]           Base TCP port for UART sockets (default: 5760)\n");
#endif
    fprintf(stderr, "--chanmap=[mapstring]          Channel mapping. Maps INAVs motor and servo PWM outputs to the virtual receiver output in the simulator.\n");    
    fprintf(stderr, "                               The mapstring has the following format: M(otor)|S(servo)<INAV-OUT>-<RECEIVER-OUT>,... All numbers must have two digits\n");
    fprintf(stderr, "                               For example: Map motor 1 to virtal receiver output 1, servo 1 to output 2 and servo 2 to output 3:\n");
    fprintf(stderr, "                               --chanmap=M01-01,S01-02,S02-03\n");
#if defined(WASM_BUILD)
    fprintf(stderr, "--proxyPort=[port]            Port on which the websocket proxy server is listening (default: 8081)\n");
#endif
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
            {"version", no_argument, 0, 'v'},
#if defined(SITL_BUILD)
            {"serialuart", required_argument, 0, '0'},
            {"serialport", required_argument, 0, '1'},
            {"baudrate", required_argument, 0, '2'},
            {"stopbits", required_argument, 0, '3'},
            {"parity", required_argument, 0, '4'},
            {"fcproxy", no_argument, 0, '5'},
            {"tcpbaseport", required_argument, 0, '6'},
#elif defined(WASM_BUILD)
            {"proxyPort", required_argument, 0, '7'},
#endif
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
            case 'v':
                printVersion();
                exit(0);
#if defined(SITL_BUILD)
            case '0':
                serialUartIndex = atoi(optarg);
                if ( (serialUartIndex<1) || (serialUartIndex>8) ) {
                    fprintf(stderr, "[serialuart] Invalid argument\n.");
                    exit(0);
                }
                break;
            case '1':
                if ( (strlen(optarg)<1) || (strlen(optarg)>63) ) {
                    fprintf(stderr, "[serialport] Invalid argument\n.");
                    exit(0);
                } else {
                    strcpy( serialPort, optarg );
                }
                break;
            case '2':
                serialBaudRate = atoi(optarg);
                if ( serialBaudRate < 1200 )
                {
                    fprintf(stderr, "[baudrate] Invalid argument\n.");
                    exit(0);
                }
                break;
            case '3':
                serialStopBits = parseStopBits(optarg);
                if ( serialStopBits == OPT_SERIAL_STOP_BITS_INVALID )
                {
                    fprintf(stderr, "[stopbits] Invalid argument\n.");
                    exit(0);
                }
                break;

            case '4':
                serialParity = parseParity(optarg);
                if ( serialParity== OPT_SERIAL_PARITY_INVALID )
                {
                    fprintf(stderr, "[parity] Invalid argument\n.");
                    exit(0);
                }
                break;
            case '5':
                serialFCProxy = true;
                break;
            case '6': {
                char *endptr = NULL;
                long basePort = strtol(optarg, &endptr, 10);
                if ((endptr == NULL) || (*endptr != '\0') || basePort <= 0 || basePort > UINT16_MAX || basePort + SERIAL_PORT_COUNT - 1 > UINT16_MAX) {
                    fprintf(stderr, "[tcpbaseport] Invalid argument\n.");
                    exit(0);
                }
                tcpBasePort = (uint16_t)basePort;
                break;
            }
#elif defined(WASM_BUILD)
            case '7':
            {   
                char *endptr = NULL;
                long proxyPort = strtol(optarg, &endptr, 10);
                if ((endptr == NULL) || (*endptr != '\0') || proxyPort <= 0 || proxyPort > UINT16_MAX) {
                    fprintf(stderr, "[proxyPort] Invalid argument\n.");
                    exit(0);
                }
                wasmProxyPort = (int)proxyPort;
                break;
            }
#endif
            default:
                printCmdLineOptions();
                exit(0);
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
 #if defined(SITL_BUILD)
    usleep(us);
#elif defined(WASM_BUILD)
    // very short delays are only used in hardware drivers
    // and not relevant in WASM/SITL simulation
    UNUSED(us);
 #endif
}

void delay(timeMs_t ms)
{
#if defined(SITL_BUILD)
    delayMicroseconds(ms * 1000UL);
#elif defined(WASM_BUILD)
    emscripten_sleep(ms);
#endif
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
#ifdef SITL_BUILD
    serialProxyClose();
#endif
    execvp(c_argv[0], c_argv); // restart
}

void systemResetToBootloader(void)
{
    fprintf(stderr, "[SYSTEM] Reset to bootloader\n");
#if defined(SITL_BUILD)
    exit(0);
#elif defined(WASM_BUILD)
    wasmExit();
#endif
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

int lookupAddress (char *name, int port, int type, struct sockaddr *addr, socklen_t* len )
{
    struct addrinfo *servinfo, *p;
    struct addrinfo hints = {.ai_family = AF_UNSPEC, .ai_socktype = type, .ai_flags = AI_V4MAPPED|AI_ADDRCONFIG};
    if (name == NULL) {
	hints.ai_flags |= AI_PASSIVE;
    }
  /*
    This nonsense is to uniformly deliver the same sa_family regardless of whether
    name is NULL or non-NULL ** ON LINUX **
    Otherwise, at least on Linux, we get
    - V6,V4 for the non-null case and
    - V4,V6 for the null case, regardless of gai.conf
    Which may confuse consumers
    FreeBSD and Windows behave consistently, giving V6 for Ipv6 enabled stacks
    unless a quad dotted address is specified (or a name resolveds to V4,
    or system policy enforces IPv4 over V6
  */
    struct addrinfo *p4 = NULL;
    struct addrinfo *p6 = NULL;

    int result;
    char aport[16];
    snprintf(aport, sizeof(aport), "%d", port);

    if ((result = getaddrinfo(name, aport, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        return result;
    } else {
	for(p = servinfo; p != NULL; p = p->ai_next) {
	    if(p->ai_family == AF_INET6)
		p6 = p;
	    else if(p->ai_family == AF_INET)
		p4 = p;
	}

	if (p6 != NULL)
	    p = p6;
	else if (p4 != NULL)
	    p = p4;
	else
	    return -1;
	memcpy(addr, p->ai_addr, p->ai_addrlen);
	*len = p->ai_addrlen;
	freeaddrinfo(servinfo);
    }
    return 0;
}

char *prettyPrintAddress(struct sockaddr* p, char *outbuf, size_t buflen)
{
    if (buflen < IPADDRESS_PRINT_BUFLEN) {
	return NULL;
    }
    char *bufp = outbuf;
    void *addr;
    uint16_t port;
    if (p->sa_family == AF_INET6) {
	struct sockaddr_in6 * ip = (struct sockaddr_in6*)p;
	addr = &ip->sin6_addr;
	port = ntohs(ip->sin6_port);
    } else {
	struct sockaddr_in * ip = (struct sockaddr_in*)p;
	port = ntohs(ip->sin_port);
	addr = &ip->sin_addr;
    }
    const char *res = inet_ntop(p->sa_family, addr, outbuf+1, buflen-1);
    if (res != NULL) {
	char *ptr = (char*)res+strlen(res);
	if (p->sa_family == AF_INET6) {
	    *bufp ='[';
	    *ptr++ = ']';
	} else {
	    bufp++;
	}
	sprintf(ptr, ":%d", port);
	return bufp;
    }
    return NULL;
}

#if defined(WASM_BUILD)

void wasmMainLoop(void) 
{
    if (!wasmMainWorkerThreadStarted && wasmMainThreadWorker != NULL) {
        /*
        * In Webassembly/emscripten, a classic infinite loop cannot be used, as otherwise the entire browser tab freezes. 
        * The main loop emscripten_set_main_loop() runs too slowly (max approx. 60 Hz/FPS), 
        * so the main loop must run in a separate thread (pthread -> in emscripten a wrapper around web worker) 
        * that can ‘rev up’ to full speed (ca. 1 kHz or more, depending on the host CPU and browser capabilities).
        * Here we start that main worker thread.
        */
        int err = pthread_create(&wasmMainThread, NULL, wasmMainThreadWorker, NULL);
        if (err != 0) {
            fprintf(stderr, "[SYSTEM] Failed to start WASM scheduler thread %s\n", strerror(err));
            wasmExit();
        };
        wasmMainWorkerThreadStarted = true;
    }
}

void wasmInitFilesystem(void)
{
    // Must be done in main thread
    const char *idbfsMount = MOUNT_POINT;
    const backend_t backend = wasmfs_create_opfs_backend();
    int res = wasmfs_create_directory(idbfsMount, 0777, backend);
    if (res < 0) {
        fprintf(stderr, "[FILESYSTEM] Failed to create IDBFS mount directory '%s': %s\n", idbfsMount, strerror(-res));
        fprintf(stderr, "[FILESYSTEM] Using in-memory filesystem fallback, eeprom file will not be persistent\n");
    }
}

void wasmStart(wasmMainThreadType thread)
{
    wasmMainThreadWorker = thread;
    wasmInitFilesystem();
    emscripten_set_main_loop(wasmMainLoop, 0, false);
}

void wasmExit(void)
{
   emscripten_force_exit(1);
}

bool isSocketProxyConnected(void) 
{ 
    return wasmProxyConnected; 
}


#endif
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

#include "platform.h"

#include "target.h"
#include "target/SITL/sim/adumsim.h"
#include "target/SITL/sim/simHelper.h"
#include "fc/runtime_config.h"
#include "drivers/time.h"
#include "drivers/accgyro/accgyro_fake.h"
#include "drivers/barometer/barometer_fake.h"
#include "sensors/battery_sensor_fake.h"
#include "sensors/acceleration.h"
#include "drivers/pitotmeter/pitotmeter_fake.h"
#include "drivers/compass/compass_fake.h"
#include "drivers/rangefinder/rangefinder_virtual.h"
#include "io/rangefinder.h"
#include "common/utils.h"
#include "common/maths.h"
#include "flight/mixer.h"
#include "flight/servos.h"
#include "flight/imu.h"
#include "io/gps.h"
#include "rx/sim.h"

#define ADUM_XP_PORT 2323
#define ADUM_JOYSTICK_AXIS_COUNT 8
#define BUF_SIZE 1024

#define PWM_TO_FLOAT_0_1(x) ((float)(((int)x - 1000) / 1000.0f))
#define PWM_TO_FLOAT_MINUS_1_1(x) ((float)(((int)x - 1500) / 500.0f))
#define FLOAT_0_1_TO_PWM(x) ((uint16_t)(x * 1000.0f) + 1000.0f)
#define FLOAT_MINUS_1_1_TO_PWM(x) ((float)((uint16_t)((x + 1.0f) / 2.0f * 1000.0f) + 1000.0f))

static uint8_t pwmMapping[XP_MAX_PWM_OUTS];
static uint8_t mappingCount;

static int listenFd, connFd;
static int sockfd;

static pthread_t listenThread;
static bool initalized = false;
static bool useImu = false;

static float lattitude = 0;
static float longitude = 0;
static float elevation = 0;
static float agl = 0;
static float local_vx = 0;
static float local_vy = 0;
static float local_vz = 0;
static float groundspeed = 0;
static float airspeed = 0;
static float roll = 0;
static float pitch = 0;
static float yaw = 0;
static float hpath = 0;
static float accel_x = 0;
static float accel_y = 0;
static float accel_z = 0;
static float gyro_x = 0;
static float gyro_y = 0;
static float gyro_z = 0;
static float barometer = 0;
static bool  hasJoystick = false;
static float joystickRaw[ADUM_JOYSTICK_AXIS_COUNT];



int init_fgear_socket(char* ip, int port) {

    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    // Create TCP socket
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        perror("socket");
        return 1;
    }

    // Set SO_REUSEADDR
    int opt = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listenFd);
        return 1;
    }

    // Bind to IP and port
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(listenFd);
        return 1;
    }

    if (bind(listenFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind");
        close(listenFd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenFd, 1) < 0) {
        perror("listen");
        close(listenFd);
        return 1;
    }

    printf("Listening on %s:%d\n", ip, port);

    // Block until a client connects
    connFd = accept(listenFd, (struct sockaddr*)&clientAddr, &clientLen);
    if (connFd < 0) {
        perror("accept");
        close(listenFd);
        return 1;
    }

    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
    printf("Connection established with %s:%d\n", clientIP, ntohs(clientAddr.sin_port));

    return 0;

}


int init_fgear_client(const char* server_ip, int server_port) {

    struct sockaddr_in serverAddr;

    // Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    // Fill server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return -1;
    }

    // Connect to server

    int conn_status = -1;
    for (int i=0; i<100; i++) {

        //printf("Connecting attempt nr: %d\n", i);
        conn_status = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        if (conn_status == 0) break;

        sleep(1);

    }

    if (conn_status < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    printf("Connected to server %s:%d\n", server_ip, server_port);

    return 0; // return socket descriptor for sending/receiving
}

static void* listenWorker(void* arg)
{
    char buf[BUF_SIZE];
    char *token;


    while (1) {


        // Read all outputs from inav
        // ----------------------------------------------------- //

        float motorValue[4] = { 0.0f };
        float yokeValues[3] = { 0 };
        int y = 0;
        int k = 0;
        for (int i = 0; i < mappingCount; i++) {
            if (y > 2) {
                break;
            }
            if (pwmMapping[i] & 0x80) { // Motor
                if (k < 4) motorValue[k] = PWM_TO_FLOAT_0_1(motor[pwmMapping[i] & 0x7f]);
                k++;
                //printf("Motor %d value: %f\n", i, motorValue);
                //printf("Motor %d value: %d\n", i, motor[pwmMapping[i] & 0x7f]);
            } else {
                yokeValues[y] = PWM_TO_FLOAT_MINUS_1_1(servo[pwmMapping[i]]);
                y++;
            }

        }
        

        // Send motor data to simulation
        char msg[256];  
        int len = snprintf(msg, sizeof(msg),
                        "%.6f;%.6f;%.6f;%.6f;\n",
                        motorValue[0], motorValue[1], motorValue[2], motorValue[3]);
        if (len > 0) {
            ssize_t sent = send(sockfd, msg, len, 0);
        }

        // Read all data from socket
        // ----------------------------------------------------- //

        uint16_t channelValues[ADUM_JOYSTICK_AXIS_COUNT];


        ssize_t n = recv(sockfd, buf, sizeof(buf) - 1, 0);

        if (n > 0) {
            buf[n] = '\0';  // null terminate
            //printf("Rx: %s\n", buf);

            int index = 0;
            token = strtok(buf, ";");
            while (token != NULL) {

                float value = atof(token);  // convert to float

                switch(index) {
                    case 0:
                        //trel=value;
                        break;

                    case 1:
                        lattitude = value;
                        break;
                    
                    case 2:
                        longitude = value;
                        break;
                    
                    case 3:
                        elevation = value; // Altitude MSL in meters
                        break;

                    case 4:
                        hpath = value; // Track
                        break;
                    
                    case 5:
                        yaw = value;  // Heading
                        break;
                    
                    case 6:
                        //posx = value;
                        break;
                   
                    case 7:
                        //posy = value;
                        break;
                    
                    case 8:
                        agl = value;  // posz
                        break;

                    case 9:
                        groundspeed = value;

                    case 10:
                        roll = value;
                        break;
                        
                    case 11:
                        pitch = value;
                        break;
                    
                    case 12:
                        //yaw = value;  // Gets this from heading
                        break;
                    
                    case 13:
                        accel_x = value;
                        break;
                    
                    case 14:
                        accel_y = value;
                        break;
                    
                    case 15:
                        accel_z = value;
                        break;

                    case 16:
                        gyro_x = value;
                        break;
                    
                    case 17:
                        gyro_y = value;
                        break;
                    
                    case 18:
                        gyro_z = value;
                        break;

                    case 19:
                        channelValues[0] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;

                    case 20:
                        channelValues[1] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;
                    
                    case 21:
                        channelValues[2] = FLOAT_0_1_TO_PWM(value);
                        break;
                    
                    case 22:
                        channelValues[3] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;
                    
                    case 23:
                        channelValues[4] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;
                    
                    case 24:
                        channelValues[5] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;

                    case 25:
                        channelValues[6] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;

                    case 26:
                        channelValues[7] = FLOAT_MINUS_1_1_TO_PWM(value);
                        break;

                    default:
                        break;
                }

                token = strtok(NULL, ";");
                index++;
            }

            rxSimSetChannelValue(channelValues, ADUM_JOYSTICK_AXIS_COUNT);


            gpsFakeSet(
                GPS_FIX_3D,
                16,
                (int32_t)roundf(lattitude * 10000000),
                (int32_t)roundf(longitude * 10000000),
                (int32_t)roundf(elevation * 100),
                (int16_t)roundf(groundspeed * 100),
                (int16_t)roundf(hpath * 10),
                0, //(int16_t)roundf(-local_vz * 100),
                0, //(int16_t)roundf(local_vx * 100),
                0, //(int16_t)roundf(-local_vy * 100),
                0
            );


            const int32_t altitideOverGround = (int32_t)roundf(agl * 100);
            if (altitideOverGround > 0 && altitideOverGround <= RANGEFINDER_VIRTUAL_MAX_RANGE_CM) {
                fakeRangefindersSetData(altitideOverGround);
            } else {
                fakeRangefindersSetData(-1);
            }

            const int16_t roll_inav = roll * 10;
            const int16_t pitch_inav = -pitch * 10;
            const int16_t yaw_inav = yaw * 10;

            if (!useImu) {
                imuSetAttitudeRPY(roll_inav, pitch_inav, yaw_inav);
                imuUpdateAttitude(micros());
            }

            fakeAccSet(
                constrainToInt16(accel_x * GRAVITY_MSS * 1000.0f),
                constrainToInt16(accel_y * GRAVITY_MSS * 1000.0f),
                constrainToInt16(accel_z * GRAVITY_MSS * 1000.0f)
            );

            fakeGyroSet(
                constrainToInt16(gyro_x * 16.0f),
                constrainToInt16(-gyro_y * 16.0f),
                constrainToInt16(-gyro_z * 16.0f)
            );

            fakeBaroSet((int32_t)(101325.0f - 12.0f*elevation),DEGREES_TO_CENTIDEGREES(21));

            fakePitotSetAirspeed(airspeed * 100.0f);
            fakeBattSensorSetVbat(16.8f * 100);

            fpQuaternion_t quat;
            fpVector3_t north;
            north.x = 1.0f;
            north.y = 0.0f;
            north.z = 0.0f;
            computeQuaternionFromRPY(&quat, roll_inav, pitch_inav, yaw_inav);
            transformVectorEarthToBody(&north, &quat);
            fakeMagSet(
                constrainToInt16(north.x * 1024.0f),
                constrainToInt16(north.y * 1024.0f),
                constrainToInt16(north.z * 1024.0f)
            );

            if (!initalized) {
                ENABLE_ARMING_FLAG(SIMULATOR_MODE_SITL);
                // Aircraft can wobble on the runway and prevents calibration of the accelerometer
                ENABLE_STATE(ACCELEROMETER_CALIBRATED);
                initalized = true;
            }

            unlockMainPID();




        // Check connection and close if required
        // ----------------------------------------------------- //

        } else if (n == 0) {
            // Connection closed by client
            printf("Client disconnected\n");
            break;

        } else {

            // No data available or error
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No data right now, sleep briefly
                usleep(1000); // 1 ms
            } else {
                perror("recv");
                break;
            }

        }

    }

    close(connFd);
    close(listenFd);

    return 0;
}

bool simAdumInit(char* ip, int port, uint8_t* mapping, uint8_t mapCount, bool imu)
{

    printf("[Juhu SIM] Pozz world\n");

    memcpy(pwmMapping, mapping, mapCount);
    mappingCount = mapCount;
    useImu = imu;

    if (port == 0) {
        port = ADUM_XP_PORT; // use default port
    }

    const int stat = init_fgear_client(ip, port); //init_fgear_socket(ip, port);


    if (pthread_create(&listenThread, NULL, listenWorker, NULL) < 0) {
        return false;
    }


    return true;
}

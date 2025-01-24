/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common/time.h"

typedef enum {
    TASK_PRIORITY_IDLE = 0,     // Disables dynamic scheduling, task is executed only if no other task is active this cycle
    TASK_PRIORITY_LOW = 1,
    TASK_PRIORITY_MEDIUM = 3,
    TASK_PRIORITY_MEDIUM_HIGH = 4,
    TASK_PRIORITY_HIGH = 5,
    TASK_PRIORITY_REALTIME = 18,
    TASK_PRIORITY_MAX = 255
} cfTaskPriority_e;

typedef struct {
    timeUs_t     maxExecutionTime;
    timeUs_t     totalExecutionTime;
    timeUs_t     averageExecutionTime;
} cfCheckFuncInfo_t;

typedef struct {
    const char * taskName;
    bool         isEnabled;
    uint8_t      staticPriority;
    timeDelta_t     desiredPeriod;
    timeUs_t     maxExecutionTime;
    timeUs_t     totalExecutionTime;
    timeUs_t     averageExecutionTime;
    timeDelta_t     latestDeltaTime;
} cfTaskInfo_t;

typedef enum {
    /* Actual tasks */
    TASK_SYSTEM = 0,
    TASK_PID,
    TASK_GYRO,
    TASK_RX,
    TASK_SERIAL,
    TASK_BATTERY,
    TASK_TEMPERATURE,
#if defined(BEEPER) || defined(USE_DSHOT)
    TASK_BEEPER,
#endif
#ifdef USE_LIGHTS
    TASK_LIGHTS,
#endif
#ifdef USE_GPS
    TASK_GPS,
#endif
#ifdef USE_MAG
    TASK_COMPASS,
#endif
#ifdef USE_BARO
    TASK_BARO,
#endif
#ifdef USE_ADSB
    TASK_ADSB,
#endif
#ifdef USE_PITOT
    TASK_PITOT,
#endif
#ifdef USE_RANGEFINDER
    TASK_RANGEFINDER,
#endif
#ifdef USE_DASHBOARD
    TASK_DASHBOARD,
#endif
#ifdef USE_TELEMETRY
    TASK_TELEMETRY,
#endif
#ifdef USE_LED_STRIP
    TASK_LEDSTRIP,
#endif
#if defined(USE_SERVO_SBUS)
    TASK_PWMDRIVER,
#endif
#ifdef STACK_CHECK
    TASK_STACK_CHECK,
#endif
#ifdef USE_OSD
    TASK_OSD,
#endif
#ifdef USE_CMS
    TASK_CMS,
#endif
#ifdef USE_OPFLOW
    TASK_OPFLOW,
#endif
#ifdef USE_RCDEVICE
    TASK_RCDEVICE,
#endif
#ifdef USE_VTX_CONTROL
    TASK_VTXCTRL,
#endif
#ifdef USE_PROGRAMMING_FRAMEWORK
    TASK_PROGRAMMING_FRAMEWORK,
#endif
#ifdef USE_RPM_FILTER
    TASK_RPM_FILTER,
#endif
    TASK_AUX,
#if defined(USE_SMARTPORT_MASTER)
    TASK_SMARTPORT_MASTER,
#endif
#ifdef USE_IRLOCK
    TASK_IRLOCK,
#endif
#ifdef USE_ADAPTIVE_FILTER
    TASK_ADAPTIVE_FILTER,
#endif
#ifdef USE_SERIAL_GIMBAL
    TASK_GIMBAL,
#endif

#ifdef USE_HEADTRACKER
    TASK_HEADTRACKER,
#endif

#if defined(USE_TELEMETRY) && defined(USE_TELEMETRY_SBUS2)
    TASK_TELEMETRY_SBUS2,
#endif

#if defined (USE_GEOZONE) && defined(USE_GPS)
    TASK_GEOZONE,
#endif

    /* Count of real tasks */
    TASK_COUNT,

    /* Service task IDs */
    TASK_NONE = TASK_COUNT,
    TASK_SELF
} cfTaskId_e;

typedef struct {
    /* Configuration */
    const char * taskName;
    bool (*checkFunc)(timeUs_t currentTimeUs, timeDelta_t currentDeltaTimeUs);
    void (*taskFunc)(timeUs_t currentTimeUs);
    timeDelta_t desiredPeriod;         // target period of execution
    const uint8_t staticPriority;   // dynamicPriority grows in steps of this size, shouldn't be zero

    /* Scheduling */
    uint16_t dynamicPriority;       // measurement of how old task was last executed, used to avoid task starvation
    uint16_t taskAgeCycles;
    timeUs_t lastExecutedAt;        // last time of invocation
    timeUs_t lastSignaledAt;        // time of invocation event for event-driven tasks
    timeDelta_t taskLatestDeltaTime;

    /* Statistics */
    timeUs_t movingSumExecutionTime;  // moving sum over 32 samples
    timeUs_t maxExecutionTime;
    timeUs_t totalExecutionTime;    // total time consumed by task since boot
} cfTask_t;

extern cfTask_t cfTasks[TASK_COUNT];
extern uint16_t averageSystemLoadPercent;

void getCheckFuncInfo(cfCheckFuncInfo_t *checkFuncInfo);
void getTaskInfo(cfTaskId_e taskId, cfTaskInfo_t *taskInfo);
void rescheduleTask(cfTaskId_e taskId, timeDelta_t newPeriodUs);
void setTaskEnabled(cfTaskId_e taskId, bool newEnabledState);
timeDelta_t getTaskDeltaTime(cfTaskId_e taskId);
void schedulerResetTaskStatistics(cfTaskId_e taskId);

void schedulerInit(void);
void scheduler(void);
void taskSystem(timeUs_t currentTimeUs);
void taskRunRealtimeCallbacks(timeUs_t currentTimeUs);

#define TASK_PERIOD_HZ(hz) (1000000 / (hz))
#define TASK_PERIOD_MS(ms) ((ms) * 1000)
#define TASK_PERIOD_US(us) (us)

#define isSystemOverloaded() (averageSystemLoadPercent >= 100)

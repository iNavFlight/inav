/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Reproduction test for the NULL gpsPort hard-fault bug.
 *
 * Scenario
 * --------
 * The FC boots with a driver-based GPS provider (GPS_MSP / GPS_FAKE).
 * gpsInit() sets gpsState.gpsPort = NULL because driver-based providers
 * do not open a serial port.  If the user then changes gps_provider via
 * the CLI to GPS_UBLOX (a serial-based provider) without rebooting, the
 * next call to gpsUpdate() dispatches into gpsHandleUBLOX(), which calls
 * serialRxBytesWaiting(gpsState.gpsPort) with a NULL pointer → hard fault
 * on real hardware.
 *
 * There is a second exposure: gpsEnablePassthrough() dereferences
 * gpsState.gpsPort without a NULL check, but that path requires a
 * separate passthrough session, so only the gpsUpdate() path is exercised
 * here.
 *
 * How the test encodes the fault
 * ------------------------------
 * The stub for serialRxBytesWaiting() calls ADD_FAILURE() whenever it is
 * invoked with a NULL pointer.  Before the fix gpsUpdate() triggers this
 * failure; after the fix it returns false gracefully without ever calling
 * into gpsHandleUBLOX().
 *
 * Expected result (after the fix)
 * --------------------------------
 * gpsUpdate() detects that the active provider is serial-based but
 * gpsState.gpsPort == NULL and returns false without calling the protocol
 * handler.  The serialRxBytesWaiting stub is never called with NULL.
 */

#include "gtest/gtest.h"
#include "unittest_macros.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

extern "C" {

/* Pull in the types we need. */
#include "common/time.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/serial.h"

/* fc/config.h provides the FEATURE_GPS enum value. */
#include "fc/config.h"
#include "fc/runtime_config.h"

#include "io/gps.h"
#include "io/gps_private.h"

/* Expose the globals defined in gps.c so the test can manipulate them. */
extern gpsReceiverData_t gpsState;
extern gpsSolutionData_t gpsSolDRV;
extern gpsSolutionData_t gpsSol;

/* PG_DECLARE in gps.h expands to:
 *   extern gpsConfig_t gpsConfig_System;
 *   static inline const gpsConfig_t *gpsConfig(void) { return &gpsConfig_System; }
 * gps.c's PG_REGISTER_WITH_RESET_TEMPLATE provides the storage. */
extern gpsConfig_t gpsConfig_System;

/* ---- serial infrastructure stubs ------------------------------------ */
/* io/serial.c normally provides these; we stub them since we do not     */
/* compile serial.c as part of this test.                                */

/* baudRates[] maps baudRate_e enum values to integer baud rates.        */
/* Must match the baudRate_e enum in io/serial.h (BAUD_AUTO through      */
/* BAUD_2470000).                                                        */
const uint32_t baudRates[] = { 0, 1200, 2400, 4800, 9600, 19200, 38400,
    57600, 115200, 230400, 250000, 460800, 921600,
    1000000, 1500000, 2000000, 2470000 }; /* see baudRate_e */

/* serialConfig_System is the PG storage for serialConfig().             */
/* gpsInit() accesses it; gpsUpdate() does not, but the linker needs it  */
/* because gpsInit() lives in the same translation unit as gpsUpdate().  */
serialConfig_t serialConfig_System;

/* ---- test-observable state -------------------------------------------- */
static bool serialRxBytesWaitingCalledWithNull = false;

/* ---- millis() controllable clock -------------------------------------- */
static uint32_t fakeMillis = 0;

uint32_t millis(void) { return fakeMillis; }
uint32_t micros(void) { return fakeMillis * 1000UL; }

/* ---- feature / sensors stubs ----------------------------------------- */
static uint32_t fakeFeatureMask = 0;

bool feature(uint32_t mask)      { return (fakeFeatureMask & mask) != 0; }
void featureSet(uint32_t mask)   { fakeFeatureMask |=  mask; }
void featureClear(uint32_t mask) { fakeFeatureMask &= ~mask; }

static uint32_t fakeSensorMask = 0;

bool sensors(uint32_t mask)      { return (fakeSensorMask & mask) != 0; }
void sensorsSet(uint32_t mask)   { fakeSensorMask |=  mask; }
void sensorsClear(uint32_t mask) { fakeSensorMask &= ~mask; }

/* ---- serial stubs ----------------------------------------------------- */

/*
 * This is the critical mock.  On real hardware, if gpsState.gpsPort is NULL
 * the firmware hard-faults when the serial driver dereferences the pointer.
 * In the unit test we record the call and fail the test instead.
 *
 * Before the fix: gpsUpdate() reaches this with a NULL pointer → failure.
 * After the fix:  gpsUpdate() returns before calling this → no failure.
 */
uint32_t serialRxBytesWaiting(const serialPort_t *instance)
{
    if (instance == NULL) {
        serialRxBytesWaitingCalledWithNull = true;
        ADD_FAILURE() << "serialRxBytesWaiting() called with NULL gpsPort — "
                         "this would hard-fault on real hardware";
    }
    return 0;
}

uint32_t serialTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return 256;
}

bool isSerialTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

uint8_t serialRead(serialPort_t *instance)
{
    UNUSED(instance);
    return 0;
}

void serialWrite(serialPort_t *instance, uint8_t ch)
{
    UNUSED(instance);
    UNUSED(ch);
}

void serialWriteBuf(serialPort_t *instance, const uint8_t *data, int count)
{
    UNUSED(instance);
    UNUSED(data);
    UNUSED(count);
}

void serialPrint(serialPort_t *instance, const char *str)
{
    UNUSED(instance);
    UNUSED(str);
}

void serialSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

void serialSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}

serialPort_t *openSerialPort(serialPortIdentifier_e identifier,
                             serialPortFunction_e function,
                             serialReceiveCallbackPtr rxCallback,
                             void *rxCallbackData,
                             uint32_t baudRate, portMode_t mode,
                             portOptions_t options)
{
    UNUSED(identifier); UNUSED(function); UNUSED(rxCallback);
    UNUSED(rxCallbackData); UNUSED(baudRate); UNUSED(mode); UNUSED(options);
    return NULL;
}

void closeSerialPort(serialPort_t *serialPort) { UNUSED(serialPort); }

serialPortConfig_t *findSerialPortConfig(serialPortFunction_e function)
{
    UNUSED(function);
    return NULL;
}

void waitForSerialPortToFinishTransmitting(serialPort_t *serialPort)
{
    UNUSED(serialPort);
}

/* ---- stateFlags / armingFlags stubs ---------------------------------- */
uint32_t stateFlags   = 0;
uint32_t armingFlags  = 0;

/* ---- system stubs ----------------------------------------------------- */
bool isMPUSoftReset(void) { return false; }

/* ---- LED stubs -------------------------------------------------------- */
void LED0_ON(void)     {}
void LED0_OFF(void)    {}
void LED1_ON(void)     {}
void LED1_OFF(void)    {}
void LED1_TOGGLE(void) {}

/* ---- navigation / sensor stubs --------------------------------------- */
void onNewGPSData(void) {}

bool rtcHasTime(void) { return false; }
bool rtcSetDateTime(dateTime_t *dt) { UNUSED(dt); return true; }

bool baroIsHealthy(void)  { return false; }
bool pitotIsHealthy(void) { return false; }

/* ---- UBLOX protocol stubs -------------------------------------------- */
/*
 * gps.c references gpsRestartUBLOX() and gpsHandleUBLOX() which are
 * normally provided by gps_ublox.c.  We do NOT compile gps_ublox.c
 * (it carries too many dependencies), so we stub them here.
 *
 * The stubs are intentionally minimal: gpsRestartUBLOX() does nothing
 * (the test checks the guard before restart is called), and
 * gpsHandleUBLOX() calls serialRxBytesWaiting() to simulate the exact
 * operation that hard-faults in production.
 */
void gpsRestartUBLOX(void)
{
    /* In the production code this resets the protothread state and starts
     * the receiver/state threads.  For the test we do nothing: the guard
     * under test should prevent this function from being reached when
     * gpsPort is NULL. */
}

void gpsHandleUBLOX(void)
{
    /*
     * In production this runs the protothread that immediately calls
     * serialRxBytesWaiting(gpsState.gpsPort).  Replicate that first
     * operation here so the test observes the NULL dereference if the
     * guard in gpsUpdate() is absent.
     */
    (void)serialRxBytesWaiting(gpsState.gpsPort);
}

/* ---- MSP / FAKE GPS stubs (not compiled in test target, but needed
 *       for link completeness if the linker sees the symbols) ----------- */
/* These are guarded by USE_GPS_PROTO_MSP / USE_GPS_FAKE in gps.c so they
 * will not appear in the compilation unit.  Nothing to stub. */

} // extern "C"

/* ======================================================================
 * Test fixture
 * ====================================================================== */

class GpsNullPortTest : public ::testing::Test {
protected:
    void SetUp() override {
        /* Reset observable state. */
        serialRxBytesWaitingCalledWithNull = false;

        /* Enable FEATURE_GPS so gpsUpdate() does not exit early. */
        featureSet(FEATURE_GPS);

        /*
         * Advance millis() past GPS_BOOT_DELAY (3000 ms).  gpsUpdate()
         * has a boot-delay guard: if millis() < GPS_BOOT_DELAY it returns
         * false immediately without exercising the state machine.
         */
        fakeMillis = 5000;

        /* Clear state/arming flags. */
        stateFlags  = 0;
        armingFlags = 0;

        /* Initialise solution structs. */
        memset(&gpsSol,    0, sizeof(gpsSol));
        memset(&gpsSolDRV, 0, sizeof(gpsSolDRV));

        /*
         * Set up the GPS config for a serial-based provider (GPS_UBLOX).
         * gpsConfig_System is the storage used by the PG accessor
         * gpsConfig() defined in gps.h and allocated by gps.c.
         */
        memset(&gpsConfig_System, 0, sizeof(gpsConfig_System));
        gpsConfig_System.provider = GPS_UBLOX;

        /* Point gpsState at the config and set sane defaults. */
        gpsState.gpsConfig     = &gpsConfig_System;
        gpsState.baseTimeoutMs = 1000;
        /* lastMessageMs == fakeMillis: no timeout yet. */
        gpsState.lastMessageMs = fakeMillis;

        /*
         * Simulate the buggy scenario: the FC booted with a driver-based
         * provider (e.g. GPS_MSP), so gpsInit() left gpsPort as NULL
         * (no serial port was opened).  The user then changed provider
         * to GPS_UBLOX via CLI without rebooting.
         */
        gpsState.gpsPort = NULL;
    }

    void TearDown() override {
        fakeFeatureMask = 0;
        fakeSensorMask  = 0;
    }
};

/* ======================================================================
 * Test 1 — Bug reproduction: GPS_RUNNING + NULL port + serial provider
 *
 * The state machine is placed directly into GPS_RUNNING so the protocol
 * handler (gpsHandleUBLOX) is called on the very first gpsUpdate().
 *
 * Before the fix: gpsHandleUBLOX() calls serialRxBytesWaiting(NULL)
 *   → ADD_FAILURE() fires inside the stub.
 *
 * After the fix: gpsUpdate() detects the NULL port for a serial-based
 *   provider and returns false before reaching gpsHandleUBLOX().
 * ====================================================================== */
TEST_F(GpsNullPortTest, NullPortInRunningState_ShouldNotCrash)
{
    /* Place the state machine in GPS_RUNNING so the protocol handler
     * is invoked immediately, skipping the INITIALIZING delay. */
    gpsState.state             = GPS_RUNNING;
    gpsState.lastStateSwitchMs = fakeMillis - 600; /* > GPS_INIT_DELAY (500 ms) */

    bool result = gpsUpdate();

    /*
     * The key assertion: serialRxBytesWaiting must NOT have been called
     * with NULL.  If it was, ADD_FAILURE() already recorded the failure
     * in the stub; this EXPECT makes the intent explicit.
     */
    EXPECT_FALSE(serialRxBytesWaitingCalledWithNull)
        << "gpsUpdate() called serialRxBytesWaiting(NULL) — "
           "the null-port guard is missing from gpsUpdate()";

    /* With the guard in place gpsUpdate() returns false (no new data). */
    EXPECT_FALSE(result)
        << "gpsUpdate() should return false when gpsPort is NULL "
           "for a serial-based provider";
}

/* ======================================================================
 * Test 2 — Bug reproduction: INITIALIZING → RUNNING transition
 *
 * Same scenario starting from GPS_INITIALIZING.  After the init delay
 * expires, gpsUpdate() calls restart() and transitions to GPS_RUNNING,
 * then breaks — protocol() is NOT called in that same tick.  The dangerous
 * serialRxBytesWaiting(NULL) would only fire on the next entry when state
 * is already GPS_RUNNING (covered by Test 1).
 *
 * The pre-switch guard fires before restart() is reached, so a stale
 * GPS_INITIALIZING state left by the boot-with-driver-based scenario is
 * also protected.
 * ====================================================================== */
TEST_F(GpsNullPortTest, NullPortTransitionFromInitializing_ShouldNotCrash)
{
    /* Start in INITIALIZING with the delay already expired. */
    gpsState.state             = GPS_INITIALIZING;
    gpsState.lastStateSwitchMs = fakeMillis - 1000; /* >> GPS_INIT_DELAY (500 ms) */

    bool result = gpsUpdate();

    EXPECT_FALSE(serialRxBytesWaitingCalledWithNull)
        << "gpsUpdate() called serialRxBytesWaiting(NULL) during "
           "INITIALIZING→RUNNING transition";

    EXPECT_FALSE(result);
}

/* ======================================================================
 * Test 3 — Positive case: serial provider + non-NULL port
 *
 * When gpsPort is valid, gpsUpdate() must proceed normally.  The guard
 * must not block a legitimate call.
 * ====================================================================== */
TEST_F(GpsNullPortTest, NonNullPortSerialProvider_GuardDoesNotBlock)
{
    /* Provide a fake non-NULL port. The serial stubs accept any pointer. */
    static serialPort_t fakePort;
    gpsState.gpsPort           = &fakePort;
    gpsState.state             = GPS_RUNNING;
    gpsState.lastStateSwitchMs = fakeMillis - 600;

    /*
     * gpsUpdate() is allowed to call into gpsHandleUBLOX() here.
     * gpsHandleUBLOX() passes &fakePort to serialRxBytesWaiting(), which
     * is non-NULL, so the failure path in the stub is not triggered.
     */
    gpsUpdate();

    EXPECT_FALSE(serialRxBytesWaitingCalledWithNull)
        << "serialRxBytesWaiting was unexpectedly called with NULL "
           "even though gpsPort was non-NULL";
}

/* ======================================================================
 * Test 4 — Negative: FEATURE_GPS disabled
 *
 * If the GPS feature is not enabled, gpsUpdate() must return false
 * immediately without touching gpsPort at all.  This is an existing
 * guard, but we verify it is not broken by the new guard.
 * ====================================================================== */
TEST_F(GpsNullPortTest, FeatureGpsDisabled_ReturnsEarlyWithoutTouchingPort)
{
    featureClear(FEATURE_GPS);

    gpsState.state             = GPS_RUNNING;
    gpsState.lastStateSwitchMs = fakeMillis - 600;
    /* gpsPort stays NULL — any dereference would fire the stub. */

    bool result = gpsUpdate();

    EXPECT_FALSE(serialRxBytesWaitingCalledWithNull)
        << "serialRxBytesWaiting was called with NULL even though "
           "FEATURE_GPS was disabled";

    EXPECT_FALSE(result);
}

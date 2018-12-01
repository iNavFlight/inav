#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "build/build_config.h"

#include "common/time.h"
#include "common/maths.h"

#include "drivers/io.h"
#include "drivers/time.h"
#include "drivers/bus.h"
#include "drivers/bus_i2c.h"

#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

#define PCA9685_SERVO_FREQUENCY 60
#define PCA9685_SERVO_COUNT 16
#define PCA9685_SYNC_THRESHOLD 5

static busDevice_t * busDev;
static uint16_t currentOutputState[PCA9685_SERVO_COUNT] = {0};
static uint16_t temporaryOutputState[PCA9685_SERVO_COUNT] = {0};

void pca9685setPWMOn(uint8_t servoIndex, uint16_t on) {
    if (servoIndex < PCA9685_SERVO_COUNT) {
        busWrite(busDev, LED0_ON_L + (servoIndex * 4), on);
        busWrite(busDev, LED0_ON_H + (servoIndex * 4), on>>8);
    }
}

void pca9685setPWMOff(uint8_t servoIndex, uint16_t off) {
    if (servoIndex < PCA9685_SERVO_COUNT) {
        busWrite(busDev, LED0_OFF_L + (servoIndex * 4), off);
        busWrite(busDev, LED0_OFF_H + (servoIndex * 4), off>>8);
    }
}

/*
Writing new state every cycle for each servo is extremely time consuming
and does not makes sense.
Trying to sync 5 servos every 2000us extends looptime
to 3500us. Very, very bad...
Instead of that, write desired values to temporary
table and write it to PCA9685 only when there a need.
Also, because of resultion of PCA9685 internal counter of about 5us, do not write
small changes, since thwy will only clog the bandwidch and will not
be represented on output
PWM Driver runs at 200Hz, every cycle every 4th servo output is updated:
cycle 0: servo 0, 4, 8, 12
cycle 1: servo 1, 5, 9, 13
cycle 2: servo 2, 6, 10, 14
cycle 3: servo3, 7, 11, 15
*/
void pca9685sync(uint8_t cycleIndex) {
    uint8_t i;

    for (i = 0; i < PCA9685_SERVO_COUNT; i++) {
        if (cycleIndex == i % 4 && ABS(temporaryOutputState[i] - currentOutputState[i]) > PCA9685_SYNC_THRESHOLD) {
            pca9685setPWMOff(i, temporaryOutputState[i]);
            currentOutputState[i] = temporaryOutputState[i];
        }
    }
}

void pca9685setServoPulse(uint8_t servoIndex, uint16_t pulse) {

    static double pulselength = 0;

    if (pulselength == 0) {
        pulselength = 1000000;   // 1,000,000 us per second
        pulselength /= PCA9685_SERVO_FREQUENCY;
        pulselength /= 4096;  // 12 bits of resolution
    }
    pulse /= pulselength;

    temporaryOutputState[servoIndex] = pulse;
}

void pca9685setPWMFreq(uint16_t freq) {

  uint32_t prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;

  busWrite(busDev, PCA9685_MODE1, 16);
  delay(1);
  busWrite(busDev, PCA9685_PRESCALE, (uint8_t) prescaleval);
  delay(1);
  busWrite(busDev, PCA9685_MODE1, 128);
}

static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < 5; retry++) {
        uint8_t sig;

        delay(150);

        bool ack = busRead(busDev, PCA9685_MODE1, &sig);
        if (ack) {
            return true;
        }
    };

    return false;
}

bool pca9685Initialize(void)
{
    busDev = busDeviceInit(BUSTYPE_I2C, DEVHW_PCA9685, 0, 0);
    if (busDev == NULL) {
        return false;
    }

    if (!deviceDetect(busDev)) {
        busDeviceDeInit(busDev);
        return false;
    }

    /* Reset device */
    busWrite(busDev, PCA9685_MODE1, 0x00);

    /* Set refresh rate */
    pca9685setPWMFreq(PCA9685_SERVO_FREQUENCY);

    delay(1);

    for (uint8_t i = 0; i < PCA9685_SERVO_COUNT; i++) {
        pca9685setPWMOn(i, 0);
        pca9685setPWMOff(i, 1500);
    }

    return true;
}

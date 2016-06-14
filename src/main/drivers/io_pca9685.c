#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "gpio.h"
#include "system.h"
#include "bus_i2c.h"

#include "debug.h"

#include "common/maths.h"

#include "config/config.h"
#include "config/runtime_config.h"

#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

#define PCA9685_SERVO_FREQUENCY 60
#define PCA9685_SERVO_COUNT 16
#define PCA9685_SYNC_THRESHOLD 5
#define PCA9685_UPDATE_FREQUENCY 50

uint8_t pca9685Enabled = 0;

uint8_t isPca9685Enabled(void) {
    return pca9685Enabled;
}

uint16_t currentOutputState[PCA9685_SERVO_FREQUENCY] = {0};
uint16_t temporaryOutputState[PCA9685_SERVO_FREQUENCY] = {0};

void pca9685setPWM(uint8_t servoIndex, uint16_t on, uint16_t off) {
    if (servoIndex < PCA9685_SERVO_COUNT) {
        i2cWrite(PCA9685_ADDR, LED0_ON_L + (servoIndex * 4), on);
        i2cWrite(PCA9685_ADDR, LED0_ON_H + (servoIndex * 4), on>>8);
        i2cWrite(PCA9685_ADDR, LED0_OFF_L + (servoIndex * 4), off);
        i2cWrite(PCA9685_ADDR, LED0_OFF_H + (servoIndex * 4), off>>8);
    }
}

/*
Writing new state every cycle for each servo is extremely time consuming
and does not makes sense.
On Flip32/Naze32 trying to sync 5 servos every 2000us extends looptime
to 3500us. Very, very bad...
Instead of that, write desired values to temporary
table and write it to PCA9685 only when there a need.
Also, because of resultion of PCA9685 internal counter of about 5us, do not write
small changes, since thwy will only clog the bandwidch and will not
be represented on output
*/
//TODO move it to separate task
void pca9685sync(uint32_t currentTime) {

    static uint32_t lastProcessTime = 0;

    if (currentTime - lastProcessTime > 1000000 / PCA9685_UPDATE_FREQUENCY) {
        uint8_t i;

        for (i = 0; i < PCA9685_SERVO_COUNT; i++) {
            if (ABS(temporaryOutputState[i] - currentOutputState[i]) > PCA9685_SYNC_THRESHOLD) {
                pca9685setPWM(i, 0, temporaryOutputState[i]);
                currentOutputState[i] = temporaryOutputState[i];
            }
        }

        lastProcessTime = currentTime;
    }
}

void pca9685setServoPulse(uint8_t servoIndex, uint16_t pulse) {
    double pulselength;

    pulselength = 1000000;   // 1,000,000 us per second
    pulselength /= PCA9685_SERVO_FREQUENCY;
    pulselength /= 4096;  // 12 bits of resolution

    pulse /= pulselength;

    temporaryOutputState[servoIndex] = pulse;
}

void pca9685setPWMFreq(uint16_t freq) {

  uint32_t prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;

  i2cWrite(PCA9685_ADDR, PCA9685_MODE1, 16);
  delay(1);
  i2cWrite(PCA9685_ADDR, PCA9685_PRESCALE, (uint8_t) prescaleval);
  delay(1);
  i2cWrite(PCA9685_ADDR, PCA9685_MODE1, 128);
}

void pca9685Detect(void) {

    bool ack = false;
    uint8_t sig;

    ack = i2cRead(PCA9685_ADDR, PCA9685_MODE1, 1, &sig);

    if (!ack) {
        pca9685Enabled = 0;
        featureClear(FEATURE_PWM_SERVO_DRIVER);
    } else {
        pca9685Enabled = 1;

        /*
        Reset device
        */
        i2cWrite(PCA9685_ADDR, PCA9685_MODE1, 0x0);

        /*
        Set refresh rate
        */
        pca9685setPWMFreq(PCA9685_SERVO_FREQUENCY);
    }
}

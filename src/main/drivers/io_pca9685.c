#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "gpio.h"
#include "system.h"
#include "bus_i2c.h"

#include "debug.h"

#include "common/maths.h"

#include "build_config.h"

#define PCA9685_ADDR 0x40
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

#define PCA9685_SERVO_FREQUENCY 60

uint8_t pca9685Enabled = 0;

uint8_t isPca9685Enabled(void) {
    return pca9685Enabled;
}

void pca9685setServoPulse(uint8_t n, uint16_t pulse) {
  double pulselength;

  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= PCA9685_SERVO_FREQUENCY;
  pulselength /= 4096;  // 12 bits of resolution

  pulse /= pulselength;
  pca9685setPWM(n, 0, pulse);
}

void pca9685setPWM(uint8_t num, uint16_t on, uint16_t off) {
    i2cWrite(PCA9685_ADDR, LED0_ON_L + (num * 4), on);
    i2cWrite(PCA9685_ADDR, LED0_ON_H + (num * 4), on>>8);
    i2cWrite(PCA9685_ADDR, LED0_OFF_L + (num * 4), off);
    i2cWrite(PCA9685_ADDR, LED0_OFF_H + (num * 4), off>>8);
}

void pca9685setPWMFreq(uint16_t freq) {

  // freq *= 0.9;  // Correct for overshoot in the frequency setting (see issue #11).

  uint32_t prescaleval = 25000000;
  prescaleval /= 4096;
  prescaleval /= freq;
  prescaleval -= 1;

  i2cWrite(PCA9685_ADDR, PCA9685_MODE1, 16);
  delay(1);
  i2cWrite(PCA9685_ADDR, PCA9685_PRESCALE, (uint8_t) prescaleval);
  delay(1);
  i2cWrite(PCA9685_ADDR, PCA9685_MODE1, 128);

  delay(10);
}

void pca9685Detect(void) {

    bool ack = false;
    uint8_t sig;

    delay(10); // No idea how long the chip takes to power-up, but let's make it 10ms

    ack = i2cRead(PCA9685_ADDR, PCA9685_MODE1, 1, &sig);

    if (!ack) {
        pca9685Enabled = 0;
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

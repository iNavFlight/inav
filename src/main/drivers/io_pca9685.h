uint8_t isPca9685Enabled(void);

void pca9685Detect(void);
void pca9685setPWM(uint8_t num, uint16_t on, uint16_t off);
void pca9685setPWMFreq(float freq);

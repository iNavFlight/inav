
#pragma once
#ifdef USE_STATS

typedef struct statsConfig_s {
    uint32_t stats_total_time; // [Seconds]
    uint32_t stats_total_dist; // [Metres]
    uint16_t stats_flight_count;
#ifdef USE_ADC
    uint32_t stats_total_energy; // deciWatt hour (x0.1Wh)
#endif
    uint8_t  stats_enabled;
} statsConfig_t;

uint32_t getFlyingEnergy(void);
void statsInit(void);
void statsOnArm(void);
void statsOnDisarm(void);

#else

#define statsInit()     do {} while (0)
#define statsOnArm()    do {} while (0)
#define statsOnDisarm() do {} while (0)

#endif

#pragma once

#include <stdbool.h>
#include <stdint.h>

// Don't access the fields directly. Use the getter
// functions below.
typedef struct {
    int32_t unix;
    uint32_t nanos;
} timestamp_t;

timestamp_t timestamp_unix(int32_t unix, uint32_t nanos);
int32_t timestamp_get_unix(timestamp_t ts);
uint32_t timestamp_get_nanos(timestamp_t ts);

typedef struct _date_time_s {
    // full year
    uint16_t year;
    // 1-12
    uint8_t month;
    // 1-31
    uint8_t day;
    // 0-23
    uint8_t hours;
    // 0-59
    uint8_t minutes;
    // 0-59
    uint8_t seconds;
    uint32_t nanos;
} date_time_t;

#define FORMATTED_DATE_TIME_BUFSIZE 30

// buf must be at least FORMATTED_DATE_TIME_BUFSIZE
void date_time_format(char *buf, date_time_t *dt);

bool rtc_has_time();

bool rtc_get(timestamp_t *ts);
bool rtc_set(timestamp_t ts);

bool rtc_get_dt(date_time_t *dt);
bool rtc_set_dt(date_time_t *dt);
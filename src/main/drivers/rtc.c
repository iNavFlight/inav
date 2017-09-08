#include "common/printf.h"

#include "drivers/rtc.h"
#include "drivers/time.h"

#define UNIX_REFERENCE_YEAR 1970
#define NANOS_PER_SECOND 1000000000

// millis() when rtc_set() was called
static timeMs_t rtcMillisSet = 0;
// timestamp_t set via set_rtc_ts(), used
// as an offset to count from.
static timestamp_t rtcTsSet = {0,};

timestamp_t timestamp_unix(int32_t unix, uint32_t nanos)
{
    while (nanos >= NANOS_PER_SECOND) {
        nanos -= NANOS_PER_SECOND;
        unix++;
    }
    return (timestamp_t){ .unix = unix, nanos = nanos};
}

int32_t timestamp_get_unix(timestamp_t ts)
{
    return ts.unix;
}

uint32_t timestamp_get_nanos(timestamp_t ts)
{
    return ts.nanos;
}

static timestamp_t timestamp_add_millis(timestamp_t ts, timeMs_t ms) {
    uint32_t seconds = ms / 1000;
    uint32_t rem = ms % 1000;
    return timestamp_unix(timestamp_get_unix(ts) + seconds, timestamp_get_nanos(ts) + rem * 1000000);
}

static uint16_t days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};


static timestamp_t date_time_to_timestamp(date_time_t *dt)
{
    unsigned int second = dt->seconds;  // 0-59
    unsigned int minute = dt->minutes;  // 0-59
    unsigned int hour = dt->hours;      // 0-23
    unsigned int day = dt->day - 1;     // 0-30
    unsigned int month = dt->month - 1; // 0-11
    unsigned int year = dt->year - UNIX_REFERENCE_YEAR; // 0-99
    int32_t unix = (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
    return timestamp_unix(unix, dt->nanos);
}

static void timestamp_to_date_time(date_time_t *dt, timestamp_t ts)
{
    int32_t unix = timestamp_get_unix(ts);
    dt->seconds = unix%60;
    unix /= 60;
    dt->minutes = unix%60;
    unix /= 60;
    dt->hours = unix%24;
    unix /= 24;

    unsigned int years = unix/(365*4+1)*4;
    unix %= 365*4+1;

    unsigned int year;
    for (year=3; year>0; year--) {
        if (unix >= days[year][0]) {
            break;
        }
    }

    unsigned int month;
    for (month = 11; month > 0; month--) {
        if (unix >= days[year][month]) {
            break;
        }
    }

    dt->year = years + year + UNIX_REFERENCE_YEAR;
    dt->month = month + 1;
    dt->day = unix - days[year][month] + 1;
    dt->nanos = timestamp_get_nanos(ts);
}

void date_time_format(char *buf, date_time_t *dt)
{
    tfp_sprintf(buf, "%04u-%02u-%02uT%02u:%02u:%02u.%09d",
        dt->year, dt->month, dt->day,
        dt->hours, dt->minutes, dt->seconds, dt->nanos);
}

bool rtc_has_time()
{
    return rtcMillisSet > 0;
}

bool rtc_get(timestamp_t *ts)
{
    if (!rtc_has_time()) {
        return false;
    }
    timeMs_t delta = millis() - rtcMillisSet;
    *ts = timestamp_add_millis(rtcTsSet, delta);
    return true;
}

bool rtc_set(timestamp_t ts)
{
    rtcMillisSet = millis();
    rtcTsSet = ts;
    return true;
}

bool rtc_get_dt(date_time_t *dt)
{
    timestamp_t ts;
    if (rtc_get(&ts)) {
        timestamp_to_date_time(dt, ts);
        return true;
    }
    return false;
}

bool rtc_set_dt(date_time_t *dt)
{
    return rtc_set(date_time_to_timestamp(dt));
}
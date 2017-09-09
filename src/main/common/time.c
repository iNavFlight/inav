#include "common/printf.h"
#include "common/time.h"

#include "drivers/time.h"

#define UNIX_REFERENCE_YEAR 1970

// rtcTime_t when the system was started.
// Calculated in rtcSet().
static rtcTime_t started = 0;

static const uint16_t days[4][12] =
{
    {   0,  31,     60,     91,     121,    152,    182,    213,    244,    274,    305,    335},
    { 366,  397,    425,    456,    486,    517,    547,    578,    609,    639,    670,    700},
    { 731,  762,    790,    821,    851,    882,    912,    943,    974,    1004,   1035,   1065},
    {1096,  1127,   1155,   1186,   1216,   1247,   1277,   1308,   1339,   1369,   1400,   1430},
};


static rtcTime_t dateTimeToRtcTime(dateTime_t *dt)
{
    unsigned int second = dt->seconds;  // 0-59
    unsigned int minute = dt->minutes;  // 0-59
    unsigned int hour = dt->hours;      // 0-23
    unsigned int day = dt->day - 1;     // 0-30
    unsigned int month = dt->month - 1; // 0-11
    unsigned int year = dt->year - UNIX_REFERENCE_YEAR; // 0-99
    int32_t unix = (((year / 4 * (365 * 4 + 1) + days[year % 4][month] + day) * 24 + hour) * 60 + minute) * 60 + second;
    return rtcTimeMake(unix, dt->millis);
}

static void rtcTimeToDateTime(dateTime_t *dt, rtcTime_t *t)
{
    int32_t unix = *t / 1000;
    dt->seconds = unix % 60;
    unix /= 60;
    dt->minutes = unix % 60;
    unix /= 60;
    dt->hours = unix % 24;
    unix /= 24;

    unsigned int years = unix / (365 * 4 + 1) * 4;
    unix %= 365 * 4 + 1;

    unsigned int year;
    for (year = 3; year > 0; year--) {
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
    dt->millis = *t % 1000;
}

rtcTime_t rtcTimeMake(int32_t secs, uint16_t millis)
{
    return ((rtcTime_t)secs) * 1000 + millis;
}

void dateTimeFormat(char *buf, dateTime_t *dt)
{
    tfp_sprintf(buf, "%04u-%02u-%02uT%02u:%02u:%02u.%03d",
        dt->year, dt->month, dt->day,
        dt->hours, dt->minutes, dt->seconds, dt->millis);
}

bool rtcHasTime()
{
    return started != 0;
}

bool rtcGet(rtcTime_t *t)
{
    if (!rtcHasTime()) {
        return false;
    }
    *t = started + millis();
    return true;
}

bool rtcSet(rtcTime_t *t)
{
    started = *t - millis();
    return true;
}

bool rtcGetDateTime(dateTime_t *dt)
{
    rtcTime_t t;
    if (rtcGet(&t)) {
        rtcTimeToDateTime(dt, &t);
        return true;
    }
    // No time stored, fill dt with 0000-01-01T00:00:00.000
    dt->year = 0;
    dt->month = 1;
    dt->day = 1;
    dt->hours = 0;
    dt->minutes = 0;
    dt->seconds = 0;
    dt->millis = 0;
    return false;
}

bool rtcSetDateTime(dateTime_t *dt)
{
    rtcTime_t t = dateTimeToRtcTime(dt);
    return rtcSet(&t);
}
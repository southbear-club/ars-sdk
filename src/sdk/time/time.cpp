/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file time.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/time/time.hpp"
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

namespace aru {

namespace sdk {

static const char* s_weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static const char* s_months[] = {"January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"};

static const uint8_t s_days[] = \
//   1       3       5       7   8       10      12
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

datetime_t time_now(void) {
    datetime_t  dt;
    struct timeval tv;
    struct tm tm;

    gettimeofday(&tv, NULL);
    time_t tt = tv.tv_sec;
    localtime_r(&tt, &tm);

    dt.year     = tm.tm_year + 1900;
    dt.month    = tm.tm_mon  + 1;
    dt.day      = tm.tm_mday;
    dt.hour     = tm.tm_hour;
    dt.min      = tm.tm_min;
    dt.sec      = tm.tm_sec;
    dt.ms       = tv.tv_usec/1000;

    return dt;
}

time_t time_mktime(const datetime_t *dt) {
    struct tm tm;
    time_t ts;
    time(&ts);
    struct tm* ptm = localtime(&ts);
    memcpy(&tm, ptm, sizeof(struct tm));
    tm.tm_yday  = dt->year   - 1900;
    tm.tm_mon   = dt->month  - 1;
    tm.tm_mday  = dt->day;
    tm.tm_hour  = dt->hour;
    tm.tm_min   = dt->min;
    tm.tm_sec   = dt->sec;
    return mktime(&tm);
}

int days_of_month(int month, int year) {
    if (month < 1 || month > 12) {
        return 0;
    }
    int days = s_days[month-1];
    return (month == 2 && ARU_IS_LEAP_YEAR(year)) ? ++days : days;
}

datetime_t* datetime_past(datetime_t* dt, int days) {
    assert(days >= 0);
    int sub = days;
    while (sub) {
        if (dt->day > sub) {
            dt->day -= sub;
            break;
        }
        sub -= dt->day;
        if (--dt->month == 0) {
            dt->month = 12;
            --dt->year;
        }
        dt->day = days_of_month(dt->month, dt->year);
    }
    return dt;
}

datetime_t* datetime_future(datetime_t* dt, int days) {
    assert(days >= 0);
    int sub = days;
    int mdays;
    while (sub) {
        mdays = days_of_month(dt->month, dt->year);
        if (dt->day + sub <= mdays) {
            dt->day += sub;
            break;
        }
        sub -= (mdays - dt->day + 1);
        if (++dt->month > 12) {
            dt->month = 1;
            ++dt->year;
        }
        dt->day = 1;
    }
    return dt;
}

char* duration_fmt(int sec, char* buf) {
    int h, m, s;
    m = sec / 60;
    s = sec % 60;
    h = m / 60;
    m = m % 60;
    sprintf(buf, ARU_TIME_FMT, h, m, s);
    return buf;
}

char* datetime_fmt(datetime_t* dt, char* buf) {
    sprintf(buf, ARU_DATETIME_FMT,
        dt->year, dt->month, dt->day,
        dt->hour, dt->min, dt->sec);
    return buf;
}

char* gmtime_fmt(time_t time, char* buf) {
    struct tm* tm = gmtime(&time);
    //strftime(buf, ARU_GMTIME_FMT_BUFLEN, "%a, %d %b %Y %H:%M:%S GMT", tm);
    sprintf(buf, ARU_GMTIME_FMT,
        s_weekdays[tm->tm_wday],
        tm->tm_mday, s_months[tm->tm_mon], tm->tm_year + 1900,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
    return buf;
}

int month_atoi(const char* month) {
    for (size_t i = 0; i < 12; ++i) {
        if (strncasecmp(month, s_months[i], strlen(month)) == 0)
            return i+1;
    }
    return 0;
}

const char* month_itoa(int month) {
    assert(month >= 1 && month <= 12);
    return s_months[month-1];
}

int weekday_atoi(const char* weekday) {
    for (size_t i = 0; i < 7; ++i) {
        if (strncasecmp(weekday, s_weekdays[i], strlen(weekday)) == 0)
            return i;
    }
    return 0;
}

const char* weekday_itoa(int weekday) {
    assert(weekday >= 0 && weekday <= 7);
    if (weekday == 7) weekday = 0;
    return s_weekdays[weekday];
}

datetime_t compile_datetime() {
    datetime_t dt;
    char month[32];
    sscanf(__DATE__, "%s %d %d", month, &dt.day, &dt.year);
    sscanf(__TIME__, "%d:%d:%d", &dt.hour, &dt.min, &dt.sec);
    dt.month = month_atoi(month);
    return dt;
}

time_t cron_next_timeout(int minute, int hour, int day, int week, int month) {
    enum {
        UNKOWN,
        HOURLY,
        DAILY,
        WEEKLY,
        MONTHLY,
        YEARLY,
    } period_type = UNKOWN;
    struct tm tm;
    time_t tt;
    time(&tt);
    localtime_r(&tt, &tm);
    time_t tt_round = 0;

    tm.tm_sec = 0;
    if (minute >= 0) {
        period_type = HOURLY;
        tm.tm_min = minute;
    }
    if (hour >= 0) {
        period_type = DAILY;
        tm.tm_hour = hour;
    }
    if (week >= 0) {
        period_type = WEEKLY;
    }
    else if (day > 0) {
        period_type = MONTHLY;
        tm.tm_mday = day;
        if (month > 0) {
            period_type = YEARLY;
            tm.tm_mon = month - 1;
        }
    }

    if (period_type == UNKOWN) {
        return -1;
    }

    tt_round = mktime(&tm);
    if (week >= 0) {
        tt_round = tt + (week-tm.tm_wday)*ARU_SECONDS_PER_DAY;
    }
    if (tt_round > tt) {
        return tt_round;
    }

    switch(period_type) {
    case HOURLY:
        tt_round += ARU_SECONDS_PER_HOUR;
        return tt_round;
    case DAILY:
        tt_round += ARU_SECONDS_PER_DAY;
        return tt_round;
    case WEEKLY:
        tt_round += ARU_SECONDS_PER_WEEK;
        return tt_round;
    case MONTHLY:
        if (++tm.tm_mon == 12) {
            tm.tm_mon = 0;
            ++tm.tm_year;
        }
        break;
    case YEARLY:
        ++tm.tm_year;
        break;
    default:
        return -1;
    }

    return mktime(&tm);
}

} // namesapce sdk

} // namesapce aru

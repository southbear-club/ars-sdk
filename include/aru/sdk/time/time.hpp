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
 * @file time.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

namespace aru {

namespace sdk {

#define ARU_SECONDS_PER_HOUR    3600
#define ARU_SECONDS_PER_DAY     86400   // 24*3600
#define ARU_SECONDS_PER_WEEK    604800  // 7*24*3600

#define ARU_IS_LEAP_YEAR(year) (((year)%4 == 0 && (year)%100 != 0) || (year)%100 == 0)

typedef struct datetime_s {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int ms;
} datetime_t;

typedef struct {
    time_t sec;
    time_t usec;
} timeval_t;

typedef struct {
    time_t sec;
    time_t usec;
} timespec_t;

struct timezone {
    int tz_minuteswest; /* of Greenwich */
    int tz_dsttime;     /* type of dst correction to apply */
};

static inline void ddelay(time_t day) {
    usleep(day * 24 * 60 * 60 * 1000 * 1000);
}

static inline void hdelay(time_t hour) {
    usleep(hour * 60 * 60 * 1000 * 1000);
}

static inline void mdelay(time_t min) {
    usleep(min * 60 * 1000 * 1000);
}

static inline void sdelay(time_t sec) {
    usleep(sec * 1000 * 1000);
}

static inline void msdelay(time_t ms) {
    usleep(ms * 1000);
}

static inline void usdelay(time_t us) {
    usleep(us);
}

static inline timeval_t gettime_val(void) {
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    timeval_t ret {
        tv.tv_sec,
        tv.tv_usec,
    };
    return ret;
}

static inline timespec_t gettime_spec(void) {
    struct timespec sp;
    ::clock_gettime(CLOCK_MONOTONIC, &sp);
    timespec_t ret {
        sp.tv_sec,
        sp.tv_nsec,
    };

    return ret;
}

static inline unsigned long long gettimeofday_ms() {
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return tv.tv_sec * (unsigned long long)1000 + tv.tv_usec/1000;
}

static inline unsigned int gettick(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static inline unsigned long long gethrtime_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec*(unsigned long long)1000000 + ts.tv_nsec / 1000;
}

datetime_t time_now(void);
time_t time_mktime(const datetime_t *dt);
datetime_t* datetime_past(datetime_t* dt, int days = 1);
datetime_t* datetime_future(datetime_t* dt, int days = 1);

#define ARU_TIME_FMT            "%02d:%02d:%02d"
#define ARU_TIME_FMT_BUFLEN     12
char* duration_fmt(int sec, char* buf);

#define ARU_DATETIME_FMT        "%04d-%02d-%02d %02d:%02d:%02d"
#define ARU_DATETIME_FMT_BUFLEN 24
char* datetime_fmt(datetime_t* dt, char* buf);

#define ARU_GMTIME_FMT          "%.3s, %02d %.3s %04d %02d:%02d:%02d GMT"
#define ARU_GMTIME_FMT_BUFLEN   30
char* gmtime_fmt(time_t time, char* buf);

int days_of_month(int month, int year);

int month_atoi(const char* month);
const char* month_itoa(int month);

int weekday_atoi(const char* weekday);
const char* weekday_itoa(int weekday);

datetime_t compile_datetime();

/*
 * minute   hour    day     week    month       action
 * 0~59     0~23    1~31    0~6     1~12
 *  30      -1      -1      -1      -1          cron.hourly
 *  30      1       -1      -1      -1          cron.daily
 *  30      1       15      -1      -1          cron.monthly
 *  30      1       -1       0      -1          cron.weekly
 *  30      1        1      -1      10          cron.yearly
 */
time_t cron_next_timeout(int minute, int hour, int day, int week, int month);

} // namespace sdk

} // namespace aru

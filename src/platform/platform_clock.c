#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "platform/platform_clock.h"

#ifdef _WIN32

#include <windows.h>

uint64_t platform_clock_now_ms(void)
{
    return (uint64_t)GetTickCount64();
}

void platform_clock_sleep_ms(uint32_t ms)
{
    Sleep(ms);
}

#else

#include <time.h>

uint64_t platform_clock_now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000u + (uint64_t)ts.tv_nsec / 1000000u;
}

void platform_clock_sleep_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(ms / 1000u);
    ts.tv_nsec = (long)(ms % 1000u) * 1000000L;
    nanosleep(&ts, NULL);
}

#endif

#include "utils.h"
#include "core.h"

#include <time.h>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <threads.h>
#endif


void timespec_diff(const struct timespec *lhs, const struct timespec *rhs, struct timespec *result)
{
    result->tv_sec = lhs->tv_sec - rhs->tv_sec;
    result->tv_nsec = lhs->tv_nsec - rhs->tv_nsec;

    if (result->tv_nsec < 0)
    {
        result->tv_sec--;
        result->tv_nsec += 1000000000L;
    }
}


float getUptime(void)
{
    struct timespec now, diff;

    timespec_get(&now, TIME_UTC);
    timespec_diff(&now, &engine.metrics.time.startTime, &diff);

    return (float)diff.tv_sec + (float)diff.tv_nsec / 1000000000.f;
}


// https://www.c-plusplus.net/forum/topic/109539/usleep-unter-windows
void denymSleepTimespec(const struct timespec *duration)
{
    if(duration->tv_nsec < 0 || duration->tv_sec < 0)
        return;

#ifdef _MSC_VER
    // convert to 100ns intervals
    LARGE_INTEGER ft = {
        .QuadPart = -(duration->tv_nsec / 100 + duration->tv_sec * 10000000)
    };

    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
#else
    thrd_sleep(duration, NULL);
#endif
}


void denymSleep(float duration)
{
    if(duration <= 0)
        return;

#ifdef _MSC_VER
    // convert to 100ns intervals
    LARGE_INTEGER ft = {
        .QuadPart = -(duration * 10000000)
    };

    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
#else
    float sec = floorf(duration);
    long nsec = (long)((duration - sec) * 1000000000.f);

    struct timespec ts = {
        .tv_sec = (long)sec,
        .tv_nsec = nsec
    };

    thrd_sleep(&ts, NULL);
#endif
}

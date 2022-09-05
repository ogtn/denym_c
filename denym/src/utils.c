#include "utils.h"
#include "core.h"
#include <time.h>


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
    timespec_diff(&now, &engine.uptime, &diff);
    
    return (float)diff.tv_sec + (float)diff.tv_nsec / 1000000000.f;
}

#ifndef _utils_h_
#define _utils_h_


#include "denym_common.h"
#include <time.h>


 void timespec_diff(const struct timespec *lhs, const struct timespec *rhs, struct timespec *result);

 float getUptime(void);

 void denymSleep(const struct timespec *duration);


#endif

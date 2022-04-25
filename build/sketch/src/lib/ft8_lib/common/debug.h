#line 1 "/home/corrado/programming/arduino/ft8-light/src/lib/ft8_lib/common/debug.h"
#pragma once

#include <stdio.h>

#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARN    2
#define LOG_ERROR   3
#define LOG_FATAL   4


#define LOG(level, ...)     if (level >= LOG_LEVEL) fprintf(stderr, __VA_ARGS__)

#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef DEBUG
#include <stdio.h>
#define TRACE(fmt, ...) fprintf(stderr, "%s:%d:%s\tTRACE\t" fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define TRACE(fmt, ...)
#endif

#endif
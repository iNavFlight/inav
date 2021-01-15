#ifndef DBGTRACE_H_
#define DBGTRACE_H_

#include "chprintf.h"

#if !defined(DEBUG_TRACE_PRINT)
#define DEBUG_TRACE_PRINT     FALSE
#endif

#if !defined(DEBUG_TRACE_WARNING)
#define DEBUG_TRACE_WARNING   FALSE
#endif

#if !defined(DEBUG_TRACE_ERROR)
#define DEBUG_TRACE_ERROR     FALSE
#endif

/* user must provide correctly initialized pointer to print channel */
#if DEBUG_TRACE_PRINT || DEBUG_TRACE_WARNING || DEBUG_TRACE_ERROR
extern BaseSequentialStream   *GlobalDebugChannel;
#endif

#if DEBUG_TRACE_PRINT
#define dbgprintf(fmt, ...)  chprintf(GlobalDebugChannel, fmt, ##__VA_ARGS__)
#else
#define dbgprintf(fmt, ...)  do {} while(0)
#endif

#if DEBUG_TRACE_WARNING
#define warnprintf(fmt, ...)  chprintf(GlobalDebugChannel, fmt, ##__VA_ARGS__)
#else
#define warnprintf(fmt, ...)  do {} while(0)
#endif

#if DEBUG_TRACE_ERROR
#define errprintf(fmt, ...)  chprintf(GlobalDebugChannel, fmt, ##__VA_ARGS__)
#else
#define errprintf(fmt, ...)  do {} while(0)
#endif

#endif /* DBGTRACE_H_ */

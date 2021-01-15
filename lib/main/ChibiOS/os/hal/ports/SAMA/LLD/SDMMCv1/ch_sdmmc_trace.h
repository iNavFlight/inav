#ifndef CH_SDMMC_TRACE_H_
#define CH_SDMMC_TRACE_H_


#if SAMA_SDMMC_TRACE == 1

#include "chprintf.h"
extern BaseSequentialStream * ts;

#define TRACE(s)		chprintf(ts,s)
#define TRACE_1(s,v1)	chprintf(ts,s,v1)
#define TRACE_2(s,v1,v2)	chprintf(ts,s,v1,v2)
#define TRACE_3(s,v1,v2,v3)	chprintf(ts,s,v1,v3)
#define TRACE_4(s,v1,v2,v3,v4)	chprintf(ts,s,v1,v2,v3,v4)
#define TRACE_5(s,v1,v2,v3,v4,v5)	chprintf(ts,s,v1,v2,v3,v4,v5)
#define TRACE_6(s,v1,v2,v3,v4,v5,v6)	chprintf(ts,s,v1,v2,v3,v4,v5,v6)
#define TRACE_LEV_1(s,v1) TRACE_1(s,v1);
#else
#define TRACE(s)
#define TRACE_1(s,v1)
#define TRACE_2(s,v1,v2)
#define TRACE_3(s,v1,v2,v3)
#define TRACE_4(s,v1,v2,v3,v4)
#define TRACE_5(s,v1,v2,v3,v4,v5)
#define TRACE_6(s,v1,v2,v3,v4,v5,v6)
#endif

#if SAMA_SDMMC_TRACE_LEVEL >= 1
#define TRACE_INFO(s)						TRACE(s)
#define TRACE_INFO_1(s,v1)					TRACE_1(s,v1)
#define TRACE_INFO_2(s,v1,v2)				TRACE_2(s,v1,v2)
#define TRACE_INFO_3(s,v1,v2,v3)			TRACE_3(s,v1,v2,v3)
#define TRACE_INFO_4(s,v1,v2,v3,v4)			TRACE_4(s,v1,v2,v3,v4)
#define TRACE_INFO_5(s,v1,v2,v3,v4,v5)		TRACE_5(s,v1,v2,v3,v4,v5)
#define TRACE_INFO_6(s,v1,v2,v3,v4,v5,v6)	TRACE_6(s,v1,v2,v3,v4,v5,v6)
#else
#define TRACE_INFO(s)
#define TRACE_INFO_1(s,v1)
#define TRACE_INFO_2(s,v1,v2)
#define TRACE_INFO_3(s,v1,v2,v3)
#define TRACE_INFO_4(s,v1,v2,v3,v4)
#define TRACE_INFO_5(s,v1,v2,v3,v4,v5)
#define TRACE_INFO_6(s,v1,v2,v3,v4,v5,v6)
#endif


#if SAMA_SDMMC_TRACE_LEVEL >= 2
#define TRACE_WARNING(s)						TRACE(s)
#define TRACE_WARNING_1(s,v1)					TRACE_1(s,v1)
#define TRACE_WARNING_2(s,v1,v2)				TRACE_2(s,v1,v2)
#define TRACE_WARNING_3(s,v1,v2,v3)				TRACE_3(s,v1,v2,v3)
#define TRACE_WARNING_4(s,v1,v2,v3,v4)			TRACE_4(s,v1,v2,v3,v4)
#define TRACE_WARNING_5(s,v1,v2,v3,v4,v5)		TRACE_5(s,v1,v2,v3,v4,v5)
#define TRACE_WARNING_6(s,v1,v2,v3,v4,v5,v6)	TRACE_6(s,v1,v2,v3,v4,v5,v6)
#else
#define TRACE_WARNING(s)
#define TRACE_WARNING_1(s,v1)
#define TRACE_WARNING_2(s,v1,v2)
#define TRACE_WARNING_3(s,v1,v2,v3)
#define TRACE_WARNING_4(s,v1,v2,v3,v4)
#define TRACE_WARNING_5(s,v1,v2,v3,v4,v5)
#define TRACE_WARNING_6(s,v1,v2,v3,v4,v5,v6)
#endif


#if SAMA_SDMMC_TRACE_LEVEL >= 3
#define TRACE_ERROR(s)						TRACE(s)
#define TRACE_ERROR_1(s,v1)					TRACE_1(s,v1)
#define TRACE_ERROR_2(s,v1,v2)				TRACE_2(s,v1,v2)
#define TRACE_ERROR_3(s,v1,v2,v3)			TRACE_3(s,v1,v2,v3)
#define TRACE_ERROR_4(s,v1,v2,v3,v4)		TRACE_4(s,v1,v2,v3,v4)
#define TRACE_ERROR_5(s,v1,v2,v3,v4,v5)		TRACE_5(s,v1,v2,v3,v4,v5)
#define TRACE_ERROR_6(s,v1,v2,v3,v4,v5,v6)	TRACE_6(s,v1,v2,v3,v4,v5,v6)
#else
#define TRACE_ERROR(s)
#define TRACE_ERROR_1(s,v1)
#define TRACE_ERROR_2(s,v1,v2)
#define TRACE_ERROR_3(s,v1,v2,v3)
#define TRACE_ERROR_4(s,v1,v2,v3,v4)
#define TRACE_ERROR_5(s,v1,v2,v3,v4,v5)
#define TRACE_ERROR_6(s,v1,v2,v3,v4,v5,v6)
#endif

#if SAMA_SDMMC_TRACE_LEVEL >= 4
#define TRACE_DEBUG(s)						TRACE(s)
#define TRACE_DEBUG_1(s,v1)					TRACE_1(s,v1)
#define TRACE_DEBUG_2(s,v1,v2)				TRACE_2(s,v1,v2)
#define TRACE_DEBUG_3(s,v1,v2,v3)				TRACE_3(s,v1,v2,v3)
#define TRACE_DEBUG_4(s,v1,v2,v3,v4)			TRACE_4(s,v1,v2,v3,v4)
#define TRACE_DEBUG_5(s,v1,v2,v3,v4,v5)		TRACE_5(s,v1,v2,v3,v4,v5)
#define TRACE_DEBUG_6(s,v1,v2,v3,v4,v5,v6)	TRACE_6(s,v1,v2,v3,v4,v5,v6)
#else
#define TRACE_DEBUG(s)
#define TRACE_DEBUG_1(s,v1)
#define TRACE_DEBUG_2(s,v1,v2)
#define TRACE_DEBUG_3(s,v1,v2,v3)
#define TRACE_DEBUG_4(s,v1,v2,v3,v4)
#define TRACE_DEBUG_5(s,v1,v2,v3,v4,v5)
#define TRACE_DEBUG_6(s,v1,v2,v3,v4,v5,v6)
#endif



#endif /* CH_SDMMC_TRACE_H_ */

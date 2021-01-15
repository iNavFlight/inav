#ifndef CH_SDMMC_MACROS_H_
#define CH_SDMMC_MACROS_H_


#if !defined(CACHE_ALIGNED)
#define CACHE_ALIGNED  __attribute__((aligned(L1_CACHE_BYTES)))
#endif

/*
 * @brief    NO CACHE attribute
 */
#if !defined(NO_CACHE)
#define NO_CACHE                      __attribute__((section (".nocache")))
#endif

#define IS_CACHE_ALIGNED(x) ((((uint32_t)(x)) & (L1_CACHE_BYTES - 1)) == 0)
#if !defined(ROUND_INT_DIV)
#define ROUND_INT_DIV(n,d) (((n) + ((d)-1)) / (d))
#endif
#define ROUND_UP_MULT(x,m) (((x) + ((m)-1)) & ~((m)-1))
#define CEIL_INT_DIV(n,d) (((n) + (d) - 1) / (d))
#define ABS_DIFF(a,b) ((a) < (b) ? (b) - (a) : (a) - (b))
#define ARRAY_SIZE(x) (sizeof ((x)) / sizeof(*(x)))

#define _PrintTitle(s) TRACE(s)
#define _PrintField(f,v)	TRACE_1(f,v)

static inline uint32_t max_u32(uint32_t a, uint32_t b)
{
	return a > b ? a : b;
}

static inline uint32_t min_u32(uint32_t a, uint32_t b)
{
	return a < b ? a : b;
}

#endif /* CH_SDMMC_MACROS_H_ */

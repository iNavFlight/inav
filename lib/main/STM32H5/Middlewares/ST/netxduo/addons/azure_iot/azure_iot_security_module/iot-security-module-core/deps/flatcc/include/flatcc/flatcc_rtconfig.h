#ifndef FLATCC_RTCONFIG_H
#define FLATCC_RTCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif


/* Include portability layer here since all other files depend on it. */
#ifdef FLATCC_PORTABLE
#include "flatcc/portable/portable.h"
#endif

/*
 * Fast printing and parsing of double.
 *
 * This requires the grisu3/grisu3_* files to be in the include path,
 * otherwise strod and sprintf will be used (these needed anyway
 * as fallback for cases not supported by grisu3).
 */
#ifndef FLATCC_USE_GRISU3
#define FLATCC_USE_GRISU3 1
#endif

/*
 * This requires compiler that has enabled marc=native or similar so
 * __SSE4_2__ flag is defined. Otherwise it will have no effect.
 *
 * While SSE may be used for different purposes, it has (as of this
 * writing) only be used to test the effect on JSON whitespace handling
 * which improved, but not by a lot, assuming 64-bit unligned access is
 * otherwise available:
 *
 * With 8 space indentation, the JSON benchmark handles 308K parse ops/sec
 * while SSE ups that to 333 parse ops/sec or 336 if \r\n is also
 * consumed by SSE. Disabling indentation leaves SSE spacing handling
 * ineffective, and performance reaches 450K parse ops/sec and can
 * improve further to 500+K parse ops/sec if inexact GRISU3 numbers are
 * allowed (they are pretty accurate anyway, just not exact). This
 * feature requires hacking a flag direct in the grisu3 double parsing
 * lib directly and only mentioned for comparison.
 *
 * In conclusion SSE doesn't add a lot to JSON space handling at least.
 *
 * Disabled by default, but can be overriden by build system.
 */
#ifndef FLATCC_USE_SSE4_2
#define FLATCC_USE_SSE4_2 0
#endif

/*
 * The verifier only reports yes and no. The following setting
 * enables assertions in debug builds. It must be compiled into
 * the runtime library and is not normally the desired behavior.
 *
 * NOTE: enabling this can break test cases so use with build, not test.
 */
#if !defined(FLATCC_DEBUG_VERIFY) && !defined(NDEBUG)
#define FLATCC_DEBUG_VERIFY 0
#endif

#if !defined(FLATCC_TRACE_VERIFY)
#define FLATCC_TRACE_VERIFY 0
#endif


/*
 * Limit recursion level for tables. Actual level may be deeper
 * when structs are deeply nested - but these are limited by the
 * schema compiler.
 */
#ifndef FLATCC_JSON_PRINT_MAX_LEVELS
#define FLATCC_JSON_PRINT_MAX_LEVELS 100
#endif

/* Maximum length of names printed exluding _type suffix. */
#ifndef FLATCC_JSON_PRINT_NAME_LEN_MAX
#define FLATCC_JSON_PRINT_NAME_LEN_MAX 100
#endif

/*
 * Print float and double values with C99 hexadecimal floating point
 * notation. This option is not valid JSON but it avoids precision
 * loss, correctly handles NaN, +/-Infinity and is significantly faster
 * to parse and print. Some JSON parsers rely on strtod which does
 * support hexadecimal floating points when C99 compliant.
 */
#ifndef FLATCC_JSON_PRINT_HEX_FLOAT
#define FLATCC_JSON_PRINT_HEX_FLOAT 0
#endif

/*
 * Always print multipe enum flags like `color: "Red Green"`
 * even when unquote is selected as an option for single
 * value like `color: Green`. Otherwise multiple values
 * are printed as `color: Red Green`, but this could break
 * some flatbuffer json parser.
 */
#ifndef FLATCC_JSON_PRINT_ALWAYS_QUOTE_MULTIPLE_FLAGS
#define FLATCC_JSON_PRINT_ALWAYS_QUOTE_MULTIPLE_FLAGS 1
#endif

/*
 * The general nesting limit may be lower, but for skipping
 * JSON we do not need to - we can set this high as it only
 * costs a single char per level in a stack array.
 */
#ifndef FLATCC_JSON_PARSE_GENERIC_MAX_NEST
#define FLATCC_JSON_PARSE_GENERIC_MAX_NEST 512
#endif

/* Store value even if it is default. */
#ifndef FLATCC_JSON_PARSE_FORCE_DEFAULTS
#define FLATCC_JSON_PARSE_FORCE_DEFAULTS 0
#endif

#ifndef FLATCC_JSON_PARSE_ALLOW_UNQUOTED
#define FLATCC_JSON_PARSE_ALLOW_UNQUOTED 1
#endif

/*
 * Multiple enum values are by default not permitted unless
 * quoted like `color: "Red Green" as per Googles flatc JSON
 * parser while a single value like `color: Red` can be
 * unquoted.  Enabling this setting will allow `color: Red
 * Green`, but only if FLATCC_JSON_PARSE_ALLOW_UNQUOTED is
 * also enabled.
 */
#ifndef FLATCC_JSON_PARSE_ALLOW_UNQUOTED_LIST
#define FLATCC_JSON_PARSE_ALLOW_UNQUOTED_LIST 0
#endif

#ifndef FLATCC_JSON_PARSE_ALLOW_UNKNOWN_FIELD
#define FLATCC_JSON_PARSE_ALLOW_UNKNOWN_FIELD 1
#endif

#ifndef FLATCC_JSON_PARSE_ALLOW_TRAILING_COMMA
#define FLATCC_JSON_PARSE_ALLOW_TRAILING_COMMA 1
#endif

/*
 * Just parse to the closing bracket '}' if set.
 * Otherwise parse to end by consuming space and
 * fail if anything but space follows.
 */
#ifndef FLATCC_PARSE_IGNORE_TRAILING_DATA
#define FLATCC_PARSE_IGNORE_TRAILING_DATA 0
#endif

/*
 * Optimize to parse a lot of white space, but
 * in most cases it probably slows parsing down.
 */
#ifndef FLATCC_JSON_PARSE_WIDE_SPACE
#define FLATCC_JSON_PARSE_WIDE_SPACE 0
#endif

#ifdef __cplusplus
}
#endif

#endif /* FLATCC_RTCONFIG_H */

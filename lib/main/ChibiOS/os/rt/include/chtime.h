/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio.

    This file is part of ChibiOS.

    ChibiOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    chtime.h
 * @brief   Time and intervals macros and structures.
 *
 * @addtogroup time_intervals
 * @details This module is responsible for handling of system time and time
 *          intervals.
 * @{
 */

#ifndef CHTIME_H
#define CHTIME_H

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/**
 * @name    Special time constants
 * @{
 */
/**
 * @brief   Zero interval specification for some functions with a timeout
 *          specification.
 * @note    Not all functions accept @p TIME_IMMEDIATE as timeout parameter,
 *          see the specific function documentation.
 */
#define TIME_IMMEDIATE      ((sysinterval_t)0)

/**
 * @brief   Infinite interval specification for all functions with a timeout
 *          specification.
 * @note    Not all functions accept @p TIME_INFINITE as timeout parameter,
 *          see the specific function documentation.
 */
#define TIME_INFINITE       ((sysinterval_t)-1)

/**
 * @brief   Maximum interval constant usable as timeout.
 */
#define TIME_MAX_INTERVAL   ((sysinterval_t)-2)

/**
 * @brief   Maximum system of system time before it wraps.
 */
#define TIME_MAX_SYSTIME    ((systime_t)-1)
/** @} */

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (CH_CFG_ST_RESOLUTION != 16) && (CH_CFG_ST_RESOLUTION != 32) &&         \
    (CH_CFG_ST_RESOLUTION != 64)
#error "invalid CH_CFG_ST_RESOLUTION specified, must be 16, 32 or 64"
#endif

#if CH_CFG_ST_FREQUENCY < 10
#error "invalid CH_CFG_ST_FREQUENCY specified, must be >= 10"
#endif

#if (CH_CFG_INTERVALS_SIZE != 16) && (CH_CFG_INTERVALS_SIZE != 32) &&       \
    (CH_CFG_INTERVALS_SIZE != 64)
#error "invalid CH_CFG_INTERVALS_SIZE specified, must be 16, 32 or 64"
#endif

#if (CH_CFG_TIME_TYPES_SIZE != 16) && (CH_CFG_TIME_TYPES_SIZE != 32)
#error "invalid CH_CFG_TIME_TYPES_SIZE specified, must be 16 or 32"
#endif

#if CH_CFG_INTERVALS_SIZE < CH_CFG_ST_RESOLUTION
#error "CH_CFG_INTERVALS_SIZE must be >= CH_CFG_ST_RESOLUTION"
#endif

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of system time.
 * @note    It is selectable in configuration between 16, 32 or 64 bits.
 */
#if (CH_CFG_ST_RESOLUTION == 64) || defined(__DOXYGEN__)
typedef uint64_t systime_t;
#elif CH_CFG_ST_RESOLUTION == 32
typedef uint32_t systime_t;
#elif CH_CFG_ST_RESOLUTION == 16
typedef uint16_t systime_t;
#endif

/**
 * @brief   Type of time interval.
 * @note    It is selectable in configuration between 16, 32 or 64 bits.
 */
#if (CH_CFG_INTERVALS_SIZE == 64) || defined(__DOXYGEN__)
typedef uint64_t sysinterval_t;
#elif CH_CFG_INTERVALS_SIZE == 32
typedef uint32_t sysinterval_t;
#elif CH_CFG_INTERVALS_SIZE == 16
typedef uint16_t sysinterval_t;
#endif

#if (CH_CFG_TIME_TYPES_SIZE == 32) || defined(__DOXYGEN__)
/**
 * @brief   Type of seconds.
 * @note    It is selectable in configuration between 16 or 32 bits.
 */
typedef uint32_t time_secs_t;

/**
 * @brief   Type of milliseconds.
 * @note    It is selectable in configuration between 16 or 32 bits.
 */
typedef uint32_t time_msecs_t;

/**
 * @brief   Type of microseconds.
 * @note    It is selectable in configuration between 16 or 32 bits.
 */
typedef uint32_t time_usecs_t;

/**
 * @brief   Type of time conversion variable.
 * @note    This type must have double width than other time types, it is
 *          only used internally for conversions.
 */
typedef uint64_t time_conv_t;

#else
typedef uint16_t time_secs_t;
typedef uint16_t time_msecs_t;
typedef uint16_t time_usecs_t;
typedef uint32_t time_conv_t;
#endif

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @name    Fast time conversion utilities
 * @{
 */
/**
 * @brief   Seconds to time interval.
 * @details Converts from seconds to system ticks number.
 * @note    The result is rounded upward to the next tick boundary.
 * @note    Use of this macro for large values is not secure because
 *          integer overflows, make sure your value can be correctly
 *          converted.
 *
 * @param[in] secs      number of seconds
 * @return              The number of ticks.
 *
 * @api
 */
#define TIME_S2I(secs)                                                      \
  ((sysinterval_t)((time_conv_t)(secs) * (time_conv_t)CH_CFG_ST_FREQUENCY))

/**
 * @brief   Milliseconds to time interval.
 * @details Converts from milliseconds to system ticks number.
 * @note    The result is rounded upward to the next tick boundary.
 * @note    Use of this macro for large values is not secure because
 *          integer overflows, make sure your value can be correctly
 *          converted.
 *
 * @param[in] msecs     number of milliseconds
 * @return              The number of ticks.
 *
 * @api
 */
#define TIME_MS2I(msecs)                                                    \
  ((sysinterval_t)((((time_conv_t)(msecs) *                                 \
                     (time_conv_t)CH_CFG_ST_FREQUENCY) +                    \
                    (time_conv_t)999) / (time_conv_t)1000))

/**
 * @brief   Microseconds to time interval.
 * @details Converts from microseconds to system ticks number.
 * @note    The result is rounded upward to the next tick boundary.
 * @note    Use of this macro for large values is not secure because
 *          integer overflows, make sure your value can be correctly
 *          converted.
 *
 * @param[in] usecs     number of microseconds
 * @return              The number of ticks.
 *
 * @api
 */
#define TIME_US2I(usecs)                                                    \
  ((sysinterval_t)((((time_conv_t)(usecs) *                                 \
                     (time_conv_t)CH_CFG_ST_FREQUENCY) +                    \
                    (time_conv_t)999999) / (time_conv_t)1000000))

/**
 * @brief   Time interval to seconds.
 * @details Converts from system ticks number to seconds.
 * @note    The result is rounded up to the next second boundary.
 * @note    Use of this macro for large values is not secure because
 *          integer overflows, make sure your value can be correctly
 *          converted.
 *
 * @param[in] interval  interval in ticks
 * @return              The number of seconds.
 *
 * @api
 */
#define TIME_I2S(interval)                                                  \
  (time_secs_t)(((time_conv_t)(interval) +                                  \
                 (time_conv_t)CH_CFG_ST_FREQUENCY -                         \
                 (time_conv_t)1) / (time_conv_t)CH_CFG_ST_FREQUENCY)

/**
 * @brief   Time interval to milliseconds.
 * @details Converts from system ticks number to milliseconds.
 * @note    The result is rounded up to the next millisecond boundary.
 * @note    Use of this macro for large values is not secure because
 *          integer overflows, make sure your value can be correctly
 *          converted.
 *
 * @param[in] interval  interval in ticks
 * @return              The number of milliseconds.
 *
 * @api
 */
#define TIME_I2MS(interval)                                                 \
  (time_msecs_t)((((time_conv_t)(interval) * (time_conv_t)1000) +           \
                  (time_conv_t)CH_CFG_ST_FREQUENCY - (time_conv_t)1) /      \
                 (time_conv_t)CH_CFG_ST_FREQUENCY)

/**
 * @brief   Time interval to microseconds.
 * @details Converts from system ticks number to microseconds.
 * @note    The result is rounded up to the next microsecond boundary.
 * @note    Use of this macro for large values is not secure because
 *          integer overflows, make sure your value can be correctly
 *          converted.
 *
 * @param[in] interval  interval in ticks
 * @return              The number of microseconds.
 *
 * @api
 */
#define TIME_I2US(interval)                                                 \
    (time_msecs_t)((((time_conv_t)(interval) * (time_conv_t)1000000) +      \
                    (time_conv_t)CH_CFG_ST_FREQUENCY - (time_conv_t)1) /    \
                   (time_conv_t)CH_CFG_ST_FREQUENCY)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/*
 * Virtual Timers APIs.
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/**
 * @name    Secure time conversion utilities
 * @{
 */
/**
 * @brief   Seconds to time interval.
 * @details Converts from seconds to system ticks number.
 * @note    The result is rounded upward to the next tick boundary.
 *
 * @param[in] secs      number of seconds
 * @return              The number of ticks.
 *
 * @special
 */
static inline sysinterval_t chTimeS2I(time_secs_t secs) {
  time_conv_t ticks;

  ticks = (time_conv_t)secs * (time_conv_t)CH_CFG_ST_FREQUENCY;

  chDbgAssert(ticks <= (time_conv_t)TIME_MAX_INTERVAL,
              "conversion overflow");

  return (sysinterval_t)ticks;
}

/**
 * @brief   Milliseconds to time interval.
 * @details Converts from milliseconds to system ticks number.
 * @note    The result is rounded upward to the next tick boundary.
 *
 * @param[in] msec      number of milliseconds
 * @return              The number of ticks.
 *
 * @special
 */
static inline sysinterval_t chTimeMS2I(time_msecs_t msec) {
  time_conv_t ticks;

  ticks = (((time_conv_t)msec * (time_conv_t)CH_CFG_ST_FREQUENCY) +
           (time_conv_t)999) / (time_conv_t)1000;

  chDbgAssert(ticks <= (time_conv_t)TIME_MAX_INTERVAL,
              "conversion overflow");

  return (sysinterval_t)ticks;
}

/**
 * @brief   Microseconds to time interval.
 * @details Converts from microseconds to system ticks number.
 * @note    The result is rounded upward to the next tick boundary.
 *
 * @param[in] usec      number of microseconds
 * @return              The number of ticks.
 *
 * @special
 */
static inline sysinterval_t chTimeUS2I(time_usecs_t usec) {
  time_conv_t ticks;

  ticks = (((time_conv_t)usec * (time_conv_t)CH_CFG_ST_FREQUENCY) +
           (time_conv_t)999999) / (time_conv_t)1000000;

  chDbgAssert(ticks <= (time_conv_t)TIME_MAX_INTERVAL,
              "conversion overflow");

  return (sysinterval_t)ticks;
}

/**
 * @brief   Time interval to seconds.
 * @details Converts from system interval to seconds.
 * @note    The result is rounded up to the next second boundary.
 *
 * @param[in] interval  interval in ticks
 * @return              The number of seconds.
 *
 * @special
 */
static inline time_secs_t chTimeI2S(sysinterval_t interval) {
  time_conv_t secs;

  secs = ((time_conv_t)interval +
          (time_conv_t)CH_CFG_ST_FREQUENCY -
          (time_conv_t)1) / (time_conv_t)CH_CFG_ST_FREQUENCY;

  chDbgAssert(secs < (time_conv_t)((time_secs_t)-1),
              "conversion overflow");

  return (time_secs_t)secs;
}

/**
 * @brief   Time interval to milliseconds.
 * @details Converts from system interval to milliseconds.
 * @note    The result is rounded up to the next millisecond boundary.
 *
 * @param[in] interval  interval in ticks
 * @return              The number of milliseconds.
 *
 * @special
 */
static inline time_msecs_t chTimeI2MS(sysinterval_t interval) {
  time_conv_t msecs;

  msecs = (((time_conv_t)interval * (time_conv_t)1000) +
           (time_conv_t)CH_CFG_ST_FREQUENCY - (time_conv_t)1) /
          (time_conv_t)CH_CFG_ST_FREQUENCY;

  chDbgAssert(msecs < (time_conv_t)((time_msecs_t)-1),
              "conversion overflow");

  return (time_msecs_t)msecs;
}

/**
 * @brief   Time interval to microseconds.
 * @details Converts from system interval to microseconds.
 * @note    The result is rounded up to the next microsecond boundary.
 *
 * @param[in] interval  interval in ticks
 * @return              The number of microseconds.
 *
 * @special
 */
static inline time_usecs_t chTimeI2US(sysinterval_t interval) {
  time_conv_t usecs;

  usecs = (((time_conv_t)interval * (time_conv_t)1000000) +
           (time_conv_t)CH_CFG_ST_FREQUENCY - (time_conv_t)1) /
          (time_conv_t)CH_CFG_ST_FREQUENCY;

  chDbgAssert(usecs <= (time_conv_t)((time_usecs_t)-1),
              "conversion overflow");

  return (time_usecs_t)usecs;
}

/**
 * @brief   Adds an interval to a system time returning a system time.
 *
 * @param[in] systime   base system time
 * @param[in] interval  interval to be added
 * @return              The new system time.
 *
 * @xclass
 */
static inline systime_t chTimeAddX(systime_t systime,
                                   sysinterval_t interval) {

#if CH_CFG_ST_RESOLUTION != CH_CFG_INTERVALS_SIZE
  chDbgCheck(interval <= (sysinterval_t)TIME_MAX_SYSTIME);
#endif

  return systime + (systime_t)interval;
}

/**
 * @brief   Subtracts two system times returning an interval.
 *
 * @param[in] start     first system time
 * @param[in] end       second system time
 * @return              The interval representing the time difference.
 *
 * @xclass
 */
static inline sysinterval_t chTimeDiffX(systime_t start, systime_t end) {

  /*lint -save -e9033 [10.8] This cast is required by the operation, it is
    known that the destination type can be wider.*/
  return (sysinterval_t)((systime_t)(end - start));
  /*lint -restore*/
}

/**
 * @brief   Checks if the specified time is within the specified time range.
 * @note    When start==end then the function returns always true because the
 *          whole time range is specified.
 *
 * @param[in] time      the time to be verified
 * @param[in] start     the start of the time window (inclusive)
 * @param[in] end       the end of the time window (non inclusive)
 * @retval true         current time within the specified time window.
 * @retval false        current time not within the specified time window.
 *
 * @xclass
 */
static inline bool chTimeIsInRangeX(systime_t time,
                                    systime_t start,
                                    systime_t end) {

  return (bool)((systime_t)((systime_t)time - (systime_t)start) <
                (systime_t)((systime_t)end - (systime_t)start));
}

/** @} */

#endif /* CHTIME_H */

/** @} */

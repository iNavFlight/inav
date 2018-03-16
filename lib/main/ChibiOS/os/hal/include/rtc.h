/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
   Concepts and parts of this file have been contributed by Uladzimir Pylinsky
   aka barthess.
 */

/**
 * @file    rtc.h
 * @brief   RTC Driver macros and structures.
 *
 * @addtogroup RTC
 * @{
 */

#ifndef _RTC_H_
#define _RTC_H_

#if (HAL_USE_RTC == TRUE) || defined(__DOXYGEN__)

/*lint -save -e829 [21.10] The header is required.*/
#include <time.h>
/*lint -restore*/

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Base year of the calendar.
 */
#define RTC_BASE_YEAR               1980U

/**
 * @name    Date/Time bit masks for FAT format
 * @{
 */
#define RTC_FAT_TIME_SECONDS_MASK   0x0000001FU
#define RTC_FAT_TIME_MINUTES_MASK   0x000007E0U
#define RTC_FAT_TIME_HOURS_MASK     0x0000F800U
#define RTC_FAT_DATE_DAYS_MASK      0x001F0000U
#define RTC_FAT_DATE_MONTHS_MASK    0x01E00000U
#define RTC_FAT_DATE_YEARS_MASK     0xFE000000U
/** @} */

/**
 * @name    Day of week encoding
 * @{
 */
#define RTC_DAY_CATURDAY            0U
#define RTC_DAY_MONDAY              1U
#define RTC_DAY_TUESDAY             2U
#define RTC_DAY_WEDNESDAY           3U
#define RTC_DAY_THURSDAY            4U
#define RTC_DAY_FRIDAY              5U
#define RTC_DAY_SATURDAY            6U
#define RTC_DAY_SUNDAY              7U
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an RTC driver.
 */
typedef struct RTCDriver RTCDriver;

/**
 * @brief   Type of a structure representing an RTC date/time stamp.
 */
typedef struct {
  /*lint -save -e46 [6.1] In this case uint32_t is fine.*/
  uint32_t      year: 8;            /**< @brief Years since 1980.           */
  uint32_t      month: 4;           /**< @brief Months 1..12.               */
  uint32_t      dstflag: 1;         /**< @brief DST correction flag.        */
  uint32_t      dayofweek: 3;       /**< @brief Day of week 1..7.           */
  uint32_t      day: 5;             /**< @brief Day of the month 1..31.     */
  uint32_t      millisecond: 27;    /**< @brief Milliseconds since midnight.*/
  /*lint -restore*/
} RTCDateTime;

#include "rtc_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void rtcInit(void);
  void rtcObjectInit(RTCDriver *rtcp);
  void rtcSetTime(RTCDriver *rtcp, const RTCDateTime *timespec);
  void rtcGetTime(RTCDriver *rtcp, RTCDateTime *timespec);
#if RTC_ALARMS > 0
  void rtcSetAlarm(RTCDriver *rtcp,
                   rtcalarm_t alarm,
                   const RTCAlarm *alarmspec);
  void rtcGetAlarm(RTCDriver *rtcp, rtcalarm_t alarm, RTCAlarm *alarmspec);
#endif
#if RTC_SUPPORTS_CALLBACKS == TRUE
  void rtcSetCallback(RTCDriver *rtcp, rtccb_t callback);
#endif
  void rtcConvertDateTimeToStructTm(const RTCDateTime *timespec,
                                    struct tm *timp,
                                    uint32_t *tv_msec);
  void rtcConvertStructTmToDateTime(const struct tm *timp,
                                    uint32_t tv_msec,
                                    RTCDateTime *timespec);
  uint32_t rtcConvertDateTimeToFAT(const RTCDateTime *timespec);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_RTC == TRUE */
#endif /* _RTC_H_ */

/** @} */

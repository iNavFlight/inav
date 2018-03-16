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
 * @file    STM32/RTCv2/rtc_lld.h
 * @brief   STM32L1xx/STM32F2xx/STM32F4xx RTC low level driver header.
 *
 * @addtogroup RTC
 * @{
 */

#ifndef _RTC_LLD_H_
#define _RTC_LLD_H_

#if HAL_USE_RTC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Implementation capabilities
 */
/**
 * @brief   Callback support int the driver.
 */
#define RTC_SUPPORTS_CALLBACKS      STM32_RTC_HAS_INTERRUPTS

/**
 * @brief   Number of alarms available.
 */
#define RTC_ALARMS                  STM32_RTC_NUM_ALARMS

/**
 * @brief   Presence of a local persistent storage.
 */
#define RTC_HAS_STORAGE             FALSE
/** @} */

/**
 * @brief   RTC PRER register initializer.
 */
#define RTC_PRER(a, s)              ((((a) - 1) << 16) | ((s) - 1))

/**
 * @name    Alarm helper macros
 * @{
 */
#define RTC_ALRM_MSK4               (1U << 31)
#define RTC_ALRM_WDSEL              (1U << 30)
#define RTC_ALRM_DT(n)              ((n) << 28)
#define RTC_ALRM_DU(n)              ((n) << 24)
#define RTC_ALRM_MSK3               (1U << 23)
#define RTC_ALRM_HT(n)              ((n) << 20)
#define RTC_ALRM_HU(n)              ((n) << 16)
#define RTC_ALRM_MSK2               (1U << 15)
#define RTC_ALRM_MNT(n)             ((n) << 12)
#define RTC_ALRM_MNU(n)             ((n) << 8)
#define RTC_ALRM_MSK1               (1U << 7)
#define RTC_ALRM_ST(n)              ((n) << 4)
#define RTC_ALRM_SU(n)              ((n) << 0)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   RTC PRES register initialization.
 * @note    The default is calculated for a 32768Hz clock.
 */
#if !defined(STM32_RTC_PRESA_VALUE) || defined(__DOXYGEN__)
#define STM32_RTC_PRESA_VALUE               32
#endif

/**
 * @brief   RTC PRESS divider initialization.
 * @note    The default is calculated for a 32768Hz clock.
 */
#if !defined(STM32_RTC_PRESS_VALUE) || defined(__DOXYGEN__)
#define STM32_RTC_PRESS_VALUE               1024
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if HAL_USE_RTC && !STM32_HAS_RTC
#error "RTC not present in the selected device"
#endif

#if !(STM32_RTCSEL == STM32_RTCSEL_LSE) &&                                  \
    !(STM32_RTCSEL == STM32_RTCSEL_LSI) &&                                  \
    !(STM32_RTCSEL == STM32_RTCSEL_HSEDIV)
#error "invalid source selected for RTC clock"
#endif

#if STM32_PCLK1 < (STM32_RTCCLK * 7)
#error "STM32_PCLK1 frequency is too low"
#endif

/**
 * @brief   Initialization for the RTC_PRER register.
 */
#define STM32_RTC_PRER_BITS                 RTC_PRER(STM32_RTC_PRESA_VALUE, \
                                                     STM32_RTC_PRESS_VALUE)

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   FileStream specific methods.
 */
#define _rtc_driver_methods                                                 \
  _file_stream_methods

/**
 * @brief   Type of an RTC alarm number.
 */
typedef uint32_t rtcalarm_t;

/**
 * @brief   Type of a structure representing an RTC alarm time stamp.
 */
typedef struct {
  /**
   * @brief   Type of an alarm as encoded in RTC ALRMxR registers.
   */
  uint32_t                  alrmr;
} RTCAlarm;

#if STM32_RTC_HAS_PERIODIC_WAKEUPS
/**
 * @brief   Type of a wakeup as encoded in RTC WUTR register.
 */
typedef struct {
  /**
   * @brief   Wakeup as encoded in RTC WUTR register.
   * @note    ((WUTR == 0) || (WUCKSEL == 3)) are a forbidden combination.
   */
  uint32_t                  wutr;
} RTCWakeup;
#endif

#if RTC_HAS_STORAGE || defined(__DOXYGEN__)
/**
 * @extends FileStream
 *
 * @brief   @p RTCDriver virtual methods table.
 */
struct RTCDriverVMT {
  _rtc_driver_methods
};
#endif

/**
 * @brief   Structure representing an RTC driver.
 */
struct RTCDriver {
#if RTC_HAS_STORAGE || defined(__DOXYGEN__)
  /**
   * @brief Virtual Methods Table.
   */
  const struct RTCDriverVMT *vmt;
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Pointer to the RTC registers block.
   */
  RTC_TypeDef               *rtc;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern RTCDriver RTCD1;
#if RTC_HAS_STORAGE
extern struct RTCDriverVMT _rtc_lld_vmt;
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void rtc_lld_init(void);
  void rtc_lld_set_time(RTCDriver *rtcp, const RTCDateTime *timespec);
  void rtc_lld_get_time(RTCDriver *rtcp, RTCDateTime *timespec);
#if RTC_ALARMS > 0
  void rtc_lld_set_alarm(RTCDriver *rtcp,
                         rtcalarm_t alarm,
                         const RTCAlarm *alarmspec);
  void rtc_lld_get_alarm(RTCDriver *rtcp,
                         rtcalarm_t alarm,
                         RTCAlarm *alarmspec);
#endif
#if STM32_RTC_HAS_PERIODIC_WAKEUPS
  void rtcSTM32SetPeriodicWakeup(RTCDriver *rtcp, const RTCWakeup *wakeupspec);
  void rtcSTM32GetPeriodicWakeup(RTCDriver *rtcp, RTCWakeup *wakeupspec);
#endif /* STM32_RTC_HAS_PERIODIC_WAKEUPS */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_RTC */

#endif /* _RTC_LLD_H_ */

/** @} */

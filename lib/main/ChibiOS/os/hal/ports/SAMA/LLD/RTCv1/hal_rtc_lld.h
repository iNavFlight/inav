/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    RTCv1/hal_rtc_lld.h
 * @brief   SAMA RTC low level driver header.
 *
 * @addtogroup RTC
 * @{
 */

#ifndef HAL_RTC_LLD_H
#define HAL_RTC_LLD_H

#if HAL_USE_RTC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/
#define SYSC_WPMR                           (0xF80480E4u)
#define SYSC_WPMR_WPKEY_PASSWD              (0x535943u << 8)
#define SYSC_WPMR_WPEN                      (0x1u << 0)

/**
 * @name    Implementation capabilities
 */
/**
 * @brief   Number of available alarms.
 */
#define RTC_ALARMS                  1

/**
 * @brief   Presence of a local persistent storage.
 */
#define RTC_HAS_STORAGE             FALSE

/**
 * @brief   Callback supported.
 */
#define RTC_SUPPORTS_CALLBACKS      TRUE
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/*
 * RTC driver system settings.
 */
#define SAMA_RTC_IRQ_PRIORITY       7
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
  * @brief   Type of an RTC event.
  */
typedef enum {
  RTC_EVENT_SECOND = 0,                 /** Triggered every second.         */
  RTC_EVENT_ALARM = 1                   /** Triggered on alarm.             */
} rtcevent_t;

/**
  * @brief   Type of a generic RTC callback.
  */
typedef void (*rtccb_t)(RTCDriver *rtcp, rtcevent_t event);

/**
 * @brief   Type of a structure representing an RTC alarm time stamp.
 */
typedef struct hal_rtc_alarm {
  /**
   * @brief   Type of an alarm as encoded in RTC registers.
   */
  uint32_t                  timralrm;
  uint32_t                  calralrm;
} RTCAlarm;

/**
 * @brief   Implementation-specific @p RTCDriver fields.
 */
#define rtc_lld_driver_fields                                               \
  /* Pointer to the RTC registers block.*/                                  \
  Rtc               *rtc;                                                   \
  /* Callback pointer.*/                                                    \
  rtccb_t           callback

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/
/**
 * @brief   Enable write protection on SYSC registers block.
 *
 * @notapi
 */
#define syscEnableWP() {                                                      \
    *(uint32_t *)(SYSC_WPMR) = SYSC_WPMR_WPKEY_PASSWD | SYSC_WPMR_WPEN;       \
}

/**
 * @brief   Disable write protection on SYSC registers block.
 *
 * @notapi
 */
#define syscDisableWP() {                                                     \
    *(uint32_t *)(SYSC_WPMR) = SYSC_WPMR_WPKEY_PASSWD;                        \
}

/**
 * @brief   Configure RTC_MR register.
 *
 * @param[in] rtcp        pointer to RTC driver structure
 * @param[in] value       value to be written in the MR register
 *
 */
#define rtcConfigureMode(rtcp, value) {                                      \
  (rtcp)->rtc->RTC_MR = value;                                               \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void rtc_lld_init(void);
  void rtc_lld_set_time(RTCDriver *rtcp, const RTCDateTime *timespec);
  void rtc_lld_get_time(RTCDriver *rtcp, RTCDateTime *timespec);
  void rtc_lld_set_alarm(RTCDriver *rtcp,
                         rtcalarm_t alarm,
                         const RTCAlarm *alarmspec);
  void rtc_lld_get_alarm(RTCDriver *rtcp,
                         rtcalarm_t alarm,
                         RTCAlarm *alarmspec);
  void rtc_lld_set_callback(RTCDriver *rtcp, rtccb_t callback);
  /* Driver specific */
  void rtcGetTamperTime(RTCDriver *rtcp, uint8_t reg, RTCDateTime *timespec);
  uint32_t rtcGetTamperSource(RTCDriver *rtcp, uint8_t reg);
  uint32_t rtcGetTamperEventCounter(RTCDriver *rtcp);
  uint8_t rtcGetTamperMode(RTCDriver *rtcp, uint8_t reg);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_RTC */

#endif /* HAL_RTC_LLD_H */

/** @} */

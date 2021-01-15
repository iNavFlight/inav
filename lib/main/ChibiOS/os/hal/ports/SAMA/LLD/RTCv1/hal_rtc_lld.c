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
 * @file    RTCv1/hal_rtc_lld.c
 * @brief   SAMA RTC low level driver.
 *
 * @addtogroup RTC
 * @{
 */

#include "hal.h"

#if HAL_USE_RTC || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/
#define RTC_TIMR_PM_OFFSET                  22
#define RTC_TIMR_HT_OFFSET                  20
#define RTC_TIMR_HU_OFFSET                  16
#define RTC_TIMR_MNT_OFFSET                 12
#define RTC_TIMR_MNU_OFFSET                 8
#define RTC_TIMR_ST_OFFSET                  4
#define RTC_TIMR_SU_OFFSET                  0

#define RTC_CALR_DT_OFFSET                  28
#define RTC_CALR_DU_OFFSET                  24
#define RTC_CALR_WDU_OFFSET                 21
#define RTC_CALR_MT_OFFSET                  20
#define RTC_CALR_MU_OFFSET                  16
#define RTC_CALR_YT_OFFSET                  12
#define RTC_CALR_YU_OFFSET                  8
#define RTC_CALR_CT_OFFSET                  4
#define RTC_CALR_CU_OFFSET                  0

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief RTC driver identifier.
 */
RTCDriver RTCD0;

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   RTC interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_RTC_HANDLER) {
  uint16_t sr, imr;

  OSAL_IRQ_PROLOGUE();

  sr = RTCD0.rtc->RTC_SR;
  imr = RTCD0.rtc->RTC_IMR;

  if ((sr & RTC_SR_SEC) && (imr & RTC_IMR_SEC)) {
    RTCD0.rtc->RTC_SCCR = RTC_SCCR_SECCLR;
    RTCD0.callback(&RTCD0, RTC_EVENT_SECOND);
  }
  if ((sr & RTC_SR_ALARM) && (imr & RTC_IMR_ALR)) {
    RTCD0.rtc->RTC_SCCR = RTC_SCCR_ALRCLR;
    RTCD0.callback(&RTCD0, RTC_EVENT_ALARM);
  }
  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}
/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Beginning of configuration procedure.
 *
 * @notapi
 */
static void rtc_enter_init(void) {

  /* Stop RTC_TIMR and RTC_CALR */
  RTCD0.rtc->RTC_CR |= RTC_CR_UPDCAL;
  RTCD0.rtc->RTC_CR |= RTC_CR_UPDTIM;
}

/**
 * @brief   Finalizing of configuration procedure.
 *
 * @notapi
 */
static void rtc_exit_init(void) {

  RTCD0.rtc->RTC_CR &= ~RTC_CR_UPDTIM;
  RTCD0.rtc->RTC_CR &= ~RTC_CR_UPDCAL;
}

/**
 * @brief   Converts time from RTC_TIMR register encoding to timespec.
 *
 * @param[in] timr        TIMR register value
 * @param[out] timespec pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
static void rtc_decode_time(uint32_t timr, RTCDateTime *timespec) {
  uint32_t n;

  n  = ((timr >> RTC_TIMR_HT_OFFSET) & 3)   * 36000000;
  n += ((timr >> RTC_TIMR_HU_OFFSET) & 15)  * 3600000;
  n += ((timr >> RTC_TIMR_MNT_OFFSET) & 7)  * 600000;
  n += ((timr >> RTC_TIMR_MNU_OFFSET) & 15) * 60000;
  n += ((timr >> RTC_TIMR_ST_OFFSET) & 7)   * 10000;
  n += ((timr >> RTC_TIMR_SU_OFFSET) & 15)  * 1000;
  timespec->millisecond = n;
}

/**
 * @brief   Converts date from RTC_CALR register encoding to timespec.
 *
 * @param[in] calr        RTC_CALR register value
 * @param[out] timespec pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
static void rtc_decode_date(uint32_t calr, RTCDateTime *timespec) {

  uint32_t centuryYear = (((calr >> RTC_CALR_CT_OFFSET) & 7) * 1000) +
                         (((calr >> RTC_CALR_CU_OFFSET) & 15) * 100) +
                         (((calr >> RTC_CALR_YT_OFFSET) & 15) * 10) +
                          ((calr >> RTC_CALR_YU_OFFSET) & 15);
  timespec->year  = (centuryYear - 1980);
  timespec->month = (((calr >> RTC_CALR_MT_OFFSET) & 1) * 10) +
                     ((calr >> RTC_CALR_MU_OFFSET) & 15);
  timespec->day   = (((calr >> RTC_CALR_DT_OFFSET) & 3) * 10) +
                     ((calr >> RTC_CALR_DU_OFFSET) & 15);
  timespec->dayofweek = (calr >> RTC_CALR_WDU_OFFSET) & 7;
}

/**
 * @brief   Converts time from timespec to RTC_TIMR register encoding.
 *
 * @param[in] timespec  pointer to a @p RTCDateTime structure
 * @return              the TIMR register encoding.
 *
 * @notapi
 */
static uint32_t rtc_encode_time(const RTCDateTime *timespec) {
  uint32_t n, timr = 0;

  /* Subseconds cannot be set.*/
  n = timespec->millisecond / 1000;

  /* Seconds conversion.*/
  timr = timr | ((n % 10) << RTC_TIMR_SU_OFFSET);
  n /= 10;
  timr = timr | ((n % 6) << RTC_TIMR_ST_OFFSET);
  n /= 6;

  /* Minutes conversion.*/
  timr = timr | ((n % 10) << RTC_TIMR_MNU_OFFSET);
  n /= 10;
  timr = timr | ((n % 6) << RTC_TIMR_MNT_OFFSET);
  n /= 6;

  /* Hours conversion.*/
  timr = timr | ((n % 10) << RTC_TIMR_HU_OFFSET);
  n /= 10;
  timr = timr | (n << RTC_TIMR_HT_OFFSET);

  return timr;
}

/**
 * @brief   Converts a date from timespec to RTC_CALR register encoding.
 *
 * @param[in] timespec  pointer to a @p RTCDateTime structure
 * @return              the CALR register encoding.
 *
 * @notapi
 */
static uint32_t rtc_encode_date(const RTCDateTime *timespec) {
  uint32_t n, calr = 0;

  /* Year conversion. */
  n = timespec->year + 1980;
  calr = calr | ((n % 10) << RTC_CALR_YU_OFFSET);
  n /= 10;
  calr = calr | ((n % 10) << RTC_CALR_YT_OFFSET);
  n /= 10;
  calr = calr | ((n % 10) << RTC_CALR_CU_OFFSET);
  n /= 10;
  calr = calr | ((n % 10) << RTC_CALR_CT_OFFSET);

  /* Months conversion.*/
  n = timespec->month;
  calr = calr | ((n % 10) << RTC_CALR_MU_OFFSET);
  n /= 10;
  calr = calr | ((n % 10) << RTC_CALR_MT_OFFSET);

  /* Days conversion.*/
  n = timespec->day;
  calr = calr | ((n % 10) << RTC_CALR_DU_OFFSET);
  n /= 10;
  calr = calr | ((n % 10) << RTC_CALR_DT_OFFSET);

  /* Days of week conversion.*/
  calr = calr | (timespec->dayofweek << RTC_CALR_WDU_OFFSET);

  return calr;
}

#if RTC_HAS_STORAGE
/* TODO: Map on the backup SRAM on devices that have it.*/
static size_t _write(void *instance, const uint8_t *bp, size_t n) {

  (void)instance;
  (void)bp;
  (void)n;

  return 0;
}

static size_t _read(void *instance, uint8_t *bp, size_t n) {

  (void)instance;
  (void)bp;
  (void)n;

  return 0;
}

static msg_t _put(void *instance, uint8_t b) {

  (void)instance;
  (void)b;

  return FILE_OK;
}

static msg_t _get(void *instance) {

  (void)instance;

  return FILE_OK;
}

static msg_t _close(void *instance) {

  /* Close is not supported.*/
  (void)instance;

  return FILE_OK;
}

static msg_t _geterror(void *instance) {

  (void)instance;

  return (msg_t)0;
}

static msg_t _getsize(void *instance) {

  (void)instance;

  return 0;
}

static msg_t _getposition(void *instance) {

  (void)instance;

  return 0;
}

static msg_t _lseek(void *instance, fileoffset_t offset) {

  (void)instance;
  (void)offset;

  return FILE_OK;
}

/**
 * @brief   VMT for the RTC storage file interface.
 */
struct RTCDriverVMT _rtc_lld_vmt = {
  _write, _read, _put, _get,
  _close, _geterror, _getsize, _getposition, _lseek
};
#endif /* RTC_HAS_STORAGE */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Enable access to registers.
 *
 * @notapi
 */
void rtc_lld_init(void) {

#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_SYSC, SECURE_PER);
#endif

  /* RTC object initialization.*/
  rtcObjectInit(&RTCD0);

  /* RTC pointer initialization.*/
  RTCD0.rtc = RTC;

  /* Disable write protection */
//  syscDisableWP();

  RTCD0.rtc->RTC_IDR = RTC_IDR_ALRDIS | RTC_IDR_SECDIS;

  /* Clear all status flag.*/
  RTCD0.rtc->RTC_SCCR = 0x3F;

  /* Disable match alarms */
  RTCD0.rtc->RTC_TIMALR &= ~(RTC_TIMALR_SECEN | RTC_TIMALR_MINEN | RTC_TIMALR_HOUREN);
  RTCD0.rtc->RTC_CALALR &= ~(RTC_CALALR_MTHEN | RTC_CALALR_DATEEN);

  /* Callback initially disabled.*/
  RTCD0.callback = NULL;

  /* Enable write protection */
//  syscEnableWP();

  /* IRQ vector permanently assigned to this driver.*/
  aicSetSourcePriority(ID_SYSC, SAMA_RTC_IRQ_PRIORITY);
  aicSetSourceHandler(ID_SYSC, SAMA_RTC_HANDLER);
  aicEnableInt(ID_SYSC);
}

/**
 * @brief   Set current time.
 * @note    Fractional part will be silently ignored.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] timespec  pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
void rtc_lld_set_time(RTCDriver *rtcp, const RTCDateTime *timespec) {
  uint32_t calr, timr, ver;
  syssts_t sts;

  timr = rtc_encode_time(timespec);
  calr = rtc_encode_date(timespec);
  ver = rtcp->rtc->RTC_VER;

  /* Disable write protection */
//  syscDisableWP();

  /* Synchronization on a second periodic event polling the RTC_SR.SEC status bit */
  wait: while ((rtcp->rtc->RTC_SR & RTC_SR_SEC) == 0);

  /* Entering a reentrant critical zone.*/
  sts = osalSysGetStatusAndLockX();

  if (!(rtcp->rtc->RTC_SR & RTC_SR_SEC)) {
    /* Leaving a reentrant critical zone.*/
    osalSysRestoreStatusX(sts);
    goto wait;
  }

  /* Writing the registers.*/
  rtc_enter_init();

  /* Leaving a reentrant critical zone.*/
  osalSysRestoreStatusX(sts);

  while ((RTCD0.rtc->RTC_SR & RTC_SR_ACKUPD) == 0);

  /* Entering a reentrant critical zone.*/
  sts = osalSysGetStatusAndLockX();

  if (!(rtcp->rtc->RTC_SR & RTC_SR_SEC)) {
    /* Leaving a reentrant critical zone.*/
    osalSysRestoreStatusX(sts);
    goto wait;
  }

  /* Clear ACKUPD status flag */
  rtcp->rtc->RTC_SCCR = RTC_SCCR_ACKCLR;

  /* Date and Time updating */
  rtcp->rtc->RTC_TIMR = timr;
  rtcp->rtc->RTC_CALR = calr;

  rtc_exit_init();

  /* Enable write protection */
//  syscEnableWP();

  /* Check time and data fields */
  osalDbgAssert(((ver &  RTC_VER_NVCAL) == 0) || ((ver &  RTC_VER_NVTIM) == 0),
                "invalid date-time");

  /* Leaving a reentrant critical zone.*/
  osalSysRestoreStatusX(sts);
}

/**
 * @brief   Get current time.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[out] timespec pointer to a @p RTCDateTime structure
 *
 * @notapi
 */
void rtc_lld_get_time(RTCDriver *rtcp, RTCDateTime *timespec) {
  uint32_t calr, timr;
  uint32_t subs = 0;
  syssts_t sts;

  /* Entering a reentrant critical zone.*/
  sts = osalSysGetStatusAndLockX();

  do {
    timr  = rtcp->rtc->RTC_TIMR;
  } while (timr != rtcp->rtc->RTC_TIMR);

  do {
    calr  = rtcp->rtc->RTC_CALR;
  } while (calr != rtcp->rtc->RTC_CALR);

  /* Leaving a reentrant critical zone.*/
  osalSysRestoreStatusX(sts);

  rtc_decode_time(timr, timespec);
  timespec->millisecond += subs;

  /* Decoding date, this concludes the atomic read sequence.*/
  rtc_decode_date(calr, timespec);

  /* Retrieving the DST bit.*/
  timespec->dstflag = 0;
}

/**
 * @brief   Get tamper time.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] reg       number of register to return
 * @param[out] timespec pointer to a @p RTCDateTime structure
 *
 * @note RTC_TSSR0 and RTC_TSDR cannot be overwritten, so once it has been written
 *       all data are stored until the registers are reset: these register are
 *       storing the first tamper occurrence after a read.
 *       RTC_TSSR0 and RTC_TSDR are overwritten each time a tamper event is detected.
 *
 */
void rtcGetTamperTime(RTCDriver *rtcp, uint8_t reg, RTCDateTime *timespec) {
  uint32_t calr, timr;
  uint32_t subs = 0;

  timr  = rtcp->rtc->RTC_TS[reg].RTC_TSTR;
  calr  = rtcp->rtc->RTC_TS[reg].RTC_TSDR;

  rtc_decode_time(timr, timespec);
  timespec->millisecond += subs;

  /* Decoding date, this concludes the atomic read sequence.*/
  rtc_decode_date(calr, timespec);

  /* Retrieving the DST bit.*/
  timespec->dstflag = 0;
}

/**
 * @brief   Returns source of tamper register.
 *
 * @param[in] rtcp        pointer to RTC driver structure
 * @param[in] reg         number of register source to return
 *
 * return content of RTC_SSRx register
 *
 * @note RTC_TSSR0  cannot be overwritten, so once it has been written
 *       all data are stored until the registers are reset: that register is
 *       storing the first tamper occurrence after a read.
 *       RTC_TSSR1 is overwritten each time a tamper event is detected.
 *
 */
uint32_t rtcGetTamperSource(RTCDriver *rtcp, uint8_t reg) {
  return ((rtcp)->rtc->RTC_TS[reg].RTC_TSSR);
}

/**
 * @brief   Returns numbers of total tamper events.
 *
 * @param[in] rtcp        pointer to RTC driver structure
 *
 * return numbers of total tamper events.
 */
uint32_t rtcGetTamperEventCounter(RTCDriver *rtcp) {
  return ((rtcp)->rtc->RTC_TS[0].RTC_TSTR & RTC_TSTR_TEVCNT_Msk) >> RTC_TSTR_TEVCNT_Pos;
}

/**
 * @brief   Returns backup or normal mode of tamper event.
 *
 * @param[in] rtcp        pointer to RTC driver structure
 * @param[in] reg         number of register source to return
 *
 * @return 0x1 if system is in backup mode when tamper events occurs
 *         0x0 if system is not in backup mode when tamper events occurs
 *
 * @note RTC_TSTR0  cannot be overwritten, so once it has been written
 *       all data are stored until the registers are reset: that register is
 *       storing the first tamper occurrence after a read.
 *       RTC_TSTR1 is overwritten each time a tamper event is detected.
 *
 */
uint8_t rtcGetTamperMode(RTCDriver *rtcp, uint8_t reg) {
    return (rtcp)->rtc->RTC_TS[reg].RTC_TSTR & RTC_TSTR_BACKUP ? 0x1u : 0x0u;
}

/**
 * @brief   Set alarm time.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure.
 * @param[in] alarm     alarm identifier.
 * @param[in] alarmspec pointer to a @p RTCAlarm structure.
 *
 * @notapi
 */
void rtc_lld_set_alarm(RTCDriver *rtcp,
                       rtcalarm_t alarm,
                       const RTCAlarm *alarmspec) {

  /* SAMA has only one alarm, this field is ignored */
  (void)alarm;
  syssts_t sts;

  /* Disable write protection */
//  syscDisableWP();

  /* Entering a reentrant critical zone.*/
  sts = osalSysGetStatusAndLockX();

  if (alarmspec != NULL) {
    if (alarmspec->timralrm != 0){
      rtcp->rtc->RTC_TIMALR = alarmspec->timralrm;
      /* Check time alarm fields */
      osalDbgAssert(((rtcp->rtc->RTC_VER & RTC_VER_NVTIMALR) == 0),
                    "invalid time-alarm");
    }
    if (alarmspec->calralrm != 0){
      rtcp->rtc->RTC_CALALR = alarmspec->calralrm;
      /* Check calendar alarm fields */
      osalDbgAssert(((rtcp->rtc->RTC_VER & RTC_VER_NVCALALR) == 0),
                    "invalid date-alarm");
    }
  }
  else {
    rtcp->rtc->RTC_TIMALR = 0;
    rtcp->rtc->RTC_CALALR = 0;
  }

  /* Leaving a reentrant critical zone.*/
  osalSysRestoreStatusX(sts);

  /* Enable write protection */
//  syscEnableWP();
}

/**
 * @brief   Get alarm time.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp       pointer to RTC driver structure
 * @param[in] alarm      alarm identifier
 * @param[out] alarmspec pointer to a @p RTCAlarm structure
 *
 * @notapi
 */
void rtc_lld_get_alarm(RTCDriver *rtcp,
                       rtcalarm_t alarm,
                       RTCAlarm *alarmspec) {

  (void)alarm;
  alarmspec->timralrm = rtcp->rtc->RTC_TIMALR;
  alarmspec->calralrm = rtcp->rtc->RTC_CALALR;
}

/**
 * @brief   Enables or disables RTC callbacks.
 * @details This function enables or disables callbacks, use a @p NULL pointer
 *          in order to disable a callback.
 * @note    The function can be called from any context.
 *
 * @param[in] rtcp      pointer to RTC driver structure
 * @param[in] callback  callback function pointer or @p NULL
 *
 * @notapi
 */
void rtc_lld_set_callback(RTCDriver *rtcp, rtccb_t callback) {
  syssts_t sts;

  /* Entering a reentrant critical zone.*/
  sts = osalSysGetStatusAndLockX();

  if (callback != NULL) {

    /* IRQ sources enabled only after setting up the callback.*/
    rtcp->callback = callback;

    rtcp->rtc->RTC_SCCR = RTC_SCCR_ALRCLR | RTC_SCCR_SECCLR | RTC_SCCR_TIMCLR |
                          RTC_SCCR_CALCLR | RTC_SCCR_TDERRCLR;
    rtcp->rtc->RTC_IER = RTC_IER_ALREN | RTC_IER_SECEN;
  }
  else {
    rtcp->rtc->RTC_IDR = RTC_IDR_ALRDIS | RTC_IDR_SECDIS;

    /* Callback set to NULL only after disabling the IRQ sources.*/
    rtcp->callback = NULL;
  }

  /* Leaving a reentrant critical zone.*/
  osalSysRestoreStatusX(sts);
}

#endif /* HAL_USE_RTC */

/** @} */

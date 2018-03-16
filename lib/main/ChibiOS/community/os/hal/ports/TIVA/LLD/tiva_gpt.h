/*
    Copyright (C) 2014..2016 Marco Veeneman

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

/**
 * @file    tiva_gpt.h
 * @brief   TIVA GPT registers layout header.
 *
 * @addtogroup TIVA_GPT
 * @{
 */

#ifndef TIVA_GPT_H_
#define TIVA_GPT_H_

// cfg
#define GPTM_CFG_CFG_MASK       (7 << 0)
#define GPTM_CFG_CFG_WHOLE      (0 << 0)
#define GPTM_CFG_CFG_RTC        (1 << 0)
#define GPTM_CFG_CFG_SPLIT      (4 << 0)

// tamr
#define GPTM_TAMR_TAMR_MASK     (3 << 0)
#define GPTM_TAMR_TAMR_ONESHOT  (1 << 0)
#define GPTM_TAMR_TAMR_PERIODIC (2 << 0)
#define GPTM_TAMR_TAMR_CAPTURE  (3 << 0)

#define GPTM_TAMR_TACMR         (1 << 2)

#define GPTM_TAMR_TAAMS         (1 << 3)

#define GPTM_TAMR_TACDIR        (1 << 4)

#define GPTM_TAMR_TAMIE         (1 << 5)

#define GPTM_TAMR_TAWOT         (1 << 6)

#define GPTM_TAMR_TASNAPS       (1 << 7)

#define GPTM_TAMR_TAILD         (1 << 8)

#define GPTM_TAMR_TAPWMIE       (1 << 9)

#define GPTM_TAMR_TAMRSU        (1 << 10)

#define GPTM_TAMR_TAPLO         (1 << 11)

// ctl
#define GPTM_CTL_TAEN           (1 << 0)

#define GPTM_CTL_TASTALL        (1 << 1)

#define GPTM_CTL_TAEVENT_MASK   (3 << 2)
#define GPTM_CTL_TAEVENT_POS    (0 << 2)
#define GPTM_CTL_TAEVENT_NEG    (1 << 2)
#define GPTM_CTL_TAEVENT_BOTH   (3 << 2)

#define GPTM_CTL_RTCEN          (1 << 4)

#define GPTM_CTL_TAOTE          (1 << 5)

#define GPTM_CTL_TAPWML         (1 << 6)

#define GPTM_CTL_TBEN           (1 << 8)

#define GPTM_CTL_TBSTALL        (1 << 9)

#define GPTM_CTL_TBEVENT_MASK   (3 << 10)
#define GPTM_CTL_TBEVENT_POS    (0 << 10)
#define GPTM_CTL_TBEVENT_NEG    (1 << 10)
#define GPTM_CTL_TBEVENT_BOTH   (3 << 10)

#define GPTM_CTL_TBOTE          (1 << 13)

#define GPTM_CTL_TBPWML         (1 << 14)

// imr
#define GPTM_IMR_TATOIM         (1 << 0)

#define GPTM_IMR_CAMIM          (1 << 1)

#define GPTM_IMR_CAEIM          (1 << 2)

#define GPTM_IMR_RTCIM          (1 << 3)

#define GPTM_IMR_TAMIM          (1 << 4)

#define GPTM_IMR_TBTOIM         (1 << 8)

#define GPTM_IMR_CBMIM          (1 << 9)

#define GPTM_IMR_CBEIM          (1 << 10)

#define GPTM_IMR_TBMIM          (1 << 11)

#define GPTM_IMR_WUEIM          (1 << 16)

// icr
#define GPTM_ICR_TATOCINT         (1 << 0)

#define GPTM_ICR_CAMCINT          (1 << 1)

#define GPTM_ICR_CAECINT          (1 << 2)

#define GPTM_ICR_RTCCINT          (1 << 3)

#define GPTM_ICR_TAMCINT          (1 << 4)

#define GPTM_ICR_TBTOCINT         (1 << 8)

#define GPTM_ICR_CBMCINT          (1 << 9)

#define GPTM_ICR_CBECINT          (1 << 10)

#define GPTM_ICR_TBMCINT          (1 << 11)

#define GPTM_ICR_WUECINT          (1 << 16)

#endif /* TIVA_GPT_H_ */

/*
 * @}
 */

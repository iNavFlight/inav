/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

#include "ch.h"
#include "hal.h"

#define ADC_NUM_CHANNELS          6
#define ADC_BUF_DEPTH             8

/* human readable names */
#define ADC_CURRENT_SENS          ADC_CHANNEL_IN10
#define ADC_MAIN_SUPPLY           ADC_CHANNEL_IN11
#define ADC_6V_SUPPLY             ADC_CHANNEL_IN12
#define ADC_AN33_0                ADC_CHANNEL_IN13
#define ADC_AN33_1                ADC_CHANNEL_IN14
#define ADC_AN33_2                ADC_CHANNEL_IN15

#define ADC_CURRENT_SENS_OFFSET   (ADC_CHANNEL_IN10 - 10)
#define ADC_MAIN_VOLTAGE_OFFSET   (ADC_CHANNEL_IN11 - 10)
#define ADC_6V_OFFSET             (ADC_CHANNEL_IN12 - 10)
#define ADC_AN33_0_OFFSET         (ADC_CHANNEL_IN13 - 10)
#define ADC_AN33_1_OFFSET         (ADC_CHANNEL_IN14 - 10)
#define ADC_AN33_2_OFFSET         (ADC_CHANNEL_IN15 - 10)

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err);
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n);

static adcsample_t samples[ADC_NUM_CHANNELS * ADC_BUF_DEPTH];

static uint32_t ints = 0;
static uint32_t errors = 0;

static const ADCConversionGroup adccg = {
  TRUE,
  ADC_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  ADC_SMPR1_SMP_AN10(ADC_SAMPLE_3) |
    ADC_SMPR1_SMP_AN11(ADC_SAMPLE_3) |
    ADC_SMPR1_SMP_AN12(ADC_SAMPLE_3) |
    ADC_SMPR1_SMP_AN13(ADC_SAMPLE_3) |
    ADC_SMPR1_SMP_AN14(ADC_SAMPLE_3) |
    ADC_SMPR1_SMP_AN15(ADC_SAMPLE_3),
  0,                        /* SMPR2 */
  0,
  0,
  ADC_SQR1_NUM_CH(ADC_NUM_CHANNELS),
  0,
  ADC_SQR3_SQ6_N(ADC_AN33_2)          |
    ADC_SQR3_SQ5_N(ADC_AN33_1)        |
    ADC_SQR3_SQ4_N(ADC_AN33_0)        |
    ADC_SQR3_SQ3_N(ADC_6V_SUPPLY)     |
    ADC_SQR3_SQ2_N(ADC_MAIN_SUPPLY)   |
    ADC_SQR3_SQ1_N(ADC_CURRENT_SENS)
};

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {
  (void)adcp;
  (void)err;

  osalSysHalt("");
}

static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {
  (void)adcp;
  (void)buffer;
  (void)n;
  ints++;
}

/*
 *
 */
void dma_storm_adc_start(void){
  ints = 0;
  errors = 0;

  /* Activates the ADC1 driver and the temperature sensor.*/
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTSVREFE();

  /* Starts an ADC continuous conversion.*/
  adcStartConversion(&ADCD1, &adccg, samples, ADC_BUF_DEPTH);
}

/*
 *
 */
uint32_t dma_storm_adc_stop(void){
  adcStopConversion(&ADCD1);
  adcSTM32DisableTSVREFE();
  adcStop(&ADCD1);
  return ints;
}


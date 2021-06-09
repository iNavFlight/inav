/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/debug.h"

#include "platform.h"
#include "drivers/time.h"

#include "drivers/io.h"
#include "io_impl.h"
#include "rcc.h"
#include "dma.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"

#include "adc.h"
#include "adc_impl.h"


static adcDevice_t adcHardware[ADCDEV_COUNT] = {
    {
        .ADCx = ADC1,
        .rccADC = RCC_AHB1(ADC12),
        .rccDMA = RCC_AHB1(DMA2), 
        .DMAy_Streamx = DMA2_Stream0,
        .channel = DMA_REQUEST_ADC1,
        .enabled = false, 
        .usedChannelCount = 0
    },
    /* currently not used
    { 
        .ADCx = ADC2,
        .rccADC = RCC_AHB1(ADC12),
        .rccDMA = RCC_AHB1(DMA2), 
        .DMAy_Streamx = DMA2_Stream1,
        .channel = DMA_REQUEST_ADC2,
        .enabled = false, 
        .usedChannelCount = 0
    }
    */
};

adcDevice_t adcDevice[ADCDEV_COUNT];

/* note these could be packed up for saving space */
const adcTagMap_t adcTagMap[] = {
    { DEFIO_TAG_E__PC0, ADC_CHANNEL_10 },
    { DEFIO_TAG_E__PC1, ADC_CHANNEL_11 },
    { DEFIO_TAG_E__PC2, ADC_CHANNEL_0  },
    { DEFIO_TAG_E__PC3, ADC_CHANNEL_1  },
    { DEFIO_TAG_E__PC4, ADC_CHANNEL_4  },
    { DEFIO_TAG_E__PC5, ADC_CHANNEL_8  },
    { DEFIO_TAG_E__PB0, ADC_CHANNEL_9  },
    { DEFIO_TAG_E__PB1, ADC_CHANNEL_5  },
    { DEFIO_TAG_E__PA0, ADC_CHANNEL_16 },
    { DEFIO_TAG_E__PA1, ADC_CHANNEL_17 },
    { DEFIO_TAG_E__PA2, ADC_CHANNEL_14 },
    { DEFIO_TAG_E__PA3, ADC_CHANNEL_15 },
    { DEFIO_TAG_E__PA4, ADC_CHANNEL_18 },
    { DEFIO_TAG_E__PA5, ADC_CHANNEL_19 },
    { DEFIO_TAG_E__PA6, ADC_CHANNEL_3  },
    { DEFIO_TAG_E__PA7, ADC_CHANNEL_7  },
};

// Translate rank number x to ADC_REGULAR_RANK_x (Note that array index is 0-origin)
static const uint32_t adcRegularRankMap[] = {
    ADC_REGULAR_RANK_1,
    ADC_REGULAR_RANK_2,
    ADC_REGULAR_RANK_3,
    ADC_REGULAR_RANK_4,
    ADC_REGULAR_RANK_5,
    ADC_REGULAR_RANK_6,
    ADC_REGULAR_RANK_7,
    ADC_REGULAR_RANK_8,
    ADC_REGULAR_RANK_9,
    ADC_REGULAR_RANK_10,
    ADC_REGULAR_RANK_11,
    ADC_REGULAR_RANK_12,
    ADC_REGULAR_RANK_13,
    ADC_REGULAR_RANK_14,
    ADC_REGULAR_RANK_15,
    ADC_REGULAR_RANK_16,
};

ADCDevice adcDeviceByInstance(ADC_TypeDef *instance)
{
    if (instance == ADC1)
        return ADCDEV_1;

    if (instance == ADC2)
        return ADCDEV_2;

    return ADCINVALID;
}

static void adcInstanceInit(ADCDevice adcDevice)
{
    adcDevice_t * adc = &adcHardware[adcDevice];

    RCC_ClockCmd(adc->rccDMA, ENABLE);
    RCC_ClockCmd(adc->rccADC, ENABLE);

    adc->ADCHandle.Instance = adc->ADCx;

    adc->ADCHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
    adc->ADCHandle.Init.Resolution            = ADC_RESOLUTION_12B;
    adc->ADCHandle.Init.ContinuousConvMode    = ENABLE;
    adc->ADCHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIG_T1_CC1;
    adc->ADCHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc->ADCHandle.Init.NbrOfConversion       = adc->usedChannelCount;
    adc->ADCHandle.Init.ScanConvMode          = adc->usedChannelCount > 1 ? ENABLE : DISABLE; // 1=scan more that one channel in group
    adc->ADCHandle.Init.DiscontinuousConvMode = DISABLE;
    adc->ADCHandle.Init.NbrOfDiscConversion   = 0;
    adc->ADCHandle.Init.EOCSelection          = DISABLE;
    adc->ADCHandle.Init.LowPowerAutoWait      = DISABLE;

    // Enable circular DMA.
    adc->ADCHandle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;

    adc->ADCHandle.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
    adc->ADCHandle.Init.OversamplingMode         = DISABLE;

    if (HAL_ADC_Init(&adc->ADCHandle) != HAL_OK) {
        return;
    }

    if (HAL_ADCEx_Calibration_Start(&adc->ADCHandle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK) {
      return;
    }

    adc->DmaHandle.Init.Request = adc->channel;
    adc->DmaHandle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    adc->DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    adc->DmaHandle.Init.MemInc = ((adc->usedChannelCount > 1) || (ADC_AVERAGE_N_SAMPLES > 1)) ? DMA_MINC_ENABLE : DMA_MINC_DISABLE;
    adc->DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    adc->DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    adc->DmaHandle.Init.Mode = DMA_CIRCULAR;
    adc->DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;
    adc->DmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    adc->DmaHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    adc->DmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;
    adc->DmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;
    adc->DmaHandle.Instance = adc->DMAy_Streamx;

    if (HAL_DMA_Init(&adc->DmaHandle) != HAL_OK) {
        return;
    }

    __HAL_LINKDMA(&adc->ADCHandle, DMA_Handle, adc->DmaHandle);

    uint8_t rank = 0;
    for (int i = ADC_CHN_1; i < ADC_CHN_COUNT; i++) {
        if (!adcConfig[i].enabled || adcConfig[i].adcDevice != adcDevice) {
            continue;
        }

        ADC_ChannelConfTypeDef sConfig;
        sConfig.Channel      = adcConfig[i].adcChannel;
        sConfig.Rank         = adcRegularRankMap[rank++];
        sConfig.SamplingTime = adcConfig[i].sampleTime;
        sConfig.SingleDiff   = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset       = 0;

        if (HAL_ADC_ConfigChannel(&adc->ADCHandle, &sConfig) != HAL_OK) {
            return;
        }
    }

    if (HAL_ADC_Start_DMA(&adc->ADCHandle, (uint32_t*)&adcValues[adcDevice], adc->usedChannelCount * ADC_AVERAGE_N_SAMPLES) != HAL_OK) {
        return;
    }
}

void adcHardwareInit(drv_adc_config_t *init)
{
    UNUSED(init);
    int configuredAdcChannels = 0;

    for (int i = ADC_CHN_1; i < ADC_CHN_COUNT; i++) {
        if (!adcConfig[i].tag)
            continue;

        adcDevice_t * adc = &adcHardware[adcConfig[i].adcDevice];

        IOInit(IOGetByTag(adcConfig[i].tag), OWNER_ADC, RESOURCE_ADC_CH1 + (i - ADC_CHN_1), 0);
        IOConfigGPIO(IOGetByTag(adcConfig[i].tag), IO_CONFIG(GPIO_MODE_ANALOG, 0, GPIO_NOPULL));

        adcConfig[i].adcChannel = adcChannelByTag(adcConfig[i].tag);
        adcConfig[i].dmaIndex = adc->usedChannelCount++;
        adcConfig[i].sampleTime = ADC_SAMPLETIME_387CYCLES_5;
        adcConfig[i].enabled = true;

        adc->enabled = true;
        configuredAdcChannels++;
    }

    if (configuredAdcChannels == 0)
        return;

    for (int i = 0; i < ADCDEV_COUNT; i++) {
        if (adcHardware[i].enabled) {
            adcInstanceInit(i);
        }
    }
}

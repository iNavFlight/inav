/*
 * This file is part of ATbetaflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "drivers/time.h"

#include "drivers/io.h"
#include "io_impl.h"
#include "rcc.h"
#include "drivers/dma.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"

#include "adc.h"
#include "adc_impl.h"

#if !defined(ADC1_DMA_STREAM)
#define ADC1_DMA_STREAM DMA2_CHANNEL1
#endif

static adcDevice_t adcHardware[ADCDEV_COUNT] = {
    { .ADCx = ADC1, .rccADC = RCC_APB2(ADC1), .rccDMA = RCC_AHB1(DMA2), .DMAy_Channelx = ADC1_DMA_STREAM, .dmaMuxid= DMAMUX_DMAREQ_ID_ADC1,.enabled = false, .usedChannelCount = 0 },
};

/* note these could be packed up for saving space */
const adcTagMap_t adcTagMap[] = {
    { DEFIO_TAG_E__PC0, ADC_CHANNEL_10 },
    { DEFIO_TAG_E__PC1, ADC_CHANNEL_11 },
    { DEFIO_TAG_E__PC2, ADC_CHANNEL_12 },
    { DEFIO_TAG_E__PC3, ADC_CHANNEL_13 },
    { DEFIO_TAG_E__PC4, ADC_CHANNEL_14 },
    { DEFIO_TAG_E__PC5, ADC_CHANNEL_15 },
    { DEFIO_TAG_E__PB0, ADC_CHANNEL_8  },
    { DEFIO_TAG_E__PB1, ADC_CHANNEL_9  },
    { DEFIO_TAG_E__PA0, ADC_CHANNEL_0  },
    { DEFIO_TAG_E__PA1, ADC_CHANNEL_1  },
    { DEFIO_TAG_E__PA2, ADC_CHANNEL_2  },
    { DEFIO_TAG_E__PA3, ADC_CHANNEL_3  },
    { DEFIO_TAG_E__PA4, ADC_CHANNEL_4  },
    { DEFIO_TAG_E__PA5, ADC_CHANNEL_5  },
    { DEFIO_TAG_E__PA6, ADC_CHANNEL_6  },
    { DEFIO_TAG_E__PA7, ADC_CHANNEL_7  },
};

ADCDevice adcDeviceByInstance(adc_type *instance)
{
    if (instance == ADC1)
        return ADCDEV_1;
/*
    if (instance == ADC2) // TODO add ADC2 and 3
        return ADCDEV_2;
*/
    return ADCINVALID;
}

/*
 * Init the Config of the adc common \dma \adc base ,use the regular ordinary to init the ADC instance ，eg.ADC1.
 * @param adcDevice ADCdevice 
 *
 */
static void adcInstanceInit(ADCDevice adcDevice)
{
    dma_init_type dma_init_struct;
    adcDevice_t * adc = &adcHardware[adcDevice];
    // get dma channel, then assign the dma channel
    DMA_t dmadac= dmaGetByRef(adc->DMAy_Channelx);
    if(!dmadac)
    {
        return;
    }
    // dmadac->dmaMuxref = adc->dmaMuxid;//this cause an error
    dmaInit(dmadac, OWNER_ADC, adcDevice);
    // dma_channel_type
    dma_reset(dmadac->ref); 

    dma_default_para_init(&dma_init_struct);
    dma_init_struct.peripheral_base_addr = (uint32_t)&adc->ADCx->odt;
    // dma buffer_size= usedChannelCount*sanmple rate
    dma_init_struct.buffer_size = adc->usedChannelCount * ADC_AVERAGE_N_SAMPLES;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.memory_inc_enable =  ((adc->usedChannelCount > 1) || (ADC_AVERAGE_N_SAMPLES > 1)) ? TRUE : FALSE;
    dma_init_struct.loop_mode_enable = TRUE;
    dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_base_addr = (uint32_t)adcValues[adcDevice];
    dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;  
    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;
    dma_init(dmadac->ref, &dma_init_struct);
    //enable dma transfer and dma interrupt 
    dma_interrupt_enable(dmadac->ref, DMA_FDT_INT, TRUE);
    //AT32 DMA MUX 
    dmaMuxEnable(dmadac, adc->dmaMuxid);
    dma_channel_enable(dmadac->ref,TRUE);
   //init adc common 
    adc_reset();
    adc_common_config_type adc_common_struct;
    adc_common_default_para_init(&adc_common_struct);
    adc_common_struct.combine_mode = ADC_INDEPENDENT_MODE;
    adc_common_struct.div = ADC_HCLK_DIV_4;
    adc_common_struct.common_dma_mode = ADC_COMMON_DMAMODE_DISABLE;
    adc_common_struct.sampling_interval = ADC_SAMPLING_INTERVAL_5CYCLES;
    adc_common_struct.tempervintrv_state = FALSE;
    adc_common_struct.common_dma_request_repeat_state = FALSE;
    adc_common_struct.vbat_state = FALSE;
    adc_common_config(&adc_common_struct);
    //enable adc RCC Clock 
    RCC_ClockCmd(adc->rccADC, ENABLE);
    //config adc base
    adc_base_config_type adc_base_struct;
    adc_base_default_para_init(&adc_base_struct);
    adc_base_struct.sequence_mode = TRUE;
    adc_base_struct.repeat_mode = TRUE;
    adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
    adc_base_struct.ordinary_channel_length = adc->usedChannelCount; //based on used channel count 
    adc_base_config(adc->ADCx, &adc_base_struct);
    adc_resolution_set(adc->ADCx, ADC_RESOLUTION_12B);

    uint8_t rank = 1;
    for (int i = ADC_CHN_1; i < ADC_CHN_COUNT; i++) {
        if (!adcConfig[i].enabled || adcConfig[i].adcDevice != adcDevice) {
            continue;
        }
        adc_ordinary_channel_set(adc->ADCx, adcConfig[i].adcChannel, rank++, adcConfig[i].sampleTime);
    }

    // config dma repeat
    adc_dma_request_repeat_enable(adc->ADCx, TRUE);
    adc_dma_mode_enable(adc->ADCx, TRUE);

    //enable over flow interupt 
    adc_interrupt_enable(adc->ADCx, ADC_OCCO_INT, TRUE);

    adc_enable(adc->ADCx,TRUE);
    
    // wait ready to start adc calibration
    while(adc_flag_get(adc->ADCx, ADC_RDY_FLAG) == RESET);
    //start calibration
    adc_calibration_init(adc->ADCx);
    while(adc_calibration_init_status_get(adc->ADCx));
    adc_calibration_start(adc->ADCx);
    while(adc_calibration_status_get(adc->ADCx));
    //start adc
    adc_enable(adc->ADCx, TRUE);
    adc_ordinary_software_trigger_enable(adc->ADCx, TRUE);
}

/*
 * ADC Hardware init ，config the GPIO Port and count the used ADC channel
 *@parm init 
*/
void adcHardwareInit(drv_adc_config_t *init)
{
    UNUSED(init);
    int configuredAdcChannels = 0;
    
    for (int i = ADC_CHN_1; i < ADC_CHN_COUNT; i++) {
        if (!adcConfig[i].tag)
            continue;
        // only use adc1 for now
        adcDevice_t * adc = &adcHardware[adcConfig[i].adcDevice];
        // init adc gpio port
        IOInit(IOGetByTag(adcConfig[i].tag), OWNER_ADC, RESOURCE_ADC_CH1 + (i - ADC_CHN_1), 0);
        IOConfigGPIO(IOGetByTag(adcConfig[i].tag), IO_CONFIG(GPIO_MODE_ANALOG, 0, GPIO_OUTPUT_OPEN_DRAIN, GPIO_PULL_NONE));

        adcConfig[i].adcChannel = adcChannelByTag(adcConfig[i].tag);
        adcConfig[i].dmaIndex = adc->usedChannelCount++; 
        adcConfig[i].sampleTime = ADC_SAMPLETIME_6_5;
        adcConfig[i].enabled = true;

        adc->enabled = true;
        configuredAdcChannels++;
    }

    if (configuredAdcChannels == 0)
        return;
    //  config adc instance 
    for (int i = 0; i < ADCDEV_COUNT; i++) {
        if (adcHardware[i].enabled) {
            adcInstanceInit(i);
        }
    }
}

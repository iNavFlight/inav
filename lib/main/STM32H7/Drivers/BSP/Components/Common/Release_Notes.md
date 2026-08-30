---
pagetitle: Release Notes for BSP Common Components Drivers
lang: en
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# <small>Release Notes for</small> <mark>BSP Common Components Drivers</mark>
Copyright &copy; 2014 STMicroelectronics\
    
[![ST logo](_htmresc/st_logo.png)](https://www.st.com){.logo}
</center>
:::
:::

# License

Licensed by ST under BSD 3-Clause license (the \"License\"). You may
not use this package except in compliance with the License. You may
obtain a copy of the License at:

[https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

# Purpose

This directory contains the BSP Common components drivers.

:::

::: {.col-sm-12 .col-lg-8}
# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section16" checked aria-hidden="true">
<label for="collapse-section16" aria-hidden="true">V7.0.0 / 25-February-2020</label>
<div>			

## Main Changes

### Component release

-	Rename GUI_Drv_t structure into UTILS_LCD_Drv_t

## Backward Compatibility

-	This release breaks compatibility with previous versions.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section15" aria-hidden="true">
<label for="collapse-section15" aria-hidden="true">V6.0.1 / 15-October-2019</label>
<div>			

## Main Changes

### Component release

-	Update st_logo.png inclusion path in Release notes.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section14" aria-hidden="true">
<label for="collapse-section14" aria-hidden="true">V6.0.0 / 12-April-2019</label>
<div>			

## Main Changes

### Component release

Official release of BSP Common components drivers in line with STM32Cube BSP drivers development guidelines (UM2298).

## Backward Compatibility

This release breaks compatibility with previous versions.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13" aria-hidden="true">
<label for="collapse-section13" aria-hidden="true">V5.1.1 / 31-August-2018</label>
<div>			

## Main Changes
Reformat the BSD 3-Clause license declaration in the files header (replace license terms by a web reference to OSI website where those terms lie)  
Correct sensor names in headers files hsensor.h and psensor.h  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12" aria-hidden="true">
<label for="collapse-section12" aria-hidden="true">V5.1.0 / 21-November-2017</label>
<div>			

## Main Changes
Add dpredriver.h: support of DP redriver class  
Add pwrmon.h: support of power monitor class  
Add usbtypecswitch.h: support of USB type C switch class  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11" aria-hidden="true">
<label for="collapse-section11" aria-hidden="true">V5.0.0 / 01-March-2017</label>
<div>			

## Main Changes
Add hsensor.h: support of humidity class  
Add psensor.h: support of pressure class  
Update tsensor.h: Temperature can be negative  
Update accelero.h: LowPower API can enable or disable the low power mode  
Update magneto.h: LowPower API can enable or disable the low power mode  

## Backward Compatibility

This release breaks compatibility with previous versions.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10" aria-hidden="true">
<label for="collapse-section10" aria-hidden="true">V4.0.1 / 21-July-2015</label>
<div>			

## Main Changes
tsensor.h: Fix compilation issue on *TSENSOR_InitTypeDef*

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9" aria-hidden="true">
<label for="collapse-section9" aria-hidden="true">V4.0.0 / 22-June-2015</label>
<div>			

## Main Changes
accelero.h: add *DeInit* field in *ACCELERO_DrvTypeDef* structure  
audio.h: add *DeInit* field in *AUDIO_DrvTypeDef* structure  
idd.h:  

  -   add *Shunt0StabDelay*, *Shunt1StabDelay*, *Shunt2StabDelay*, *Shunt3StabDelay*, *Shunt4StabDelay* and *ShuntNbOnBoard* fields in *IDD_ConfigTypeDef* structure  
  -   rename *ShuntNumber* field to *ShuntNbUsed* in *IDD_ConfigTypeDef* structure  

magneto.h: add *DeInit* field in *MAGNETO_DrvTypeDef* structure  

## Backward Compatibility

This release breaks compatibility with previous versions.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8" aria-hidden="true">
<label for="collapse-section8" aria-hidden="true">V3.0.0 / 28-April-2015</label>
<div>			

## Main Changes

accelero.h: add *LowPower* field in *ACCELERO_DrvTypeDef* structure  
magneto.h: add *LowPower* field in *MAGNETO_DrvTypeDef* structure  
gyro.h: add *DeInit* and *LowPower* fields in *GYRO_DrvTypeDef* structure  
camera.h: add CAMERA_COLOR_EFFECT_NONE define  
idd.h:  

-   add *MeasureNb*, *DeltaDelayUnit* and *DeltaDelayValue* fields in *IDD_ConfigTypeDef* structure  
-   rename *PreDelay* field to *PreDelayUnit* in *IDD_ConfigTypeDef* structure  
  
## Backward Compatibility

This release breaks compatibility with previous versions.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7" aria-hidden="true">
<label for="collapse-section7" aria-hidden="true">V2.2.0 / 09-February-2015</label>
<div>						

## Main Changes

Magnetometer driver function prototypes added (magneto.h file)  
Update "idd.h" file to provide DeInit() and WakeUp() services in IDD current measurement driver  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6" aria-hidden="true">
<label for="collapse-section6" aria-hidden="true">V2.1.0 / 06-February-2015</label>
<div>						

## Main Changes

IDD current measurement driver function prototypes added (idd.h file)  
io.h: add new typedef enum IO_PinState with IO_PIN_RESET and IO_PIN_SET values  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5" aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">V2.0.0 / 15-December-2014</label>
<div>			

## Main Changes

Update "io.h" file to support MFX (Multi Function eXpander) device available on some STM32 boards

-   add new entries for *IO_ModeTypedef* enumeration structure
-   update the *IO_DrvTypeDef* structure
  -   Update all return values and function parameters to uint32_t
  -   Add a return value for *Config* field
    
## Backward Compatibility

This release breaks compatibility with previous versions.

</div>
:::


::: {.collapse}
<input type="checkbox" id="collapse-section4" aria-hidden="true">
<label for="collapse-section4" aria-hidden="true">V1.2.1 / 02-December-2014</label>
<div>			

## Main Changes
gyro.h: change “__GIRO_H” by “__GYRO_H” to fix compilation issue under Mac OS  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">V1.2.0 / 18-June-2014</label>
<div>			

## Main Changes
EPD (E Paper Display)  driver function prototype added (epd.h file)  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2" aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">V1.1.0 / 21-March-2014</label>
<div>			

## Main Changes
Temperature Sensor driver function prototype added  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">V1.0.0 / 18-February-2014</label>
<div>			

## Main Changes
First official release with Accelerometer, Audio, Camera, Gyroscope, IO, LCD and Touch Screen drivers function prototypes  

</div>
:::

:::
:::

<footer class="sticky">
For complete documentation on <mark>STM32 Microcontrollers</mark> ,
visit: [http://www.st.com/STM32](http://www.st.com/STM32)
</footer>

---
pagetitle: Release Notes for MFXSTM32L152 Component Drivers
lang: en
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# <small>Release Notes for </small> <mark>MFXSTM32L152 Component Drivers</mark>
Copyright &copy; 2015 STMicroelectronics\
    
[![ST logo](_htmresc/st_logo.png)](https://www.st.com){.logo}
</center>
:::
:::

# License

This software component is licensed by ST under BSD 3-Clause license, the "License"; You may not use this component except in compliance with the License. You may obtain a copy of the License at:

[https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)

# Purpose

This driver provides a set of functions needed to drive MFXSTM32L152,  IO Expander component

:::

::: {.col-sm-12 .col-lg-8}
# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section5" checked aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">__V3.0.2 / 30-October-2019__</label>
<div>			

## Main Changes

-	Update st_logo.png inclusion path in Release notes.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7" checked aria-hidden="true">
<label for="collapse-section7" aria-hidden="true">V3.0.1 / 18-June-2019</label>
<div>			
## Main Changes
Update MFXSTM32L152_DeInit to avoid disabling IO bus that could be used by others components

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6" checked aria-hidden="true">
<label for="collapse-section6" aria-hidden="true">V3.0.0 / 12-April-2019</label>
<div>			
## Main Changes

### Component release

Official release of component drivers for MFXSTM32L152 in line with STM32Cube BSP drivers development guidelines (UM2298)

## Backward Compatibility

This release breaks compatibility with previous versions.

## Dependencies

STM32Cube BSP Common drivers V6.0.0

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5" aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">V2.0.1 / 02-June-2017</label>
<div>			
## Main Changes
Update comments to be used for PDSC generation
</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" aria-hidden="true">
<label for="collapse-section4" aria-hidden="true">V2.0.0 / 24-June-2015</label>
<div>			
## Main Changes
Add Shunt management of MFXSTM32L152 component
new mfxstm32l152_IDD_ConfigShuntNbLimit() and mfxstm32l152_IDD_GetShuntUsed() APIs
Add mfxstm32l152_WriteReg() API
Note: This release must be used with BSP Common driver V4.0.0 or later
</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">V1.1.0 / 28-April-2015</label>
<div>			
## Main Changes
mfxstm32l152_IO_Config(): remove unnecessary delay
mfxstm32l152_TS_DetectTouch(): improve TouchScreen speed
mfxstm32l152_IDD_Config(): add configuration of number of measure to be performed, with delay between 2 measures
Note: This release must be used with BSP Common driver V3.0.0
</div>
:::


::: {.collapse}
<input type="checkbox" id="collapse-section2" aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">V1.1.0 / 10-February-2015</label>
<div>			
## Main Changes
Low Power management of MFXSTM32L152 component:
New mfxstm32l152_DeInit() and mfxstm32l152_WakeUp() API
mfxstm32l152_LowPower() API completed to be MFXSTM32L152 in Standby mode
Note: This release must be used with BSP Common driver V2.2.0 or later
</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">V1.0.0 / 5-February-2015</label>
<div>			
## Main Changes
First official release of MFXSTM32L152 IO Expander component driver.
Note: This release must be used with BSP Common driver V2.1.0 or later.
</div>
:::


:::
:::

<footer class="sticky">
For complete documentation on STM32 Microcontrollers ,
visit: [http://www.st.com/STM32](http://www.st.com/STM32)
</footer>

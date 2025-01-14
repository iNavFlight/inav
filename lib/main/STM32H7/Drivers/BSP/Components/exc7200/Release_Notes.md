---
pagetitle: Release Notes for EXC7200 Component Driver
lang: en
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# **Release Notes for EXC7200 Component Driver**
Copyright &copy; 2015 STMicroelectronics\
    
[![ST logo](_htmresc/st_logo.png)](https://www.st.com){.logo}
</center>
:::
:::

# License

This software component is licensed by ST under BSD 3-Clause license, the "License"; You may not use this component except in 
compliance with the License. You may obtain a copy of the License at:
<center>
[https://opensource.org/licenses/BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)
</center>

# Purpose

This driver provides a set of audio functions offered by EXC7200 TS component

:::

::: {.col-sm-12 .col-lg-8}
# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section5" checked aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">__V2.0.1 / 23-November-2019__</label>
<div>			

## Main Changes

-	Update st_logo.png inclusion path in Release notes

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section4" aria-hidden="true">__V2.0.0 / 14-October-2019__</label>
<div>			

## Main Changes

-	First Official release of EXC7200 BSP drivers in line with STM32Cube BSP drivers development guidelines (UM2298) 
-	The component drivers are composed of
	-	component core drivers files: exc7200.h/.c
	-	component register drivers files: exc7200_reg.h/.c
	-	component configuration files: exc7200_conf_template.h
	
## Backward Compatibility

-	This version breaks the compatibility with previous versions

## Dependencies

-	This software release is compatible with:

	-	BSP Common v6.0.1 or above

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" checked aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">__V1.0.2 / 07-April-2017__</label>
<div>			

## Main Changes

-	Update comments to be used for PDSC generation

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2" checked aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">__V1.0.1 / 21-September-2015__</label>
<div>			

## Main Changes

-	exc7200.c:
	-	Update the I2C slave read address within exc7200_TS_DetectTouch() function.
	-	Update exc7200_TS_GetXY() function to return correct Touch Screen positions.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__V1.0.0 / 28-April-2015__</label>
<div>			

## Main Changes

-	First official release of **EXC7200** Toush Screen Component drivers 

</div>
:::

:::
:::

<footer class="sticky">
For complete documentation on <mark>STM32 Microcontrollers</mark> ,
visit: [[www.st.com](http://www.st.com/STM32)]{style="font-color: blue;"}
</footer>

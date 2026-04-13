---
pagetitle: Release Notes for TS3510 Component Driver
lang: en
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# **Release Notes for TS3510 Component Driver**
Copyright &copy; 2014 STMicroelectronics\

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

This driver provides a set of functions needed to drive TS3510, Touch Screen component

:::

::: {.col-sm-12 .col-lg-8}

# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section6" checked aria-hidden="true">
<label for="collapse-section6" aria-hidden="true">V2.0.0 / 23-November-2019</label>
<div>

## Main Changes

-	First Official release of component drivers for TS3510 in line with STM32Cube BSP drivers development guidelines (UM2298)
-	The component drivers are composed of
	-	component core drivers files: ts3510.h/.c
	-	component register drivers files: ts3510_reg.h/.c	
	-	component configuration file: ts3510_conf_template.h
	
## Backward Compatibility

-	This version breaks the compatibility with previous versions

## Dependencies

-	This software release is compatible with BSP Common V6.0.0 or above

## Notes

-	ts3510_conf_template.h file must be copied in user application as ts3510_conf.h with optional configuration update 

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" checked aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">V1.0.2 / 05-June-2017</label>
<div>

## Main Changes

-	Update comments to be used for PDSC generation

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2" checked aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">V1.0.1 / 02-December-2014</label>
<div>

## Main Changes

-	st7735.h: change "\" by "/" in the include path to fix compilation issues under Linux.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">V1.0.0 / 18-February-2014</label>
<div>

## Main Changes

-	First official release of **TS3510** TS  driver

</div>
:::

:::
:::

<footer class="sticky">
For complete documentation on <mark>STM32 Microcontrollers</mark> ,
visit: [[www.st.com](http://www.st.com/STM32)]{style="font-color: blue;"}
</footer>

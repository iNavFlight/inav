---
pagetitle: Release Notes for STMPE811 Component Driver
lang: en
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# **Release Notes for STMPE811 Component Driver**
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

This driver provides a set of functions needed to drive STMPE811, IOExpander component

:::

::: {.col-sm-12 .col-lg-8}

# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section6" checked aria-hidden="true">
<label for="collapse-section6" aria-hidden="true">V3.0.0 / 23-November-2019</label>
<div>

## Main Changes

-	First Official release of component drivers for STMPE811 in line with STM32Cube BSP drivers development guidelines (UM2298)
-	The component drivers are composed of
	-	component core drivers files: stmpe811.h/.c
	-	component register drivers files: stmpe811_reg.h/.c	
	-	component configuration file: stmpe811_conf_template.h
	
## Backward Compatibility

-	This version breaks the compatibility with previous versions

## Dependencies

-	This software release is compatible with BSP Common V6.0.0 or above

## Notes

-	stmpe811_conf_template.h file must be copied in user application as stmpe811_conf.h with optional configuration update 

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5" checked aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">V2.0.1 / 05-June-2017</label>
<div>

## Main Changes

-	Update comments to be used for PDSC generation

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section4" aria-hidden="true">V2.0.0 / 15-December-2014</label>
<div>

## Main Changes

-	All functions:
	-	update IO_Pin parameter to uint32_t instead of uint16_t
-	Add a return value for STMPE811_IO_Config() function

## Backward Compatibility

-	This version breaks the compatibility with previous versions

## Dependencies

-	This software release is compatible with BSP Common V2.0.0

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" checked aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">V1.0.2 / 02-December-2014</label>
<div>

## Main Changes

-	st7735.h: change "\" by "/" in the include path to fix compilation issues under Linux.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2" checked aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">V1.0.1 / 11-November-2014</label>
<div>

## Main Changes

-	Fix limitation related to the selection of alternate function for TS physical IO
-	Fix wrong pins definition of the TS
-	Swap implementation of STMPE811_IO_EnableAF() and STMPE811_IO_DisableAF() functions
-	Miscellaneous code cleanup of comments update

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">V1.0.0 / 18-February-2014</label>
<div>

## Main Changes

-	First official release of **STMPE811** IOExpander  driver

</div>
:::

:::
:::

<footer class="sticky">
For complete documentation on <mark>STM32 Microcontrollers</mark> ,
visit: [[www.st.com](http://www.st.com/STM32)]{style="font-color: blue;"}
</footer>

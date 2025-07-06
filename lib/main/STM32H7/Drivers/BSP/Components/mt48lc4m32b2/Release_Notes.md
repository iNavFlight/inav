---
pagetitle: Release Notes for MT48LC4M32B2 Component Driver
lang: en
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# **Release Notes for MT48LC4M32B2 Component Driver**
Copyright &copy; 2019 STMicroelectronics\
    
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

This driver provides a set of SDRAM functions offered by MT48LC4M32B2 component.
The mt48lc4m32b2_conf_template.h must be copied to application level, renamed into mt48lc4m32b2_conf.h and updated with adequate information.

:::

::: {.col-sm-12 .col-lg-8}
# Update History

::: {.collapse}
<input type="checkbox" id="collapse-section2" checked aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">__V1.0.1 / 07-Mai-2020__</label>
<div>			

## Main Changes

-	mt48lc4m32b2_conf_template.h: Remove reference to F7 HAL as drivers can be used with all families. 

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__V1.0.0 / 14-October-2019__</label>
<div>			

## Main Changes

-	First Official release of MT48LC4M32B2 BSP drivers in line with STM32Cube BSP drivers development guidelines (UM2298) 
-	The component drivers are composed of
	-	component core drivers files: mt48lc4m32b2.h/.c
	-	component configuration file: mt48lc4m32b2_conf_template.h

## Dependencies

This software release is compatible with:

-	BSP Common v6.0.1 or above

</div>
:::

:::
:::

<footer class="sticky">
For complete documentation on <mark>STM32 Microcontrollers</mark> ,
visit: [[www.st.com](http://www.st.com/STM32)]{style="font-color: blue;"}
</footer>

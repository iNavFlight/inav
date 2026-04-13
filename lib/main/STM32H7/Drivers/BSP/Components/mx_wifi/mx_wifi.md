---
pagetitle: Release Notes for MX_WIFI Component Driver
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}
::: {.sectione .dark}
<center>
# <small>Release Notes for</small> <mark>MX_WIFI Component Driver</mark>
Copyright &copy; 2020 STMicroelectronics\
Microcontrollers Division - Application Team

[![ST logo](_htmresc/st_logo.png)](https://www.st.com){.logo}
</center>
:::
:::

# License

Licensed by ST under BSD 3-Clause  (the "License"). You may not use this package except in compliance with the License.

You may obtain a copy of the License at: [BSD-3-Clause](https://opensource.org/licenses/BSD-3-Clause)


# Purpose

This directory contains the MX_WIFI component driver.
:::

::: {.col-sm-12 .col-lg-8}
# Update History
::: {.collapse}
<input type="checkbox" id="collapse-section1" checked aria-hidden="false">
<label for="collapse-section1" aria-hidden="true">V1.0.4 / 15-May-2020 </label>
<div>

## Main changes
  - First official release of MX_WIFI WiFi component.
</div>
:::

# Firmware update (click to expand)
::: {.collapse}

<input type="checkbox" id="collapse-section30" aria-hidden="true">
<label for="collapse-section30" aria-hidden="true">Update via UART</label>
<div>

This section provides you with a step by step procedure to upload the MXCHIP firmware of an MXCHIP Arduino shield using the MXCHIP UART interface.

Prerequisites: 

  - The steps described in this document use Tera Term as a terminal emulator. For the firmware update, you should not use a version more recent than 4.85 as you may encounter some issues when using YModem.
  - The material you need to have is:
    - a USB to TTL connector
    - 4 male to female cables with PCX connectors
    - 1 female to female cable with PCX connectors
<br/><br/>

Connect the TTL signals to the MXCHIP Arduino shield: 3.3 V, GND, Tx and Rx and loop the Boot MXCHIP Arduino shield – See the below picture:  

![](_htmresc/fwupdate/picture1.png "picture1")

<br/><br/>
Check that the version of your Tera Term is not more recent than 4.85 using the “Help/About Tera Term” … menu:

![](_htmresc/fwupdate/picture2.png "picture2")

<br/><br/>
Open the Setup/Serial port… and set the Port com correcponding to your setup and the other values as follows:

![](_htmresc/fwupdate/picture3.png "picture3")

<br/><br/>
Press the “Reset” button of the MXCHIP Arduino shield. A menu will be displayed in the Tera Term window as in the below picture:

![](_htmresc/fwupdate/picture4.png "picture4")

<br/><br/>
You have to enter this command: 4 -dev 1 -start 0x0 -end 0x160000:

![](_htmresc/fwupdate/picture5.png "picture5")

<br/><br/>
The board is ready to receive the new firmware:

![](_htmresc/fwupdate/picture6.png "picture6")

<br/>
Select YMODEM Sending:

![](_htmresc/fwupdate/picture7.png "picture7")

<br/><br/>
Then select the firmware available under Firmware\\Drivers\\BSP\\Components\\mx_wifi\\firmware or Firmware\\Drivers\\BSPv2\\Components\\mx_wifi\\firmware (one option only will be available in your package).

<br/><br/>
The firmware download is about to start. It takes up to around 30 seconds.

![](_htmresc/fwupdate/picture8.png "picture8")

<br/><br/>
The firmware upgrade is started:

![](_htmresc/fwupdate/picture9.png "picture9")

<br/><br/>
The firmware upgrade is completed:

![](_htmresc/fwupdate/picture10.png "picture10")

<br/>
More documentation on the MXCHIP bootloader can be found at <https://en.docs.mxchip.com/#/docs/mxoscharacter/1.Bootloader>

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section31" aria-hidden="true">
<label for="collapse-section31" aria-hidden="true">Update via SWD</label>
<div>

This section provides you with a step by step procedure to upload the MXCHIP firmware of an MXCHIP Arduino shield using the SWD interface.

The MXCHIP-FlashTool is a software tool running on PC in order to download a MXCHIP firmware using the SWD interface.

The MXCHIP-FlashTool is supported on these PC OS:

  - Windows 7/10
  - MacOS
  - Linux

And with these debug probes:

  - J-LINK
  - ST-LINK_V2
  - ST-LINK_V2-1

Prerequisites:

  - The MXCHIP-Flash tool is to be installed as described into the below "Tool installation" section.
  - The material you need to have is:
    - A debug probe
    - 4 female to female cables with PCX connectors
    - A 3.3 V generator. Another board can be used as shown in the below picture supplying the power via 2 male to male cables with PCX connectors
Tool installation:

  - Python3 installation:
    - Goto the official website to download python: <https://www.python.org/>

  - Download and install the MXCHIP-Flash tool with pip3
    - run this command: pip3 install mflash -i https://pypi.tuna.tsinghua.edu.cn/simple

  - For Windows only, install the needed USB driver
    - Download zadig from <https://zadig.akeo.ie>
    - Run zadig with administrator rights
      - Select: Options / List All Devices / Bulk interface / Replace Driver

Connect the debug probe to the MXCHIP board

  - Debug probe VCC to MXCHIP VDD
  - Debug probe DIO to MXCHIP DIO
  - Debug probe SCK to MXCHIP CLK
  - Debug probe GND to MXCHIP GND

Connect the +3.3 V and GND MXCHIP board to a power supply.

![](_htmresc/fwupdate/picture11.png "picture11"){width=60%}

<br/><br/>

USB driver verification. Opening your Device Manager (on Windows) you should see the USB driver as per the below picture

![](_htmresc/fwupdate/picture12.png "picture12"){.center}
<br/><br/>

Download your image

  - Right click on your bin file to download, select mflash-Download, see next picture
    - The image to be downloaded shall have the ALL.BIN suffix

![](_htmresc/fwupdate/picture13.png "picture13")
<br/><br/>

  - Select chip: mx1290
  - Select interface: stlink-v2 (or stlink-v2-1/j-link depend on your hardware)
  - Set address: 0

![](_htmresc/fwupdate/picture14.png "picture14")
<br/><br/>

  - Click download button to start.
  - Wait until the progress bar reaches 100%.

![](_htmresc/fwupdate/picture15.png "picture15")
<br/><br/>

</div>
:::




:::
:::

<footer class="sticky">
For complete documentation on <mark>STM32</mark> microcontrollers please visit <http://www.st.com/stm32>
</footer>

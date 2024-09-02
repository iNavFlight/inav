# Azure RTOS NetX Duo

This advanced, industrial-grade TCP/IP network stack is designed specifically for deeply embedded real-time and IoT applications. Azure RTOS NetX Duo is a dual IPv4 and IPv6 network stack, while Azure RTOS NetX is the original IPv4 network stack, essentially a subset of Azure RTOS NetX Duo.

Here are the key features and modules of NetX Duo:

![NetX Duo Key Features](./docs/netx-duo-features.png)

## Getting Started

Azure RTOS NetX Duo as part of Azure RTOS has been integrated to the semiconductor's SDKs and development environment. You can develop using the tools of choice from [STMicro](https://www.st.com/content/st_com/en/campaigns/x-cube-azrtos-azure-rtos-stm32.html), [NXP](https://www.nxp.com/design/software/embedded-software/azure-rtos-for-nxp-microcontrollers:AZURE-RTOS), [Renesas](https://github.com/renesas/azure-rtos) and [Microchip](https://mu.microchip.com/get-started-simplifying-your-iot-design-with-azure-rtos).

We also provide [getting started guide](https://github.com/azure-rtos/getting-started) and [samples](https://github.com/azure-rtos/samples) using hero development boards from semiconductors you can build and test with.

See [Overview of Azure RTOS NetX Duo](https://learn.microsoft.com/azure/rtos/netx-duo/overview-netx-duo) for the high-level overview, and all documentation and APIs can be found in: [Azure RTOS NetX Duo documentation](https://learn.microsoft.com/azure/rtos/netx-duo/).

Also there is dedicated [learning path of Azure RTOS NetX Duo](https://learn.microsoft.com/training/paths/azure-rtos-netx-duo/) for learning systematically.

## Repository Structure and Usage

### Directory layout

    .
    ├── addons                  # NetX Duo addon modules for protocols and connectivity
    ├── cmake                   # CMakeList files for building the project
    ├── common                  # Core NetX Duo files
    ├── crypto_libraries        # NetX Crypto files
    ├── nx_secure               # NetX Secure files
    ├── ports                   # Architecture and compiler specific files
    ├── samples                 # Sample codes
    ├── utility                 # Test cases and utilities (e.g. iperf)
    ├── LICENSE.txt             # License terms
    ├── LICENSE-HARDWARE.txt    # Licensed hardware from semiconductors
    ├── CONTRIBUTING.md         # Contribution guidance
    └── SECURITY.md             # Microsoft repo security guidance

### Branches & Releases

The master branch has the most recent code with all new features and bug fixes. It does not represent the latest General Availability (GA) release of the library. Each official release (preview or GA) will be tagged to mark the commit and push it into the Github releases tab, e.g. `v6.2-rel`.

## Protocols and connectivity

Protocols and connectivity support are provided as addon modules within NetX Duo in `addons` folder. Some key modules are: [**azure_iot**](https://github.com/azure-rtos/netxduo/tree/master/addons/azure_iot), [**dhcp**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-dhcp-client/chapter1), [**dns**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-dns/chapter1), [**ftp**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-dns/chapter1), [**http**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-http/chapter1), [**mqtt**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-mqtt/chapter1), [**pop3**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-pop3-client/chapter1), [**lwm2m**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-lwm2m/chapter1), [**ppp**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-ppp/chapter1), [**sntp**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-sntp-client/chapter1), and [**web**](https://learn.microsoft.com/azure/rtos/netx-duo/netx-duo-web-http/chapter1). For a full list of NetX Duo addons, you can find in the same [Azure RTOS NetX Duo documentation](https://learn.microsoft.com/azure/rtos/netx-duo/).

### Samples

We provide sample codes about how to use various addons in the [`samples`](./samples/) folder.

### Azure IoT Middleware for Azure RTOS

[Azure IoT Middleware for Azure RTOS](https://github.com/azure-rtos/netxduo/tree/master/addons/azure_iot) (a.k.a IoT Middleware) is a platform specific library that acts as a binding layer between the Azure RTOS and the [Azure SDK for Embedded C](https://github.com/Azure/azure-sdk-for-c). It simplifies building device application that connects to Azure IoT services.

The IoT Middleware also includes built-in support for:

- **[Device Update for IoT Hub](https://learn.microsoft.com/azure/iot-hub-device-update/device-update-azure-real-time-operating-system)**: an Azure service for IoT devices to enable the over-the-air (OTA) updates easily.
- **[Microsoft Defender for IoT](https://learn.microsoft.com/azure/defender-for-iot/device-builders/iot-security-azure-rtos)**: an Azure service makes IoT devices visibility into security posture management and threat detection, and integrates with other Microsoft tools for unified security management.

## Component dependencies

The main components of Azure RTOS are each provided in their own repository, but there are dependencies between them, as shown in the following graph. This is important to understand when setting up your builds.

![dependency graph](docs/deps.png)

> You will have to take the dependency graph above into account when building anything other than ThreadX itself.

### Building and using the library

Instruction for building the NetX Duo as static library using Arm GNU Toolchain and CMake. If you are using toolchain and IDE from semiconductor, you might follow its own instructions to use Azure RTOS components as explained in the [Getting Started](#getting-started) section.

1. Install the following tools:

    * [CMake](https://cmake.org/download/) version 3.0 or later
    * [Arm GNU Toolchain for arm-none-eabi](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
    * [Ninja](https://ninja-build.org/)

1. Build the [ThreadX library](https://github.com/azure-rtos/threadx#building-and-using-the-library) as the dependency.

1. Cloning the repo. NetX Duo has a couple of dependencies that are included as submodules.

    ```bash
    $ git clone --recursive https://github.com/azure-rtos/netxduo.git
    ```

1. Define the features and addons you need in `nx_user.h` and build together with the component source code. You can refer to [`nx_user_sample.h`](https://github.com/azure-rtos/netxduo/blob/master/common/inc/nx_user_sample.h) as an example.

1. Building as a static library

    Each component of Azure RTOS comes with a composable CMake-based build system that supports many different MCUs and host systems. Integrating any of these components into your device app code is as simple as adding a git submodule and then including it in your build using the CMake `add_subdirectory()`.

    While the typical usage pattern is to include NetX Duo into your device code source tree to be built & linked with your code, you can compile this project as a standalone static library to confirm your build is set up correctly.

    An example of building the library for Cortex-M4:

    ```bash
    $ cmake -Bbuild -GNinja -DCMAKE_TOOLCHAIN_FILE=cmake/cortex_m4.cmake .

    $ cmake --build ./build
    ```

## Professional support

[Professional support plans](https://azure.microsoft.com/support/options/) are available from Microsoft. For community support and others, see the [Resources](#resources) section below.

## Licensing

License terms for using Azure RTOS are defined in the LICENSE.txt file of this repo. Please refer to this file for all definitive licensing information. No additional license fees are required for deploying Azure RTOS on hardware defined in the [LICENSED-HARDWARE.txt](./LICENSED-HARDWARE.txt) file. If you are using hardware not listed in the file or having licensing questions in general, please contact Microsoft directly at https://aka.ms/azrtos-license.

## Resources

The following are references to additional Azure RTOS resources:

- **Product introduction and white papers**: https://azure.com/rtos
- **General technical questions**: https://aka.ms/QnA/azure-rtos
- **Product issues and bugs, or feature requests**: https://github.com/azure-rtos/netxduo/issues
- **Licensing and sales questions**: https://aka.ms/azrtos-license
- **Product roadmap and support policy**: https://aka.ms/azrtos/lts
- **Blogs and videos**: http://msiotblog.com and https://aka.ms/iotshow
- **Azure RTOS TraceX Installer**: https://aka.ms/azrtos-tracex-installer

You can also check [previous questions](https://stackoverflow.com/questions/tagged/azure-rtos+netxduo) or ask new ones on StackOverflow using the `azure-rtos` and `netxduo` tags.

## Security

Azure RTOS provides OEMs with components to secure communication and to create code and data isolation using underlying MCU/MPU hardware protection mechanisms. It is ultimately the responsibility of the device builder to ensure the device fully meets the evolving security requirements associated with its specific use case.

## Contribution

Please follow the instructions provided in the [CONTRIBUTING.md](./CONTRIBUTING.md) for the corresponding repository.

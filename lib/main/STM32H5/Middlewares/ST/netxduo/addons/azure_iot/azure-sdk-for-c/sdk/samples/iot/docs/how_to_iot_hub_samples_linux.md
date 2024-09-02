# How to Setup and Run Azure SDK for Embedded C IoT Hub Certificate Samples on Linux

- [How to Setup and Run Azure SDK for Embedded C IoT Hub Certificate Samples on Linux](#how-to-setup-and-run-azure-sdk-for-embedded-c-iot-hub-certificate-samples-on-linux)
  - [Introduction](#introduction)
    - [What is Covered](#what-is-covered)
  - [Prerequisites](#prerequisites)
  - [Setup Instructions](#setup-instructions)
  - [Configure and Run the Samples](#configure-and-run-the-samples)
  - [Sample Instructions](#sample-instructions)
    - [IoT Hub C2D Sample](#iot-hub-c2d-sample)
    - [IoT Hub Methods Sample](#iot-hub-methods-sample)
    - [IoT Hub Telemetry Sample](#iot-hub-telemetry-sample)
    - [IoT Hub Twin Sample](#iot-hub-twin-sample)
    - [IoT Hub Plug and Play Sample](#iot-hub-plug-and-play-sample)
    - [IoT Hub Plug and Play Multiple Component Sample](#iot-hub-plug-and-play-multiple-component-sample)
  - [Troubleshooting](#troubleshooting)
  - [Contributing](#contributing)
    - [License](#license)

## Introduction

This is a step-by-step guide of how to start from scratch and get the Azure SDK for Embedded C IoT Hub Certificate Samples running on Linux.

Samples are designed to highlight the function calls required to connect with the Azure IoT Hub. These calls illustrate the happy path of the [mqtt state machine](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/docs/iot/mqtt_state_machine.md). As a result, **these samples are NOT designed to be used as production-level code**. Production code needs to incorporate other elements, such as connection retries and more extensive error-handling, which these samples do not include.

For Linux, the command line examples are tailored to Debian/Ubuntu environments. While Linux devices are not likely to be considered constrained, these samples enable developers to test the Azure SDK for Embedded C libraries, debug, and step through the code, even without a real device. We understand not everyone will have a real device to test and that sometimes these devices won't have debugging capabilities.

NOTE: For simplicity in this instruction set, all repository downloads will be performed at the `\` root. Please feel free to use your preferred location.

**WARNING: Samples are generic and should not be used in any production-level code.**

### What is Covered

- Setup instructions for the Azure SDK for Embedded C suite.
- Configuration, build, and run instructions for the IoT Hub Client Certificate Samples.

_The following was run on an Ubuntu Desktop 18.04 environment, but it also works on WSL 1 and 2 (Windows Subsystem for Linux)._

## Prerequisites

To run the samples, ensure you have the following programs and tools installed on your system:

- Have an [Azure account](https://azure.microsoft.com/) created.
- Have an [Azure IoT Hub](https://docs.microsoft.com/azure/iot-hub/iot-hub-create-through-portal) created.
- Have the following setup: build environment, tools, [Git](https://git-scm.com/download/linux), and OpenSSL.  These commands can be run from any directory.

    ```bash
    sudo apt-get update
    sudo apt-get install build-essential curl unzip tar pkg-config git openssl libssl-dev
    ```

## Setup Instructions

1. Install Microsoft [vcpkg](https://github.com/microsoft/vcpkg) package manager and [Eclipse Paho MQTT C client](https://www.eclipse.org/paho/). This installation may take an extended amount of time (~15-20 minutes).

    NOTE: For the correct vcpkg commit, see [vcpkg-commit.txt](https://github.com/Azure/azure-sdk-for-c/blob/main/eng/vcpkg-commit.txt).

    ```bash
    cd ~ # Run this command from any directory to go to your user home directory.
    ~$ sudo git clone https://github.com/Microsoft/vcpkg.git
    ~$ cd vcpkg
    ~/vcpkg git checkout <vcpkg commit> # Checkout the vcpkg commit per vcpkg-commit.txt above.
    ~/vcpkg$ sudo ./bootstrap-vcpkg.sh
    ~/vcpkg$ sudo ./vcpkg install --triplet x64-linux curl cmocka paho-mqtt
    ~/vcpkg$ cd ..
    ```

2. Set the vcpkg environment variables.

    ```bash
    ~$ export VCPKG_DEFAULT_TRIPLET=x64-linux
    ~$ export VCPKG_ROOT=~/vcpkg
    ```

    NOTE: Please keep in mind, **every time a new terminal is opened, the environment variables will have to be reset**.

3. Install [CMake](https://cmake.org/files).

    For Ubuntu 18.04 or 20.04, run the following command:

    ```
    sudo apt-get install cmake
    ```

    For Ubuntu 16.04, run the following commands:

    ```bash
    ~$ wget https://cmake.org/files/v3.18/cmake-3.18.3-Linux-x86_64.sh # Use latest version.
    ~$ sudo chmod 777 cmake-3.18.3-Linux-x86_64.sh # Update permissions to execute script.
    ~$ sudo ./cmake-3.18.3-Linux-x86_64.sh --prefix=/usr
    ```

    - When prompted to include the default subdirectory, enter `n` so to install in `/usr/local`.

4. Clone the Azure SDK for Embedded C IoT repository.

    ```bash
    ~$ git clone https://github.com/Azure/azure-sdk-for-c.git
    ```

## Configure and Run the Samples

1. Generate a self-signed device certificate.

    For the certificate samples, x509 authentication is used to connect to Azure IoT Hub.

    **WARNING: Certificates created by these commands MUST NOT be used in production-level code on Windows or macOS.** These certificates expire after one day and are provided ONLY to help you easily understand CA Certificates. When productizing against CA Certificates, you will need to use your own security best practices for certificate creation and lifetime management.

    The resulting thumbprint will be placed in `fingerprint.txt` and the generated pem file is named `device_ec_cert.pem`.

    Run the following commands:

    ```bash
    ~$ cd azure-sdk-for-c/sdk/samples/iot/

    ~/azure-sdk-for-c/sdk/samples/iot$ openssl ecparam -out device_ec_key.pem -name prime256v1 -genkey
    ~/azure-sdk-for-c/sdk/samples/iot$ openssl req -new -days 1 -nodes -x509 -key device_ec_key.pem -out device_ec_cert.pem -extensions client_auth -config x509_config.cfg -subj "/CN=paho-sample-device1"
    ~/azure-sdk-for-c/sdk/samples/iot$ openssl x509 -noout -text -in device_ec_cert.pem
    ```

    <details><summary><i>The output will look similar to:</i></summary>
    <p>

    ```bash
    Certificate:
    Data:
        Version: 1 (0x0)
        Serial Number:
            40:1a:fa:d2:fd:a5:b2:b7:e7:59:b8:0d:17:4d:9a:10:19:6f:56:0b
        Signature Algorithm: ecdsa-with-SHA256
        Issuer: CN = paho-sample-device1
        Validity
            Not Before: Sep 17 22:10:12 2020 GMT
            Not After : Sep 17 22:10:12 2021 GMT
        Subject: CN = paho-sample-device1
        Subject Public Key Info:
            Public Key Algorithm: id-ecPublicKey
                Public-Key: (256 bit)
                pub:
                    04:d0:23:f4:71:8a:5b:d2:2b:e3:95:94:0f:62:1b:
                    03:52:f2:e3:99:50:e8:23:84:26:ac:aa:88:e5:28:
                    44:ba:56:5c:80:0d:4f:3b:e2:a3:28:60:87:a4:d1:
                    e5:13:49:45:cd:e0:e6:ad:f1:39:e6:47:47:7d:d5:
                    55:1b:fd:53:3e
                ASN1 OID: prime256v1
                NIST CURVE: P-256
    Signature Algorithm: ecdsa-with-SHA256
        30:46:02:21:00:a6:c6:63:16:97:e6:19:ec:a2:f5:c2:20:da:
        91:73:5e:c1:a3:9a:02:76:c7:89:ab:65:c7:22:8b:ea:21:2e:
        cf:02:21:00:9a:c9:15:c7:b3:ac:c0:ef:38:9b:ed:3b:ff:3d:
        62:88:71:29:56:ce:3f:d7:39:fb:0f:54:a3:78:65:c6:be:2f
    ```

    </p>
    </details>

    Run the following commands:

    ```bash
    ~/azure-sdk-for-c/sdk/samples/iot$ rm -f device_cert_store.pem
    ~/azure-sdk-for-c/sdk/samples/iot$ cat device_ec_cert.pem device_ec_key.pem > device_cert_store.pem
    ~/azure-sdk-for-c/sdk/samples/iot$ openssl x509 -noout -fingerprint -in device_ec_cert.pem | sed 's/://g'| sed 's/\(SHA1 Fingerprint=\)//g' | tee fingerprint.txt
    ```

    <details><summary><i>The output will be the fingerprint and will look similar to:</i></summary>
    <p>

    ```bash
    87B4BAEE5F21CE235A887D703C66FD054AD96701
    ```

    - NOTE: This fingerprint is also stored in the generated `fingerprint.txt`.

    </p>
    </details>

    Complete by setting the cert pem file path environment variable:

    ```bash
    ~/azure-sdk-for-c/sdk/samples/iot$ export AZ_IOT_DEVICE_X509_CERT_PEM_FILE_PATH=$(pwd)/device_cert_store.pem
    ```

2. Create a logical device.

    In your Azure IoT Hub, add a new device using a self-signed certificate. See [here](https://docs.microsoft.com/azure/iot-hub/iot-hub-security-x509-get-started#create-an-x509-device-for-your-iot-hub) to get started, but use the values below:

    - **Device ID**: testdevice-x509
    - **Authentication type**: X.509 Self-Signed
    - **Primary and Secondary Thumbprint**: Use the recently generated fingerprint, which has been placed in the file `fingerprint.txt`.

    Select "Save".

3. Set the remaining environment variables needed for the samples.

    - `AZ_IOT_HUB_DEVICE_ID`: Select your device from the IoT Devices page and copy its Device Id.
    - `AZ_IOT_HUB_HOSTNAME`: Copy the Hostname from the Overview tab in your Azure IoT Hub.

    Run the following commands:

    ```bash
    export AZ_IOT_HUB_DEVICE_ID=testdevice-x509
    export AZ_IOT_HUB_HOSTNAME=myiothub.azure-devices.net # Use the your hostname instead.
    ```

4. Build the Azure SDK for Embedded C directory structure.

    ```bash
    ~/azure-sdk-for-c/sdk/samples/iot$ cd ../../..
    ~/azure-sdk-for-c$ mkdir build
    ~/azure-sdk-for-c$ cd build
    ~/azure-sdk-for-c/build$ cmake -DTRANSPORT_PAHO=ON ..
    ```

5. Compile and run your sample of choice from within the `build` directory.

    ```bash
    ~/azure-sdk-for-c/build$ cmake --build .
    ~/azure-sdk-for-c/build$ ./sdk/samples/iot/paho_iot_hub_telemetry_sample  # Use the executable of your choice.
    ```

## Sample Instructions

### IoT Hub C2D Sample

- *Executable:* `paho_iot_hub_c2d_sample`

For the sample description and interaction instructions, please go [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/README.md#iot-hub-c2d-sample).

### IoT Hub Methods Sample

- *Executable:* `paho_iot_hub_methods_sample`

For the sample description and interaction instructions, please go [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/README.md#iot-hub-methods-sample).

### IoT Hub Telemetry Sample

- *Executable:* `paho_iot_hub_telemetry_sample`

For the sample description and interaction instructions, please go [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/README.md#iot-hub-telemetry-sample).

### IoT Hub Twin Sample

- *Executable:* `paho_iot_hub_twin_sample`

For the sample description and interaction instructions, please go [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/README.md#iot-hub-twin-sample).

### IoT Hub Plug and Play Sample

- *Executable:* `paho_iot_pnp_sample`

For the sample description and interaction instructions, please go [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/README.md#iot-hub-plug-and-play-sample).

### IoT Hub Plug and Play Multiple Component Sample

- *Executable:* `paho_iot_pnp_component_sample`

For the sample description and interaction instructions, please go [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/samples/iot/README.md#iot-hub-plug-and-play-multiple-component-sample).

## Troubleshooting

- The error policy for the Embedded C SDK client library is documented [here](https://github.com/Azure/azure-sdk-for-c/blob/main/sdk/docs/iot/mqtt_state_machine.md#error-policy).
- File an issue via [Github Issues](https://github.com/Azure/azure-sdk-for-c/issues/new/choose).
- Check [previous questions](https://stackoverflow.com/questions/tagged/azure+c) or ask new ones on StackOverflow using the `azure` and `c` tags.

## Contributing

This project welcomes contributions and suggestions. Find more contributing details [here](https://github.com/Azure/azure-sdk-for-c/blob/main/CONTRIBUTING.md).

### License

Azure SDK for Embedded C is licensed under the [MIT](https://github.com/Azure/azure-sdk-for-c/blob/main/LICENSE) license.

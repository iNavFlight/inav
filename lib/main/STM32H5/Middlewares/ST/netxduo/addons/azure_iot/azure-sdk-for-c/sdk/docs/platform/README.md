# Azure SDK Platform & Http Abstractions

The Azure SDK platform layer provides abstractions for platform specific APIs which are required by the Azure SDK.  The Azure SDK transport layer provides an abstraction for replacing the HTTP stack.

## Platform

Azure SDK Core depends on some system-specific functions. These functions are not part of the C99 standard library and their implementation depends on system architecture (for example, clock, or thread sleep).

Azure SDK provides three platform implementations for you, one for Windows (`az_win32`), another for Linux and MacOS (`az_posix`) and an empty implementation (`az_noplatform`).

See [CMake Options][azure_sdk_cmake_options] to learn about how to build and use each of these available targets or how to use your own platform implementation.

>Note: `az_noplatform` can be used to link your application and enable Azure Core to run. However, this is not recommended for a production application. It is suggested to be used for testing or just for getting started.

## HTTP Transport Adapter

The Azure SDK has its own HTTP request and response structures. For this reason, an HTTP transport adapter is required to be implemented to use the Azure SDK HTTP request and response as input for any HTTP stack (e.g. libcurl or win32).  The SDK clients using HTTP require a transport adapter to communicate with Azure.

Azure SDK provides one implementation for libcurl (`az_curl`). To consume this implementation, link your application against `az_core` and `az_curl` (if using Cmake) and then HTTP requests will be sent using `libcurl`.

>Note: See [CMake Options][azure_sdk_cmake_options]. You have to turn on building curl transport in order to have this adapter available.

The Azure SDK also provides empty HTTP adapter (`az_nohttp`). This transport allows you to build `az_core` without any specific HTTP adapter. Use this option when the application is not using HTTP based Azure SDK services.

>Note: An `AZ_ERROR_DEPENDENCY_NOT_PROVIDED` will be returned from the `az_nohttp` transport APIs.

You can also implement your own HTTP transport adapter and use it. This allows you to use a different HTTP stack other than `libcurl`. Follow the instructions on [using your own HTTP stack implementation](https://github.com/Azure/azure-sdk-for-c/blob/main/README.md#using-your-own-http-stack-implementation).


## Contributing

If you'd like to contribute to this library, please read the [contributing guide][azure_sdk_for_c_contributing] to learn more about how to build and test the code.

### License

Azure SDK for Embedded C is licensed under the [MIT][azure_sdk_for_c_license] license.

<!-- LINKS -->
[azure_sdk_for_c_contributing]: https://github.com/Azure/azure-sdk-for-c/blob/main/CONTRIBUTING.md
[azure_sdk_for_c_license]: https://github.com/Azure/azure-sdk-for-c/blob/main/LICENSE
[azure_sdk_cmake_options]: https://github.com/Azure/azure-sdk-for-c/blob/main/README.md#cmake-options

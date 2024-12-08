# Release History

## 1.4.0 (2022-11-11)

### Features Added

- [[#2329](https://github.com/Azure/azure-sdk-for-c/pull/2329)] Add Base64 URL decoder.
- [[#2375](https://github.com/Azure/azure-sdk-for-c/pull/2375)] Add Azure Device Update for IoT Hub, enabling Over-the-Air (OTA) updates for embedded devices.

### Breaking Changes

- Modified the `az_json_string_unescape()` function signature to accept `az_span` as the destination.

### Bugs Fixed

- [[#2372](https://github.com/Azure/azure-sdk-for-c/pull/2372)] Incorrect minimum buffer size calculation when logging an HTTP request.

### Other Changes

- [[#2400](https://github.com/Azure/azure-sdk-for-c/pull/2400)] Updated the vcpkg baseline for installing SDK dependencies to the October 2022 tag.

## 1.4.0-beta.1 (2022-08-09)

### Features Added

- Added support in `az_json.h` to unescape JSON string tokens within an `az_span` using the `az_json_string_unescape()` API.

## 1.3.2 (2022-07-07)

### Other Changes
 - Removed unreachable code in `az_http_policy_retry.c`.

## 1.3.1 (2022-04-05)

### Bugs Fixed

- [[#2152]](https://github.com/Azure/azure-sdk-for-c/pull/2152) Fix value in user agent string.
- [[#2162]](https://github.com/Azure/azure-sdk-for-c/pull/2162) Remove failure if $version is not present in IoT Twin reported properties response topic.

## 1.3.0 (2022-02-08)

### Features Added

- Added a total bytes written field to the JSON writer.

### Breaking Changes

- Compared to the previous 1.2.0 release, there are **no** breaking changes.
- Removed `az_storage_blobs.h`, which included some beta APIs for Azure Blob Storage such as `az_storage_blobs_blob_client()`.
  - These will ship in a future release, and are still available from [the previous beta release](https://github.com/Azure/azure-sdk-for-c/releases/tag/1.3.0-beta.1).

### Bugs Fixed

- [[#2027]](https://github.com/Azure/azure-sdk-for-c/pull/2027) Update IoT user agent to append property name before property value in cases where a custom user agent was specified.
- [[#1885]](https://github.com/Azure/azure-sdk-for-c/pull/1885) Fix compilation error C4576 with C++ and MSVC 2019. (A community contribution, courtesy of _[hwmaier](https://github.com/hwmaier)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure SDK for C better with their contributions to this release:

- Henrik Maier _([GitHub](https://github.com/hwmaier))_

## 1.3.0-beta.1 (2021-11-09)

### Features Added

- Added `az_http_response_get_status_code()` convenience function to get HTTP status code from requests.

### Bugs Fixed

- Fixed `az_curl` CMake dependency propagation on `libcurl`.

### Other Changes

- Improved HTTP request telemetry.

## 1.2.0 (2021-09-08)

### Features Added

- Add `az_iot_provisioning_client_get_request_payload()` to create MQTT payload bodies during Device Provisioning.
- This version provides new APIs to follow the IoT Plug and Play convention to implement Telemetry, Commands, Properties and Components defined in a DTDL model.
  - To read/write properties, the SDK now provides functions to produce the right payload for components, as shown in the header `azure/iot/az_iot_hub_client_properties.h`.
  - To send telemetry messages, the required header is added to identify components.
  - When responding to a command invocation the component name is automatically parsed and provided when available.
  - All new samples follow the IoT Plug and Play convention and can be connected to IoT Hub (with or without DPS), or IoT Central.

### Bugs Fixed

- [[#1905]](https://github.com/Azure/azure-sdk-for-c/pull/1905) Fix the internal state of the JSON writer during calls to `az_json_writer_append_json_text()` by taking into account the required buffer space for commas. (A community contribution, courtesy of _[hwmaier](https://github.com/hwmaier)_)

### Acknowledgments

Thank you to our developer community members who helped to make Azure SDK for C better with their contributions to this release:

- Henrik Maier _([GitHub](https://github.com/hwmaier))_

## 1.2.0-beta.1 (2021-07-09)

### New Features

- Added a current depth field to the JSON reader.
- Added base64 encoding and decoding APIs that accept `az_span`, available from the `azure/core/az_base64.h` header.
- Public preview version of a new set of APIs to simplify the experience using Azure IoT Plug and Play. To consume this new feature, the `az::iot::hub` CMake target has been updated. The APIs can be found in the header files: `az_iot_hub_client.h` and `az_iot_hub_client_properties.h`.
- New samples showing how to consume the new IoT Plug and Play APIs:
  - paho_iot_pnp_component_sample.c
  - paho_iot_pnp_sample.c
  - paho_iot_pnp_with_provisioning_sample.c

### Bug Fixes

- [[#1640]](https://github.com/Azure/azure-sdk-for-c/pull/1640) Update precondition on `az_iot_provisioning_client_parse_received_topic_and_payload()` to require topic and payload minimum size of 1 instead of 0.
- [[#1699]](https://github.com/Azure/azure-sdk-for-c/pull/1699) Update precondition on `az_iot_message_properties_init()` to not allow `written_length` larger than the passed span.

## 1.1.0 (2021-03-09)

### Breaking Changes

- Compared to the previous 1.0.0 release, there are **no** breaking changes.
- Removed `az_iot_pnp_client.h`, which included some beta APIs related to IoT Plug and Play such as `az_iot_pnp_client()`.
  - These will ship in a future release (1.2.0).

### Bug Fixes

- [[#1600]](https://github.com/Azure/azure-sdk-for-c/pull/1600) Make sure `az_json_writer_append_json_text()` appends a comma between elements of a JSON array.
- [[#1580]](https://github.com/Azure/azure-sdk-for-c/pull/1580) Fix build on Ubuntu 18.04 by updating CMake policy and MSVC runtime libraries.

## 1.1.0-beta.2 (2020-11-11)

### Bug Fixes

- [[#1472]](https://github.com/Azure/azure-sdk-for-c/pull/1472) Fix `az_iot_message_properties_next()` when the buffer in which the properties were kept was bigger than the length of characters in the buffer.

### Other Changes and Improvements

- [[#1473]](https://github.com/Azure/azure-sdk-for-c/pull/1473) Add remote server certificate validation on paho and ESP8266 samples.
- [[#1449]](https://github.com/Azure/azure-sdk-for-c/pull/1449) Add basic reconnection capability for the ESP8266 sample.
- [[#1490]](https://github.com/Azure/azure-sdk-for-c/pull/1490) Fix static analyzer flagging of non-checked return value in `az_iot_hub_client_c2d_parse_received_topic()`.

## 1.1.0-beta.1 (2020-10-06)

### New Features

- Added an `az_log_classification_filter_fn` callback function type along with a setter `az_log_set_classification_filter_callback()`, allowing the caller to filter log messages.

### Bug Fixes

- Fix bounds check while processing incomplete JSON string containing escaped characters to avoid out-of-range access.
- Fix Windows to use /MT when building the CRT and static libraries.
- Fail gracefully on invalid/incomplete HTTP response processing by avoiding reading from size 0 span.

### Other Changes and Improvements

- Add precondition check to validate clients are initialized before passed in to public APIs.
- Add high-level and simplified az_core.h and az_iot.h files for simpler include experience for customers.

## 1.0.0 (2020-09-21)

### Breaking Changes

- Removed `az_storage_blobs.h`, including APIs related to storage service such as `az_storage_blobs_blob_client_init()` and `az_storage_blobs_blob_upload()`, and types such as `az_storage_blobs_blob_client` and `az_storage_blobs_blob_client_options`.
  - These will ship in the upcoming 1.1.0 release and will continue to be available as preview from the following branch: https://github.com/Azure/azure-sdk-for-c/tree/feature/StorageBlobs
- Updated provisioning client struct member name in `az_iot_provisioning_client_register_response` from `registration_result` to `registration_state`.
- Changed `operation_status` in `az_iot_provisioning_client_register_response` from `az_span` to `az_iot_provisioning_client_operation_status` enum.
- Removed `az_iot_provisioning_client_parse_operation_status()` from `az_iot_provisioning_client.h`.
- Renamed `az_iot_hub_client_twin_response_type` enum names:
  - `AZ_IOT_CLIENT_TWIN_RESPONSE_TYPE_GET` to `AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_GET`
  - `AZ_IOT_CLIENT_TWIN_RESPONSE_TYPE_DESIRED_PROPERTIES` to `AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_DESIRED_PROPERTIES`
  - `AZ_IOT_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES` to `AZ_IOT_HUB_CLIENT_TWIN_RESPONSE_TYPE_REPORTED_PROPERTIES`
- Platform: `az_platform_clock_msec()`, and `az_platform_sleep_msec()` return `az_result`, which is `AZ_ERROR_DEPENDENCY_NOT_PROVIDED` when `az_noplatform` implementation is used.
- Moved these IoT specific result codes from `az_result.h` to `az_iot_common.h`:
  - `AZ_ERROR_IOT_TOPIC_NO_MATCH`
  - `AZ_ERROR_IOT_END_OF_PROPERTIES`
- Moved these IoT specific log classifications from `az_log.h` to `az_iot_common.h`:
  - `AZ_LOG_MQTT_RECEIVED_TOPIC`
  - `AZ_LOG_MQTT_RECEIVED_PAYLOAD`
  - `AZ_LOG_IOT_RETRY`
  - `AZ_LOG_IOT_SAS_TOKEN`
  - `AZ_LOG_IOT_AZURERTOS`
- Removed `AZ_LOG_END_OF_LIST` log classification and `az_log_set_classifications()` from `az_log.h`.
- Renamed `az_log_set_callback()` to `az_log_set_message_callback()`.
- Removed `AZ_HTTP_STATUS_CODE_END_OF_LIST` HTTP status code and `status_codes` field from `az_http_policy_retry_options`.

### Bug Fixes

- Fixed [Pe188] warning from IAR when initializing structs using `{ 0 }`.

## 1.0.0-preview.5 (2020-09-08)

### New Features

- Add `az_json_writer_append_json_text()` to support appending existing JSON with the JSON writer.
- Add support for system properties for IoT Hub messages to `az_iot_common.h`.
- Add new HTTP result named `AZ_ERROR_HTTP_END_OF_HEADERS` to designate the end of the headers iterated over by `az_http_response_get_next_header()`.
- Add new IoT result named `AZ_ERROR_IOT_END_OF_PROPERTIES` to designate the end of the properties iterated over by `az_iot_message_properties_next()`.
- Add `AZ_IOT_MESSAGE_PROPERTIES_USER_ID` and `AZ_IOT_MESSAGE_PROPERTIES_CREATION_TIME` helper macros.
- Add new `az_result` value `AZ_ERROR_DEPENDENCY_NOT_PROVIDED` which is returned by the HTTP adapter.

### Breaking Changes

- Rename `az_iot_hub_client_properties` to `az_iot_message_properties` and move it from `az_iot_hub_client.h` to `az_iot_common.h`.
- Remove `az_pair`, and its usage from `az_http_request_append_header()`, `az_http_response_get_next_header()`, and `az_iot_message_properties_next()` in favor of individual name and value `az_span` parameters.
- Remove `az_credential_client_secret` structure, and `az_credential_client_secret_init()` function.
- Remove `az_platform_atomic_compare_exchange()` from platform.
- In `az_result.h`, rename `az_failed()` to `az_result_failed()` and `az_succeeded()` to `az_result_succeeded()`.
- `az_iot_is_success_status()` renamed to `az_iot_status_succeeded()`.
- `az_iot_is_retriable_status()` renamed to `az_iot_status_retriable()`.
- `az_iot_retry_calc_delay()` renamed to `az_iot_calculate_retry_delay()`.
- `az_iot_hub_client_sas_get_password()` parameter `token_expiration_epoch_time` moved to second parameter.
- `az_iot_provisioning_client_init()` parameter `global_device_endpoint` renamed to `global_device_hostname`.
- `az_iot_provisioning_client_query_status_get_publish_topic()` now accepts the `operation_id` from the `register_response` as the second parameter instead of the whole `az_iot_provisioning_client_register_response` struct.
- Renamed the macro `AZ_SPAN_NULL` to `AZ_SPAN_EMPTY`.
- Renamed the `az_result` value `AZ_ERROR_INSUFFICIENT_SPAN_SIZE` to `AZ_ERROR_NOT_ENOUGH_SPACE`.
- Removed the helper macros `AZ_RETURN_IF_FAILED()` and `AZ_RETURN_IF_NOT_ENOUGH_SIZE()` from `az_result.h`.
- Behavioral change to disallow passing `NULL` pointers to `az_context` APIs and update documentation.
- Removed `AZ_HUB_CLIENT_DEFAULT_MQTT_TELEMETRY_DUPLICATE` and `AZ_HUB_CLIENT_DEFAULT_MQTT_TELEMETRY_RETAIN` named constants from `az_iot_hub_client.h`.

### Bug Fixes

- Fix the strict-aliasing issue in `az_span_dtoa()` and `az_span_atod()`.
- Fix the SDK warnings for the release configurations.
- Do not use a shared static scratch buffer for JSON token parsing. Instead use stack space.

### Other Changes and Improvements

- Refactor and update IoT samples.
- Optimize the code size for URL encoding and setting HTTP query parameters.
- Add support for building the SDK on ARM (Cortex M4) and adding it to CI.

## 1.0.0-preview.4 (2020-08-10)

### New Features

- Support for writing JSON to non-contiguous buffers.
- Support for reading JSON from non-contiguous buffers.
- Add support for national cloud auth URLs.

### Breaking Changes

- `az_span.h`:
  - `az_span_init()` is renamed to `az_span_create()`.
  - `az_span_from_str()` is renamed to `az_span_create_from_str()`.
  - Removed `az_pair_from_str()`.
- `az_context`:
  - `key` and `value` are `const`.
  - `az_context_with_expiration()` is renamed to `az_context_create_with_expiration()`.
  - `az_context_with_value()` is renamed to `az_context_create_with_value()`.
  - `az_context_app` is renamed to `az_context_application`.
- `az_credential_client_secret_init()` now takes fourth parameter, `authority`.
- `az_http_policy_retry_options`:
  - `status_codes` now should be terminated by `AZ_HTTP_STATUS_CODE_END_OF_LIST`.
  - `max_retries` is now `int32_t` instead of `int16_t`.
- `az_config.h`:
  - `AZ_HTTP_REQUEST_URL_BUF_SIZE` renamed to `AZ_HTTP_REQUEST_URL_BUFFER_SIZE`.
  - `AZ_HTTP_REQUEST_BODY_BUF_SIZE` renamed to `AZ_HTTP_REQUEST_BODY_BUFFER_SIZE`.
  - `AZ_LOG_MSG_BUF_SIZE` renamed to `AZ_LOG_MESSAGE_BUFFER_SIZE`.
- `az_result`:
  - `AZ_ERROR_HTTP_PLATFORM` renamed to `AZ_ERROR_HTTP_ADAPTER`.
  - `AZ_ERROR_EOF` renamed to `AZ_ERROR_UNEXPECTED_END`.
  - Removed `AZ_CONTINUE`.
- `az_storage_blobs_blob_client`:
  - `retry` field renamed to `retry_options` in `az_storage_blobs_blob_client_options`.
  - Moved `az_context* context` parameter from `az_storage_blobs_blob_upload()` into a public field on `az_storage_blobs_blob_upload_options`.
- `az_json_writer`:
  - `az_json_writer_get_json()` is renamed to `az_json_writer_get_bytes_used_in_destination()`.

### Bug Fixes

- Remove support for non-finite double values while parsing/formatting.
- Use custom, portable implementation of IEEE 754 compliant `isfinite()` since some embedded platforms don't have it.
- Limit use of `sscanf` only to double parsing, using a custom implementation for {u}int{32|64} parsing because of incompatibility with `sscanf` format and the `GCC newlib-nano` implementation.

### Other Changes and Improvements

- Made `az_http_request` and related APIs to get URL, body, and headers, public.
- Add and update IoT samples, including DPS.
- Add samples for IoT Hub Plug and Play.

## 1.0.0-preview.3 (2020-07-20)

- Updated `az_result` values:
  - Rename `az_result` value `AZ_ERROR_PARSER_UNEXPECTED_CHAR` to `AZ_ERROR_UNEXPECTED_CHAR`.
  - Remove unused `az_result` error codes: `AZ_ERROR_JSON_STRING_END`, `AZ_ERROR_JSON_POINTER_TOKEN_END`, and `AZ_ERROR_MUTEX`.
- Add permutations of numeric type parsing and formatting APIs on span - ato[u|i][32|64], atod and the inverse [u|i][32|64]toa, dtoa.
- Updates to the JSON APIs:
  - Rename JSON parser/builder APIs to reader/writer.
  - Add double parsing and formatting support to JSON reader and JSON writer.
  - Redesign JSON reader and JSON token APIs with lazy evaluation of tokens, proper unescaping support, and hardened validation.
- Update samples, README docs along with deep dive video, and VSCode and CMake instructions.
  - Add PnP sample for Azure IoT Hub.
- Add log classification for the IoT convenience layer.
- Fixed SAS token generation by URL-encoding the components.
- Rename the http response function `az_http_response_write_span` to `az_http_response_append`.
- Add thread safety for client secret credential.
- Transform `apply_credential` into an HTTP policy.
- Default behavior for failed preconditions changed to infinite loop instead of thread sleep.

## 1.0.0-preview.2 (2020-05-18)

- Update top-level CMakeLists.txt to only add subdirectory for specified platform.
- Add compilation option to remove all logging from SDK code.

## 1.0.0-preview.1 (2020-05-12)

Initial release. Please see the [README](https://github.com/Azure/azure-sdk-for-c/blob/main/README.md) for more information.

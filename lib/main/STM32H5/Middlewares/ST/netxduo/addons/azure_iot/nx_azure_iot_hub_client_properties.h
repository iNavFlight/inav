/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* Version: 6.1 */

/**
 * @file nx_azure_iot_hub_client.h
 *
 * @brief Definition for the Azure IoT Hub client.
 * @remark The IoT Hub MQTT protocol is described at
 * https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-mqtt-support.
 *
 */

#ifndef NX_AZURE_IOT_HUB_CLIENT_PROPERTIES_H
#define NX_AZURE_IOT_HUB_CLIENT_PROPERTIES_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "azure/iot/az_iot_hub_client_properties.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_json_reader.h"
#include "nx_azure_iot_json_writer.h"

/* Property type.  */
#define NX_AZURE_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE       0
#define NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE                   1

/**
 * @brief Append the necessary characters to a reported property JSON payload belonging to a
 * subcomponent.
 *
 * The payload will be of the form:
 *
 * @code
 * "reported": {
 *     "<component_name>": {
 *         "__t": "c",
 *         "temperature": 23
 *     }
 * }
 * @endcode
 *
 * @note This API should be used in conjunction with
 * nx_azure_iot_hub_client_reported_properties_component_end().
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] writer_ptr A pointer to a #NX_AZURE_IOT_JSON_WRITER
 * @param[in] component_name_ptr A pointer to a component name
 * @param[in] component_name_length Length of `component_name_ptr`
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if JSON payload was prefixed successfully.
 */
UINT nx_azure_iot_hub_client_reported_properties_component_begin(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                 NX_AZURE_IOT_JSON_WRITER *writer_ptr,
                                                                 const UCHAR *component_name_ptr,
                                                                 USHORT component_name_length);

/**
 * @brief Append the necessary characters to end a reported property JSON payload belonging to a
 * subcomponent.
 *
 * @note This API should be used in conjunction with
 * nx_azure_iot_hub_client_reported_properties_component_begin().
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] writer_ptr A pointer to a #NX_AZURE_IOT_JSON_WRITER
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS The JSON payload was suffixed successfully.
 */
UINT nx_azure_iot_hub_client_reported_properties_component_end(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                               NX_AZURE_IOT_JSON_WRITER *writer_ptr);

/**
 * @brief Begin a property response payload with confirmation status.
 *
 * This API should be used in response to an incoming writable property. More details can be found
 * here:
 *
 * https://docs.microsoft.com/en-us/azure/iot-pnp/concepts-convention#writable-properties
 *
 * The payload will be of the form:
 *
 * **Without component**
 * @code
 * //{
 * //  "<property_name>":{
 * //    "ac": <status_code>,
 * //    "av": <version>,
 * //    "ad": "<description>",
 * //    "value": <user_value>
 * //  }
 * //}
 * @endcode
 *
 * To send a status for a property belonging to a component, first call the
 * nx_azure_iot_hub_client_reported_property_status_begin() API to prefix the payload with the
 * necessary identification. The API call flow would look like the following with the listed JSON
 * payload being generated.
 *
 * **With component**
 * @code
 *
 * nx_azure_iot_hub_client_reported_properties_component_begin()
 * nx_azure_iot_hub_client_reported_properties_status_begin()
 * // Append user value here (<user_value>)
 * nx_azure_iot_hub_client_reported_properties_status_end()
 * nx_azure_iot_hub_client_reported_properties_component_end()
 *
 * //{
 * //  "<component_name>": {
 * //    "__t": "c",
 * //    "<property_name>": {
 * //      "ac": <status_code>,
 * //      "av": <version>,
 * //      "ad": "<description>",
 * //      "value": <user_value>
 * //    }
 * //  }
 * //}
 * @endcode
 *
 * @note This API should be used in conjunction with
 * nx_azure_iot_hub_client_reported_properties_status_end().
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] writer_ptr A pointer to a #NX_AZURE_IOT_JSON_WRITER
 * @param[in] property_name_ptr A pointer to property name.
 * @param[in] property_name_length Length of `property_name_ptr`.
 * @param[in] status_code The HTTP-like status code to respond with.
 * @param[in] version The version of the property the application is acknowledging.
 * This can be retrieved from the service request by calling nx_azure_iot_hub_client_properties_version_get.
 * @param[in] description_ptr An optional pointer to description detailing the context or any details about
 *            the acknowledgement. This can be empty string.
 * @param[in] description_length Length of description_ptr
 *
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful appended JSON prefix.
 */
UINT nx_azure_iot_hub_client_reported_properties_status_begin(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                              NX_AZURE_IOT_JSON_WRITER *writer_ptr,
                                                              const UCHAR *property_name_ptr, UINT property_name_length,
                                                              UINT status_code, ULONG version,
                                                              const UCHAR *description_ptr, UINT description_length);

/**
 * @brief End a property response payload with confirmation status.
 *
 * @note This API should be used in conjunction with
 * nx_azure_iot_hub_client_reported_properties_status_begin().
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] writer_ptr A pointer to a #NX_AZURE_IOT_JSON_WRITER
 *
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful appended JSON suffix.
 */
UINT nx_azure_iot_hub_client_reported_properties_status_end(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            NX_AZURE_IOT_JSON_WRITER *writer_ptr);


/**
 * @brief Get the property version. the next writable property in the property document passed.
 *
 * @warning This modifies the state of the json reader. To then use the same json reader
 * with nx_azure_iot_hub_client_properties_component_property_next_get(), you must call
 * nx_azure_iot_json_reader_init() again after this call and before the call to
 * nx_azure_iot_hub_client_properties_component_property_next_get() or make an additional copy before
 * calling this API.
 *
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] reader_ptr A pointer to a #NX_AZURE_IOT_JSON_READER containing properties document
 * @param[in] message_type Type of message repsonse, only valid value are NX_AZURE_IOT_HUB_PROPERTIES or NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES
 * @param[out] version_ptr The numeric version of the properties in JSON payload
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if next writable property is found.
 */
UINT nx_azure_iot_hub_client_properties_version_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                    NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                    UINT message_type, ULONG *version_ptr);

/**
 * @brief Iteratively read the Azure IoT Plug and Play component properties.
 * 
 * Note that between calls, the UCHAR* pointed to by \p component_name_pptr shall not be modified,
 * only checked and compared. Internally, the pointer is only changed if the component name changes
 * in the JSON document and is not necessarily set every invocation of the function.
 *
 * On success, the `reader_ptr` will be set on a valid property name. After checking the
 * property name, the reader can be advanced to the property value by calling
 * nx_azure_iot_json_reader_next_token(). Note that on the subsequent call to this API, it is expected that
 * the json reader will be placed AFTER the read property name and value. That means that after
 * reading the property value (including single values or complex objects), the user must call
 * nx_azure_iot_json_reader_next_token().
 *
 * Below is a code snippet which you can use as a starting point:
 *
 * @code
 *
 * while ((status = nx_azure_iot_hub_client_properties_component_property_next_get(&iothub_client,
 *                                                                                 &json_reader,
 *                                                                                 message_type,
 *                                                                                 NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE,
 *                                                                                 &component_name_ptr, &component_length)) == NX_AZURE_IOT_SUCCESS)
 * {
 * 
 *     // Check if property is of interest (substitute user_property for your own property name)
 *     if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, user_property, user_property_length))
 *     {
 *         nx_azure_iot_json_reader_next_token(&json_reader);
 *
 *         // Get the property value here
 *         // Example: nx_azure_iot_json_reader_token_int32_get(&json_reader, &user_int); 
 *
 *         // Skip to next property value
 *         nx_azure_iot_json_reader_next_token(&json_reader);
 *     }
 *     else
 *     {
 *         // The JSON reader must be advanced regardless of whether the property
 *         // is of interest or not.
 *         nx_azure_iot_json_reader_next_token(&json_reader);
 *
 *         // Skip children in case the property value is an object
 *         nx_azure_iot_json_reader_skip_children(&json_reader);
 *         nx_azure_iot_json_reader_next_token(&json_reader);
 *     }
 * }
 *
 * @endcode
 * 
 * @warning If you need to retrieve more than one \p property_type, you should first complete the
 * scan of all components for the first property type (until the API returns
 * NX_AZURE_IOT_NOT_FOUND). Then you must call nx_azure_iot_json_reader_init() again after this call
 * and before the next call to `nx_azure_iot_hub_client_properties_component_property_next_get` with the
 * different \p property_type.
 * 
 * @param[in] hub_client_ptr A pointer to a #NX_AZURE_IOT_HUB_CLIENT.
 * @param[in] reader_ptr A pointer to a #NX_AZURE_IOT_JSON_READER containing properties document
 * @param[in] message_type Type of message repsonse, only valid value are NX_AZURE_IOT_HUB_PROPERTIES or NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES
 * @param[in] property_type Type of property, only valid value are NX_AZURE_IOT_HUB_CLIENT_PROPERTY_REPORTED_FROM_DEVICE or NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE
 * @param[out] component_name_pptr A pointer to component name for the property returned using reader_ptr
 * @param[out] component_name_length_ptr Length of the component name
 * @return A `UINT` with the result of the API.
 *   @retval #NX_AZURE_IOT_SUCCESS Successful if next writable property is found.
 */
UINT nx_azure_iot_hub_client_properties_component_property_next_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                    NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                                    UINT message_type, UINT property_type,
                                                                    const UCHAR **component_name_pptr,
                                                                    USHORT *component_name_length_ptr);

#ifdef __cplusplus
}
#endif
#endif /* NX_AZURE_IOT_HUB_CLIENT_PROPERTIES_H */

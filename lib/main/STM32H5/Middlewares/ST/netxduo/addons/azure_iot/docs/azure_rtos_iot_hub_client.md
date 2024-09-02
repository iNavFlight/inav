# Azure IoT Hub Client

## APIs:

### Connect

* [nx_azure_iot_hub_client_initialize](#nx_azure_iot_hub_client_initialize)
* [nx_azure_iot_hub_client_deinitialize](#nx_azure_iot_hub_client_deinitialize)
* [nx_azure_iot_hub_client_model_id_set](#nx_azure_iot_hub_client_model_id_set)
* [nx_azure_iot_hub_client_component_add](#nx_azure_iot_hub_client_component_add)
* [nx_azure_iot_hub_client_trusted_cert_add](#nx_azure_iot_hub_client_trusted_cert_add)
* [nx_azure_iot_hub_client_device_cert_set](#nx_azure_iot_hub_client_device_cert_set)
* [nx_azure_iot_hub_client_symmetric_key_set](#nx_azure_iot_hub_client_symmetric_key_set)
* [nx_azure_iot_hub_client_websocket_enable](#nx_azure_iot_hub_client_websocket_enable)
* [nx_azure_iot_hub_client_connect](#nx_azure_iot_hub_client_connect)
* [nx_azure_iot_hub_client_disconnect](#nx_azure_iot_hub_client_disconnect)
* [nx_azure_iot_hub_client_connection_status_callback_set](#nx_azure_iot_hub_client_connection_status_callback_set)
* [nx_azure_iot_hub_client_receive_callback_set](#nx_azure_iot_hub_client_receive_callback_set)

### C2D

* [nx_azure_iot_hub_client_cloud_message_enable](#nx_azure_iot_hub_client_cloud_message_enable)
* [nx_azure_iot_hub_client_cloud_message_disable](#nx_azure_iot_hub_client_cloud_message_disable)
* [nx_azure_iot_hub_client_cloud_message_receive](#nx_azure_iot_hub_client_cloud_message_receive)
* [nx_azure_iot_hub_client_cloud_message_property_get](#nx_azure_iot_hub_client_cloud_message_property_get)

### Telemetry

* [nx_azure_iot_hub_client_telemetry_message_create](#nx_azure_iot_hub_client_telemetry_message_create)
* [nx_azure_iot_hub_client_telemetry_message_delete](#nx_azure_iot_hub_client_telemetry_message_delete)
* [nx_azure_iot_hub_client_telemetry_component_set](#nx_azure_iot_hub_client_telemetry_component_set)
* [nx_azure_iot_hub_client_telemetry_property_add](#nx_azure_iot_hub_client_telemetry_property_add)
* [nx_azure_iot_hub_client_telemetry_send](#nx_azure_iot_hub_client_telemetry_send)

### Command

* [nx_azure_iot_hub_client_command_enable](#nx_azure_iot_hub_client_command_enable)
* [nx_azure_iot_hub_client_command_disable](#nx_azure_iot_hub_client_command_disable)
* [nx_azure_iot_hub_client_command_message_receive](#nx_azure_iot_hub_client_command_message_receive)
* [nx_azure_iot_hub_client_command_message_response](#nx_azure_iot_hub_client_command_message_response)

### Direct Method

* [nx_azure_iot_hub_client_direct_method_enable](#nx_azure_iot_hub_client_direct_method_enable)
* [nx_azure_iot_hub_client_direct_method_disable](#nx_azure_iot_hub_client_direct_method_disable)
* [nx_azure_iot_hub_client_direct_method_message_receive](#nx_azure_iot_hub_client_direct_method_message_receive)
* [nx_azure_iot_hub_client_direct_method_message_response](#nx_azure_iot_hub_client_direct_method_message_response)

### Properties

* [nx_azure_iot_hub_client_properties_enable](#nx_azure_iot_hub_client_properties_enable)
* [nx_azure_iot_hub_client_properties_disable](#nx_azure_iot_hub_client_properties_disable)
* [nx_azure_iot_hub_client_reported_properties_response_callback_set](#nx_azure_iot_hub_client_reported_properties_response_callback_set)
* [nx_azure_iot_hub_client_reported_properties_create](#nx_azure_iot_hub_client_reported_properties_create)
* [nx_azure_iot_hub_client_reported_properties_send](#nx_azure_iot_hub_client_reported_properties_send)
* [nx_azure_iot_hub_client_properties_request](#nx_azure_iot_hub_client_properties_request)
* [nx_azure_iot_hub_client_properties_receive](#nx_azure_iot_hub_client_properties_receive)
* [nx_azure_iot_hub_client_writable_properties_receive](#nx_azure_iot_hub_client_writable_properties_receive)

### Device Twin

* [nx_azure_iot_hub_client_device_twin_enable](#nx_azure_iot_hub_client_device_twin_enable)
* [nx_azure_iot_hub_client_device_twin_disable](#nx_azure_iot_hub_client_device_twin_disable)
* [nx_azure_iot_hub_client_device_twin_reported_properties_send](#nx_azure_iot_hub_client_device_twin_reported_properties_send)
* [nx_azure_iot_hub_client_device_twin_properties_request](#nx_azure_iot_hub_client_device_twin_properties_request)
* [nx_azure_iot_hub_client_device_twin_properties_receive](#nx_azure_iot_hub_client_device_twin_properties_receive)
* [nx_azure_iot_hub_client_device_twin_desired_properties_receive](#nx_azure_iot_hub_client_device_twin_desired_properties_receive)

#### **nx_azure_iot_hub_client_initialize**
***
<div style="text-align: right"> Initialize Azure IoT hub instance</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_initialize(NX_AZURE_IOT_HUB_CLIENT* hub_client_ptr,
                                        NX_AZURE_IOT *nx_azure_iot_ptr,
                                        const UCHAR *host_name, UINT host_name_length,
                                        const UCHAR *device_id, UINT device_id_length,
                                        const UCHAR *module_id, UINT module_id_length,
                                        const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                        const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                        UCHAR * metadata_memory, UINT memory_size,
                                        NX_SECURE_X509_CERT *trusted_certificate);
```
**Description**

<p>This routine initializes the IoT Hub client.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| nx_azure_iot_ptr [in]      | A pointer to a `NX_AZURE_IOT`.|
| host_name [in] | A pointer to IoTHub hostname. Must be NULL terminated string.   |
| host_name_length [in] | Length of the IoTHub hostname.  |
| device_id [in]  | A pointer to device ID.     |
| device_id_length [in] | Length of the device ID. |
| module_id [in]  | A pointer to module ID.     |
| module_id_length [in] | Length of the module ID. |
| crypto_array [in] | A pointer to `NX_CRYPTO_METHOD`    |
| crypto_array_size [in] | Size of crypto method array   |
| cipher_map [in] | A pointer to `NX_CRYPTO_CIPHERSUITE`    |
| cipher_map_size [in] | Size of cipher map    |
| metadata_memory [in] | A pointer to metadata memory buffer. |
| memory_size [in]  | Size of metadata buffer     |
| trusted_certificate [in] | A pointer to `NX_SECURE_X509_CERT`, which is server side certs |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully initialized the Azure IoT hub.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to initialize the Azure IoT hub client due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to initialize the Azure IoT hub client due to SDK core error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_deinitialize**
***
<div style="text-align: right"> Cleanup the Azure IoT Hub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_deinitialize(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>The routine deinitializes the IoT Hub client</p>

**Parameters**
|               |               |
| ------------- |:-------------|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully de-initialized the Azure IoT hub client.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to deinitialize the Azure IoT hub client due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_trusted_cert_add**
***
<div style="text-align: right"> Add trusted certificate </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_trusted_cert_add(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                              NX_SECURE_X509_CERT *trusted_certificate);
```
**Description**

<p>This routine adds the trusted certificate. It can be called multiple times to set multiple trusted certificates.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| trusted_certificate [in]    | A pointer to a `NX_SECURE_X509_CERT` |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully add trusted certificate to Azure IoT Hub Instance.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to add trusted certificate to Azure IoT Hub Instance due to invalid parameter.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to add trusted certificate due to `NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS` is too small.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_cert_set**
***
<div style="text-align: right"> Set client certificate </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_cert_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                             NX_SECURE_X509_CERT *device_certificate);
```
**Description**

<p>This routine sets the device certificate. It can be called multiple times to set certificate chain.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| device_certificate [in]    | A pointer to a `NX_SECURE_X509_CERT` |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set device certificate to Azure IoT Hub Instance.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set device certificate to Azure IoT Hub Instance due to invalid parameter.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to set device certificate due to `NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS` is too small.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_symmetric_key_set**
***
<div style="text-align: right"> Set symmetric key </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_symmetric_key_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                               const UCHAR *symmetric_key, UINT symmetric_key_length);
```
**Description**

<p>This routine sets the symmetric key.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| symmetric_key [in]    | A pointer to a symmetric key. |
| symmetric_key_length [in]    | Length of symmetric key |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set symmetric key to IoTHub client.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set symmetric key to IoTHub client due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_model_id_set**
***
<div style="text-align: right"> Set Device Twin model id in the IoT Hub client. </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_model_id_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                          const UCHAR *model_id_ptr, UINT model_id_length);
```
**Description**

<p>This routine sets the model id in the IoT Hub client to enable PnP.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| model_id_ptr [in]    | A pointer to a model id. |
| model_id_length [in]    | Length of model id |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set model id to IoTHub client.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set model id to IoTHub client due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_component_add**
***
<div style="text-align: right"> Add component to IoT Hub client </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_component_add(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                           const UCHAR *component_name_ptr,
                                           USHORT component_name_length);
```
**Description**

<p>This routine should be called for all the components in the IoT hub model.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| component_name_ptr [in]    | A pointer to component, that is part of IoT hub model. |
| component_name_length [in]    | Length of the `component_name_ptr`. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set device certificate to AZ IoT Hub Instance.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to add the component name due to out of memory.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_websocket_enable**
***
<div style="text-align: right"> Enables MQTT over WebSocket to connect to IoT Hub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_websocket_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine enables MQTT over WebSocket to connect to the Azure IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if MQTT over Websocket is enabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to enable MQTT over WebSocket due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_connect**
***
<div style="text-align: right"> Connects to IoT Hub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_connect(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                     UINT clean_session, UINT wait_option);
```
**Description**

<p>This routine connects to the Azure IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| clean_session [in]    | 0 re-use current session, or 1 to start new session |
| wait_option [in]    | Number of ticks to wait for internal resources to be available. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if connected to Azure IoT Hub.
* NX_AZURE_IOT_CONNECTING Successfully started connection but not yet completed.
* NX_AZURE_IOT_ALREADY_CONNECTED Already connected to Azure IoT Hub.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to connect to Azure IoT Hub due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to connect to Azure IoT Hub due to SDK core error.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to connect to Azure IoT Hub due to buffer size is too small.
* NX_DNS_QUERY_FAILED Fail to connect to Azure IoT Hub due to hostname can not be resolved.
* NX_NO_PACKET Fail to connect to Azure IoT Hub due to no available packet in pool.
* NX_INVALID_PARAMETERS Fail to connect to Azure IoT Hub due to invalid parameters.
* NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE Fail to connect to Azure IoT Hub due to insufficient metadata space.
* NX_SECURE_TLS_UNSUPPORTED_CIPHER Fail to connect to Azure IoT Hub due to unsupported cipher.
* NXD_MQTT_ALREADY_CONNECTED Fail to connect to Azure IoT Hub due to MQTT session is not disconnected.
* NXD_MQTT_CONNECT_FAILURE Fail to connect to Azure IoT Hub due to TCP/TLS connect error.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to connect to Azure IoT Hub due to MQTT connect error.
* NXD_MQTT_ERROR_SERVER_UNAVAILABLE Fail to connect to Azure IoT Hub due to server unavailable.
* NXD_MQTT_ERROR_NOT_AUTHORIZED Fail to connect to Azure IoT Hub due to authentication error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_disconnect**
***
<div style="text-align: right"> Disconnects the client</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_disconnect(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine disconnects the client.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if client disconnects.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to disconnect due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_connection_status_callback_set**
***
<div style="text-align: right"> Sets connection status callback function</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_connection_status_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            VOID (*connection_status_cb)(
                                                                 struct NX_AZURE_IOT_HUB_CLIENT_STRUCT
                                                                 *hub_client_ptr, UINT status));
```
**Description**

<p>This routine sets the connection status callback. This callback function is invoked when the IoT Hub status is changed, such as: return NX_AZURE_IOT_SUCCESS status once client is connected to IoT Hub. Setting the callback function to NULL disables the callback function. Following status will be returned on disconnection.</p>

* NX_SECURE_TLS_ALERT_RECEIVED
* NX_SECURE_TLS_NO_SUPPORTED_CIPHERS
* NX_SECURE_X509_CHAIN_VERIFY_FAILURE
* NXD_MQTT_CONNECT_FAILURE
* NXD_MQTT_ERROR_SERVER_UNAVAILABLE
* NXD_MQTT_ERROR_NOT_AUTHORIZED
* NX_AZURE_IOT_DISCONNECTED
* NX_AZURE_IOT_SAS_TOKEN_EXPIRED

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |
| connection_status_cb [in]    | Pointer to a callback function invoked once connection status is changed. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if connection status callback is set.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set connection status callback due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_receive_callback_set**
***
<div style="text-align: right"> Sets receive callback function</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_receive_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                  UINT message_type,
                                                  VOID (*callback_ptr)(
                                                        NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *args),
                                                  VOID *callback_args);
```
**Description**

<p>This routine sets the IoT Hub receive callback function. This callback function is invoked when a message is received from Azure IoT hub. Setting the callback function to NULL disables the callback function. Message types can be NX_AZURE_IOT_HUB_CLOUD_TO_DEVICE_MESSAGE, NX_AZURE_IOT_HUB_COMMAND, NX_AZURE_IOT_HUB_PROPERTIES, NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES, NX_AZURE_IOT_HUB_DIRECT_METHOD, NX_AZURE_IOT_HUB_DEVICE_TWIN_PROPERTIES and NX_AZURE_IOT_HUB_DEVICE_TWIN_DESIRED_PROPERTIES. </p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| message_type [in]    | Message type of callback function. |
| callback_ptr [in]    | Pointer to a callback function invoked on specified message type is received. |
| callback_args [in]    | Pointer to an argument passed to callback function. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if callback function is set.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set receive callback due to invalid parameter.
* NX_AZURE_IOT_NOT_SUPPORTED Fail to set receive callback due to message_type not supported.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_telemetry_message_create**
***
<div style="text-align: right"> Creates telemetry message</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_telemetry_message_create(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET **packet_pptr,
                                                      UINT wait_option);
```
**Description**

<p>This routine prepares a packet for sending telemetry data. After the packet is properly created, application owns the `NX_PACKET` and can add additional user-defined properties before sending out.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | Return allocated packet on success. Caller owns the `NX_PACKET` memory. |
| wait_option [in]    | Ticks to wait if no packet is available. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if a packet is allocated.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to allocate telemetry message due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to allocate telemetry message due to SDK core error.
* NX_NO_PACKET Fail to allocate telemetry message due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_telemetry_message_delete**
***
<div style="text-align: right"> Deletes telemetry message</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_telemetry_message_delete(NX_PACKET *packet_ptr);
```
**Description**

<p>This routine deletes the telemetry message.</p>

**Parameters**

| Name | Description |
| - |:-|
| packet_ptr [in]    | Release the `NX_PACKET` on success. |


**Return Values**
* NX_AZURE_IOT_SUCCESS (0x0)  Successful if a packet is deallocated.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_telemetry_component_set**
***
<div style="text-align: right"> Sets component to telemetry message</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_telemetry_component_set(NX_PACKET *packet_ptr,
                                                     const UCHAR *component_name_ptr,
                                                     USHORT component_name_length,
                                                     UINT wait_option));
```
**Description**

<p>This routine allows an application to set a component name to a telemetry message before it is being sent. The component is stored in the sequence which the routine is called. The component must be set after a telemetry packet is created, and before the telemetry message is being sent.</p>

**Parameters**

| Name | Description |
| - |:-|
| packet_ptr [in]    | A pointer to telemetry component packet. |
| component_name_ptr [in]    |  A pointer to a component name. |
| component_name_length [in]    | Length of component name. |
| wait_option [in]    | Ticks to wait if packet needs to be expanded. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if component is set.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set component due to invalid parameter.
* NX_NO_PACKET Fail to set component due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_telemetry_property_add**
***
<div style="text-align: right"> Adds property to telemetry message</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_telemetry_property_add(NX_PACKET *packet_ptr,
                                                    const UCHAR *property_name, USHORT property_name_length,
                                                    const UCHAR *property_value, USHORT property_value_length,
                                                    UINT wait_option);
```
**Description**

<p>This routine allows an application to add user-defined properties to a telemetry message before it is being sent. This routine can be called multiple times to add all the properties to the message. The properties are stored in the sequence which the routine is called. The property must be added after a telemetry packet is created, and before the telemetry message is being sent.</p>

**Parameters**

| Name | Description |
| - |:-|
| packet_ptr [in]    | A pointer to telemetry property packet. |
| property_name [in]    | Pointer to property name. |
| property_name_length [in]    | Length of property name. |
| property_value [in]    | Pointer to property value. |
| property_value_length [in]    | Length of property value. |
| wait_option [in]    | Ticks to wait if packet needs to be expanded. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if property is added.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to add property due to invalid parameter.
* NX_NO_PACKET Fail to add property due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_telemetry_send**
***
<div style="text-align: right"> Sends telemetry message to IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_telemetry_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, NX_PACKET *packet_ptr,
                                            const UCHAR *telemetry_data, UINT data_size, UINT wait_option);
```
**Description**

<p>This routine sends telemetry to IoTHub, with packet_ptr containing all the properties. On successful return of this function, ownership of `NX_PACKET` is released.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_ptr [in]    | A pointer to telemetry property packet. |
| telemetry_data [in]    | Pointer to telemetry data. |
| data_size [in]    | Size of telemetry data. |
| wait_option [in]    | Ticks to wait for message to be sent. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if telemetry message is sent out.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to send telemetry message due to invalid parameter.
* NX_AZURE_IOT_INVALID_PACKET Fail to send telemetry message due to packet is invalid.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to send telemetry message due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to send telemetry message due to TCP/TLS error.
* NX_NO_PACKET Fail to send telemetry message due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_cloud_message_enable**
***
<div style="text-align: right"> Enables receiving C2D message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_cloud_message_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine enables receiving C2D message from IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if C2D message receiving is enabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to enable C2D message receiving due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to enable C2D message receiving due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to enable C2D message receiving due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to enable C2D message receiving due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_cloud_message_disable**
***
<div style="text-align: right"> Disables receiving C2D message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_cloud_message_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine disables receiving C2D message from IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if C2D message receiving is disabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to disable C2D message receiving due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to disable C2D message receiving due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to disable C2D message receiving due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to disable C2D message receiving due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_cloud_message_receive**
***
<div style="text-align: right"> Receives C2D message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_cloud_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                   NX_PACKET **packet_pptr,
                                                   UINT wait_option);
```
**Description**

<p>This routine receives C2D message from IoT Hub. If there are no messages in the receive queue, this routine can block. The amount of time it waits for a message is determined by the wait_option parameter.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | Return a packet pointer with C2D message on success. Caller owns the `NX_PACKET` memory. |
| wait_option [in]    | Ticks to wait for message to arrive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if C2D message is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive C2D message due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive C2D message due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive C2D message due to timeout.
* NX_AZURE_IOT_DISCONNECTED Fail to receive C2D message due to disconnection.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_cloud_message_property_get**
***
<div style="text-align: right"> Retrieve the property with given property name in the C2D message</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_cloud_message_property_get(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, NX_PACKET *packet_ptr,
                                                        const UCHAR *property_name, USHORT property_name_length,
                                                        const UCHAR **property_value, USHORT *property_value_length);
```
**Description**

<p>This routine retrieves the property with given property name in the NX_PACKET.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_ptr [in]    | Pointer to `NX_PACKET` containing C2D message. |
| property_name [in]    | Pointer to property name. |
| property_name_length [in]    | Property name length. |
| property_value [out]    | Pointer to memory that contains property value |
| property_value_length [out]    | Pointer to size of property value. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if property is found and copied successfully into user buffer.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to find the property due to invalid parameter.
* NX_AZURE_IOT_INVALID_PACKET Fail to find the property due to the packet is invalid.
* NX_AZURE_IOT_NOT_FOUND Property is not found.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to find the property due to parsing error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_direct_method_enable**
***
<div style="text-align: right"> Enables receiving direct method messages from IoTHub </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_direct_method_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

```
**Description**

<p>This routine enables receiving direct method messages from IoT Hub. </p>

**Parameters**
| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if direct method message receiving is enabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to enable direct method message receiving due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to enable direct method message receiving due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to enable direct method message receiving due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to enable direct method message receiving due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_direct_method_disable**
***
<div style="text-align: right"> Disables receiving direct method messages from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_direct_method_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine disables receiving direct method messages from IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if direct method message receiving is disabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to disable direct method message receiving due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to disable direct method message receiving due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to disable direct method message receiving due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to disable direct method message receiving due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_direct_method_message_receive**
***
<div style="text-align: right"> Receives direct method message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_direct_method_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                           const UCHAR **method_name_pptr, USHORT *method_name_length_ptr,
                                                           VOID **context_pptr, USHORT *context_length_ptr,
                                                           NX_PACKET **packet_pptr, UINT wait_option);
```
**Description**

<p>This routine receives direct method message from IoT Hub. If there are no messages in the receive queue, this routine can block. The amount of time it waits for a message is determined by the wait_option parameter.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| method_name_pptr [out]    | Return a pointer to method name on success. |
| method_name_length_ptr [out]    | Return length of method name on success. |
| context_pptr [out]    | Return a pointer to context pointer on success. |
| context_length_ptr [out]    | Return length of context on success. |
| packet_pptr [out]    | Return `NX_PACKET` containing the method payload on success. Caller owns the `NX_PACKET` memory. |
| wait_option [in]    | Ticks to wait for message to arrive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if direct method message is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive direct method message due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive direct method message due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive direct method message due to timeout.
* NX_AZURE_IOT_INVALID_PACKET Fail to receive direct method message due to invalid packet.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive direct method message due to SDK core error.
* NX_AZURE_IOT_DISCONNECTED Fail to receive direct method message due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_direct_method_message_response**
***
<div style="text-align: right"> Return response to direct method message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_direct_method_message_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT status_code, VOID *context_ptr, USHORT context_length,,
                                                            const UCHAR *payload, UINT payload_length, UINT wait_option);
```
**Description**

<p>This routine returns response to the direct method message from IoT Hub. Note: request_id ties the correlation between direct method receive and response.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| status_code [in]    | Status code for direct method. |
| context_ptr [in]    | Pointer to context return from nx_azure_iot_hub_client_direct_method_message_receive. |
| context_length [in]    | Length of context. |
| payload [in]    | Pointer to `UCHAR` containing the payload for the direct method response. Payload is in JSON format. |
| payload_length [in]    | Length of the payload |
| wait_option [in]    | Ticks to wait for message to send. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if direct method response is send.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to send direct method response due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to send direct method response due to SDK core error.
* NX_NO_PACKET Fail send direct method response due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_command_enable**
***
<div style="text-align: right"> Enables receiving command messages from IoTHub </div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_command_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);

```
**Description**

<p>This routine enables receiving command messages from IoT Hub. </p>

**Parameters**
| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT` |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if command message receiving is enabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to enable command message receiving due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to enable command message receiving due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to enable command message receiving due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to enable command message receiving due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_command_disable**
***
<div style="text-align: right"> Disables receiving command messages from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_command_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine disables receiving command messages from IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if command message receiving is disabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to disable command message receiving due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to disable command message receiving due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to disable command message receiving due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to disable command message receiving due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_command_message_receive**
***
<div style="text-align: right"> Receives IoT Hub command message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_command_message_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                     const UCHAR **component_name_pptr, 
                                                     USHORT *component_name_length_ptr,
                                                     const UCHAR **pnp_command_name_pptr, 
                                                     USHORT *pnp_command_name_length_ptr,
                                                     VOID **context_pptr, 
                                                     USHORT *context_length_ptr,
                                                     NX_PACKET **packet_pptr, 
                                                     UINT wait_option);
```
**Description**

<p>This routine receives IoT Hub command message from IoT Hub. If there are no messages in the receive queue, this routine can block.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| component_name_pptr [out]    | Return a pointer to IoT Hub component name on success. |
| component_name_length_ptr [out]    | Return length of `*component_name_pptr` on success. |
| pnp_command_name_pptr [out]    | Return a pointer to IoT Hub command name on success. |
| pnp_command_name_length_ptr [out]    | Return length of `*pnp_command_name_pptr` on success. |
| context_pptr [out]    | Return a pointer to context pointer on success. |
| context_length_ptr [out]    | Return length of context on success. |
| packet_pptr [out]    | Return `NX_PACKET` containing the command payload on success. Caller owns the `NX_PACKET` memory. |
| wait_option [in]    | Ticks to wait for message to arrive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if IoT Hub command message is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive IoT Hub command message due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive IoT Hub command message due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive IoT Hub command message due to timeout.
* NX_AZURE_IOT_INVALID_PACKET Fail to receive IoT Hub command message due to invalid packet.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive IoT Hub command message due to SDK core error.
* NX_AZURE_IOT_DISCONNECTED Fail to receive IoT Hub command message due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_command_message_response**
***
<div style="text-align: right"> Return response to IoT hub command message from IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_command_message_response(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      UINT status_code, VOID *context_ptr,
                                                      USHORT context_length, const UCHAR *payload_ptr,
                                                      UINT payload_length, UINT wait_option);
```
**Description**

<p>This routine returns response to the IoT hub command message from IoT Hub. Note: request_id ties the correlation between command receive and response.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| status_code [in]    | Status code for pnp command message. |
| context_ptr [in]    | Pointer to context return from nx_azure_iot_hub_client_command_receive. |
| context_length [in]    | Length of context. |
| payload [in]    | Pointer to `UCHAR` containing the payload for the IoT Hub command response. Payload is in JSON format. |
| payload_length [in]    | Length of the payload |
| wait_option [in]    | Ticks to wait for message to send. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if IoT Hub command response is send.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to send IoT Hub command response due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to send IoT Hub command response due to SDK core error.
* NX_NO_PACKET Fail send IoT Hub command response due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_twin_enable**
***
<div style="text-align: right">Enables device twin feature</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_twin_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine enables device twin feature.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if device twin feature is enabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to enable device twin feature due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to enable device twin feature due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to enable device twin feature due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to enable device twin feature due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_twin_disable**
***
<div style="text-align: right">Disables device twin feature</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_twin_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine disables device twin feature.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if device twin feature is disabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to disable device twin feature due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to disable device twin feature due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to disable device twin feature due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to disable device twin feature due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_reported_properties_response_callback_set**
***
<div style="text-align: right">Sets reported properties response callback function</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_reported_properties_response_callback_set(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                       VOID (*callback_ptr)(
                                                                             NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                             UINT request_id,
                                                                             UINT response_status,
                                                                             ULONG version,
                                                                             VOID *args),
                                                                       VOID *callback_args);
```
**Description**

<p>This routine sets the response receive callback function for reported properties. This callback function is invoked when a response is received from Azure IoT hub for reported properties and no  thread is waiting for response. Setting the callback function to NULL disables the callback function.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| callback_ptr [in]    | Pointer to a callback function invoked. |
| callback_args [in]    | Pointer to an argument passed to callback function. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if callback function is set.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set callback due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_twin_reported_properties_send**
***
<div style="text-align: right">Send device twin reported properties to IoT Hub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_twin_reported_properties_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                  const UCHAR *message_buffer, UINT message_length,
                                                                  UINT *request_id_ptr, UINT *response_status_ptr,
                                                                  ULONG *version_ptr, UINT wait_option);
```
**Description**

<p>This routine sends device twin reported properties to IoT Hub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| message_buffer [in]    | JSON document containing the reported properties. |
| message_length [in]    | Length of JSON document. |
| request_id_ptr [out]    |  Request Id assigned to the request. |
| response_status_ptr [out]    | Status return for successful send of reported properties.|
| version_ptr [out]    | Version return for successful send of reported properties.|
| wait_option [in]    | Ticks to wait for message to send. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if device twin reported properties is sent.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to send reported properties due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to send reported properties due to device twin is not enabled.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to send reported properties due to SDK core error.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to send reported properties due to buffer size is too small.
* NX_AZURE_IOT_NO_PACKET Fail to send reported properties due to no packet available.
* NX_NO_PACKET Fail to send reported properties due to no packet available.
* NX_AZURE_IOT_DISCONNECTED Fail to send reported properties due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_twin_properties_request**
***
<div style="text-align: right">Request complete device twin properties</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_twin_properties_request(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            UINT wait_option);
```
**Description**

<p>This routine requests complete device twin properties.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| wait_option [in]    | Ticks to wait for to wait for sending request. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if device twin properties is requested.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to send device twin request due to invalid parameter.
* NX_AZURE_IOT_NO_SUBSCRIBE_ACK Fail to send device twin request due to no subscribe ack.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to send device twin request due to SDK core error.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to send device twin request due to buffer size is too small.
* NX_AZURE_IOT_NO_PACKET Fail to send reported properties due to no packet available.
* NX_NO_PACKET Fail to send reported properties due to no packet available.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_twin_properties_receive**
***
<div style="text-align: right">Receive complete device twin properties</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_twin_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                            NX_PACKET **packet_pptr, UINT wait_option);
```
**Description**

<p>This routine receives complete device twin properties.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | Pointer to `NX_PACKET*` that contains complete device twin properties. Caller owns the `NX_PACKET` memory. |
| wait_option [in]    | Ticks to wait for message to receive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if device twin properties is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive device twin properties due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive device twin properties due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive device twin properties due to timeout.
* NX_AZURE_IOT_INVALID_PACKET Fail to receive device twin properties due to invalid packet.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive device twin properties due to SDK core error.
* NX_AZURE_IOT_SERVER_RESPONSE_ERROR Response code from server is not 2xx.
* NX_AZURE_IOT_DISCONNECTED Fail to receive device twin properties due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_device_twin_desired_properties_receive**
***
<div style="text-align: right">Receive desired properties form IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_device_twin_desired_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                    NX_PACKET **packet_pptr, UINT wait_option);
```
**Description**

<p>This routine receives desired properties from IoTHub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | Pointer to `NX_PACKET*` that contains complete twin document. Caller owns the `NX_PACKET` memory. |
| wait_option [in]    | Ticks to wait for message to receive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if desired properties is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive desired properties due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive desired properties due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive desired properties due to timeout.
* NX_AZURE_IOT_INVALID_PACKET Fail to receive desired properties due to invalid packet.
* NX_AZURE_IOT_DISCONNECTED Fail to receive desired properties due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_properties_enable**
***
<div style="text-align: right">Enables properties feature</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_properties_enable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine enables properties feature.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if properties feature is enabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to enable properties feature due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to enable properties feature due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to enable properties feature due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to enable properties feature due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_properties_disable**
***
<div style="text-align: right">Disables properties feature</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_properties_disable(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
```
**Description**

<p>This routine disables properties feature.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if properties feature is disabled.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to disable properties feature due to invalid parameter.
* NXD_MQTT_NOT_CONNECTED Fail to disable properties feature due to MQTT not connected.
* NXD_MQTT_PACKET_POOL_FAILURE Fail to disable properties feature due to no available packet in pool.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to disable properties feature due to TCP/TLS error.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_reported_properties_create**
***
<div style="text-align: right">Creates IoT Hub reported property message.</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_reported_properties_create(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                        NX_PACKET **packet_pptr,
                                                        UINT wait_option)
```
**Description**

<p>This routine creates a reported properties message.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | Return allocated packet on success. |
| wait_option [in]    | Ticks to wait for writer creation. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if a message writer is created.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to create message writer due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to create message writer due to SDK core error.
* NX_NO_PACKET Fail to create message writer due to no available packet in pool.

**Allowed From**

Threads

**Example**

**See Also**


<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_reported_properties_send**
***
<div style="text-align: right">Sends IoT Hub reported properties message to IoTHub.</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_reported_properties_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                      NX_PACKET *packet_ptr,
                                                      UINT *request_id_ptr, UINT *response_status_ptr,
                                                      ULONG *version_ptr, UINT wait_option);
```
**Description**

<p>This routine sends the reported properties contain in the packet.</p>
<p>Note: The return status of the API indicates if the reported properties is sent out successfully or not,
the response status is used to track if the reported properties is accepted or not by IoT Hub, and the
reponse status is available only when the return status is NX_AZURE_IOT_SUCCESS.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_ptr [in]    | A pointer to a #NX_PACKET. |
| request_id_ptr [out]    | Request Id assigned to the request. |
| response_status_ptr [out]    | Status return for successful send of reported properties. |
| version_ptr [out]    | Version return for successful send of reported properties. |
| wait_option [in]    | Ticks to wait for message to send. |


**Return Values**
 * NX_AZURE_IOT_SUCCESS Successful if reported properties is sent.
 * NX_AZURE_IOT_INVALID_PARAMETER Fail to send reported properties due to invalid parameter.
 * NX_AZURE_IOT_NOT_ENABLED Fail to send reported properties due to property is not enabled.
 * NX_AZURE_IOT_SDK_CORE_ERROR Fail to send reported properties due to SDK core error.
 * NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to send reported properties due to buffer size is too small.
 * NX_NO_PACKET Fail to send reported properties due to no packet available.
 * NX_AZURE_IOT_DISCONNECTED Fail to send reported properties due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_properties_request**
***
<div style="text-align: right">Request complete properties</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_properties_request(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                UINT wait_option);
```
**Description**

<p>This routine requests complete properties.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| wait_option [in]    | Ticks to wait for to wait for sending request. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if request get all properties is sent..
* NX_AZURE_IOT_INVALID_PARAMETER Fail to request get all properties due to invalid parameter.
* NX_AZURE_IOT_NO_SUBSCRIBE_ACK Fail to request get all properties due to no subscribe ack.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to request get all properties due to SDK core error.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to request get all properties due to buffer size is too small.
* NX_AZURE_IOT_NO_PACKET Fail to request get all properties due to no packet available.
* NX_NO_PACKET Fail to request get all properties due to no packet available.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_properties_receive**
***
<div style="text-align: right">Receive all the properties</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                NX_PACKET **packet_pptr,
                                                UINT wait_option);
```
**Description**

<p>This routine receives all the properties.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | A pointer to a #NX_PACKET containing all the properties. |
| wait_option [in]    | Ticks to wait for message to receive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if all properties is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive all properties due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive all properties due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive all properties due to timeout.
* NX_AZURE_IOT_INVALID_PACKET Fail to receive all properties due to invalid packet.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to receive all properties due to SDK core error.
* NX_AZURE_IOT_SERVER_RESPONSE_ERROR Response code from server is not 2xx.
* NX_AZURE_IOT_DISCONNECTED Fail to receive all properties due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

#### **nx_azure_iot_hub_client_writable_properties_receive**
***
<div style="text-align: right">Receive writable properties form IoTHub</div>

**Prototype**
```c
UINT nx_azure_iot_hub_client_writable_properties_receive(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                         NX_PACKET **packet_pptr,
                                                         UINT wait_option);
```
**Description**

<p>This routine receives writable properties from IoTHub.</p>

**Parameters**

| Name | Description |
| - |:-|
| hub_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_HUB_CLIENT`. |
| packet_pptr [out]    | A pointer to a #NX_PACKET containing writable properties. |
| wait_option [in]    | Ticks to wait for message to receive. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if writable properties is received.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to receive writable properties due to invalid parameter.
* NX_AZURE_IOT_NOT_ENABLED Fail to receive writable properties due to it is not enabled.
* NX_AZURE_IOT_NO_PACKET Fail to receive writable properties due to timeout.
* NX_AZURE_IOT_INVALID_PACKET Fail to receive writable properties due to invalid packet.
* NX_AZURE_IOT_DISCONNECTED Fail to receive writable properties due to disconnect.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>


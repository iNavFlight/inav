# Azure IoT Provisioning Client

**nx_azure_iot_provisioning_client_initialize**
***
<div style="text-align: right"> Initialize Azure IoT Provisioning instance</div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_initialize(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                 NX_AZURE_IOT *nx_azure_iot_ptr,
                                                 const UCHAR *endpoint, UINT endpoint_length,
                                                 const UCHAR *id_scope, UINT id_scope_length,
                                                 const UCHAR *registration_id, UINT registration_id_length,
                                                 const NX_CRYPTO_METHOD **crypto_array, UINT crypto_array_size,
                                                 const NX_CRYPTO_CIPHERSUITE **cipher_map, UINT cipher_map_size,
                                                 UCHAR *metadata_memory, UINT memory_size,
                                                 NX_SECURE_X509_CERT *trusted_certificate);
```
**Description**

<p>This routine initializes the device to the IoT provisioning service.</p>

**Parameters**

| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT`. |
| nx_azure_iot_ptr [in]      | A pointer to a `NX_AZURE_IOT`.|
| endpoint [in] | A pointer to IoT Provisioning endpoint. Must be NULL terminated string.   |
| endpoint_length [in] | Length of the IoT Provisioning endpoint.  |
| id_scope [in]  | A pointer to ID Scope.     |
| id_scope_length [in] | Length of the ID Scope. |
| registration_id [in]  | A pointer to registration ID.     |
| registration_id_length [in] | Length of the registration ID. |
| crypto_array [in] | A pointer to `NX_CRYPTO_METHOD`    |
| crypto_array_size [in] | Size of crypto method array   |
| cipher_map [in] | A pointer to `NX_CRYPTO_CIPHERSUITE`    |
| cipher_map_size [in] | Size of cipher map    |
| metadata_memory [in] | A pointer to metadata memory buffer. |
| memory_size [in]  | Size of metadata buffer     |
| trusted_certificate [in] | A pointer to `NX_SECURE_X509_CERT`, which is server side certs |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully initialized to Azure IoT Provisioning Client.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to initialize the Azure IoT Provisioning Client due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to initialize the Azure IoT Provisioning Client due to SDK core error.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to initialize the Azure IoT Provisioning Client due to buffer size is too small.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_deinitialize**
***
<div style="text-align: right"> Cleanup the Azure IoT Provisioning Client</div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_deinitialize(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr);
```
**Description**

<p>This routine de-initializes AZ IoT Provisioning Client. </p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully cleaned up AZ IoT Provisioning Client Instance.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to deinitialize the AZ IoT Provisioning Client Instance due to invalid parameter.
* NX_AZURE_IOT_NOT_FOUND Fail to deinitialize the AZ IoT Provisioning Client Instance due to resource not found.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_trusted_cert_add**
***
<div style="text-align: right"> Add trusted certificate </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_trusted_cert_add(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                       NX_SECURE_X509_CERT *trusted_certificate);
```
**Description**

<p>This routine adds trusted certificate. It can be called multiple times to set certificate chain.</p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| trusted_certificate [in]    | A pointer to a `NX_SECURE_X509_CERT` |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully add trusted certs to AZ IoT Provisioning Client Instance.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to add trusted certs to AZ IoT Provisioning Client Instance due to invalid parameter.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to add trusted certificate due to `NX_AZURE_IOT_MAX_NUM_OF_TRUSTED_CERTS` is too small.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_device_cert_set**
***
<div style="text-align: right"> Set client certificate </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_device_cert_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                      NX_SECURE_X509_CERT *client_x509_cert);
```
**Description**

<p>This routine sets device certificate. It can be called multiple times to set certificate chain.</p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| client_x509_cert [in]    | A pointer to a `NX_SECURE_X509_CERT`, client cert. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set device certs to AZ IoT Provisioning Client Instance.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set device certs to AZ IoT Provisioning Client Instance due to invalid parameter.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to set device certificate due to `NX_AZURE_IOT_MAX_NUM_OF_DEVICE_CERTS` is too small.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_symmetric_key_set**
***
<div style="text-align: right"> Set symmetric key </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_symmetric_key_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                        const UCHAR *symmetric_key, UINT symmetric_key_length);
```
**Description**

<p>This routine sets symmetric key.</p>

**Parameters**

| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| symmetric_key [in]    | A pointer to a symmetric key. |
| symmetric_key_length [in]    | Length of symmetric key |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set symmetric key to the IoT Provisioning client.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set symmetric key to AZ IoT Provisioning Client Instance due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_register**
***
<div style="text-align: right"> Register device to Azure IoT Provisioning service </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_register(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr, UINT wait_option);

```
**Description**

<p>This routine registers device to Azure IoT Provisioning service.</p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| wait_option [in]    | Number of ticks to block for device registration. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully register device to AZ IoT Provisioning.
* NX_AZURE_IOT_PENDING Successfully started registration of device but not yet completed.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to register device to AZ IoT Provisioning due to invalid parameter.
* NX_AZURE_IOT_SDK_CORE_ERROR Fail to register device to AZ IoT Provisioning due to SDK core error.
* NX_AZURE_IOT_SERVER_RESPONSE_ERROR Fail to register device to AZ IoT Provisioning due to error response from server.
* NX_AZURE_IOT_DISCONNECTED Fail to register device to AZ IoT Provisioning due to disconnection.
* NX_DNS_QUERY_FAILED Fail to register device to AZ IoT Provisioning due to hostname can not be resolved.
* NX_NO_PACKET Fail to register device to AZ IoT Provisioning due to no available packet in pool.
* NX_INVALID_PARAMETERS Fail to register device to AZ IoT Provisioning due to invalid parameters.
* NX_SECURE_TLS_INSUFFICIENT_METADATA_SPACE Fail to register device to AZ IoT Provisioning due to insufficient metadata space.
* NX_SECURE_TLS_UNSUPPORTED_CIPHER Fail to register device to AZ IoT Provisioning due to unsupported cipher.
* NXD_MQTT_ALREADY_CONNECTED Fail to register device to AZ IoT Provisioning due to MQTT session is not disconnected.
* NXD_MQTT_CONNECT_FAILURE Fail to register device to AZ IoT Provisioning due to TCP/TLS connect error.
* NXD_MQTT_COMMUNICATION_FAILURE Fail to register device to AZ IoT Provisioning due to MQTT connect error.
* NXD_MQTT_ERROR_SERVER_UNAVAILABLE Fail to register device to AZ IoT Provisioning due to server unavailable.
* NXD_MQTT_ERROR_NOT_AUTHORIZED Fail to register device to AZ IoT Provisioning due to authentication error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>


**nx_azure_iot_provisioning_client_completion_callback_set**
***
<div style="text-align: right"> Set registration completion callback </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_completion_callback_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                              VOID (*on_complete_callback)(struct NX_AZURE_IOT_PROVISIONING_CLIENT_STRUCT *prov_client_ptr, UINT status));

```
**Description**

<p>This routine sets the callback for registration completion </p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| on_complete_callback [in]    | Registration completion callback. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successful register completion callback.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to register completion callback due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_iothub_device_info_get**
***
<div style="text-align: right"> Get IoTHub device info into user supplied buffer </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_iothub_device_info_get(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                             UCHAR *iothub_hostname, UINT *iothub_hostname_len,
                                                             UCHAR *device_id, UINT *device_id_len);

```
**Description**

<p>This routine gets IoTHub device info into user supplied buffer </p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| iothub_hostname [out]    | Buffer pointer that will contain IoTHub hostname. |
| iothub_hostname_len [inout]    | Pointer to UINT that contains size of buffer supplied, once successfully return it contains bytes copied to buffer |
| device_id [out]    | Buffer pointer that will contain IoTHub deviceId. |
| device_id_len [inout]    | Pointer to UINT that contains size of buffer supplied, once successfully return it contains bytes copied to buffer  |

**Return Values**
* NX_AZURE_IOT_SUCCESS The device info is successfully retrieved to user supplied buffers.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to retrieve device info due to invalid parameter.
* NX_AZURE_IOT_WRONG_STATE Fail to retrieve device info due to wrong state.
* NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE Fail to retrieve device info due to buffer size is too small.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_provisioning_client_registration_payload_set**
***
<div style="text-align: right"> Set registration payload </div>

**Prototype**
```c
UINT nx_azure_iot_provisioning_client_registration_payload_set(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                                                               const UCHAR *payload_ptr, UINT payload_length);

```
**Description**

<p>This routine sets registration payload, which is JSON object. </p>

**Parameters**
| Name | Description |
| - |:-|
| prov_client_ptr [in]    | A pointer to a `NX_AZURE_IOT_PROVISIONING_CLIENT` |
| payload_ptr [in]    | A pointer to registration payload. |
| payload_length [in]    | Length of `payload`. Does not include the `NULL` terminator. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully set registration payload to Azure IoT Provisioning Client.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to set registration payload Azure IoT Provisioning Client due to invalid parameter.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

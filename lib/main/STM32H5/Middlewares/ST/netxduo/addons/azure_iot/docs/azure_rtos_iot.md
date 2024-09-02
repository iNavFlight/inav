# Azure IoT

**nx_azure_iot_create**
***
<div style="text-align: right"> Create the Azure IoT subsystem</div>

**Prototype**
```c
UINT nx_azure_iot_create(NX_AZURE_IOT *nx_azure_iot_ptr, const UCHAR *name_ptr,
                         NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr,
                         VOID *stack_memory_ptr, UINT stack_memory_size,
                         UINT priority, UINT (*unix_time_callback)(ULONG *unix_time));
```
**Description**

<p>This routine creates the Azure IoT subsystem.  An internal thread is created to manage activities related to Azure IoT services. Only one `NX_AZURE_IOT` instance is needed to manage instances for Azure IoT hub, IoT Central, Device Provisioning Services (DPS), and Azure Security Center (ASC). </p>

<p>This routine enables ASC by default. ASC provides a comprehensive security solution for
Azure RTOS devices in which it collects network information and send it to the IoTHub.
More details can be found https://docs.microsoft.com/en-us/azure/defender-for-iot/iot-security-azure-rtos.
To disable ASC from your application, we provide both compile time and runtime option:
    - compile-time : NX_AZURE_DISABLE_IOT_SECURITY_MODULE in NetXDuo header file such as nx_port.h
                     when building the middleware.
    - runtime : Call UINT nx_azure_iot_security_module_disable(NX_AZURE_IOT *nx_azure_iot_ptr)
                in your application code.
</p>

**Parameters**
| Name | Description |
| - |:-|
| nx_azure_iot_ptr [in]    | A pointer to a NX_AZURE_IOT |
| name_ptr [in]      | A pointer to a NULL-terminated string indicating the name of the Azure IoT instance. |
| ip_ptr [in] | A pointer to a `NX_IP`, which is the IP stack used to connect to Azure IoT Services.     |
| pool_ptr [in] | A pointer to a `NX_PACKET_POOL`, which is the packet pool used by Azure IoT Services.     |
| dns_ptr [in] | A pointer to a `NX_DNS`.     |
| stack_memory_ptr [in] | A pointer to memory to be used as a stack space for the internal thread.     |
| stack_memory_size [in] | Size of stack memory area.  |
| priority [in] | Priority of the internal thread.    |
| unix_time_callback [in] | Callback to fetch unix time from platform.  |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully created the Azure IoT instance.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to create the Azure IoT instance due to invalid parameter.
* NX_OPTION_ERROR Fail to create the Azure IoT instance due to invalid priority.
* NX_PTR_ERROR Fail to create the Azure IoT instance due to invalid parameter.
* NX_SIZE_ERROR Fail to create the Azure IoT instance due to insufficient size of stack memory.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_delete**
***
<div style="text-align: right"> Shutdown and cleanup the Azure IoT subsystem</div>

**Prototype**
```c
UINT  nx_azure_iot_delete(NX_AZURE_IOT *nx_azure_iot_ptr);
```
**Description**

<p>This routine stops all Azure services managed by this instance, and cleans up internal resources. </p>

**Parameters**

| Name | Description |
| - |:-|
| nx_azure_iot_ptr  [in]    | A pointer to a `NX_AZURE_IOT` |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful stopped Azure IoT services and cleaned up internal resources.
* NX_AZURE_IOT_INVALID_PARAMETER Fail to delete the Azure IoT instance due to invalid parameter.
* NX_AZURE_IOT_DELETE_ERROR Fail to delete the Azure IoT instance due to resource is still in use.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_log_init**
***
<div style="text-align: right"> Initialize logging</div>

**Prototype**
```c
VOID nx_azure_iot_log_init(VOID(*log_callback)(az_log_classification classification, UCHAR *msg, UINT msg_len));
```
**Description**

<p>This routine initialized the logging with customer specific callback to output the logs for different classifications </p>

* AZ_LOG_IOT_AZURERTOS
* AZ_LOG_MQTT_RECEIVED_TOPIC
* AZ_LOG_MQTT_RECEIVED_PAYLOAD
* AZ_LOG_IOT_RETRY
* AZ_LOG_IOT_SAS_TOKEN

**Parameters**

| Name | Description |
| - |:-|
| log_callback  [in]    | A pointer to a callback |

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

# Azure RTOS IoT Security Module API

## Azure IoT Security Module

**nx_azure_iot_security_module_enable**
***
<div style="text-align: right"> Enable Azure IoT Security Module</div>

**Prototype**
```c
UINT nx_azure_iot_security_module_enable(NX_AZURE_IOT *nx_azure_iot_ptr);
```
**Description**

<p>This routine enables the Azure IoT Security Module subsystem. An internal state machine manage security events collection and sends them to Azure IoT Hub. Only one NX_AZURE_IOT_SECURITY_MODULE instance exists at most and needed to manage data collection. </p>

**Parameters**
| Name | Description |
| - |:-|
| nx_azure_iot_ptr [in]    | A pointer to a `NX_AZURE_IOT` |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully enabled Azure IoT Security Module.
* NX_AZURE_IOT_FAILURE Fail to enable Azure IoT Security Module due to internal error.
* NX_AZURE_IOT_INVALID_PARAMETER Security module requires valid #NX_AZURE_IOT instance.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_security_module_disable**
***
<div style="text-align: right"> Disable Azure IoT Security Module</div>

**Prototype**
```c
UINT nx_azure_iot_security_module_disable(NX_AZURE_IOT *nx_azure_iot_ptr);
```
**Description**

<p>This routine disables the Azure IoT Security Module subsystem. </p>

**Parameters**

| Name | Description |
| - |:-|
| nx_azure_iot_ptr  [in]    | A pointer to a `NX_AZURE_IOT`, if NULL the singleton instance will be disabled. |


**Return Values**
* NX_AZURE_IOT_SUCCESS Successful if Azure IoT Security Module has been disabled successfully.
* NX_AZURE_IOT_INVALID_PARAMETER Azure IoT Hub instance differ than the singleton composite instance.
* NX_AZURE_IOT_FAILURE Fail to disable Azure IoT Security Module due to internal error.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

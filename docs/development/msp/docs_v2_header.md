
# INAV MSP Messages reference
 
**This page is auto-generated from the [Master INAV-MSP definition file](https://github.com/xznhj8129/msp_documentation/blob/master/lib/msp_messages.json)**  

For details on the structure of MSP, see [The wiki page](https://github.com/iNavFlight/inav/wiki/MSP-V2)

For list of enums, see [Enum documentation page](https://github.com/xznhj8129/msp_documentation/blob/master/docs/inav_enums_ref.md)

For current generation code, see [documentation project](https://github.com/xznhj8129/msp_documentation) (temporary until official implementation)  



**Warning: Verification needed, exercise caution until completely verified for accuracy and cleared, Refer to source for absolute certainty** 

**Note: A handful of complex, variable-payload messages have not been fully parsed, their documentation is temporary (MSP_SET_VTX_CONFIG, MSP2_INAV_SET_GEOZONE_VERTEX, MSP2_COMMON_SET_SETTING, MSP2_SENSOR_HEADTRACKER)**  

**Guide:**

*   **MSP Versions:**
    *   **MSPv1:** The original protocol. Uses command IDs from 0 to 254.
    *   **MSPv2:** An extended version. Uses command IDs from 0x1000 onwards.
*   **Request Payload:** The request payload sent to the destination (usually the flight controller). May be empty or hold data for setting or requesting data from the FC. 
*   **Reply Payload:** The reply sent from the FC to the sender. May be empty or hold data.
*   **Notes:** Pay attention to message notes and description.

---
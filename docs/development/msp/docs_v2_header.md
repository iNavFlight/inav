
# INAV MSP Messages reference
 
**This page is auto-generated from the [master INAV MSP definitions file](https://github.com/iNavFlight/inav/blob/master/docs/development/msp/msp_messages.json)**  

For details on the structure of MSP, see [The wiki page](https://github.com/iNavFlight/inav/wiki/MSP-V2)

For list of enums, see [Enum documentation page](https://github.com/iNavFlight/inav/wiki/Enums-reference)

For current generation code, see [documentation project](https://github.com/xznhj8129/msp_documentation) (temporary until official implementation)  


**JSON file rev: <file_rev> - <file_hash>**

**Warning: Verification needed, exercise caution until completely verified for accuracy and cleared, especially for integer signs. Source-based generation/validation is forthcoming. Refer to source for absolute certainty** 

**If you find an error, it must be corrected in the JSON spec, not this markdown.**

**Guide:**

*   **MSP Versions:**
    *   **MSPv1:** The original protocol. Uses command IDs from 0 to 254.
    *   **MSPv2:** An extended version. Uses command IDs from 0x1000 onwards.
*   **Request Payload:** The request payload sent to the destination (usually the flight controller). May be empty or hold data for setting or requesting data from the FC. 
*   **Reply Payload:** The reply sent from the FC to the sender. May be empty or hold data.
*   **Notes:** Pay attention to message notes and description.

<format>

---

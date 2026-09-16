
# INAV MSP Messages reference
 
**This page is auto-generated from the [master INAV MSP definitions file](https://github.com/iNavFlight/inav/blob/master/docs/development/msp/msp_messages.json)**  

For details on the structure of MSP, see [The wiki page](https://github.com/iNavFlight/inav/wiki/MSP-V2)

For list of enums, see [Enum documentation page](https://github.com/iNavFlight/inav/wiki/Enums-reference)


**When To Regenerate**

Run `docs/development/msp/gen_docs.sh` whenever MSP docs inputs change:
- `msp_messages.json` message content/schema updates
- source enum changes under `src/main` that affect `inav_enums.json`
- `format.md` or this header template (`docs_v2_header.md`) changes

By default the script removes temporary generated headers. Use `--keep_headers` only when you need them.

**Versioning Rule**

When the MSP JSON specification changes, bump `msp_messages.json` version:
- breaking schema/compatibility change: increment `version.major`, reset `minor` and `patch`
- backward-compatible schema extension: increment `version.minor`, reset `patch`
- message/content/docs-only update inside current schema: increment `version.patch`

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

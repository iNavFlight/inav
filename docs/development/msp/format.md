# Format:
## JSON format example:
```
    "MSP_API_VERSION": {
        "code": 1,
        "mspv": 1,
        "request": null,
        "reply": {
            "payload": [
                {
                    "name": "mspProtocolVersion",
                    "ctype": "uint8_t",
                    "units": "",
                    "desc": "MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0)."
                },
                {
                    "name": "apiVersionMajor",
                    "ctype": "uint8_t",
                    "units": "",
                    "desc": "INAV API Major version (`API_VERSION_MAJOR`)."
                },
                {
                    "name": "apiVersionMinor",
                    "ctype": "uint8_t",
                    "units": "",
                    "desc": "INAV API Minor version (`API_VERSION_MINOR`)."
                }
            ],
        },
        "notes": "Used by configurators to check compatibility.",
        "description": "Provides the MSP protocol version and the INAV API version."
    },
```
## Message fields:
**name**: MSP message name\
**code**: Integer message code\
**description**: String with description of message\
**request**: null or dict of data sent\
**reply**: null or dict of data received\
**variable_len**: Optional boolean, if true, message does not have a predefined fixed length and needs appropriate handling\
**variants**: Optional special case, message has different cases of reply/request. Key/description is not a strict expression or code; just a readable condition\
**not_implemented**: Optional special case, message is not implemented\
**notes**: String with details of message\

## Data dict fields:
**payload**: Array of payload fields\
**repeating**: Optional Special Case, integer or string of how many times the *entire* payload is repeated\

## Payload fields:
### Fields:
**name**: field name from code\
**ctype**: C value type\
**desc**: Optional string with description and details of field\
**units**: Optional defined units\
**enum**: Optional string of enum struct if value is an enum
**array**: Optional boolean to denote field is array of more values\
**array_ctype**: If array, string to describe type of each element\
**array_size**: If array, Integer OR String to define array size, 0 if undefined, string is unparseable (ie: probably needs to be corrected)\
**repeating**: Optional Special case, contains array of more payload fields that are added Times * Key\
**payload**: If repeating, contains more payload fields\
**polymorph**: Optional boolean special case, field does not have a defined C type and could be anything\

**Simple value**
```
{
    "name": "mspProtocolVersion",
    "ctype": "uint8_t",
    "units": "",
    "desc": "MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0)."
},
```
**Fixed length array**
```
{
    "name": "fcVariantIdentifier",
    "ctype": "char[4]",
    "desc": "4-character identifier string (e.g., \"INAV\"). Defined by `flightControllerIdentifier",
    "array": true,
    "array_ctype": "char",
    "array_size": 4,
    "units": ""
}
```
**Undefined length array**
```
{
    "name": "firmwareChunk",
    "ctype": "uint8_t[]",
    "desc": "Chunk of firmware data",
    "array": true,
    "array_ctype": "uint8_t",
    "array_size": 0,
}
```
**As of yet unknown length array**
```
{
    "name": "elementText",
    "ctype": "char[]",
    "desc": "Static text bytes, not NUL-terminated"
    "array": true,
    "array_ctype": "char",
    "array_size": "OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1"
}
```
**Nested array with struct**
```
{
    "repeating": "maxVehicles",
    "payload": [
        {
            "name": "adsbVehicle",
            "ctype": "adsbVehicle_t[]",
            "desc": "Array of `adsbVehicle_t` Repeated `maxVehicles` times",
            "repeating": "maxVehicles",
            "array": true,
            "array_ctype": "adsbVehicle_t",
            "array_size": 0,
            "units": ""
        }
    ]
}
```
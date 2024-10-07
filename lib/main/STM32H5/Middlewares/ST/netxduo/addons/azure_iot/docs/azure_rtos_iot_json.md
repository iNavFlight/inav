# Azure IoT JSON

## Azure IoT JSON Reader

**nx_azure_iot_json_reader_with_buffer_init**
***
<div style="text-align: right">Initializes JSON Reader</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_with_buffer_init(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               const UCHAR *buffer_ptr, UINT buffer_len);
```
**Description**

<p>Initializes an #NX_AZURE_IOT_JSON_READER to read the JSON payload contained within the provided 
buffer. </p>

**Parameters**
| Name | Description |
| - |:-|
| reader_ptr [out]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance to initialize. |
| buffer_ptr [in]     | An pointer to buffer containing the JSON text to read. |
| buffer_len [in]     | Length of buffer.     |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully initialized JSON reader.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_init**
***
<div style="text-align: right">Initializes JSON Reader</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_init(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                   NX_PACKET *packet_ptr);
```
**Description**

<p>Initializes an #NX_AZURE_IOT_JSON_READER to read the JSON payload contained within #NX_PACKET</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [out]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance to initialize. |
| packet_ptr [in]     | A pointer to #NX_PACKET containing the JSON text. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully initialized JSON reader.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_deinit**
***
<div style="text-align: right">De-initializes JSON Reader</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_deinit(NX_AZURE_IOT_JSON_READER *reader_ptr);

```
**Description**

<p>De-initializes an #NX_AZURE_IOT_JSON_READER</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance to de-initialize |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully de-initialized JSON reader.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_next_token**
***
<div style="text-align: right">Move to next JSON token</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_next_token(NX_AZURE_IOT_JSON_READER *reader_ptr);

```
**Description**

<p>Reads the next token in the JSON text and updates the reader state.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The token was read successfully.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_skip_children**
***
<div style="text-align: right">Reads and skips over any nested JSON elements</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_skip_children(NX_AZURE_IOT_JSON_READER *reader_ptr);

```
**Description**

<p>Reads and skips over any nested JSON elements.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The children of the current JSON token are skipped successfully.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_bool_get**
***
<div style="text-align: right">Gets the JSON token's boolean value</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_bool_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                             UINT *value_ptr);

```
**Description**

<p>Gets the JSON token's boolean value.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |
| value_ptr [out]    | A pointer to a variable to receive the value. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The boolean value is returned.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_uint32_get**
***
<div style="text-align: right">Gets the JSON token's number as a 32-bit unsigned integer</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_uint32_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               uint32_t *value_ptr);

```
**Description**

<p>Gets the JSON token's number as a 32-bit unsigned integer.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |
| value_ptr [out]    | A pointer to a variable to receive the value. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The number is returned.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_int32_get**
***
<div style="text-align: right">Gets the JSON token's number as a 32-bit signed integer</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_int32_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                              int32_t *value_ptr);

```
**Description**

<p>Gets the JSON token's number as a 32-bit signed integer.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |
| value_ptr [out]    | A pointer to a variable to receive the value. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The number is returned.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_double_get**
***
<div style="text-align: right">Gets the JSON token's number as a `double`</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_double_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               double *value_ptr);

```
**Description**

<p>Gets the JSON token's number as a `double`.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |
| value_ptr [out]    | A pointer to a variable to receive the value. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The number is returned.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_string_get**
***
<div style="text-align: right">Gets the JSON token's string</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_string_get(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                               UCHAR *buffer_ptr, UINT buffer_size, UINT *bytes_copied);

```
**Description**

<p>Gets the JSON token's string after unescaping it, if required.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |
| buffer_ptr [out]   | A pointer to a buffer where the string should be copied into. |
| buffer_size [in]   | The maximum available space within the buffer referred to by buffer_ptr. |
| bytes_copied [out] | Contains the number of bytes written to the destination which denote the length of the unescaped string. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The string is returned.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_is_text_equal**
***
<div style="text-align: right">Compare token to text</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_is_text_equal(NX_AZURE_IOT_JSON_READER *reader_ptr,
                                                  UCHAR *expected_text_ptr, UINT expected_text_len);

```
**Description**

<p>Determines whether the unescaped JSON token value that the #NX_AZURE_IOT_JSON_READER points to is equal to the expected text within the provided buffer bytes by doing a case-sensitive comparison.</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]          | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |
| expected_text_ptr [in]   | A pointer to lookup text to compare the token against. |
| expected_text_len [in]   | Length of expected_text_ptr. |

**Return Values**
* NX_TRUE If the current JSON token value in the JSON source semantically matches the expected lookup text.
* NX_FALSE Otherwise.

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_reader_token_type**
***
<div style="text-align: right">Determines type of token currently #NX_AZURE_IOT_JSON_READER points to</div>

**Prototype**
```c
UINT nx_azure_iot_json_reader_token_type(NX_AZURE_IOT_JSON_READER *reader_ptr);

```
**Description**

<p>Determines type of token currently #NX_AZURE_IOT_JSON_READER points to..</p>

**Parameters**

| Name | Description |
| - |:-|
| reader_ptr [in]    | A pointer to an #NX_AZURE_IOT_JSON_READER instance containing the JSON to read. |

**Return Values**
* NX_AZURE_IOT_READER_TOKEN_NONE There is no value.
* NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT The token kind is the start of a JSON object.     
* NX_AZURE_IOT_READER_TOKEN_END_OBJECT The token kind is the end of a JSON object.     
* NX_AZURE_IOT_READER_TOKEN_BEGIN_ARRAY The token kind is the start of a JSON array.  
* NX_AZURE_IOT_READER_TOKEN_END_ARRAY The token kind is the end of a JSON array.        
* NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME The token kind is a JSON property name.    
* NX_AZURE_IOT_READER_TOKEN_STRING The token kind is a JSON string.           
* NX_AZURE_IOT_READER_TOKEN_NUMBER The token kind is a JSON number.           
* NX_AZURE_IOT_READER_TOKEN_TRUE The token kind is the JSON literal `true`.             
* NX_AZURE_IOT_READER_TOKEN_FALSE The token kind is the JSON literal `false`.            
* NX_AZURE_IOT_READER_TOKEN_NULL The token kind is the JSON literal `null`.              

**Allowed From**

Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

## Azure IoT JSON Writer

**nx_azure_iot_json_writer_init**
***
<div style="text-align: right">Initializes an JSON writer</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_init(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                   NX_PACKET *packet_ptr, UINT wait_option);
```
**Description**

<p>Initializes an #NX_AZURE_IOT_JSON_WRITER which writes JSON text into a NX_PACKET.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [out]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER the instance to initialize. |
| packet_ptr [in]        | A pointer to #NX_PACKET. |
| wait_option [in]       | Ticks to wait for allocating next packet     |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully initialized JSON writer.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_with_buffer_init**
***
<div style="text-align: right">Initializes an JSON writer</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_with_buffer_init(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                               UCHAR *buffer_ptr, UINT buffer_len);
```
**Description**

<p>Initializes an #NX_AZURE_IOT_JSON_WRITER which writes JSON text into a buffer passed.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [out]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER the instance to initialize. |
| buffer_ptr [in]        | A buffer pointer to which JSON text will be written. |
| buffer_len [in]        | Length of buffer.    |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully initialized JSON writer.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_deinit**
***
<div style="text-align: right">De-initializes an JSON writer</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_deinit(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Deinitializes an #NX_AZURE_IOT_JSON_WRITER.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER the instance to de-initialize. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Successfully de-initialized JSON writer.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_property_with_int32_value**
***
<div style="text-align: right">Appends the UTF-8 property name and value where value is int32</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_property_with_int32_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                               const UCHAR *property_name, UINT property_name_len,
                                                               int32_t value);
```
**Description**

<p>Appends the UTF-8 property name and value where value is int32.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| property_name [in]  | The UTF-8 encoded property name of the JSON value to be written. The name is escaped before writing |
| property_name_len [in]  | Length of property_name. |
| value [in]  | The value to be written as a JSON number. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The property name and int32 value was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_property_with_double_value**
***
<div style="text-align: right">Appends the UTF-8 property name and value where value is double</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_property_with_double_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                                const UCHAR *property_name, UINT property_name_len,
                                                                double value, UINT fractional_digits);
```
**Description**

<p>Appends the UTF-8 property name and value where value is double.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| property_name [in]  | The UTF-8 encoded property name of the JSON value to be written. The name is escaped before writing |
| property_name_len [in]  | Length of property_name. |
| value [in]  | The value to be written as a JSON number. |
| fractional_digits [in]  | The number of digits of the value to write after the decimal point and truncate the rest. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The property name and double value was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_property_with_bool_value**
***
<div style="text-align: right">Appends the UTF-8 property name and value where value is boolean</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_property_with_bool_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                              const UCHAR *property_name, UINT property_name_len,
                                                              UINT value);
```
**Description**

<p>Appends the UTF-8 property name and value where value is boolean.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| property_name [in]  | The UTF-8 encoded property name of the JSON value to be written. The name is escaped before writing |
| property_name_len [in]  | Length of property_name. |
| value [in]  | The value to be written as a JSON literal `true` or `false`. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The property name and bool value was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_property_with_string_value**
***
<div style="text-align: right">Appends the UTF-8 property name and value where value is string</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_property_with_string_value(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                                const UCHAR *property_name, UINT property_name_len,
                                                                const UCHAR *value, UINT value_len);
```
**Description**

<p>Appends the UTF-8 property name and value where value is string.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| property_name [in] | The UTF-8 encoded property name of the JSON value to be written. The name is escaped before writing |
| property_name_len [in]  | Length of property_name. |
| value [in]  |  The UTF-8 encoded property name of the JSON value to be written. The name is escaped before writing. |
| value_len [in]  |  Length of value. |
 
**Return Values**
* NX_AZURE_IOT_SUCCESS The property name and string value was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_get_bytes_used**
***
<div style="text-align: right">Returns the length containing the JSON text written to the underlying buffer</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_get_bytes_used(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Returns the length containing the JSON text written to the underlying buffer.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
 
**Return Values**
* UINT Value containing the length of JSON text built so far.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_string**
***
<div style="text-align: right">Appends the UTF-8 text value (as a JSON string) into the buffer</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_string(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                            const UCHAR *value, UINT value_len);
```
**Description**

<p>Appends the UTF-8 text value (as a JSON string) into the buffer.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| value [in]  | Pointer of UCHAR buffer that contains UTF-8 encoded value to be written as a JSON string The value is escaped before writing. |
| value_len [in]  | Length of value. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The string value was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_json_text**
***
<div style="text-align: right">Appends an existing UTF-8 encoded JSON text into the buffer</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_json_text(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                               const UCHAR *json, UINT json_len);
```
**Description**

<p>Appends an existing UTF-8 encoded JSON text into the buffer, useful for appending nested JSON.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| json [in]  | A pointer to single, possibly nested, valid, UTF-8 encoded, JSON value to be written as is, without any formatting or spacing changes. No modifications are made to this text, including  escaping. |
| json_len [in]  | Length of json. |


**Return Values**
* NX_AZURE_IOT_SUCCESS The provided json_text was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_property_name**
***
<div style="text-align: right">Appends property name</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_property_name(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                                   const UCHAR *value, UINT value_len);
```
**Description**

<p>Appends the UTF-8 property name (as a JSON string) which is the first part of a name/value pair of a JSON object.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| value [in]  | The UTF-8 encoded property name of the JSON value to be written. The name is escaped before writing. |
| value_len [in]  | Length of property name. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The property name was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_bool**
***
<div style="text-align: right">Appends a boolean value</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_bool(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, UINT value);
```
**Description**

<p>Appends a boolean value (as a JSON literal `true` or `false`).</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| value [in]  | The value to be written as a JSON literal `true` or `false`. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The bool was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_int32**
***
<div style="text-align: right">Appends a number value.</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_int32(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr, int32_t value);
```
**Description**

<p>Appends an `int32_t` number value.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| value [in]  | The value to be written as a JSON number. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The number was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_double**
***
<div style="text-align: right">Appends a number value.</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_double(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr,
                                            double value, int32_t fractional_digits);
```
**Description**

<p>Appends a `double` number value.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |
| value [in]  | The value to be written as a JSON number. |
| fractional_digits [in]  | The number of digits of the value to write after the decimal point and truncate the rest. |

**Return Values**
* NX_AZURE_IOT_SUCCESS The number was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_null**
***
<div style="text-align: right">Appends the JSON literal `null`</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_null(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Appends the JSON literal `null`.</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |

**Return Values**
* NX_AZURE_IOT_SUCCESS `null` was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_begin_object**
***
<div style="text-align: right">Appends the beginning of a JSON object</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_begin_object(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Appends the beginning of a JSON object (i.e. `{`).</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Object start was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_begin_array**
***
<div style="text-align: right">Appends the beginning of a JSON array</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_begin_array(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Appends the beginning of a JSON array (i.e. `[`).</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Array start was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_end_object**
***
<div style="text-align: right">Appends the end of the current JSON object</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_end_object(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Appends the end of the current JSON object (i.e. `}`).</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Object end was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

**nx_azure_iot_json_writer_append_end_array**
***
<div style="text-align: right">Appends the end of the current JSON array</div>

**Prototype**
```c
UINT nx_azure_iot_json_writer_append_end_array(NX_AZURE_IOT_JSON_WRITER *json_writer_ptr);
```
**Description**

<p>Appends the end of the current JSON array (i.e. `]`).</p>

**Parameters**
| Name | Description |
| - |:-|
| json_writer_ptr [in]  | A pointer to an #NX_AZURE_IOT_JSON_WRITER. |

**Return Values**
* NX_AZURE_IOT_SUCCESS Array end was appended successfully.

**Allowed From**

Initialization, Threads

**Example**

**See Also**

<div style="page-break-after: always;"></div>

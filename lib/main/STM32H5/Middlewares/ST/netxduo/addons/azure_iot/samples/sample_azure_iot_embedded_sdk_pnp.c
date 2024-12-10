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

#include <stdio.h>

#include "nx_api.h"
#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_hub_client_properties.h"
#include "nx_azure_iot_provisioning_client.h"

/* These are sample files, user can build their own certificate and ciphersuites.  */
#include "nx_azure_iot_cert.h"
#include "nx_azure_iot_ciphersuites.h"
#include "sample_config.h"

/* Define sample wait option.  */
#ifndef SAMPLE_WAIT_OPTION
#define SAMPLE_WAIT_OPTION                                              (NX_NO_WAIT)
#endif /* SAMPLE_WAIT_OPTION */

/* Define sample events.  */
#define SAMPLE_ALL_EVENTS                                               ((ULONG)0xFFFFFFFF)
#define SAMPLE_CONNECTED_EVENT                                          ((ULONG)0x00000001)
#define SAMPLE_DISCONNECT_EVENT                                         ((ULONG)0x00000002)
#define SAMPLE_PERIODIC_EVENT                                           ((ULONG)0x00000004)
#define SAMPLE_TELEMETRY_SEND_EVENT                                     ((ULONG)0x00000008)
#define SAMPLE_COMMAND_RECEIVE_EVENT                                    ((ULONG)0x00000010)
#define SAMPLE_PROPERTIES_RECEIVE_EVENT                                 ((ULONG)0x00000020)
#define SAMPLE_WRITABLE_PROPERTIES_RECEIVE_EVENT                        ((ULONG)0x00000040)
#define SAMPLE_REPORTED_PROPERTIES_SEND_EVENT                           ((ULONG)0x00000080)

#define SAMPLE_DEAFULT_START_TEMP_CELSIUS                               (22)
#define DOUBLE_DECIMAL_PLACE_DIGITS                                     (2)

#define SAMPLE_COMMAND_SUCCESS_STATUS                                   (200)
#define SAMPLE_COMMAND_ERROR_STATUS                                     (500)

#define SAMPLE_PNP_MODEL_ID                                             "dtmi:com:example:Thermostat;3"
#define SAMPLE_PNP_DPS_PAYLOAD                                          "{\"modelId\":\"" SAMPLE_PNP_MODEL_ID "\"}"

/* Generally, IoTHub Client and DPS Client do not run at the same time, user can use union as below to
   share the memory between IoTHub Client and DPS Client.

   NOTE: If user can not make sure sharing memory is safe, IoTHub Client and DPS Client must be defined seperately.  */
typedef union SAMPLE_CLIENT_UNION
{
    NX_AZURE_IOT_HUB_CLIENT                         iothub_client;

#ifdef ENABLE_DPS_SAMPLE
    NX_AZURE_IOT_PROVISIONING_CLIENT                prov_client;
#endif /* ENABLE_DPS_SAMPLE */

} SAMPLE_CLIENT;

static SAMPLE_CLIENT                                client;

#define iothub_client client.iothub_client
#ifdef ENABLE_DPS_SAMPLE
#define prov_client client.prov_client
#endif /* ENABLE_DPS_SAMPLE */

void sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time));

#ifdef ENABLE_DPS_SAMPLE
static UINT sample_dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                             UCHAR **iothub_hostname, UINT *iothub_hostname_length,
                             UCHAR **iothub_device_id, UINT *iothub_device_id_length);
#endif /* ENABLE_DPS_SAMPLE */

/* Define Azure RTOS TLS info.  */
static NX_SECURE_X509_CERT root_ca_cert;
static NX_SECURE_X509_CERT root_ca_cert_2;
static NX_SECURE_X509_CERT root_ca_cert_3;
static UCHAR nx_azure_iot_tls_metadata_buffer[NX_AZURE_IOT_TLS_METADATA_BUFFER_SIZE];
static ULONG nx_azure_iot_thread_stack[NX_AZURE_IOT_STACK_SIZE / sizeof(ULONG)];

/* Using X509 certificate authenticate to connect to IoT Hub,
   set the device certificate as your device.  */
#if (USE_DEVICE_CERTIFICATE == 1)
extern const UCHAR sample_device_cert_ptr[];
extern const UINT sample_device_cert_len;
extern const UCHAR sample_device_private_key_ptr[];
extern const UINT sample_device_private_key_len;
NX_SECURE_X509_CERT device_certificate;
#endif /* USE_DEVICE_CERTIFICATE */

/* Define buffer for IoTHub info.  */
#ifdef ENABLE_DPS_SAMPLE
static UCHAR sample_iothub_hostname[SAMPLE_MAX_BUFFER];
static UCHAR sample_iothub_device_id[SAMPLE_MAX_BUFFER];
#endif /* ENABLE_DPS_SAMPLE */

/* Define the prototypes for AZ IoT.  */
static NX_AZURE_IOT nx_azure_iot;

static TX_EVENT_FLAGS_GROUP sample_events;
static TX_TIMER sample_timer;
static volatile UINT sample_connection_status = NX_AZURE_IOT_NOT_INITIALIZED;
static volatile ULONG sample_periodic_counter = 0;

/* Telemetry.  */
static const CHAR telemetry_name[] = "temperature";

/* Device command.  */
static const CHAR report_command_name[] = "getMaxMinReport";

/* Device properties.  */
static const CHAR writable_temp_property_name[] = "targetTemperature";
static const CHAR reported_max_temp_since_last_reboot[] = "maxTempSinceLastReboot";
static const CHAR report_max_temp_name[] = "maxTemp";
static const CHAR report_min_temp_name[] = "minTemp";
static const CHAR report_avg_temp_name[] = "avgTemp";
static const CHAR report_start_time_name[] = "startTime";
static const CHAR report_end_time_name[] = "endTime";
static const CHAR reported_temp_property_name[] = "targetTemperature";
static const CHAR temp_response_description[] = "success";

/* Fake device data.  */
static const CHAR fake_start_report_time[] = "2020-01-10T10:00:00Z";
static const CHAR fake_end_report_time[] = "2023-01-10T10:00:00Z";
static double current_device_temp = SAMPLE_DEAFULT_START_TEMP_CELSIUS;
static double last_device_max_tem_reported = 0;
static double device_temperature_avg_total = SAMPLE_DEAFULT_START_TEMP_CELSIUS;
static int32_t device_temperature_avg_count = 1;
static double device_max_temp = SAMPLE_DEAFULT_START_TEMP_CELSIUS;
static double device_min_temp = SAMPLE_DEAFULT_START_TEMP_CELSIUS;
static double device_avg_temp = SAMPLE_DEAFULT_START_TEMP_CELSIUS;
static UCHAR scratch_buffer[256];

/* Include the connection monitor function from sample_azure_iot_embedded_sdk_connect.c.  */
extern VOID sample_connection_monitor(NX_IP *ip_ptr, NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr, UINT connection_status,
                                      UINT (*iothub_init)(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr));

#ifndef DISABLE_ADU_SAMPLE
/* Include the adu start function from sample_azure_iot_embedded_sdk_adu.c.  */
extern VOID sample_adu_start(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr);
#endif /* DISABLE_ADU_SAMPLE */

/* Send writable properties response as reported property.  */
static VOID sample_send_target_temperature_report(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, double current_device_temp_value,
                                                  UINT status, ULONG version, UCHAR *description_ptr,
                                                  UINT description_len)
{
NX_AZURE_IOT_JSON_WRITER json_writer;
NX_PACKET *packet_ptr;
UINT response_status;
UINT request_id;

    if (nx_azure_iot_hub_client_reported_properties_create(hub_client_ptr,
                                                           &packet_ptr, NX_WAIT_FOREVER))
    {
        printf("Failed to build reported response\r\n");
        return;
    }

    if (nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER))
    {
        printf("Failed to build reported response\r\n");
        nx_packet_release(packet_ptr);
        return;
    }

    if (nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
        nx_azure_iot_hub_client_reported_properties_status_begin(hub_client_ptr,
                                                                 &json_writer, (const UCHAR *)reported_temp_property_name,
                                                                 sizeof(reported_temp_property_name) - 1,
                                                                 status, version,
                                                                 description_ptr, description_len) ||
        nx_azure_iot_json_writer_append_double(&json_writer,
                                               current_device_temp_value,
                                               DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_hub_client_reported_properties_status_end(hub_client_ptr, &json_writer) ||
        nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        printf("Failed to build reported response\r\n");
        nx_packet_release(packet_ptr);
    }
    else
    {
        if (nx_azure_iot_hub_client_reported_properties_send(hub_client_ptr,
                                                             packet_ptr, &request_id,
                                                             &response_status, NX_NULL,
                                                             (5 * NX_IP_PERIODIC_RATE)))
        {
            printf("Failed to send reported response\r\n");
            nx_packet_release(packet_ptr);
        }
    }
}

/* Parses writable properties document.  */
static UINT sample_parse_writable_temp_property(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                                UINT message_type, ULONG version)
{
double parsed_value;
UINT status;
const UCHAR *component_name_ptr;
USHORT component_length = 0;

    while ((status = nx_azure_iot_hub_client_properties_component_property_next_get(hub_client_ptr,
                                                                                    json_reader_ptr,
                                                                                    message_type,
                                                                                    NX_AZURE_IOT_HUB_CLIENT_PROPERTY_WRITABLE,
                                                                                    &component_name_ptr, &component_length)) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                         (UCHAR *)writable_temp_property_name,
                                                         sizeof(writable_temp_property_name) - 1))
        {
            if ((status = nx_azure_iot_json_reader_next_token(json_reader_ptr)) ||
                (status = nx_azure_iot_json_reader_token_double_get(json_reader_ptr, &parsed_value)))
            {
                return(status);
            }

            break;
        }
        else
        {

            /* The JSON reader must be advanced regardless of whether the property
               is of interest or not.  */
            nx_azure_iot_json_reader_next_token(json_reader_ptr);
 
            /* Skip children in case the property value is an object.  */
            if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
            {
                nx_azure_iot_json_reader_skip_children(json_reader_ptr);
            }
            nx_azure_iot_json_reader_next_token(json_reader_ptr);
        }
    }

    if (status)
    {
        return(status);
    }

    current_device_temp = parsed_value;
    if (current_device_temp > device_max_temp)
    {
        device_max_temp = current_device_temp;
    }

    if (current_device_temp < device_min_temp)
    {
        device_min_temp = current_device_temp;
    }

    /* Increment the avg count, add the new temp to the total, and calculate the new avg.  */
    device_temperature_avg_count++;
    device_temperature_avg_total += current_device_temp;
    device_avg_temp = device_temperature_avg_total / device_temperature_avg_count;

    sample_send_target_temperature_report(hub_client_ptr, current_device_temp, 200,
                                          version, (UCHAR *)temp_response_description,
                                          sizeof(temp_response_description) - 1);

    return(NX_AZURE_IOT_SUCCESS);
}

/* sample command implementation.  */
static UINT sample_get_maxmin_report(NX_AZURE_IOT_JSON_READER *json_reader_ptr,
                                     NX_AZURE_IOT_JSON_WRITER *out_json_writer_ptr)
{
UINT status;
UCHAR *start_time = (UCHAR *)fake_start_report_time;
UINT start_time_len = sizeof(fake_start_report_time) - 1;
UCHAR time_buf[32];

    /* Check for start time if present  */
    if ((status = nx_azure_iot_json_reader_next_token(json_reader_ptr)) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_string_get(json_reader_ptr, time_buf,
                                                      sizeof(time_buf), &start_time_len))
        {
            return(NX_NOT_SUCCESSFUL);
        }

        start_time = time_buf;
    }
    else
    {
        if (status != NX_AZURE_IOT_EMPTY_JSON)
        {
            return(NX_NOT_SUCCESSFUL);
        }
    }
    
    if (nx_azure_iot_json_writer_append_begin_object(out_json_writer_ptr) ||
        nx_azure_iot_json_writer_append_property_with_double_value(out_json_writer_ptr,
                                                                   (UCHAR *)report_max_temp_name,
                                                                   sizeof(report_max_temp_name) - 1,
                                                                   device_max_temp, DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(out_json_writer_ptr,
                                                                   (UCHAR *)report_min_temp_name,
                                                                   sizeof(report_min_temp_name) - 1,
                                                                   device_min_temp, DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_double_value(out_json_writer_ptr,
                                                                   (UCHAR *)report_avg_temp_name,
                                                                   sizeof(report_avg_temp_name) - 1,
                                                                   device_avg_temp, DOUBLE_DECIMAL_PLACE_DIGITS) ||
        nx_azure_iot_json_writer_append_property_with_string_value(out_json_writer_ptr,
                                                                   (UCHAR *)report_start_time_name,
                                                                   sizeof(report_start_time_name) - 1,
                                                                   start_time, start_time_len) ||
        nx_azure_iot_json_writer_append_property_with_string_value(out_json_writer_ptr,
                                                                   (UCHAR *)report_end_time_name,
                                                                   sizeof(report_end_time_name) - 1,
                                                                   (UCHAR *)fake_end_report_time,
                                                                   sizeof(fake_end_report_time) - 1) ||
        nx_azure_iot_json_writer_append_end_object(out_json_writer_ptr))
    {
        printf("Failed to build getMaxMinReport response \r\n");
        status = NX_NOT_SUCCESSFUL;
    }
    else
    {
        status = NX_AZURE_IOT_SUCCESS;
    }

    return(status);
}

static VOID connection_status_callback(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, UINT status)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    if (status)
    {
        printf("Disconnected from IoTHub!: error code = 0x%08x\r\n", status);
        tx_event_flags_set(&sample_events, SAMPLE_DISCONNECT_EVENT, TX_OR);
    }
    else
    {
        printf("Connected to IoTHub.\r\n");
        tx_event_flags_set(&sample_events, SAMPLE_CONNECTED_EVENT, TX_OR);
    }

    sample_connection_status = status;
}

static VOID message_receive_callback_properties(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{

    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&sample_events, SAMPLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_command(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(sample_events), SAMPLE_COMMAND_RECEIVE_EVENT, TX_OR);
}

static VOID message_receive_callback_writable_properties(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, VOID *context)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(sample_events), SAMPLE_WRITABLE_PROPERTIES_RECEIVE_EVENT, TX_OR);
}

static UINT sample_initialize_iothub(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr)
{
UINT status;
#ifdef ENABLE_DPS_SAMPLE
UCHAR *iothub_hostname = NX_NULL;
UCHAR *iothub_device_id = NX_NULL;
UINT iothub_hostname_length = 0;
UINT iothub_device_id_length = 0;
#else
UCHAR *iothub_hostname = (UCHAR *)HOST_NAME;
UCHAR *iothub_device_id = (UCHAR *)DEVICE_ID;
UINT iothub_hostname_length = sizeof(HOST_NAME) - 1;
UINT iothub_device_id_length = sizeof(DEVICE_ID) - 1;
#endif /* ENABLE_DPS_SAMPLE */

#ifdef ENABLE_DPS_SAMPLE

    /* Run DPS.  */
    if ((status = sample_dps_entry(&prov_client, &iothub_hostname, &iothub_hostname_length,
                                   &iothub_device_id, &iothub_device_id_length)))
    {
        printf("Failed on sample_dps_entry!: error code = 0x%08x\r\n", status);
        return(status);
    }
#endif /* ENABLE_DPS_SAMPLE */

    printf("IoTHub Host Name: %.*s; Device ID: %.*s.\r\n",
           iothub_hostname_length, iothub_hostname, iothub_device_id_length, iothub_device_id);

    /* Initialize IoTHub client.  */
    if ((status = nx_azure_iot_hub_client_initialize(iothub_client_ptr, &nx_azure_iot,
                                                     iothub_hostname, iothub_hostname_length,
                                                     iothub_device_id, iothub_device_id_length,
                                                     (const UCHAR *)MODULE_ID, sizeof(MODULE_ID) - 1,
                                                     _nx_azure_iot_tls_supported_crypto,
                                                     _nx_azure_iot_tls_supported_crypto_size,
                                                     _nx_azure_iot_tls_ciphersuite_map,
                                                     _nx_azure_iot_tls_ciphersuite_map_size,
                                                     nx_azure_iot_tls_metadata_buffer,
                                                     sizeof(nx_azure_iot_tls_metadata_buffer),
                                                     &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_hub_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Set the model id.  */
    if ((status = nx_azure_iot_hub_client_model_id_set(iothub_client_ptr,
                                                       (const UCHAR *)SAMPLE_PNP_MODEL_ID,
                                                       sizeof(SAMPLE_PNP_MODEL_ID) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_model_id_set!: error code = 0x%08x\r\n", status);
    }
    
    /* Add more CA certificates.  */
    else if ((status = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &root_ca_cert_2)))
    {
        printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_trusted_cert_add(iothub_client_ptr, &root_ca_cert_3)))
    {
        printf("Failed on nx_azure_iot_hub_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Initialize the device certificate.  */
    else if ((status = nx_secure_x509_certificate_initialize(&device_certificate,
                                                             (UCHAR *)sample_device_cert_ptr, (USHORT)sample_device_cert_len,
                                                             NX_NULL, 0,
                                                             (UCHAR *)sample_device_private_key_ptr, (USHORT)sample_device_private_key_len,
                                                             DEVICE_KEY_TYPE)))
    {
        printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_hub_client_device_cert_set(iothub_client_ptr, &device_certificate)))
    {
        printf("Failed on nx_azure_iot_hub_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    else if ((status = nx_azure_iot_hub_client_symmetric_key_set(iothub_client_ptr,
                                                                 (UCHAR *)DEVICE_SYMMETRIC_KEY,
                                                                 sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!\r\n");
    }
#endif /* USE_DEVICE_CERTIFICATE */

#ifdef NXD_MQTT_OVER_WEBSOCKET

    /* Enable MQTT over WebSocket to connect to IoT Hub  */
    else if ((status = nx_azure_iot_hub_client_websocket_enable(iothub_client_ptr)))
    {
        printf("Failed on nx_azure_iot_hub_client_websocket_enable!\r\n");
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    /* Enable command and properties features.  */
    else if ((status = nx_azure_iot_hub_client_command_enable(iothub_client_ptr)))
    {
        printf("Failed on nx_azure_iot_hub_client_command_enable!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_properties_enable(iothub_client_ptr)))
    {
        printf("Failed on nx_azure_iot_hub_client_properties_enable!: error code = 0x%08x\r\n", status);
    }

    /* Set connection status callback.  */
    else if ((status = nx_azure_iot_hub_client_connection_status_callback_set(iothub_client_ptr,
                                                                              connection_status_callback)))
    {
        printf("Failed on connection_status_callback!\r\n");
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_COMMAND,
                                                                    message_receive_callback_command,
                                                                    NX_NULL)))
    {
        printf("device command callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_PROPERTIES,
                                                                    message_receive_callback_properties,
                                                                    NX_NULL)))
    {
        printf("device properties callback set!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_hub_client_receive_callback_set(iothub_client_ptr,
                                                                    NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                                                                    message_receive_callback_writable_properties,
                                                                    NX_NULL)))
    {
        printf("device writable properties callback set!: error code = 0x%08x\r\n", status);
    }

    if (status)
    {
        nx_azure_iot_hub_client_deinitialize(iothub_client_ptr);
    }

    return(status);
}

static void sample_command_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
const UCHAR *component_name_ptr;
USHORT component_name_length;
const UCHAR *command_name_ptr;
USHORT command_name_length;
USHORT context_length;
VOID *context_ptr;
UINT dm_status = 404;
UINT response_payload = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
NX_AZURE_IOT_JSON_WRITER json_writer;

    /* Loop to receive command message.  */
    while (1)
    {

        if (sample_connection_status != NX_SUCCESS)
        {
            return;
        }

        if ((status = nx_azure_iot_hub_client_command_message_receive(hub_client_ptr,
                                                                      &component_name_ptr, &component_name_length,
                                                                      &command_name_ptr, &command_name_length,
                                                                      &context_ptr, &context_length,
                                                                      &packet_ptr, NX_NO_WAIT)))
        {
            return;
        }

        printf("Received command: %.*s", (INT)command_name_length, (CHAR *)command_name_ptr);
        printf("\r\n");

        if ((status = nx_azure_iot_json_reader_init(&json_reader,
                                                    packet_ptr)))
        {
            printf("Failed to initialize json reader \r\n");
            nx_packet_release(packet_ptr);
            return;
        }

        if ((status = nx_azure_iot_json_writer_with_buffer_init(&json_writer,
                                                                scratch_buffer,
                                                                sizeof(scratch_buffer))))
        {
            printf("Failed to initialize json writer response \r\n");
            nx_packet_release(packet_ptr);
            return;
        }

        if ((command_name_length == (sizeof(report_command_name) - 1)) &&
            (memcmp((VOID *)command_name_ptr, (VOID *)report_command_name,
                    sizeof(report_command_name) - 1) == 0))
        {
            if (sample_get_maxmin_report(&json_reader, &json_writer) != NX_AZURE_IOT_SUCCESS)
            {
                dm_status = SAMPLE_COMMAND_ERROR_STATUS;
            }
            else
            {
                dm_status = SAMPLE_COMMAND_SUCCESS_STATUS;
                response_payload = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
            }
        }

        nx_packet_release(packet_ptr);

        if ((status = nx_azure_iot_hub_client_command_message_response(hub_client_ptr, dm_status,
                                                                       context_ptr, context_length, scratch_buffer,
                                                                       response_payload, NX_WAIT_FOREVER)))
        {
            printf("Command response failed!: error code = 0x%08x\r\n", status);
        }
    }
}

static void sample_writable_properties_receive_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
ULONG properties_version;

    if (sample_connection_status != NX_SUCCESS)
    {
        return;
    }

    if ((status = nx_azure_iot_hub_client_writable_properties_receive(hub_client_ptr,
                                                                      &packet_ptr,
                                                                      NX_WAIT_FOREVER)))
    {
        printf("Receive writable property receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Received writable property");
    printf("\r\n");

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    /* Get the version.  */
    if ((status = nx_azure_iot_hub_client_properties_version_get(hub_client_ptr, &json_reader, 
                                                                 NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES,
                                                                 &properties_version)))
    {
        printf("Properties version get failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    status = sample_parse_writable_temp_property(hub_client_ptr, &json_reader,
                                                 NX_AZURE_IOT_HUB_WRITABLE_PROPERTIES, properties_version);
    if (status && (status != NX_AZURE_IOT_NOT_FOUND))
    {
        printf("Failed to parse value\r\n");
    }

    nx_packet_release(packet_ptr);
}

static void sample_reported_properties_send_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
UINT response_status;
UINT request_id;
NX_AZURE_IOT_JSON_WRITER json_writer;
NX_PACKET *packet_ptr;
ULONG reported_property_version;

    if (sample_connection_status != NX_SUCCESS)
    {
        return;
    }

    if (((last_device_max_tem_reported - 0.01) < device_max_temp) &&
        ((last_device_max_tem_reported + 0.01) > device_max_temp))
    {
        return;
    }

    if ((status = nx_azure_iot_hub_client_reported_properties_create(hub_client_ptr,
                                                                     &packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed create reported properties: error code = 0x%08x\r\n", status);
        return;
    }
    
    if ((status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed init json writer: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (status = nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                             (const UCHAR *)reported_max_temp_since_last_reboot,
                                                                             sizeof(reported_max_temp_since_last_reboot) - 1,
                                                                             device_max_temp, DOUBLE_DECIMAL_PLACE_DIGITS)) ||
        (status = nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        printf("Build reported property failed: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_hub_client_reported_properties_send(hub_client_ptr,
                                                                   packet_ptr,
                                                                   &request_id, &response_status,
                                                                   &reported_property_version,
                                                                   (5 * NX_IP_PERIODIC_RATE))))
    {
        printf("Reported properties failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((response_status < 200) || (response_status >= 300))
    {
        printf("Reported properties failed with code : %d\r\n", response_status);
        return;
    }

    last_device_max_tem_reported = device_max_temp;
}

static void sample_properties_receive_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
ULONG writable_properties_version;

    if (sample_connection_status != NX_SUCCESS)
    {
        return;
    }

    if ((status = nx_azure_iot_hub_client_properties_receive(hub_client_ptr,
                                                             &packet_ptr,
                                                             NX_WAIT_FOREVER)))
    {
        printf("Get all properties receive failed!: error code = 0x%08x\r\n", status);
        return;
    }

    printf("Received all properties");
    printf("\r\n");

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_hub_client_properties_version_get(hub_client_ptr, &json_reader,
                                                                 NX_AZURE_IOT_HUB_PROPERTIES,
                                                                 &writable_properties_version)))
    {
        printf("Properties version get failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    if ((status = nx_azure_iot_json_reader_init(&json_reader, packet_ptr)))
    {
        printf("Init json reader failed!: error code = 0x%08x\r\n", status);
        nx_packet_release(packet_ptr);
        return;
    }

    status = sample_parse_writable_temp_property(hub_client_ptr, &json_reader,
                                                 NX_AZURE_IOT_HUB_PROPERTIES, writable_properties_version);
    if (status && (status != NX_AZURE_IOT_NOT_FOUND))
    {
        printf("Failed to parse value\r\n");
    }

    nx_packet_release(packet_ptr);
}

static void sample_telemetry_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status = 0;
NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_WRITER json_writer;
UINT buffer_length;

    if (sample_connection_status != NX_SUCCESS)
    {
        return;
    }

    /* Create a telemetry message packet.  */
    if ((status = nx_azure_iot_hub_client_telemetry_message_create(hub_client_ptr,
                                                                   &packet_ptr,
                                                                   NX_WAIT_FOREVER)))
    {
        printf("Telemetry message create failed!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Build telemetry JSON payload.  */
    if(nx_azure_iot_json_writer_with_buffer_init(&json_writer, scratch_buffer, sizeof(scratch_buffer)))
    {
        printf("Telemetry message failed to build message\r\n");
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return;
    }

    if (nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
        nx_azure_iot_json_writer_append_property_with_double_value(&json_writer,
                                                                   (UCHAR *)telemetry_name,
                                                                   sizeof(telemetry_name) - 1,
                                                                   current_device_temp,
                                                                   DOUBLE_DECIMAL_PLACE_DIGITS) ||
         nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        printf("Telemetry message failed to build message\r\n");
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return;
    }

    buffer_length = nx_azure_iot_json_writer_get_bytes_used(&json_writer);
    if ((status = nx_azure_iot_hub_client_telemetry_send(hub_client_ptr, packet_ptr,
                                                         (UCHAR *)scratch_buffer, buffer_length,
                                                         SAMPLE_WAIT_OPTION)))
    {
        printf("Telemetry message send failed!: error code = 0x%08x\r\n", status);
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
        return;
    }

    printf("Telemetry message send: %.*s.\r\n", buffer_length, scratch_buffer);
}

#ifdef ENABLE_DPS_SAMPLE
static UINT sample_dps_entry(NX_AZURE_IOT_PROVISIONING_CLIENT *prov_client_ptr,
                             UCHAR **iothub_hostname, UINT *iothub_hostname_length,
                             UCHAR **iothub_device_id, UINT *iothub_device_id_length)
{
UINT status;

    printf("Start Provisioning Client...\r\n");

    /* Initialize IoT provisioning client.  */
    if ((status = nx_azure_iot_provisioning_client_initialize(prov_client_ptr, &nx_azure_iot,
                                                              (UCHAR *)ENDPOINT, sizeof(ENDPOINT) - 1,
                                                              (UCHAR *)ID_SCOPE, sizeof(ID_SCOPE) - 1,
                                                              (UCHAR *)REGISTRATION_ID, sizeof(REGISTRATION_ID) - 1,
                                                              _nx_azure_iot_tls_supported_crypto,
                                                              _nx_azure_iot_tls_supported_crypto_size,
                                                              _nx_azure_iot_tls_ciphersuite_map,
                                                              _nx_azure_iot_tls_ciphersuite_map_size,
                                                              nx_azure_iot_tls_metadata_buffer,
                                                              sizeof(nx_azure_iot_tls_metadata_buffer),
                                                              &root_ca_cert)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_initialize!: error code = 0x%08x\r\n", status);
        return(status);
    }

    /* Initialize length of hostname and device ID.  */
    *iothub_hostname_length = sizeof(sample_iothub_hostname);
    *iothub_device_id_length = sizeof(sample_iothub_device_id);

    /* Add more CA certificates.  */
    if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(prov_client_ptr, &root_ca_cert_2)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }
    else if ((status = nx_azure_iot_provisioning_client_trusted_cert_add(prov_client_ptr, &root_ca_cert_3)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_trusted_cert_add!: error code = 0x%08x\r\n", status);
    }

#if (USE_DEVICE_CERTIFICATE == 1)

    /* Initialize the device certificate.  */
    else if ((status = nx_secure_x509_certificate_initialize(&device_certificate, (UCHAR *)sample_device_cert_ptr, (USHORT)sample_device_cert_len, NX_NULL, 0,
                                                             (UCHAR *)sample_device_private_key_ptr, (USHORT)sample_device_private_key_len, DEVICE_KEY_TYPE)))
    {
        printf("Failed on nx_secure_x509_certificate_initialize!: error code = 0x%08x\r\n", status);
    }

    /* Set device certificate.  */
    else if ((status = nx_azure_iot_provisioning_client_device_cert_set(prov_client_ptr, &device_certificate)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_device_cert_set!: error code = 0x%08x\r\n", status);
    }
#else

    /* Set symmetric key.  */
    else if ((status = nx_azure_iot_provisioning_client_symmetric_key_set(prov_client_ptr, (UCHAR *)DEVICE_SYMMETRIC_KEY,
                                                                          sizeof(DEVICE_SYMMETRIC_KEY) - 1)))
    {
        printf("Failed on nx_azure_iot_hub_client_symmetric_key_set!: error code = 0x%08x\r\n", status);
    }
#endif /* USE_DEVICE_CERTIFICATE */

#ifdef NXD_MQTT_OVER_WEBSOCKET

    /* Enable MQTT over WebSocket.  */
    else if ((status = nx_azure_iot_provisioning_client_websocket_enable(prov_client_ptr)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_websocket_enable!\r\n");
    }
#endif /* NXD_MQTT_OVER_WEBSOCKET */

    else if ((status = nx_azure_iot_provisioning_client_registration_payload_set(prov_client_ptr, (UCHAR *)SAMPLE_PNP_DPS_PAYLOAD,
                                                                                 sizeof(SAMPLE_PNP_DPS_PAYLOAD) - 1)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_registration_payload_set!: error code = 0x%08x\r\n", status);
    }
    /* Register device.  */
    else if ((status = nx_azure_iot_provisioning_client_register(prov_client_ptr, NX_WAIT_FOREVER)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_register!: error code = 0x%08x\r\n", status);
    }

    /* Get Device info.  */
    else if ((status = nx_azure_iot_provisioning_client_iothub_device_info_get(prov_client_ptr,
                                                                               sample_iothub_hostname, iothub_hostname_length,
                                                                               sample_iothub_device_id, iothub_device_id_length)))
    {
        printf("Failed on nx_azure_iot_provisioning_client_iothub_device_info_get!: error code = 0x%08x\r\n", status);
    }
    else
    {
        *iothub_hostname = sample_iothub_hostname;
        *iothub_device_id = sample_iothub_device_id;
        printf("Registered Device Successfully.\r\n");
    }

    /* Destroy Provisioning Client.  */
    nx_azure_iot_provisioning_client_deinitialize(prov_client_ptr);

    return(status);
}
#endif /* ENABLE_DPS_SAMPLE */

static VOID sample_connected_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
UINT status;

#ifndef DISABLE_ADU_SAMPLE
    /* Start adu agent.   */
    sample_adu_start(hub_client_ptr);
#endif /* DISABLE_ADU_SAMPLE */

    /* Request all properties.  */
    if ((status = nx_azure_iot_hub_client_properties_request(hub_client_ptr, NX_WAIT_FOREVER)))
    {
        printf("Properties request failed!: error code = 0x%08x\r\n", status);
    }
    else
    {
        printf("Sent properties request.\r\n");
    }
}

static VOID sample_periodic_timer_entry(ULONG context)
{

    NX_PARAMETER_NOT_USED(context);
    tx_event_flags_set(&(sample_events), SAMPLE_PERIODIC_EVENT, TX_OR);
}

static VOID sample_periodic_action(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr)
{
    NX_PARAMETER_NOT_USED(hub_client_ptr);
    
    if ((sample_periodic_counter % 5) == 0)
    {

        /* Set telemetry send event and reported properties send event.  */
        tx_event_flags_set(&(sample_events), (SAMPLE_TELEMETRY_SEND_EVENT | SAMPLE_REPORTED_PROPERTIES_SEND_EVENT), TX_OR);
    }

    sample_periodic_counter++;
}

static void log_callback(az_log_classification classification, UCHAR *msg, UINT msg_len)
{
    if (classification == AZ_LOG_IOT_AZURERTOS)
    {
        printf("%.*s", msg_len, (CHAR *)msg);
    }
}

void sample_entry(NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, NX_DNS *dns_ptr, UINT (*unix_time_callback)(ULONG *unix_time))
{
UINT status = 0;
UINT loop = NX_TRUE;
ULONG app_events;

    nx_azure_iot_log_init(log_callback);

    /* Create Azure IoT handler.  */
    if ((status = nx_azure_iot_create(&nx_azure_iot, (UCHAR *)"Azure IoT", ip_ptr, pool_ptr, dns_ptr,
                                      nx_azure_iot_thread_stack, sizeof(nx_azure_iot_thread_stack),
                                      NX_AZURE_IOT_THREAD_PRIORITY, unix_time_callback)))
    {
        printf("Failed on nx_azure_iot_create!: error code = 0x%08x\r\n", status);
        return;
    }

    /* Initialize CA certificates.  */
    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert, (UCHAR *)_nx_azure_iot_root_cert,
                                                        (USHORT)_nx_azure_iot_root_cert_size,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert_2, (UCHAR *)_nx_azure_iot_root_cert_2,
                                                        (USHORT)_nx_azure_iot_root_cert_size_2,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    if ((status = nx_secure_x509_certificate_initialize(&root_ca_cert_3, (UCHAR *)_nx_azure_iot_root_cert_3,
                                                        (USHORT)_nx_azure_iot_root_cert_size_3,
                                                        NX_NULL, 0, NULL, 0, NX_SECURE_X509_KEY_TYPE_NONE)))
    {
        printf("Failed to initialize ROOT CA certificate!: error code = 0x%08x\r\n", status);
        nx_azure_iot_delete(&nx_azure_iot);
        return;
    }

    tx_timer_create(&(sample_timer), (CHAR*)"sample_app_timer", sample_periodic_timer_entry, 0,
                    NX_IP_PERIODIC_RATE, NX_IP_PERIODIC_RATE, TX_AUTO_ACTIVATE);
    tx_event_flags_create(&sample_events, (CHAR*)"sample_app_event");

    while (loop)
    {

        /* Pickup sample event flags.  */
        tx_event_flags_get(&(sample_events), SAMPLE_ALL_EVENTS, TX_OR_CLEAR, &app_events, NX_WAIT_FOREVER);

        if (app_events & SAMPLE_CONNECTED_EVENT)
        {
            sample_connected_action(&iothub_client);
        }

        if (app_events & SAMPLE_PERIODIC_EVENT)
        {
            sample_periodic_action(&iothub_client);
        }

        if (app_events & SAMPLE_TELEMETRY_SEND_EVENT)
        {
            sample_telemetry_action(&iothub_client);
        }

        if (app_events & SAMPLE_COMMAND_RECEIVE_EVENT)
        {
            sample_command_action(&iothub_client);
        }

        if (app_events & SAMPLE_PROPERTIES_RECEIVE_EVENT)
        {
            sample_properties_receive_action(&iothub_client);
        }

        if (app_events & SAMPLE_WRITABLE_PROPERTIES_RECEIVE_EVENT)
        {
            sample_writable_properties_receive_action(&iothub_client);
        }

        if (app_events & SAMPLE_REPORTED_PROPERTIES_SEND_EVENT)
        {
            sample_reported_properties_send_action(&iothub_client);
        }

        /* Connection monitor.  */
        sample_connection_monitor(ip_ptr, &iothub_client, sample_connection_status, sample_initialize_iothub);
    }

    /* Cleanup.  */
    tx_event_flags_delete(&sample_events);
    tx_timer_delete(&sample_timer);
    nx_azure_iot_hub_client_deinitialize(&iothub_client);
    nx_azure_iot_delete(&nx_azure_iot);
}

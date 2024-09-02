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

/* Version: 6.1 */

#include "nx_azure_iot_adu_agent.h"

/* Update buffer pointer and buffer size.  */
#define NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(a, b, c, d)                   { \
                                                                            (a) = (c); \
                                                                            (c) += (b); \
                                                                            (d) -= (b); \
                                                                        }

static VOID nx_azure_iot_adu_agent_event_process(VOID *adu_agent, ULONG common_events, ULONG module_own_events);
static VOID nx_azure_iot_adu_agent_timer_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static VOID nx_azure_iot_adu_agent_update_check_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static VOID nx_azure_iot_adu_agent_download_install_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static VOID nx_azure_iot_adu_agent_apply_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static UINT nx_azure_iot_adu_agent_manifest_verify(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static UINT nx_azure_iot_adu_agent_jws_split(UCHAR *jws, UINT jws_length,
                                             UCHAR **header, UINT *header_length,
                                             UCHAR **payload, UINT *payload_length,
                                             UCHAR **signature, UINT *signature_length);
static UINT nx_azure_iot_adu_agent_service_properties_get(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                          NX_AZURE_IOT_JSON_READER *json_reader_ptr);
static UINT nx_azure_iot_adu_agent_service_update_manifest_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                                   UCHAR *update_manifest,
                                                                   UINT update_manifest_size,
                                                                   NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *update_manifest_content,
                                                                   UCHAR *update_manifest_content_buffer,
                                                                   UINT update_manifest_content_buffer_size);
static UINT nx_azure_iot_adu_agent_service_reported_properties_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, 
                                                                    UINT status_code, ULONG version, const CHAR *description,
                                                                    ULONG wait_option);
static UINT nx_azure_iot_adu_agent_reported_properties_startup_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, UINT wait_option);
static UINT nx_azure_iot_adu_agent_reported_properties_state_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static VOID nx_azure_iot_adu_agent_step_state_update(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, UINT step_state);
static UINT nx_azure_iot_adu_agent_file_find(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, 
                                             NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *manifest_content,
                                             UCHAR *file_id, UINT file_id_length,
                                             NX_AZURE_IOT_ADU_AGENT_FILE **file);
static UINT nx_azure_iot_adu_agent_method_is_installed(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                       NX_AZURE_IOT_ADU_AGENT_COMPATIBILITY *compatibility,
                                                       NX_AZURE_IOT_ADU_AGENT_STEP *step,
                                                       UINT *is_installed);
static UINT nx_azure_iot_adu_agent_method_download(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                   NX_AZURE_IOT_ADU_AGENT_FILE *file,
                                                   UINT type, UCHAR *buffer_ptr, UINT buffer_size,
                                                   VOID (*adu_agent_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *));
static UINT nx_azure_iot_adu_agent_method_install(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                  VOID (*adu_agent_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *));
static UINT nx_azure_iot_adu_agent_method_apply(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                VOID (*adu_agent_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *));
static const NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY *nx_azure_iot_adu_agent_rsa_root_key_find(const UCHAR* kid, UINT kid_size);
static UINT nx_azure_iot_adu_agent_sha256_calculate(NX_CRYPTO_METHOD *sha256_method,
                                                    UCHAR *metadata_ptr, UINT metadata_size,
                                                    UCHAR *input_ptr, ULONG input_size,
                                                    UCHAR *output_ptr, ULONG output_size);
static UINT nx_azure_iot_adu_agent_rs256_verify(NX_AZURE_IOT_ADU_AGENT_CRYPTO *adu_agent_crypto,
                                                UCHAR *input_ptr, ULONG input_size,
                                                UCHAR *signature_ptr, ULONG signature_size,
                                                UCHAR *n, ULONG n_size,
                                                UCHAR *e, ULONG e_size,
                                                UCHAR *buffer_ptr, UINT buffer_size);
static UINT nx_azure_iot_adu_agent_file_url_parse(UCHAR *file_url, ULONG file_url_length, 
                                                  UCHAR *buffer_ptr, UINT buffer_size,
                                                  NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr);
static void nx_azure_iot_adu_agent_dns_query(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static void nx_azure_iot_adu_agent_dns_response_notify(NX_UDP_SOCKET *socket_ptr);
static void nx_azure_iot_adu_agent_dns_response_get(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static void nx_azure_iot_adu_agent_http_connect(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static void nx_azure_iot_adu_agent_http_request_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static void nx_azure_iot_adu_agent_http_response_receive(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr);
static void nx_azure_iot_adu_agent_http_establish_notify(NX_TCP_SOCKET *socket_ptr);
static void nx_azure_iot_adu_agent_http_receive_notify(NX_TCP_SOCKET *socket_ptr);
static void nx_azure_iot_adu_agent_download_state_update(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, UINT success);
static UINT nx_azure_iot_adu_agent_component_properties_process(VOID *reader_ptr,
                                                                ULONG version,
                                                                VOID *args);
extern UINT nx_azure_iot_hub_client_component_add_internal(NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                                           const UCHAR *component_name_ptr,
                                                           USHORT component_name_length,
                                                           UINT (*callback_ptr)(VOID *json_reader_ptr,
                                                                                ULONG version,
                                                                                VOID *args),
                                                           VOID *callback_args);
extern VOID nx_azure_iot_hub_client_properties_component_process(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
                                                                 NX_PACKET *packet_ptr, UINT message_type);

UINT nx_azure_iot_adu_agent_start(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                  NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr,
                                  const UCHAR *manufacturer, UINT manufacturer_length,
                                  const UCHAR *model, UINT model_length,
                                  const UCHAR *installed_criteria, UINT installed_criteria_length,
                                  VOID (*adu_agent_update_notify)(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                                  UINT update_state,
                                                                  UCHAR *provider, UINT provider_length,
                                                                  UCHAR *name, UINT name_length,
                                                                  UCHAR *version, UINT version_length),
                                  VOID (*adu_agent_update_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *))
{
UINT i;
UINT status;
NX_AZURE_IOT *nx_azure_iot_ptr;
NX_AZURE_IOT_RESOURCE *resource_ptr;
NX_CRYPTO_METHOD *method_sha256 = NX_NULL;
NX_CRYPTO_METHOD *method_rsa = NX_NULL;
NX_SECURE_TLS_SESSION *tls_session;
NX_AZURE_IOT_ADU_AGENT_CRYPTO *adu_agent_crypto;
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;
UINT component_added = NX_FALSE;

    if ((adu_agent_ptr == NX_NULL) || (iothub_client_ptr == NX_NULL) ||
        (manufacturer == NX_NULL) || (manufacturer_length == 0) ||
        (model == NX_NULL) || (model_length == 0) ||
        ((installed_criteria == NX_NULL) && (installed_criteria_length != 0)) ||
        (adu_agent_update_driver == NX_NULL))
    {
        LogError(LogLiteralArgs("ADU agent start fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Check if IoT Hub connected.  */
    if (iothub_client_ptr -> nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
    {
        LogError(LogLiteralArgs("ADU agent start fail: IOTHUB NOT CONNECTED"));
        return(NX_AZURE_IOT_WRONG_STATE);
    }

    /* Check if properties is enabled.  */
    if (iothub_client_ptr -> nx_azure_iot_hub_client_writable_properties_message.message_enable == NX_NULL)
    {
        LogError(LogLiteralArgs("ADU agent start fail: PROPERTIES NOT ENABLED"));
        return(NX_AZURE_IOT_NOT_ENABLED);
    }

    memset(adu_agent_ptr, 0, sizeof(NX_AZURE_IOT_ADU_AGENT));

    /* Set iothub client pointer and azure iot pointer.  */
    adu_agent_ptr -> nx_azure_iot_hub_client_ptr = iothub_client_ptr;
    nx_azure_iot_ptr = iothub_client_ptr -> nx_azure_iot_ptr;

    /* Set component process routine for system component.  */
    iothub_client_ptr -> nx_azure_iot_hub_client_component_properties_process = nx_azure_iot_hub_client_properties_component_process;

    /* Check if the component has been added.  */
    for (i = 0; i < (UINT)iothub_client_ptr -> iot_hub_client_core._internal.options.component_names_length; i++)
    {
 
        /* Compare the component.  */
        if ((iothub_client_ptr -> iot_hub_client_core._internal.options.component_names[i]._internal.size == sizeof(NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME) - 1) &&
            (!memcmp(iothub_client_ptr -> iot_hub_client_core._internal.options.component_names[i]._internal.ptr, NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME, sizeof(NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME) - 1)))
        {
            component_added = NX_TRUE;
        }
    }

    /* Add ADU component.  */
    if (component_added == NX_FALSE)
    {
        if ((status = nx_azure_iot_hub_client_component_add_internal(iothub_client_ptr, 
                                                                    (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME) - 1,
                                                                    nx_azure_iot_adu_agent_component_properties_process,
                                                                    adu_agent_ptr)))
        {
            LogError(LogLiteralArgs("ADU agent start fail: COMPONENT ADD FAIL: %d"), status);
            return(status);
        }
    }

    /* Save the mutex.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr = nx_azure_iot_ptr -> nx_azure_iot_mutex_ptr;

    /* Find RSA and SHA256.  */
    resource_ptr = &(iothub_client_ptr -> nx_azure_iot_hub_client_resource);
    for(i = 0; i < resource_ptr -> resource_crypto_array_size; i++)
    {
        if(resource_ptr -> resource_crypto_array[i] -> nx_crypto_algorithm == NX_CRYPTO_HASH_SHA256)
        {
            method_sha256 = (NX_CRYPTO_METHOD *)resource_ptr -> resource_crypto_array[i];
        }
        else if(resource_ptr -> resource_crypto_array[i] -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA)
        {
            method_rsa = (NX_CRYPTO_METHOD *)resource_ptr -> resource_crypto_array[i];
        }

        if ((method_sha256) && (method_rsa))
        {
            break;
        }
    }

    /* Check if find the crypto method.  */
    if ((method_sha256 == NX_NULL) || 
        (method_sha256 -> nx_crypto_operation == NX_NULL) ||
        (method_rsa == NX_NULL) ||
        (method_rsa -> nx_crypto_init == NX_NULL) ||
        (method_rsa -> nx_crypto_operation == NX_NULL) )
    {
        LogError(LogLiteralArgs("ADU agent start fail: NO AVAILABLE CIPHER"));
        return(NX_AZURE_IOT_NO_AVAILABLE_CIPHER);
    }

    /* Check if the metadata size is enough.  */
    if (method_sha256 -> nx_crypto_metadata_area_size > NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE)
    {
        LogError(LogLiteralArgs("ADU agent start fail: INSUFFICIENT BUFFER FOR SHA256"));
        return(NX_AZURE_IOT_INSUFFICIENT_BUFFER_SPACE);
    }

    /* Save the crypto methods (RS256) for verifying update manifest.  */
    adu_agent_crypto = &(adu_agent_ptr -> nx_azure_iot_adu_agent_crypto);

    /* Set RSA crypto, reuse the metadata from tls session.  */
    tls_session = &(iothub_client_ptr -> nx_azure_iot_hub_client_resource.resource_mqtt.nxd_mqtt_tls_session);
    adu_agent_crypto -> method_rsa = method_rsa;
    adu_agent_crypto -> method_rsa_metadata = tls_session -> nx_secure_public_cipher_metadata_area;
    adu_agent_crypto -> method_rsa_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

    /* Set SHA256 crypto.  */
    adu_agent_crypto -> method_sha256 = method_sha256;

    /* Call the driver to initialize the hardware.  */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE;
    driver_request.nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
    (adu_agent_update_driver)(&driver_request);

    /* Check status.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        LogError(LogLiteralArgs("ADU agent start fail: DRIVER ERROR"));
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Save the device properties (manufacturer and model).  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device_properties.manufacturer = manufacturer;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device_properties.manufacturer_length = manufacturer_length;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device_properties.model = model;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device_properties.model_length = model_length;

    /* Save the device properties for compatibility. Compatibility defines the criteria of a device that can install the update.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].device_properties.manufacturer = manufacturer;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].device_properties.manufacturer_length = manufacturer_length;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].device_properties.model = model;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].device_properties.model_length = model_length;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].installed_criteria = installed_criteria;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].installed_criteria_length = installed_criteria_length;

    /* Save the driver.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].device_driver_entry = adu_agent_update_driver;

    /* Set the entry as valid.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[0].valid = NX_TRUE;

    /* Send agent startup message.  */
    status = nx_azure_iot_adu_agent_reported_properties_startup_send(adu_agent_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        LogError(LogLiteralArgs("ADU agent start fail: CLIENT REPORTED PROPERTIES SEND FAIL"));
        return(status);
    }

    /* Set the state change notification.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_update_notify = adu_agent_update_notify;

    /* Set the dns pointer.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.dns_ptr = nx_azure_iot_ptr -> nx_azure_iot_dns_ptr;

    /* Set the UDP socket receive callback function for non-blocking DNS.  */
    nx_azure_iot_ptr -> nx_azure_iot_dns_ptr -> nx_dns_socket.nx_udp_socket_reserved_ptr = adu_agent_ptr;
    nx_udp_socket_receive_notify(&(nx_azure_iot_ptr -> nx_azure_iot_dns_ptr -> nx_dns_socket),
                                 nx_azure_iot_adu_agent_dns_response_notify);

    /* Register ADU module on cloud helper.  */
    status = nx_cloud_module_register(&(nx_azure_iot_ptr -> nx_azure_iot_cloud),
                                      &(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                      "Azure Device Update Module", 
                                      (NX_CLOUD_MODULE_AZURE_ADU_EVENT | NX_CLOUD_COMMON_PERIODIC_EVENT),
                                      nx_azure_iot_adu_agent_event_process, adu_agent_ptr);
    if (status)
    {
        LogError(LogLiteralArgs("ADU module register fail status: %d"), status);
        return(status);
    }

    LogInfo(LogLiteralArgs("ADU agent started successfully!"));

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_adu_agent_stop(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{

    if ((adu_agent_ptr == NX_NULL) || (adu_agent_ptr -> nx_azure_iot_hub_client_ptr == NX_NULL))
    {
        LogError(LogLiteralArgs("ADU agent stop fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Obtain the mutex.  */
    tx_mutex_get(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr, NX_WAIT_FOREVER);

    /* Check if there is downloading socket.  */
    if ((adu_agent_ptr -> nx_azure_iot_adu_agent_state == NX_AZURE_IOT_ADU_AGENT_STATE_DEPLOYMENT_IN_PROGRESS) &&
        (adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.state >= NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONNECT))
    {

        /* Delete http client.  */
        nx_web_http_client_delete(&(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.http_client));
    }

    /* Release the mutex.  */
    tx_mutex_put(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr);

    /* Deregister ADU module on cloud helper.  */
     nx_cloud_module_deregister(&(adu_agent_ptr -> nx_azure_iot_hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_cloud),
                                &(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module));

    LogInfo(LogLiteralArgs("ADU agent stopped!"));

    return(NX_AZURE_IOT_SUCCESS);
}

UINT nx_azure_iot_adu_agent_update_download_and_install(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
UINT status;

    if (adu_agent_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("ADU agent download&install fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Reset the step.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_current_step = NX_NULL;

    /* Set event to download and install update.  */
    status = nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                       NX_AZURE_IOT_ADU_AGENT_DOWNLOAD_INSTALL_EVENT);

    return(status);
}

UINT nx_azure_iot_adu_agent_update_apply(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
UINT status;

    if (adu_agent_ptr == NX_NULL)
    {
        LogError(LogLiteralArgs("ADU agent apply fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Set event to deploy update.  */
    status = nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                       NX_AZURE_IOT_ADU_AGENT_APPLY_EVENT);

    return(status);
}

#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
UINT nx_azure_iot_adu_agent_proxy_update_add(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                             const UCHAR *manufacturer, UINT manufacturer_length,
                                             const UCHAR *model, UINT model_length,
                                             const UCHAR *installed_criteria, UINT installed_criteria_length,
                                             VOID (*adu_agent_proxy_update_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *))
{
UINT i;
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;

    if ((adu_agent_ptr == NX_NULL) ||
        (manufacturer == NX_NULL) || (manufacturer_length == 0) ||
        (model == NX_NULL) || (model_length == 0) ||
        ((installed_criteria == NX_NULL) && (installed_criteria_length != 0)) ||
        (adu_agent_proxy_update_driver == NX_NULL))
    {
        LogError(LogLiteralArgs("ADU agent proxy update fail: INVALID POINTER"));
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Compatibility defines the criteria of a device that can install the update. It contains device properties (manufacturer and model). */

    /* Obtain the mutex.  */
    tx_mutex_get(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr, NX_WAIT_FOREVER);

    /* Find available entry.  */
    for (i = 0; i < NX_AZURE_IOT_ADU_AGENT_DEVICE_COUNT; i++)
    {
        if (adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].valid == NX_FALSE)
        {

            /* Find available entry.   */
            break;
        }
    }

    /* Check if find an available entry.  */
    if (i >= NX_AZURE_IOT_ADU_AGENT_DEVICE_COUNT)
    {

        /* Release the mutex.  */
        tx_mutex_put(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr);
        return(NX_AZURE_IOT_NO_MORE_ENTRIES);
    }

    /* Call the driver to initialize the hardware.  */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_INITIALIZE;
    driver_request.nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
    (adu_agent_proxy_update_driver)(&driver_request);

    /* Check status.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {

        /* Release the mutex.  */
        tx_mutex_put(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr);
        LogError(LogLiteralArgs("ADU agent start fail: DRIVER ERROR"));
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Setup the driver.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_driver_entry = adu_agent_proxy_update_driver;

    /* Save the device properties for compatibility. Compatibility defines the criteria of a device that can install the update.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.manufacturer = manufacturer;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.manufacturer_length = manufacturer_length;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.model = model;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.model_length = model_length;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].installed_criteria = installed_criteria;
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].installed_criteria_length = installed_criteria_length;

    /* Set the entry as valid.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].valid = NX_TRUE;

    /* Release the mutex.  */
    tx_mutex_put(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr);

    return(NX_AZURE_IOT_SUCCESS);
}
#endif /* (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1) */

static UINT nx_azure_iot_adu_agent_component_properties_process(VOID *reader_ptr,
                                                                ULONG version,
                                                                VOID *args)
{
UINT status;
NX_AZURE_IOT_JSON_READER *json_reader_ptr = (NX_AZURE_IOT_JSON_READER *)reader_ptr;
NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr = (NX_AZURE_IOT_ADU_AGENT *)args;

    /* Check the state.  */
    if (adu_agent_ptr -> nx_azure_iot_adu_agent_state == NX_AZURE_IOT_ADU_AGENT_STATE_DEPLOYMENT_IN_PROGRESS)
    {

        /* The JSON reader must be advanced regardless of whether the property
            is of interest or not.  */
        nx_azure_iot_json_reader_next_token(reader_ptr);
 
        /* Skip children in case the property value is an object.  */
        if (nx_azure_iot_json_reader_token_type(reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            nx_azure_iot_json_reader_skip_children(reader_ptr);
        }
        nx_azure_iot_json_reader_next_token(reader_ptr);

        return(NX_AZURE_IOT_SUCCESS);
    }

    /* Check "service" property name.   */
    if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                     (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SERVICE,
                                                     sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SERVICE) - 1))
    {

        /* Get service property value.  */
        status = nx_azure_iot_adu_agent_service_properties_get(adu_agent_ptr, json_reader_ptr);
        if (status)
        {
            LogError(LogLiteralArgs("ADU agent component process fail: SERVICE PROPERTIES GET FAIL"));
            return(status);
        }

        /* Send service response.  */
        nx_azure_iot_adu_agent_service_reported_properties_send(adu_agent_ptr, 
                                                                NX_AZURE_IOT_ADU_AGENT_STATUS_SUCCESS, version, "",
                                                                NX_NO_WAIT);

        /* Check the action.  */
        if (adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.action == NX_AZURE_IOT_ADU_AGENT_ACTION_CANCEL)
        {

            /* Reset the state.  */
            adu_agent_ptr -> nx_azure_iot_adu_agent_state = NX_AZURE_IOT_ADU_AGENT_STATE_IDLE;

            LogInfo(LogLiteralArgs("Cancel Command received"));
            return(NX_AZURE_IOT_SUCCESS);
        }

        else if (adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.action != NX_AZURE_IOT_ADU_AGENT_ACTION_APPLY_DEPLOYMENT)
        {
            return(NX_AZURE_IOT_FAILURE);
        }


        /* Verify manifest.  */
        if (nx_azure_iot_adu_agent_manifest_verify(adu_agent_ptr) != NX_TRUE)
        {
            LogError(LogLiteralArgs("Failed to verify update manifest signature"));
            return(NX_AZURE_IOT_FAILURE);
        }

        /* Process deployable update manifest.  */
        if (nx_azure_iot_adu_agent_service_update_manifest_process(adu_agent_ptr,
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest,
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_size,
                                                                   &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content),
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_buffer,
                                                                   NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE))
        {
            LogError(LogLiteralArgs("ADU agent component process fail: UPDATE MANIFEST PROCESS FAIL"));
            return(NX_AZURE_IOT_FAILURE);
        }

        /* Reset.  */
        adu_agent_ptr -> nx_azure_iot_adu_agent_current_step = NX_NULL;
        adu_agent_ptr -> nx_azure_iot_adu_agent_update_flag = NX_FALSE;

        /* Update the state.  */
        adu_agent_ptr -> nx_azure_iot_adu_agent_state = NX_AZURE_IOT_ADU_AGENT_STATE_DEPLOYMENT_IN_PROGRESS;

        /* Set event to start update automatically.  */
        nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                    NX_AZURE_IOT_ADU_AGENT_UPDATE_EVENT);
    }
    else
    {
        return(NX_AZURE_IOT_FAILURE);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static VOID nx_azure_iot_adu_agent_event_process(VOID *adu_agent, ULONG common_events, ULONG module_own_events)
{

NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr = (NX_AZURE_IOT_ADU_AGENT *)adu_agent;

    /* Obtain the mutex.  */
    tx_mutex_get(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr, NX_WAIT_FOREVER);

    /* Process common periodic event.   */
    if (common_events & NX_CLOUD_COMMON_PERIODIC_EVENT)
    {

        /* Process timer event.  */
        nx_azure_iot_adu_agent_timer_event_process(adu_agent_ptr);
    }

    /* Process events.  */
    if (module_own_events & NX_AZURE_IOT_ADU_AGENT_UPDATE_EVENT)
    {

        /* Update check.  */
        nx_azure_iot_adu_agent_update_check_event_process(adu_agent_ptr);
    }
    if (module_own_events & NX_AZURE_IOT_ADU_AGENT_DOWNLOAD_INSTALL_EVENT)
    {

        /* Update download and install.  */
        nx_azure_iot_adu_agent_download_install_event_process(adu_agent_ptr);
    }
    if (module_own_events & NX_AZURE_IOT_ADU_AGENT_APPLY_EVENT)
    {

        /* Update apply.  */
        nx_azure_iot_adu_agent_apply_event_process(adu_agent_ptr);
    }
    if (module_own_events & NX_AZURE_IOT_ADU_AGENT_DNS_RESPONSE_RECEIVE_EVENT)
    {

        /* Process DNS response get event.  */
        nx_azure_iot_adu_agent_dns_response_get(adu_agent_ptr);
    }
    if (module_own_events & NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_DONE_EVENT)
    {

        /* Process HTTP connect done event.  */
        nx_azure_iot_adu_agent_http_request_send(adu_agent_ptr);
    }
    if (module_own_events & NX_AZURE_IOT_ADU_AGENT_HTTP_RECEIVE_EVENT)
    {

        /* Process HTTP receive event.  */
        nx_azure_iot_adu_agent_http_response_receive(adu_agent_ptr);
    }

    /* Release the mutex.  */
    tx_mutex_put(adu_agent_ptr -> nx_azure_iot_adu_agent_mutex_ptr);
}

static VOID nx_azure_iot_adu_agent_timer_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{

NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);

    /* Check the timer for DNS/HTTP.  */
    if (downloader_ptr -> timeout)
    {

        /* Decrease the timeout.  */
        downloader_ptr -> timeout--;

        /* Check if it is timeout.  */
        if (downloader_ptr -> timeout != 0)
        {
            return;
        }

        /* Check the state.  */
        if (downloader_ptr -> state == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_QUERY)
        {

            /* DNS query timeout, try to receive dns response, if there is no DNS response. Retry DNS query.  */
            nx_azure_iot_adu_agent_dns_response_get(adu_agent_ptr);
        }
        else if ((downloader_ptr -> state == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONNECT) ||
                 (downloader_ptr -> state == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONTENT_GET))
        {
            LogError(LogLiteralArgs("Firmware download fail: TIMEOUT"));

            /* Timeout for http connect or http content get.  */
            nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        }
    }
}

static VOID nx_azure_iot_adu_agent_update_check_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *update_manifest_content = &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content);
NX_AZURE_IOT_ADU_AGENT_STEP *step;
NX_AZURE_IOT_ADU_AGENT_FILE *file;
UINT is_installed = NX_FALSE;
UINT i;
#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
UCHAR *json_data_ptr;
NX_AZURE_IOT_JSON_READER json_reader;
UCHAR *proxy_update_manifest_ptr;
UINT proxy_update_manifest_size;
NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *proxy_update_manifest_content;
NX_AZURE_IOT_ADU_AGENT_STEP *proxy_step;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);
#endif /* (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1) */

    /* Check if current step is completed or not.  */
    if ((adu_agent_ptr -> nx_azure_iot_adu_agent_current_step == NX_NULL) ||
        (adu_agent_ptr -> nx_azure_iot_adu_agent_current_step -> state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_STARTED) ||
        (adu_agent_ptr -> nx_azure_iot_adu_agent_current_step -> state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_APPLY_SUCCEEDED))
    {
        for (i = 0; i < update_manifest_content -> steps_count; i++)
        {

            /* Check the state.  */
            if (update_manifest_content -> steps[i].state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_IDLE)
            {
                adu_agent_ptr -> nx_azure_iot_adu_agent_current_step = &(update_manifest_content -> steps[i]);
                break;
            }
        }

        /* Check if all steps are checked.  */
        if (i == update_manifest_content -> steps_count)
        {

            /* Check if all updates are installed.  */
            if (adu_agent_ptr -> nx_azure_iot_adu_agent_update_flag == NX_TRUE)
            {

                /* If there is a new update, report deployment in progress state to server and start to download.  */
                nx_azure_iot_adu_agent_reported_properties_state_send(adu_agent_ptr);

                /* Check if set the update notify.  */
                if (adu_agent_ptr -> nx_azure_iot_adu_agent_update_notify)
                {

                    /* Notify the user and let users control the update.   */
                    adu_agent_ptr -> nx_azure_iot_adu_agent_update_notify(adu_agent_ptr,
                                                                        NX_AZURE_IOT_ADU_AGENT_UPDATE_RECEIVED,
                                                                        (UCHAR *)update_manifest_content -> update_id.provider,
                                                                        update_manifest_content -> update_id.provider_length,
                                                                        (UCHAR *)update_manifest_content -> update_id.name,
                                                                        update_manifest_content -> update_id.name_length,
                                                                        (UCHAR *)update_manifest_content -> update_id.version,
                                                                        update_manifest_content -> update_id.version_length);
                }
                else
                {

                    /* Set event to start update automatically.  */
                    nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                              NX_AZURE_IOT_ADU_AGENT_DOWNLOAD_INSTALL_EVENT);
                }
            }
            else
            {

                /* All updates are installed and applied.  */                
                adu_agent_ptr -> nx_azure_iot_adu_agent_state = NX_AZURE_IOT_ADU_AGENT_STATE_IDLE;

                /* Send reported properties to notify server.  */
                nx_azure_iot_adu_agent_reported_properties_state_send(adu_agent_ptr);
            }
            
            return;
        }
    }

    step = adu_agent_ptr -> nx_azure_iot_adu_agent_current_step;
    switch (step -> state)
    {

        /* Idle.  */
        case NX_AZURE_IOT_ADU_AGENT_STEP_STATE_IDLE:
        {

            /* Check the type and handler properties.  */
            if (((step -> type_length != 0) && 
                 (step -> type_length == sizeof(NX_AZURE_IOT_ADU_AGENT_STEP_TYPE_INLINE) - 1) &&
                 (!memcmp(step -> type, NX_AZURE_IOT_ADU_AGENT_STEP_TYPE_INLINE, step -> type_length))) ||
                ((step -> type_length == 0) &&
                 (step -> handler_length == sizeof(NX_AZURE_IOT_ADU_AGENT_STEP_HANDLER_SWUPDATE) - 1) &&
                 (!memcmp(step -> handler, NX_AZURE_IOT_ADU_AGENT_STEP_HANDLER_SWUPDATE, step -> handler_length))))
            {

                /* Call method to check if the update is installed.  */
                if (nx_azure_iot_adu_agent_method_is_installed(adu_agent_ptr, &(update_manifest_content -> compatibility), step, &is_installed))
                {
                    LogError(LogLiteralArgs("Failed to check the update"));
                    nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                    return;
                }

                if (is_installed == NX_TRUE)
                {
                    nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_APPLY_SUCCEEDED);
                }
                else
                {

                    /* Find the new update file and update it into step.  */
                    if (nx_azure_iot_adu_agent_file_find(adu_agent_ptr, update_manifest_content, (UCHAR *)step -> file_id, step -> file_id_length, &file))
                    {
                        LogError(LogLiteralArgs("Failed to find update file"));
                        nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                        return;
                    }

                    /* Set the file as new update.  */
                    step -> file = file;
                    adu_agent_ptr -> nx_azure_iot_adu_agent_update_flag = NX_TRUE;

                    nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_STARTED);
                }
            }
#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
            else if ((step -> type_length == sizeof(NX_AZURE_IOT_ADU_AGENT_STEP_TYPE_REFERENCE) - 1) &&
                     (!memcmp(step -> type, NX_AZURE_IOT_ADU_AGENT_STEP_TYPE_REFERENCE, step -> type_length)))
            {

                /* Leaf update.  */

                /* Find the manifest file.  */
                if (nx_azure_iot_adu_agent_file_find(adu_agent_ptr, update_manifest_content, (UCHAR *)step -> file_id, step -> file_id_length, &file))
                {
                    LogError(LogLiteralArgs("Failed to find proxy manifest file"));
                    nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                    return;
                }

                /* Update the state.  */
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_MANIFEST_DOWNLOAD_STARTED);

                /* Start to download firmware.  */
                if (nx_azure_iot_adu_agent_method_download(adu_agent_ptr, file, NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_MANIFEST, 
                                                           adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature,
                                                           NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE,
                                                           NX_NULL))
                {
                    LogError(LogLiteralArgs("Failed to download proxy manifest file"));
                    nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                }

                return;
            }
#endif /* (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1) */

            break;
        }

#if (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1)
        case NX_AZURE_IOT_ADU_AGENT_STEP_STATE_MANIFEST_DOWNLOAD_SUCCEEDED:
        {
            json_data_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature;
            proxy_update_manifest_ptr = json_data_ptr + downloader_ptr -> received_firmware_size;
            proxy_update_manifest_size = NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE - downloader_ptr -> received_firmware_size;
            proxy_update_manifest_content = &(adu_agent_ptr -> nx_azure_iot_adu_agent_proxy_update_manifest_content);

            /* There may be 3 BOM bytes (EF BB BF) for UTF-8 json, skip them if exist.  */
            if ((downloader_ptr -> received_firmware_size > 3) &&
                ((json_data_ptr[0] == 0xEF) && (json_data_ptr[1] == 0xBB) && (json_data_ptr[2] == 0XBF)))
            {
                json_data_ptr +=3;
                downloader_ptr -> received_firmware_size -= 3;
            }

            /* Initialize the update manifest string as json.  */
            if (nx_azure_iot_json_reader_with_buffer_init(&json_reader,
                                                          json_data_ptr,
                                                          downloader_ptr -> received_firmware_size))
            {
                LogError(LogLiteralArgs("Failed to initialize proxy update manifest"));
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                return;
            }

            /* Skip the first begin object. */
            if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT) ||
                (nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME) ||
                (!nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST,
                                                                sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST) - 1)) ||
                (nx_azure_iot_json_reader_next_token(&json_reader)) ||
                (nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                            proxy_update_manifest_ptr,
                                                            proxy_update_manifest_size,
                                                            &proxy_update_manifest_size)))
            {
                LogError(LogLiteralArgs("Failed to process proxy update manifest"));
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                return;
            }

            /* Process proxy update manifest.  */
            if (nx_azure_iot_adu_agent_service_update_manifest_process(adu_agent_ptr,
                                                                       proxy_update_manifest_ptr,
                                                                       proxy_update_manifest_size,
                                                                       proxy_update_manifest_content,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_sjwk,
                                                                       NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE))
            {
                LogError(LogLiteralArgs("Failed to process proxy update manifest"));
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                return;
            }

            if (proxy_update_manifest_content -> steps_count != 1)
            {
                LogError(LogLiteralArgs("Failed to process proxy update manifest"));
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                return;
            }

            proxy_step = &(proxy_update_manifest_content -> steps[0]);

            /* Update the installed criteria for reference step.  */
            step -> installed_criteria = proxy_step -> installed_criteria;
            step -> installed_criteria_length = proxy_step -> installed_criteria_length;

            /* Call method to check if the update is installed.  */
            if (nx_azure_iot_adu_agent_method_is_installed(adu_agent_ptr, &(proxy_update_manifest_content -> compatibility), step, &is_installed))
            {
                LogError(LogLiteralArgs("Failed to check the update"));
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                return;
            }

            if (is_installed == NX_TRUE)
            {
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_APPLY_SUCCEEDED);
            }
            else
            {

                /* Find the new update file and update it into step.  */
                if (nx_azure_iot_adu_agent_file_find(adu_agent_ptr, proxy_update_manifest_content, (UCHAR *)proxy_step -> file_id, proxy_step -> file_id_length, &file))
                {
                    LogError(LogLiteralArgs("Failed to find update file"));
                    nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                    return;
                }

                /* Set the file as new update.  */
                step -> file = file;
                adu_agent_ptr -> nx_azure_iot_adu_agent_update_flag = NX_TRUE;

                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_STARTED);
            }

            break;
        }
#endif /* (NX_AZURE_IOT_ADU_AGENT_PROXY_UPDATE_COUNT >= 1) */

        default:
        {
            return;
        }
    }

    /* Set event for next update check.  */        
    nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                              NX_AZURE_IOT_ADU_AGENT_UPDATE_EVENT);
}

static VOID nx_azure_iot_adu_agent_download_install_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{

NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *update_manifest_content = &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content);
NX_AZURE_IOT_ADU_AGENT_STEP *step;
UINT i;

    /* Check if current step is completed or not.  */
    if ((adu_agent_ptr -> nx_azure_iot_adu_agent_current_step == NX_NULL) ||
        (adu_agent_ptr -> nx_azure_iot_adu_agent_current_step -> state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_INSTALL_SUCCEEDED) ||
        (adu_agent_ptr -> nx_azure_iot_adu_agent_current_step -> state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_APPLY_SUCCEEDED))
    {

        /* Find next step to download and install the next firmware.   */
        for (i = 0; i < update_manifest_content -> steps_count; i++)
        {
            if (update_manifest_content -> steps[i].state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_STARTED)
            {
                break;
            }
        }

        if (i < update_manifest_content -> steps_count)
        {

            /* Update the step to execute.  */
            adu_agent_ptr -> nx_azure_iot_adu_agent_current_step = &(update_manifest_content -> steps[i]);
        }
        else
        {

            /* All updates are downloaded and installed.  */
            if (adu_agent_ptr -> nx_azure_iot_adu_agent_update_notify)
            {

                /* Notify the user and let users control the update.  */
                adu_agent_ptr -> nx_azure_iot_adu_agent_update_notify(adu_agent_ptr,
                                                                      NX_AZURE_IOT_ADU_AGENT_UPDATE_INSTALLED,
                                                                      (UCHAR *)update_manifest_content -> update_id.provider,
                                                                      update_manifest_content -> update_id.provider_length,
                                                                      (UCHAR *)update_manifest_content -> update_id.name,
                                                                      update_manifest_content -> update_id.name_length,
                                                                      (UCHAR *)update_manifest_content -> update_id.version,
                                                                      update_manifest_content -> update_id.version_length);
            }
            else
            {

                /* Set event to apply update automatically.  */
                nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                          NX_AZURE_IOT_ADU_AGENT_APPLY_EVENT);
            }
            return;
        }
    }

    step = adu_agent_ptr -> nx_azure_iot_adu_agent_current_step;
    switch (step -> state)
    {
        case NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_STARTED:
        {

            /* Download firmware.  */
            LogInfo(LogLiteralArgs("Updating firmware..."));
            LogInfo(LogLiteralArgs("Manufacturer: %s"), step -> device  -> device_properties.manufacturer, step -> device  -> device_properties.manufacturer_length);
            LogInfo(LogLiteralArgs("Model: %s"), step -> device  -> device_properties.model, step -> device  -> device_properties.model_length);

            /* Start to download firmware for host update.  */
            if (nx_azure_iot_adu_agent_method_download(adu_agent_ptr, step -> file, NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE, 
                                                       NX_NULL, 0,
                                                       step -> device -> device_driver_entry))
            {
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
                return;
            }

            break;
        }

        case NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_SUCCEEDED:
        {

            /* Install firmware.  */
            if (nx_azure_iot_adu_agent_method_install(adu_agent_ptr, step -> device -> device_driver_entry))
            {
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
            }
            else
            {
                nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_INSTALL_SUCCEEDED);
            }

            break;
        }

        default:
        {
            break;
        }
    }

    return;
}

static VOID nx_azure_iot_adu_agent_apply_event_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *manifest_content = &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content);
NX_AZURE_IOT_ADU_AGENT_STEP *step;
UINT i;
UINT step_fail = NX_FALSE;


    /* Loop to apply the updates.  */
    for (i = 0; i < manifest_content -> steps_count; i++)
    {
        step = &(manifest_content -> steps[i]);

        /* Check the state.  */
        if (step -> state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_INSTALL_SUCCEEDED)
        {
            LogInfo(LogLiteralArgs("Applying firmware..."));
            LogInfo(LogLiteralArgs("Manufacturer: %s"), step -> device  -> device_properties.manufacturer, step -> device  -> device_properties.manufacturer_length);
            LogInfo(LogLiteralArgs("Model: %s"), step -> device  -> device_properties.model, step -> device  -> device_properties.model_length);

            /* Apply the update.  */
            nx_azure_iot_adu_agent_method_apply(adu_agent_ptr, manifest_content -> steps[i].device -> device_driver_entry);
        }
        else if (manifest_content -> steps[i].state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED)
        {
            step_fail = NX_TRUE;
            break;
        }
    }

    /* All steps done, update the state.  */
    if (step_fail == NX_TRUE)
    {
        adu_agent_ptr -> nx_azure_iot_adu_agent_state = NX_AZURE_IOT_ADU_AGENT_STATE_FAILED;
    }
    else
    {
        adu_agent_ptr -> nx_azure_iot_adu_agent_state = NX_AZURE_IOT_ADU_AGENT_STATE_IDLE;
    }

    /* Send reported properties to notify server.  */
    nx_azure_iot_adu_agent_reported_properties_state_send(adu_agent_ptr);
}

static UINT nx_azure_iot_adu_agent_manifest_verify(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
UINT   status;
UCHAR *header_b64;
UINT   header_b64_length;
UCHAR *payload_b64;
UINT   payload_b64_length;
UCHAR *signature_b64;
UINT   signature_b64_length;
UCHAR *jwk_header_b64;
UINT   jwk_header_b64_length;
UCHAR *jwk_payload_b64;
UINT   jwk_payload_b64_length;
UCHAR *jwk_signature_b64;
UINT   jwk_signature_b64_length;
UCHAR *signature;
UINT   signature_length;
UINT   sjwk_size = 0;
UCHAR *alg_ptr = NX_NULL;
UINT   alg_size = 0;
UCHAR *kid_ptr = NX_NULL;
UINT   kid_size = 0;
UCHAR *kty_ptr = NX_NULL;
UINT   kty_size = 0;
UCHAR *n_b64_ptr = NX_NULL;
UINT   n_b64_size = 0;
UCHAR *e_b64_ptr = NX_NULL;
UINT   e_b64_size = 0;
UCHAR *n_ptr = NX_NULL;
UINT   n_size = 0;
UCHAR *e_ptr = NX_NULL;
UINT   e_size = 0;
UCHAR *buffer_ptr;
UINT   buffer_size;
UINT   bytes_copied;
NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY *rsa_root_key;
NX_AZURE_IOT_ADU_AGENT_CRYPTO *adu_agent_crypto = &(adu_agent_ptr -> nx_azure_iot_adu_agent_crypto);
NX_AZURE_IOT_JSON_READER json_reader;
UCHAR  *sha256_generated_hash_ptr;
UCHAR  *sha256_decoded_hash_64_ptr;
UCHAR  *sha256_decoded_hash_ptr;

    /* The updateManifestSignature is used to ensure that the information contained within the updateManifest
       hasn't been tampered with. 
       https://docs.microsoft.com/en-us/azure/iot-hub-device-update/device-update-security#json-web-signature-jws  */

    /* JWS value format: BASE64URL(UTF8(header)) + "." + BASE64URL(UTF8(payload) + "." + BASE64URL(signature)).  */

    /* Step1. Parse JWS data.  */

    /* Header:
       {
           "alg": "RS256",
           "sjwk": "signed JWK"
       }

       Payload:
       {
           "sha256":"xxx...xxx"
       }

       Signature:
    */

    /* Initialize.  */
    alg_size = 0;
    sjwk_size = 0;
    buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_buffer;
    buffer_size = NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE;

    /* 1.1 Split header, payload and signature.  */
    if (nx_azure_iot_adu_agent_jws_split(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature,
                                         adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature_size,
                                         &header_b64, &header_b64_length,
                                         &payload_b64, &payload_b64_length,
                                         &signature_b64, &signature_b64_length) == NX_FALSE)
    {
        return(NX_FALSE);
    }

    /* 1.2 Decode header.  */
    if (_nx_utility_base64_decode(header_b64, header_b64_length,
                                  buffer_ptr, buffer_size, &bytes_copied))
    {
        return(NX_FALSE);
    }
    
    /* Initialize the header string as json.  */
    if (nx_azure_iot_json_reader_with_buffer_init(&json_reader, buffer_ptr, bytes_copied))
    {
        return(NX_FALSE);
    }
    buffer_ptr += bytes_copied;
    buffer_size -= bytes_copied;

    /* Skip the first begin object. */
    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
    {
        return(NX_FALSE);
    }

    /* Loop to process all data.  */
    while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {

            /* Get alg value.  */
            if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"alg", sizeof("alg") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &alg_size))
                {
                    return(NX_FALSE);
                }
                alg_ptr = buffer_ptr;
                buffer_ptr += alg_size;
                buffer_size -= alg_size;
            }

            /* Get sjwk value.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"sjwk", sizeof("sjwk") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_sjwk,
                                                              NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE,
                                                              &sjwk_size))
                {
                    return(NX_FALSE);
                }
            }
            else
            {
                return(NX_FALSE);
            }
        }
        else
        {
            break;
        }
    }

    /* Check if there are "alg" and "sjwk" properties.  */
    if ((alg_size == 0) || (sjwk_size == 0))
    {
        return(NX_FALSE);
    }

    /* Check if alg is supported.  */
    if ((alg_size != sizeof("RS256") - 1) || memcmp(alg_ptr, "RS256", alg_size))
    {
        return(NX_FALSE);
    }

    /* Step2. Verify signing key is signed by master key.  */
    
    /* Header:
       {
           "alg": "RS256",
           "kid": "ADU.200702.R"
       }
       
       Payload:
       {
           "kty": "RSA",
           "n": "xxx...xxx",
           "e": "AQAB",
           "alg": "RS256"
           "kid": "ADU.Signing.2020-04-29"
       }

       Signature:
    */

    /* Initialize.  */
    alg_size = 0;
    kid_size = 0;
    buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_buffer;
    buffer_size = NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE;

    /* 2.1 Split sjwk header, payload and signature.  */
    if (nx_azure_iot_adu_agent_jws_split(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_sjwk, sjwk_size,
                                         &jwk_header_b64, &jwk_header_b64_length,
                                         &jwk_payload_b64, &jwk_payload_b64_length,
                                         &jwk_signature_b64, &jwk_signature_b64_length) == NX_FALSE)
    {
        return(NX_FALSE);
    }

    /* 2.2 Decode sjwk header.  */
    if (_nx_utility_base64_decode(jwk_header_b64, jwk_header_b64_length,
                                  buffer_ptr, NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE, &bytes_copied))
    {
        return(NX_FALSE);
    }
    
    /* Initialize the header string as json.  */
    if (nx_azure_iot_json_reader_with_buffer_init(&json_reader, buffer_ptr, bytes_copied))
    {
        return(NX_FALSE);
    }
    buffer_ptr += bytes_copied;
    buffer_size -= bytes_copied;

    /* Skip the first begin object. */
    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
    {
        return(NX_FALSE);
    }

    /* Loop to process all header data.  */
    while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {

            /* Get alg value.  */
            if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"alg", sizeof("alg") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &alg_size))
                {
                    return(NX_FALSE);
                }
                alg_ptr = buffer_ptr;
                buffer_ptr += alg_size;
                buffer_size -= alg_size;
            }

            /* Get kid value.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"kid", sizeof("kid") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &kid_size))
                {
                    return(NX_FALSE);
                }
                kid_ptr = buffer_ptr;
                buffer_ptr += kid_size;
                buffer_size -= kid_size;
            }
            else
            {
                return(NX_FALSE);
            }
        }
        else
        {
            break;
        }
    }

    /* Check if there are "alg" and "kid" properties.  */
    if ((alg_size == 0) || (kid_size == 0))
    {
        return(NX_FALSE);
    }

    /* Check if alg is supported.  */
    if ((alg_size != sizeof("RS256") - 1) || memcmp(alg_ptr, "RS256", alg_size))
    {
        return(NX_FALSE);
    }

    /* Search master key.  */
    rsa_root_key = (NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY *)nx_azure_iot_adu_agent_rsa_root_key_find(kid_ptr, kid_size);
    if (rsa_root_key == NX_NULL)
    {
        return(NX_FALSE);
    }

    /* 2.3 Decode sjwk signature.  */
    signature = adu_agent_ptr -> nx_azure_iot_adu_agent_buffer;
    signature_length = NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE;
    if (_nx_utility_base64_decode(jwk_signature_b64, jwk_signature_b64_length,
                                  signature, signature_length, &signature_length))
    {
        return(NX_FALSE);
    }

    /* 2.4 Verify signature.  */
    if (nx_azure_iot_adu_agent_rs256_verify(&adu_agent_ptr -> nx_azure_iot_adu_agent_crypto,
                                            jwk_header_b64, (jwk_header_b64_length + 1 + jwk_payload_b64_length),
                                            signature, signature_length,
                                            (UCHAR *)rsa_root_key -> n, rsa_root_key -> n_size,
                                            (UCHAR *)rsa_root_key -> e, rsa_root_key -> e_size,
                                            adu_agent_ptr -> nx_azure_iot_adu_agent_buffer + signature_length,
                                            NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE - signature_length) == NX_FALSE)
    {
        return(NX_FALSE);
    }

    /* Step3. Verify distroman signature is signed by the signing key.  */

    /* Initialize.  */
    kty_size = 0;
    n_size = 0;
    e_size = 0;
    kid_size = 0;
    buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_buffer;
    buffer_size = NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE;

    /* 3.1 Decode sjwk payload to get the signing key.  */
    if (_nx_utility_base64_decode(jwk_payload_b64, jwk_payload_b64_length,
                                  buffer_ptr, buffer_size, &bytes_copied))
    {
        return(NX_FALSE);
    }
    
    /* Initialize the payload string as json.  */
    if (nx_azure_iot_json_reader_with_buffer_init(&json_reader, buffer_ptr, bytes_copied))
    {
        return(NX_FALSE);
    }
    buffer_ptr += bytes_copied;
    buffer_size -= bytes_copied;

    /* Skip the first begin object. */
    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
    {
        return(NX_FALSE);
    }

    /* Loop to process all header data.  */
    while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {

            /* Get kty value.  */
            if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"kty", sizeof("kty") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &kty_size))
                {
                    return(NX_FALSE);
                }
                kty_ptr = buffer_ptr;
                buffer_ptr += kty_size;
                buffer_size -= kty_size;
            }

            /* Get n value.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"n", sizeof("n") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &n_b64_size))
                {
                    return(NX_FALSE);
                }
                n_b64_ptr = buffer_ptr;
                buffer_ptr += n_b64_size;
                buffer_size -= n_b64_size;
            }
            
            /* Get e value.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"e", sizeof("e") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &e_b64_size))
                {
                    return(NX_FALSE);
                }
                e_b64_ptr = buffer_ptr;
                buffer_ptr += e_b64_size;
                buffer_size -= e_b64_size;
            }
            
            /* Get alg value.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"alg", sizeof("alg") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &alg_size))
                {
                    return(NX_FALSE);
                }
                alg_ptr = buffer_ptr;
                buffer_ptr += alg_size;
                buffer_size -= alg_size;
            }
            
            /* Get kid value.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"kid", sizeof("kid") - 1))
            {
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &kid_size))
                {
                    return(NX_FALSE);
                }
                kid_ptr = buffer_ptr;
                buffer_ptr += kid_size;
                buffer_size -= kid_size;
            }

            else
            {
                return(NX_FALSE);
            }
        }
        else
        {
            break;
        }
    }

    /* Check if there are "alg" and "kid" properties.  */
    if ((kty_size == 0) || (n_b64_size == 0) || (e_b64_size == 0) || (kid_size == 0))
    {
        return(NX_FALSE);
    }
    
    /* Check if alg is supported.  */
    if ((alg_size != sizeof("RS256") - 1) || memcmp(alg_ptr, "RS256", alg_size))
    {
        return(NX_FALSE);
    }

    /* Check if alg is supported.  */
    if ((kty_size != sizeof("RSA") - 1) || memcmp(kty_ptr, "RSA", kty_size))
    {
        return(NX_FALSE);
    }

    /* 3.2 Use sjwk to decode n, e, signature and verify signature.  */
    buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_sjwk;
    buffer_size = NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SJWK_SIZE;
    n_ptr = buffer_ptr;
    n_size = buffer_size;
    if (_nx_utility_base64_decode(n_b64_ptr, n_b64_size,
                                  n_ptr, n_size, &n_size))
    {
        return(NX_FALSE);
    }
    buffer_ptr += n_size;
    buffer_size -= n_size;

    e_ptr = buffer_ptr;
    e_size = buffer_size;
    if (_nx_utility_base64_decode(e_b64_ptr, e_b64_size,
                                  e_ptr, e_size, &e_size))
    {
        return(NX_FALSE);
    }
    buffer_ptr += e_size;
    buffer_size -= e_size;

    signature = buffer_ptr;
    signature_length = buffer_size;
    if (_nx_utility_base64_decode(signature_b64, signature_b64_length,
                                  signature, signature_length, &signature_length))
    {
        return(NX_FALSE);
    }
    buffer_ptr += signature_length;
    buffer_size -= signature_length;

    /* 3.3 Verify signature.  */
    if (nx_azure_iot_adu_agent_rs256_verify(&adu_agent_ptr -> nx_azure_iot_adu_agent_crypto,
                                            header_b64, (header_b64_length + 1 + payload_b64_length),
                                            signature, signature_length,
                                            n_ptr, n_size,
                                            e_ptr, e_size,
                                            buffer_ptr, buffer_size) == NX_FALSE)
    {
        return(NX_FALSE);
    }

    /* Step4. Verify distroman body digest (update manifest) matches what's in JWS payload section.  */

    /* Initialize.  */
    buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_buffer;
    buffer_size = NX_AZURE_IOT_ADU_AGENT_BUFFER_SIZE;

    /* 4.1 Calculate update manifest sha256 value.  */
    sha256_generated_hash_ptr = buffer_ptr;
    status = nx_azure_iot_adu_agent_sha256_calculate(adu_agent_crypto -> method_sha256,
                                                     adu_agent_crypto -> method_sha256_metadata,
                                                     NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE,
                                                     adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest,
                                                     adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_size,
                                                     sha256_generated_hash_ptr,
                                                     NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE);

    /* Check status.  */
    if (status)
    {
        return(NX_FALSE);
    }
    buffer_ptr += NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE;
    buffer_size -= NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE;

    /* 4.2 Decode the payload to get the sha256 base64 value.  */
    status = _nx_utility_base64_decode(payload_b64, payload_b64_length,
                                       buffer_ptr, buffer_size, &bytes_copied);

    /* Initialize the payload string as json.  */
    if (nx_azure_iot_json_reader_with_buffer_init(&json_reader, buffer_ptr, bytes_copied))
    {
        return(NX_FALSE);
    }
    buffer_ptr += bytes_copied;
    buffer_size -= bytes_copied;

    /* Skip the first begin object. */
    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
    {
        return(NX_FALSE);
    }

    /* Get the next token. */
    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME))
    {
        return(NX_FALSE);
    }
    sha256_decoded_hash_64_ptr = buffer_ptr;

    /* Get sha256 base64 value.  */
    if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader, (UCHAR *)"sha256", sizeof("sha256") - 1))
    {
        if (nx_azure_iot_json_reader_next_token(&json_reader) ||
            nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                      sha256_decoded_hash_64_ptr,
                                                      buffer_size,
                                                      &bytes_copied))
        {
            return(NX_FALSE);
        }
    }
    else
    {
        return(NX_FALSE);
    }
    buffer_ptr += bytes_copied;
    buffer_size -= bytes_copied;

    sha256_decoded_hash_ptr = buffer_ptr;

    /* Decode sha256 base64 hash.  */
    if (_nx_utility_base64_decode(sha256_decoded_hash_64_ptr, NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_BASE64_SIZE,
                                  sha256_decoded_hash_ptr, buffer_size, &bytes_copied))
    {
        return(NX_FALSE);
    }

    /* Verify the hash value.  */
    if (memcmp(sha256_generated_hash_ptr, sha256_decoded_hash_ptr, NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE))
    {
        return(NX_FALSE);
    }

    return(NX_TRUE);
}

static UINT nx_azure_iot_adu_agent_jws_split(UCHAR *jws, UINT jws_length,
                                             UCHAR **header, UINT *header_length,
                                             UCHAR **payload, UINT *payload_length,
                                             UCHAR **signature, UINT *signature_length)
{

UCHAR *dot1_pointer;
UCHAR *dot2_pointer;
UINT   dot_count = 0;
UINT   i = 0;

    /* Set the header pointer.  */
    *header = jws;

    /* Loop to find the dots.  */
    while(i < jws_length)
    {
        if (*jws == '.')
        {
            dot_count++;

            if (dot_count == 1)
            {
                dot1_pointer = jws;
            }
            else if (dot_count == 2)
            {
                dot2_pointer = jws;
            }
            else if (dot_count > 2)
            {
                return(NX_FALSE);
            }
        }
        jws++;
        i++;
    }

    /* Check if the dot count is correct.  */
    if ((dot_count != 2) || (dot2_pointer >= (*header + jws_length - 1)))
    {
        return(NX_FALSE);
    }

    /* Set the header, payload and signature.  */
    *header_length = (UINT)(dot1_pointer - *header);
    *payload = dot1_pointer + 1;
    *payload_length = (UINT)(dot2_pointer - *payload);
    *signature = dot2_pointer + 1;
    *signature_length = (UINT)((*header + jws_length) - *signature);

    return(NX_TRUE);
}

static VOID nx_azure_iot_adu_agent_step_state_update(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, UINT step_state)
{

    /* Update state.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_current_step -> state = step_state;

    /* Check the step state.  */
    if (step_state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED)
    {
        adu_agent_ptr -> nx_azure_iot_adu_agent_state = NX_AZURE_IOT_ADU_AGENT_STATE_FAILED;

        LogError(LogLiteralArgs("Failed to deploy update!"));

        /* Report state to server.  */
        nx_azure_iot_adu_agent_reported_properties_state_send(adu_agent_ptr);
    }
    else if (step_state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_MANIFEST_DOWNLOAD_SUCCEEDED)
    {

        /* Set event for update check.  */        
        nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                  NX_AZURE_IOT_ADU_AGENT_UPDATE_EVENT);
    }
    else if ((step_state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_DOWNLOAD_SUCCEEDED) ||
             (step_state == NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FIRMWARE_INSTALL_SUCCEEDED))
    {
        nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                  NX_AZURE_IOT_ADU_AGENT_DOWNLOAD_INSTALL_EVENT);
    }
}

static UINT nx_azure_iot_adu_agent_file_find(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, 
                                             NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *manifest_content,
                                             UCHAR *file_id, UINT file_id_length,
                                             NX_AZURE_IOT_ADU_AGENT_FILE **file)
{
UINT i;

    *file = NX_NULL;

    /* Find file.  */
    for (i = 0; i < manifest_content -> files_count; i++)
    {

        /* Check the file id.  */
        if ((file_id_length == manifest_content -> files[i].file_id_length) &&
            (!memcmp(file_id, manifest_content -> files[i].file_id, file_id_length)))
        {

            /* Find the file.  */
            *file = &(manifest_content -> files[i]);
            break;
        }
    }

    /* Check if find the file.  */
    if (*file == NX_NULL)
    {
        return(NX_AZURE_IOT_NOT_FOUND);
    }

    /* Find the file url.  */    
    for (i = 0; i < adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls_count; i++)
    {

        /* Check the file id.  */
        if ((file_id_length == adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls[i].file_id_length) &&
            (!memcmp(file_id, adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls[i].file_id, file_id_length)))
        {

            /* Find the file url.  */
            (*file) -> file_url = adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls[i].file_url;
            (*file) -> file_url_length = adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls[i].file_url_length;
            break;
        }
    }

    /* Check if find the file url.  */
    if (i >= adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls_count)
    {
        return(NX_AZURE_IOT_NOT_FOUND);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_adu_agent_method_is_installed(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                       NX_AZURE_IOT_ADU_AGENT_COMPATIBILITY *compatibility,
                                                       NX_AZURE_IOT_ADU_AGENT_STEP *step,
                                                       UINT *is_installed)
{
UINT i;
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;

    *is_installed = NX_FALSE;

    /* Check if compatibility and installed criteria in step are correct.  */
    if ((compatibility -> device_manufacturer == NX_NULL) ||
        (compatibility -> device_manufacturer_length == 0) ||
        (compatibility -> device_model == NX_NULL) ||
        (compatibility -> device_model_length == 0) ||
        (step -> installed_criteria == NX_NULL) ||
        (step -> installed_criteria_length == 0))
    {
        return(NX_AZURE_IOT_INVALID_PARAMETER);
    }

    /* Find the device according to compatibility.  */
    for (i = 0; i < NX_AZURE_IOT_ADU_AGENT_DEVICE_COUNT; i++)
    {

        /* Check if the device is valid.  */
        if (adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].valid == NX_FALSE)
        {
            continue;
        }

        /* Compare the compatility (manufacturer and model).  */
        if ((compatibility -> device_manufacturer_length == adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.manufacturer_length) &&
            (!memcmp(compatibility -> device_manufacturer, adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.manufacturer, compatibility -> device_manufacturer_length)) &&
            (compatibility -> device_model_length == adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.model_length) &&
            (!memcmp(compatibility -> device_model, adu_agent_ptr -> nx_azure_iot_adu_agent_device[i].device_properties.model, compatibility -> device_model_length)))
        {

            /* Set the matching device.  */
            step -> device = &adu_agent_ptr -> nx_azure_iot_adu_agent_device[i];
            break;
        }
    }

    /* Check if device with matching properties.  */
    if (step -> device == NX_NULL)
    {
        return(NX_AZURE_IOT_NOT_FOUND);
    }

    /* Check if agent has the installed criteria.  */
    if (step -> device -> installed_criteria_length)
    {

        /* Check if already installed this update.  */
        if ((step -> installed_criteria_length == step -> device -> installed_criteria_length) &&
            (!memcmp(step -> installed_criteria, step -> device -> installed_criteria, step -> installed_criteria_length)))
        {
            *is_installed = NX_TRUE;
        }
        else
        {
            *is_installed = NX_FALSE;
        }

    }
    else
    {

        /* Call the driver to check if this update is installed.  */
        driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_UPDATE_CHECK;
        driver_request.nx_azure_iot_adu_agent_driver_installed_criteria = step -> installed_criteria;
        driver_request.nx_azure_iot_adu_agent_driver_installed_criteria_length = step -> installed_criteria_length;
        driver_request.nx_azure_iot_adu_agent_driver_return_ptr = (ULONG *)is_installed;
        driver_request.nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
        (step -> device -> device_driver_entry)(&driver_request);

        /* Check status.  */
        if (driver_request.nx_azure_iot_adu_agent_driver_status)
        {
            return(NX_AZURE_IOT_FAILURE);
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_adu_agent_method_download(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                   NX_AZURE_IOT_ADU_AGENT_FILE *file,
                                                   UINT type, UCHAR *manifest_buffer_ptr, UINT manifest_buffer_size,
                                                   VOID (*adu_agent_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *))
{
UINT                status;
UCHAR              *buffer_ptr;
UINT                buffer_size;
NX_DNS             *dns_ptr;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr;
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;


    /* Check if include download file.  */
    if ((file -> file_url == NX_NULL) ||
        (file -> file_url_length == 0) ||
        (file -> file_sha256 == NX_NULL) ||
        (file -> file_sha256_length == 0) ||
        (file -> file_size_in_bytes == 0))
    {
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Check type.  */
    if (((type == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE) && (adu_agent_driver == NX_NULL)) ||
        ((type == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_MANIFEST) && ((manifest_buffer_ptr == NX_NULL) || (manifest_buffer_size == 0))))
    {
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Initialization.  */
    downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);
    dns_ptr = downloader_ptr -> dns_ptr;
    memset(downloader_ptr, 0, sizeof(NX_AZURE_IOT_ADU_AGENT_DOWNLOADER));
    downloader_ptr -> dns_ptr = dns_ptr;
    downloader_ptr -> file = file;
    downloader_ptr -> type = type;
    downloader_ptr -> manifest_buffer_ptr = manifest_buffer_ptr;
    downloader_ptr -> manifest_buffer_size = manifest_buffer_size;
    downloader_ptr -> driver_entry = adu_agent_driver;

    buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest;
    buffer_size = NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE;

    if (type == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE)
    {

        /* Output info.  */
        LogInfo(LogLiteralArgs("Firmware downloading..."));

        /* Send the preprocess request to the driver.   */
        driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_PREPROCESS;
        driver_request.nx_azure_iot_adu_agent_driver_firmware_size = file -> file_size_in_bytes;
        driver_request.nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
        (downloader_ptr -> driver_entry)(&driver_request);

        /* Check status.  */
        if (driver_request.nx_azure_iot_adu_agent_driver_status)
        {
            LogError(LogLiteralArgs("Firmware download fail: DRIVER PREPROCESS ERROR"));
            return(NX_AZURE_IOT_FAILURE);
        }
    }

    /* Parse the url.  */
    status = nx_azure_iot_adu_agent_file_url_parse(file -> file_url,
                                                   file -> file_url_length,
                                                   buffer_ptr, buffer_size, downloader_ptr);

    /* Check status.  */
    if (status)
    {
        LogError(LogLiteralArgs("Firmware download fail: URL PARSE ERROR"));
        return(status);
    }

    /* Check if start dns query to get the address.  */
    if (downloader_ptr -> state ==  NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_URL_PARSED)
    {

        /* Start dns query.  */
        nx_azure_iot_adu_agent_dns_query(adu_agent_ptr);
    }
    else if (downloader_ptr -> state == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_DONE)
    {

        /* Start HTTP connect.  */
        nx_azure_iot_adu_agent_http_connect(adu_agent_ptr);
    }

    /* Return.  */
    return(status);
}

static UINT nx_azure_iot_adu_agent_method_install(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                  VOID (*adu_agent_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *))
{
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;

    NX_PARAMETER_NOT_USED(adu_agent_ptr);

    /* Output info.  */
    LogInfo(LogLiteralArgs("Firmware installing..."));

    /* Send the firmware install request to the driver.   */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_INSTALL;
    driver_request.nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
    (adu_agent_driver)(&driver_request);
    
    /* Install firmware.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        LogError(LogLiteralArgs("Firmware install fail: DRIVER ERROR"));
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Output info.  */
    LogInfo(LogLiteralArgs("Firmware installed"));

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_adu_agent_method_apply(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                VOID (*adu_agent_driver)(NX_AZURE_IOT_ADU_AGENT_DRIVER *))
{
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;

    NX_PARAMETER_NOT_USED(adu_agent_ptr);

    /* Applying...  */
    LogInfo(LogLiteralArgs("Firmware applying..."));

    /* Send the firmware apply request to the driver.   */
    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_APPLY;
    (adu_agent_driver)(&driver_request);

    /* Install firmware.  */
    if (driver_request.nx_azure_iot_adu_agent_driver_status)
    {
        LogError(LogLiteralArgs("Firmware apply fail: DRIVER ERROR"));
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Applying host update will reboot the device and never return.  */
    LogInfo(LogLiteralArgs("Firmware applied\r\n\r\n"));

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_adu_agent_service_properties_get(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                          NX_AZURE_IOT_JSON_READER *json_reader_ptr)
{
UCHAR *file_buffer_ptr;
UINT file_buffer_size;
NX_AZURE_IOT_ADU_AGENT_FILE_URLS *file_urls = &(adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls);
NX_AZURE_IOT_ADU_AGENT_WORKFLOW *workflow = &(adu_agent_ptr -> nx_azure_iot_adu_agent_workflow);


    /* Initialization.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_size = 0;
    adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature_size = 0;
    memset(file_urls, 0, sizeof (NX_AZURE_IOT_ADU_AGENT_FILE_URLS));
    memset(workflow, 0, sizeof (NX_AZURE_IOT_ADU_AGENT_WORKFLOW));

    /* Skip service property.  */
    nx_azure_iot_json_reader_next_token(json_reader_ptr);

    /* Next one should be begin object.  */
    if (nx_azure_iot_json_reader_token_type(json_reader_ptr) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Loop to process all data.  */
    while (nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {

            /* Workflow.  */
            if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                             (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW,
                                                             sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW) - 1))
            {

                /*  Skip the workflow property name.  */
                if (nx_azure_iot_json_reader_next_token(json_reader_ptr))
                {
                    return(NX_NOT_SUCCESSFUL);
                }

                /* Check the token type.  */
                if (nx_azure_iot_json_reader_token_type(json_reader_ptr) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
                {
                    return(NX_NOT_SUCCESSFUL);
                }

                /* Loop to process workflow content.  */
                while (nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_AZURE_IOT_SUCCESS)
                {
                    if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
                    {
                        if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                                         (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION,
                                                                         sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION) - 1))
                        {
                            /* Get action data.  */
                            if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
                                nx_azure_iot_json_reader_token_int32_get(json_reader_ptr, (int32_t *)&(workflow -> action)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }

                        /* Get id.  */
                        else if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                                              (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID,
                                                                              sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID) - 1))
                        {

                            /* Get id string.  */
                            if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
                                nx_azure_iot_json_reader_token_string_get(json_reader_ptr,
                                                                          workflow -> id,
                                                                          NX_AZURE_IOT_ADU_AGENT_WORKFLOW_ID_SIZE,
                                                                          &(workflow -> id_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }

                        /* Get retry timestamp.  */
                        else if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                                              (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP,
                                                                              sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP) - 1))
                        {

                            /* Get retry timestamp string.  */
                            if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
                                nx_azure_iot_json_reader_token_string_get(json_reader_ptr,
                                                                          workflow -> retry_timestamp,
                                                                          NX_AZURE_IOT_ADU_AGENT_WORKFLOW_RETRY_TIMESTAMP_SIZE,
                                                                          &(workflow -> retry_timestamp_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }

                        /* Skip the unknown properties.  */
                        else
                        {
                            if (nx_azure_iot_json_reader_skip_children(json_reader_ptr))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }
                    }
                    else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                    {
                        break;
                    }
                }
            }

            /* Update manifest.  */
            if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                                (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST,
                                                                sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST) - 1))
            {

                /* Get update manifest string.  */
                if ((nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_SUCCESS) &&
                    (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_STRING))
                {
                    if (nx_azure_iot_json_reader_token_string_get(json_reader_ptr,
                                                                  adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest,
                                                                  NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE,
                                                                  &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_size)))
                    {
                        return(NX_NOT_SUCCESSFUL);
                    }
                }
            }

            /* Update manifest signature.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                                    (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST_SIGNATURE,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST_SIGNATURE) - 1))
            {

                /* Get update manifest signature.  */
                if ((nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_SUCCESS) &&
                    (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_STRING))
                {
                    if (nx_azure_iot_json_reader_token_string_get(json_reader_ptr,
                                                                  adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature,
                                                                  NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIGNATURE_SIZE,
                                                                  &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_signature_size)))
                    {
                        return(NX_NOT_SUCCESSFUL);
                    }
                }
            }

            /* File URLs. 
                Note: 1. file urls property can exist or not.
                        2. file urls property value can be object.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(json_reader_ptr,
                                                                    (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILEURLS,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILEURLS) - 1))
            {

                /*  Skip the file urls property name.  */
                if (nx_azure_iot_json_reader_next_token(json_reader_ptr))
                {
                    return(NX_NOT_SUCCESSFUL);
                }

                /* Check the token type.  */
                if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
                {

                    /* Start to parse file array.  */
                    file_buffer_ptr = adu_agent_ptr -> nx_azure_iot_adu_agent_file_urls.file_urls_buffer;
                    file_buffer_size = NX_AZURE_IOT_ADU_AGENT_FILE_URLS_MAX;

                    while (nx_azure_iot_json_reader_next_token(json_reader_ptr) == NX_AZURE_IOT_SUCCESS)
                    {

                        /* Check if it is the end object.  */ 
                        if (nx_azure_iot_json_reader_token_type(json_reader_ptr) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                        {
                            break;
                        }

                        if (file_urls -> file_urls_count < NX_AZURE_IOT_ADU_AGENT_FILES_MAX)
                        {

                            /* Store the file number.  */
                            if (nx_azure_iot_json_reader_token_string_get(json_reader_ptr,
                                                                            file_buffer_ptr,
                                                                            file_buffer_size,
                                                                            &(file_urls -> file_urls[file_urls -> file_urls_count].file_id_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            /* Set file number pointer and update the buffer size.  */
                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(file_urls -> file_urls[file_urls -> file_urls_count].file_id,
                                                                file_urls -> file_urls[file_urls -> file_urls_count].file_id_length,
                                                                file_buffer_ptr, file_buffer_size);

                            /* Get file url.  */
                            if (nx_azure_iot_json_reader_next_token(json_reader_ptr) ||
                                nx_azure_iot_json_reader_token_string_get(json_reader_ptr,
                                                                            file_buffer_ptr,
                                                                            file_buffer_size,
                                                                            &(file_urls -> file_urls[file_urls -> file_urls_count].file_url_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            /* Set file url pointer and update the buffer size.  */
                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(file_urls -> file_urls[file_urls -> file_urls_count].file_url,
                                                                file_urls -> file_urls[file_urls -> file_urls_count].file_url_length,
                                                                file_buffer_ptr, file_buffer_size);

                            file_urls -> file_urls_count++;
                        }
                        else
                        {
                            if (nx_azure_iot_json_reader_next_token(json_reader_ptr))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }
                    }
                }
            }

            /* Skip the unknown properties.  */
            else
            {
                if (nx_azure_iot_json_reader_skip_children(json_reader_ptr))
                {
                    return(NX_NOT_SUCCESSFUL);
                }
            }
        }
        else if (nx_azure_iot_json_reader_token_type(json_reader_ptr) ==
                    NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
        {
            break;
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

static UINT nx_azure_iot_adu_agent_service_update_manifest_process(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                                   UCHAR *update_manifest,
                                                                   UINT update_manifest_size,
                                                                   NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *update_manifest_content,
                                                                   UCHAR *update_manifest_content_buffer,
                                                                   UINT update_manifest_content_buffer_size)
{

UCHAR *buffer_ptr = update_manifest_content_buffer;
UINT buffer_size = update_manifest_content_buffer_size;
UINT i = 0;
NX_AZURE_IOT_JSON_READER json_reader;
NX_AZURE_IOT_ADU_AGENT_COMPATIBILITY *compatibility = &(update_manifest_content -> compatibility);

    NX_PARAMETER_NOT_USED(adu_agent_ptr);

    /* Initialization.  */
    memset(update_manifest_content, 0, sizeof (NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT));

    /* Initialize the update manifest string as json.  */
    if (nx_azure_iot_json_reader_with_buffer_init(&json_reader, update_manifest, update_manifest_size))
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Skip the first begin object. */
    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Loop to process all data.  */
    while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
    {
        if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
        {

            /* Manifest version.  */
            if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                             (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MANIFEST_VERSION,
                                                             sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MANIFEST_VERSION) - 1))
            {

                /* Get manifest version value.  */
                if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                    nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                              buffer_ptr,
                                                              buffer_size,
                                                              &(update_manifest_content -> manifest_version_length)))
                {
                    return(NX_NOT_SUCCESSFUL);
                }

                /* Set file number pointer and update the buffer size.  */
                NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> manifest_version,
                                                  update_manifest_content -> manifest_version_length,
                                                  buffer_ptr, buffer_size);
            }

            /* Update id.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                  (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_ID,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_ID) - 1))
            {

                /* Skip the first begin object. */
                if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
                {
                    return(NX_NOT_SUCCESSFUL);
                }

                /* Loop to process all update id field.  */
                while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
                {
                    if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
                    {

                        /* Provider.  */
                        if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                         (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_PROVIDER,
                                                                         sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_PROVIDER) - 1))
                        {

                            /* Get the provider value.  */
                            if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                          buffer_ptr,
                                                                          buffer_size,
                                                                          &(update_manifest_content -> update_id.provider_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            /* Set file number pointer and update the buffer size.  */
                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> update_id.provider,
                                                              update_manifest_content -> update_id.provider_length,
                                                              buffer_ptr, buffer_size);
                        }
                        
                        /* Name.  */
                        else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                              (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_NAME,
                                                                              sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_NAME) - 1))
                        {

                            /* Get the name value.  */
                            if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                          buffer_ptr,
                                                                          buffer_size,
                                                                          &(update_manifest_content -> update_id.name_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            /* Set file number pointer and update the buffer size.  */
                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> update_id.name,
                                                              update_manifest_content -> update_id.name_length,
                                                              buffer_ptr, buffer_size);
                        }

                        /* Version.  */
                        else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                              (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_VERSION,
                                                                              sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_VERSION) - 1))
                        {

                            /* Get the version value.  */
                            if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                          buffer_ptr,
                                                                          buffer_size,
                                                                          &(update_manifest_content -> update_id.version_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            /* Set file number pointer and update the buffer size.  */
                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> update_id.version,
                                                              update_manifest_content -> update_id.version_length,
                                                              buffer_ptr, buffer_size);
                        }

                        /* Skip the unknown properties.  */
                        else
                        {
                            if (nx_azure_iot_json_reader_skip_children(&json_reader))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }
                    }

                    else if (nx_azure_iot_json_reader_token_type(&json_reader) ==
                                NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
                    {
                        if (nx_azure_iot_json_reader_skip_children(&json_reader))
                        {
                            return(NX_NOT_SUCCESSFUL);
                        }
                    }
                    else if (nx_azure_iot_json_reader_token_type(&json_reader) ==
                                NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                    {
                        break;
                    }
                }
            }

            /* Compatibility.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                  (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_COMPATIBILITY,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_COMPATIBILITY) - 1))
            {
                if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_ARRAY) ||
                    (nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
                {
                    return(NX_NOT_SUCCESSFUL);
                }
                
                while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
                {
                    if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
                    {

                        /* Device manufacturer.  */
                        if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                         (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICE_MANUFACTURER,
                                                                         sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICE_MANUFACTURER) - 1))
                        {
                            if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                          buffer_ptr,
                                                                          buffer_size,
                                                                          &(compatibility -> device_manufacturer_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(compatibility -> device_manufacturer,
                                                              compatibility -> device_manufacturer_length,
                                                              buffer_ptr, buffer_size);
                        }

                        /* Device model.  */
                        else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                              (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICE_MODEL,
                                                                              sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICE_MODEL) - 1))
                        {
                            if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                          buffer_ptr,
                                                                          buffer_size,
                                                                          &(compatibility -> device_model_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(compatibility -> device_model,
                                                              compatibility -> device_model_length,
                                                              buffer_ptr, buffer_size);
                        }

                        /* Group.  */
                        else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                              (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_GROUP,
                                                                              sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_GROUP) - 1))
                        {
                            if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                          buffer_ptr,
                                                                          buffer_size,
                                                                          &(compatibility -> group_length)))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }

                            NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(compatibility -> group,
                                                              compatibility -> group_length,
                                                              buffer_ptr, buffer_size);
                        }

                        /* Skip the unknown properties.  */
                        else
                        {
                            if (nx_azure_iot_json_reader_skip_children(&json_reader))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                        }
                    }
                    else if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                    {
                        break;
                    }
                }

                if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_END_ARRAY))
                {
                    return(NX_NOT_SUCCESSFUL);
                }
            }

            /* Instructions.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                  (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTRUCTIONS,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTRUCTIONS) - 1))
            {

                /* Skip the first begin object. */
                if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT) ||
                    (nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                  (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STEPS,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STEPS) - 1) != NX_TRUE) ||
                    (nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_ARRAY))
                {
                    return(NX_NOT_SUCCESSFUL);
                }

                i = 0;

                /* Loop to process steps array.  */
                while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
                {
                    if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_END_ARRAY)
                    {
                        break;
                    }

                    if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
                    {

                        /* Check if have enough space.  */
                        if (update_manifest_content -> steps_count >= NX_AZURE_IOT_ADU_AGENT_STEPS_MAX)
                        {
                            if (nx_azure_iot_json_reader_skip_children(&json_reader))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                            continue;
                        }

                        /* Loop to process properties in step.  */
                        while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
                        {
                            if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
                            {

                                /* Type.  */
                                if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                 (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_TYPE,
                                                                                 sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_TYPE) - 1))
                                {

                                    /* Get type value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> steps[i].type_length)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    /* Set file number pointer and update the buffer size.  */
                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> steps[i].type,
                                                                      update_manifest_content -> steps[i].type_length,
                                                                      buffer_ptr, buffer_size);
                                }

                                /* Handler.  */
                                else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HANDLE,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HANDLE) - 1))
                                {

                                    /* Get handle value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> steps[i].handler_length)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> steps[i].handler,
                                                                      update_manifest_content -> steps[i].handler_length,
                                                                      buffer_ptr, buffer_size);
                                }

                                /* Handler properties: installedCriteria.  */
                                else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HANDLER_PROPERTIES,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HANDLER_PROPERTIES) - 1))
                                {

                                    /* Skip the installedCriteria property name. */
                                    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                                        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT) ||
                                        (nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                                        (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTALLED_CRITERIA,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTALLED_CRITERIA) - 1) != NX_TRUE))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    /* Get installedCriteria value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> steps[i].installed_criteria_length)) ||
                                        (nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                                        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_END_OBJECT))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> steps[i].installed_criteria,
                                                                      update_manifest_content -> steps[i].installed_criteria_length,
                                                                      buffer_ptr, buffer_size);
                                }

                                /* Files.  */
                                else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILES,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILES) - 1))
                                {

                                    /* Get files value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_ARRAY) ||
                                        nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> steps[i].file_id_length)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> steps[i].file_id,
                                                                      update_manifest_content -> steps[i].file_id_length,
                                                                      buffer_ptr, buffer_size);

                                    /* Skip end array.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_END_ARRAY))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }
                                }

                                /* Detached Manifest File Id.  */
                                else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DETACHED_MANIFEST_FILED,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DETACHED_MANIFEST_FILED) - 1))
                                {

                                    /* Get files value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> steps[i].file_id_length)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> steps[i].file_id,
                                                                      update_manifest_content -> steps[i].file_id_length,
                                                                      buffer_ptr, buffer_size);
                                }

                                /* Skip the unknown properties.  */
                                else
                                {
                                    if (nx_azure_iot_json_reader_skip_children(&json_reader))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }
                                }
                            }
                            else if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                            {
                                i++;
                                update_manifest_content -> steps_count++;
                                break;
                            }
                        }
                    }
                }

                /* Skip the end object of steps. */
                if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                    (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_END_OBJECT))
                {
                    return(NX_NOT_SUCCESSFUL);
                }
            }

            /* Files.  */
            else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                  (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILES,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILES) - 1))
            {
                i = 0;
                while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
                {
                    if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
                    {
                        if (update_manifest_content -> files_count >= NX_AZURE_IOT_ADU_AGENT_FILES_MAX)
                        {
                            if (nx_azure_iot_json_reader_skip_children(&json_reader))
                            {
                                return(NX_NOT_SUCCESSFUL);
                            }
                            continue;
                        }

                        /* Get the file id.  */
                        if (nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                      buffer_ptr,
                                                                      buffer_size,
                                                                      &(update_manifest_content -> files[i].file_id_length)))
                        {
                            return(NX_NOT_SUCCESSFUL);
                        }

                        /* Set file id pointer and update the buffer size.  */
                        NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> files[i].file_id,
                                                          update_manifest_content -> files[i].file_id_length,
                                                          buffer_ptr, buffer_size);

                        while (nx_azure_iot_json_reader_next_token(&json_reader) == NX_AZURE_IOT_SUCCESS)
                        {
                            if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_PROPERTY_NAME)
                            {

                                /* Filename.  */
                                if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                 (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILE_NAME,
                                                                                 sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_FILE_NAME) - 1))
                                {

                                    /* Get the file name value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> files[i].file_name_length)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    /* Set file name pointer and update the buffer size.  */
                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> files[i].file_name,
                                                                      update_manifest_content -> files[i].file_name_length,
                                                                      buffer_ptr, buffer_size);
                                }
                                
                                /* Size in bytes.  */
                                else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SIZE_IN_BYTES,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SIZE_IN_BYTES) - 1))
                                {
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_uint32_get(&json_reader,
                                                                                  (uint32_t *)&(update_manifest_content -> files[i].file_size_in_bytes)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }
                                }

                                /* Hashes.  */
                                else if (nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HASHES,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_HASHES) - 1))
                                {

                                    /* Skip the begin object of hashes property. */
                                    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                                        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    /* sha256.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        !nx_azure_iot_json_reader_token_is_text_equal(&json_reader,
                                                                                      (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SHA256,
                                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SHA256) - 1))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    /* Get the sha256 value value.  */
                                    if (nx_azure_iot_json_reader_next_token(&json_reader) ||
                                        nx_azure_iot_json_reader_token_string_get(&json_reader,
                                                                                  buffer_ptr,
                                                                                  buffer_size,
                                                                                  &(update_manifest_content -> files[i].file_sha256_length)))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }

                                    NX_AZURE_IOT_ADU_AGENT_PTR_UPDATE(update_manifest_content -> files[i].file_sha256,
                                                                      update_manifest_content -> files[i].file_sha256_length,
                                                                      buffer_ptr, buffer_size);

                                    /* Skip the end object of hashes property. */
                                    if ((nx_azure_iot_json_reader_next_token(&json_reader) != NX_AZURE_IOT_SUCCESS) ||
                                        (nx_azure_iot_json_reader_token_type(&json_reader) != NX_AZURE_IOT_READER_TOKEN_END_OBJECT))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }
                                }

                                /* Skip the unknown properties.  */
                                else
                                {
                                    if (nx_azure_iot_json_reader_skip_children(&json_reader))
                                    {
                                        return(NX_NOT_SUCCESSFUL);
                                    }
                                }
                            }
                            else if (nx_azure_iot_json_reader_token_type(&json_reader) == NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                            {
                                i++;
                                update_manifest_content -> files_count++;
                                break;
                            }
                        }
                    }
                    else if (nx_azure_iot_json_reader_token_type(&json_reader) ==
                                NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
                    {
                        break;
                    }
                }
            }

            /* Skip the unknown properties.  */
            else
            {
                if (nx_azure_iot_json_reader_skip_children(&json_reader))
                {
                    return(NX_NOT_SUCCESSFUL);
                }
            }
        }

        else if (nx_azure_iot_json_reader_token_type(&json_reader) ==
                    NX_AZURE_IOT_READER_TOKEN_BEGIN_OBJECT)
        {
            if (nx_azure_iot_json_reader_skip_children(&json_reader))
            {
                return(NX_NOT_SUCCESSFUL);
            }
        }
        else if (nx_azure_iot_json_reader_token_type(&json_reader) ==
                    NX_AZURE_IOT_READER_TOKEN_END_OBJECT)
        {
            break;
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

/* agent reported properties for startup:
{
    "deviceUpdate": {
        "__t": "c",
        "agent": {
            "deviceProperties": {
                "manufacturer": "Microsoft",
                "model": "MS-Board",
                "interfaceId": "dtmi:azure:iot:deviceUpdate;1",
            },
            "compatPropertyNames": "manufacturer,model",
        }
    }
}
*/
static UINT nx_azure_iot_adu_agent_reported_properties_startup_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, UINT wait_option)
{

NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_WRITER json_writer;
NX_AZURE_IOT_ADU_AGENT_DEVICE_PROPERTIES *device_properties = &(adu_agent_ptr -> nx_azure_iot_adu_agent_device_properties);
UINT status;
UINT response_status;

    /* Create json writer for client reported property.  */
    status = nx_azure_iot_hub_client_reported_properties_create(adu_agent_ptr -> nx_azure_iot_hub_client_ptr, &packet_ptr, wait_option);
    if (status)
    {
        return(status);
    }

    /* Init json writer.  */
    status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, wait_option);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Append begin object.  */
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the ADU agent component name.  */
    status = nx_azure_iot_hub_client_reported_properties_component_begin(adu_agent_ptr -> nx_azure_iot_hub_client_ptr,
                                                                         &json_writer, 
                                                                         (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME,
                                                                         sizeof(NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME) - 1);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Fill the agent property name.  */
    if (nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                      (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_AGENT,
                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_AGENT) - 1))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Start to fill agent property value.   */
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the deviceProperties.  */
    if ((nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                        (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICEPROPERTIES,
                                                        sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_DEVICEPROPERTIES) - 1)) ||
        (nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
        (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                    (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MANUFACTURER,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MANUFACTURER) - 1,
                                                                    device_properties -> manufacturer, device_properties -> manufacturer_length)) ||
        (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                    (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MODEL,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_MODEL) - 1,
                                                                    device_properties -> model, device_properties -> model_length)) ||
        (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                    (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INTERFACE_ID,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INTERFACE_ID) - 1,
                                                                    (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_INTERFACE_ID,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_INTERFACE_ID) - 1)) ||
        (nx_azure_iot_json_writer_append_end_object(&json_writer)))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the comatability property.  */
    if (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                   (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_COMPAT_PROPERTY_NAMES,
                                                                   sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_COMPAT_PROPERTY_NAMES) - 1,
                                                                   (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_VALUE_COMPATIBILITY,
                                                                   sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_VALUE_COMPATIBILITY) - 1))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* End the client property value.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* End ADU agent component.  */
    if (nx_azure_iot_hub_client_reported_properties_component_end(adu_agent_ptr -> nx_azure_iot_hub_client_ptr, &json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* End json object.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Send agent reported properties startup message to IoT Hub.  */
    status = nx_azure_iot_hub_client_reported_properties_send(adu_agent_ptr -> nx_azure_iot_hub_client_ptr,
                                                              packet_ptr,
                                                              NX_NULL, &response_status,
                                                              NX_NULL, wait_option);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Check the response statue for blocking.  */
    if (wait_option)
    {
        if ((response_status < 200) || (response_status >= 300))
        {
            return(NX_NOT_SUCCESSFUL);
        }
    }

    return(NX_AZURE_IOT_SUCCESS);
}

/* agent reported properties state:
{
    "deviceUpdate": {
        "__t": "c",
        "agent": {
            "state": 0,
            "workflow": {
                "action": 3,
                "id": "someguid",
                "retryTimestamp": "2020-04-22T12:12:12.0000000+00:00"
            },
            "installedUpdateId": "{\"provider\":\"Microsoft\",\"Name\":\"MS-Board\",\"Version\":\"1.0.0\"}",,
            "lastInstallResult": {
                "resultCode": 700,
                "extendedResultCode": 0,
                "resultDetails": "",
                "stepResults": {
                    "step_0": {
                        "resultCode": 700,
                        "extendedResultCode": 0,
                        "resultDetails": ""
                    }
                }
            }
        }
    }
}
*/
static UINT nx_azure_iot_adu_agent_reported_properties_state_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{

NX_PACKET *packet_ptr;
NX_AZURE_IOT_JSON_WRITER json_writer;
NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_CONTENT *manifest_content = &(adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content);
UINT status;
UINT result_code;
UINT i;
/* Prepare the buffer for step name: such as: "step_0", the max name is "step_xxx".  */
CHAR step_property_name[8] = "step_";
UINT step_size = sizeof("step_") - 1;
UINT step_property_name_size;
UINT update_id_length;

    /* Create json writer for client reported property.  */
    status = nx_azure_iot_hub_client_reported_properties_create(adu_agent_ptr -> nx_azure_iot_hub_client_ptr, &packet_ptr, NX_NO_WAIT);
    if (status)
    {
        return(status);
    }

    /* Init json writer.  */
    status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, NX_NO_WAIT);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Append begin object.  */
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the ADU agent component name.  */
    status = nx_azure_iot_hub_client_reported_properties_component_begin(adu_agent_ptr -> nx_azure_iot_hub_client_ptr,
                                                                         &json_writer, 
                                                                         (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME,
                                                                         sizeof(NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME) - 1);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Fill the agent property name.  */
    if (nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                      (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_AGENT,
                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_AGENT) - 1))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Start to fill agent property value.   */
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the state.   */
    if (nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                  (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STATE,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STATE) - 1,
                                                                  (INT)adu_agent_ptr -> nx_azure_iot_adu_agent_state))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the workflow.  */
    if (adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.id_length)
    {
        if (nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                            (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW,
                                                            sizeof (NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW) - 1) ||
            nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
            nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                      (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION,
                                                                      sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION) - 1,
                                                                      (INT)adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.action) ||
            nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID) - 1,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.id,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.id_length))
        {
            nx_packet_release(packet_ptr);
            return (NX_NOT_SUCCESSFUL);
        }

        /* End workflow object.  */
        if (nx_azure_iot_json_writer_append_end_object(&json_writer))
        {
            nx_packet_release(packet_ptr);
            return(NX_NOT_SUCCESSFUL);
        }
    }

    /* Append retry timestamp in workflow if existed.  */
    if (adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.retry_timestamp_length)
    {
        if (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP) - 1,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.retry_timestamp,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.retry_timestamp_length))
        {
            nx_packet_release(packet_ptr);
            return (NX_NOT_SUCCESSFUL);
        }
    }

    /* Fill installed update id.  */
    if ((adu_agent_ptr -> nx_azure_iot_adu_agent_state == NX_AZURE_IOT_ADU_AGENT_STATE_IDLE) && 
        (adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content.steps_count))
    {

        /* Use nx_azure_iot_adu_agent_update_manifest as temporary buffer to encode the update id as string.*/
        update_id_length = (UINT)snprintf((CHAR *)adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest,
                                          NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE,
                                          "{\"%.*s\":\"%.*s\",\"%.*s\":\"%.*s\",\"%.*s\":\"%.*s\"}",
                                          sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_PROVIDER) - 1,
                                          NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_PROVIDER,
                                          manifest_content -> update_id.provider_length, manifest_content -> update_id.provider, 
                                          sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_NAME) - 1,
                                          NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_NAME,
                                          manifest_content -> update_id.name_length, manifest_content -> update_id.name,
                                          sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_VERSION) - 1,
                                          NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_VERSION,
                                          manifest_content -> update_id.version_length, manifest_content -> update_id.version);

        if (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTALLED_CONTENT_ID,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_INSTALLED_CONTENT_ID) - 1,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest,
                                                                       update_id_length))
        {
            nx_packet_release(packet_ptr);
            return (NX_NOT_SUCCESSFUL);
        }
    }

    /* Fill the last install result.  */
    if ((adu_agent_ptr -> nx_azure_iot_adu_agent_state != NX_AZURE_IOT_ADU_AGENT_STATE_DEPLOYMENT_IN_PROGRESS) && 
        (adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content.steps_count))
    {

        /* Check the state.  */
        if (adu_agent_ptr -> nx_azure_iot_adu_agent_state == NX_AZURE_IOT_ADU_AGENT_STATE_IDLE)
        {
            result_code = NX_AZURE_IOT_ADU_AGENT_RESULT_CODE_APPLY_SUCCESS;
        }
        else
        {
            result_code = NX_AZURE_IOT_ADU_AGENT_RESULT_CODE_FAILURE;
        }

        if ((nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                           (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_LAST_INSTALL_RESULT,
                                                           sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_LAST_INSTALL_RESULT) - 1)) ||
            (nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
            (nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_CODE,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_CODE) - 1,
                                                                       (int32_t)result_code)) ||
            (nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE) - 1,
                                                                       0)) ||
            (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_DETAILS,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_DETAILS) - 1,
                                                                       NX_NULL, 0)) ||
            (nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                           (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STEP_RESULTS,
                                                           sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_STEP_RESULTS) - 1)) ||
            (nx_azure_iot_json_writer_append_begin_object(&json_writer)))
        {
            nx_packet_release(packet_ptr);
            return(NX_NOT_SUCCESSFUL);
        }

        /* Loop to fill the step results.  */
        for (i = 0; i < adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_content.steps_count; i++)
        {
            if ((step_property_name_size = _nx_utility_uint_to_string(i, 10, &step_property_name[step_size], 8 - step_size)) == 0)
            {
                nx_packet_release(packet_ptr);
                return(NX_NOT_SUCCESSFUL);
            }
            step_property_name_size += step_size;

            if ((nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                               (const UCHAR *)step_property_name,
                                                               step_property_name_size)) ||
                (nx_azure_iot_json_writer_append_begin_object(&json_writer)) ||
                (nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                           (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_CODE,
                                                                           sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_CODE) - 1,
                                                                           (int32_t)result_code)) ||
                (nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                           (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE,
                                                                           sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_EXTENDED_RESULT_CODE) - 1,
                                                                           0)) ||
                (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                           (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_DETAILS,
                                                                           sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RESULT_DETAILS) - 1,
                                                                           NX_NULL, 0)) ||
                (nx_azure_iot_json_writer_append_end_object(&json_writer)))
            {
                nx_packet_release(packet_ptr);
                return(NX_NOT_SUCCESSFUL);
            }
        }

        if ((nx_azure_iot_json_writer_append_end_object(&json_writer)) ||
            (nx_azure_iot_json_writer_append_end_object(&json_writer)))
        {
            nx_packet_release(packet_ptr);
            return(NX_NOT_SUCCESSFUL);
        }
    }

    /* End the client property value.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* End ADU agent component.  */
    if (nx_azure_iot_hub_client_reported_properties_component_end(adu_agent_ptr -> nx_azure_iot_hub_client_ptr, &json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* End json object.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Send device info reported properties message to IoT Hub.  */
    status = nx_azure_iot_hub_client_reported_properties_send(adu_agent_ptr -> nx_azure_iot_hub_client_ptr,
                                                              packet_ptr,
                                                              NX_NULL, NX_NULL,
                                                              NX_NULL, NX_NO_WAIT);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

/* service reported properties sample:

    {
        "azureDeviceUpdateAgent": {
            "__t": "c",
            "<service>": {
                "ac": <status_code>,
                "av": <version>,
                "ad": "<description>",
                "value": <user_value>
            }
        }
    }
*/

/**
 * @brief Send service reported properties.
 * 
 * @param updateState state to report.
 * @param result Result to report (optional, can be NULL).
 */
static UINT nx_azure_iot_adu_agent_service_reported_properties_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr,
                                                                    UINT status_code, ULONG version, const CHAR *description,
                                                                    ULONG wait_option)
{
NX_AZURE_IOT_HUB_CLIENT *iothub_client_ptr = adu_agent_ptr -> nx_azure_iot_hub_client_ptr;
NX_AZURE_IOT_JSON_WRITER json_writer;
NX_PACKET *packet_ptr;
UINT status;

    /* Create json writer for service reported property.  */
    status = nx_azure_iot_hub_client_reported_properties_create(iothub_client_ptr, &packet_ptr, wait_option);
    if (status)
    {
        return(status);
    }
    
    /* Init json writer.  */
    status = nx_azure_iot_json_writer_init(&json_writer, packet_ptr, wait_option);
    if (status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Append begin object.  */
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill the response of writable service properties.  */
    if (nx_azure_iot_hub_client_reported_properties_component_begin(iothub_client_ptr, &json_writer,
                                                                    (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME,
                                                                    sizeof(NX_AZURE_IOT_ADU_AGENT_COMPONENT_NAME) - 1) ||
        nx_azure_iot_hub_client_reported_properties_status_begin(iothub_client_ptr, &json_writer,
                                                                 (UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SERVICE,
                                                                 sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_SERVICE) - 1,
                                                                 status_code, version,
                                                                 (const UCHAR *)description, strlen(description)))
    {
        nx_packet_release(packet_ptr);
        return (NX_NOT_SUCCESSFUL);
    }

    /* Append begin object to start to fill user value.  */
    if (nx_azure_iot_json_writer_append_begin_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return (NX_NOT_SUCCESSFUL);
    }

    /* Fill the workflow.  */
    if (nx_azure_iot_json_writer_append_property_name(&json_writer,
                                                        (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW,
                                                        sizeof (NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_WORKFLOW) - 1) ||
        nx_azure_iot_json_writer_append_begin_object(&json_writer) ||
        nx_azure_iot_json_writer_append_property_with_int32_value(&json_writer,
                                                                  (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION,
                                                                  sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ACTION) - 1,
                                                                  (INT)adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.action) ||
        nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                   (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID,
                                                                   sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_ID) - 1,
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.id,
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.id_length))
    {
        nx_packet_release(packet_ptr);
        return (NX_NOT_SUCCESSFUL);
    }

    /* Append retry timestamp in workflow if existed.  */
    if (adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.retry_timestamp_length)
    {
        if (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                       (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP,
                                                                       sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_RETRY_TIMESTAMP) - 1,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.retry_timestamp,
                                                                       adu_agent_ptr -> nx_azure_iot_adu_agent_workflow.retry_timestamp_length))
        {
            nx_packet_release(packet_ptr);
            return (NX_NOT_SUCCESSFUL);
        }
    }

    /* End workflow object.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Fill updateManifest.  */
    if (nx_azure_iot_json_writer_append_property_with_string_value(&json_writer,
                                                                   (const UCHAR *)NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST,
                                                                   sizeof(NX_AZURE_IOT_ADU_AGENT_PROPERTY_NAME_UPDATE_MANIFEST) - 1,
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest,
                                                                   adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest_size))
    {
        nx_packet_release(packet_ptr);
        return (NX_NOT_SUCCESSFUL);
    }

    /* Append end object.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return (NX_NOT_SUCCESSFUL);
    }

    /* End status and component.  */
    if (nx_azure_iot_hub_client_reported_properties_status_end(iothub_client_ptr, &json_writer) ||
        nx_azure_iot_hub_client_reported_properties_component_end(iothub_client_ptr, &json_writer))
    {
        nx_packet_release(packet_ptr);
        return (NX_NOT_SUCCESSFUL);
    }

    /* End json object.  */
    if (nx_azure_iot_json_writer_append_end_object(&json_writer))
    {
        nx_packet_release(packet_ptr);
        return(NX_NOT_SUCCESSFUL);
    }

    /* Send service reported property.  */
    status = nx_azure_iot_hub_client_reported_properties_send(iothub_client_ptr,
                                                              packet_ptr, NX_NULL,
                                                              NX_NULL, NX_NULL,
                                                              wait_option);
    if(status)
    {
        nx_packet_release(packet_ptr);
        return(status);
    }

    return(NX_AZURE_IOT_SUCCESS);
}

extern const NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY _nx_azure_iot_adu_agent_rsa_root_key_list[];
extern const UINT _nx_azure_iot_adu_agent_rsa_root_key_list_size;
static const NX_AZURE_IOT_ADU_AGENT_RSA_ROOT_KEY *nx_azure_iot_adu_agent_rsa_root_key_find(const UCHAR* kid, UINT kid_size)
{

    /* Loop to find the root key.  */
    for (UINT i = 0; i < _nx_azure_iot_adu_agent_rsa_root_key_list_size; i++)
    {

        /* Check the kid.  */
        if ((kid_size == _nx_azure_iot_adu_agent_rsa_root_key_list[i].kid_size) &&
            (memcmp(kid, _nx_azure_iot_adu_agent_rsa_root_key_list[i].kid, kid_size) == 0))
        {

            /* Find the root key.  */
            return(&_nx_azure_iot_adu_agent_rsa_root_key_list[i]);
        }
    }

    return(NX_NULL);
}

/* SHA256. */
static UINT nx_azure_iot_adu_agent_sha256_calculate(NX_CRYPTO_METHOD *sha256_method,
                                                    UCHAR *metadata_ptr, UINT metadata_size,
                                                    UCHAR *input_ptr, ULONG input_size,
                                                    UCHAR *output_ptr, ULONG output_size)
{
UINT status;


    /* Initialize crypto method.  */
    if (sha256_method -> nx_crypto_init)
    {
        status = sha256_method -> nx_crypto_init((NX_CRYPTO_METHOD*)sha256_method,
                                                 NX_NULL,
                                                 0,
                                                 NX_NULL,
                                                 metadata_ptr,
                                                 metadata_size);

        /* Check status.  */
        if (status)
        {
            return(status);
        }
    } 

    /* Initialize hash.  */
    status = sha256_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                  NX_NULL,
                                                  (NX_CRYPTO_METHOD*)sha256_method,
                                                  NX_NULL,
                                                  0,
                                                  NX_NULL,
                                                  0,
                                                  NX_NULL,
                                                  NX_NULL,
                                                  0,
                                                  metadata_ptr,
                                                  metadata_size,
                                                  NX_NULL,
                                                  NX_NULL);

    /* Update hash value for data.  */
    if (status == NX_SUCCESS)
    {
        status = sha256_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)sha256_method,
                                                      NX_NULL,
                                                      0,
                                                      input_ptr,
                                                      input_size,
                                                      NX_NULL,
                                                      NX_NULL,
                                                      0,
                                                      metadata_ptr,
                                                      metadata_size,
                                                      NX_NULL,
                                                      NX_NULL);
    }

    /* Calculate the hash value.  */
    if (status == NX_SUCCESS)
    {
        status = sha256_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)sha256_method,
                                                      NX_NULL,
                                                      0,
                                                      NX_NULL,
                                                      0,
                                                      NX_NULL,
                                                      output_ptr,
                                                      output_size,
                                                      metadata_ptr,
                                                      metadata_size,
                                                      NX_NULL,
                                                      NX_NULL);
    }

    /* Cleanup.  */
    if (sha256_method -> nx_crypto_cleanup)
    {
        sha256_method -> nx_crypto_cleanup(metadata_ptr);
    }

    return(status);
}


/* RS256.  */
static UINT nx_azure_iot_adu_agent_rs256_verify(NX_AZURE_IOT_ADU_AGENT_CRYPTO *adu_agent_crypto,
                                                UCHAR *input_ptr, ULONG input_size,
                                                UCHAR *signature_ptr, ULONG signature_size,
                                                UCHAR *n, ULONG n_size,
                                                UCHAR *e, ULONG e_size,
                                                UCHAR *buffer_ptr, UINT buffer_size)
{

UINT   status;
UCHAR *oid;
UINT   oid_length;
UCHAR *decrypted_hash;
UINT   decrypted_hash_length;
UCHAR *rsa_buffer = buffer_ptr;
UCHAR *sha_buffer = buffer_ptr + NX_AZURE_IOT_ADU_AGENT_RSA3072_SIZE;

    /* Check buffer size.  */
    if (buffer_size < (NX_AZURE_IOT_ADU_AGENT_RSA3072_SIZE + NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE))
    {
        return(NX_FALSE);
    }

    /* Decrypt the signature by RSA.  */

    /* Initialize.  */
    status = adu_agent_crypto -> method_rsa -> nx_crypto_init((NX_CRYPTO_METHOD*)adu_agent_crypto -> method_rsa,
                                                              n,
                                                              n_size << 3,
                                                              NX_NULL,
                                                              adu_agent_crypto -> method_rsa_metadata,
                                                              adu_agent_crypto -> method_rsa_metadata_size);

    /* Check status.  */
    if (status)
    {
        return(NX_FALSE);
    }

    /* Decrypt the signature.  */
    status = adu_agent_crypto -> method_rsa -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                                   NX_NULL,
                                                                   (NX_CRYPTO_METHOD*)adu_agent_crypto -> method_rsa,
                                                                   e,
                                                                   e_size << 3, 
                                                                   signature_ptr,
                                                                   signature_size,
                                                                   NX_NULL,
                                                                   rsa_buffer,
                                                                   NX_AZURE_IOT_ADU_AGENT_RSA3072_SIZE,
                                                                   adu_agent_crypto -> method_rsa_metadata,
                                                                   adu_agent_crypto -> method_rsa_metadata_size,
                                                                   NX_NULL,
                                                                   NX_NULL);

    /* Cleanup.  */
    if (adu_agent_crypto -> method_rsa -> nx_crypto_cleanup)
    {
        adu_agent_crypto -> method_rsa -> nx_crypto_cleanup(adu_agent_crypto -> method_rsa_metadata);
    }

    /* Check status.  */
    if (status)
    {
        return(NX_FALSE);
    }

    /* Decode the decrypted signature, which should be in PKCS#7 format. */
    status = _nx_secure_x509_pkcs7_decode(rsa_buffer, signature_size,
                                          (const UCHAR **)&oid, &oid_length,
                                          (const UCHAR **)&decrypted_hash, &decrypted_hash_length);

    /* Check status.  */
    if (status)
    {
        return(NX_FALSE);
    }

    /* Calculate input by SHA256.  */    
    status = nx_azure_iot_adu_agent_sha256_calculate(adu_agent_crypto -> method_sha256,
                                                     adu_agent_crypto -> method_sha256_metadata,
                                                     NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE,
                                                     input_ptr, input_size,
                                                     sha_buffer, NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE);

    /* Check status.  */
    if (status)
    {
        return(NX_FALSE);
    }

    /* Verify.  */
    if ((decrypted_hash_length != NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE) || 
        (memcmp(decrypted_hash, sha_buffer, NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE)))
    {
        return(NX_FALSE);
    }

    return(NX_TRUE);
}

static UINT nx_azure_iot_adu_agent_file_url_parse(UCHAR *file_url, ULONG file_url_length, 
                                                  UCHAR *buffer_ptr, UINT buffer_size,
                                                  NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr)
{
UINT    i;
UINT    dot_count = 0;
UINT    temp = 0;
ULONG   ip_address = 0;
UCHAR   address_found = NX_FALSE;
UCHAR   port_found = NX_FALSE;


    /* Initialize.  */
    downloader_ptr -> host = NX_NULL;
    downloader_ptr -> resource = NX_NULL;

    /* Format: http://host:port/resource.  */
    if (memcmp(file_url, NX_AZURE_IOT_ADU_AGENT_HTTP_PROTOCOL, sizeof(NX_AZURE_IOT_ADU_AGENT_HTTP_PROTOCOL) - 1))
    {
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Set the host ptr.  */
    file_url += (sizeof(NX_AZURE_IOT_ADU_AGENT_HTTP_PROTOCOL) - 1);
    file_url_length -= (sizeof(NX_AZURE_IOT_ADU_AGENT_HTTP_PROTOCOL) - 1);

    /* Try to detect whether the host is numerical IP address. */
    for (i = 0; i < file_url_length; i++)
    {
        if (file_url[i] >= '0' && file_url[i] <= '9')
        {
            temp = (UINT)(temp * 10 + (UINT)(file_url[i] - '0'));
            if ((temp > 0xFF && port_found == NX_FALSE) ||
                (temp > 0xFFFF && port_found == NX_TRUE))
            {
                break;
            }
        }
        else if (file_url[i] == '.')
        {
            if (dot_count++ == 3)
            {
                break;
            }
            ip_address = (ip_address << 8) + temp;
            temp = 0;
        }
        else if (file_url[i] == ':')
        {
            if ((dot_count != 3) || (port_found == NX_TRUE))
            {
                break;
            }
            ip_address = (ip_address << 8) + temp;

            /* Set the address.  */
            downloader_ptr -> address.nxd_ip_version = NX_IP_VERSION_V4;
            downloader_ptr -> address.nxd_ip_address.v4 = ip_address;
            address_found = NX_TRUE;
                
            /* Try to reslove the port.  */
            temp = 0;
            port_found = NX_TRUE;
        }
        else if (file_url[i] == '/')
        {   
            if (dot_count == 3)
            {
                if (port_found)
                {
                    downloader_ptr -> port = temp;
                }
                else
                {
                    ip_address = (ip_address << 8) + temp;
                            
                    /* Set the address.  */
                    downloader_ptr -> address.nxd_ip_version = NX_IP_VERSION_V4;
                    downloader_ptr -> address.nxd_ip_address.v4 = ip_address;
                    address_found = NX_TRUE;                
                }                
            }    
            break;
        }
        else
        {
            break;
        }
    }
 
    /* Check if there is enough buffer.  */
    if (file_url_length >= buffer_size)
    {
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Split host and resource url . */
    for (; i < file_url_length; i++)
    {
        if (file_url[i] == '/')
        {

            /* Store the host ans resource.  */
            downloader_ptr -> host = buffer_ptr;
            memcpy(downloader_ptr -> host, file_url, i); /* Use case of memcpy is verified. */
            *(buffer_ptr + i) = NX_NULL;

            /* Set the resource url.  */
            downloader_ptr -> resource = (buffer_ptr + i + 1);
            memcpy(downloader_ptr -> resource, &file_url[i + 1], (file_url_length - i - 1)); /* Use case of memcpy is verified. */
            *(buffer_ptr + file_url_length) = NX_NULL;

            /* Update buffer size.  */
            buffer_size -= (file_url_length + 1);
            break;
        }
    }
    
    /* Check the host and resource.   */
    if ((downloader_ptr -> host == NX_NULL) || (downloader_ptr -> resource == NX_NULL))
    {
        return(NX_AZURE_IOT_FAILURE);
    }

    /* Update the state.  */
    if (address_found == NX_FALSE)
    {
        downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_URL_PARSED;
    }
    else
    {
        downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_DONE;
    }

    /* Check if found the port.  */
    if (port_found == NX_FALSE)
    {

        /* Set tht http port as default.  */
        downloader_ptr -> port = NX_WEB_HTTP_SERVER_PORT;
    }
    
    return(NX_AZURE_IOT_SUCCESS);
}

static void nx_azure_iot_adu_agent_dns_query(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
UINT status;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);


    /* Check the state.  */
    if ((downloader_ptr -> state != NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_URL_PARSED) &&
        (downloader_ptr -> state != NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_QUERY))
    {
        return;
    }

    /* Check if reach the max retry count.  */
    if (downloader_ptr -> dns_query_count <= NX_AZURE_IOT_ADU_AGENT_DNS_RETRANSMIT_COUNT)
    {

        /* Set the timeout.  */
        downloader_ptr -> timeout = (ULONG)(NX_AZURE_IOT_ADU_AGENT_DNS_INITIAL_TIMEOUT << downloader_ptr -> dns_query_count);
    
        /* Update the query count.  */
        downloader_ptr -> dns_query_count++;

        /* Update state.  */
        downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_QUERY;

        /* Resolve the host name by DNS.  */
        status = nxd_dns_host_by_name_get(downloader_ptr -> dns_ptr, 
                                          downloader_ptr -> host,
                                          &(downloader_ptr -> address),
                                          NX_NO_WAIT, NX_IP_VERSION_V4);

        /* Check status.  */
        if (status == NX_SUCCESS)
        {

            /* Got the address, update the state.  */
            downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_DONE;

            /* Start HTTP connect.  */
            nx_azure_iot_adu_agent_http_connect(adu_agent_ptr);
            return;
        }
        else if (status == NX_IN_PROGRESS)
        {

            /* Query in progress.  */
            return;
        }
    }

    LogError(LogLiteralArgs("Firmware download fail: DNS QUERY FAIL"));

    /* Send dns query failed or already reach the max retransmission count.  */
    nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
}

static void nx_azure_iot_adu_agent_dns_response_notify(NX_UDP_SOCKET *socket_ptr)
{
NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr;


    /* Set adu agent pointer.  */
    adu_agent_ptr = (NX_AZURE_IOT_ADU_AGENT *)socket_ptr -> nx_udp_socket_reserved_ptr;

    /* Set the DNS response receive event.  */
    nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                              NX_AZURE_IOT_ADU_AGENT_DNS_RESPONSE_RECEIVE_EVENT);
}

static void nx_azure_iot_adu_agent_dns_response_get(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
UINT status;
UINT record_count;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);


    /* Try to get the response.  */
    status = _nx_dns_response_get(downloader_ptr -> dns_ptr, downloader_ptr -> host,
                                  (UCHAR *)&downloader_ptr -> address.nxd_ip_address.v4, sizeof(ULONG),
                                  &record_count, NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {

        /* Retry DNS query.  */
        nx_azure_iot_adu_agent_dns_query(adu_agent_ptr);
    }
    else
    {

        /* Set the address version.  */
        downloader_ptr -> address.nxd_ip_version = NX_IP_VERSION_V4;

        /* Update the state.  */
        adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_ADDRESS_DONE;

        /* Start HTTP connect.  */
        nx_azure_iot_adu_agent_http_connect(adu_agent_ptr);
    }
}

static void nx_azure_iot_adu_agent_http_connect(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{

UINT                status;
NX_IP              *ip_ptr;
NX_CRYPTO_METHOD   *sha256_method;
UCHAR              *sha256_method_metadata;
ULONG               sha256_method_metadata_size;
VOID               *handler;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);

    /* Initialize.  */
    ip_ptr = adu_agent_ptr -> nx_azure_iot_hub_client_ptr -> nx_azure_iot_ptr -> nx_azure_iot_ip_ptr;
    downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);

    /* Initialize hash for downloading firmware.  */
    sha256_method = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.method_sha256;
    sha256_method_metadata = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.method_sha256_metadata;
    sha256_method_metadata_size = NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE;
    handler = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.handler;

    /* Initialize the crypto. */
    if (sha256_method -> nx_crypto_init)
    {
        status = sha256_method -> nx_crypto_init((NX_CRYPTO_METHOD*)sha256_method,
                                                 NX_NULL,
                                                 0,
                                                 &handler,
                                                 sha256_method_metadata,
                                                 sha256_method_metadata_size);

        /* Check status.  */
        if (status)
        {
            LogError(LogLiteralArgs("Firmware download fail: SHA256 INIT ERROR"));
            nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
            return;
        }
    }

    /* Initialize the sha256 for firmware hash. */
    status = sha256_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                  handler,
                                                  (NX_CRYPTO_METHOD*)sha256_method,
                                                  NX_NULL,
                                                  0,
                                                  NX_NULL,
                                                  0,
                                                  NX_NULL,
                                                  NX_NULL,
                                                  0,
                                                  sha256_method_metadata,
                                                  sha256_method_metadata_size,
                                                  NX_NULL,
                                                  NX_NULL); 

    /* Check status.  */
    if (status)
    {
        LogError(LogLiteralArgs("Firmware download fail: SHA256 INIT ERROR"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }

    /* Create an HTTP client instance.  */
    status = nx_web_http_client_create(&(downloader_ptr -> http_client),
                                       "HTTP Client",
                                       ip_ptr, ip_ptr -> nx_ip_default_packet_pool,
                                       NX_AZURE_IOT_ADU_AGENT_HTTP_WINDOW_SIZE);

    /* Check status.  */
    if (status)
    {
        LogError(LogLiteralArgs("Firmware download fail: CLIENT CREATE FAIL"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }

    /* Update the state and timeout.  */
    downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONNECT;
    downloader_ptr -> timeout = NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_TIMEOUT;

    /* Set the notify.  */
    downloader_ptr -> http_client.nx_web_http_client_socket.nx_tcp_socket_reserved_ptr = adu_agent_ptr;
    nx_tcp_socket_establish_notify(&(downloader_ptr -> http_client.nx_web_http_client_socket),
                                   nx_azure_iot_adu_agent_http_establish_notify);
    nx_tcp_socket_receive_notify(&(downloader_ptr -> http_client.nx_web_http_client_socket), 
                                 nx_azure_iot_adu_agent_http_receive_notify);

    /* Connect to Server.  */
    status = nx_web_http_client_connect(&(downloader_ptr -> http_client),
                                        &(downloader_ptr -> address),
                                        downloader_ptr -> port,
                                        NX_NO_WAIT);
    
    /* Check status.  */
    if (status == NX_SUCCESS)
    {

        /* Connection established. Start to get file content.  */
        nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                                  NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_DONE_EVENT);
        return;
    }
    else if (status == NX_IN_PROGRESS)
    {

        /* Query in progress.  */
        return;
    }

    LogError(LogLiteralArgs("Firmware download fail: CLIENT CONNECT FAIL"));

    /* Failed.  */
    nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
    return;
}

static void nx_azure_iot_adu_agent_http_request_send(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
UINT status;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);


    /* Update the state and timeout.  */
    downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONTENT_GET;
    downloader_ptr -> timeout = NX_AZURE_IOT_ADU_AGENT_HTTP_DOWNLOAD_TIMEOUT;

    /* Use the service to send a GET request to the server . */
    status = nx_web_http_client_request_initialize(&(downloader_ptr -> http_client),
                                                   NX_WEB_HTTP_METHOD_GET, 
                                                   (CHAR *)downloader_ptr -> resource,
                                                   (CHAR *)downloader_ptr -> host,
                                                   0, NX_FALSE, NX_NULL, NX_NULL,
                                                   NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {
        LogError(LogLiteralArgs("Firmware download fail: CLIENT REQUEST INIT FAIL"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }

    /* Send the HTTP request we just built. */
    status = nx_web_http_client_request_send(&(downloader_ptr -> http_client), NX_NO_WAIT);

    /* Check status.  */
    if (status)
    {
        LogError(LogLiteralArgs("Firmware download fail: CLIENT REQUEST SEND FAIL"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }
}

static void nx_azure_iot_adu_agent_http_response_receive(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr)
{
    
UINT        status;
UINT        get_status;
NX_PACKET  *received_packet;
NX_PACKET  *data_packet;
UINT        data_size;
NX_CRYPTO_METHOD *sha256_method = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.method_sha256;
UCHAR      *sha256_method_metadata = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.method_sha256_metadata;;
ULONG       sha256_method_metadata_size = NX_AZURE_IOT_ADU_AGENT_SHA256_METADATA_SIZE;
VOID       *handler = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.handler;
UCHAR      *generated_hash;
UCHAR      *decoded_hash;
UINT        bytes_copied;
NX_AZURE_IOT_ADU_AGENT_DRIVER driver_request;
NX_AZURE_IOT_ADU_AGENT_DOWNLOADER *downloader_ptr = &(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader);

    /* Check the state.  */
    if (downloader_ptr -> state != NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONTENT_GET)
    {
        return;
    }

    /* Receive response data from the server. Loop until all data is received. */
    get_status = NX_SUCCESS;
    while ((get_status != NX_WEB_HTTP_GET_DONE) && (downloader_ptr -> received_firmware_size < downloader_ptr -> file -> file_size_in_bytes))
    {
        get_status = nx_web_http_client_response_body_get(&(downloader_ptr -> http_client), &received_packet, NX_NO_WAIT);

        /* Check for error.  */
        if ((get_status == NX_SUCCESS) || (get_status == NX_WEB_HTTP_GET_DONE) || (get_status == NX_WEB_HTTP_STATUS_CODE_PARTIAL_CONTENT))
        {

            /* Loop to write the data from packet into flash.  */
            data_packet = received_packet;
#ifndef NX_DISABLE_PACKET_CHAIN
            while(data_packet)
            {
#endif /* NX_DISABLE_PACKET_CHAIN  */

                /* Calculate the data size in current packet.  */
                data_size = (UINT)(data_packet -> nx_packet_append_ptr - data_packet -> nx_packet_prepend_ptr);

                /* Update the hash value for data.  */
                status = sha256_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                              NX_NULL,
                                                              (NX_CRYPTO_METHOD*)sha256_method,
                                                              NX_NULL,
                                                              0,
                                                              data_packet -> nx_packet_prepend_ptr,
                                                              (ULONG)data_size,
                                                              NX_NULL,
                                                              NX_NULL,
                                                              0,
                                                              sha256_method_metadata,
                                                              sha256_method_metadata_size,
                                                              NX_NULL,
                                                              NX_NULL);
    
                /* Check status.  */
                if (status)
                {

                    /* Release the packet.  */
                    nx_packet_release(received_packet);
                    LogError(LogLiteralArgs("Firmware download fail: HASH UPDATE ERROR"));
                    nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
                    return;
                }

                if (downloader_ptr -> type == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE)
                {

                    /* Send the firmware write request to the driver.   */
                    driver_request.nx_azure_iot_adu_agent_driver_command = NX_AZURE_IOT_ADU_AGENT_DRIVER_WRITE;
                    driver_request.nx_azure_iot_adu_agent_driver_firmware_data_offset = downloader_ptr -> received_firmware_size;
                    driver_request.nx_azure_iot_adu_agent_driver_firmware_data_ptr = data_packet -> nx_packet_prepend_ptr;
                    driver_request.nx_azure_iot_adu_agent_driver_firmware_data_size = data_size;
                    driver_request.nx_azure_iot_adu_agent_driver_status = NX_AZURE_IOT_SUCCESS;
                    (downloader_ptr -> driver_entry)(&driver_request);
                
                    /* Check status.  */
                    if (driver_request.nx_azure_iot_adu_agent_driver_status)
                    {

                        /* Release the packet.  */
                        nx_packet_release(received_packet);
                        LogError(LogLiteralArgs("Firmware download fail: DRIVER WRITE ERROR"));
                        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
                        return;
                    }
                }
                else
                {

                    if ((downloader_ptr -> received_firmware_size + data_size) > downloader_ptr -> manifest_buffer_size)
                    {

                        /* Release the packet.  */
                        nx_packet_release(received_packet);
                        LogError(LogLiteralArgs("Firmware download fail: BUFFER ERROR"));
                        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
                        return;
                    }

                    memcpy(downloader_ptr -> manifest_buffer_ptr + downloader_ptr -> received_firmware_size, /* Use case of memcpy is verified. */
                           data_packet -> nx_packet_prepend_ptr, data_size);
                }

                /* Update received firmware size.  */
                downloader_ptr -> received_firmware_size += data_size;
                
#ifndef NX_DISABLE_PACKET_CHAIN
                data_packet = data_packet -> nx_packet_next;
            }
#endif /* NX_DISABLE_PACKET_CHAIN  */

            /* Release the packet.  */
            nx_packet_release(received_packet);

            if (downloader_ptr -> type == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE)
            {
                LogInfo(LogLiteralArgs("Getting download data... %d"), downloader_ptr -> received_firmware_size);
            }
        }
        else
        {
            if (get_status != NX_NO_PACKET)
            {
                LogError(LogLiteralArgs("Firmware download fail: RECEIVE ERROR"));
                nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
            }
            return;
        }
    }

    /* Output info.  */
    if (downloader_ptr -> type == NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_TYPE_FIRMWARE)
    {
        LogInfo(LogLiteralArgs("Firmware downloaded"));
    }
    downloader_ptr -> state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_DONE;

    /* Firmware downloaded. Verify the hash.  */

    /* Set hash buffer.  */
    if ((NX_AZURE_IOT_ADU_AGENT_UPDATE_MANIFEST_SIZE - (downloader_ptr -> host_length + 1 + downloader_ptr -> resource_length + 1)) < 
        ((NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE + 1) << 1))
    {
        LogError(LogLiteralArgs("Firmware verify fail: INSUFFICIENT BUFFER FOR SHA256"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }
    generated_hash = adu_agent_ptr -> nx_azure_iot_adu_agent_update_manifest + 
                    downloader_ptr -> host_length + 1 + downloader_ptr -> resource_length + 1;
    decoded_hash = generated_hash + NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE + 1;

    /* Calculate the hash value.  */
    status = sha256_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                  handler,
                                                  (NX_CRYPTO_METHOD*)sha256_method,
                                                  NX_NULL,
                                                  0,
                                                  NX_NULL,
                                                  0,
                                                  NX_NULL,
                                                  generated_hash,
                                                  NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE,
                                                  sha256_method_metadata,
                                                  sha256_method_metadata_size,
                                                  NX_NULL,
                                                  NX_NULL);

    /* Check status.  */
    if (status)
    {
        LogError(LogLiteralArgs("Firmware verify fail: HASH ERROR"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }

    /* Decode the file hash (base64).  */
    if (_nx_utility_base64_decode(downloader_ptr -> file -> file_sha256,
                                  downloader_ptr -> file -> file_sha256_length,
                                  decoded_hash, NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE + 1, &bytes_copied))
    {
        LogError(LogLiteralArgs("Firmware verify fail: HASH ERROR"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }
    
    /* Verify the hash value.  */
    if (memcmp(generated_hash, decoded_hash, NX_AZURE_IOT_ADU_AGENT_SHA256_HASH_SIZE))
    {
        LogError(LogLiteralArgs("Firmware verify fail: HASH ERROR"));
        nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_FALSE);
        return;
    }

    /* Update download state.  */
    nx_azure_iot_adu_agent_download_state_update(adu_agent_ptr, NX_TRUE);
}

static void nx_azure_iot_adu_agent_http_establish_notify(NX_TCP_SOCKET *socket_ptr)
{
NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr;


    /* Set adu agent pointer.  */
    adu_agent_ptr = (NX_AZURE_IOT_ADU_AGENT *)socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the DNS response receive event.  */
    nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                              NX_AZURE_IOT_ADU_AGENT_HTTP_CONNECT_DONE_EVENT);
}

static void nx_azure_iot_adu_agent_http_receive_notify(NX_TCP_SOCKET *socket_ptr)
{
NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr;


    /* Set adu agent pointer.  */
    adu_agent_ptr = (NX_AZURE_IOT_ADU_AGENT *)socket_ptr -> nx_tcp_socket_reserved_ptr;

    /* Set the DNS response receive event.  */
    nx_cloud_module_event_set(&(adu_agent_ptr -> nx_azure_iot_adu_agent_cloud_module),
                              NX_AZURE_IOT_ADU_AGENT_HTTP_RECEIVE_EVENT);
}

static void nx_azure_iot_adu_agent_download_state_update(NX_AZURE_IOT_ADU_AGENT *adu_agent_ptr, UINT success)
{
NX_CRYPTO_METHOD   *sha256_method = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.method_sha256;
UCHAR              *sha256_method_metadata = adu_agent_ptr -> nx_azure_iot_adu_agent_crypto.method_sha256_metadata;

    /* Cleanup download socket.  */
    if (adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.state >= NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_HTTP_CONNECT)
    {

        /* Delete http client.  */
        nx_web_http_client_delete(&(adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.http_client));
    }

    /* Reset the state.  */
    adu_agent_ptr -> nx_azure_iot_adu_agent_downloader.state = NX_AZURE_IOT_ADU_AGENT_DOWNLOADER_IDLE;

    /* Cleanup sha256.  */
    if (sha256_method -> nx_crypto_cleanup)
    {
        sha256_method -> nx_crypto_cleanup(sha256_method_metadata);
    }

    /* Update the state according to the download status.  */
    if (success == NX_TRUE)
    {

        /* Download complete, update state to next state.  */
        nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, adu_agent_ptr -> nx_azure_iot_adu_agent_current_step -> state + 1);
    }
    else
    {
        nx_azure_iot_adu_agent_step_state_update(adu_agent_ptr, NX_AZURE_IOT_ADU_AGENT_STEP_STATE_FAILED);
    }
}

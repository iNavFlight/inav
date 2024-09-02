/*
 * This is a demonstration of the NetX LWM2M Client.
 */

#include "nx_api.h"
#include "nx_lwm2m_client.h"

#ifndef SAMPLE_DHCP_DISABLE
#include "nxd_dhcp_client.h"
#endif /* SAMPLE_DHCP_DISABLE */

#ifndef SAMPLE_DNS_DISABLE
#include "nxd_dns.h"
#endif /* SAMPLE_DNS_DISABLE */

#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls_api.h"
#include "nx_crypto_aes.h"
#include "nx_crypto_sha2.h"
#include "nx_crypto_tls_prf_sha256.h"
#endif /* NX_SECURE_ENABLE_DTLS */


/* Enable LWM2M Bootstrap to start Client Initiated Bootstrap.   */
//#define BOOTSTRAP

/* LwM2M Client endpoint.  */
#ifndef LWM2M_CLIENT_ENDPOINT
#define LWM2M_CLIENT_ENDPOINT                   "nxlwm2mclient"
#endif /* LWM2M_CLIENT_ENDPOINT */

/* IP address of LwM2M Server or LwM2M Bootstrap Server.  */
#ifndef LWM2M_SERVER_ADDRESS
#define LWM2M_SERVER_ADDRESS                    IP_ADDRESS(23, 97, 187, 154)
#endif /* LWM2M_SERVER_ADDRESS*/

/* Port of LwM2M Server or LwM2M Bootstrap Server.  */
#ifndef LWM2M_SERVER_PORT
#define LWM2M_SERVER_PORT                       5683
#endif /* LWM2M_SERVER_PORT*/

#ifndef LWM2M_BOOTSTRAP_SERVER_PORT
#define LWM2M_BOOTSTRAP_SERVER_PORT             5783
#endif /* LWM2M_BOOTSTRAP_SERVER_PORT*/

#ifndef LWM2M_SERVER_DTLS_PORT
#define LWM2M_SERVER_DTLS_PORT                  5684
#endif /* LWM2M_SERVER_DTLS_PORT*/

#ifndef LWM2M_BOOTSTRAP_SERVER_DTLS_PORT
#define LWM2M_BOOTSTRAP_SERVER_DTLS_PORT        5784
#endif /* LWM2M_BOOTSTRAP_SERVER_DTLS_PORT */

/* LwM2M server ID */
#ifndef LWM2M_SERVER_ID
#define LWM2M_SERVER_ID                         1234
#endif /* LWM2M_SERVER_ID */

/* Use DTLS to establish a secure DTLS connection with LwM2M Server or LwM2M Bootstrap Server.
   Note: NX_SECURE_ENABLE_DTLS must be defined, and configure the identity and key.  */
//#define USE_DTLS

#ifdef NX_SECURE_ENABLE_DTLS
#ifndef LWM2M_DTLS_IDENTITY
#define LWM2M_DTLS_IDENTITY                     "Identity"
#endif /* LWM2M_DTLS_IDENTITY */

/* LWM2M_DTLS_KEY */
#ifndef LWM2M_DTLS_KEY
#define LWM2M_DTLS_KEY                          "Key"
#endif /* LWM2M_DTLS_KEY */
#endif /* NX_SECURE_ENABLE_DTLS  */


/* Define the ThreadX and NetX object control blocks...  */
static NX_PACKET_POOL   pool_0;
static NX_IP            ip_0;
#ifndef SAMPLE_DHCP_DISABLE
static NX_DHCP          dhcp_client;
#endif /* SAMPLE_DHCP_DISABLE */
#ifndef SAMPLE_DNS_DISABLE
static NX_DNS           dns_client;
#endif /* SAMPLE_DNS_DISABLE */

/* Define the IP thread's stack area.  */
static ULONG            ip_thread_stack[2048 / sizeof(ULONG)];

/* Define packet pool for the demonstration.  */
static ULONG            packet_pool_area[((1536 + sizeof(NX_PACKET)) * 32) / sizeof(ULONG)];

/* Define the ARP cache area.  */
static ULONG            arp_space_area[512 / sizeof(ULONG)];

/* Define an error counter.  */
static ULONG            error_counter;

/* Define application thread.  */
static TX_THREAD        main_thread;
static ULONG            main_stack[4096 / sizeof(ULONG)];

#ifdef BOOTSTRAP
static TX_SEMAPHORE     semaphore_bootstarp_finish;
#endif /* BOOTSTRAP */

#ifdef NX_SECURE_ENABLE_DTLS

/* Declare the NULL encrypt */
extern NX_CRYPTO_METHOD crypto_method_null;

/* Declare the AES-CCM-8 encrytion method. */
extern NX_CRYPTO_METHOD crypto_method_aes_ccm_8;

/* Declare the SHA256 hash method */
extern NX_CRYPTO_METHOD crypto_method_sha256;

/* Declare the TLSv1.2 default PRF hash method */
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha256;

/* Declare the MD5 hash method */
extern NX_CRYPTO_METHOD crypto_method_md5;

/* Declare the SHA1 hash method */
extern NX_CRYPTO_METHOD crypto_method_sha1;

/* Declare the TLSv1.0/1.1 PRF hash method */
extern NX_CRYPTO_METHOD crypto_method_tls_prf_1;

/* Declare a placeholder for PSK. */
extern NX_CRYPTO_METHOD crypto_method_auth_psk;

/* Lookup table used to map ciphersuites to cryptographic routines.  */
NX_SECURE_TLS_CIPHERSUITE_INFO crypto_ciphersuite_lookup_table[] =
{
    /* Ciphersuite,                       public cipher,       public_auth,             session cipher & cipher mode,  iv size, key size,  hash method,          hash size,   TLS PRF */
    {TLS_PSK_WITH_AES_128_CCM_8,    &crypto_method_null, &crypto_method_auth_psk, &crypto_method_aes_ccm_8,     16,       16,        &crypto_method_null,      0,         &crypto_method_tls_prf_sha256},
};

/* Define the object we can pass into TLS.  */
const NX_SECURE_TLS_CRYPTO crypto_tls_ciphers =
{
    /* Ciphersuite lookup table and size. */
    crypto_ciphersuite_lookup_table,
    sizeof(crypto_ciphersuite_lookup_table) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO),

#ifndef NX_SECURE_DISABLE_X509
    /* X.509 certificate cipher table and size. */
    NX_NULL,
    0,
#endif

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    &crypto_method_md5,
    &crypto_method_sha1,
    &crypto_method_tls_prf_1,
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    &crypto_method_sha256,
    &crypto_method_tls_prf_sha256
#endif
};

static NX_SECURE_DTLS_SESSION   dtls_session;
static UCHAR                    dtls_metadata_buffer[10 * 1024];
static UCHAR                    dtls_packet_buffer[5 * 1024];
#endif /* NX_SECURE_ENABLE_DTLS */

#ifndef SAMPLE_DHCP_DISABLE
#define IPV4_ADDRESS            IP_ADDRESS(0, 0, 0, 0)
#define IPV4_NETWORK_MASK       IP_ADDRESS(0, 0, 0, 0)
#else
#define IPV4_ADDRESS            IP_ADDRESS(10, 1, 0, 212)
#define IPV4_NETWORK_MASK       IP_ADDRESS(255, 255, 0, 0)
#define IPV4_GATEWAY_ADDR       IP_ADDRESS(10, 1, 0, 1)
#define DNS_SERVER_ADDRESS      IP_ADDRESS(10, 1, 0, 1)
#endif /* SAMPLE_DHCP_DISABLE */

#ifndef SAMPLE_DNS_DISABLE
static CHAR host_buffer[256];
#endif /* SAMPLE_DNS_DISABLE */

/* Define the Temperature IPSO Resources IDs */
#define IPSO_TEMPERATURE_OBJECT_ID   3303
#define IPSO_RESOURCE_MIN_VALUE      5601
#define IPSO_RESOURCE_MAX_VALUE      5602
#define IPSO_RESOURCE_RESET_MINMAX   5605
#define IPSO_RESOURCE_VALUE          5700
#define IPSO_RESOURCE_UNITS          5701

/* Define the Actuation IPSO Resources IDs */
#define IPSO_ACTUATION_OBJECT_ID     3306
#define IPSO_RESOURCE_ONOFF          5850

/* Define the Temperature Object Instance structure */
typedef struct
{
    /* The LWM2M Object Instance */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE    object_instance;

    /* Resources Data */
    NX_LWM2M_FLOAT32                   temperature;
    NX_LWM2M_FLOAT32                   min_temperature;
    NX_LWM2M_FLOAT32                   max_temperature;

} IPSO_TEMPERATURE_INSTANCE;

/* Define the Actuation Object Instance structure */
typedef struct
{
    /* The LWM2M Object Instance */
    NX_LWM2M_CLIENT_OBJECT_INSTANCE    object_instance;

    /* Resources Data */
    NX_LWM2M_BOOL                      onoff;

} IPSO_ACTUATION_INSTANCE;

/* LWM2M Client data */
NX_LWM2M_CLIENT             client;
ULONG                       client_stack[4096 / sizeof(ULONG)];
NX_LWM2M_CLIENT_SESSION     session;
NX_LWM2M_CLIENT_FIRMWARE    firmware;

/* Custom Object Data */
NX_LWM2M_CLIENT_OBJECT      temperature_object;
IPSO_TEMPERATURE_INSTANCE   temperature_instance;
NX_LWM2M_CLIENT_OBJECT      actuation_object;
IPSO_ACTUATION_INSTANCE     actuation_instance;
IPSO_ACTUATION_INSTANCE     actuation_instance_new;
UINT                        actuation_instance_new_created = NX_FALSE;

/* Device Object */
const CHAR manufacturer[] = "Microsoft Corporation";

/* Firmware Update Object (emulation) */
int firmware_timer = 0;
int firmware_state = NX_LWM2M_CLIENT_FIRMWARE_STATE_IDLE;
int firmware_version_number = 1;
const CHAR firmware_name[] = "Test Firmware";
CHAR firmware_version[] = "Version 1";

/***** Substitute your ethernet driver entry function here *********/
#ifndef NETWORK_DRIVER
#define NETWORK_DRIVER      _nx_ram_network_driver
#endif
extern VOID NETWORK_DRIVER(NX_IP_DRIVER *driver_req_ptr);

#ifndef SAMPLE_DHCP_DISABLE
static void dhcp_wait();
#endif /* SAMPLE_DHCP_DISABLE */

#ifndef SAMPLE_DNS_DISABLE
static UINT dns_create();
#endif /* SAMPLE_DNS_DISABLE */

#ifdef NX_SECURE_ENABLE_DTLS
static UINT dtls_setup(NX_SECURE_DTLS_SESSION *dtls_session_ptr, UCHAR *key, UINT key_len, UCHAR *identity, UINT identity_len);
#endif /* NX_SECURE_ENABLE_DTLS */

static UINT uri_parse(const char *uri, NXD_ADDRESS *ip_addr, UINT *udp_port);

/*
 * Custom Object implementations
 */

/* IPSO Temperature */
/* Define the 'Read' Method */
UINT ipso_temperature_read(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT resource_count)
{
IPSO_TEMPERATURE_INSTANCE *temp = ((IPSO_TEMPERATURE_INSTANCE *) instance_ptr);
UINT i;
NX_LWM2M_ID resource_id;

    for (i = 0 ; i < resource_count; i++)
    {
        nx_lwm2m_client_resource_info_get(&resource[i], &resource_id, NX_NULL);
        switch (resource_id)
        {

        case IPSO_RESOURCE_MIN_VALUE:

            /* return the minimum measured temperature value */
            nx_lwm2m_client_resource_float32_set(&resource[i], temp -> min_temperature);
            break;

        case IPSO_RESOURCE_MAX_VALUE:

            /* return the maximum measured temperature value */
            nx_lwm2m_client_resource_float32_set(&resource[i], temp -> max_temperature);
            break;

        case IPSO_RESOURCE_VALUE:

            /* return the temperature value */
            nx_lwm2m_client_resource_float32_set(&resource[i], temp -> temperature);
            break;

        case IPSO_RESOURCE_RESET_MINMAX:

            /* Not readable */
            return(NX_LWM2M_CLIENT_METHOD_NOT_ALLOWED);

        case IPSO_RESOURCE_UNITS:

            /* return the temperature units */
            nx_lwm2m_client_resource_string_set(&resource[i], "Cel", 3);
            break;

        default:

            /* unknown resource ID */
            return(NX_LWM2M_CLIENT_NOT_FOUND);
        }
    }

    return(NX_SUCCESS);
}

/* Define the 'Discover' method */
UINT ipso_temperature_discover(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resources, UINT *resource_count)
{
    if (*resource_count < 5)
    {
        return(NX_LWM2M_CLIENT_BUFFER_TOO_SMALL);
    }

    /* return the list of supported resources IDs */
    *resource_count = 5;
    nx_lwm2m_client_resource_info_set(&resources[0], IPSO_RESOURCE_MIN_VALUE, NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ);
    nx_lwm2m_client_resource_info_set(&resources[1], IPSO_RESOURCE_MAX_VALUE, NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ);
    nx_lwm2m_client_resource_info_set(&resources[2], IPSO_RESOURCE_RESET_MINMAX, NX_LWM2M_CLIENT_RESOURCE_OPERATION_EXECUTABLE);
    nx_lwm2m_client_resource_info_set(&resources[3], IPSO_RESOURCE_VALUE, NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ);
    nx_lwm2m_client_resource_info_set(&resources[4], IPSO_RESOURCE_UNITS, NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ);

    return(NX_SUCCESS);
}

/* Define the 'Execute' method */
UINT ipso_temperature_execute(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, const CHAR *args_ptr, UINT args_length)
{
IPSO_TEMPERATURE_INSTANCE *temp = ((IPSO_TEMPERATURE_INSTANCE *) instance_ptr);
NX_LWM2M_CLIENT_RESOURCE value;
NX_LWM2M_ID resource_id;

    /* Get resource id */
    nx_lwm2m_client_resource_info_get(resource, &resource_id, NX_NULL);

    switch (resource_id)
    {

    case IPSO_RESOURCE_MIN_VALUE:
    case IPSO_RESOURCE_MAX_VALUE:
    case IPSO_RESOURCE_VALUE:
    case IPSO_RESOURCE_UNITS:

        /* read-only resource */
        return(NX_LWM2M_CLIENT_METHOD_NOT_ALLOWED);

    case IPSO_RESOURCE_RESET_MINMAX:

        /* reset min/max values to current temperature */
        nx_lwm2m_client_resource_float32_set(&value, temp -> temperature);
        if (temp -> min_temperature != temp -> temperature)
        {
            temp -> min_temperature = temp -> temperature;
            nx_lwm2m_client_resource_info_set(&value, IPSO_RESOURCE_MIN_VALUE, NX_NULL);
            nx_lwm2m_client_object_resource_changed(object_ptr, instance_ptr, &value);
        }
        if (temp -> max_temperature != temp -> temperature)
        {
            temp -> max_temperature = temp -> temperature;
            nx_lwm2m_client_resource_info_set(&value, IPSO_RESOURCE_MAX_VALUE, NX_NULL);
            nx_lwm2m_client_object_resource_changed(object_ptr, instance_ptr, &value);
        }

        break;

    default:

        /* unknown resource ID */
        return(NX_LWM2M_CLIENT_NOT_FOUND);
    }

    return(NX_SUCCESS);
}

/* Define the operation callback function of Temperature Object */
UINT ipso_temperature_operation(UINT operation, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT *resource_count, VOID *args_ptr, UINT args_length)
{

    switch (operation)
    {
    case NX_LWM2M_CLIENT_OBJECT_READ:

        /* Call read function */
        return ipso_temperature_read(object_ptr, object_instance_ptr, resource, *resource_count);
    case NX_LWM2M_CLIENT_OBJECT_DISCOVER:

        /* Call discover function */
        return ipso_temperature_discover(object_ptr, object_instance_ptr, resource, resource_count);

    case NX_LWM2M_CLIENT_OBJECT_EXECUTE:

        /* Call execute function */
        return ipso_temperature_execute(object_ptr, object_instance_ptr, resource, args_ptr, args_length);
    default:

        /*Unsupported operation */
        return(NX_LWM2M_CLIENT_NOT_SUPPORTED);
    }
}

/* Update the temperature */
void ipso_temperature_update()
{
NX_LWM2M_FLOAT32 temp;
NX_LWM2M_CLIENT_RESOURCE value;

    /* Simulate some temperature variation */
    switch (NX_RAND() % 10)
    {
    case 0:

        temp = temperature_instance.temperature + 0.1f;
        break;

    case 1:

        temp = temperature_instance.temperature - 0.1f;
        break;

    default:

        temp = temperature_instance.temperature;
        break;
    }

    if (temp != temperature_instance.temperature)
    {

        /* update object value */
        temperature_instance.temperature = temp;
        nx_lwm2m_client_resource_info_set(&value, IPSO_RESOURCE_VALUE, NX_NULL);
        nx_lwm2m_client_resource_float32_set(&value, temp);
        nx_lwm2m_client_object_resource_changed(&temperature_object, &(temperature_instance.object_instance), &value);

        /* update min/max */
        if (temp < temperature_instance.min_temperature)
        {
            temperature_instance.min_temperature = temp;
            nx_lwm2m_client_resource_info_set(&value, IPSO_RESOURCE_MIN_VALUE, NX_NULL);
            nx_lwm2m_client_object_resource_changed(&temperature_object, &(temperature_instance.object_instance), &value);
        }
        if (temp > temperature_instance.max_temperature)
        {
            temperature_instance.max_temperature = temp;
            nx_lwm2m_client_resource_info_set(&value, IPSO_RESOURCE_MAX_VALUE, NX_NULL);
            nx_lwm2m_client_object_resource_changed(&temperature_object, &(temperature_instance.object_instance), &value);
        }
    }
}

/* IPSO Actuation */
/* Define the 'Read' Method */
UINT ipso_actuation_read(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT resource_count)
{
IPSO_ACTUATION_INSTANCE *act = ((IPSO_ACTUATION_INSTANCE *) instance_ptr);
UINT i;
NX_LWM2M_ID resource_id;

    for (i = 0 ; i < resource_count; i++)
    {
        nx_lwm2m_client_resource_info_get(&resource[i], &resource_id, NX_NULL);
        switch (resource_id)
        {

        case IPSO_RESOURCE_ONOFF:

            /* return the on/off value */
            nx_lwm2m_client_resource_boolean_set(&resource[i], act -> onoff);
            break;

        default:

            /* unknown resource ID */
            return(NX_LWM2M_CLIENT_NOT_FOUND);
        }
    }

    return(NX_SUCCESS);
}

/* Define the 'Discover' method */
UINT ipso_actuation_discover(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resources, UINT *resource_count)
{
    if (*resource_count < 1)
    {
        return(NX_LWM2M_CLIENT_BUFFER_TOO_SMALL);
    }

    /* return the list of supported resources IDs */
    *resource_count = 1;
    nx_lwm2m_client_resource_info_set(&resources[0], IPSO_RESOURCE_ONOFF, NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ_WRITE);

    return(NX_SUCCESS);
}

/* Define the 'Write' method */
UINT ipso_actuation_write(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT resource_count, UINT flags)
{
IPSO_ACTUATION_INSTANCE *act = ((IPSO_ACTUATION_INSTANCE *) instance_ptr);
UINT ret;
NX_LWM2M_BOOL onoff;
UINT i;
NX_LWM2M_ID resource_id;

    for (i = 0 ; i < resource_count; i++)
    {
        nx_lwm2m_client_resource_info_get(&resource[i], &resource_id, NX_NULL);
        switch (resource_id)
        {

        case IPSO_RESOURCE_ONOFF:

            /* assign on/off boolean value */
            ret = nx_lwm2m_client_resource_boolean_get(&resource[i], &onoff);
            if (ret != NX_SUCCESS)
            {
                /* invalid value type */
                return(ret);
            }
            if (onoff != act->onoff)
            {
                act->onoff = onoff;

                printf("Set actuation switch %s\n", onoff ? "On" : "Off");
            }
            break;

        default:

            /* unknown resource ID */
            return(NX_LWM2M_CLIENT_NOT_FOUND);
        }
    }

    return(NX_SUCCESS);
}

/* Define the 'Create' method */
UINT ipso_actuation_create(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_ID *instance_id, NX_LWM2M_CLIENT_RESOURCE *resource, UINT resource_count)
{
UINT status;

    /* Check if there is free instance */
    if (actuation_instance_new_created)
    {
        return(NX_LWM2M_CLIENT_NO_MEMORY);
    }

    status = ipso_actuation_write(object_ptr, &(actuation_instance_new.object_instance), resource, resource_count, NX_NULL);
    if (status)
    {
        return(status);
    }

    /* Add a new instance */
    status = nx_lwm2m_client_object_instance_add(object_ptr, &(actuation_instance_new.object_instance), instance_id);
    if (status)
    {
        return(status);
    }

    actuation_instance_new_created = NX_TRUE;
    return(NX_SUCCESS);
}

/* Define the 'Delete' method */
UINT ipso_actuation_delete(NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *instance_ptr)
{
UINT status;

    /* Only support deleting the created new instance */
    if (!actuation_instance_new_created || instance_ptr != &(actuation_instance_new.object_instance))
    {
        return(NX_LWM2M_CLIENT_METHOD_NOT_ALLOWED);
    }

    /* Remove the instance from instance list */
    status = nx_lwm2m_client_object_instance_remove(object_ptr, instance_ptr);
    if (status)
    {
        return(status);
    }

    actuation_instance_new_created = NX_FALSE;
    return(status);
}

/* Define the operation callback function of Actuation Object */
UINT ipso_actuation_operation(UINT operation, NX_LWM2M_CLIENT_OBJECT *object_ptr, NX_LWM2M_CLIENT_OBJECT_INSTANCE *object_instance_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT *resource_count, VOID *args_ptr, UINT args_length)
{
UINT write_op;

    switch (operation)
    {
    case NX_LWM2M_CLIENT_OBJECT_READ:

        /* Call read function */
        return ipso_actuation_read(object_ptr, object_instance_ptr, resource, *resource_count);
    case NX_LWM2M_CLIENT_OBJECT_DISCOVER:

        /* Call discover function */
        return ipso_actuation_discover(object_ptr, object_instance_ptr, resource, resource_count);
    case NX_LWM2M_CLIENT_OBJECT_WRITE:

        /* Get the type of write operation */
        write_op = *(UINT *)args_ptr;

        /* Call write function */
        return ipso_actuation_write(object_ptr, object_instance_ptr, resource, *resource_count, write_op);
    case NX_LWM2M_CLIENT_OBJECT_CREATE:

        /* Call create function */
        return ipso_actuation_create(object_ptr, (NX_LWM2M_ID *)args_ptr, resource, *resource_count);
    case NX_LWM2M_CLIENT_OBJECT_DELETE:

        /* Call delete function */
        return ipso_actuation_delete(object_ptr, object_instance_ptr);
    default:

        /* Unsupported operation */
        return(NX_LWM2M_CLIENT_NOT_SUPPORTED);
    }
}

/* Define the 'Read' method */
UINT device_read(NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT resource_count)
{
UINT i;
NX_LWM2M_ID resource_id;

    for (i = 0; i < resource_count; i++)
    {
        nx_lwm2m_client_resource_info_get(&resource[i], &resource_id, NX_NULL);
        switch (resource_id)
        {
        case NX_LWM2M_CLIENT_DEVICE_MANUFACTURER_ID:

            /* return 'Manufacturer' string */
            nx_lwm2m_client_resource_string_set(&resource[i], (CHAR *)manufacturer, sizeof(manufacturer) - 1);
            break;

        default:

            return(NX_LWM2M_CLIENT_NOT_FOUND);
        }
    }

    return(NX_SUCCESS);
}

/* Define the operation callback function of Device Object */
UINT device_operation(UINT operation, NX_LWM2M_CLIENT *client_ptr, NX_LWM2M_CLIENT_RESOURCE *resource, UINT *resource_count, VOID *args_ptr, UINT args_length)
{

    switch (operation)
    {
    case NX_LWM2M_CLIENT_OBJECT_READ:

        /* Call read function */
        return device_read(client_ptr, resource, *resource_count);
    case NX_LWM2M_CLIENT_OBJECT_DISCOVER:

        /* Call discover function */
        *resource_count = 1;
        nx_lwm2m_client_resource_info_set(resource, NX_LWM2M_CLIENT_DEVICE_MANUFACTURER_ID, NX_LWM2M_CLIENT_RESOURCE_OPERATION_READ);
        return(NX_SUCCESS);
    default:

        /* Unsupported operation */
        return(NX_LWM2M_CLIENT_NOT_SUPPORTED);
    }
}

/* Firmware Package URI callback */
UINT firmware_package_uri(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, const CHAR *uri, UINT uri_length)
{

    if (uri_length == 0)
    {
        printf("Firmware package URI: Cancel Download\n");

        /* cancel package download */
        firmware_timer = 0;
        firmware_state = NX_LWM2M_CLIENT_FIRMWARE_STATE_IDLE;

        nx_lwm2m_client_firmware_state_set(firmware_ptr, NX_LWM2M_CLIENT_FIRMWARE_STATE_IDLE);
        nx_lwm2m_client_firmware_result_set(firmware_ptr, NX_LWM2M_CLIENT_FIRMWARE_RESULT_INIT);
    }
    else if (firmware_timer != 0)
    {
        printf("Firmware package URI: Download already in progress!\n");

        /* download in progress */
        return(NX_LWM2M_CLIENT_NOT_ACCEPTABLE);
    }
    else
    {
        static CHAR tmp[256];
        if (uri_length > 255)
        {
            uri_length = 255;
        }
        if (uri_length > 0)
        {
            memcpy(tmp, uri, uri_length); /* Use case of memcpy is verified. */
        }
        tmp[uri_length] = 0;
        printf("Firmware package URI: start downloading '%s'...\n", tmp);

        /* Emulate the firmware "downloading" */
        firmware_timer = 30;
        firmware_state = NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADING;

        nx_lwm2m_client_firmware_state_set(firmware_ptr, NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADING);
        nx_lwm2m_client_firmware_result_set(firmware_ptr, NX_LWM2M_CLIENT_FIRMWARE_RESULT_INIT);
    }

    return(NX_SUCCESS);
}

/* Firmware Update callback */
UINT firmware_update(NX_LWM2M_CLIENT_FIRMWARE *firmware_ptr, NX_LWM2M_BOOL update_objects, const CHAR *args_ptr, UINT args_length)
{
    if (firmware_state == NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADED)
    {
        printf("Firmware update: start updating firmware...\n");

        /* Emulate the firmware "updating" */
        firmware_timer = 15;
        firmware_state = NX_LWM2M_CLIENT_FIRMWARE_STATE_UPDATING;

        nx_lwm2m_client_firmware_state_set(firmware_ptr, NX_LWM2M_CLIENT_FIRMWARE_STATE_UPDATING);

        return(NX_SUCCESS);
    }

    printf("Firmware update: cannot update firmware: invalid state.\n");

    return(NX_LWM2M_CLIENT_NOT_ACCEPTABLE);
}

/* Update the firmware state */
void firmware_state_update()
{

    /* firmware update timers */
    if (firmware_timer != 0)
    {
        if (--firmware_timer == 0)
        {
            if (firmware_state == NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADING)
            {
                printf("Firmware update: package downloaded!\n");
                firmware_state = NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADED;

                nx_lwm2m_client_firmware_state_set(&firmware, NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADED);
            }
            else
            {
                printf("Firmware update: updated!\n");
                firmware_state = NX_LWM2M_CLIENT_FIRMWARE_STATE_IDLE;

                nx_lwm2m_client_firmware_state_set(&firmware, NX_LWM2M_CLIENT_FIRMWARE_STATE_IDLE);
                nx_lwm2m_client_firmware_result_set(&firmware, NX_LWM2M_CLIENT_FIRMWARE_RESULT_SUCCESS);

                /* Update firmware version */
                firmware_version_number++;
                firmware_version[sizeof(firmware_version) - 2] = firmware_version_number + '0';
                nx_lwm2m_client_firmware_package_info_set(&firmware, firmware_name, sizeof(firmware_name) - 1, firmware_version, sizeof(firmware_version) - 1);
            }
        }
        else if (firmware_timer % 5 == 0)
        {
            printf("Firmware update: %s...\n", firmware_state == NX_LWM2M_CLIENT_FIRMWARE_STATE_DOWNLOADING ? "downloading" : "updating");
        }
    }

}

/* Define the session state callback */
void session_callback(NX_LWM2M_CLIENT_SESSION *session_ptr, UINT state)
{
    printf("LWM2M Callback: -> %d\n", state);

    switch (state)
    {

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_REQUESTING:

        printf("Start client initiated bootstrap\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_INITIATED:

        printf("Got message from boostrap server\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_FINISHED:

         /* Bootstrap session done, we can register to the LWM2M Server */
        printf( "Boostrap finished.\n");
#ifdef BOOTSTRAP
        tx_semaphore_put(&semaphore_bootstarp_finish);
#endif
        break;

    case NX_LWM2M_CLIENT_SESSION_BOOTSTRAP_ERROR:

        /* Failed to Bootstrap the LWM2M Client. */
        printf( "Failed to boostrap device, error=%02x\n", nx_lwm2m_client_session_error_get(session_ptr));
        break;

    case NX_LWM2M_CLIENT_SESSION_REGISTERED:

        /* Registration to the LWM2M Client done. */
        printf( "LWM2M device registered.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_DISABLED:

        /* Registration to the LWM2M Client done. */
        printf( "LWM2M device disabled.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_DEREGISTERED:

        /* Registration to the LWM2M Client done. */
        printf( "LWM2M device deregistered.\n");
        break;

    case NX_LWM2M_CLIENT_SESSION_ERROR:

        /* Failed to register to the LWM2M Client. */
        printf( "Failed to register device, error=%02x\n", nx_lwm2m_client_session_error_get(session_ptr));
        break;
    }
}

/* Application main thread */
void application_thread(ULONG info)
{

NX_LWM2M_ID server_id = 0;
NXD_ADDRESS server_addr;
UINT udp_port = 0;
UINT status;

#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES && defined (BOOTSTRAP)
CHAR *server_uri = NX_NULL;
UINT server_uri_len = 0;
UCHAR security_mode = 0;
#ifdef USE_DTLS
UCHAR *pub_key_or_id = NX_NULL;
UINT pub_key_or_id_len = 0;
UCHAR *secret_key = NX_NULL;
UINT secret_key_len = 0;
#endif /* USE_DTLS */
#endif /* BOOTSTRAP */

#ifndef SAMPLE_DHCP_DISABLE
    dhcp_wait();
#endif /* SAMPLE_DHCP_DISABLE */

#ifndef SAMPLE_DNS_DISABLE
    /* Create DNS instance.  */
    dns_create();
#endif /* SAMPLE_DNS_DISABLE */

    /* Create the LWM2M client */
    status = nx_lwm2m_client_create(&client, &ip_0, &pool_0, LWM2M_CLIENT_ENDPOINT, sizeof(LWM2M_CLIENT_ENDPOINT) - 1, NX_NULL, 0, NX_LWM2M_CLIENT_BINDING_U, client_stack, sizeof(client_stack), 4);
    if (status)
    {
        return;
    }

    /* Define our custom objects: */
    /* Add Temperature Object */
    status = nx_lwm2m_client_object_add(&client, &temperature_object, IPSO_TEMPERATURE_OBJECT_ID, ipso_temperature_operation);
    if (status)
    {
        return;
    }

    /* Define a single instance */
    temperature_instance.temperature = 22.5f;
    temperature_instance.min_temperature = temperature_instance.temperature;
    temperature_instance.max_temperature = temperature_instance.temperature;
    status = nx_lwm2m_client_object_instance_add(&temperature_object, &temperature_instance.object_instance, NX_NULL);
    if (status)
    {
        return;
    }

    /* Add Actuation Object */
    status = nx_lwm2m_client_object_add(&client, &actuation_object, IPSO_ACTUATION_OBJECT_ID, ipso_actuation_operation);
    if (status)
    {
        return;
    }

    /* Define a single instance */
    actuation_instance.onoff = NX_FALSE;
    status = nx_lwm2m_client_object_instance_add(&actuation_object, &actuation_instance.object_instance, NX_NULL);
    if (status)
    {
        return;
    }

    /* Add firmware update object */
    status = nx_lwm2m_client_firmware_create(&firmware, &client, NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_HTTP|NX_LWM2M_CLIENT_FIRMWARE_PROTOCOL_HTTPS, NX_NULL, firmware_package_uri, firmware_update);
    if (status)
    {
        return;
    }

    /* Set firmware info */
    nx_lwm2m_client_firmware_package_info_set(&firmware, firmware_name, sizeof(firmware_name) - 1, firmware_version, sizeof(firmware_version) - 1);

    /* Setup device object callback */
    nx_lwm2m_client_device_callback_set(&client, device_operation);

    /* Create a session */
    status = nx_lwm2m_client_session_create(&session, &client, session_callback);
    if (status)
    {
        return;
    }

    /* start bootstrap/lwm2m session */
    server_addr.nxd_ip_version = NX_IP_VERSION_V4;
    server_addr.nxd_ip_address.v4 = LWM2M_SERVER_ADDRESS;

#ifdef NX_SECURE_ENABLE_DTLS

    /* Create the DTLS Client Session.  */
    dtls_setup(&dtls_session, LWM2M_DTLS_KEY, sizeof(LWM2M_DTLS_KEY) - 1, LWM2M_DTLS_IDENTITY, sizeof(LWM2M_DTLS_IDENTITY) - 1);
#endif /* NX_SECURE_ENABLE_DTLS */

#if NX_LWM2M_CLIENT_MAX_SECURITY_INSTANCES && defined (BOOTSTRAP)

#ifdef USE_DTLS
    printf("Start boostraping with DTLS\r\n");
    status = nx_lwm2m_client_session_bootstrap_dtls(&session, 0, &server_addr, LWM2M_BOOTSTRAP_SERVER_DTLS_PORT, &dtls_session);
#else
    printf("Start boostraping\r\n");
    status = nx_lwm2m_client_session_bootstrap(&session, 0, &server_addr, LWM2M_BOOTSTRAP_SERVER_PORT);
#endif
    if (status)
    {
        return;
    }

    tx_semaphore_get(&semaphore_bootstarp_finish, NX_WAIT_FOREVER);

    /* Get the info for register */
#ifdef USE_DTLS
    status = nx_lwm2m_client_session_register_info_get(&session, NX_LWM2M_CLIENT_RESERVED_ID, &server_id, &server_uri, &server_uri_len, &security_mode, &pub_key_or_id, &pub_key_or_id_len, NX_NULL, NX_NULL, &secret_key, &secret_key_len);
    if (status || (security_mode != NX_LWM2M_CLIENT_SECURITY_MODE_PRE_SHARED_KEY))
    {
        return;
    }
#else
    status = nx_lwm2m_client_session_register_info_get(&session, NX_LWM2M_CLIENT_RESERVED_ID, &server_id, &server_uri, &server_uri_len, &security_mode, NX_NULL, NX_NULL, NX_NULL, NX_NULL, NX_NULL, NX_NULL);
    if (status || (security_mode != NX_LWM2M_CLIENT_SECURITY_MODE_NOSEC))
    {
        return;
    }
#endif

    printf("Got LWM2M server info, uri='%s', id=%d\r\n", server_uri, server_id);

    /* Get IP address and UDP port from server URI */
    status = uri_parse(server_uri, &server_addr, &udp_port);
    if (status)
    {
        return;
    }

#ifdef USE_DTLS

    /* Re-create the DTLS Client Session.  */
    nx_secure_dtls_session_delete(&dtls_session);
    dtls_setup(&dtls_session, secret_key, secret_key_len, pub_key_or_id, pub_key_or_id_len);
#endif /* USE_DTLS */

#else /* !BOOTSTRAP */

    /* Set the info for register */
    server_id = LWM2M_SERVER_ID;

#ifdef USE_DTLS
    udp_port = LWM2M_SERVER_DTLS_PORT;
#else
    udp_port = LWM2M_SERVER_PORT;
#endif /* USE_DTLS */

#endif /* BOOTSTRAP */

#ifdef USE_DTLS
    printf("Register to LWM2M server with DTLS\r\n");
    status = nx_lwm2m_client_session_register_dtls(&session, server_id, &server_addr, udp_port, &dtls_session);
#else
    printf("Register to LWM2M server\r\n");
    status = nx_lwm2m_client_session_register(&session, server_id, &server_addr, udp_port);
#endif /* USE_DTLS */
    if (status)
    {
        return;
    }

    /* Application main loop */
    while (1)
    {

        /* application code... */
        tx_thread_sleep(NX_IP_PERIODIC_RATE);

        /* Update the firmware state */
        firmware_state_update();

        /* Update the temperature */
        ipso_temperature_update();
    }

    /* Terminate the LWM2M Client */
    nx_lwm2m_client_delete(&client);
}

void main(void)
{
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


void tx_application_define(void *first_unused_memory)
{
UINT  status;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status =  nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 1536,  (ULONG*)(((int)packet_pool_area + 15) & ~15) , sizeof(packet_pool_area));

    /* Check for pool creation error.  */
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0,
                          "NetX IP Instance 0",
                          IPV4_ADDRESS,
                          IPV4_NETWORK_MASK,
                          &pool_0,
                          NETWORK_DRIVER,
                          (UCHAR*)ip_thread_stack,
                          sizeof(ip_thread_stack),
                          1);

    /* Check for IP create errors.  */
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status =  nx_arp_enable(&ip_0, (void *)arp_space_area, sizeof(arp_space_area));

    /* Check for ARP enable errors.  */
    if (status)
        error_counter++;

    /* Check for IP gateway errors.  */
    if (status)
        error_counter++;

    /* Enable ICMP.  */
    status =  nxd_icmp_enable(&ip_0);

    /* Check for errors.  */
    if (status)
        error_counter++;

    /* Enable TCP traffic.  */
    status =  nx_tcp_enable(&ip_0);

    /* Check for TCP enable errors.  */
    if (status)
        error_counter++;

    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&ip_0);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

#ifdef BOOTSTRAP
    tx_semaphore_create(&semaphore_bootstarp_finish, "semaphore bootstarp finish", 0);
#endif /* BOOTSTRAP */

    /* Create the main thread.  */
    tx_thread_create(&main_thread, "main thread", application_thread, 0,
                     main_stack, sizeof(main_stack),
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
}

/* DHCP */
#ifndef SAMPLE_DHCP_DISABLE
void dhcp_wait(void)
{

ULONG   actual_status;
ULONG   ip_address;
ULONG   network_mask;
ULONG   gw_address;

    printf("DHCP In Progress...\n");

    /* Create the DHCP instance.  */
    nx_dhcp_create(&dhcp_client, &ip_0, "dhcp_client");

    /* Start the DHCP Client.  */
    nx_dhcp_start(&dhcp_client);

    /* Wait util address is solved. */
    nx_ip_status_check(&ip_0, NX_IP_ADDRESS_RESOLVED, &actual_status, NX_WAIT_FOREVER);

    /* Get IP address. */
    nx_ip_address_get(&ip_0, &ip_address, &network_mask);
    nx_ip_gateway_address_get(&ip_0, &gw_address);

    /* Output IP address. */
    printf("IP address: %d.%d.%d.%d\r\nMask: %d.%d.%d.%d\r\nGateway: %d.%d.%d.%d\r\n",
           (ip_address >> 24),
           (ip_address >> 16 & 0xFF),
           (ip_address >> 8 & 0xFF),
           (ip_address & 0xFF),
           (network_mask >> 24),
           (network_mask >> 16 & 0xFF),
           (network_mask >> 8 & 0xFF),
           (network_mask & 0xFF),
           (gw_address >> 24),
           (gw_address >> 16 & 0xFF),
           (gw_address >> 8 & 0xFF),
           (gw_address & 0xFF));
}
#endif /* SAMPLE_DHCP_DISABLE  */

#ifndef SAMPLE_DNS_DISABLE
/* DNS.  */
static UINT dns_create()
{

UINT    status;
ULONG   dns_server_address[3];
UINT    dns_server_address_size = 12;

    /* Create a DNS instance for the Client.  Note this function will create
       the DNS Client packet pool for creating DNS message packets intended
       for querying its DNS server. */
    status = nx_dns_create(&dns_client, &ip_0, (UCHAR *)"DNS Client");
    if (status)
    {
        return(status);
    }

    /* Is the DNS client configured for the host application to create the packet pool? */
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL

    /* Yes, use the packet pool created above which has appropriate payload size
       for DNS messages. */
    status = nx_dns_packet_pool_set(&dns_client, ip_0.nx_ip_default_packet_pool);
    if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }
#endif /* NX_DNS_CLIENT_USER_CREATE_PACKET_POOL */

#ifndef SAMPLE_DHCP_DISABLE
    /* Retrieve DNS server address.  */
    nx_dhcp_interface_user_option_retrieve(&dhcp_client, 0, NX_DHCP_OPTION_DNS_SVR, (UCHAR *)(dns_server_address),
                                           &dns_server_address_size);
#else
    dns_server_address[0] = DNS_SERVER_ADDRESS;
#endif /* SAMPLE_DHCP_DISABLE */

    /* Add an IPv4 server address to the Client list. */
    status = nx_dns_server_add(&dns_client, dns_server_address[0]);
    if (status)
    {
        nx_dns_delete(&dns_client);
        return(status);
    }

    /* Output DNS Server address.  */
    printf("DNS Server address: %lu.%lu.%lu.%lu\r\n",
           (dns_server_address[0] >> 24),
           (dns_server_address[0] >> 16 & 0xFF),
           (dns_server_address[0] >> 8 & 0xFF),
           (dns_server_address[0] & 0xFF));

    return(NX_SUCCESS);
}
#endif /* SAMPLE_DNS_DISABLE */

/* LwM2M server uri parse.  */
static UINT uri_parse(const char *uri, NXD_ADDRESS *ip_addr, UINT *udp_port)
{
#ifndef SAMPLE_DNS_DISABLE
UINT    i;
UINT    dot_count = 0;
UINT    temp = 0;
ULONG   ip_address = 0;
UCHAR   address_found = NX_FALSE;

    /* coap-URI = "coap:" "//" host [ ":" port ] path-abempty [ "?" query ]
       coaps-URI = "coaps:" "//" host [ ":" port ] path-abempty [ "?" query ] */
    if (memcmp(uri, "coap://", sizeof("coap://") - 1) == 0)
    {

        /* Set the host ptr.  */
        uri += (sizeof("coap://") - 1);
        *udp_port = LWM2M_SERVER_PORT;
    }
    else if (memcmp(uri, "coaps://", sizeof("coaps://") - 1) == 0)
    {

        /* Set the host ptr.  */
        uri += (sizeof("coaps://") - 1);
        *udp_port = LWM2M_SERVER_DTLS_PORT;
    }
    else
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Try to detect whether the host is numerical IP address. */
    for (i = 0; ; i++)
    {

        if (uri[i] >= '0' && uri[i] <= '9')
        {
            temp = temp * 10 + (uri[i] - '0');
            if (temp > 0xFF )
            {
                break;
            }
        }
        else if (uri[i] == '.')
        {
            if (dot_count++ == 3)
            {
                break;
            }
            ip_address = (ip_address << 8) + temp;
            temp = 0;
        }
        else if (uri[i] == ':' || uri[i] == '/' || uri[i] == '\0')
        {
            if (dot_count != 3)
            {
                break;
            }
            ip_address = (ip_address << 8) + temp;
        
            /* Set the address.  */
            ip_addr -> nxd_ip_version = NX_IP_VERSION_V4;
            ip_addr -> nxd_ip_address.v4 = ip_address;
            address_found = NX_TRUE;
            break;
        }
        else
        {
            break;
        }
    }
    
    /* Split host and port. */
    while(1)
    {
        if (uri[i] == ':' || uri[i] == '/' || uri[i] == '\0')
        {

            /* Store the host uri.  */
            if (i == 0 || i >= 255)
            {
                return(NX_NOT_SUCCESSFUL);
            }
            memcpy(host_buffer, uri, i); /* Use case of memcpy is verified. */
            host_buffer[i] = NX_NULL;

            if (uri[i] == ':')
            {
                temp = 0;
                i++;

                /* Get port number. */
                while (uri[i] >= '0' && uri[i] <= '9')
                {
                    temp = temp * 10 + (uri[i++] - '0');
                    if (temp > 0xFFFF)
                    {
                        break;
                    }
                }

                if (temp > 0 && temp <= 0xFFFF)
                {
                    *udp_port = temp;
                }
            }
            break;
        }

        i++;
    }
    
    /* Check if found the address.  */
    if (address_found == NX_FALSE)
    {
        
        /* Resolve the host name by DNS.  */
        if (nxd_dns_host_by_name_get(&dns_client, (UCHAR *)host_buffer, ip_addr, NX_IP_PERIODIC_RATE, NX_IP_VERSION_V4))
            return(NX_NOT_SUCCESSFUL);
    }
#else

    /* XXX parse URI for real... */
    ip_addr -> nxd_ip_version = NX_IP_VERSION_V4;
    ip_addr -> nxd_ip_address.v4 = LWM2M_SERVER_ADDRESS;

#ifdef USE_DTLS
    *udp_port = LWM2M_SERVER_DTLS_PORT;
#else
    *udp_port = LWM2M_SERVER_PORT;
#endif /* USE_DTLS */

#endif /* SAMPLE_DNS_DISABLE  */

    return(NX_SUCCESS);
}

#ifdef NX_SECURE_ENABLE_DTLS
static UINT dtls_setup(NX_SECURE_DTLS_SESSION *dtls_session_ptr, UCHAR *key, UINT key_len, UCHAR *identity, UINT identity_len)
{
UINT status;

    /* Create the DTLS Client Session.  */
    status = nx_secure_dtls_session_create(dtls_session_ptr, &crypto_tls_ciphers,
                                           dtls_metadata_buffer, sizeof(dtls_metadata_buffer),
                                           dtls_packet_buffer, sizeof(dtls_packet_buffer), 0, NX_NULL, 0);
    if (status)
    {
        return(status);
    }

    /* Set the PSK.  */
    status = nx_secure_tls_psk_add(&(dtls_session_ptr -> nx_secure_dtls_tls_session),
                                   key, key_len, identity, identity_len, NX_NULL, 0);
    status += nx_secure_tls_client_psk_set(&(dtls_session_ptr -> nx_secure_dtls_tls_session),
                                           key, key_len, identity, identity_len, NX_NULL, 0);
    if (status)
    {
        nx_secure_dtls_session_delete(dtls_session_ptr);
        return(status);
    }

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_DTLS */
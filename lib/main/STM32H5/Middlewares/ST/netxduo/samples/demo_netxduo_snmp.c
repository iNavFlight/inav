/* This is a small demo of the NetX SNMP Agent on the high-performance NetX TCP/IP stack.  
   This demo relies on ThreadX and NetX to show simple SNMP GET/GETNEXT/SET requests on 
   the SNMP MIB-2 objects.  */

#include  "tx_api.h"
#include  "nx_api.h"
#include  "nxd_snmp.h"
#include  "demo_snmp_helper.h"

#define     DEMO_STACK_SIZE         4096    
#define     AGENT_PRIMARY_ADDRESS   IP_ADDRESS(1,2,3,4) 

/* Define the ThreadX and NetX object control blocks...  */

TX_THREAD               thread_0;
NX_PACKET_POOL          pool_0;
NX_IP                   ip_0;
NX_SNMP_AGENT           my_agent;


#ifdef FEATURE_NX_IPV6

/* Indicate if using IPv6 to communicate with SNMP servers. Note that
   IPv6 must be enabled in the NetX Duo library first. Further, IPv6
   and ICMPv6 services are enabled before starting the SNMP agent. */

/* #define USE_IPV6 */

#endif /* FEATURE_NX_IPV6 */

/* To use SNMPv3 features such as authentication and encryption, define NX_SNMP_DISABLE_V3.  */

/* Define authentication and privacy keys.  */

#ifdef AUTHENTICATION_REQUIRED
NX_SNMP_SECURITY_KEY    my_authentication_key;
#endif

#ifdef PRIVACY_REQUIRED
NX_SNMP_SECURITY_KEY    my_privacy_key;
#endif

/* Define an error counter variable. */
UINT error_counter = 0;

/* This binds a secondary interfaces to the primary IP network interface 
   if SNMP is required for required for that interface. */
/* #define  MULTI_HOMED_DEVICE */

/* Define function prototypes.  A generic ram driver is used in this demo.  However to properly 
   run an SNMP agent demo, a real driver should be substituted. */

VOID    thread_agent_entry(ULONG thread_input);
VOID    _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);
UINT    mib2_get_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *object_requested, NX_SNMP_OBJECT_DATA *object_data);
UINT    mib2_getnext_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *object_requested, NX_SNMP_OBJECT_DATA *object_data);
UINT    mib2_set_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *object_requested, NX_SNMP_OBJECT_DATA *object_data);
UINT    mib2_username_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *username);
VOID    mib2_variable_update(NX_IP *ip_ptr, NX_SNMP_AGENT *agent_ptr);


UCHAR context_engine_id[] = {0x80, 0x00, 0x0d, 0xfe, 0x03, 0x00, 0x11, 0x23, 0x23, 0x44, 0x55};
UINT  context_engine_size = 11;
UCHAR context_name[] = {0x69, 0x6e, 0x69, 0x74, 0x69, 0x61, 0x6c};
UINT  context_name_size = 7;

/* Define main entry point.  */

int main()
{

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */
void    tx_application_define(void *first_unused_memory)
{

UCHAR   *pointer;
UINT    status;


    /* Setup the working pointer.  */
    pointer =  (UCHAR *) first_unused_memory;

    status = tx_thread_create(&thread_0, "agent thread", thread_agent_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    if (status != NX_SUCCESS)
    {
         return;
    }

    pointer =  pointer + DEMO_STACK_SIZE;


    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Packet Pool 0", 2048, pointer, 20000);

    if (status != NX_SUCCESS)
    {
         return;
    }

    pointer = pointer + 20000;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "SNMP Agent IP Instance", AGENT_PRIMARY_ADDRESS, 
                        0xFFFFFF00UL, &pool_0, _nx_ram_network_driver,
                        pointer, 4096, 1);

    if (status != NX_SUCCESS)
    {
         return;
    }

    pointer =  pointer + 4096;

#ifndef NX_DISABLE_IPV4
    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Enable ICMP for ping.  */
    nx_icmp_enable(&ip_0);
#endif /* NX_DISABLE_IPV4  */
  
    /* Enable UPD processing for IP instance.  */
    nx_udp_enable(&ip_0);

    /* Create an SNMP agent instance.  */
    status = nx_snmp_agent_create(&my_agent, "SNMP Agent", &ip_0, pointer, 4096, &pool_0, 
                                  mib2_username_processing, mib2_get_processing, mib2_getnext_processing, 
                                  mib2_set_processing);

    if (status != NX_SUCCESS)
    {
         return;
    }

    pointer =  pointer + 4096;

#ifdef NX_SNMP_DISABLE_V3
    status = nx_snmp_agent_context_engine_set(&my_agent, context_engine_id, context_engine_size);

    if (status != NX_SUCCESS)
    {
         error_counter++;
    }
#endif /* NX_SNMP_DISABLE_V3 */

    return;
}

VOID thread_agent_entry(ULONG thread_input)
{

UINT status;
#ifdef USE_IPV6
UINT        iface_index, address_index;
NXD_ADDRESS agent_ipv6_address;
#endif

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allow NetX time to get initialized. */
    tx_thread_sleep(NX_IP_PERIODIC_RATE);

#ifdef USE_IPV6

    /* If using IPv6, enable IPv6 and ICMPv6 services and get IPv6 addresses 
       registered with NetX Dou. */

    /* Enable IPv6 on the IP instance. */
    status = nxd_ipv6_enable(&ip_0);

    /* Check for enable errors.  */
    if (status)
    {

        error_counter++;
        return;
     }
    /* Enable ICMPv6 on the IP instance. */
    status = nxd_icmp_enable(&ip_0);

    /* Check for enable errors.  */
    if (status)
    {

        error_counter++;
        return;
    }

    agent_ipv6_address.nxd_ip_address.v6[3] = 0x101;
    agent_ipv6_address.nxd_ip_address.v6[2] = 0x0;
    agent_ipv6_address.nxd_ip_address.v6[1] = 0x0000f101;
    agent_ipv6_address.nxd_ip_address.v6[0] = 0x20010db8;
    agent_ipv6_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Set the primary interface for our DNS IPv6 addresses. */
    iface_index = 0;

    /* This assumes we are using the primary network interface (index 0). */
    status = nxd_ipv6_address_set(&ip_0, iface_index, NX_NULL, 10, &address_index);

    /* Check for link local address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }

    /* Set the host global IP address. We are assuming a 64 
       bit prefix here but this can be any value (< 128). */
    status = nxd_ipv6_address_set(&ip_0, iface_index, &agent_ipv6_address, 64, &address_index);

    /* Check for global address set error.  */
    if (status) 
    {

        error_counter++;
        return;
     }
    
    /* Wait while NetX validates the link local and global address. */
    tx_thread_sleep(5 * NX_IP_PERIODIC_RATE);
#endif

#ifdef AUTHENTICATION_REQUIRED
    
    /* Create an authentication key.  */
    status =  nx_snmp_agent_md5_key_create(&my_agent, (UCHAR *)"authpassword", &my_authentication_key);
    if (status)
      error_counter++;

    /* Use the authentication key.  */
    status =  nx_snmp_agent_authenticate_key_use(&my_agent, &my_authentication_key);
    if (status)
      error_counter++;
    
#endif

#ifdef PRIVACY_REQUIRED

    /* Create a privacy key.  */
    status = nx_snmp_agent_md5_key_create(&my_agent, (UCHAR *)"privpassword", &my_privacy_key);
    if (status)
      error_counter++;
    
    /* Use the privacy key.  */
    status = nx_snmp_agent_privacy_key_use(&my_agent, &my_privacy_key);
    if (status)
      error_counter++;
#endif

    /* Start the SNMP instance.  */
    status = nx_snmp_agent_start(&my_agent);
    if (status)
      error_counter++;

}

/* Define the application's GET processing routine.  */

UINT    mib2_get_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *object_requested, NX_SNMP_OBJECT_DATA *object_data)
{

UINT    i;
UINT    status;

    NX_PARAMETER_NOT_USED(agent_ptr);


    /* Loop through the sample MIB to see if we have information for the supplied variable.  */
    i =  0;
    status =  NX_SNMP_ERROR;
    while (mib2_mib[i].object_name)
    {

        /* See if we have found the matching entry.  */
        status =  nx_snmp_object_compare(object_requested, mib2_mib[i].object_name);

        /* Was it found?  */
        if (status == NX_SUCCESS)
        {

            /* Yes it was found.  */
            break;
        }

        /* Move to the next index.  */
        i++;
    }

    /* Determine if a not found condition is present.  */
    if (status != NX_SUCCESS)
    {


        /* The object was not found - return an error.  */
        return(NX_SNMP_ERROR_NOSUCHNAME);
    }

    /* Determine if the entry has a get function.  */
    if (mib2_mib[i].object_get_callback)
    {

        /* Yes, call the get function.  */
        status =  (mib2_mib[i].object_get_callback)(mib2_mib[i].object_value_ptr, object_data);
    }
    else
    {


        /* No get function, return no access.  */
        status =  NX_SNMP_ERROR_NOACCESS;
    }


    
    /* Return the status.  */
    return(status);
}


/* Define the application's GETNEXT processing routine.  */

UINT    mib2_getnext_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *object_requested, NX_SNMP_OBJECT_DATA *object_data)
{

UINT    i;
UINT    status;

    NX_PARAMETER_NOT_USED(agent_ptr);



    /* Loop through the sample MIB to see if we have information for the supplied variable.  */
    i =  0;
    status =  NX_SNMP_ERROR;
    while (mib2_mib[i].object_name)
    {

        /* See if we have found the next entry.  */
        status =  nx_snmp_object_compare(object_requested, mib2_mib[i].object_name);

        /* Is the next entry the mib greater?  */
        if (status == NX_SNMP_NEXT_ENTRY)
        {

            /* Yes it was found.  */
            break;
        }

        /* Move to the next index.  */
        i++;
    }

    /* Determine if a not found condition is present.  */
    if (status != NX_SNMP_NEXT_ENTRY)
    {


        /* The object was not found - return an error.  */
        return(NX_SNMP_ERROR_NOSUCHNAME);
    }


    /* Copy the new name into the object.  */
    nx_snmp_object_copy(mib2_mib[i].object_name, object_requested);


    /* Determine if the entry has a get function.  */
    if (mib2_mib[i].object_get_callback)
    {

        /* Yes, call the get function.  */
        status =  (mib2_mib[i].object_get_callback)(mib2_mib[i].object_value_ptr, object_data);

        /* Determine if the object data indicates an end-of-mib condition.  */
        if (object_data -> nx_snmp_object_data_type == NX_SNMP_END_OF_MIB_VIEW)
        {

            /* Copy the name supplied in the mib table.  */
            nx_snmp_object_copy(mib2_mib[i].object_value_ptr, object_requested);
        }
    }
    else
    {


        /* No get function, return no access.  */
        status =  NX_SNMP_ERROR_NOACCESS;
    }


    
    /* Return the status.  */
    return(status);
}


/* Define the application's SET processing routine.  */

UINT    mib2_set_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *object_requested, NX_SNMP_OBJECT_DATA *object_data)
{

UINT    i;
UINT    status;

    NX_PARAMETER_NOT_USED(agent_ptr);


    /* Loop through the sample MIB to see if we have information for the supplied variable.  */
    i =  0;
    status =  NX_SNMP_ERROR;
    while (mib2_mib[i].object_name)
    {

        /* See if we have found the matching entry.  */
        status =  nx_snmp_object_compare(object_requested, mib2_mib[i].object_name);

        /* Was it found?  */
        if (status == NX_SUCCESS)
        {

            /* Yes it was found.  */
            break;
        }

        /* Move to the next index.  */
        i++;
    }

    /* Determine if a not found condition is present.  */
    if (status != NX_SUCCESS)
    {


        /* The object was not found - return an error.  */
        return(NX_SNMP_ERROR_NOSUCHNAME);
    }


    /* Determine if the entry has a set function.  */
    if (mib2_mib[i].object_set_callback)
    {

        /* Yes, call the set function.  */
        status =  (mib2_mib[i].object_set_callback)(mib2_mib[i].object_value_ptr, object_data);
    }
    else
    {


        /* No get function, return no access.  */
        status =  NX_SNMP_ERROR_NOACCESS;
    }

    
    /* Return the status.  */
    return(status);
}


/* Define the application's authentication routine.  */

UINT  mib2_username_processing(NX_SNMP_AGENT *agent_ptr, UCHAR *username)
{

    NX_PARAMETER_NOT_USED(agent_ptr);
    NX_PARAMETER_NOT_USED(username);

    /* Update MIB-2 objects. In this example, it is only the SNMP objects.  */
    mib2_variable_update(&ip_0, &my_agent);

    /* No authentication is done, just return success!  */
    return(NX_SUCCESS);
}


/* Define the application's update routine.  */ 

VOID  mib2_variable_update(NX_IP *ip_ptr, NX_SNMP_AGENT *agent_ptr)
{

    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(agent_ptr);

    /* This section is for updating the snmp parameters defined in the MIB table definition files.  */
}


/* This is an include file for the NetX SNMP demo programs for setting up the MIB for 
   user callback functions. It is not part of the official release of NetX SNMP Agent. */
 
#ifndef SNMP_HELPER_H
#define SNMP_HELPER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {
#endif

#include "tx_api.h"
#include "nx_api.h"
#include "nxd_snmp.h"

/* Define application MIB data structure. Actual application structures would certainly vary.  */

typedef struct MIB_ENTRY_STRUCT
{

    UCHAR       *object_name;
    void        *object_value_ptr;
    UINT        (*object_get_callback)(VOID *source_ptr, NX_SNMP_OBJECT_DATA *object_data);
    UINT        (*object_get_octet_callback)(VOID *source_ptr, NX_SNMP_OBJECT_DATA *object_data, UINT length);
    UINT        (*object_set_callback)(VOID *destination_ptr, NX_SNMP_OBJECT_DATA *object_data);
    UINT        length;
} MIB_ENTRY;

/* Define the MIB-2 "system" group.  */

UCHAR   sysDescr[] =                "NetX SNMP Agent";              /* sysDescr:OctetString                 RO */
UCHAR   sysObjectID[] =             "1.3.6.1.2.1.1";                /* sysObjectID:ObjectID                 RO */
ULONG   sysUpTime =                  0;                             /* sysUpTime:TimeTicks                  RO */
UCHAR   sysContact[128] =           "NetX sysContact Name";         /* sysContact:OctetString               RW */
UCHAR   sysName[128] =              "NetX sysName";                 /* sysName:OctetString                  RW */
UCHAR   sysLocation[128] =          "NetX sysLocation";             /* sysLocation:OctetString              RW */
ULONG   sysServices =               1;                              /* sysServices:Integer                  RW */
ULONG   ipForwarding =              0;                              /* ipForwarding:Integer                 RW */
ULONG   ipDefaultTTL =              NX_IP_TIME_TO_LIVE;             /* ipDefaultTTL:Integer                 RW */

/* Define the MIB-2 "interfaces" group, assuming one interface. Update of these variables could be added to the
   underlying application driver, but for now simple defaults are used.  */     
ULONG   ifLastChange =              2048;                           /* ifLastChange:TimeTicks               RO */
ULONG   ifInOctets =                155;                            /* ifInOctets:Counter                   RO */
ULONG   ifInUcastPkts =             0;                              /* ifInUcastPkts:Counter                RO */
UCHAR   ifDescr[] =                 "NetX Physical Interface";      /* ifDescr:OctetString                  RO */

/* Define the MIB-2 "address translation" group, assuming one address translation.  */

UCHAR   atPhysAddress[] =           {0x00,0x04,0xac,0xe3,0x1d,0xc5};/* atPhysAddress:OctetString            RW */ 
ULONG   atNetworkAddress =          0;                              /* atNetworkAddress:NetworkAddr         RW */
UCHAR   atIPv6NetworkAddress[16];                                   /* atNetworkAddress:NetworkAddr IPv6    RW */


/* Define the actual MIB-2.  */

MIB_ENTRY   mib2_mib[] = {

    /*    OBJECT ID                OBJECT VARIABLE                    GET ROUTINE/ GET_OCTET_ROUTINE            SET ROUTINE      LENGTH */

    {(UCHAR *) "1.3.6.1.2.1.1.1.0",       sysDescr,                   nx_snmp_object_string_get, NX_NULL,      nx_snmp_object_string_set, 0},
    {(UCHAR *) "1.3.6.1.2.1.1.2.0",       sysObjectID,                nx_snmp_object_id_get, NX_NULL,          NX_NULL, 0},
    {(UCHAR *) "1.3.6.1.2.1.1.3.0",       &sysUpTime,                 nx_snmp_object_timetics_get, NX_NULL,    NX_NULL, 0},
    {(UCHAR *) "1.3.6.1.2.1.1.4.0",       sysContact,                 nx_snmp_object_string_get, NX_NULL,      nx_snmp_object_string_set, 0},
    {(UCHAR *) "1.3.6.1.2.1.1.5.0",       sysName,                    nx_snmp_object_string_get, NX_NULL,      nx_snmp_object_string_set, 0},
    {(UCHAR *) "1.3.6.1.2.1.1.6.0",       sysLocation,                nx_snmp_object_string_get, NX_NULL,      nx_snmp_object_string_set, 0},
    {(UCHAR *) "1.3.6.1.2.1.1.7.0",       &sysServices,               nx_snmp_object_integer_get, NX_NULL,     NX_NULL,  0},

    {(UCHAR *) "1.3.6.1.2.1.3.1.1.3.0",   &atNetworkAddress,          nx_snmp_object_ip_address_get, NX_NULL,  nx_snmp_object_ip_address_set, 0},
#ifdef FEATURE_NX_IPV6
     /* Either GET method should work. IPv6 addresses are handled as octet strings and accept any IPv6 address format e.g. addresses with '::'s are accepted as is. */
    {(UCHAR *) "1.3.6.1.2.1.3.1.1.3.1",   &atIPv6NetworkAddress,      nx_snmp_object_ipv6_address_get, NX_NULL, nx_snmp_object_ipv6_address_set, 0},
    {(UCHAR *) "1.3.6.1.2.1.3.1.1.3.2",   &atIPv6NetworkAddress,      NX_NULL, nx_snmp_object_octet_string_get, nx_snmp_object_octet_string_set, 0},
#endif

    {(UCHAR *) "1.3.6.1.2.1.2.2.1.2.0",   ifDescr,                    nx_snmp_object_string_get, NX_NULL,      NX_NULL,  0},
    {(UCHAR *) "1.3.6.1.2.1.3.1.1.2.0",   &atPhysAddress,             NX_NULL, nx_snmp_object_octet_string_get, nx_snmp_object_octet_string_set, 0},
    {(UCHAR *) "1.3.6.1.2.1.2.2.1.9.0",   &ifLastChange,              nx_snmp_object_timetics_get, NX_NULL,    nx_snmp_object_timetics_set,  0},
    {(UCHAR *) "1.3.6.1.2.1.2.2.1.10.0",  &ifInOctets,                nx_snmp_object_counter_get, NX_NULL,     nx_snmp_object_counter_set,  0},
    {(UCHAR *) "1.3.6.1.2.1.2.2.1.11.0",  &ifInUcastPkts,             nx_snmp_object_counter64_get, NX_NULL,   nx_snmp_object_counter64_set,  0},

    {(UCHAR *) "1.3.6.1.2.1.4.1.0",       &ipForwarding,              nx_snmp_object_integer_get, NX_NULL,     nx_snmp_object_integer_set,  0},
    {(UCHAR *) "1.3.6.1.2.1.4.2.0",       &ipDefaultTTL,              nx_snmp_object_integer_get, NX_NULL,     NX_NULL,  0},

    {(UCHAR *) "1.3.6.1.7",               (UCHAR *) "1.3.6.1.7",      nx_snmp_object_end_of_mib,  NX_NULL,     NX_NULL, 0},
    {NX_NULL, NX_NULL, NX_NULL, NX_NULL, NX_NULL, 0}

};


#endif /* SNMP_HELPER_H */

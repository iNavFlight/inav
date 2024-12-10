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

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX SNTP Client Component                                            */
/**                                                                       */
/**   Simple Network Time Protocol (SNTP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_sntp_client.h                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Simple Network Time Protocol (SNTP)      */
/*    Client component, including all data types and external references. */
/*    It is assumed that tx_api.h, tx_port.h, nx_api.h, and nx_port.h,    */
/*    have already been included.                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_SNTP_CLIENT_H
#define NXD_SNTP_CLIENT_H

#include "nx_ip.h"

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */ 
extern   "C" {

#endif


#define NXD_SNTP_ID                              0x534E5460UL


/* Conversion between seconds and timer ticks. This must equal the 
   NetX timer tick to seconds ratio.  */ 

#define NX_SNTP_MILLISECONDS_PER_TICK           (1000 / NX_IP_PERIODIC_RATE)   

/* Set minimum and maximum Client unicast poll period (in seconds) for requesting  
   time data.  RFC 4330 polling range is from 16 - 131072 seconds.  
   Note that the RFC 4330 strongly recommends polling intervals of at least 
   one minute to unnecessary reduce demand on public time servers.  */

#define NX_SNTP_CLIENT_MIN_UNICAST_POLL_INTERVAL    64
#define NX_SNTP_CLIENT_MAX_UNICAST_POLL_INTERVAL    131072

/* Define Client request types. Note that this does not limit the 
   NetX SNTP Client from using MANYCAST or MULTICAST discovery options.  */

#define     BROADCAST_MODE      1
#define     UNICAST_MODE        2

#define NX_SNTP_CLIENT_RECEIVE_EVENT                ((ULONG) 0x00000001)    /* Event flag to signal a receive packet event                          */ 

/* Define the minimum size of the packet NTP/SNTP time message 
   (e.g. without authentication data).  */

#define NX_SNTP_TIME_MESSAGE_MIN_SIZE           48


/* Define the maximum size of the packet NTP/SNTP time message (includes 20 bytes for
   optional authentication data).  */

#define NX_SNTP_TIME_MESSAGE_MAX_SIZE           68

/* Define the largest IP4 size for ip address e.g. xxx.xxx.xxx.xxx */

#define NX_SNTP_CLIENT_MAX_IP_ADDRESS_SIZE      15



/* Define fields in the NTP message format.  */

#define REFERENCE_TIME      0
#define ORIGINATE_TIME      1
#define RECEIVE_TIME        2
#define TRANSMIT_TIME       3

/* Define masks for stratum levels.  */

#define STRATUM_KISS_OF_DEATH   0x00
#define STRATUM_PRIMARY         0x01
#define STRATUM_SECONDARY       0x0E  /* 2 - 15 */
#define STRATUM_RESERVED        0xF0  /* 16 - 255*/


/* Kiss of death strings (see Codes below). Applicable when stratum = 0    

                            Code    Meaning
      --------------------------------------------------------------  */


#define       ANYCAST       "ACST"    /* The association belongs to an anycast server.  */
#define       AUTH_FAIL     "AUTH"    /* Server authentication failed.  */
#define       AUTOKEY_FAIL  "AUTO"    /* Autokey sequence failed.  */
#define       BROADCAST     "BCST"    /* The association belongs to a broadcast server.  */
#define       CRYP_FAIL     "CRYP"    /* Cryptographic authentication or identification failed.  */
#define       DENY          "DENY"    /* Access denied by remote server.  */
#define       DROP          "DROP"    /* Lost peer in symmetric mode.  */
#define       DENY_POLICY   "RSTR"    /* Access denied due to local policy.  */
#define       NOT_INIT      "INIT"    /* The association has not yet synchronized for the first time.  */
#define       MANYCAST      "MCST"    /* The association belongs to a manycast server.  */
#define       NO_KEY        "NKEY"    /* No key found.  Either the key was never installed or is not trusted.  */
#define       RATE          "RATE"    /* Rate exceeded.  The server temporarily denied access; client exceeded rate threshold.  */
#define       RMOT          "RMOT"    /* Somebody is tinkering with association from remote host running ntpdc.  OK unless they've stolen your keys.  */
#define       STEP          "STEP"    /* A step change in system time has occurred, but association has not yet resynchronized.  */

/* Define Kiss of Death error codes.  Note: there should be a 1 : 1 correspondence of 
   KOD strings to KOD error codes! */

#define       NX_SNTP_KISS_OF_DEATH_PACKET                  0xF00

#define       NX_SNTP_KOD_ANYCAST               (NX_SNTP_KISS_OF_DEATH_PACKET | 0x01)
#define       NX_SNTP_KOD_AUTH_FAIL             (NX_SNTP_KISS_OF_DEATH_PACKET | 0x02)
#define       NX_SNTP_KOD_AUTOKEY_FAIL          (NX_SNTP_KISS_OF_DEATH_PACKET | 0x03)  
#define       NX_SNTP_KOD_BROADCAST             (NX_SNTP_KISS_OF_DEATH_PACKET | 0x04)     
#define       NX_SNTP_KOD_CRYP_FAIL             (NX_SNTP_KISS_OF_DEATH_PACKET | 0x05)     
#define       NX_SNTP_KOD_DENY                  (NX_SNTP_KISS_OF_DEATH_PACKET | 0x06)          
#define       NX_SNTP_KOD_DROP                  (NX_SNTP_KISS_OF_DEATH_PACKET | 0x07)          
#define       NX_SNTP_KOD_DENY_POLICY           (NX_SNTP_KISS_OF_DEATH_PACKET | 0x08)   
#define       NX_SNTP_KOD_NOT_INIT              (NX_SNTP_KISS_OF_DEATH_PACKET | 0x09)      
#define       NX_SNTP_KOD_MANYCAST              (NX_SNTP_KISS_OF_DEATH_PACKET | 0x0A)      
#define       NX_SNTP_KOD_NO_KEY                (NX_SNTP_KISS_OF_DEATH_PACKET | 0x0B)        
#define       NX_SNTP_KOD_RATE                  (NX_SNTP_KISS_OF_DEATH_PACKET | 0x0C)          
#define       NX_SNTP_KOD_RMOT                  (NX_SNTP_KISS_OF_DEATH_PACKET | 0x0D)          
#define       NX_SNTP_KOD_STEP                  (NX_SNTP_KISS_OF_DEATH_PACKET | 0x0E)          

/* Define SNTP protocol modes. SNTP is limited to primarily Client, server and broadcast modes.  */

#define     PROTOCOL_MODE_RESERVED          0
#define     PROTOCOL_MODE_SYMM_ACTIVE       1
#define     PROTOCOL_MODE_SYMM_PASSIVE      2
#define     PROTOCOL_MODE_CLIENT            3
#define     PROTOCOL_MODE_SERVER_UNICAST    4
#define     PROTOCOL_MODE_SERVER_BROADCAST  5
#define     PROTOCOL_MODE_NTP_CNTL_MSG      6
#define     PROTOCOL_MODE_PRIVATE           7


/* NX SNTP Client configurable options.  */



/* Set the NetX SNTP client thread stack size .  */

#ifndef NX_SNTP_CLIENT_THREAD_STACK_SIZE
#define NX_SNTP_CLIENT_THREAD_STACK_SIZE            2048   
#endif


/* Set the client thread time slice.  */

#ifndef NX_SNTP_CLIENT_THREAD_TIME_SLICE
#define NX_SNTP_CLIENT_THREAD_TIME_SLICE            TX_NO_TIME_SLICE
#endif



#ifndef NX_SNTP_CLIENT_THREAD_PRIORITY                 
#define NX_SNTP_CLIENT_THREAD_PRIORITY              2
#endif


/* Set NetX SNTP client thread preemption threshold.  */

#ifndef NX_SNTP_CLIENT_PREEMPTION_THRESHOLD     
#define NX_SNTP_CLIENT_PREEMPTION_THRESHOLD         NX_SNTP_CLIENT_THREAD_PRIORITY
#endif



/* Configure the NetX SNTP client network parameters */

/* Set Client UDP socket name.  */

#ifndef NX_SNTP_CLIENT_UDP_SOCKET_NAME     
#define NX_SNTP_CLIENT_UDP_SOCKET_NAME              "SNTP Client socket"    
#endif


/* Set port for client to bind UDP socket.  */

#ifndef NX_SNTP_CLIENT_UDP_PORT              
#define NX_SNTP_CLIENT_UDP_PORT                    123      
#endif

/* Set port for client to connect to SNTP server.  */

#ifndef NX_SNTP_SERVER_UDP_PORT              
#define NX_SNTP_SERVER_UDP_PORT                    123      
#endif

/* Set Time to Live (TTL) value for transmitted UDP packets, including manycast and
   multicast transmissions. The default TTL for windows operating system time 
   server is used.  */

#ifndef NX_SNTP_CLIENT_TIME_TO_LIVE    
#define NX_SNTP_CLIENT_TIME_TO_LIVE                  NX_IP_TIME_TO_LIVE
#endif


/* Set maximum queue depth for client socket.*/

#ifndef NX_SNTP_CLIENT_MAX_QUEUE_DEPTH    
#define NX_SNTP_CLIENT_MAX_QUEUE_DEPTH               5
#endif


/* Set the time out (timer ticks) for packet allocation from the client packet pool.  */

#ifndef NX_SNTP_CLIENT_PACKET_TIMEOUT
#define NX_SNTP_CLIENT_PACKET_TIMEOUT               (1 * NX_IP_PERIODIC_RATE)    
#endif


/* Set the NTP/SNTP version of this NTP Client.  */

#ifndef NX_SNTP_CLIENT_NTP_VERSION
#define NX_SNTP_CLIENT_NTP_VERSION                   3
#endif


/* Set minimum NTP/SNTP version the Client will accept from its time server.  */

#ifndef NX_SNTP_CLIENT_MIN_NTP_VERSION
#define NX_SNTP_CLIENT_MIN_NTP_VERSION               3
#endif



/* Define the minimum (numerically highest) stratum the Client will 
   accept for a time server. Valid range is 1 - 15.  */

#ifndef NX_SNTP_CLIENT_MIN_SERVER_STRATUM
#define NX_SNTP_CLIENT_MIN_SERVER_STRATUM           2
#endif


/* Define minimum time difference (msec) between server and client time
   the Client requires to change its local time.  */

#ifndef NX_SNTP_CLIENT_MIN_TIME_ADJUSTMENT
#define NX_SNTP_CLIENT_MIN_TIME_ADJUSTMENT          10
#endif


/* Define maximum time update (msec) the Client will make to its local time
   per update.  */
        
#ifndef NX_SNTP_CLIENT_MAX_TIME_ADJUSTMENT
#define NX_SNTP_CLIENT_MAX_TIME_ADJUSTMENT          180000
#endif


/* Determine if the Client should ignore the maximum time adjustment on startup. If the
   host application has a pretty accurate notion of time, this can be set to false. If not
   the SNTP Client may be unable to apply the first (any) updates. */

#ifndef NX_SNTP_CLIENT_IGNORE_MAX_ADJUST_STARTUP    
#define NX_SNTP_CLIENT_IGNORE_MAX_ADJUST_STARTUP    NX_TRUE
#endif


/* Determine if the Client should create a random delay before starting SNTP polling. */

#ifndef NX_SNTP_CLIENT_RANDOMIZE_ON_STARTUP
#define NX_SNTP_CLIENT_RANDOMIZE_ON_STARTUP         NX_FALSE
#endif


/* Set the maximum time lapse (in seconds) without a time update that can be tolerated by 
   the Client (application). This should be determined by application time sensitivity. 
   For unicast operation, the max lapse could be set to three successive poll requests 
   without an update.  Here we set it to 2 hour based on the RFC recommendation to limit
   traffic congestion.  */

#ifndef NX_SNTP_CLIENT_MAX_TIME_LAPSE
#define NX_SNTP_CLIENT_MAX_TIME_LAPSE              7200
#endif


/* Define a time out (seconds) for the SNTP Client timer.  */

#ifndef NX_SNTP_UPDATE_TIMEOUT_INTERVAL
#define NX_SNTP_UPDATE_TIMEOUT_INTERVAL             1
#endif

/* Define the SNTP Client task sleep interval in timer ticks when
   the SNTP CLient thread is idle. */

#ifndef NX_SNTP_CLIENT_SLEEP_INTERVAL
#define NX_SNTP_CLIENT_SLEEP_INTERVAL                1 
#endif


/* Set the unicast poll interval (in seconds) for Client time update requests. RFC 4330
   recommends a minimum polling interval of once per hour to minimize traffic congestion. */

#ifndef NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL    
#define NX_SNTP_CLIENT_UNICAST_POLL_INTERVAL        3600
#endif


/* Set the Client exponential back off rate for extending Client poll interval.
   To effectively disable, set to 1.  */

#ifndef NX_SNTP_CLIENT_EXP_BACKOFF_RATE             
#define NX_SNTP_CLIENT_EXP_BACKOFF_RATE             2
#endif

/* Determine if the Client requires round trip calculation of SNTP messages.  */

#ifndef NX_SNTP_CLIENT_RTT_REQUIRED             
#define NX_SNTP_CLIENT_RTT_REQUIRED                 NX_FALSE
#endif


/* Define the upper limit of server clock dispersion (usec) the Client
   will accept. To disable this check, set this parameter to 0x0.  */

#ifndef NX_SNTP_CLIENT_MAX_ROOT_DISPERSION
#define NX_SNTP_CLIENT_MAX_ROOT_DISPERSION          50000
#endif


/* Set the limit on consecutive bad updates from server before Client 
   switches to alternate server.  */

#ifndef NX_SNTP_CLIENT_INVALID_UPDATE_LIMIT             
#define NX_SNTP_CLIENT_INVALID_UPDATE_LIMIT         3
#endif

/* Set size of SNTP data in bytes, not including IP and UDP header fields or authentication.  */
#define NX_SNTP_CLIENT_PACKET_DATA_SIZE            48

/* To display date in year/month/date format, set the current year (same year as in NTP time being evaluated) e.g 2015. 
   Otherwise set to zero. */
#ifndef NX_SNTP_CURRENT_YEAR
#define NX_SNTP_CURRENT_YEAR                        2015
#endif /* NX_SNTP_CURRENT_YEAR */


/* Define status levels for SNTP update processing */

#define UPDATE_STATUS_CONTINUE                      1
#define UPDATE_STATUS_BREAK                         2
#define UPDATE_STATUS_ERROR                         3
#define UPDATE_STATUS_SUCCESS                       4


/* Define the number of seconds from 01-01-1990 00:00:00 to 01-01-1999 00:00:00 (last occurrance 
   of a leap second) to be able to define time in year, month, date. If zero, 
   no date time is displayed. */
#ifndef NTP_SECONDS_AT_01011999
#define NTP_SECONDS_AT_01011999                     0xBA368E80
#endif /* NTP_SECONDS_AT_01011999 */

/* Define which epoch time is relative on the host system. */

#define UNIX_EPOCH                 1
#define NTP_EPOCH                  2

    

/* Enumerate months*/

#define JANUARY         1
#define FEBRUARY        2
#define MARCH           3
#define APRIL           4
#define MAY             5
#define JUNE            6
#define JULY            7
#define AUGUST          8
#define SEPTEMBER       9
#define OCTOBER         10
#define NOVEMBER        11
#define DECEMBER        12


/* Compute seconds per month for convenience computating date and time. */

#define SEC_IN_JAN           (31 * SECONDS_PER_DAY)
#define SEC_IN_LEAPFEB       (29 * SECONDS_PER_DAY) 
#define SEC_IN_NONLEAPFEB    (28 * SECONDS_PER_DAY)
#define SEC_IN_MAR           (31 * SECONDS_PER_DAY)
#define SEC_IN_APR           (30 * SECONDS_PER_DAY)
#define SEC_IN_MAY           (31 * SECONDS_PER_DAY)
#define SEC_IN_JUN           (30 * SECONDS_PER_DAY)
#define SEC_IN_JUL           (31 * SECONDS_PER_DAY)
#define SEC_IN_AUG           (31 * SECONDS_PER_DAY)
#define SEC_IN_SEP           (30 * SECONDS_PER_DAY)
#define SEC_IN_OCT           (31 * SECONDS_PER_DAY)
#define SEC_IN_NOV           (30 * SECONDS_PER_DAY)
#define SEC_IN_DEC           (31 * SECONDS_PER_DAY)

/* Compute seconds per year, month,day for convenience computating date and time. */

#define SECONDS_PER_LEAPYEAR        (86400 * 366)
#define SECONDS_PER_NONLEAPYEAR     (86400 * 365)
#define SECONDS_PER_DAY             86400
#define SECONDS_PER_HOUR            3600
#define SECONDS_PER_MINUTE          60


/* Internal SNTP error processing codes.  */

#define NX_SNTP_ERROR_CONSTANT                  0xD00


/* Client side errors.  */
#define NX_SNTP_CLIENT_NOT_INITIALIZED      (NX_SNTP_ERROR_CONSTANT | 0x01)       /* Client not properly initialized to receive time data.  */
#define NX_SNTP_OVER_LIMIT_ON_SERVERS       (NX_SNTP_ERROR_CONSTANT | 0x02)       /* Cannot accept server because client list has reached max # servers.  */      
#define NX_SNTP_INVALID_DOMAIN              (NX_SNTP_ERROR_CONSTANT | 0x03)       /* Invalid domain such as bad IP format or empty string. Applicable to broadcast mode.  */      
#define NX_SNTP_NO_AVAILABLE_SERVERS        (NX_SNTP_ERROR_CONSTANT | 0x04)       /* Client has no available time servers to contact.  */
#define NX_SNTP_INVALID_LOCAL_TIME          (NX_SNTP_ERROR_CONSTANT | 0x05)       /* Client local time has not been set or is invalid.  */
#define NX_SNTP_OUT_OF_DOMAIN_SERVER        (NX_SNTP_ERROR_CONSTANT | 0x06)       /* Broadcast server does not belong to client broadcast domain.  */
#define NX_SNTP_INVALID_DATETIME_BUFFER     (NX_SNTP_ERROR_CONSTANT | 0x07)       /* Insufficient or invalid buffer for writing human readable time date string.  */
#define NX_SNTP_ERROR_CONVERTING_DATETIME   (NX_SNTP_ERROR_CONSTANT | 0x08)       /* An internal error has occurred converting NTP time to mm-dd-yy time format.  */
#define NX_SNTP_UNABLE_TO_CONVERT_DATETIME  (NX_SNTP_ERROR_CONSTANT | 0x09)       /* An internal error has occurred converting NTP time to mm-dd-yy time format.  */
#define NX_SNTP_INVALID_SERVER_ADDRESS      (NX_SNTP_ERROR_CONSTANT | 0x0A)       /* Invalid server type e.g. IPv4 or IPv6 incompatible.                          */
#define NX_SNTP_CLIENT_NOT_STARTED          (NX_SNTP_ERROR_CONSTANT | 0x0B)       /* SNTP Client task is not running */
#define NX_SNTP_CLIENT_ALREADY_STARTED      (NX_SNTP_ERROR_CONSTANT | 0x0C)       /* SNTP Client task is already running */
#define NX_SNTP_PARAM_ERROR                 (NX_SNTP_ERROR_CONSTANT | 0x0D)       /* Invalid non pointer parameter.  */ 

    /* Server side errors */
#define NX_SNTP_SERVER_NOT_AVAILABLE        (NX_SNTP_ERROR_CONSTANT | 0x10)       /* Client will not get any time update service from current server.  */
#define NX_SNTP_NO_UNICAST_FROM_SERVER      (NX_SNTP_ERROR_CONSTANT | 0x11)       /* Client did not receive a valid unicast response from server.  */
#define NX_SNTP_SERVER_CLOCK_NOT_SYNC       (NX_SNTP_ERROR_CONSTANT | 0x12)       /* Server clock not synchronized.  */      
#define NX_SNTP_KOD_SERVER_NOT_AVAILABLE    (NX_SNTP_ERROR_CONSTANT | 0x13)       /* Server sent a KOD packet indicating service temporarily not available.  */      
#define NX_SNTP_KOD_REMOVE_SERVER           (NX_SNTP_ERROR_CONSTANT | 0x14)       /* Server sent a KOD packet indicating service is not available to client (ever).  */      
#define NX_SNTP_SERVER_AUTH_FAIL            (NX_SNTP_ERROR_CONSTANT | 0x15)       /* Server rejects Client packet on basis of missing or invalid authorization data.  */      

/* Bad packet and time update errors */
#define NX_SNTP_INVALID_TIME_PACKET         (NX_SNTP_ERROR_CONSTANT | 0x20)       /* Invalid packet (length or data incorrect).   */
#define NX_SNTP_INVALID_NTP_VERSION         (NX_SNTP_ERROR_CONSTANT | 0x21)       /* Server NTP/SNTP version not incompatible with client.  */      
#define NX_SNTP_INVALID_SERVER_MODE         (NX_SNTP_ERROR_CONSTANT | 0x22)       /* Server association invalid (out of protocol with client).  */
#define NX_SNTP_INVALID_SERVER_PORT         (NX_SNTP_ERROR_CONSTANT | 0x23)       /* Server port does not match what the client expects.  */
#define NX_SNTP_INVALID_IP_ADDRESS          (NX_SNTP_ERROR_CONSTANT | 0x24)       /* Server IP address does not match what the client expects.  */
#define NX_SNTP_INVALID_SERVER_STRATUM      (NX_SNTP_ERROR_CONSTANT | 0x25)       /* Server stratum is invalid or below client stratum.  */
#define NX_SNTP_BAD_SERVER_ROOT_DISPERSION  (NX_SNTP_ERROR_CONSTANT | 0x26)       /* Server root dispersion (clock precision) is too high or invalid value (0) reported.  */
#define NX_SNTP_OVER_INVALID_LIMIT          (NX_SNTP_ERROR_CONSTANT | 0x27)       /* Client over the limit on consecutive server updates with bad data received.  */
#define NX_SNTP_DUPE_SERVER_PACKET          (NX_SNTP_ERROR_CONSTANT | 0x28)       /* Client has received duplicate packets from server.  */
#define NX_SNTP_INVALID_TIMESTAMP           (NX_SNTP_ERROR_CONSTANT | 0x29)       /* Server time packet has one or more invalid time stamps in update message.  */
#define NX_SNTP_INSUFFICIENT_PACKET_PAYLOAD (NX_SNTP_ERROR_CONSTANT | 0x2A)       /* Packet payload not large enough for SNTP message.  */
#define NX_SNTP_INVALID_SNTP_PACKET         (NX_SNTP_ERROR_CONSTANT | 0x2B)       /* Server IP version does not match what the client expects.  */

/* Arithmetic errors or invalid results */
#define NX_SNTP_INVALID_TIME                (NX_SNTP_ERROR_CONSTANT | 0x30)       /* Invalid time resulting from arithmetic operation.  */
#define NX_SNTP_INVALID_RTT_TIME            (NX_SNTP_ERROR_CONSTANT | 0x31)       /* Round trip time correction to server time yields invalid time (e.g. <0).  */
#define NX_SNTP_OVERFLOW_ERROR              (NX_SNTP_ERROR_CONSTANT | 0x32)       /* Overflow error resulting from multiplying/adding two 32 bit (timestamp) numbers.  */      
#define NX_SNTP_SIGN_ERROR                  (NX_SNTP_ERROR_CONSTANT | 0x33)       /* Loss of sign error resulting from multiplying/adding two 32 bit (timestamp) numbers.  */      

/* Time out errors */
#define NX_SNTP_TIMED_OUT_ON_SERVER         (NX_SNTP_ERROR_CONSTANT | 0x44)       /* Client did not receive update from the current server within specified timeout.  */
#define NX_SNTP_MAX_TIME_LAPSE_EXCEEDED     (NX_SNTP_ERROR_CONSTANT | 0x45)       /* Client has not received update from any server within the max allowed time lapse.  */



/* Define the Netx Date Time structure.  */

    typedef struct NX_SNTP_DATE_TIME_STRUCT
    {
        UINT     year;
        UINT     month;
        UINT     day;
        UINT     hour;
        UINT     minute;
        UINT     second;
        UINT     millisecond;                                               /* This is the fraction part of the NTP time data. */
        UINT     time_zone;                                                 /* NTP time is represented in Coordinated Universal Time (UTC). */
        UINT     leap_year;                                                 /* Indicates if current time is in a leap year. */

    } NX_SNTP_DATE_TIME;


/* Define the Netx SNTP Time structure.  */

    typedef struct NX_SNTP_TIME_STRUCT
    {
        ULONG    seconds;                                                   /* Seconds, in the 32 bit field of an NTP time data.  */
        ULONG    fraction;                                                  /* Fraction of a second, in the 32 bit fraction field of an NTP time data.  */

    } NX_SNTP_TIME;


/* Define the NetX SNTP Time Message structure.  The Client only uses the flags field and the transmit_time_stamp field
   in time requests it sends to its time server.  */

    typedef struct NX_SNTP_TIME_MESSAGE_STRUCT
    {
        /* These are represented as 8 bit data fields in the time message format.  */
        UINT flags;                                                         /* Flag containing host's NTP version, mode and leap indicator.  */
        UINT peer_clock_stratum;                                            /* Level of precision in the NTP network. Applicable only in server NTP messages.  */
        UINT peer_poll_interval;                                            /* Frequency at which an NTP host polls its NTP peer. Applicable only in server NTP messages.  */
        UINT peer_clock_precision;                                          /* Precision of the NTP server clock. Applicable only in server NTP messages.  */

        /* These are represented as 32 bit data fields in the time message format*/
        ULONG root_delay;                                                   /* Round trip time from NTP Server to primary reference source. Applicable only in server NTP messages.  */
        ULONG clock_dispersion;                                             /* Server reference clock type (but if stratum is zero, indicates server status when not able to send time updates.  */ 
        UCHAR reference_clock_id[4];                                        /* Maximum error in server clock based to the clock frequency tolerance. Applicable only in server NTP messages.  */ 

        /* These are represented as 64 bit data fields in the time message format*/
        ULONG reference_clock_update_time_stamp[2];                         /* Time at which the server clock was last set or corrected in a server time message.  */
        ULONG originate_time_stamp[2];                                      /* Time at which the Client update request left the Client in a server time message.  */
        ULONG receive_time_stamp[2];                                        /* Time at which the server received the Client request in a server time message.  */
        ULONG transmit_time_stamp[2];                                       /* Time at which the server transmitted its reply to the Client in a server time message (or the time client request was sent in the client request message).  */

        /* Optional authenticator fields.  */
        UCHAR KeyIdentifier[4];                                             /* Key Identifier and Message Digest fields contain the...  */
        UCHAR MessageDigest[16];                                            /* ...message authentication code (MAC) information defined.  */ 

        /* These fields are used internally for 'convert' UCHAR data in NX_SNTP_TIME data e.g. seconds and fractions.  */
        NX_SNTP_TIME reference_clock_update_time;                           /* Time at which the server clock was last set or corrected in a server time message.  */
        NX_SNTP_TIME originate_time;                                        /* Time at which the Client update request left the Client in a server time message.  */
        NX_SNTP_TIME receive_time;                                          /* Time at which the server received the Client request in a server time message.  */
        NX_SNTP_TIME transmit_time;                                         /* Time at which the server transmitted its reply to the Client in a server time message (or the time client request was sent in the client request message).  */

    } NX_SNTP_TIME_MESSAGE;


/* Define the SNTP client structure.  */

    typedef struct NX_SNTP_CLIENT_STRUCT
    {
        ULONG                           nx_sntp_client_id;                       /* SNTP ID for identifying the client service task.  */
        NX_IP                          *nx_sntp_client_ip_ptr;                   /* Pointer to the Client IP instance.  */
        UINT                            nx_sntp_client_interface_index;          /* Index to SNTP network interface  */
        NX_PACKET_POOL                 *nx_sntp_client_packet_pool_ptr;          /* Pointer to the Client packet pool.  */
        UINT                            nx_sntp_client_sleep_flag;               /* The flag indicating the SNTP Client thread is sleeping          */ 
        UINT                            nx_sntp_client_started;                  /* Flag indicating the SNTP Client task is running */
        TX_THREAD                       nx_sntp_client_thread;                   /* The SNTP Client processing thread                               */
        TX_MUTEX                        nx_sntp_client_mutex;                    /* The SNTP Client mutex for protecting access                     */ 
        UCHAR                           nx_sntp_client_thread_stack[NX_SNTP_CLIENT_THREAD_STACK_SIZE];  /* Stack size for SNTP client thread        */
        NXD_ADDRESS                     nx_sntp_server_ip_address;               /* Client's current time server IP address.  */
        NX_UDP_SOCKET                   nx_sntp_client_udp_socket;               /* Client UDP socket for sending and receiving time updates.  */
        UINT                            nx_sntp_client_first_update_pending;     /* First SNTP update not yet received with current server   */
        UINT                            nx_sntp_client_time_start_wait;          /* Initial time at start of receiving broadcast SNTP updates */
        UINT                            nx_sntp_client_sent_initial_unicast;     /* Status on initial unicast transmittal for clients in broadcast mode */
        UINT                            nx_sntp_client_invalid_time_updates;     /* Number of invalid SNTP messages received */
        UINT                            nx_sntp_valid_server_status;             /* Server status; if receiving valid updates, status is TRUE */
        UINT                            nx_sntp_client_protocol_mode;            /* Mode of operation, either unicast or broadcast */
        UINT                            nx_sntp_client_broadcast_initialized;    /* Client task is ready to receive broadcast time data.  */
        NXD_ADDRESS                     nx_sntp_broadcast_time_server;           /* Client's broadcast SNTP server */
        NXD_ADDRESS                     nx_sntp_multicast_server_address;        /* IP address Client should listen on to receive broadcasts from a multicast server.  */
        UINT                            nx_sntp_client_unicast_initialized;      /* Client task is ready to receive unicast time data.  */
        NXD_ADDRESS                     nx_sntp_unicast_time_server;             /* Client's unicast time server.  */
        ULONG                           nx_sntp_client_unicast_poll_interval;    /* Unicast interval at which client is polling the time server.  */
        UINT                            nx_sntp_client_backoff_count;            /* Count of times the back off rate has been applied to the poll interval */
        TX_TIMER                        nx_sntp_update_timer;                    /* SNTP update timer; expires when no data received for specified time lapse.  */
        ULONG                           nx_sntp_update_time_remaining;           /* Time (in seconds) remaining that the Client task can continue running without receiving a valid update.  */
        LONG                            nx_sntp_client_roundtrip_time_msec;      /* Round trip time (msec) for a packet to travel to server and back to client. Does not include server processing time.  */
        ULONG                           nx_sntp_client_local_ntp_time_elapsed;   /* Seconds elapsed since local time is updated last time. */
        NX_SNTP_TIME_MESSAGE            nx_sntp_current_server_time_message;     /* Time update which the Client has just received from its server.  */
        NX_SNTP_TIME_MESSAGE            nx_sntp_current_time_message_request;    /* Client request to send to its time server.  */
        NX_SNTP_TIME_MESSAGE            nx_sntp_previous_server_time_message;    /* Previous valid time update received from the Client time server.  */
        NX_SNTP_TIME                    nx_sntp_client_local_ntp_time;           /* Client's notion of local time.  */
        NX_SNTP_TIME                    nx_sntp_server_update_time;              /* Time (based on client local time) when the server update was received in response to the current request.  */
        UINT                            (*apply_custom_sanity_checks)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, NX_SNTP_TIME_MESSAGE *client_time_msg_ptr, NX_SNTP_TIME_MESSAGE *server_time_msg_ptr);
                                                                                 /* Pointer to callback service for  performing additional sanity checks on received time data.  */
        UINT                            (*leap_second_handler)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, UINT indicator);   
                                                                                 /* Pointer to callback service for handling an impending leap second.  */
        UINT                            (*kiss_of_death_handler)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, UINT code);
                                                                                 /* Pointer to callback service for handling kiss of death packets received from server.  */   
        VOID                            (*random_number_generator)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, ULONG *rand);
                                                                                 /* Pointer to callback service for random number generator.  */

        VOID                            (*nx_sntp_client_time_update_notify)(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time);

    } NX_SNTP_CLIENT;


#ifndef     NX_SNTP_SOURCE_CODE     

/* Define the system API mappings based on the error checking selected by the user.   */


/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */


#ifdef NX_SNTP_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define   nx_sntp_client_create                             _nx_sntp_client_create
#define   nx_sntp_client_delete                             _nx_sntp_client_delete
#define   nx_sntp_client_get_local_time                     _nx_sntp_client_get_local_time
#define   nx_sntp_client_get_local_time_extended            _nx_sntp_client_get_local_time_extended
#define   nxd_sntp_client_initialize_broadcast              _nxd_sntp_client_initialize_broadcast
#define   nxd_sntp_client_initialize_unicast                _nxd_sntp_client_initialize_unicast
#define   nx_sntp_client_initialize_broadcast               _nx_sntp_client_initialize_broadcast
#define   nx_sntp_client_initialize_unicast                 _nx_sntp_client_initialize_unicast
#define   nx_sntp_client_receiving_updates                  _nx_sntp_client_receiving_updates
#define   nx_sntp_client_run_broadcast                      _nx_sntp_client_run_broadcast
#define   nx_sntp_client_run_unicast                        _nx_sntp_client_run_unicast
#define   nx_sntp_client_set_local_time                     _nx_sntp_client_set_local_time
#define   nx_sntp_client_stop                               _nx_sntp_client_stop
#define   nx_sntp_client_utility_msecs_to_fraction          _nx_sntp_client_utility_msecs_to_fraction
#define   nx_sntp_client_utility_usecs_to_fraction          _nx_sntp_client_utility_usecs_to_fraction
#define   nx_sntp_client_utility_fraction_to_usecs          _nx_sntp_client_utility_fraction_to_usecs
#define   nx_sntp_client_utility_display_date_time          _nx_sntp_client_utility_display_date_time
#define   nx_sntp_client_request_unicast_time               _nx_sntp_client_request_unicast_time
#define   nx_sntp_client_set_time_update_notify             _nx_sntp_client_set_time_update_notify

#else

/* Services with error checking.  */

#define   nx_sntp_client_create                             _nxe_sntp_client_create
#define   nx_sntp_client_delete                             _nxe_sntp_client_delete
#define   nx_sntp_client_get_local_time                     _nxe_sntp_client_get_local_time
#define   nx_sntp_client_get_local_time_extended            _nxe_sntp_client_get_local_time_extended
#define   nxd_sntp_client_initialize_broadcast              _nxde_sntp_client_initialize_broadcast
#define   nxd_sntp_client_initialize_unicast                _nxde_sntp_client_initialize_unicast
#define   nx_sntp_client_initialize_broadcast               _nxe_sntp_client_initialize_broadcast
#define   nx_sntp_client_initialize_unicast                 _nxe_sntp_client_initialize_unicast
#define   nx_sntp_client_receiving_updates                  _nxe_sntp_client_receiving_updates
#define   nx_sntp_client_run_broadcast                      _nxe_sntp_client_run_broadcast
#define   nx_sntp_client_run_unicast                        _nxe_sntp_client_run_unicast
#define   nx_sntp_client_set_local_time                     _nxe_sntp_client_set_local_time
#define   nx_sntp_client_stop                               _nxe_sntp_client_stop
#define   nx_sntp_client_utility_msecs_to_fraction          _nxe_sntp_client_utility_msecs_to_fraction
#define   nx_sntp_client_utility_usecs_to_fraction          _nxe_sntp_client_utility_usecs_to_fraction
#define   nx_sntp_client_utility_fraction_to_usecs          _nxe_sntp_client_utility_fraction_to_usecs
#define   nx_sntp_client_utility_display_date_time          _nxe_sntp_client_utility_display_date_time
#define   nx_sntp_client_request_unicast_time               _nxe_sntp_client_request_unicast_time
#define   nx_sntp_client_set_time_update_notify             _nxe_sntp_client_set_time_update_notify

#endif /* NX_SNTP_DISABLE_ERROR_CHECKING */


/* Define the prototypes accessible to the application software.  */

UINT   nx_sntp_client_create(NX_SNTP_CLIENT *client_ptr, NX_IP *ip_ptr, UINT iface_index, NX_PACKET_POOL *packet_pool_ptr,   
                             UINT (*leap_second_handler)(NX_SNTP_CLIENT *client_ptr, UINT indicator), 
                             UINT (*kiss_of_death_handler)(NX_SNTP_CLIENT *client_ptr, UINT code),
                             VOID (random_number_generator)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, ULONG *rand));
UINT    nx_sntp_client_delete (NX_SNTP_CLIENT *client_ptr);
UINT    nx_sntp_client_get_local_time(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer); 
UINT    nx_sntp_client_get_local_time_extended(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer, UINT buffer_size);
UINT    nxd_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *multicast_server_address, NXD_ADDRESS *broadcast_time_server);
UINT    nx_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr,  ULONG multicast_server_address, ULONG broadcast_time_server);
UINT    nxd_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *unicast_time_server);
UINT    nx_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, ULONG unicast_time_server);
UINT    nx_sntp_client_receiving_updates(NX_SNTP_CLIENT *client_ptr, UINT *server_status);
UINT    nx_sntp_client_run_broadcast(NX_SNTP_CLIENT *client_ptr);
UINT    nx_sntp_client_run_unicast(NX_SNTP_CLIENT *client_ptr);
UINT    nx_sntp_client_set_local_time(NX_SNTP_CLIENT *client_ptr, ULONG seconds, ULONG fraction);
UINT    nx_sntp_client_stop(NX_SNTP_CLIENT *client_ptr);
UINT    nx_sntp_client_utility_msecs_to_fraction(ULONG msecs, ULONG *fraction);
UINT    nx_sntp_client_utility_usecs_to_fraction(ULONG usecs, ULONG *fraction);
UINT    nx_sntp_client_utility_fraction_to_usecs(ULONG fraction, ULONG *usecs); 
UINT    nx_sntp_client_utility_display_date_time(NX_SNTP_CLIENT *client_ptr, CHAR *buffer, UINT length);
UINT    nx_sntp_client_request_unicast_time(NX_SNTP_CLIENT *client_ptr, UINT wait_option);       
UINT    nx_sntp_client_set_time_update_notify(NX_SNTP_CLIENT *client_ptr, VOID (time_update_cb)(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time));

#else  /*  NX_SNTP_SOURCE_CODE */


/* SNTP source code is being compiled, do not perform any API mapping.  */

UINT   _nx_sntp_client_create(NX_SNTP_CLIENT *client_ptr, NX_IP *ip_ptr, UINT iface_index, NX_PACKET_POOL *packet_pool_ptr,   
                            UINT (*leap_second_handler)(NX_SNTP_CLIENT *client_ptr, UINT indicator), 
                            UINT (*kiss_of_death_handler)(NX_SNTP_CLIENT *client_ptr, UINT code),
                            VOID (random_number_generator)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, ULONG *rand));
UINT   _nxe_sntp_client_create(NX_SNTP_CLIENT *client_ptr, NX_IP *ip_ptr, UINT iface_index, NX_PACKET_POOL *packet_pool_ptr,   
                            UINT (*leap_second_handler)(NX_SNTP_CLIENT *client_ptr, UINT indicator), 
                            UINT (*kiss_of_death_handler)(NX_SNTP_CLIENT *client_ptr, UINT code),
                            VOID (random_number_generator)(struct NX_SNTP_CLIENT_STRUCT *client_ptr, ULONG *rand));
UINT    _nx_sntp_client_delete (NX_SNTP_CLIENT *client_ptr);
UINT    _nxe_sntp_client_delete (NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_get_local_time(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer); 
UINT    _nxe_sntp_client_get_local_time(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer); 
UINT    _nx_sntp_client_get_local_time_extended(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer, UINT buffer_size); 
UINT    _nxe_sntp_client_get_local_time_extended(NX_SNTP_CLIENT *client_ptr, ULONG *seconds, ULONG *fraction, CHAR *buffer, UINT buffer_size);
UINT    _nxde_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *multicast_server_address, NXD_ADDRESS *broadcast_time_server);
UINT    _nxd_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *multicast_server_address, NXD_ADDRESS *broadcast_time_server);
UINT    _nx_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, ULONG multicast_server_address, ULONG broadcast_time_server);
UINT    _nxe_sntp_client_initialize_broadcast(NX_SNTP_CLIENT *client_ptr, ULONG multicast_server_address, ULONG broadcast_time_server);
UINT    _nxde_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *unicast_time_server);
UINT    _nxd_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, NXD_ADDRESS *unicast_time_server);
UINT    _nx_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, ULONG unicast_time_server);
UINT    _nxe_sntp_client_initialize_unicast(NX_SNTP_CLIENT *client_ptr, ULONG unicast_time_server);
UINT    _nx_sntp_client_receiving_updates(NX_SNTP_CLIENT *client_ptr, UINT *server_status);
UINT    _nxe_sntp_client_receiving_updates(NX_SNTP_CLIENT *client_ptr, UINT *server_status);
UINT    _nx_sntp_client_run_broadcast(NX_SNTP_CLIENT *client_ptr);
UINT    _nxe_sntp_client_run_broadcast(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_run_unicast(NX_SNTP_CLIENT *client_ptr);
UINT    _nxe_sntp_client_run_unicast(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_set_local_time(NX_SNTP_CLIENT *client_ptr, ULONG seconds, ULONG fraction); 
UINT    _nxe_sntp_client_set_local_time(NX_SNTP_CLIENT *client_ptr, ULONG seconds, ULONG fraction); 
UINT    _nx_sntp_client_stop(NX_SNTP_CLIENT *client_ptr);
UINT    _nxe_sntp_client_stop(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_utility_msecs_to_fraction(ULONG msecs, ULONG *fraction);
UINT    _nxe_sntp_client_utility_msecs_to_fraction(ULONG msecs, ULONG *fraction);
UINT    _nx_sntp_client_utility_usecs_to_fraction(ULONG usecs, ULONG *fraction);
UINT    _nxe_sntp_client_utility_usecs_to_fraction(ULONG usecs, ULONG *fraction);
UINT    _nx_sntp_client_utility_fraction_to_usecs(ULONG fraction, ULONG *usecs); 
UINT    _nxe_sntp_client_utility_fraction_to_usecs(ULONG fraction, ULONG *usecs); 
UINT    _nx_sntp_client_utility_display_date_time(NX_SNTP_CLIENT *client_ptr, CHAR *buffer, UINT length);
UINT    _nxe_sntp_client_utility_display_date_time(NX_SNTP_CLIENT *client_ptr, CHAR *buffer, UINT length);
UINT    _nxe_sntp_client_utility_display_NTP_time(NX_SNTP_CLIENT *client_ptr, CHAR *buffer);
UINT    _nx_sntp_client_request_unicast_time(NX_SNTP_CLIENT *client_ptr, UINT wait_option);
UINT    _nxe_sntp_client_request_unicast_time(NX_SNTP_CLIENT *client_ptr, UINT wait_option);
UINT    _nx_sntp_client_set_time_update_notify(NX_SNTP_CLIENT *client_ptr, VOID (time_update_cb)(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time));
UINT    _nxe_sntp_client_set_time_update_notify(NX_SNTP_CLIENT *client_ptr, VOID (time_update_cb)(NX_SNTP_TIME_MESSAGE *time_update_ptr, NX_SNTP_TIME *local_time));

/* Define internal SNTP Client functions.  */
                     
UINT    _nx_sntp_client_apply_sanity_checks(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_calculate_roundtrip(LONG *roundtrip_time);
UINT    _nx_sntp_client_check_server_clock_dispersion(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_create_time_request_packet(NX_SNTP_CLIENT *client_ptr, NX_PACKET *packet_ptr, NX_SNTP_TIME_MESSAGE *time_message_ptr);
UINT    _nx_sntp_client_duplicate_update_check(NX_SNTP_TIME_MESSAGE *timeA_msg_ptr, NX_SNTP_TIME_MESSAGE *timeB_msg_ptr, UINT *is_a_dupe);
UINT    _nx_sntp_client_extract_time_message_from_packet(NX_SNTP_CLIENT *client_ptr, NX_PACKET *packet_ptr);
VOID    _nx_sntp_client_process(NX_SNTP_CLIENT *client_ptr);
VOID    _nx_sntp_client_process_broadcast(NX_SNTP_CLIENT *client_ptr);
VOID    _nx_sntp_client_process_unicast(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_process_time_data(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_process_update_packet(NX_SNTP_CLIENT *client_ptr); 
VOID    _nx_sntp_client_receive_notify(NX_UDP_SOCKET *socket_ptr);
UINT    _nx_sntp_client_receive_time_update(NX_SNTP_CLIENT *client_ptr, ULONG timeout);
UINT    _nx_sntp_client_reset_current_time_message(NX_SNTP_CLIENT *client_ptr);
UINT    _nx_sntp_client_send_unicast_request(NX_SNTP_CLIENT *client_ptr); 
VOID    _nx_sntp_client_thread_entry(ULONG sntp_instance);
VOID    _nx_sntp_client_update_timeout_entry(ULONG info);
UINT    _nx_sntp_client_utility_add_msecs_to_ntp_time(NX_SNTP_TIME *timeA_ptr, LONG msecs_to_add);
UINT    _nx_sntp_client_utility_convert_fraction_to_msecs(ULONG *milliseconds, NX_SNTP_TIME *time_ptr);
UINT    _nx_sntp_client_utility_convert_seconds_to_date(NX_SNTP_TIME *current_NTP_time_ptr, UINT current_year, NX_SNTP_DATE_TIME *current_date_time_ptr);
UINT    _nx_sntp_client_utility_convert_refID_KOD_code(UCHAR *reference_id, UINT *code_id);
UINT    _nx_sntp_client_utility_get_msec_diff(NX_SNTP_TIME *timeA_ptr, NX_SNTP_TIME *timeB_ptr, ULONG *total_difference_msecs, UINT *pos_diff);    
UINT    _nx_sntp_client_utility_addition_overflow_check(ULONG temp1, ULONG temp2);
UINT    _nx_sntp_client_utility_convert_time_to_UCHAR(NX_SNTP_TIME *time, NX_SNTP_TIME_MESSAGE *time_message_ptr, UINT which_stamp);
UINT    _nx_sntp_client_utility_is_zero_data(UCHAR *data, UINT size);



#endif   /*  NX_SNTP_SOURCE_CODE */

/* If a C++ compiler is being used....*/
#ifdef   __cplusplus
}
#endif


#endif /* NXD_SNTP_CLIENT_H  */

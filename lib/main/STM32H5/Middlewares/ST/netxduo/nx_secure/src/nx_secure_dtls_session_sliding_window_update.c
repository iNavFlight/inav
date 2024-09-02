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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"


#ifdef NX_SECURE_ENABLE_DTLS
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_session_sliding_window_update       PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates the DTLS sliding window used to validate      */
/*    incoming DTLS application records. If the sequence number of a      */
/*    received DTLS record is less than the "right" side of the window but*/
/*    greater than the "left" side and not a repeat of another record, the*/
/*    record is accepted (RFC 6347 Section 4.1.2.6).                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          Pointer to DTLS control block */
/*    sequence_number                       New "right" side of window    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  01-31-2022     Timothy Stapko           Initial Version 6.1.10        */
/*                                                                        */
/**************************************************************************/

UINT _nx_secure_dtls_session_sliding_window_update(NX_SECURE_DTLS_SESSION *dtls_session, ULONG *sequence_number)
{
ULONG delta;
ULONG mask;
NX_SECURE_TLS_SESSION *tls_session;

    /* Extract TLS session for sequence numbers from DTLS session. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* The incoming sequence number is assumed to be OK, so update window accordingly. */


    /* Double check new sequence number. */
    if (sequence_number[0] == tls_session -> nx_secure_tls_remote_sequence_number[0] &&
        sequence_number[1] == tls_session -> nx_secure_tls_remote_sequence_number[1])
    {
        /* Equal to our current - this is a repeat. */
        return(NX_SECURE_TLS_OUT_OF_ORDER_MESSAGE);
    }

    /* See if the incoming number is smaller than the last one we saw. */
    if (sequence_number[0] < tls_session -> nx_secure_tls_remote_sequence_number[0] ||
        (sequence_number[0] == tls_session -> nx_secure_tls_remote_sequence_number[0] &&
        sequence_number[1] < tls_session -> nx_secure_tls_remote_sequence_number[1]))
    {
        delta = 0;
        if(sequence_number[0] == tls_session -> nx_secure_tls_remote_sequence_number[0])
        {
            /* Upper halves match so just subtract. */
            delta = tls_session -> nx_secure_tls_remote_sequence_number[1] - sequence_number[1];
        }
        else
        {
            /* Top halves don't match, adjust before subtract. */
            delta = (0xFFFFFFFFul - sequence_number[1]) + tls_session -> nx_secure_tls_remote_sequence_number[1];
        }

        /* Incoming sequence number is smaller than last seen. Update the bitfield without shifting. */
        mask = 0x1ul << delta; 
        dtls_session -> nx_secure_dtls_sliding_window = dtls_session -> nx_secure_dtls_sliding_window | mask;
    }
    else
    {
        /* Compare sequence numbers. At this point, the incoming number is greater than the last seen
        so we can update the window. */
        if(sequence_number[0] > tls_session -> nx_secure_tls_remote_sequence_number[0])
        {
            /* Upper halves don't match so adjust delta accordingly. */
            delta = (0xFFFFFFFFul - tls_session -> nx_secure_tls_remote_sequence_number[1]) + sequence_number[1];
        }
        else
        {
            /* Top halves match, just subtract. */
            delta = sequence_number[1] - tls_session -> nx_secure_tls_remote_sequence_number[1];
        }

        /* Now we can update the window. (delta represents a *bit* position in the window). */
        if(delta > (sizeof(dtls_session -> nx_secure_dtls_sliding_window) * 8))
        {
            /* Delta is larger than window size - just clear it out. */
            dtls_session -> nx_secure_dtls_sliding_window = 1;
        }
        else
        {
            /* Delta is within the window size, just left-shift to new position. */            
            dtls_session -> nx_secure_dtls_sliding_window <<= delta;
            dtls_session -> nx_secure_dtls_sliding_window |= 0x1;
        }

        /* Update the sequence number to reflect the window change. */
        tls_session -> nx_secure_tls_remote_sequence_number[1] = sequence_number[1];
        tls_session -> nx_secure_tls_remote_sequence_number[0] = sequence_number[0];
    }    


    return(NX_SUCCESS);
}
#endif
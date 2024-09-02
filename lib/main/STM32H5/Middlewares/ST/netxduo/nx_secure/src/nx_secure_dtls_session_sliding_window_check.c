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
/*    _nx_secure_dtls_process_sliding_window_check        PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks a received record against the DTLS sliding     */
/*    window used to validate incoming DTLS records. If the sequence      */
/*    number of a received DTLS record is less than the "right" side of   */
/*    the window but greater than the "left" side and not a repeat of     */
/*    another record, the record is accepted (RFC 6347 Section 4.1.2.6).  */
/*    NOTE: sequence numbers must be in target endian format before       */
/*          calling this routine!                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          Pointer to DTLS control block */
/*    sequence_number                       Incoming sequence number      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                True/False - record is OK     */
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

UINT _nx_secure_dtls_session_sliding_window_check(NX_SECURE_DTLS_SESSION *dtls_session, ULONG *sequence_number)
{
ULONG window;
ULONG delta;
ULONG mask;
NX_SECURE_TLS_SESSION *tls_session;

    /* Extract TLS session for sequence numbers and window from DTLS session. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;
    window = dtls_session -> nx_secure_dtls_sliding_window;

    /* See if the incoming number is inside the window. */
    if (sequence_number[0] == tls_session -> nx_secure_tls_remote_sequence_number[0] &&
        sequence_number[1] == tls_session -> nx_secure_tls_remote_sequence_number[1])
    {
        /* Equal to our current - this is a repeat. */
        return(NX_FALSE);
    }

    /* See if the incoming number is larger than the last one we saw. */
    if (sequence_number[0] > tls_session -> nx_secure_tls_remote_sequence_number[0] ||
        (sequence_number[0] == tls_session -> nx_secure_tls_remote_sequence_number[0] &&
        sequence_number[1] > tls_session -> nx_secure_tls_remote_sequence_number[1]))
    {
        /* Incoming sequence number is bigger than last seen. This is OK, new sequence number. 
           Outside window to the "right" side. */
        return(NX_TRUE);
    }

    /* Compare sequence numbers. At this point, the incoming number is less than the last seen
       but we need to know if it fits into the window. */
    delta = 0;
    if(sequence_number[0] + 1 == tls_session -> nx_secure_tls_remote_sequence_number[0])
    {
        /* Incoming number is less than last seen, but upper halves don't match so adjust. */
        delta = (0xFFFFFFFFul - sequence_number[1]) + tls_session -> nx_secure_tls_remote_sequence_number[1];
    }
    else if(sequence_number[0] == tls_session -> nx_secure_tls_remote_sequence_number[0])
    {
        /* Top halves match, just subtract. */
        delta = tls_session -> nx_secure_tls_remote_sequence_number[1] - sequence_number[1];
    }
    else
    {
        /* Incoming number is significantly smaller than expected (delta of top half is > 1)
           so really outside the window to the left. */
        return(NX_FALSE);
    }


    /* Now we can check the delta against the window. (delta represents a *bit* position in the window). */
    if(delta > (sizeof(window) * 8))
    {
        /* Delta is larger than window size - record fell off the left side. */
        return(NX_FALSE);
    }

    /* Sequence number is inside the sliding window, check the bitfield. */
    mask = 0x1ul << delta; 
    if(window & mask)
    {
        /* Saw this one already! */
        return(NX_FALSE);
    }

    /* If we get here, the record was in the window but not yet seen. */
    return(NX_TRUE);
}
#endif
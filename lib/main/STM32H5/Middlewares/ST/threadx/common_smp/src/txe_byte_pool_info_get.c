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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Byte Memory                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_byte_pool.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txe_byte_pool_info_get                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the byte pool information get    */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                          Pointer to byte pool control block*/
/*    name                              Destination for the pool name     */
/*    available_bytes                   Number of free bytes in byte pool */
/*    fragments                         Number of fragments in byte pool  */
/*    first_suspended                   Destination for pointer of first  */
/*                                        thread suspended on byte pool   */
/*    suspended_count                   Destination for suspended count   */
/*    next_pool                         Destination for pointer to next   */
/*                                        byte pool on the created list   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    TX_POOL_ERROR                     Invalid byte pool pointer         */
/*    status                            Completion status                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_byte_pool_info_get            Actual byte pool info get service */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _txe_byte_pool_info_get(TX_BYTE_POOL *pool_ptr, CHAR **name, ULONG *available_bytes,
                    ULONG *fragments, TX_THREAD **first_suspended,
                    ULONG *suspended_count, TX_BYTE_POOL **next_pool)
{

UINT    status;


    /* Check for an invalid byte pool pointer.  */
    if (pool_ptr == TX_NULL)
    {

        /* Block pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }

    /* Now check for invalid pool ID.  */
    else if (pool_ptr -> tx_byte_pool_id != TX_BYTE_POOL_ID)
    {

        /* Block pool pointer is invalid, return appropriate error code.  */
        status =  TX_POOL_ERROR;
    }
    else
    {

        /* Otherwise, call the actual byte pool information get service.  */
        status =  _tx_byte_pool_info_get(pool_ptr, name, available_bytes,
                            fragments, first_suspended, suspended_count, next_pool);
    }

    /* Return completion status.  */
    return(status);
}


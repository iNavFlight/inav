@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/


@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */
@/** ThreadX Component                                                     */
@/**                                                                       */
@/**   Thread                                                              */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/


    .text 32
    .align 4
    .syntax unified

@/**************************************************************************/
@/*                                                                        */
@/*  FUNCTION                                               RELEASE        */
@/*                                                                        */
@/*    _tx_thread_interrupt_control                      Cortex-M0/GNU     */
@/*                                                           6.1          */
@/*  AUTHOR                                                                */
@/*                                                                        */
@/*    William E. Lamie, Microsoft Corporation                             */
@/*                                                                        */
@/*  DESCRIPTION                                                           */
@/*                                                                        */
@/*    This function is responsible for changing the interrupt lockout     */
@/*    posture of the system.                                              */
@/*                                                                        */
@/*  INPUT                                                                 */
@/*                                                                        */
@/*    new_posture                           New interrupt lockout posture */
@/*                                                                        */
@/*  OUTPUT                                                                */
@/*                                                                        */
@/*    old_posture                           Old interrupt lockout posture */
@/*                                                                        */
@/*  CALLS                                                                 */
@/*                                                                        */
@/*    None                                                                */
@/*                                                                        */
@/*  CALLED BY                                                             */
@/*                                                                        */
@/*    Application Code                                                    */
@/*                                                                        */
@/*  RELEASE HISTORY                                                       */
@/*                                                                        */
@/*    DATE              NAME                      DESCRIPTION             */
@/*                                                                        */
@/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
@/*  09-30-2020     William E. Lamie         Modified comment(s),          */
@/*                                            resulting in version 6.1    */        
@/*                                                                        */
@/**************************************************************************/
@/* UINT   _tx_thread_interrupt_control(UINT new_posture)
{  */
    .global  _tx_thread_interrupt_control
    .thumb_func
_tx_thread_interrupt_control:

@/* Pickup current interrupt lockout posture.  */

    MRS     r1, PRIMASK                             @ Pickup current interrupt lockout


@/* Apply the new interrupt posture.  */

    MSR     PRIMASK, r0                             @ Apply the new interrupt lockout
    MOV     r0, r1                                  @ Transfer old to return register
    BX      lr                                      @ Return to caller

@/* } */

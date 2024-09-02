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

#ifndef NX_AZURE_IOT_CERT_H
#define NX_AZURE_IOT_CERT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


/* Users can use this root certificate as sample, and also can build the root certificate by themself.  */
extern const unsigned char _nx_azure_iot_root_cert[];
extern const unsigned int _nx_azure_iot_root_cert_size;
extern const unsigned char _nx_azure_iot_root_cert_2[];
extern const unsigned int _nx_azure_iot_root_cert_size_2;
extern const unsigned char _nx_azure_iot_root_cert_3[];
extern const unsigned int _nx_azure_iot_root_cert_size_3;


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus
}
#endif

#endif /* NX_AZURE_IOT_CERT_H */

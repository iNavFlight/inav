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
/**   ThreadX Component                                                   */
/**                                                                       */
/**   POSIX Compliancy Wrapper (POSIX)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  EKP DEFINITIONS                                        RELEASE        */
/*                                                                        */
/*    fcntl.h                                             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the constants, structures, etc.needed to          */
/*    implement the Evacuation Kit for POSIX Users (POSIX)                */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-02-2021     William E. Lamie         Initial Version 6.1.7         */
/*                                                                        */
/**************************************************************************/

#ifndef _FCNTL_H
#define _FCNTL_H

#define O_ACCMODE	0x0003
#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_APPEND	0x0008
#define O_SYNC		0x0010
#define O_NONBLOCK	0x0080
#define O_CREAT     0x0100	
#define O_TRUNC		0x0200	
#define O_EXCL		0x0400	
#define O_NOCTTY	0x0800	
#define FASYNC		0x1000	
#define O_LARGEFILE	0x2000	
#define O_DIRECT	0x8000	
#define O_DIRECTORY	0x10000	
#define O_NOFOLLOW	0x20000	

#define O_NDELAY	O_NONBLOCK

#define F_DUPFD		0	
#define F_GETFD		1	
#define F_SETFD		2	
#define F_GETFL		3	
#define F_SETFL		4	
#define F_GETLK		14
#define F_SETLK		6
#define F_SETLKW	7

#define F_SETOWN	24	
#define F_GETOWN	23	
#define F_SETSIG	10	
#define F_GETSIG	11	

#define FD_CLOEXEC	1	

# define POSIX_FADV_NORMAL	0 
# define POSIX_FADV_RANDOM	1 
# define POSIX_FADV_SEQUENTIAL	2 
# define POSIX_FADV_WILLNEED	3 
# define POSIX_FADV_DONTNEED	4 
# define POSIX_FADV_NOREUSE	5

/* no flock structure for Threadx at this time */

#endif

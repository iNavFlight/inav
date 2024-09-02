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
/*    errno.h                                             PORTABLE C      */
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

#ifndef _ERRNO_H
#define _ERRNO_H



#ifndef TX_POSIX_SOURCE
#define errno posix_errno
#endif

/* the POSIX standard does not impose particular values for errno.h        */
/* error codes between 200 and 1000 are not used by the Threadx wrapper    */
/* but supplied for completeness.                                          */




#define E2BIG           200

#define EACCES          13

#define EADDRINUSE      201

#define EADDRNOTAVAIL   202

#define EAFNOSUPPORT    203

#define EAGAIN          11

#define EALREADY        204

#define EBADF           9

#define EBADMSG         205

#define EBUSY           9999

#define ECANCELED       206

#define ECHILD          207

#define ECONNABORTED    208

#define ECONNREFUSED    209

#define ECONNRESET      210

#define EDEADLK         3333

#define EDESTADDRREQ    211

#define EDOM            212

#define EDQUOT          213

#define EEXIST          17      

#define EFAULT          214

#define EFBIG           215

#define EHOSTUNREACH    216

#define EIDRM           217

#define EILSEQ          218

#define EINPROGRESS     219

#define EINTR           4

#define EINVAL          22

#define EIO             220

#define EISCONN         221

#define EISDIR          222

#define ELOOP           223

#define EMFILE          224

#define EMLINK          225

#define EMSGSIZE        36

#define EMULTIHOP       226

#define ENAMETOOLONG    26

#define ENETDOWN        227

#define ENETRESET       228

#define ENETUNREACH     229

#define ENFILE          230

#define ENOBUFS         231

#define ENODATA         232

#define ENODEV          233

#define ENOENT          2

#define ENOEXEC         234

#define ENOLCK          235

#define ENOLINK         236

#define ENOMEM          4444

#define ENOMSG          237

#define ENOPROTOOPT     238

#define ENOSPC          28

#define ENOSR           239

#define ENOSTR          240

#define ENOSYS          71

#define ENOTCONN        241

#define ENOTDIR         242

#define ENOTEMPTY       243

#define ENOTSOCK        244

#define ENOTSUP         126

#define ENOTTY          245

#define ENXIO           246

#define EOPNOTSUPP      247

#define EOVERFLOW       248

#define EPERM           2222

#define EPIPE           249

#define EPROTO          250

#define EPROTONOSUPPORT 251

#define EPROTOTYPE      252

#define ERANGE          253

#define EROFS           254

#define ESPIPE          255

#define ESRCH           3

#define ESTALE          256

#define ETIME           257

#define ETIMEDOUT       5555

#define ETXTBSY         258

#define EWOULDBLOCK     259

#define EXDEV           260

#endif

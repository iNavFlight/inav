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
/**    X.509 Digital Certificates                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


#include "nx_secure_x509.h"


/* OIDs for X509 items
   OID encoding scheme:
   - First byte (ISO prefix) is equal to 40X + Y for X.Y,
     so 1.2 becomes 40 + 2 = 42 = 0x2A.
   - Values longer than 7 bits are broken into 7-bit segments.
     The lowest byte has a top bit of 0 and all the rest are
     padded and have a top bit of 1. So 113549 (RSA) becomes
     encoded in 3 bytes (113549 requires 20 bits in 4-bit nibbles
     and 20 / 7 bits = 3 bytes) Therefore, 113549 becomes an
     encoded value of 0x86, 0xF7, 0x0D.
   - See Davies, "Implementing SSL/TLS" ch. 5.
 */
/* OID values that may be of use in the future.
   static const UCHAR NX_SECURE_X509_OID_ISO_PREFIX[]   = { 0x2A };                                 // 1.2
   static const UCHAR NX_SECURE_X509_OID_USA[]          = { 0x2A, 0x86, 0x48 };                     // ISO.840
   static const UCHAR NX_SECURE_X509_OID_RSA_CORP[]     = { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D };   // ISO.USA.113549
 */
static const UCHAR NX_SECURE_X509_OID_RSA[]          = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01};     /* ISO.USA.RSA.1.1.1 */
static const UCHAR NX_SECURE_X509_OID_RSA_MD5[]      = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x04};     /* ISO.USA.RSA.1.1.4 */
static const UCHAR NX_SECURE_X509_OID_RSA_SHA1[]     = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x05};     /* ISO.USA.RSA.1.1.5 */
static const UCHAR NX_SECURE_X509_OID_RSA_SHA256[]   = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B};     /* ISO.USA.RSA.1.1.11 */
static const UCHAR NX_SECURE_X509_OID_RSA_SHA384[]   = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0C};     /* ISO.USA.RSA.1.1.12 */
static const UCHAR NX_SECURE_X509_OID_RSA_SHA512[]   = {0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0D};     /* ISO.USA.RSA.1.1.13 */

/* static const UCHAR NX_SECURE_X509_OID_NIST_SHA256[]  = {0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01};  */   /* 2.16.840.1.101.3.4.2.1 NIST algorithm SHA256. */

/* RFC 3729 OIDs. */
static const UCHAR NX_SECURE_X509_OID_DH[]           = {0x2A, 0x86, 0x48, 0xCE, 0x3E, 0x02, 0x01};   /* ISO.USA.10046.2.1 - ANSI X9.42 DH public number. */
static const UCHAR NX_SECURE_X509_OID_DSS_SHA1[]     = {0x2A, 0x86, 0x48, 0xCE, 0x38, 0x04, 0x03};   /* ISO.USA.10040.4.3 - ANSI X9-57 x9Algorithm DSA-SHA1. */
/*static const UCHAR NX_SECURE_X509_OID_ECC_SHA1[]     = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01};  */  /* ISO.USA.10045.4.1 - ANSI X9-62 ECDSA with SHA1. */

/*  OIDs for X509 distinguished names.
    ASN.1 prefix (ISO-ITU.DirectoryServices.AttributeType) :
      id-at ::= { joint-iso-ccitt(2) ds(5) 4 }
      DER encoding: 0x55, 0x04
*/
static const UCHAR NX_SECURE_X509_OID_COMMON_NAME[] =   {0x55, 0x04, 0x03}; /* ISO-ITU.DirectoryServices.AttributeType.CommonName */
static const UCHAR NX_SECURE_X509_OID_SURNAME[] =       {0x55, 0x04, 0x04}; /* ISO-ITU.DirectoryServices.AttributeType.Surname */
static const UCHAR NX_SECURE_X509_OID_SERIAL_NUMBER[] = {0x55, 0x04, 0x05}; /* ISO-ITU.DirectoryServices.AttributeType.SerialNumber */
static const UCHAR NX_SECURE_X509_OID_COUNTRY_NAME[] =  {0x55, 0x04, 0x06}; /* ISO-ITU.DirectoryServices.AttributeType.CountryName */
static const UCHAR NX_SECURE_X509_OID_LOCALITY[] =      {0x55, 0x04, 0x07}; /* ISO-ITU.DirectoryServices.AttributeType.LocalityName */
static const UCHAR NX_SECURE_X509_OID_STATE[] =         {0x55, 0x04, 0x08}; /* ISO-ITU.DirectoryServices.AttributeType.StateName */
static const UCHAR NX_SECURE_X509_OID_ORGANIZATION[] =  {0x55, 0x04, 0x0A}; /* ISO-ITU.DirectoryServices.AttributeType.OrganizationName */
static const UCHAR NX_SECURE_X509_OID_ORG_UINT[] =      {0x55, 0x04, 0x0B}; /* ISO-ITU.DirectoryServices.AttributeType.OrganizationalUnitName */
static const UCHAR NX_SECURE_X509_OID_TITLE[] =         {0x55, 0x04, 0x0C}; /* ISO-ITU.DirectoryServices.AttributeType.Title */
static const UCHAR NX_SECURE_X509_OID_NAME[] =          {0x55, 0x04, 0x29}; /* ISO-ITU.DirectoryServices.AttributeType.Name */
static const UCHAR NX_SECURE_X509_OID_GIVEN_NAME[] =    {0x55, 0x04, 0x2A}; /* ISO-ITU.DirectoryServices.AttributeType.GivenName */
static const UCHAR NX_SECURE_X509_OID_INITIALS[] =      {0x55, 0x04, 0x2B}; /* ISO-ITU.DirectoryServices.AttributeType.Initials */
static const UCHAR NX_SECURE_X509_OID_GENERATION[] =    {0x55, 0x04, 0x2C}; /* ISO-ITU.DirectoryServices.AttributeType.GenerationQualifier */
static const UCHAR NX_SECURE_X509_OID_DN_QUALIFIER[] =  {0x55, 0x04, 0x2E}; /* ISO-ITU.DirectoryServices.AttributeType.DnQualifier */
static const UCHAR NX_SECURE_X509_OID_PSEUDONYM[] =     {0x55, 0x04, 0x41}; /* ISO-ITU.DirectoryServices.AttributeType.Pseudonym */

/* X.509 Certificate extensions OIDs from RFC 5280. */
/* id-ce   OBJECT IDENTIFIER ::=  { joint-iso-ccitt(2) ds(5) 29 } */
/* static const UCHAR NX_SECURE_X509_OID_EXTENSIONS_PREFIX[] =    {0x55, 0x1D }; */ /* 2.5.29 */
static const UCHAR NX_SECURE_X509_OID_DIRECTORY_ATTRIBUTES[] =   {0x55, 0x1D, 0x09};       /* id-ce.9  Directory attributes extension. */
static const UCHAR NX_SECURE_X509_OID_SUBJECT_KEY_ID[] =         {0x55, 0x1D, 0x0E};       /* id-ce.14 Subject key identifier extension. */
static const UCHAR NX_SECURE_X509_OID_KEY_USAGE[] =              {0x55, 0x1D, 0x0F};       /* id-ce.15 Key usage extension. */
static const UCHAR NX_SECURE_X509_OID_SUBJECT_ALT_NAME[] =       {0x55, 0x1D, 0x11};       /* id-ce.17 Subject alternative name. */
static const UCHAR NX_SECURE_X509_OID_ISSUER_ALT_NAME[] =        {0x55, 0x1D, 0x12};       /* id-ce.18 Issuer alternative name. */
static const UCHAR NX_SECURE_X509_OID_BASIC_CONSTRAINTS[] =      {0x55, 0x1D, 0x13};       /* id-ce.19 Basic constraints extension.*/
static const UCHAR NX_SECURE_X509_OID_NAME_CONSTRAINTS[] =       {0x55, 0x1D, 0x1E};       /* id-ce.30 Name constraints extension.*/
static const UCHAR NX_SECURE_X509_OID_CRL_DISTRIBUTION[] =       {0x55, 0x1D, 0x1F};       /* id-ce.31 CRL distribution points extension.*/
static const UCHAR NX_SECURE_X509_OID_CERTIFICATE_POLICIES[] =   {0x55, 0x1D, 0x20};       /* id-ce.32 Certificate policies extension.*/
static const UCHAR NX_SECURE_X509_OID_ANY_POLICY[] =             {0x55, 0x1D, 0x20, 0x00}; /* id-ce.32.0  anyPolicy identifier. */
static const UCHAR NX_SECURE_X509_OID_CERT_POLICY_MAPPINGS[] =   {0x55, 0x1D, 0x21};       /* id-ce.33 Certificate policy mapping extension.*/
static const UCHAR NX_SECURE_X509_OID_AUTHORITY_KEY_ID[] =       {0x55, 0x1D, 0x23};       /* id-ce.35 Authority key identifier extension.*/
static const UCHAR NX_SECURE_X509_OID_POLICY_CONSTRAINTS[] =     {0x55, 0x1D, 0x24};       /* id-ce.36 Policy constraints extension.*/
static const UCHAR NX_SECURE_X509_OID_EXTENDED_KEY_USAGE[] =     {0x55, 0x1D, 0x25};       /* id-ce.37 Extended key usage extension.*/
static const UCHAR NX_SECURE_X509_OID_ANY_EXTENDED_KEY_USAGE[] = {0x55, 0x1D, 0x25, 0x00}; /* id-ce.37.0 anyExtendedKeyUsage.*/
static const UCHAR NX_SECURE_X509_OID_FRESHEST_CRL[] =           {0x55, 0x1D, 0x2E};       /* id-ce.46 Freshest CRL distribution extension.*/
static const UCHAR NX_SECURE_X509_OID_INHIBIT_ANYPOLICY[] =      {0x55, 0x1D, 0x36};       /* id-ce.54 Inhibit anyPolicy extension.*/


/* X.509 Private Internet extensions OIDs from RFC 5280. */
/* id-pkix  OBJECT IDENTIFIER  ::=
               { iso(1) identified-organization(3) dod(6) internet(1)
                       security(5) mechanisms(5) pkix(7) }

   id-pe  OBJECT IDENTIFIER  ::=  { id-pkix 1 }
   id-qt  OBJECT IDENTIFIER  ::=  { id-pkix 2 }
*/
/* static const UCHAR NX_SECURE_X509_OID_PKIX_PREFIX[] =     {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07 }; */       /* 1.3.6.1.5.5.7 */
static const UCHAR NX_SECURE_X509_OID_PKIX_EXT_PREFIX[] = {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01};        /* id-pkix.1 (id-pe) */
static const UCHAR NX_SECURE_X509_OID_PKIX_AIA[] =        {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01};  /* id-pe.1  Authority Information Access PKIX extension. */
static const UCHAR NX_SECURE_X509_OID_PKIX_SIA[] =        {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x0B};  /* id-pe.11 Subject Information Access PKIX extension. */
static const UCHAR NX_SECURE_X509_OID_PKIX_QT[] =         {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x02};        /* id-pkix.2 Policy prefix (id-qt). */
static const UCHAR NX_SECURE_X509_OID_PKIX_QT_CPS[] =     {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x02, 0x01};  /* id-qt.1 CPS Policy. */
static const UCHAR NX_SECURE_X509_OID_PKIX_QT_UNOTICE[] = {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x02, 0x02};  /* id-qt.2 Unotice Policy prefix. */

/* Extended key usage extension OIDs. */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP[] =               {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03};        /* id-pkix.3 (id-kp) */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP_SERVER_AUTH[] =   {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x1};   /* id-pkix.3.1 Server authentication. */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP_CLIENT_AUTH[] =   {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x2};   /* id-pkix.3.2 Client authentication. */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP_CODE_SIGNING[] =  {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x3};   /* id-pkix.3.3 Code signing. */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP_EMAIL_PROTECT[] = {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x4};   /* id-pkix.3.4 Email protection. */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP_TIME_STAMPING[] = {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x8};   /* id-pkix.3.8 Time stamping. */
static const UCHAR NX_SECURE_X509_OID_PKIX_KP_OCSP_SIGNING[] =  {0x2B, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x9};   /* id-pkix.3.9 OCSP signing. */

/* Miscellaneous OIDs. */
static const UCHAR NX_SECURE_X509_OID_NETSCAPE_COMMENT[] = {0x60, 0x86, 0x48, 0x01, 0x86, 0xf8, 0x42, 0x01, 0x0d};

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
/* RFC 5480 OIDs. */
static const UCHAR NX_SECURE_X509_OID_EC[]               = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01};         /* ISO.USA.10045.2.1 - ANSI X9.62 EC public key. */
static const UCHAR NX_SECURE_X509_OID_SECP192R1[]        = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x01};   /* ISO.USA.10045.3.1.1 - Named curve: secp192r1. */
static const UCHAR NX_SECURE_X509_OID_SECP224R1[]        = {0x2B, 0x81, 0x04, 0x00, 0x21};                     /* ISO.Identified Organization.Certicom.curve.33 - Named curve: secp224r1. */
static const UCHAR NX_SECURE_X509_OID_SECP256R1[]        = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07};   /* ISO.USA.10045.3.1.7 - Named curve: secp256r1. */
static const UCHAR NX_SECURE_X509_OID_SECP384R1[]        = {0x2B, 0x81, 0x04, 0x00, 0x22};                     /* ISO.Identified Organization.Certicom.curve.34 - Named curve: secp384r1. */
static const UCHAR NX_SECURE_X509_OID_SECP521R1[]        = {0x2B, 0x81, 0x04, 0x00, 0x23};                     /* ISO.Identified Organization.Certicom.curve.35 - Named curve: secp521r1. */
static const UCHAR NX_SECURE_X509_OID_ECDSA_SHA1[]       = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x01};         /* ISO.USA.10045.4.1 - ecdsa-with-SHA1. */
static const UCHAR NX_SECURE_X509_OID_ECDSA_SHA224[]     = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x01};   /* ISO.USA.10045.4.3.1 - ecdsa-with-SHA224. */
static const UCHAR NX_SECURE_X509_OID_ECDSA_SHA256[]     = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02};   /* ISO.USA.10045.4.3.2 - ecdsa-with-SHA256. */
static const UCHAR NX_SECURE_X509_OID_ECDSA_SHA384[]     = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x03};   /* ISO.USA.10045.4.3.3 - ecdsa-with-SHA384. */
static const UCHAR NX_SECURE_X509_OID_ECDSA_SHA512[]     = {0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x04};   /* ISO.USA.10045.4.3.4 - ecdsa-with-SHA512. */
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/*  Lookup table for OID to type mapping. */
typedef struct NX_SECURE_X509_OID_MAP_STRUCT
{
    UINT         nx_secure_oid_map_type;

    const UCHAR *nx_secure_oid_map_oid;

    UINT         nx_secure_oid_map_oid_size;
} NX_SECURE_X509_OID_MAP;

NX_SECURE_X509_OID_MAP _nx_secure_x509_oid_map[] =
{
    {NX_SECURE_TLS_X509_TYPE_RSA,                    NX_SECURE_X509_OID_RSA,                     sizeof(NX_SECURE_X509_OID_RSA)},
    {NX_SECURE_TLS_X509_TYPE_RSA_MD5,                NX_SECURE_X509_OID_RSA_MD5,                 sizeof(NX_SECURE_X509_OID_RSA_MD5)},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,              NX_SECURE_X509_OID_RSA_SHA1,                sizeof(NX_SECURE_X509_OID_RSA_SHA1)},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,            NX_SECURE_X509_OID_RSA_SHA256,              sizeof(NX_SECURE_X509_OID_RSA_SHA256)},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,            NX_SECURE_X509_OID_RSA_SHA384,              sizeof(NX_SECURE_X509_OID_RSA_SHA384)},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,            NX_SECURE_X509_OID_RSA_SHA512,              sizeof(NX_SECURE_X509_OID_RSA_SHA512)},
    {NX_SECURE_TLS_X509_TYPE_DH,                     NX_SECURE_X509_OID_DH,                      sizeof(NX_SECURE_X509_OID_DH)},
    {NX_SECURE_TLS_X509_TYPE_DSS_SHA_1,              NX_SECURE_X509_OID_DSS_SHA1,                sizeof(NX_SECURE_X509_OID_DSS_SHA1)},
    {NX_SECURE_TLS_X509_TYPE_COMMON_NAME,            NX_SECURE_X509_OID_COMMON_NAME,             sizeof(NX_SECURE_X509_OID_COMMON_NAME)},
    {NX_SECURE_TLS_X509_TYPE_COUNTRY,                NX_SECURE_X509_OID_COUNTRY_NAME,            sizeof(NX_SECURE_X509_OID_COUNTRY_NAME)},
    {NX_SECURE_TLS_X509_TYPE_LOCALITY,               NX_SECURE_X509_OID_LOCALITY,                sizeof(NX_SECURE_X509_OID_LOCALITY)},
    {NX_SECURE_TLS_X509_TYPE_STATE,                  NX_SECURE_X509_OID_STATE,                   sizeof(NX_SECURE_X509_OID_STATE)},
    {NX_SECURE_TLS_X509_TYPE_ORGANIZATION,           NX_SECURE_X509_OID_ORGANIZATION,            sizeof(NX_SECURE_X509_OID_ORGANIZATION)},
    {NX_SECURE_TLS_X509_TYPE_ORG_UNIT,               NX_SECURE_X509_OID_ORG_UINT,                sizeof(NX_SECURE_X509_OID_ORG_UINT)},
    {NX_SECURE_TLS_X509_TYPE_DIRECTORY_ATTRIBUTES  , NX_SECURE_X509_OID_DIRECTORY_ATTRIBUTES  ,  sizeof(NX_SECURE_X509_OID_DIRECTORY_ATTRIBUTES)},
    {NX_SECURE_TLS_X509_TYPE_SUBJECT_KEY_ID        , NX_SECURE_X509_OID_SUBJECT_KEY_ID        ,  sizeof(NX_SECURE_X509_OID_SUBJECT_KEY_ID)},
    {NX_SECURE_TLS_X509_TYPE_KEY_USAGE             , NX_SECURE_X509_OID_KEY_USAGE             ,  sizeof(NX_SECURE_X509_OID_KEY_USAGE)},
    {NX_SECURE_TLS_X509_TYPE_SUBJECT_ALT_NAME      , NX_SECURE_X509_OID_SUBJECT_ALT_NAME      ,  sizeof(NX_SECURE_X509_OID_SUBJECT_ALT_NAME)},
    {NX_SECURE_TLS_X509_TYPE_ISSUER_ALT_NAME       , NX_SECURE_X509_OID_ISSUER_ALT_NAME       ,  sizeof(NX_SECURE_X509_OID_ISSUER_ALT_NAME)},
    {NX_SECURE_TLS_X509_TYPE_BASIC_CONSTRAINTS     , NX_SECURE_X509_OID_BASIC_CONSTRAINTS     ,  sizeof(NX_SECURE_X509_OID_BASIC_CONSTRAINTS)},
    {NX_SECURE_TLS_X509_TYPE_NAME_CONSTRAINTS      , NX_SECURE_X509_OID_NAME_CONSTRAINTS      ,  sizeof(NX_SECURE_X509_OID_NAME_CONSTRAINTS)},
    {NX_SECURE_TLS_X509_TYPE_CRL_DISTRIBUTION      , NX_SECURE_X509_OID_CRL_DISTRIBUTION      ,  sizeof(NX_SECURE_X509_OID_CRL_DISTRIBUTION)},
    {NX_SECURE_TLS_X509_TYPE_CERTIFICATE_POLICIES  , NX_SECURE_X509_OID_CERTIFICATE_POLICIES  ,  sizeof(NX_SECURE_X509_OID_CERTIFICATE_POLICIES)},
    {NX_SECURE_TLS_X509_TYPE_CERT_POLICY_MAPPINGS  , NX_SECURE_X509_OID_CERT_POLICY_MAPPINGS  ,  sizeof(NX_SECURE_X509_OID_CERT_POLICY_MAPPINGS)},
    {NX_SECURE_TLS_X509_TYPE_AUTHORITY_KEY_ID      , NX_SECURE_X509_OID_AUTHORITY_KEY_ID      ,  sizeof(NX_SECURE_X509_OID_AUTHORITY_KEY_ID)},
    {NX_SECURE_TLS_X509_TYPE_POLICY_CONSTRAINTS    , NX_SECURE_X509_OID_POLICY_CONSTRAINTS    ,  sizeof(NX_SECURE_X509_OID_POLICY_CONSTRAINTS)},
    {NX_SECURE_TLS_X509_TYPE_EXTENDED_KEY_USAGE    , NX_SECURE_X509_OID_EXTENDED_KEY_USAGE    ,  sizeof(NX_SECURE_X509_OID_EXTENDED_KEY_USAGE)},
    {NX_SECURE_TLS_X509_TYPE_ANY_EXTENDED_KEY_USAGE, NX_SECURE_X509_OID_ANY_EXTENDED_KEY_USAGE,  sizeof(NX_SECURE_X509_OID_ANY_EXTENDED_KEY_USAGE)},
    {NX_SECURE_TLS_X509_TYPE_FRESHEST_CRL          , NX_SECURE_X509_OID_FRESHEST_CRL          ,  sizeof(NX_SECURE_X509_OID_FRESHEST_CRL)},
    {NX_SECURE_TLS_X509_TYPE_INHIBIT_ANYPOLICY     , NX_SECURE_X509_OID_INHIBIT_ANYPOLICY     ,  sizeof(NX_SECURE_X509_OID_INHIBIT_ANYPOLICY)},
    {NX_SECURE_TLS_X509_TYPE_SURNAME               , NX_SECURE_X509_OID_SURNAME               ,  sizeof(NX_SECURE_X509_OID_SURNAME)},
    {NX_SECURE_TLS_X509_TYPE_SERIAL_NUMBER         , NX_SECURE_X509_OID_SERIAL_NUMBER         ,  sizeof(NX_SECURE_X509_OID_SERIAL_NUMBER)},
    {NX_SECURE_TLS_X509_TYPE_TITLE                 , NX_SECURE_X509_OID_TITLE                 ,  sizeof(NX_SECURE_X509_OID_TITLE)},
    {NX_SECURE_TLS_X509_TYPE_NAME                  , NX_SECURE_X509_OID_NAME                  ,  sizeof(NX_SECURE_X509_OID_NAME)},
    {NX_SECURE_TLS_X509_TYPE_GIVEN_NAME            , NX_SECURE_X509_OID_GIVEN_NAME            ,  sizeof(NX_SECURE_X509_OID_GIVEN_NAME)},
    {NX_SECURE_TLS_X509_TYPE_INITIALS              , NX_SECURE_X509_OID_INITIALS              ,  sizeof(NX_SECURE_X509_OID_INITIALS)},
    {NX_SECURE_TLS_X509_TYPE_GENERATION            , NX_SECURE_X509_OID_GENERATION            ,  sizeof(NX_SECURE_X509_OID_GENERATION)},
    {NX_SECURE_TLS_X509_TYPE_DN_QUALIFIER          , NX_SECURE_X509_OID_DN_QUALIFIER          ,  sizeof(NX_SECURE_X509_OID_DN_QUALIFIER)},
    {NX_SECURE_TLS_X509_TYPE_PSEUDONYM             , NX_SECURE_X509_OID_PSEUDONYM             ,  sizeof(NX_SECURE_X509_OID_PSEUDONYM)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_EXT_PREFIX       , NX_SECURE_X509_OID_PKIX_EXT_PREFIX       ,  sizeof(NX_SECURE_X509_OID_PKIX_EXT_PREFIX)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_AIA              , NX_SECURE_X509_OID_PKIX_AIA              ,  sizeof(NX_SECURE_X509_OID_PKIX_AIA)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_SIA              , NX_SECURE_X509_OID_PKIX_SIA              ,  sizeof(NX_SECURE_X509_OID_PKIX_SIA)},
    {NX_SECURE_TLS_X509_TYPE_NETSCAPE_COMMENT      , NX_SECURE_X509_OID_NETSCAPE_COMMENT      ,  sizeof(NX_SECURE_X509_OID_NETSCAPE_COMMENT)},
    {NX_SECURE_TLS_X509_TYPE_ANY_POLICY            , NX_SECURE_X509_OID_ANY_POLICY            ,  sizeof(NX_SECURE_X509_OID_ANY_POLICY)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_QT               , NX_SECURE_X509_OID_PKIX_QT               ,  sizeof(NX_SECURE_X509_OID_PKIX_QT)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_QT_CPS           , NX_SECURE_X509_OID_PKIX_QT_CPS           ,  sizeof(NX_SECURE_X509_OID_PKIX_QT_CPS)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_QT_UNOTICE       , NX_SECURE_X509_OID_PKIX_QT_UNOTICE       ,  sizeof(NX_SECURE_X509_OID_PKIX_QT_UNOTICE)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP               , NX_SECURE_X509_OID_PKIX_KP               ,  sizeof(NX_SECURE_X509_OID_PKIX_KP)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP_SERVER_AUTH   , NX_SECURE_X509_OID_PKIX_KP_SERVER_AUTH   ,  sizeof(NX_SECURE_X509_OID_PKIX_KP_SERVER_AUTH)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP_CLIENT_AUTH   , NX_SECURE_X509_OID_PKIX_KP_CLIENT_AUTH   ,  sizeof(NX_SECURE_X509_OID_PKIX_KP_CLIENT_AUTH)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP_CODE_SIGNING  , NX_SECURE_X509_OID_PKIX_KP_CODE_SIGNING  ,  sizeof(NX_SECURE_X509_OID_PKIX_KP_CODE_SIGNING)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP_EMAIL_PROTECT , NX_SECURE_X509_OID_PKIX_KP_EMAIL_PROTECT ,  sizeof(NX_SECURE_X509_OID_PKIX_KP_EMAIL_PROTECT)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP_TIME_STAMPING , NX_SECURE_X509_OID_PKIX_KP_TIME_STAMPING ,  sizeof(NX_SECURE_X509_OID_PKIX_KP_TIME_STAMPING)},
    {NX_SECURE_TLS_X509_TYPE_PKIX_KP_OCSP_SIGNING  , NX_SECURE_X509_OID_PKIX_KP_OCSP_SIGNING  ,  sizeof(NX_SECURE_X509_OID_PKIX_KP_OCSP_SIGNING)},
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    {NX_SECURE_TLS_X509_TYPE_EC                    , NX_SECURE_X509_OID_EC                    ,  sizeof(NX_SECURE_X509_OID_EC)},
    {NX_SECURE_TLS_X509_EC_SECP192R1               , NX_SECURE_X509_OID_SECP192R1             ,  sizeof(NX_SECURE_X509_OID_SECP192R1)},
    {NX_SECURE_TLS_X509_EC_SECP224R1               , NX_SECURE_X509_OID_SECP224R1             ,  sizeof(NX_SECURE_X509_OID_SECP224R1)},
    {NX_SECURE_TLS_X509_EC_SECP256R1               , NX_SECURE_X509_OID_SECP256R1             ,  sizeof(NX_SECURE_X509_OID_SECP256R1)},
    {NX_SECURE_TLS_X509_EC_SECP384R1               , NX_SECURE_X509_OID_SECP384R1             ,  sizeof(NX_SECURE_X509_OID_SECP384R1)},
    {NX_SECURE_TLS_X509_EC_SECP521R1               , NX_SECURE_X509_OID_SECP521R1             ,  sizeof(NX_SECURE_X509_OID_SECP521R1)},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1           , NX_SECURE_X509_OID_ECDSA_SHA1            ,  sizeof(NX_SECURE_X509_OID_ECDSA_SHA1)},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224         , NX_SECURE_X509_OID_ECDSA_SHA224          ,  sizeof(NX_SECURE_X509_OID_ECDSA_SHA224)},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256         , NX_SECURE_X509_OID_ECDSA_SHA256          ,  sizeof(NX_SECURE_X509_OID_ECDSA_SHA256)},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384         , NX_SECURE_X509_OID_ECDSA_SHA384          ,  sizeof(NX_SECURE_X509_OID_ECDSA_SHA384)},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512         , NX_SECURE_X509_OID_ECDSA_SHA512          ,  sizeof(NX_SECURE_X509_OID_ECDSA_SHA512)},
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
};

static const UINT _nx_secure_x509_oid_map_size = sizeof(_nx_secure_x509_oid_map) / sizeof(NX_SECURE_X509_OID_MAP);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_oid_parse                           PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses a DER-encoded Object Identifier (OID) string   */
/*    and returns an internally-defined constant for further use in X.509 */
/*    parsing routines.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    oid                                   OID data to be parsed         */
/*    length                                Length of OID data in buffer  */
/*    oid_value                             Return OID internal integer   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_crl_signature_algorithm_parse                       */
/*                                          Parse signature algorithm in  */
/*    _nx_secure_x509_distinguished_name_parse                            */
/*                                          Parse Distinguished Name      */
/*    _nx_secure_x509_extension_find        Find extension in certificate */
/*    _nx_secure_x509_extended_key_usage_extension_parse                  */
/*                                          Parse Extended KeyUsage       */
/*                                            extension                   */
/*    _nx_secure_x509_parse_public_key      Parse public key in           */
/*                                            certificate                 */
/*    _nx_secure_x509_parse_signature_algorithm                           */
/*                                          Parse signature algorithm in  */
/*    _nx_secure_x509_policy_qualifiers_parse                             */
/*                                          Parse policy qualifiers       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
VOID _nx_secure_x509_oid_parse(const UCHAR *oid, ULONG length, UINT *oid_value)
{
INT  compare_val;
UINT i;

    /*  Check for OID type. */
    for (i = 0; i < _nx_secure_x509_oid_map_size; ++i)
    {
        /* Make sure the length isn't greater than the size of the OID we are comparing against. */
        if (length <= _nx_secure_x509_oid_map[i].nx_secure_oid_map_oid_size)
        {
            compare_val = NX_SECURE_MEMCMP(oid, _nx_secure_x509_oid_map[i].nx_secure_oid_map_oid, _nx_secure_x509_oid_map[i].nx_secure_oid_map_oid_size);
            if (compare_val == 0)
            {
                *oid_value = _nx_secure_x509_oid_map[i].nx_secure_oid_map_type;
                return;
            }
        }
    }

    *oid_value = NX_SECURE_TLS_X509_TYPE_UNKNOWN;
}


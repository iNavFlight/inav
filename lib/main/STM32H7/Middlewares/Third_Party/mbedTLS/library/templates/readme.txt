
  @verbatim
  ******************************************************************************
  *
  *   COPYRIGHT (C) 2018 STMicroelectronics
  *
  * @file    readme.txt
  * @author  MCD Application Team
  * @brief   This file describes the content of the "templates" directory
  ******************************************************************************
  *
  * original licensing conditions
  * as issued by SPDX-License-Identifier: Apache-2.0
  *
  ******************************************************************************
  @endverbatim

This file contains template files that provide some alternate implementation for
mbedTLS algorithms.

aes_alt_template.[c/h], gcm_alt_template.[c/h], ccm_alt_template.[c/h], cryp_stm32.[c/h]
----------------------------------------------------------------------------------
Implements the mbedTLS AES crypto symmetric algorithms using the HAL/CRYP API.

- As the templates are generic for all STM32 families, you have to fill the appropriate
HAL header file within cryp_stm32.h,
for instance, stm32<xxxxx>_hal.h becomes stm32h7xx_hal.h for H7

- Make sure your mbed TLS config file enables the implementation with flags :
MBEDTLS_AES_ALT (or/and) MBEDTLS_GCM_ALT (or/and) MBEDTLS_CCM_ALT.

- Make sure your mbed TLS config file enables the features with the flags :
MBEDTLS_AES_C (or/and) MBEDTLS_GCM_C (or/and) MBEDTLS_CCM_C

- Files need to be copied at user level.
aes_alt_template.[c/h] renamed to "aes_alt.[c/h]"
gcm_alt_template.[c/h] renamed to "gcm_alt.[c/h]"
ccm_alt_template.[c/h] renamed to "ccm_alt.[c/h]"
cryp_stm32.[c/h], ST specific file mandatory to have whatever the implemented algorithm(s).

Note there may have a few family dependancies :
- for key length, Hw implementations may not support 192-bits key length
- for IVs, Hw implementations restric support to the length of 96 bits, to promote
interoperability, efficiency, and simplicity of design.
- for AAD, Hw implementations may restric support to an alignement over a length multiple
of 32 bits (default SW configuration).
AAD with any alignment limitation may be available by enabling STM32_AAD_ANY_LENGTH_SUPPORT.
- this implementation is thread-safe ready and can be run from different threads.

sha1_alt_template.[c/h], sha256_alt_template.[c/h], md5_alt_template.[c/h], hash_stm32.[c/h]
----------------------------------------------------------------------------------
Implements the mbedTLS secure hash and Message-Digest 5 algorithms using the HAL/CRYP API.

- As the templates are generic for all STM32 families, you have to fill the appropriate
HAL header file within hash_stm32.h,
for instance, stm32<xxxxx>_hal.h becomes stm32h7xx_hal.h for H7

- Make sure your mbed TLS config file enables the implementation with flags :
MBEDTLS_SHA1_ALT (or/and) MBEDTLS_SHA256_ALT (or/and) MBEDTLS_MD5_ALT.

- Make sure your mbed TLS config file enables the features with the flags :
MBEDTLS_SHA1_C (or/and) MBEDTLS_SHA256_C (or/and) MBEDTLS_MD5_C

- Files need to be copied at user level.
sha1_alt_template.[c/h] renamed to "sha1_alt.[c/h]"
sha256_alt_template.[c/h] renamed to "sha256_alt.[c/h]"
md5_alt_template.[c/h] renamed to "md5_alt.[c/h]"
hash_stm32.[c/h], ST specific file mandatory to have whatever the implemented algorithm(s).

Note this implementation is thread-safe ready and can be run from different threads.

net_sockets_template.c
-------------------------
implements of the mbedTLS networking API using the LwIP TCP/IP Stack.
This file implements the strict minimum required to ensure TCP/IP connection.
This file need to be copied at user level and renamed to "net_sockets.c"

rng_alt_tempate.c
---------------------
Implements the function mbedtls_hardware_poll(), required by mbedTLS to generate
random numbers. The function is using the HAL/RNG API to generate random number
using the rng hw IP.

threading_alt_template.[c/h]
-----------------------------
Implements the mutex management API required by mbedTLS, using the CMSIS-RTOS
V1 & V2 API

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
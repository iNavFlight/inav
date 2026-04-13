/**
  ******************************************************************************
  * @file    jdata_conf_template.c 
  * @brief   This file implements LibJPEG read/write functions.
  *          This file should be copied to the application folder and modified 
  *          as follows:
  *            - Rename it to 'jdata_conf.c'.
  *            - Implement read/write functions (example of implementation is 
  *              provided based on FatFs)
  ******************************************************************************
  * @attention
  ******************************************************************************
  *
  * Copyright (c) 2019 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
**/

/* Includes ------------------------------------------------------------------*/
#include "jdata_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*
 * The modules that use fread() and fwrite() always invoke them through
 * these macros.  On some systems you may need to twiddle the argument casts.
 * CAUTION: argument order is different from underlying functions!
 */

 
size_t read_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf)
{
/*  Example of implementation based on FatFS

    static size_t BytesReadfile ;  
    f_read (file, buf , sizeofbuf, &BytesReadfile); 
    return BytesReadfile;    
*/
}

size_t write_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf)
{
/*  Example of implementation based on FatFS

   static size_t BytesWritefile ;  
   f_write (file, buf , sizeofbuf, &BytesWritefile); 
   return BytesWritefile; 
*/   
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

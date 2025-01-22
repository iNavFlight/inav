/**
  ******************************************************************************
  * @file    jdata_conf_template.h
  *
  * @author  MCD Application Team
  * @brief   jdata_conf file template header file using FatFs API
  *          This file should be copied to the application "Inc" folder and modified 
  *          as follows:
  *            - Rename it to 'jdata_conf.h'.
  *            - update the  read/write functions defines (example of implementation is 
  *              provided based on FatFs)
  *          
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
#include "ff.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*
  Example of implementation based on FatFS
  
  If JFREAD/JFWRITE prototypes are complient with the standard FatFS APIs (f_read/f_write) 
  only APIs re-definition is needed and no need to wrapper APIs defined in "jdata_conf_template.c" file

   #define JFREAD  f_read
   #define JFWRITE f_write

*/


#define JFILE            FIL

#define JMALLOC   malloc    
#define JFREE     free  

size_t read_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf);
size_t write_file (JFILE  *file, uint8_t *buf, uint32_t sizeofbuf) ;

#define JFREAD(file,buf,sizeofbuf)  \
   read_file (file,buf,sizeofbuf)

#define JFWRITE(file,buf,sizeofbuf)  \
   write_file (file,buf,sizeofbuf)


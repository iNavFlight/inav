/* This is a small demo of the high-performance FileX FAT file system with LevelX 
   and the NAND simulated driver.  */

#include <stdio.h>
#include "fx_api.h"
#include "lx_api.h"


#define DEMO_STACK_SIZE 4096


/* Buffer for FileX FX_MEDIA sector cache. This must be large enough for at least one 
   sector, which are typically 512 bytes in size.  */

unsigned char media_memory[4096];


/* Define NAND simulated device driver entry.  */

VOID  _fx_nand_flash_simulator_driver(FX_MEDIA *media_ptr);


/* Define LevelX NAND simulated flash erase.  */

UINT  _lx_nand_flash_simulator_erase_all(VOID);
      
       
/* Define thread prototypes.  */

void    thread_0_entry(ULONG thread_input);
UCHAR   thread_0_stack[DEMO_STACK_SIZE];


/* Define FileX global data structures.  */

FX_MEDIA                nand_disk;
FX_FILE                 my_file;
FX_FILE                 my_file1;


/* Define ThreadX global data structures.  */

#ifndef LX_STANDALONE_ENABLE
TX_THREAD       thread_0;
#endif
ULONG           thread_0_counter;


int  main(void)
{
    /* Enter the ThreadX kernel.  */
#ifndef LX_STANDALONE_ENABLE
    tx_kernel_enter();
#else

    /* Initialize NAND flash.  */
    lx_nand_flash_initialize();    
    
    /* Initialize FileX.  */
    fx_system_initialize();

    thread_0_entry(0);
#endif

}


/* Define what the initial system looks like.  */

#ifndef LX_STANDALONE_ENABLE
void    tx_application_define(void *first_unused_memory)
{


    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    /* Create the main thread.  */
    tx_thread_create(&thread_0, "thread 0", thread_0_entry, 0,  
            thread_0_stack, DEMO_STACK_SIZE, 
            1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Initialize NAND flash.  */
    lx_nand_flash_initialize();    
    
    /* Initialize FileX.  */
    fx_system_initialize();
}
#endif


void    thread_0_entry(ULONG thread_input)
{

UINT        status;
ULONG       actual;
CHAR        local_buffer[30];

    LX_PARAMETER_NOT_USED(thread_input);
    
    /* Erase the simulated NAND flash.  */
    _lx_nand_flash_simulator_erase_all();
    
    /* Format the NAND disk - the memory for the NAND flash disk is setup in 
       the NAND simulator. Note that for best performance, the format of the
       NAND flash should be less than one full NAND flash block of sectors.  */
    fx_media_format(&nand_disk, 
                            _fx_nand_flash_simulator_driver,  // Driver entry
                            FX_NULL,                          // Unused
                            media_memory,                     // Media buffer pointer
                            sizeof(media_memory),             // Media buffer size 
                            "MY_NAND_DISK",                   // Volume Name
                            1,                                // Number of FATs
                            32,                               // Directory Entries
                            0,                                // Hidden sectors
                            120,                              // Total sectors 
                            2048,                             // Sector size   
                            1,                                // Sectors per cluster
                            1,                                // Heads
                            1);                               // Sectors per track 

    /* Loop to repeat the demo over and over!  */
    do
    {

        /* Open the NAND disk.  */
        status =  fx_media_open(&nand_disk, "NAND DISK", _fx_nand_flash_simulator_driver, FX_NULL, media_memory, sizeof(media_memory));

        /* Check the media open status.  */
        if (status != FX_SUCCESS)
        {

            /* Error, break the loop!  */
            break;
        }

        /* Create a file called TEST.TXT in the root directory.  */
        status =  fx_file_create(&nand_disk, "TEST.TXT");

        /* Check the create status.  */
        if (status != FX_SUCCESS)
        {

            /* Check for an already created status. This is expected on the
               second pass of this loop!  */
            if (status != FX_ALREADY_CREATED)
            {

                /* Create error, break the loop.  */
                break;
            }
        }

        /* Open the test file.  */
        status =  fx_file_open(&nand_disk, &my_file, "TEST.TXT", FX_OPEN_FOR_WRITE);

        /* Check the file open status.  */
        if (status != FX_SUCCESS)
        {

            /* Error opening file, break the loop.  */
            break;
        }

        /* Seek to the beginning of the test file.  */
        status =  fx_file_seek(&my_file, 0);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {

            /* Error performing file seek, break the loop.  */
            break;
        }

        /* Write a string to the test file.  */
        status =  fx_file_write(&my_file, " ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", 28);

        /* Check the file write status.  */
        if (status != FX_SUCCESS)
        {

            /* Error writing to a file, break the loop.  */
            break;
        }

        /* Seek to the beginning of the test file.  */
        status =  fx_file_seek(&my_file, 0);

        /* Check the file seek status.  */
        if (status != FX_SUCCESS)
        {

            /* Error performing file seek, break the loop.  */
            break;
        }

        /* Read the first 28 bytes of the test file.  */
        status =  fx_file_read(&my_file, local_buffer, 28, &actual);

        /* Check the file read status.  */
        if ((status != FX_SUCCESS) || (actual != 28))
        {

            /* Error reading file, break the loop.  */
            break;
        }

        /* Close the test file.  */
        status =  fx_file_close(&my_file);

        /* Check the file close status.  */
        if (status != FX_SUCCESS)
        {

            /* Error closing the file, break the loop.  */
            break;
        }

        /* Delete the file.  */
        status =  fx_file_delete(&nand_disk, "TEST.TXT");
        
        /* Check the file delete status.  */
        if (status != FX_SUCCESS)
        {

            /* Error deleting the file, break the loop.  */
            break;
        }
                
        /* Close the media.  */
        status =  fx_media_close(&nand_disk);

        /* Check the media close status.  */
        if (status != FX_SUCCESS)
        {

            /* Error closing the media, break the loop.  */
            break;
        }

        /* Increment the thread counter, which represents the number
           of successful passes through this loop.  */
        thread_0_counter++;
    } while (1);
        
    /* If we get here the FileX test failed!  */
    return;
}


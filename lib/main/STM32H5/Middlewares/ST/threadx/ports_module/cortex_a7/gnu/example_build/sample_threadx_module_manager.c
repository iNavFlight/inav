/* Small demonstration of the ThreadX module manager.  This demonstration assumes the program 
   manager is loaded at 0 and that RAM addresses 0x200000 through 0x400000 are available for
   use.  */

#include "tx_api.h"
#include "txm_module.h"


#define DEMO_STACK_SIZE  1024


/* Define the ThreadX object control blocks...  */

TX_THREAD               module_manager;

/* Define thread prototypes.  */

void    module_manager_entry(ULONG thread_input);

/* Define the module object pool area.  */
UCHAR   object_memory[16384];

/* Define the module data pool area.  */
#define MODULE_DATA_SIZE        65536
unsigned char module_data_area[MODULE_DATA_SIZE];

/* Define a module instance.  */
TXM_MODULE_INSTANCE   my_module1;
TXM_MODULE_INSTANCE   my_module2;

/* Define the count of memory faults.  */
ULONG   memory_faults;

/* Define fault handler.  */
VOID module_fault_handler(TX_THREAD *thread, TXM_MODULE_INSTANCE *module)
{
    /* Just increment the fault counter.   */
    memory_faults++;
}

/* Define main entry point.  */

int main()
{
    
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

    /* Create the module manager thread.  */
    tx_thread_create(&module_manager, "Module Manager Thread", module_manager_entry, 0,  
                     first_unused_memory, DEMO_STACK_SIZE, 
                     1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
}



/* Define the test threads.  */

void    module_manager_entry(ULONG thread_input)
{

    /* Initialize the module manager.   */
    txm_module_manager_initialize((VOID *) module_data_area, MODULE_DATA_SIZE);
    
    /* Create a pool for module objects.  */
    txm_module_manager_object_pool_create(object_memory, sizeof(object_memory));
    
    /* Register a fault handler.  */
    txm_module_manager_memory_fault_notify(module_fault_handler);
    
    /* Initialize MMU.  */
    txm_module_manager_mm_initialize();
    
    /* Load the module that is already there, in this example it is placed there by the multiple image download.  */
    txm_module_manager_in_place_load(&my_module1, "my module1", (VOID *) 0xSOME_ADDRESS);
    
    /* Load a second instance of the module.  */
    //txm_module_manager_in_place_load(&my_module2, "my module2", (VOID *) 0xSOME_ADDRESS);
    
    /* Enable shared memory regions for one module.  */
    //txm_module_manager_external_memory_enable(&my_module2, (void*)0x90000000, 0x010000, 0x3F);
    
    /* Start the modules.  */
    txm_module_manager_start(&my_module1);
    //txm_module_manager_start(&my_module2);
    
    /* Sleep for a while and let the modules run....  */
    tx_thread_sleep(50);
    
    /* Thread 0 in module1 should be terminated due to violating the MMU.  */
    
    /* Stop the modules.  */
    txm_module_manager_stop(&my_module1);
    txm_module_manager_stop(&my_module2);
    
    /* Unload the modules.  */
    txm_module_manager_unload(&my_module1);
    txm_module_manager_unload(&my_module2);
    
    /* Reload the modules.  */
    txm_module_manager_in_place_load(&my_module2, "my module2", (VOID *) module_code);
    txm_module_manager_in_place_load(&my_module1, "my module1", (VOID *) module_code);
    
    /* Give both modules shared memory.  */
    txm_module_manager_external_memory_enable(&my_module2, (void*)0x90000000, 0x010000, 0x3F);
    txm_module_manager_external_memory_enable(&my_module1, (void*)0x90000000, 0x010000, 0x3F);

    /* Start the module again.  */
    txm_module_manager_start(&my_module2);
    txm_module_manager_start(&my_module1);
    
    /* Now just spin...  */
    while(1)
    {
        tx_thread_sleep(100);
        /* Thread 0 and 5 in module1 should not exist because they violate the maximum priority.  */
    }
}





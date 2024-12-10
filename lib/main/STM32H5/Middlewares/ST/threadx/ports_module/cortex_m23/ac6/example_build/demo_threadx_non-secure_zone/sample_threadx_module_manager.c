/* Small demonstration of the ThreadX module manager.  */

#include "tx_api.h"
#include "txm_module.h"


#define DEMO_STACK_SIZE         1024

/* Define the ThreadX object control blocks...  */

static TX_THREAD                module_manager;
static TXM_MODULE_INSTANCE      my_module;


/* Define the object pool area.  */

static UCHAR                    object_memory[16384];


/* Define the module data pool area.  */

#define MODULE_DATA_SIZE (64 * 1024)
static UCHAR                    module_data_area[MODULE_DATA_SIZE];


/* The module code should be loaded here.  */

#define MODULE_CODE (0x00200000)


/* Define the shared memory area.  */

#define SHARED_MEMORY_SIZE      (256)
__attribute__((section("sharedmem")))
static UCHAR                    shared_memory[SHARED_MEMORY_SIZE] __attribute__((aligned (32)));


/* Define the count of memory faults.  */

static ULONG                    memory_faults;


/* Define thread prototypes.  */

void    module_manager_entry(ULONG thread_input);


/* Define fault handler.  */

static VOID module_fault_handler(TX_THREAD *thread, TXM_MODULE_INSTANCE *module)
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

CHAR    *pointer = (CHAR*)first_unused_memory;


    tx_thread_create(&module_manager, "Module Manager Thread", module_manager_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     1, 1, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;
}




/* Define the test threads.  */

void    module_manager_entry(ULONG thread_input)
{

    (void)thread_input;
    
    tx_thread_secure_stack_allocate(&module_manager, 256);
    
    /* Initialize the module manager.   */
    txm_module_manager_initialize((void *) module_data_area, MODULE_DATA_SIZE);

    txm_module_manager_object_pool_create(object_memory, sizeof(object_memory));

    /* Register a fault handler.  */
    txm_module_manager_memory_fault_notify(module_fault_handler);
    
    /* Load the module that is already there, in this example it is placed there by the multiple image download.  */
    txm_module_manager_in_place_load(&my_module, "my module", (void *) MODULE_CODE);
    
    /* Enable a read/write shared memory region.  */
    txm_module_manager_external_memory_enable(&my_module, (void *) shared_memory, SHARED_MEMORY_SIZE, TXM_MODULE_ATTRIBUTE_READ_WRITE);
    
    /* Start the module.  */
    txm_module_manager_start(&my_module);

    /* Sleep for a while....  */
    tx_thread_sleep(1000);
    
    /* Stop the module.  */
    txm_module_manager_stop(&my_module);
    
    /* Unload the module.  */
    txm_module_manager_unload(&my_module);

    /* Load the module that is already there.  */
    txm_module_manager_in_place_load(&my_module, "my module", (void *) MODULE_CODE);

    /* Start the module again.  */
    txm_module_manager_start(&my_module);
    
    /* Now just spin...  */
    while(1)
    {
    
        tx_thread_sleep(100);
    }
}

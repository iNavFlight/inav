#include "tx_api.h"


/* Define the prototypes for the test entry points.  */

void    tm_main(void);


/* Define main entry point.  */
int main()
{

    /* Initialize the platform if required. */
    /* Custom code goes here. */

    /* Enter the ThreadX kernel.  */
    tx_kernel_enter();
}


/* Define what the initial system looks like.  */

void    tx_application_define(void *first_unused_memory)
{

     /* Enter the Thread-Metric test main function for initialization and to start the test.  */
     tm_main();
}


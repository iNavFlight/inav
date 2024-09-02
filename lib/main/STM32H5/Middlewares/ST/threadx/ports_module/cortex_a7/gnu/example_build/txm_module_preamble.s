    .section .txm_module_preamble
    .align 4

/* Define common external references.  */

    .global     _txm_module_thread_shell_entry
    .global     demo_module_start
    .global     _txm_module_callback_request_thread_entry


__txm_module_preamble:
    .dc.l   0x4D4F4455                                  // Module ID
    .dc.l   0x6                                         // Module Major Version
    .dc.l   0x1                                         // Module Minor Version
    .dc.l   32                                          // Module Preamble Size in 32-bit words
    .dc.l   0x12345678                                  // Module ID (application defined)
    .dc.l   0x01000001                                  // Module Properties where:
                                                        //   Bits 31-24: Compiler ID
                                                        //           0 -> IAR
                                                        //           1 -> RVDS
                                                        //           2 -> GNU
                                                        //   Bits 23-1:  Reserved
                                                        //   Bit 0:  0 -> Privileged mode execution (no MMU protection)
                                                        //           1 -> User mode execution (MMU protection)
    .dc.l   _txm_module_thread_shell_entry              // Module Shell Entry Point
    .dc.l   demo_module_start                           // Module Start Thread Entry Point
    .dc.l   0                                           // Module Stop Thread Entry Point 
    .dc.l   1                                           // Module Start/Stop Thread Priority
    .dc.l   2046                                        // Module Start/Stop Thread Stack Size
    .dc.l   _txm_module_callback_request_thread_entry   // Module Callback Thread Entry
    .dc.l   1                                           // Module Callback Thread Priority
    .dc.l   2046                                        // Module Callback Thread Stack Size
    .dc.l   __code_size__                               // Module Code Size
    .dc.l   __data_size__                               // Module Data Size - default to 16K (need to make sure this is large enough for module's data needs!)
    .dc.l   0                                           // Reserved 0
    .dc.l   0                                           // Reserved 1
    .dc.l   0                                           // Reserved 2
    .dc.l   0                                           // Reserved 3
    .dc.l   0                                           // Reserved 4
    .dc.l   0                                           // Reserved 5
    .dc.l   0                                           // Reserved 6
    .dc.l   0                                           // Reserved 7
    .dc.l   0                                           // Reserved 8
    .dc.l   0                                           // Reserved 9
    .dc.l   0                                           // Reserved 10
    .dc.l   0                                           // Reserved 11
    .dc.l   0                                           // Reserved 12
    .dc.l   0                                           // Reserved 13
    .dc.l   0                                           // Reserved 14
    .dc.l   0                                           // Reserved 15

// ------------------------------------------------------------
// Cortex-A MPCore - Interrupt Controller functions
// Header File
//
// Copyright (c) 2011 ARM Ltd.  All rights reserved.
// ------------------------------------------------------------

#ifndef _CORTEXA_GIC_H
#define _CORTEXA_GIC_H

#define SPURIOUS                   (255)

// PPI IDs:
#define   MPCORE_PPI_PRIVATE_TIMER (29)
#define   MPCORE_PPI_PRIVATE_WD    (30)
#define   MPCORE_PPI_GLOBAL_TIMER  (27)
#define   MPCORE_PPI_LEGACY_IRQ    (31)
#define   MPCORE_PPI_LEGACY_FIQ    (28)

// ------------------------------------------------------------
// GIC
// ------------------------------------------------------------

// Typical calls to enable interrupt ID X:
// disableIntID(X)                 <-- Enable that ID
// setIntPriority(X, 0)           <-- Set the priority of X to 0 (the max priority)
// setPriorityMask(0x1F)          <-- Set Core's priority mask to 0x1F (the lowest priority)
// enableGIC()                     <-- Enable the GIC (global)
// enableGICProcessorInterface() <-- Enable the CPU interface (local to the core)
//


//  Global enable of the Interrupt Distributor
void enableGIC(void);

// Global disable of the Interrupt Distributor
void disableGIC(void);

// Enables the interrupt source number ID
void enableIntID(unsigned int ID);

// Disables the interrupt source number ID
void disableIntID(unsigned int ID);

// Enables the processor interface
// Must be done on each core separately
void enableGICProcessorInterface(void);

// Disables the processor interface
// Must be done on each core separately
void disableGICProcessorInterface(void);

// Sets the Priority mask register for the core run on
// The reset value masks ALL interrupts!
//
// NOTE: Bits 2:0 of this register are SBZ, the function does perform any shifting!
void setPriorityMask(unsigned int priority);

// Sets the Binary Point Register for the core run on
void setBinaryPoint(unsigned int priority);

// Sets the priority of the specifed ID
void setIntPriority(unsigned int ID, unsigned int priority);

// Sets the priority of the specifed ID
void getIntPriority(unsigned int ID, unsigned int priority);

#define MPCORE_IC_TARGET_NONE      (0x0)
#define MPCORE_IC_TARGET_CPU0      (0x1)
#define MPCORE_IC_TARGET_CPU1      (0x2)
#define MPCORE_IC_TARGET_CPU2      (0x4)
#define MPCORE_IC_TARGET_CPU3      (0x8)

// Sets the target CPUs of the specified ID
// For 'target' use one of the above defines
unsigned int setIntTarget(unsigned int ID, unsigned int target);

//Returns the target CPUs of the specified ID
unsigned int getIntTarget(unsigned int ID);

//  Returns the value of the Interrupt Acknowledge Register
unsigned int readIntAck(void);

// Writes ID to the End Of Interrupt register
void writeEOI(unsigned int ID);

// ------------------------------------------------------------
// SGI
// ------------------------------------------------------------

// Send a software generate interrupt
void sendSGI(unsigned int ID, unsigned int core_list, unsigned int filter_list);

// ------------------------------------------------------------
// TrustZone
// ------------------------------------------------------------

// Enables the sending of secure interrupts as FIQs
void enableSecureFIQs(void);

// Disables the sending of secure interrupts as FIQs
void disableSecureFIQs(void);

// Sets the specifed ID as secure
void makeIntSecure(unsigned int ID);

// Set the specified ID as non-secure
void makeIntNonSecure(unsigned int ID);

// Returns the security of the specifed ID
void getIntSecurity(unsigned int ID);

#endif

// ------------------------------------------------------------
// End of MP_GIC.h
// ------------------------------------------------------------

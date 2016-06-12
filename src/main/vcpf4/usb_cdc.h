/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usb_cdc_H
#define __usb_cdc_H

#include <stdbool.h>

// define interface for serial port driver
// only one instance is used now

extern bool usbcdc_SendStatus;

void USBCDC_Init(void);

void USBCDC_TryTx(void);
bool USBCDC_IsConnected(void);


#endif

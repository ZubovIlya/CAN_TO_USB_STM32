
#ifndef __usb_device_H
#define __usb_device_H

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"

extern USBD_HandleTypeDef hUsbDeviceHS;

void MX_USB_DEVICE_Init(void);

#endif /*__usb_device_H */

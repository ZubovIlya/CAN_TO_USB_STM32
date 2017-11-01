#ifndef USB_INTERFACE_H_
#define USB_INTERFACE_H_

#include "usbd_cdc_if.h"


void usb_analyze();
void send_mesg (uint8_t *Buf, uint32_t Len);
void recv_mesg (uint8_t *Buf, uint32_t Len);

#endif /* USB_INTERFACE_H_ */

#ifndef USB_INTERFACE_H_
#define USB_INTERFACE_H_

#include "usbd_cdc_if.h"

void send_repeat();
void send_ok();
void recv_mesg (uint8_t *Buf, uint32_t Len);
void send_mesg (uint8_t *Buf, uint32_t Len);
void usb_analyze();

enum {USB_ANALYSE_EMPTY,USB_ANALYSE_HAVE_A_MESSAGE,USB_ANALYSE_BUSY};
extern uint8_t usb_analyze_buff_flag;
enum{USB_REPEAT,USB_OK,USB_NONE};
extern uint8_t usb_ok_repeat_flag;

#endif /* USB_INTERFACE_H_ */

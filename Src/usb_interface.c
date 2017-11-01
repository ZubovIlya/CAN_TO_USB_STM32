#include "usb_interface.h"

//	PROTOCOL

#define USB_IFACE_START_BYTE 0xB0
#define USB_IFACE_START_BYTE_MASK 0xF0
#define USB_IFACE_LENGHT_MASK 0x0F

// ANSWER

uint8_t usb_iface_answer_buff;
#define USB_IFACE_ANSWER_OK 0xDD
#define USB_IFACE_ANSWER_REPEAT 0xDB

//	INPUT

#define USB_IFACE_INPUT_BUFF_SIZE 15
uint8_t usb_iface_input_buff[USB_IFACE_INPUT_BUFF_SIZE];

enum
uint8_t  {	USB_IFACE_INPUT_START_STATE
				,USB_IFACE_INPUT_DATA_STATE
				,USB_IFACE_INPUT_CRC_STATE
				,USB_IFACE_INPUT_ANSWER_STATE}
		usb_iface_input_state = USB_IFACE_INPUT_START_STATE;
uint8_t usb_iface_input_length = 0;
uint8_t usb_iface_input_counter = 0;
uint8_t usb_iface_input_crc = 0;

//	OUTPUT

#define USB_IFACE_OUTPUT_BUFF_SIZE 17
uint8_t usb_iface_output_buff[USB_IFACE_OUTPUT_BUFF_SIZE];
uint32_t usb_iface_output_buff_size;

// ANALYSE
#define USB_ANALYZE_BUFF_SIZE 15
#define USB_C_REPEAT 0x01
enum {USB_ANALYSE_EMPTY,USB_ANALYSE_HAVE_A_MESSAGE,USB_ANALYSE_BUSY};
uint8_t usb_analyze_buff_flag = USB_ANALYSE_EMPTY;
uint8_t usb_analyze_buff[USB_ANALYZE_BUFF_SIZE];
uint32_t usb_analyze_buff_length;
enum{USB_REPEAT,USB_OK,USB_NONE};
uint8_t usb_ok_repeat_flag = USB_NONE;

void usb_analyze()
{
	if (usb_analyze_buff_flag == USB_ANALYSE_HAVE_A_MESSAGE || usb_ok_repeat_flag != USB_NONE)
	{
		//	usb_ok_repeat_flag
		switch(usb_ok_repeat_flag)
		{
		case USB_OK:
			usb_iface_answer_buff = USB_IFACE_ANSWER_OK;
			CDC_Transmit_HS(&usb_iface_answer_buff, 1);
			usb_ok_repeat_flag=USB_NONE;
			break;
		case USB_REPEAT:
			usb_iface_answer_buff = USB_IFACE_ANSWER_REPEAT;
			CDC_Transmit_HS(&usb_iface_answer_buff, 1);
			usb_ok_repeat_flag=USB_NONE;
			break;
		default :
			//do nothing
			break;
		}
		//	usb_analyze_buff_flag
		if (usb_analyze_buff_flag == USB_ANALYSE_HAVE_A_MESSAGE)
		{
			usb_analyze_buff_flag = USB_ANALYSE_BUSY;
			switch (usb_iface_input_buff[0])
			{
			case USB_C_REPEAT:
				send_mesg(&usb_iface_input_buff[1],usb_analyze_buff_length-1);
				break;
			}
			usb_analyze_buff_flag = USB_ANALYSE_EMPTY;
		}
	}
}

void recv_mesg (uint8_t *buf, uint32_t Len)
{
	for (uint8_t i = 0; i<Len;i++)
	switch (usb_iface_input_state)
	{
		case USB_IFACE_INPUT_START_STATE:
			if ((buf[i] & USB_IFACE_START_BYTE_MASK) == USB_IFACE_START_BYTE)
			{
				usb_iface_input_length = buf[i] & USB_IFACE_LENGHT_MASK;
				usb_iface_input_crc = 0;
				usb_iface_input_counter = 0;
				usb_iface_input_state = USB_IFACE_INPUT_DATA_STATE;
				memset(usb_iface_input_buff,0,sizeof(uint8_t)*USB_IFACE_INPUT_BUFF_SIZE);
			}
			else
			{
				usb_ok_repeat_flag = USB_REPEAT;
				return;
			}
			break;
		case USB_IFACE_INPUT_DATA_STATE:
			usb_iface_input_buff[usb_iface_input_counter] = buf[i];
			usb_iface_input_crc ^= buf[i];
			usb_iface_input_counter++;
			if(usb_iface_input_counter==usb_iface_input_length)
				usb_iface_input_state = USB_IFACE_INPUT_CRC_STATE;
			break;
		case USB_IFACE_INPUT_CRC_STATE:
			if (buf[i] == usb_iface_input_crc)
			{
				usb_ok_repeat_flag = USB_OK;
				//ANALYZE DATA
				memset(usb_analyze_buff,0,sizeof(uint8_t)*USB_ANALYZE_BUFF_SIZE);
				memcpy(usb_analyze_buff,usb_iface_input_buff,usb_iface_input_length);
				usb_analyze_buff_length = usb_iface_input_length;
				usb_analyze_buff_flag = USB_ANALYSE_HAVE_A_MESSAGE;
				usb_iface_input_state = USB_IFACE_INPUT_START_STATE;
			}
			else
			{
				usb_ok_repeat_flag = USB_REPEAT;
				usb_iface_input_state = USB_IFACE_INPUT_START_STATE;
				return;
			}
			break;
		case USB_IFACE_INPUT_ANSWER_STATE:
			if (buf[i] == USB_IFACE_ANSWER_REPEAT)
			{
				uint8_t result = USBD_BUSY;
				while (result == USBD_BUSY)
					result = CDC_Transmit_HS(usb_iface_output_buff,usb_iface_output_buff_size);
			}
			else
				usb_iface_input_state = USB_IFACE_INPUT_START_STATE;
	}
}

void send_mesg (uint8_t *Buf, uint32_t Len)
{
	usb_iface_output_buff_size = Len+2;
	usb_iface_output_buff[0] =  USB_IFACE_START_BYTE | Len;	//	START_BYTE
	memcpy(&usb_iface_output_buff[1],Buf,Len);		//	DATA
	usb_iface_output_buff[Len+1] = 0;				//	CRC
	for (uint8_t i = 0;i<Len+1;i++)
		usb_iface_output_buff[Len+1] ^= Buf[i];
	uint8_t result = USBD_BUSY;
	while (result == USBD_BUSY)
		result = CDC_Transmit_HS(usb_iface_output_buff, usb_iface_output_buff_size);	//	PROFIT
	usb_iface_input_state = USB_IFACE_INPUT_ANSWER_STATE;
}

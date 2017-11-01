#include "usb_interface.h"

//	PROTOCOL
uint8_t answer_buff;
#define START_BYTE 0xB0
#define START_BYTE_MASK 0xF0
#define LENGHT_MASK 0x0F
#define MSG_TYPE_OK 0xDD
#define MSG_TYPE_REPEAT 0xDB
//	INPUT
#define INPUT_BUFF_SIZE 15
enum en_resv_state {USB_RS_START,USB_RS_DATA,USB_RS_CRC};
uint8_t input_buf[INPUT_BUFF_SIZE];
uint8_t resv_state = USB_RS_START;
uint8_t recv_lengt = 0;
uint8_t resv_conter = 0;
uint8_t resv_crc = 0;
//	OUTPUT
#define OUTPUT_BUFF_SIZE 17
uint8_t output_buf[OUTPUT_BUFF_SIZE];
// ANALYSE
#define USB_ANALYZE_BUFF_SIZE 15
#define USB_C_REPEAT 0x01
uint8_t usb_analyze_buff_flag = USB_ANALYSE_EMPTY;
uint8_t usb_analyze_buff[USB_ANALYZE_BUFF_SIZE];
uint32_t usb_analyze_buff_length;
uint8_t usb_ok_repeat_flag = USB_NONE;

void usb_analyze()
{
	if (usb_ok_repeat_flag == USB_OK)
	{
		send_ok();
		usb_ok_repeat_flag=USB_NONE;
	}
	if (usb_ok_repeat_flag == USB_REPEAT)
	{
		send_repeat();
		usb_ok_repeat_flag=USB_NONE;
	}

	usb_analyze_buff_flag = USB_ANALYSE_BUSY;
	switch (input_buf[0])
	{
	case USB_C_REPEAT:
		send_mesg(&input_buf[1],usb_analyze_buff_length-1);
		break;
	}
	usb_analyze_buff_flag = USB_ANALYSE_EMPTY;
}

void recv_mesg (uint8_t *buf, uint32_t Len)
{
	for (uint8_t i = 0; i<Len;i++)
	switch (resv_state)
	{
		case USB_RS_START:
			if ((buf[i] & START_BYTE_MASK) == START_BYTE)
			{
				recv_lengt = buf[i] & LENGHT_MASK;
				resv_crc = 0;
				resv_conter = 0;
				resv_state = USB_RS_DATA;
				memset(input_buf,0,sizeof(uint8_t)*INPUT_BUFF_SIZE);
			}
			else
			{
				usb_ok_repeat_flag = USB_REPEAT;
				return;
			}
			break;
		case USB_RS_DATA:
			input_buf[resv_conter] = buf[i];
			resv_crc ^= buf[i];
			resv_conter++;
			if(resv_conter==recv_lengt)
				resv_state = USB_RS_CRC;
			break;
		case USB_RS_CRC:
			if (buf[i] == resv_crc)
			{
				usb_ok_repeat_flag = USB_OK;
				//ANALYZE DATA
				memset(usb_analyze_buff,0,sizeof(uint8_t)*USB_ANALYZE_BUFF_SIZE);
				memcpy(usb_analyze_buff,input_buf,recv_lengt);
				usb_analyze_buff_length = recv_lengt;
				usb_analyze_buff_flag = USB_ANALYSE_HAVE_A_MESSAGE;
				resv_state = USB_RS_START;
			}
			else
			{
				usb_ok_repeat_flag = USB_REPEAT;
				resv_state = USB_RS_START;
				return;
			}
			break;
	}
}

inline void send_ok()
{
	answer_buff = MSG_TYPE_OK;
	CDC_Transmit_HS(&answer_buff, 1);
}

inline void send_repeat()
{
	answer_buff = MSG_TYPE_REPEAT;
	CDC_Transmit_HS(&answer_buff, 1);
}

void send_mesg (uint8_t *Buf, uint32_t Len)
{
	output_buf[0] =  START_BYTE | Len;	//	START_BYTE
	memcpy(&output_buf[1],Buf,Len);		//	DATA
	output_buf[Len+1] = 0;				//	CRC
	for (uint8_t i = 0;i<Len+1;i++)
		output_buf[Len+1] ^= Buf[i];
	uint8_t result;

	result = USBD_BUSY;
	while (result == USBD_BUSY)
		result = CDC_Transmit_HS(output_buf,Len+2);	//	PROFIT
	// сделать проверку на повтор

}

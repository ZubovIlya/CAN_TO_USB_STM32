#ifndef __can_H
#define __can_H

#include "stm32f4xx_hal.h"
#include "main.h"

extern CAN_HandleTypeDef hcan1;
extern void _Error_Handler(char *, int);

void MX_CAN1_Init(void);

#endif /*__ can_H */



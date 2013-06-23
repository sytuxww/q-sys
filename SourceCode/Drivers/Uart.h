#ifndef __UART_H__
#define __UART_H__

#include "stm32f10x.h"

void COM1_Init(void);
void COM3_Init(void);
void PrintUart3(const u8 *Str);

#endif


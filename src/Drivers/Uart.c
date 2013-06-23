/*
 * FileName:       
 * Author:         YuanYin  Version: QXW-V1.x  Date: 2010-3-11
 * Description: 此文件里的函数和例程1.Uart里的是一样的，这里就不做重复注释了。    
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */
#include "Drivers.h"     
#include <stdio.h>

void COM1_Init( void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	
	/* 开启GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	    
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;	//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//模式

	/* USART configuration */
	USART_Init(USART1, &USART_InitStructure);
	
	//使能串口中断，并设置优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART1_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);	//将结构体丢到配置函数，即写入到对应寄存器中

	/* Enable USART1 Receive interrupts */
    //USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    
	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
}

void COM3_Init( void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	
	/* 开启GPIO时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	    
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;	//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//模式

	/* USART configuration */
	USART_Init(USART3, &USART_InitStructure);
	
	//使能串口中断，并设置优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART3_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure);	//将结构体丢到配置函数，即写入到对应寄存器中

	/* Enable USART3 Receive interrupts */
    //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    
	/* Enable USART */
	USART_Cmd(USART3, ENABLE);
}

void PrintUart3(const u8 *Str)
{
	while(*Str)
	{
		USART_SendData(USART3, (unsigned char)*Str++); 
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}
}

//不使用半主机模式
#if 1 //如果没有这段，则需要在target选项中选择使用USE microLIB
#pragma import(__use_no_semihosting)
struct __FILE  
{  
	int handle;  
};  
FILE __stdout;  

_sys_exit(int x)  
{  
	x = x;  
}
#endif

extern u8 UseUsbOutputInfoFlag;
extern void USB_Send_Data(u8 Byte);
int fputc(int ch, FILE *f)
{
	if(UseUsbOutputInfoFlag) 
		USB_Send_Data((u8)ch);
	else
	{
		USART_SendData(USART1, (unsigned char)ch); 
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
	
	return ch;
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/


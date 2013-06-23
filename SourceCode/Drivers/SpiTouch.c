/*
 * FileName:       
 * Author:         YuanYin  Version: QXW-V1.x  Date: 2010-3-11
 * Description:  AT45DB161 ��������
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */
#include "Drivers.h"
#include "Spi.h"
#include "SpiTouch.h"

//��������ʱ�򷵻�0 
u16 GetTouchX(void) 
{ 
	u16 tmp_x; 

	Select_Touch(); 
	WriteByte_Touch(CMD_RDX); 

	tmp_x=ReadByte_Touch(); 
	tmp_x<<=8; 
	tmp_x|=ReadByte_Touch(); 
	tmp_x>>=3; 
	NotSelect_Touch(); 

	//Debug("x:%d\n\r",tmp_x);
	return tmp_x; 
} 

//��������ʱ�򷵻�0xfff 
u16 GetTouchY(void) 
{ 
	u16 tmp_y; 

	Select_Touch(); 
	WriteByte_Touch(CMD_RDY); 

	tmp_y=ReadByte_Touch(); 
	tmp_y<<=8; 
	tmp_y|=ReadByte_Touch(); 
	tmp_y>>=3; 
	NotSelect_Touch(); 

	//Debug("y:%d\n\r",tmp_y);
	return tmp_y; 
} 

//��ʼ��spi
void SPI_Touch_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;	
	GPIO_InitTypeDef GPIO_InitStructure;

#if QXW_LCM_ID == 210
	SPI2_Init();
	
	//����Ƭѡ�ܽ�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//��ѡ
	NotSelect_Touch();
#else
	SPIv_Init();

	//����Ƭѡ�ܽ�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//��ѡ
	NotSelect_Touch();
#endif

	//�����жϽ�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD,GPIO_Pin_12);

	//����int��
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource12);

	EXTI_InitStructure.EXTI_Line = EXTI_Line12;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);  

	//�����ж�
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI15_10_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Enable_Touch_Inperrupt(void)
{
	EXTI->IMR |= EXTI_Line12;
}

void Disable_Touch_Inperrupt(void)
{
	EXTI->IMR &=~EXTI_Line12;
}




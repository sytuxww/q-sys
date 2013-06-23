#ifndef __DRIVERS_H__
#define __DRIVERS_H__

#include "stm32f10x.h"

/* ISR Priority 0(highest)-15(lowest)*/
//����ԽС���ȼ�Խ��	
#define SDIO_IRQn_Priority	1
#define USB_LP_CAN1_RX0_IRQn_Priority 2
#define EXTI15_10_IRQn_Priority 3	//������
//#define EXTI2_IRQn_Priority	3		//�ⲿ����1
#define EXTI3_IRQn_Priority	4		//cc2500
#define EXTI9_5_IRQn_Priority		4		//cc2500
#define EXTI4_IRQn_Priority	5		//vs1003
#define RTC_Priority 6		
#define USART1_IRQn_Priority 7
#define USART3_IRQn_Priority 8
#define TIME2_IRQn_Priority 9
#define TIME4_IRQn_Priority 9
#define TIME5_IRQn_Priority 9
#define RTCAlarm_Priority 10

//-------------------��Ʒ ID-----------------------
//�˴����Բ����壬�ŵ�Keil����Ŀѡ���ﶨ�壬�����Ŀѡ����û�ж��壬��ô����һ��Ҫ���塣
//114:Summer V1.4
//115:Summer V1.5
//116:Summer V1.6
#define QXW_PRODUCT_ID 116

//210:LCM V1.0 2.2'
//211:LCM V1.1 2.2'
//212:LCM V1.2 2.2'
//220:LCM V2.0 2.8'
//221:LCM V2.1 2.8'	//ע��2011.12.1�������2.8������Ϊ�˰汾��˿ӡ��ȻΪ220
#define QXW_LCM_ID 212

//If this is a debug version,pls set to 0.
#define QXW_RELEASE_VER 1

#endif


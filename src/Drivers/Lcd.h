#ifndef __LCD_H__
#define __LCD_H__

#include "Drivers.h"
#include "stm32f10x.h"

#define LCD_BGLIGHT_PWM_MODE 1 //����pwm���Ʊ���

#if QXW_LCM_ID == 212 || QXW_LCM_ID == 211 || QXW_LCM_ID == 210

typedef enum{
	xInc_yInc=0,
	xInc_yDec,
	xDec_yInc,
	xDec_yDec
}LCD_INC_MODE;//LCD ��������ģʽ������ݲ�ͬLCD������������Ķ���λ��

#elif QXW_LCM_ID == 220 || QXW_LCM_ID == 221

typedef enum{
	xDec_yDec=0,
	xInc_yDec,
	xDec_yInc,
	xInc_yInc
}LCD_INC_MODE;

#endif

void LCD_BulkReadDataStart(void);//������ȡ����ǰ��Ҫ��������
u16 LCD_BulkReadData(void);//������ȡҺ����ɫ������
void LCD_BlukWriteDataStart(void);//����дҺ����֮ǰҪ��������
void LCD_BulkWriteData(u16 val);//������ȡҺ����ɫ������

void LCD_SetXY(u16 x,u16 y);//��������
void LCD_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end,bool yIncFrist);//���û滭��Χ��yIncFrist��ʾ������Y�ټ�X
void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection);//����xy��������
void LCD_BgrMode(bool UseBGR);//��������BGR����RGB
void LCD_AddrInc(void);//��ַ����1

void LCD_DrawPoint(u16 x,u16 y,u16 Data);//��ָ����
u16 LCD_ReadPoint(u16 x,u16 y);//��ָ����

void LCD_Light_Set(u8 Scale);//���ñ�������
u8 LCD_Light_State(void);//��ȡ��������

void LCD_Reset(void);//Ӳ����λ
void LCD_Init(void);//��ʼ��
void LCD_DeInit(void);//ע��

#endif


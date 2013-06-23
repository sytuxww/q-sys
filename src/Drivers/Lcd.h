#ifndef __LCD_H__
#define __LCD_H__

#include "Drivers.h"
#include "stm32f10x.h"

#define LCD_BGLIGHT_PWM_MODE 1 //启用pwm控制背光

#if QXW_LCM_ID == 212 || QXW_LCM_ID == 211 || QXW_LCM_ID == 210

typedef enum{
	xInc_yInc=0,
	xInc_yDec,
	xDec_yInc,
	xDec_yDec
}LCD_INC_MODE;//LCD 坐标自增模式，需根据不同LCD调节四种情况的定义位置

#elif QXW_LCM_ID == 220 || QXW_LCM_ID == 221

typedef enum{
	xDec_yDec=0,
	xInc_yDec,
	xDec_yInc,
	xInc_yInc
}LCD_INC_MODE;

#endif

void LCD_BulkReadDataStart(void);//批量读取数据前需要做的事情
u16 LCD_BulkReadData(void);//批量读取液晶屏色彩数据
void LCD_BlukWriteDataStart(void);//批量写液晶屏之前要做的事情
void LCD_BulkWriteData(u16 val);//批量读取液晶屏色彩数据

void LCD_SetXY(u16 x,u16 y);//设置坐标
void LCD_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end,bool yIncFrist);//设置绘画范围，yIncFrist表示先增加Y再加X
void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection);//设置xy自增方向
void LCD_BgrMode(bool UseBGR);//设置是用BGR或者RGB
void LCD_AddrInc(void);//地址自增1

void LCD_DrawPoint(u16 x,u16 y,u16 Data);//画指定点
u16 LCD_ReadPoint(u16 x,u16 y);//读指定点

void LCD_Light_Set(u8 Scale);//设置背光亮度
u8 LCD_Light_State(void);//读取背光亮度

void LCD_Reset(void);//硬件复位
void LCD_Init(void);//初始化
void LCD_DeInit(void);//注销

#endif


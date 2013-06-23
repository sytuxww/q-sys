#ifndef __SPI_TOUCH_H__
#define __SPI_TOUCH_H__  

#define CMD_RDY 0X90  //0B10010000即用差分方式读Y坐标
#define CMD_RDX 0XD0  //0B11010000即用差分方式读X坐标 

//片选
#if QXW_LCM_ID == 210
#define Select_Touch()     GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define NotSelect_Touch()    GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define WriteByte_Touch(x) SPI_WriteByte(SPI2,x)
#define ReadByte_Touch() SPI_ReadByte(SPI2)
#define HasTouch() GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)?FALSE:TRUE
#else
#define Select_Touch()     GPIO_ResetBits(GPIOE,GPIO_Pin_1)
#define NotSelect_Touch() GPIO_SetBits(GPIOE,GPIO_Pin_1)
#define WriteByte_Touch SPIv_WriteByte
#define ReadByte_Touch SPIv_ReadByte
#define HasTouch() GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)?FALSE:TRUE
#endif

void SPI_Touch_Init(void);
u8 Read_Once_Touch(unsigned int *Xin,unsigned int *Yin);
u16 GetTouchX(void) ;
u16 GetTouchY(void) ;
void Enable_Touch_Inperrupt(void);
void Disable_Touch_Inperrupt(void);

#endif





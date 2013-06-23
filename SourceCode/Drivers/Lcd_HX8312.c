/********************************************************************************
 * FileName:       
 * Author:         YuanYin  Version: QXW-Summer-V1.x  Date: 2010-4-29
 * Description:    
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 *      YuanYin       4-29         1.x   
*******************************************************************************/


#include "Drivers.h"
#include "Lcd.h"
#if LCD_BGLIGHT_PWM_MODE
#include "Time.h"
#endif
#include "Debug.h"

#if QXW_LCM_ID == 212 || QXW_LCM_ID == 211 || QXW_LCM_ID == 210

#define LCD_HX8312_On() GPIO_SetBits(GPIOC,GPIO_Pin_7)
#define LCD_HX8312_Off() GPIO_ResetBits(GPIOC,GPIO_Pin_7)

#define Bank1_LCD_R    ((uint32_t)0x60000000)    //disp Reg ADDR
#define Bank1_LCD_D    ((uint32_t)0x60020000)	//disp Data ADDR 

static u8 gLcdScale=100;

/**********************************************
��������LCD_DelayMs
���ܣ�����LCD������ʱ
��ڲ�������ʱ��
����ֵ����
***********************************************/
static void LCD_DelayMs(u32 Ms)
{
  u32 i;
	for(; Ms; Ms--)
		for(i=1000;i;i--);
}

/*************************************************
��������LCD_WriteReg
���ܣ���lcd�Ĵ�����д����
��ڲ������Ĵ�����ַ������
����ֵ���Ĵ���ֵ
*************************************************/
static void LCD_WriteReg(u8 index,u8 val)
{	
	*(__IO uint16_t *) (Bank1_LCD_R)= (index<<8)+val;	
}

/*************************************************
��������LCD_WR_Data_Start
���ܣ�LCD��ʼ����������ǰ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_BlukWriteDataStart(void)
{	

}

/*************************************************
��������LCD_ReadDataStart
���ܣ�LCD��ʼ����������ǰ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_BulkReadDataStart(void)
{	
	LCD_BulkReadData();
}

/*************************************************
��������LCD_BulkWriteData
���ܣ���lcd����д����
��ڲ���������ֵ
����ֵ����
*************************************************/
void LCD_BulkWriteData(u16 val)
{   
	*(__IO uint16_t *) (Bank1_LCD_D)= val;
}

/*************************************************
��������LCD_BulkReadData
���ܣ�������lcd����
��ڲ�������
����ֵ������
*************************************************/
u16 LCD_BulkReadData(void)
{
	return (*(__IO uint16_t *) (Bank1_LCD_D));	
}

/*************************************************
��������LCD_Set_XY
���ܣ�����lcd��ʾ��ʼ��
��ڲ�����xy����
����ֵ����
*************************************************/
void LCD_SetXY(u16 x,u16 y)
{
	LCD_WriteReg(0x42,x);
	LCD_WriteReg(0x43,y >> 8);
	LCD_WriteReg(0x44,y & 0xff);	
}

/*************************************************
��������LCD_Set_Region
���ܣ�����lcd��ʾ�����ڴ�����д�������Զ�����
��ڲ�����xy�����յ�,Y_IncMode��ʾ������y������x
����ֵ����
*************************************************/
void LCD_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end,bool yIncFrist)
{		
	LCD_WriteReg(0x05,0x10|(yIncFrist&0x01)<<2);
	
	LCD_WriteReg(0x45,x_start);
	LCD_WriteReg(0x46,x_end);

	LCD_WriteReg(0x47,y_start >> 8);
	LCD_WriteReg(0x48,y_start & 0xff);
	
	LCD_WriteReg(0x49,y_end >> 8);
	LCD_WriteReg(0x4a,y_end & 0xff);	

	//LCD_SetXY(x_start,y_start);
}

/*************************************************
��������LCD_Set_XY_Addr_Direction
���ܣ�����lcd����д�����ķ���
��ڲ�����0:��0���ߣ�1:�ɸߵ�0
����ֵ����
*************************************************/
void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection)
{
	LCD_WriteReg(0x01,(xyDirection&0x03)<<6);
}

/*************************************************
��������LCD_BGR_Mode
���ܣ�����lcd RGB˳��
��ڲ�����0:RGB   1:BGR
����ֵ����
*************************************************/
void LCD_BgrMode(bool UseBGR)
{
	LCD_WriteReg(0xc1,UseBGR&0x01);
}

/*************************************************
��������LCD_Addr_Inc
���ܣ���ַ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_AddrInc(void)
{
	LCD_BulkReadData();
}

/*************************************************
��������LCD_DrawPoint
���ܣ���һ����
��ڲ�������
����ֵ����
*************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 Data)
{
	LCD_SetRegion(x,y,x+1,y+1,FALSE);
	LCD_SetXY(x,y);
	LCD_BulkWriteData(Data);
}

/*************************************************
��������LCD_DrawPoint
���ܣ���һ����
��ڲ�������
����ֵ����
*************************************************/
u16 LCD_ReadPoint(u16 x,u16 y)
{
	LCD_SetRegion(x,y,x+1,y+1,FALSE);
	LCD_SetXY(x,y);
	LCD_BulkReadData();
	return LCD_BulkReadData();
}

/*************************************************
��������LCD_Power_On
���ܣ�LCD��������
��ڲ�������
����ֵ����
*************************************************/
static void LCD_PowerOn(void)
{
	LCD_Reset();	 

	LCD_WriteReg(0x01,0x10);      //Start oscillation
	LCD_WriteReg(0x00,0xA0);      //Standby mode cancel
	LCD_WriteReg(0x03,0x01);      //Software reset operation
	LCD_DelayMs(10);
	LCD_WriteReg(0x03,0x00);      //Software reset operation cancel
	LCD_DelayMs(100);
	LCD_WriteReg(0x05,0X04);      //
	LCD_WriteReg(0x2B,0x04);      //Oscillator frequency adjust setting

	LCD_WriteReg(0x59,0x01);      //Test register setting enable
	LCD_WriteReg(0x60,0x22);      //Test register setting 
	LCD_WriteReg(0x59,0x00);      //Test register setting disable

	LCD_WriteReg(0x28,0x18);      //DC/DC clock frequency adjust setting
	LCD_WriteReg(0x1A,0x05);      //Step up circuit frequensy adjust setting
	LCD_WriteReg(0x25,0x05);      //Step up factor in step up circuit 2 setting
	LCD_WriteReg(0x19,0x00);      //VR1 and VR2 regulator factor setting

	LCD_WriteReg(0x1C,0x73);      //Step up circuit operating current setting
	LCD_WriteReg(0x24,0x74);      //V18 and VCOM regulator current setting
	LCD_WriteReg(0x1E,0x01);      //Extra step up circuit1 operation
	LCD_WriteReg(0x18,0xC1);      //VR1 and VR2 regulator on
	LCD_DelayMs(10);
	LCD_WriteReg(0x18,0xE1);      //VCL turn on
	LCD_WriteReg(0x18,0xF1);      //VGH and VGL turn on
	LCD_DelayMs(60);
	LCD_WriteReg(0x18,0xF5);      //DDVDH turn on
	LCD_DelayMs(60);
	LCD_WriteReg(0x1B,0x09);      //VS/VDH turn on and set
	LCD_DelayMs(10);   
	LCD_WriteReg(0x1F,0x11);      //VCOM amplitude voltage setting
	LCD_WriteReg(0x20,0x0E);      //VCOMH voltage setting
	LCD_WriteReg(0x1E,0x81);      //VOCM operation start
	LCD_DelayMs(10);

	LCD_WriteReg(0x9D,0x00);
	LCD_WriteReg(0xC0,0x00);
	LCD_WriteReg(0xC1,0x01);      //BRG bit = "0"
	LCD_WriteReg(0x0E,0x00);
	LCD_WriteReg(0x0F,0x00);
	LCD_WriteReg(0x10,0x00);
	LCD_WriteReg(0x11,0x00);
	LCD_WriteReg(0x12,0x00);
	LCD_WriteReg(0x13,0x00);
	LCD_WriteReg(0x14,0x00);
	LCD_WriteReg(0x15,0x00);
	LCD_WriteReg(0x16,0x00);
	LCD_WriteReg(0x17,0x00);
	LCD_WriteReg(0x34,0x01);
	LCD_WriteReg(0x35,0x00);
	LCD_WriteReg(0x4B,0x00);
	LCD_WriteReg(0x4C,0x00);
	LCD_WriteReg(0x4E,0x00);
	LCD_WriteReg(0x4F,0x00);
	LCD_WriteReg(0x50,0x00);

	LCD_WriteReg(0x3C,0x00);
	LCD_WriteReg(0x3D,0x00);
	LCD_WriteReg(0x3E,0x01);
	LCD_WriteReg(0x3F,0x3F);
	LCD_WriteReg(0x40,0x02);
	LCD_WriteReg(0x41,0x02);

	LCD_WriteReg(0x42,0x00);
	LCD_WriteReg(0x43,0x00);
	LCD_WriteReg(0x44,0x00);
	LCD_WriteReg(0x45,0x00);
	LCD_WriteReg(0x46,0xEF);
	LCD_WriteReg(0x47,0x00);
	LCD_WriteReg(0x48,0x00);
	LCD_WriteReg(0x49,0x01);
	LCD_WriteReg(0x4A,0x3F);

	LCD_WriteReg(0x1D,0x08);      //Gate scan direction setting
	LCD_WriteReg(0x86,0x00);
	LCD_WriteReg(0x87,0x30);
	LCD_WriteReg(0x88,0x02);
	LCD_WriteReg(0x89,0x05);
	LCD_WriteReg(0x8D,0x01);      //Register set-up mode for one line clock number
	LCD_WriteReg(0x8B,0x30);      //One line SYSCLK number in one-line scan
	LCD_WriteReg(0x33,0x01);      //N line inversion setting
	LCD_WriteReg(0x37,0x01);      //Scanning method setting
	LCD_WriteReg(0x76,0x00);

	LCD_WriteReg(0x8F,0x10);
	LCD_WriteReg(0x90,0x67);
	LCD_WriteReg(0x91,0x07);
	LCD_WriteReg(0x92,0x65);
	LCD_WriteReg(0x93,0x07);
	LCD_WriteReg(0x94,0x01);
	LCD_WriteReg(0x95,0x76);
	LCD_WriteReg(0x96,0x56);
	LCD_WriteReg(0x97,0x00);
	LCD_WriteReg(0x98,0x00);
	LCD_WriteReg(0x99,0x00);
	LCD_WriteReg(0x9A,0x00);

	LCD_WriteReg(0x3B,0x01);
	LCD_DelayMs(40);
	LCD_WriteReg(0x00,0x20);   
	LCD_DelayMs(80);

	LCD_WriteReg(0x42,0);
	LCD_WriteReg(0x43,0);
	LCD_WriteReg(0x44,0);
}

/*************************************************
��������LCD_Power_Off
���ܣ�LCD�ر�����
��ڲ�������
����ֵ����
*************************************************/
static void LCD_PowerOff(void)
{
	LCD_WriteReg(0x1e,0x01);
	LCD_WriteReg(0x1b,0x00);
	LCD_WriteReg(0x18,0xc0);
	LCD_DelayMs(10);
	LCD_WriteReg(0x1c,0x30);
}

/*************************************************
��������LCD_Light_Set
���ܣ�LCD���ñ�������
��ڲ�����Scale:0-100��0ΪϨ��100����
����ֵ����
*************************************************/
void LCD_Light_Set(u8 Scale)
{
	Debug("LCD Light Set:%d\n\r",Scale);
#if LCD_BGLIGHT_PWM_MODE
	Tim3_PWM(Scale);
#else
	if(Scale) GPIO_SetBits(GPIOC,GPIO_Pin_6);
	else GPIO_ResetBits(GPIOC,GPIO_Pin_6);
#endif
	gLcdScale=Scale;
}

/*************************************************
��������LCD_Light_State
����:��ѯ��ǰ��������
��ڲ�������
����ֵ��0-100��0ΪϨ��100����
*************************************************/
u8 LCD_Light_State(void)
{
	return gLcdScale;
}

#if 0
/*************************************************
��������LCD_Display_On
���ܣ�LCD��ʾ��������
��ڲ�������
����ֵ����
*************************************************/
static void LCD_Display_On(void)
{
	LCD_WriteReg(0x00,0x80);
	LCD_WriteReg(0x3b,0x01);
	LCD_DelayMs(100);
	LCD_WriteReg(0x00,0x20);
}

/*************************************************
��������LCD_Display_Off
���ܣ�LCD��ʾ�ر�����
��ڲ�������
����ֵ����
*************************************************/
static void LCD_Display_Off(void)
{
	LCD_WriteReg(0x00,0x80);
	LCD_DelayMs(100);
	LCD_WriteReg(0x3b,0x00);	
}
#endif

/**********************************************
��������FSMC_LCD_Init
���ܣ�����FSMC����
��ڲ�������
����ֵ����
***********************************************/
static void LCD_FSMC_Init(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE); 

	p.FSMC_AddressSetupTime = 0x02;
	p.FSMC_AddressHoldTime = 0x00;
	p.FSMC_DataSetupTime = 0x05;
	p.FSMC_BusTurnAroundDuration = 0x00;
	p.FSMC_CLKDivision = 0x00;
	p.FSMC_DataLatency = 0x00;
	p.FSMC_AccessMode = FSMC_AccessMode_B;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_NOR;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;	  

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}

/**********************************************
��������LCD_Configuration
���ܣ�����LCD��IO����
��ڲ�������ʱ��
����ֵ����
***********************************************/
static void LCD_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOE, ENABLE); 	

  //����ic�������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //LCD Reset
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

#if LCD_BGLIGHT_PWM_MODE //LCD �������,	  
  /*����PC6 PWM*/
  //pc6����tim3 ch1��remap
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
  GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //TIM3 ��ȫ��ӳ�� 
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

  //FSMC��GPIOD�ܽ�
  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_14 	//D0
  												| GPIO_Pin_15 		//D1
  												| GPIO_Pin_0		//D2
  												| GPIO_Pin_1		//D3
  												| GPIO_Pin_8 		//D13
  												| GPIO_Pin_9 		//D14
  												| GPIO_Pin_10 		//D15  		
  												| GPIO_Pin_7		//NE1
  												| GPIO_Pin_11		//RS
  												| GPIO_Pin_4		//nRD
  												| GPIO_Pin_5;		//nWE
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  //FSMC��GPIOE�ܽ�
  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_7		//D4
  												| GPIO_Pin_8 		//D5
  												| GPIO_Pin_9 		//D6
  												| GPIO_Pin_10 		//D7
  												| GPIO_Pin_11 		//D8
  												| GPIO_Pin_12 		//D9
  												| GPIO_Pin_13		//D10
  												| GPIO_Pin_14 		//D11
  												| GPIO_Pin_15; 	//D12  												
  GPIO_Init(GPIOE, &GPIO_InitStructure); 
}

/*************************************************
��������LCD_Init
���ܣ���ʼ������lcd
��ڲ�������
����ֵ����
*************************************************/
void LCD_Init(void)
{	
	LCD_FSMC_Init();
	LCD_DelayMs(100);	 
	LCD_Configuration();
	LCD_HX8312_On();
	LCD_DelayMs(100);
	LCD_Light_Set(100);
	LCD_PowerOn();
	LCD_BgrMode(FALSE);
}

/*************************************************
��������LCD_DeInit
���ܣ�ע��
��ڲ�������
����ֵ����
*************************************************/
void LCD_DeInit(void)
{
	LCD_PowerOff();
}

/**********************************************
��������LCD_Reset
���ܣ�LCD��λ
��ڲ�������ʱ��
����ֵ����
***********************************************/ 
void LCD_Reset(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_13);
    LCD_DelayMs(50);					   
    GPIO_SetBits(GPIOD, GPIO_Pin_13);		 	 
	LCD_DelayMs(50);	
}

#endif


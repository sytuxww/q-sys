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

#if QXW_LCM_ID == 220 || QXW_LCM_ID == 221

#define LCD_ILI9320_On() GPIO_SetBits(GPIOC,GPIO_Pin_7)
#define LCD_ILI9320_Off() GPIO_ResetBits(GPIOC,GPIO_Pin_7)

#define Bank1_LCD_R    ((uint32_t)0x60000000)    //disp Reg ADDR
#define Bank1_LCD_D    ((uint32_t)0x60020000)	//disp Data ADDR 

static u8 gLcdScale=100;

//9320功能寄存器地址
#define WINDOW_XADDR_START	0x0050 // 水平的开始地址组
#define WINDOW_XADDR_END		0x0051 // 水平的结束地址组
#define WINDOW_YADDR_START	0x0052 // 垂直的开始地址组
#define WINDOW_YADDR_END		0x0053 // 垂直的结束地址组
#define GRAM_XADDR		    		0x0020 // GRAM 水平的地址组
#define GRAM_YADDR		    		0x0021 // GRAM 垂直的地址组
#define GRAMWR 			    			0x0022 // GRAM

/**********************************************
函数名：LCD_Delay
功能：用于LCD配置延时
入口参数：延时数
返回值：无
***********************************************/
static void LCD_DelayMs(u32 Ms)
{
  u32 i;
	for(; Ms; Ms--)
		for(i=1000;i;i--);
}

/*************************************************
函数名：LCD_WR_Cmd
功能：传入寄存器地址
入口参数：寄存器地址
返回值：无
*************************************************/
static void LCD_WriteIndex(u16 index)
{
	*(__IO uint16_t *) (Bank1_LCD_R)= index;	
}

/*************************************************
函数名：LCD_WR_Reg
功能：对lcd寄存器，写命令
入口参数：寄存器地址和命令
返回值：寄存器值
*************************************************/
static void LCD_WriteReg(u16 index,u16 val)
{	
	*(__IO uint16_t *) (Bank1_LCD_R)= index;	
	*(__IO uint16_t *) (Bank1_LCD_D)= val;	
}

/*************************************************
函数名：LCD_RD_Reg
功能：对lcd寄存器，读值
入口参数：寄存器地址
返回值：寄存器值
*************************************************/
static u16 LCD_ReadReg(u16 index)
{	
	*(__IO uint16_t *) (Bank1_LCD_R)= index;	
	return (*(__IO uint16_t *) (Bank1_LCD_D));
}

/*************************************************
函数名：LCD_WR_Data
功能：对lcd写数据
入口参数：数据值
返回值：无
*************************************************/
static void LCD_WriteData(u16 val)
{   
	*(__IO uint16_t *) (Bank1_LCD_D)= val; 	
}

/*************************************************
函数名：LCD_RD_Data
功能：读lcd数据
入口参数：无
返回值：数据
*************************************************/
static u16 LCD_ReadData(void)
{
	return(*(__IO uint16_t *) (Bank1_LCD_D));	
}

/*************************************************
函数名：LCD_Power_On
功能：LCD驱动序列
入口参数：无
返回值：无
*************************************************/
static void LCD_PowerOn(void)
{
	LCD_Reset();	 

#if 1
	LCD_DelayMs(50);                     //根据不同晶振速度可以调整延时，保障稳定显示
	LCD_WriteReg(0x00e5, 0x8000);	
	LCD_WriteReg(0x0000, 0x0001);	
	LCD_WriteReg(0x0001, 0x0100);
	LCD_WriteReg(0x0002, 0x0700);
	LCD_WriteReg(0x0003, 0x1030);
	LCD_WriteReg(0x0004, 0x0000);
	LCD_WriteReg(0x0008, 0x0202);	
	LCD_WriteReg(0x0009, 0x0000);	
	LCD_WriteReg(0x000A, 0x0000);
	LCD_WriteReg(0x000C, 0x0000);	
	LCD_WriteReg(0x000D, 0x0000);	
	LCD_WriteReg(0x000F, 0x0000);	
	//-----Power On sequence-----------------------	
	LCD_WriteReg(0x0010, 0x0000);	
	LCD_WriteReg(0x0011, 0x0007);	
	LCD_WriteReg(0x0012, 0x0000);	
	LCD_WriteReg(0x0013, 0x0000);	
	LCD_DelayMs(150);
	LCD_WriteReg(0x0010, 0x17B0);	
	LCD_WriteReg(0x0011, 0x0007);	
	LCD_DelayMs(50);
	LCD_WriteReg(0x0012, 0x013A);	
	LCD_DelayMs(50);
	LCD_WriteReg(0x0013, 0x1A00);	
	LCD_WriteReg(0x0029, 0x000c);		
	LCD_DelayMs(50);
    //-----Gamma control-----------------------	
	LCD_WriteReg(0x0030, 0x0000);	
	LCD_WriteReg(0x0031, 0x0505);	
	LCD_WriteReg(0x0032, 0x0004);	
	LCD_WriteReg(0x0035, 0x0006);	
	LCD_WriteReg(0x0036, 0x0707);	
	LCD_WriteReg(0x0037, 0x0105);	
	LCD_WriteReg(0x0038, 0x0002);	
	LCD_WriteReg(0x0039, 0x0707);	
	LCD_WriteReg(0x003C, 0x0704);	
	LCD_WriteReg(0x003D, 0x0807);	
    //-----Set RAM area-----------------------	
	LCD_WriteReg(0x0050, 0x0000);
	LCD_WriteReg(0x0051, 0x00EF);
	LCD_WriteReg(0x0052, 0x0000);
	LCD_WriteReg(0x0053, 0x013F);
	LCD_WriteReg(0x0060, 0x2700);
	LCD_WriteReg(0x0061, 0x0001);
	LCD_WriteReg(0x006A, 0x0000);
	LCD_WriteReg(0x0021, 0x0000);	
	LCD_WriteReg(0x0020, 0x0000);	
    //-----Partial Display Control------------	
	LCD_WriteReg(0x0080, 0x0000);	
	LCD_WriteReg(0x0081, 0x0000);
	LCD_WriteReg(0x0082, 0x0000);
	LCD_WriteReg(0x0083, 0x0000);
	LCD_WriteReg(0x0084, 0x0000);	
	LCD_WriteReg(0x0085, 0x0000);
    //-----Panel Control----------------------
	LCD_WriteReg(0x0090, 0x0010);	
	LCD_WriteReg(0x0092, 0x0000);
	LCD_WriteReg(0x0093, 0x0003);
	LCD_WriteReg(0x0095, 0x0110);
	LCD_WriteReg(0x0097, 0x0000);	
	LCD_WriteReg(0x0098, 0x0000);
    //-----Display on-----------------------	
	LCD_WriteReg(0x0007, 0x0133);	
	LCD_DelayMs(50);
#else
  /* Start Init sequence */
  LCD_WriteReg(0x00E7,0x1014);
//  DeviceCode = ili9331_ReadRegister(0x00E7);
  LCD_WriteReg(0x0001,0x0100); //Set ss(1)和sm(0),S720 to S1 
  LCD_WriteReg(0X0002,0x0200); //set 1 line inversion ?
  LCD_WriteReg(0x0003,0x0030); //设置GRAM写方向,bgr=1(SWAT RGB TO BGR)// 65K像素 
  LCD_WriteReg(0x0008,0x0202); //display control2设置前后porch 2line
  LCD_WriteReg(0x0009,0x0000); //display control3 (set nodisplay area cycle)
  LCD_WriteReg(0x000A,0X0000); //display control4 FMARK function
  LCD_WriteReg(0x000C,0X0001); //RGB interface 16bit 静态的图片，动态的需要设置为dm和rm
  LCD_WriteReg(0x000D,0X0000); //Frame marker
  LCD_WriteReg(0x000F,0X0000); //RGB interface polarity
  
  /* power on sequence*/
  LCD_WriteReg(0x0010,0x0000); //slp,stb,
  LCD_WriteReg(0x0011,0x0007);
  LCD_WriteReg(0x0012,0x0000);
  LCD_WriteReg(0x0013,0x0000);
  LCD_DelayMs(10);
  LCD_WriteReg(0x0010,0x1690); //slp,stb,
  LCD_WriteReg(0x0011,0x0227);
  LCD_DelayMs(10);
  LCD_WriteReg(0x0012,0x000C);
  LCD_DelayMs(10);
  LCD_WriteReg(0x0013,0x0800);
  
  LCD_WriteReg(0x0029,0x0011); //slp,stb,
  LCD_WriteReg(0x002B,0x000B); //fram rate
  LCD_DelayMs(10);
  LCD_WriteReg(0x0020,0x0000);
  LCD_WriteReg(0x0021,0x0000);
  /* adjust the gamma curve */
  LCD_WriteReg(0x0030,0x0000);
  LCD_WriteReg(0x0031,0x0106);
  LCD_WriteReg(0x0032,0x0000);

  LCD_WriteReg(0x0035,0x0204);

  LCD_WriteReg(0x0036,0x160A);
  LCD_WriteReg(0x0037,0x0707);
  LCD_WriteReg(0x0038,0x0106);
  LCD_WriteReg(0x0039,0x0707);
  LCD_WriteReg(0x003C,0x0402);
  LCD_WriteReg(0x003D,0x0C0F);
  /* set gram area */
  LCD_WriteReg(0x0050,0x0000); //Horizontal gram start addr
  LCD_WriteReg(0x0050,0x00EF); //Horizontal gram end   addr  
  LCD_WriteReg(0x0051,0x0000); //vertical gram start addr
  LCD_WriteReg(0x0052,0x013F); //vertical gram end   addt
  LCD_WriteReg(0x0060,0x2700); //gate scan
  LCD_WriteReg(0x0061,0x0001); //ndl vle rev
  LCD_WriteReg(0x006A,0x0000); //scrolling line
  /*partial dispaly */
  LCD_WriteReg(0x0080,0x0000);
  LCD_WriteReg(0x0081,0x0000);
  LCD_WriteReg(0x0082,0x0000);
  LCD_WriteReg(0x0083,0x0000);
  LCD_WriteReg(0x0084,0x0000);
  LCD_WriteReg(0x0085,0x0000);
  /* panel control */
  LCD_WriteReg(0x0090,0x0010);
  LCD_WriteReg(0x0092,0x0600);
  
  LCD_WriteReg(0x0007,0x0133);  //262 or8  Color and display on
#endif
}

/*************************************************
函数名：LCD_Power_Off
功能：LCD关闭序列
入口参数：无
返回值：无
*************************************************/
static void LCD_PowerOff(void)
{
	return;
}

/*************************************************
函数名：LCD_WR_Data_Start
功能：LCD开始批量传数据前调用
入口参数：无
返回值：无
*************************************************/
void LCD_BlukWriteDataStart(void)
{	
    LCD_WriteIndex(GRAMWR);
}

/*************************************************
函数名：LCD_ReadDataStart
功能：LCD开始批量传数据前调用
入口参数：无
返回值：无
*************************************************/
void LCD_BulkReadDataStart(void)
{	
    LCD_WriteIndex(GRAMWR);
}

/*************************************************
函数名：LCD_BulkWriteData
功能：对lcd批量写数据
入口参数：数据值
返回值：无
*************************************************/
void LCD_BulkWriteData(u16 val)
{   
	*(__IO uint16_t *) (Bank1_LCD_D)= val; 	
}

/*************************************************
函数名：LCD_BulkReadData
功能：批量读lcd数据
入口参数：无
返回值：数据
*************************************************/
u16 LCD_BulkReadData(void)
{
  register u16 Data;
  LCD_ReadData();//丢掉无用字节
  Data=LCD_ReadData();
#if QXW_LCM_ID == 221
  Data=((((Data>>11)&0x001f)|(Data&0x07e0)|((Data<<11)&0xf800)));//RGB换序
#endif 
  LCD_WriteData(Data);
  return Data;
}

/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void LCD_SetXY(u16 x,u16 y)
{
  LCD_WriteReg(GRAM_XADDR,x);
  LCD_WriteReg(GRAM_YADDR,y);
  //LCD_WriteIndex(GRAMWR);
}

/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点,Y_IncMode表示先自增y再自增x
返回值：无
*************************************************/
void LCD_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end,bool yIncFrist)
{		
	LCD_WriteReg(WINDOW_XADDR_START,x_start);
	LCD_WriteReg(WINDOW_XADDR_END,x_end);
	LCD_WriteReg(WINDOW_YADDR_START,y_start);
	LCD_WriteReg(WINDOW_YADDR_END,y_end);
  
  	{
		u16 ModeReg=LCD_ReadReg(0x0003);

		if(yIncFrist)
			ModeReg|=(0x8);
		else
			ModeReg&=(~0x8);
		LCD_WriteReg(0x0003, ModeReg);
	}
}

/*************************************************
函数名：LCD_Set_XY_Addr_Direction
功能：设置lcd读或写自增的方向
入口参数：0:由0到高，1:由高到0
返回值：无
*************************************************/
void LCD_SetAddrIncMode(LCD_INC_MODE xyDirection)
{
	register u16 ModeReg=LCD_ReadReg(0x0003);

	ModeReg&=(~0x30);
	ModeReg|=((xyDirection)<<4);
	LCD_WriteReg(0x0003, ModeReg);
}

/*************************************************
函数名：LCD_BGR_Mode
功能：设置lcd RGB顺序
入口参数：0:RGB   1:BGR
返回值：无
*************************************************/
void LCD_BgrMode(bool UseBGR)
{
	register u16 ModeReg=LCD_ReadReg(0x0003);

	if(UseBGR)
		ModeReg&=(~0x1000);
	else
		ModeReg|=(0x1000);
	LCD_WriteReg(0x0003, ModeReg);
}

/*************************************************
函数名：LCD_Addr_Inc
功能：地址自增
入口参数：无
返回值：无
*************************************************/
void LCD_AddrInc(void)
{
	register u16 Color16;
	LCD_ReadData();
	Color16=LCD_ReadData();
	LCD_WriteData((((Color16>>11)&0x001f)|(Color16&0x07e0)|((Color16<<11)&0xf800)));//将16位RGB(565)色彩换算成16位BGR(565)色彩
}

/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 Data)
{
  LCD_WriteReg(GRAM_XADDR,x);
  LCD_WriteReg(GRAM_YADDR,y);
  LCD_WriteIndex(GRAMWR);
  LCD_WriteData(Data);
}

/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
u16 LCD_ReadPoint(u16 x,u16 y)
{
  register u16 Data;
  LCD_WriteReg(GRAM_XADDR,x);
  LCD_WriteReg(GRAM_YADDR,y);
  LCD_WriteIndex(GRAMWR);
  LCD_ReadData();//丢掉无用字节
  Data=LCD_ReadData();
#if QXW_LCM_ID == 221
  Data=((((Data>>11)&0x001f)|(Data&0x07e0)|((Data<<11)&0xf800)));//RGB换序
#endif 
  LCD_WriteData(Data);
  return Data;
}

/*************************************************
函数名：LCD_Light_Set
功能：LCD设置背光亮度
入口参数：Scale:0-100，0为熄灭，100最亮
返回值：无
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
函数名：LCD_Light_State
功能:查询当前背光亮度
入口参数：无
返回值：0-100，0为熄灭，100最亮
*************************************************/
u8 LCD_Light_State(void)
{
	return gLcdScale;
}

/**********************************************
函数名：FSMC_LCD_Init
功能：用于FSMC配置
入口参数：无
返回值：无
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

  /* Enable FSMC Bank1_SRAM Bank */
  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
}

/**********************************************
函数名：LCD_Configuration
功能：用于LCD的IO配置
入口参数：延时数
返回值：无
***********************************************/
static void LCD_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD |RCC_APB2Periph_GPIOE, ENABLE); 	

  //驱动ic供电控制
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //LCD Reset
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

#if LCD_BGLIGHT_PWM_MODE //LCD 背光控制,	  
  /*配置PC6 PWM*/
  //pc6属于tim3 ch1的remap
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
  GPIO_PinRemapConfig(GPIO_FullRemap_TIM3,ENABLE); //TIM3 完全重映射 
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

  //FSMC的GPIOD管脚
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

  //FSMC的GPIOE管脚
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
函数名：LCD_Init
功能：初始化启动lcd
入口参数：无
返回值：无
*************************************************/
void LCD_Init(void)
{	
	LCD_FSMC_Init();
	LCD_DelayMs(100);	 
	LCD_Configuration();
	LCD_ILI9320_On();
	LCD_DelayMs(100);
	LCD_Light_Set(100);
	LCD_PowerOn();
	LCD_BgrMode(FALSE);
	Debug("LCD Driver IC:%x\n\r",LCD_ReadReg(0));
}

void LCD_DeInit(void)
{
	LCD_PowerOff();
}

/**********************************************
函数名：LCD_Reset
功能：LCD复位
入口参数：延时数
返回值：无
***********************************************/ 
void LCD_Reset(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_13);
    LCD_DelayMs(50);					   
    GPIO_SetBits(GPIOD, GPIO_Pin_13);		 	 
	LCD_DelayMs(50);	
}

#endif


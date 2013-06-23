#ifndef __VS1003B_H__
#define __VS1003B_H__  

#define TCS   (1<<1)  // PA1 
#define RST   (1<<4)  // PC4     			    
#define XDCS   (1<<2)  // PA2
#define VS_CS_SET(x)  GPIOA->ODR=(GPIOA->ODR&~TCS)|(x ? TCS:0)  
#define VS_XRST_SET(x)  GPIOC->ODR=(GPIOC->ODR&~RST)|(x ? RST:0)  
#define VS_XDCS_SET(x)  GPIOA->ODR=(GPIOA->ODR&~XDCS)|(x ? XDCS:0)  

#define VS_DREQ  (GPIOA->IDR & GPIO_Pin_4)

#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03		 		 

//寄存器定义
#define REG_MODE        	0x00   //模式控制
#define REG_STATUS      	0x01   //状态寄存器
#define REG_BASS        	0x02   //重音高音设置
#define REG_CLOCKF      	0x03   //时钟设置
#define REG_DECODE_TIME 	0x04   // 1秒之内解码次数
#define REG_AUDATA      	0x05   // 15-1bit采样率，0bit通道数
#define REG_WRAM        	0x06   //上载程序和指令到ram
#define REG_WRAMADDR    	0x07   //在REG_WRAM之前使用
#define REG_HDAT0       	0x08   //文件信息
#define REG_HDAT1       	0x09   //文件信息
#define REG_AIADDR      	0x0a   //指定应用代码开始地址
#define REG_VOL         	0x0b   //音量控制
#define REG_AICTRL0     	0x0c   //
#define REG_AICTRL1     	0x0d   //
#define REG_AICTRL2     	0x0e   //
#define REG_AICTRL3     	0x0f   //

//Mode寄存器位定义
#define SM_DIFF         	0x01   //0正常相位，1左声道反转
#define SM_JUMP         	0x02   //无意义
#define SM_RESET        	0x04   //置位时复位，复位后自动清零
#define SM_OUTOFWAV     	0x08   //置位时停止wav解码
#define SM_PDOWN        	0x10   // 置位时进入软件省电模式
#define SM_TESTS        	0x20   //置位时进入SDI测试模式
#define SM_STREAM       	0x40   //使能vs103的流模式
#define SM_PLUSV        	0x80   //MP3+V解码使能
#define SM_DACT         	0x100   // 控制sck上升沿或下降沿有效
#define SM_SDIORD       	0x200   //控制sdi总线字节顺序
#define SM_SDISHARE     	0x400   //置位时，xDCS可省去，只需xCS
#define SM_SDINEW       	0x800   //置位进入新模式
#define SM_ADPCM        	0x1000   //置位时ADPCM录音使能
#define SM_ADPCM_HP     	0x2000 //置位时使能ADPCM高通滤波器
#define SM_LINE_IN	0x4000		// 1选择line in，0选择麦克风

//Gpio寄存器描述
//0xC017 rw 0 DDR[3:0] 设定io口方向，1是输出，0是输入
//0xC018 r 0 IDATA[3:0] 从io读出的值
//0xC019 rw 0 ODATA[3:0] 设定io值，会锁存 
#define VS_GPIO_DDR  0xC017 
#define VS_GPIO_IData  0xC018 
#define VS_GPIO_OData  0xC019 

#define VS_Write_Byte(Data) SPI_WriteByte(SPI1,Data)
#define VS_Read_Byte() SPI_ReadByte(SPI1)

//得到mp3&wma的波特率
u16 VsGetHeadInfo(void);  

//重设解码时间                          
void VsResetDecodeTime(void);

//得到mp3的播放时间n sec
u16 VsGetDecodeTime(void); 

//加载频谱分析的代码到VS1003
void VsLoadPatch(void);

//得到频谱数据
void VsGetSpec(u16 *p);

//设置io方向
void  VsGpioDDR(u8 ucDDRValue) ; 

//设置io电平
void VsGpioWriteData(u8 ucWriteValue) ; 

//读取io电平
u8 VsGpioReadData(void) ; 

//设置音量
void VsSetVolume(u8  left,u8 right) ;

//软复位
void VsSoftReset(void);

//led 控制
void VsLedSet(bool LedIO2,bool LedIO3);

//启动vs
void VsStart(void);

void VsStop(void);

void VsInit(void);





#endif


/*
 * FileName:       
 * Author:         YuanYin  Version:Summer QXW-V1.x  Date: 2010-3-11
 * Description:    Q-ShareWe CC2500 Drivers
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */	
 
/*include head*/
#include "Drivers.h"
#include "Spi.h"
#include "CC2500.h"
#include "Debug.h"

u8 bTempBuf[PACKET_LEN];

/**
// PA value
**/
const u8 paTable_CC2500[8]={0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe};  //PA value


void Delay_us(u32 nCount)
{
	u32 i;
	
  	for(; nCount != 0; nCount--)
  		for(i=12; i != 0; i--);
}

void Delay_ms(u32 nCount)
{
	u32 i;
	
	for(; nCount != 0; nCount--)
  		for(i=12000; i != 0; i--);
}

//send one u8
void CC2500_SendSpiData(u8 value)
{
	CC2500_Select();
	CC2500_WriteByte(value);
	CC2500_NoSelect();
}

//reset cc2500
void CC2500_Reset(void)
{
	CC2500_SendSpiData(CCxxx0_SIDLE);

	Delay_ms(20);

	CC2500_Select();

	Delay_ms(20);

	CC2500_NoSelect();

	Delay_ms(40);

	CC2500_SendSpiData(CCxxx0_SRES);
}

//wirte cc2500 register
void CC2500_WriteReg(u8 addr, u8 value)
{
	CC2500_Select();
	addr &= 0x7F;
	CC2500_WriteByte(addr);
	CC2500_WriteByte(value);
	CC2500_NoSelect();

#if 1 //test reg value
	if(CC2500_ReadReg(addr)!=value)
	{
		Debug("The Reg0x%x value is error!(write=0x%x,read=0x%x)\n\r",addr,value,CC2500_ReadReg(addr));
		while(1);
	}
#endif
}

// read cc2500 register
u8 CC2500_ReadReg(u8 addr)
{
	u8 value;

	CC2500_Select();
	CC2500_WriteByte(addr|0x80);
	value = CC2500_ReadByte();
    CC2500_NoSelect();
	return value;
}

// wirte setting register to cc2500
void CC2500_RfSettings(void)
{
	//u8 i;
	
#if 1
		CC2500_WriteReg(CCxxx0_FSCTRL1,	0x09);	// 0x0c
		CC2500_WriteReg(CCxxx0_FSCTRL0,	0x00);
		CC2500_WriteReg(CCxxx0_FREQ2,	0x5D);
		CC2500_WriteReg(CCxxx0_FREQ1,	0x93);
		CC2500_WriteReg(CCxxx0_FREQ0,	0xB1);
//#define _10k
#ifdef _10k
		CC2500_WriteReg(CCxxx0_MDMCFG4,	0x78); //0x78-10k   0x86
		CC2500_WriteReg(CCxxx0_MDMCFG3,	0x93); //0x93-10k   0x83
//		CC2500_WriteReg(CCxxx0_MDMCFG2,	0x70);
		CC2500_WriteReg(CCxxx0_MDMCFG2,	0x03); //0x03-10k  0x03
		CC2500_WriteReg(CCxxx0_MDMCFG1,	0x22); //0x22-10k	 0x22							// 0x22????
		CC2500_WriteReg(CCxxx0_MDMCFG0,	0xF8); //0xf8        0xf8		

#else
		CC2500_WriteReg(CCxxx0_MDMCFG4,	0x2D); //0x78-10k   0x86
		CC2500_WriteReg(CCxxx0_MDMCFG3,	0x3B); //0x93-10k   0x83
//		CC2500_WriteReg(CCxxx0_MDMCFG2,	0x70);
		CC2500_WriteReg(CCxxx0_MDMCFG2,	0x73); //0x03-10k  0x03
		CC2500_WriteReg(CCxxx0_MDMCFG1,	0xA2); //0x22-10k	 0x22							// 0x22????
		CC2500_WriteReg(CCxxx0_MDMCFG0,	0xF8); //0xf8        0xf8
#endif

		CC2500_WriteReg(CCxxx0_CHANNR,	0x00);
		CC2500_WriteReg(CCxxx0_DEVIATN,	0x01);	//0x44							// 00
		CC2500_WriteReg(CCxxx0_FREND1,	0x56);								// 0xB6   56错误机率减少
		CC2500_WriteReg(CCxxx0_FREND0,	0x10);
		CC2500_WriteReg(CCxxx0_MCSM1,	0x00);								// 0X00>NO CCA; 0X30>CCA   ?????
		CC2500_WriteReg(CCxxx0_MCSM0,	0x18);
		CC2500_WriteReg(CCxxx0_FOCCFG,	0x15);								// 0x1D   频率偏移补偿
		CC2500_WriteReg(CCxxx0_BSCFG,	0x6C);								// 0x1c   位同步配置
		CC2500_WriteReg(CCxxx0_AGCCTRL2,	0x07);								// 0xc3    增益控制
		CC2500_WriteReg(CCxxx0_AGCCTRL1,	0x00);								// cca=0x10
		CC2500_WriteReg(CCxxx0_AGCCTRL0,	0x91);								// 0xb2
		CC2500_WriteReg(CCxxx0_FSCAL3,	0xEA);
		CC2500_WriteReg(CCxxx0_FSCAL2,	0x0A);  //0x0a - 250k  0x06 --10k,0x08-2.4k
		CC2500_WriteReg(CCxxx0_FSCAL1,	0x00);								// 增加频率同步校准

		CC2500_WriteReg(CCxxx0_FSCAL0,	0x11);
		CC2500_WriteReg(CCxxx0_FSTEST,	0x59);
		CC2500_WriteReg(CCxxx0_TEST2,	0x8F);								// 0x88
		CC2500_WriteReg(CCxxx0_TEST1,	0x21);								// 0x31
		CC2500_WriteReg(CCxxx0_TEST0,	0x0B);
		CC2500_WriteReg(CCxxx0_IOCFG2,	0x06);	//29							// CCA=0x09:1=free   ??????
		CC2500_WriteReg(CCxxx0_IOCFG0,	0x06);								//                  ?????
		CC2500_WriteReg(CCxxx0_PKTCTRL1,	0x00);	//无地址检查		//                      ??????
		CC2500_WriteReg(CCxxx0_PKTCTRL0,	0x04);	//0x41 whiter+val	 //crc on 05						//                          ????
		CC2500_WriteReg(CCxxx0_ADDR,		0x00);								//                     ?????
		CC2500_WriteReg(CCxxx0_PKTLEN,	PACKET_LEN);//                      ??????
      // CC2500_ReadReg(CCxxx0_MDMCFG1);
#else
	CC2500_WriteReg(CCxxx0_IOCFG2,0x0B);//0x0B,   // IOCFG2    GDO2 output pin configuration.
	CC2500_WriteReg(CCxxx0_IOCFG1,0x06);   //IOCFG1
	CC2500_WriteReg(CCxxx0_IOCFG0,0x06);  // IOCFG0   GDO0 output pin configuration. 
	CC2500_WriteReg(CCxxx0_FIFOTHR,0x07);   //FIFOTHR
	CC2500_WriteReg(CCxxx0_SYNC1,0xD3);	//SYNC1 MSB
	CC2500_WriteReg(CCxxx0_SYNC0,0x91);     //SYNC0 LSB
	CC2500_WriteReg(CCxxx0_PKTLEN,64);   // PKTLEN    Packet length.
	CC2500_WriteReg(CCxxx0_PKTCTRL1,0x02);   // PKTCTRL1  Packet automation control.
	CC2500_WriteReg(CCxxx0_PKTCTRL0,0x05);  // PKTCTRL0  Packet automation control.
	CC2500_WriteReg(CCxxx0_ADDR,0x01);  // ADDR      Device address.
	
	CC2500_WriteReg(CCxxx0_CHANNR,0x00);  // CHANNR    Channel number.
	CC2500_WriteReg(CCxxx0_FSCTRL1,0x07);  // FSCTRL1   Frequency synthesizer control.
	CC2500_WriteReg(CCxxx0_FSCTRL0,0x00);  // FSCTRL0   Frequency synthesizer control.
	CC2500_WriteReg(CCxxx0_FREQ2,0x5D);  // FREQ2     Frequency control word, high u8.
	CC2500_WriteReg(CCxxx0_FREQ1,0x93);  // FREQ1     Frequency control word, middle u8.
	CC2500_WriteReg(CCxxx0_FREQ0,0xB1);  // FREQ0     Frequency control word, low u8.
	CC2500_WriteReg(CCxxx0_MDMCFG4,0x2D);  // MDMCFG4   Modem configuration.
	CC2500_WriteReg(CCxxx0_MDMCFG3,0x3B);  // MDMCFG3   Modem configuration.
	CC2500_WriteReg(CCxxx0_MDMCFG2,0x73);  // MDMCFG2   Modem configuration.
	CC2500_WriteReg(CCxxx0_MDMCFG1,0x22);  // MDMCFG1   Modem configuration.

	CC2500_WriteReg(CCxxx0_MDMCFG0,0xF8);  // MDMCFG0   Modem configuration.
	CC2500_WriteReg(CCxxx0_DEVIATN,0x47);  // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
	CC2500_WriteReg(CCxxx0_MCSM2,0x00);   // MCSM2     Main Radio Control State Machine configuration.
	CC2500_WriteReg(CCxxx0_MCSM1,0x02);   // MCSM1     Main Radio Control State Machine configuration.
	CC2500_WriteReg(CCxxx0_MCSM0,0x18);   // MCSM0     Main Radio Control State Machine configuration.
	CC2500_WriteReg(CCxxx0_FOCCFG,0x1D);  // FOCCFG    Frequency Offset Compensation Configuration.
	CC2500_WriteReg(CCxxx0_BSCFG,0x1C);  // BSCFG     Bit synchronization Configuration.
	CC2500_WriteReg(CCxxx0_AGCCTRL2,0xC7);  // AGCCTRL2  AGC control.
	CC2500_WriteReg(CCxxx0_AGCCTRL1,0x00);  // AGCCTRL1  AGC control.
	CC2500_WriteReg(CCxxx0_AGCCTRL0,0xB2);  // AGCCTRL0  AGC control.

	CC2500_WriteReg(CCxxx0_WOREVT1,0x00);  // WOREVT1
    CC2500_WriteReg(CCxxx0_WOREVT0,0x00);  // WOREVT0
    CC2500_WriteReg(CCxxx0_WORCTRL,0x00);  // WORCTRL
    CC2500_WriteReg(CCxxx0_FREND1,0xB6);  // FREND1    Front end RX configuration.
	CC2500_WriteReg(CCxxx0_FREND0,0x10);  // FREND0    Front end RX configuration.
	CC2500_WriteReg(CCxxx0_FSCAL3,0xEA);  // FSCAL3    Frequency synthesizer calibration.
	CC2500_WriteReg(CCxxx0_FSCAL2,0x0A);  // FSCAL2    Frequency synthesizer calibration.
	CC2500_WriteReg(CCxxx0_FSCAL1,0x00);  // FSCAL1    Frequency synthesizer calibration.
	CC2500_WriteReg(CCxxx0_FSCAL0,0x11);  // FSCAL0    Frequency synthesizer calibration.
	CC2500_WriteReg(CCxxx0_RCCTRL1,0x00);   //RCCTRL1

	CC2500_WriteReg(CCxxx0_RCCTRL0,0x00);   //RCCTRL0
	CC2500_WriteReg(CCxxx0_FSTEST,0x59);  // FSTEST    Frequency synthesizer calibration.
	CC2500_WriteReg(CCxxx0_PTEST,0x00);   //PTEST
	//CC2500_WriteReg(CCxxx0_AGCTEST,0x00);  //AGCTEST
	//CC2500_WriteReg(CCxxx0_TEST2,0x88);  // TEST2     Various test settings.
	//CC2500_WriteReg(CCxxx0_TEST1,0x31);  // TEST1     Various test settings.
	//CC2500_WriteReg(CCxxx0_TEST0,0x0B);  // TEST0     Various test settings.
#endif

	//for(i=0;i<=CCxxx0_TEST0;i++)
		//Debug("0x%x:0x%x\n\r",i,CC2500_ReadReg(i));
}

// read chip id 
u16 CC2500_GetChipId(void)
{
	u16 id = 0;

	CC2500_Select();
	CC2500_WriteByte(CCxxx0_PARTNUM|0xc0);
	id = CC2500_ReadByte();
	id=(id<<8)&0xff00;
	CC2500_WriteByte(CCxxx0_VERSION|0xc0);
	id |= CC2500_ReadByte();
	CC2500_NoSelect();
    return id;
}

// set cc2500 to RX mode
void CC2500_SetRxd(void)
{
	CC2500_SendSpiData(CCxxx0_SIDLE);
	CC2500_SendSpiData(CCxxx0_SFRX);
	CC2500_SendSpiData(CCxxx0_SRX);
}

void CC2500_SetRecvAddr(CC2500_RECV_ADDR AddrMode,u8 SpecAddr)
{
	CC2500_WriteReg(CCxxx0_ADDR,SpecAddr);	
	CC2500_WriteReg(CCxxx0_PKTCTRL1,AddrMode);//地址检查寄存器
	
	CC2500_SetRxd();
}

// setup cc2500
// id不对返回false
bool CC2500_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;  
	u8 i = 0;
	u16 Id;
	
	//CS
	GPIO_InitStructure.GPIO_Pin = CC_CS_IO_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(CC_CS_IO_GROUP, &GPIO_InitStructure);
	
	SPI2_Init();
	
	CC2500_Reset();
	if((Id=CC2500_GetChipId())!=0x8003)
    {
		Debug("Error CC2500 ID: %04x\n\r",Id);
		return FALSE;
    }
	
	CC2500_RfSettings();

	CC2500_Select();
	CC2500_WriteByte(CCxxx0_PATABLE|WRITE_BURST);
	for(i=0;i<8;i++)
	{
		CC2500_WriteByte(paTable_CC2500[i]);
	}
	CC2500_NoSelect();

	CC2500_SetRxd();

	//GDO0
	GPIO_InitStructure.GPIO_Pin = CC_GDO0_IO_PIN;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(CC_GDO0_IO_GROUP, &GPIO_InitStructure);

#if QXW_LCM_ID == 210 || QXW_LCM_ID == 211 || QXW_LCM_ID == 220 || QXW_LCM_ID == 221
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI3_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;             //外部中断线 ，使用第2根
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//中断触发方式
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //打开中断
	EXTI_Init(&EXTI_InitStructure);    //调用库函数给寄存器复制

	CC2500_InterruptEnable(FALSE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource3); 
#elif QXW_LCM_ID == 212
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EXTI9_5_IRQn_Priority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;             //外部中断线 ，使用第2根
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//中断触发方式
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //打开中断
	EXTI_Init(&EXTI_InitStructure);    //调用库函数给寄存器复制

	CC2500_InterruptEnable(FALSE);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource6); 
#endif
	return TRUE;
}

void CC2500_InterruptEnable(bool Enable)
{
#if QXW_LCM_ID == 210 || QXW_LCM_ID == 211 || QXW_LCM_ID == 220 || QXW_LCM_ID == 221
	if(Enable)
		EXTI->IMR |= EXTI_Line3;//开启中断
	else
		EXTI->IMR &=~EXTI_Line3;//关闭中断
#elif QXW_LCM_ID == 212
	if(Enable)
		EXTI->IMR |= EXTI_Line6;//开启中断
	else
		EXTI->IMR &=~EXTI_Line6;//关闭中断
#endif
}

//  send paket data to cc2500
u8 CC2500_SendPacket(u8 *txBuffer, u8 size)
{ 
	u8 i =0;
	
	CC2500_SendSpiData(CCxxx0_SIDLE);
	CC2500_SendSpiData(CCxxx0_SFTX);//SFRX SFTX只用于IDLE模式 执行后将FIFOs里上溢或下溢的数据清空

	CC2500_Select();
	CC2500_WriteByte(CCxxx0_TXFIFO_Muti);
    for(i=0;i<size;i++)
    {
    	CC2500_WriteByte(txBuffer[i]);
	}
	CC2500_NoSelect();

	CC2500_SendSpiData(CCxxx0_STX);
	while(!GDO0_IN());   //wait high
	while(GDO0_IN());    //wait low ,end send 
	
	//Delay_ms(10);
    return 0;
}

// read state
u8 CC2500_ReadStatus(u8 addr)
{
	u8  value;

	CC2500_Select();
	addr  |= READ_BURST;

	CC2500_WriteByte(addr);
	value = CC2500_ReadByte();
	
	CC2500_NoSelect();

	return value;
}

// receive a paket data form cc2500
u8 CC2500_ReceivePacket(u8 *rxBuffer)
{
    u8 status;
    u8 LEN=0,i=0;

    status =CC2500_ReadStatus(CCxxx0_RXBYTES);          //RX FIFO 等于0 CRC校验失败 如果大于0CRC校验OK数据能读出
    if((status & 0x7F)==0x00)
    {
        status = CC2500_ReadStatus(CCxxx0_MARCSTATE);
        if((status!=0x0D)&&(status!=0x08))    //0x0D为RX状态
        {
        	CC2500_SetRxd();				//激活（接收或传送）
        }
        return 0;
    }
	
	//LEN = CC2500_ReadReg(CCxxx0_RXFIFO_one);
    LEN = PACKET_LEN;   //采用定长模式
    if(LEN > 0)
    {
		if(LEN<=RFTXDBUFFSIZE)
		{
			CC2500_Select();
			CC2500_WriteByte(CCxxx0_RXFIFO_Muti);
			for(i=0;i<LEN;i++)
			{
				rxBuffer[i] = CC2500_ReadByte();
			}
			CC2500_NoSelect(); 
			CC2500_SetRxd();
			
			return LEN;//(status & CRC_OK);
		}
		else
		{
			CC2500_Select();
			CC2500_WriteByte(CCxxx0_RXFIFO_Muti);
			for(i=0;i<LEN;i++)
			{
				rxBuffer[i] = CC2500_ReadByte();
			}
			CC2500_NoSelect();				
			CC2500_SetRxd();			 
			
			return 0;
		}
    }
    else
    {
        CC2500_SetRxd();
        return 0;
    }
}

// enter sleep mode,cc2500 strobe SPWD must in idle mode
void CC2500_EnterSleep(void)
{
	CC2500_SendSpiData(CCxxx0_SIDLE);
	CC2500_SendSpiData(CCxxx0_SPWD);  
}

//read RSSI value
u8 CC2500_ReadRSSIValue(void)
{
	return(CC2500_ReadStatus(0x34));
}

// Initial register for WOR mode
void CC2500_Init_WOR(void)
{
    CC2500_SendSpiData(CCxxx0_SIDLE);
    CC2500_WriteReg(CCxxx0_WORCTRL, 0x08); // 
    CC2500_WriteReg(CCxxx0_WOREVT1, 0x2b);//43B5 500ms, 28A0 300ms 1B15 200ms
    CC2500_WriteReg(CCxxx0_WOREVT0, 0x15);// 876B 1S
    CC2500_WriteReg(CCxxx0_MCSM2, 0x03);  // RX_TIME // RX_TIME_RSSI= 1  RX_TIME_QUAL = 0
    // 0:12.5%, 1:6.25%, 2:3.125%, 3:1.563%, 4:0.781%
    CC2500_WriteReg(CCxxx0_MCSM0, 0x38);  // 频率校准方式，每4次从IDLE模式到RX模式进行一次频率合成器校准
    CC2500_SendSpiData(CCxxx0_SWORRST);
    CC2500_SendSpiData(CCxxx0_SWOR);
}

// enter WOR mode
void CC2500_Enter_WOR(void)
{
  CC2500_Init_WOR(); 
  CC2500_SendSpiData(CCxxx0_SWORRST);
  CC2500_SendSpiData(CCxxx0_SWOR);
}

// RF go balk to WOR mode
void CC2500_REnter_WOR(void)
{
  CC2500_SendSpiData(CCxxx0_SWORRST);
  CC2500_SendSpiData(CCxxx0_SWOR);
}

// get connect
void CC2500_GetConnect(void)
{
	u8 i;

	i=0;
	for(i=0;i<sizeof(bTempBuf);i++)
	{
		bTempBuf[i] = 0x55;
	}
	CC2500_SendPacket(bTempBuf, PACKET_LEN);
}

// rf communication
void CC2500_Process(void)
{
	u8 i,j;

#define GDO0_
#ifdef GDO0_
	if(!GDO0_IN())
	{
		return;
	}

	while(GDO0_IN());
#else
    if(!(CC2500_ReadStatus(0x38)&0x01))
    {
    	return;
	}
	while((CC2500_ReadStatus(0x38)&0x01));
#endif

	Debug("Have a packet!\n\r");
    i = CC2500_ReadRSSIValue();

    for(i=0;i<sizeof(bTempBuf);i++) bTempBuf[i]=0;
    CC2500_ReceivePacket(bTempBuf);
	Debug("Receive over!\n\r");
	
    CC2500_SetRxd();
//	CC2500_REnter_WOR();

	
#if 1//def RF_DEBUG_
	Debug("\n\rRX Data:");
	for(j=0;j<sizeof(bTempBuf);j++)
	{	
		Debug("0x%x ",bTempBuf[j]);
	}
	Debug("\n\r");
#endif
}



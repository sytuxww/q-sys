/*
 * FileName:       
 * Author:         YuanYin  Version: QXW-V1.x  Date: 2010-3-11
 * Description:  M25P16 驱动程序
 * Version:        
 * Function List:  
 *                 1.
 * History:        
 *     <author>   <time>    <version >   <desc>
 */
#include "Drivers.h"
#include "Spi.h"
#include "SpiFlash.h"         

//片选
#define Select_Flash()     GPIO_ResetBits(GPIOC, GPIO_Pin_3)
#define NotSelect_Flash()    GPIO_SetBits(GPIOC, GPIO_Pin_3)

#if(QXW_PRODUCT_ID==114)
#define SPI_Flash_Read() SPI_ReadByte(SPI1)
#define	SPI_Flash_Write(x) SPI_WriteByte(SPI1,x)
#else
#define SPI_Flash_Read() SPI_ReadByte(SPI3)
#define	SPI_Flash_Write(x) SPI_WriteByte(SPI3,x)
#endif

void M25P16_Write_Enable(void)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_ENABLE);	
	NotSelect_Flash();
}

void M25P16_Write_Disable(void)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_DISABLE);	
	NotSelect_Flash();
}

void M25P16_Read_Id(u8 * id)
{
	u8 i;
	
	Select_Flash();	
	SPI_Flash_Write(READ_ID);	

	for(i=0;i<20;i++)
	{
		id[i] = SPI_Flash_Read();	
	}
	
	NotSelect_Flash();
}

u8 M25P16_Read_Status_Reg(void)
{
	u8 sta;
	
	Select_Flash();	
	SPI_Flash_Write(READ_STAUS_REG);	
 
	sta= SPI_Flash_Read();	
	
	NotSelect_Flash();
	
	return sta;
}

void M25P16_Write_Status_Reg(u8 reg)
{
	Select_Flash();	
	SPI_Flash_Write(WRITE_STAUS_REG);	
	SPI_Flash_Write(reg);
	NotSelect_Flash();
}

//读数据，自动翻页,addr为字节地址
void M25P16_Read_Data(u32 addr,u32 len,u8 *buf)
{
	u32 i;
	
	Select_Flash();	
	SPI_Flash_Write(READ_DATA);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	
	for(i=0;i<len;i++)
	{
		buf[i]=SPI_Flash_Read();
	}
	NotSelect_Flash();
}

//快速读数据
void M25P16_Fast_Read_Data(u32 addr,u32 len,u8 *buf)
{
	u32 i;

	Select_Flash();	
	SPI_Flash_Write(FAST_READ_DATA);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	SPI_Flash_Write(0);
	
	for(i=0;i<len;i++)
	{
		buf[i]=SPI_Flash_Read();
	}
	NotSelect_Flash();
}

//页编程函数，页编程前一定要进行页擦除!!!
void M25P16_Page_Program(u32 addr,u16 len,u8 *buf)
{
	u32 i;
	
	M25P16_Write_Enable();
	Select_Flash();	
	SPI_Flash_Write(PAGE_PROGRAM);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);

	for(i=0;i<len;i++)
		SPI_Flash_Write(buf[i]);

	NotSelect_Flash();

	while(M25P16_Read_Status_Reg()&0x01);	
}

void M25P16_Sector_Erase(u32 addr)
{
	M25P16_Write_Enable();

	Select_Flash();	
	SPI_Flash_Write(SECTOR_ERASE);	
	SPI_Flash_Write((addr>>16) & 0xff);
	SPI_Flash_Write((addr>>8) & 0xff);
	SPI_Flash_Write(addr & 0xff);
	NotSelect_Flash();

	while(M25P16_Read_Status_Reg()&0x01);
}

void M25P16_Bulk_Erase(void)
{
	M25P16_Write_Enable();
	
	Select_Flash();	
	SPI_Flash_Write(BULK_ERASE);	
	NotSelect_Flash();

	while(M25P16_Read_Status_Reg()&0x01);
}

void M25P16_Deep_Power_Down(void)
{
	u32 i;
	Select_Flash();	
	SPI_Flash_Write(DEEP_POWER_DOWN);	
	NotSelect_Flash();
	for(i=5000;i;i--);
}

u8 M25P16_Wake_Up(void)
{
	u8 res;
	u32 i;
	
	Select_Flash();	
	SPI_Flash_Write(WAKE_UP);	
	SPI_Flash_Write(0);	
	SPI_Flash_Write(0);	
	SPI_Flash_Write(0);	
	res=SPI_Flash_Read();
	
	NotSelect_Flash();
    for(i=5000;i;i--);
    
	return res;
}

void M25P16_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#if(QXW_PRODUCT_ID==114)
	SPI1_Init();
#else
	SPI3_Init();
#endif

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);//配置片选管脚pc3

	//不选flash
	NotSelect_Flash(); 
}


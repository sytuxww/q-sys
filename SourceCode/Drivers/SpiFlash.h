#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__  

//页大小和页数目
#define M25P16_Page_Size 256
#define M25P16_Page_Max 8192

//功能寄存器地址
#define WRITE_ENABLE 0X06
#define WRITE_DISABLE 0X04
#define READ_ID 0X9F
#define READ_STAUS_REG 0X05
#define WRITE_STAUS_REG 0X01
#define READ_DATA 0X03
#define FAST_READ_DATA 0X0B
#define PAGE_PROGRAM 0X02
#define SECTOR_ERASE 0XD8
#define BULK_ERASE 0XC7
#define DEEP_POWER_DOWN 0XB9
#define WAKE_UP 0XAB

void M25P16_Write_Enable(void);
void M25P16_Write_Disable(void);
void M25P16_Read_Id(u8 * id);
u8 M25P16_Read_Status_Reg(void);
void M25P16_Write_Status_Reg(u8 reg);
void M25P16_Read_Data(u32 addr,u32 len,u8 *buf);//进操作系统调度前用这个
void M25P16_Fast_Read_Data(u32 addr,u32 len,u8 *buf);//多任务用这个
void M25P16_Page_Program(u32 addr,u16 len,u8 *buf);
void M25P16_Sector_Erase(u32 addr);
void M25P16_Bulk_Erase(void);
void M25P16_Bulk_Erase2(void);
void M25P16_Deep_Power_Down(void);
u8 M25P16_Wake_Up(void);
void M25P16_Init(void);

//针对FatFs的定义
//如果字符码转换表存放到spi flash中，则开此宏
#define USE_SPI_FLASH_CHARATER_CODE_TABLE	1
#define OEM2UNI_TABLE_BASE 0
#define UNI2OEM_TABLE_BASE 341
#define SPI_FLASH_SECTOR_SIZE	256	//每个扇区256页
#define SPI_FLASH_PAGE_SIZE  256  //每页256个字节
#define CHARATER_CODE_TABLE_SIZE 87172 //表的size，单位:字节
#define TABLE_MAX_PAGE 341  //87172/512+1

//spi flash maps
//page				size				data     
//0-340			341			oem2uni
//341-681		341			uni2oem
//682-3690	3009		gbk(16x16)
//3691-5947  2257   gbk(12x12)



#endif





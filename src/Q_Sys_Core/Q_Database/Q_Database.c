#include "System.h"

#define DB_Debug Debug

#define DB_START_SECTOR	31 //spi flash的31扇区负责存放数据库
//每个扇区有256个页，每页256个字节
//0页负责存放索引，即当前数据库实际存放位置
//剩下的255个页，每DB_PAGE_NUM个页为单位顺次存放数据库
//但数据库被烧写255/DB_PAGE_NUM次时，表示整个扇区使用完毕
//则擦除整个扇区，从头开始存放

#ifndef SPI_FLASH_SECTOR_SIZE
	#define SPI_FLASH_SECTOR_SIZE	256		//Flash扇区包含页数
#endif

#ifndef SPI_FLASH_PAGE_SIZE
	#define SPI_FLASH_PAGE_SIZE	256		//Flash页大小
#endif

#define DB_PAGE_NUM	1//DB实际占用页数，可根据DB_STRUCT实际大小更改
#define DB_DATA_START_ADDR	DB_START_SECTOR*SPI_FLASH_SECTOR_SIZE*SPI_FLASH_PAGE_SIZE  //db扇区起始地址


//4系统配置实体类型，请和User.h里的SETTING_NAME一一对应
typedef __packed struct{	
	u32 DB_Ver;

	u8 BgLightTime;
	u8 BgLightScale;//0-100

}DB_STRUCT;	//数据库，不能超过DB_PAGE_NUM*SPI_FLASH_PAGE_SIZE字节，不能用指针

//DB 版本=值个数+占用总byte+数据库起始地址+数据库占用页数+用户定义值
#define DB_VERSION (Setting_DBMaxNum+sizeof(DB_STRUCT)+DB_PAGE_NUM+DB_DATA_START_ADDR+0x01)
//4系统默认配置
const static DB_STRUCT Def_DB_Setting={
	DB_VERSION,		//DB版本
	
	90,  //BgLightTime lcd背光时间
	100,//BgLightScale lcd背光强度
	
};

static u8 DB_Index_Buf[SPI_FLASH_PAGE_SIZE];		//spi flash 0页缓存,page0用来做记录索引
static u8 DB_Buf[DB_PAGE_NUM*SPI_FLASH_PAGE_SIZE];//数据库缓存
static DB_STRUCT *gpDB_Setting;

//系统状态存放区
static u32 gStatusFlag=0;

#if 0 //for debug
static void DB_BufDisp(void)
{
	u8 buf[SPI_FLASH_PAGE_SIZE];
	int i,j;

	DB_Debug("Spi Flash:\n\r");
	for(j=0;j<16;j++)
	{
		Q_SpiFlashSync(FlashRead,DB_DATA_START_ADDR+j*256,sizeof(buf),buf);
		DB_Debug("page %d context:",j);
		for(i=0;i<SPI_FLASH_PAGE_SIZE;i++)
		{
			if(buf[i]!=0xff)
			{
				DB_Debug("0x%02x ",buf[i]);
			}
		}
		DB_Debug("\n\r");
	}
}
#endif

//写默认值到flash和系统
static int DB_BurnDefaultToSpiFlash(void)
{
	int i;
	
	DB_Debug("Now burn default database to spi flash!\n\r");
	Q_SpiFlashSync(FlashSectorEarse,DB_START_SECTOR<<16,0,NULL);//擦除整个db存放扇区

	//建立索引
	memset(DB_Index_Buf,0xff,sizeof(DB_Index_Buf));
	DB_Index_Buf[0]=0;
	for(i=1;i<(1+DB_PAGE_NUM);i++)	DB_Index_Buf[i]=i;
	Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR,DB_PAGE_NUM+1,DB_Index_Buf);//写索引

	//将默认值写到系统和flash 1页
	memcpy((void *)DB_Buf,(void *)&Def_DB_Setting,sizeof(DB_STRUCT));//将默认数据拷贝到系统
	Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR+1*SPI_FLASH_PAGE_SIZE,
			sizeof(DB_STRUCT),(void *)&Def_DB_Setting);//将默认数据拷贝到flash db扇区1-4页

#if 0 //debug 
	DB_BufDisp();
#endif

	return TRUE;
}

//将缓存烧写到flash
bool Q_DB_BurnToSpiFlash(void)
{
	int i,j;
	
	//先找当前数据在flash的位置
	Q_SpiFlashSync(FlashRead,DB_DATA_START_ADDR,sizeof(DB_Index_Buf),DB_Index_Buf);
	for(i=0;i<sizeof(DB_Index_Buf);i++)
	{
		if(DB_Index_Buf[i]==0xff) break;
		else if(DB_Index_Buf[i]!=i)
		{
			DB_Debug("DB Index error!Rebuild database to flash!\n\r");
			i=SPI_FLASH_PAGE_SIZE;
			break;
		}		
	}	
	
	//建立索引
	if(i>=(SPI_FLASH_PAGE_SIZE-DB_PAGE_NUM))//写到末尾了，擦除整个扇区，重头开始写
	{
		i=1;
		Q_SpiFlashSync(FlashSectorEarse,DB_START_SECTOR<<16,0,NULL);//擦除整个db存放扇区
		memset(DB_Index_Buf,0xff,sizeof(DB_Index_Buf));
		DB_Index_Buf[0]=0;
		for(j=i;j<(i+DB_PAGE_NUM);j++)	DB_Index_Buf[j]=j;
		Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR,DB_PAGE_NUM+1,DB_Index_Buf);//写索引
	}
	else
	{		
		for(j=i;j<(i+DB_PAGE_NUM);j++)	DB_Index_Buf[j]=j;
		Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR+i,DB_PAGE_NUM,&DB_Index_Buf[i]);//写DB_PAGE_NUM个字节的索引
	}

	//将系统值写到flash 页
	DB_Debug("Burn database to flash sector %d page %d\n\r",DB_START_SECTOR,i);
	Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR+i*SPI_FLASH_PAGE_SIZE,
			sizeof(DB_STRUCT),(void *)gpDB_Setting);

#if 0 //debug 
	DB_BufDisp();
#endif

	return TRUE;
}

//从存储体读出数据库到内存，通过指针返回当前存储页
static bool DB_ReadFromSpiFlash(u8 *pPageIdx)
{
	int i;

	//初始系统指针
	memset((void *)DB_Buf,0,sizeof(DB_STRUCT));

	Q_SpiFlashSync(FlashRead,DB_DATA_START_ADDR,sizeof(DB_Index_Buf),DB_Index_Buf);

	for(i=1;i<sizeof(DB_Index_Buf);i++)//spi flash的page0用来做记录索引
	{
		if(DB_Index_Buf[i]==0xff) break;//找到第一个空页索引位置
		else if(DB_Index_Buf[i]!=i)
		{
			DB_Debug("DB Index error!Rebuild database to flash!\n\r");
			DB_BurnDefaultToSpiFlash();
			return FALSE;
		}		
	}

	if(((DB_PAGE_NUM!=1)&&(i%DB_PAGE_NUM!=1))||(i-DB_PAGE_NUM<1))//如果占用page改大，需调整这个三个"1"
	{
		DB_Debug("DB Index is not right!Brun default database to flash!\n\r");
		DB_BurnDefaultToSpiFlash();
		return FALSE;
	}
	else
	{
		i-=DB_PAGE_NUM;//找到最后一个记录页
		Q_SpiFlashSync(FlashRead,DB_DATA_START_ADDR+i*SPI_FLASH_PAGE_SIZE,
			sizeof(DB_STRUCT),(void *)gpDB_Setting);

		*pPageIdx=i;

#if 0 //debug 
		DB_BufDisp();
#endif

		if(gpDB_Setting->DB_Ver!=DB_VERSION)
		{
			DB_Debug("Database version 0x%x is not right!Burn default database to flash\n\r",gpDB_Setting->DB_Ver);
			DB_BurnDefaultToSpiFlash();
			return FALSE;
		}
	}

	return TRUE;	
}

//初始化函数 作用如下
// 1.从flash读取数据库内容
// 2.如果没有读到数据库，则将默认值数据库烧写到flash
// 3.如果有数据库，则读取数据库到缓存
bool DB_Init(void)
{	
	u8 PageIdx;
	
	Debug("Database init\n\r");

	if(sizeof(DB_STRUCT)>DB_PAGE_NUM*SPI_FLASH_PAGE_SIZE)
	{
		Debug("!!!DB init error!\n\r***Flash size for database is too small!\n\r");
		return FALSE;
	}
	
	gpDB_Setting=(DB_STRUCT *)DB_Buf;	
	DB_ReadFromSpiFlash(&PageIdx);
	
	Debug("DB Burn Sector:%d\n\r",DB_START_SECTOR);
	Debug("DB Current Saving Page Index:%d\n\r",PageIdx);
	Debug("DB Variable Num:%d\n\r",Setting_DBMaxNum);
	Debug("DB Version:%d\n\r",gpDB_Setting->DB_Ver);
	Debug("DB Size:%d Byte < DB_PAGE_NUM x SPI_FLASH_PAGE_SIZE (=%d Byte)\n\r",sizeof(DB_STRUCT),DB_PAGE_NUM*SPI_FLASH_PAGE_SIZE);
	
	return TRUE;
}

//从数据库缓存读取值，只能使用pDB_Setting获取
u32 Q_DB_GetValue(SETTING_NAME Name,void *Val)
{
	switch(Name)
	{
		case Setting_BgLightTime:
			return gpDB_Setting->BgLightTime;
		case Setting_BgLightScale:
			return gpDB_Setting->BgLightScale;
		default:
			Debug("Db not have such setting name!\n\r");
	}

	return 0;
}

//写值到数据库缓存，只能使用pDB_Setting存储
bool Q_DB_SetValue(SETTING_NAME Name,u32 IntParam,void *pParam,u8 ByteLen)
{
	OS_DeclareCritical();
	
	switch(Name)
	{
		case Setting_BgLightTime:
			OS_EnterCritical();
			gpDB_Setting->BgLightTime=IntParam;
			OS_ExitCritical();
			break;
		case Setting_BgLightScale:
			OS_EnterCritical();
			gpDB_Setting->BgLightScale=IntParam;
			OS_ExitCritical();
			break;
		default:
			Debug("Db not have such setting name!\n\r");
			return FALSE;
	}

	return TRUE;	
}

//从系统变量获取状态
u32 Q_DB_GetStatus(SETTING_NAME Name,void *Val)
{
	switch(Name)
	{
		case Status_FsInitFinish:
			return ReadBit(gStatusFlag,Name-Status_ValueStart);
		default:
			Debug("System not have such status name!\n\r");
	}

	return 0;
}

//写值到系统变量
bool Q_DB_SetStatus(SETTING_NAME Name,u32 IntParam,void *pParam,u8 ByteLen)
{
	OS_DeclareCritical();
	
	switch(Name)
	{
		case Status_FsInitFinish:
			OS_EnterCritical();
			if(IntParam) SetBit(gStatusFlag,Name-Status_ValueStart);
			else ClrBit(gStatusFlag,Name-Status_ValueStart);
			OS_ExitCritical();
			break;
		default:
			Debug("System not have such status name:%d\n\r",Name);
			return FALSE;
	}

	return TRUE;	
}




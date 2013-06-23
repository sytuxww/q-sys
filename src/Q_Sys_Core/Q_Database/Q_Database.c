#include "System.h"

#define DB_Debug Debug

#define DB_START_SECTOR	31 //spi flash��31�������������ݿ�
//ÿ��������256��ҳ��ÿҳ256���ֽ�
//0ҳ����������������ǰ���ݿ�ʵ�ʴ��λ��
//ʣ�µ�255��ҳ��ÿDB_PAGE_NUM��ҳΪ��λ˳�δ�����ݿ�
//�����ݿⱻ��д255/DB_PAGE_NUM��ʱ����ʾ��������ʹ�����
//�����������������ͷ��ʼ���

#ifndef SPI_FLASH_SECTOR_SIZE
	#define SPI_FLASH_SECTOR_SIZE	256		//Flash��������ҳ��
#endif

#ifndef SPI_FLASH_PAGE_SIZE
	#define SPI_FLASH_PAGE_SIZE	256		//Flashҳ��С
#endif

#define DB_PAGE_NUM	1//DBʵ��ռ��ҳ�����ɸ���DB_STRUCTʵ�ʴ�С����
#define DB_DATA_START_ADDR	DB_START_SECTOR*SPI_FLASH_SECTOR_SIZE*SPI_FLASH_PAGE_SIZE  //db������ʼ��ַ


//4ϵͳ����ʵ�����ͣ����User.h���SETTING_NAMEһһ��Ӧ
typedef __packed struct{	
	u32 DB_Ver;

	u8 BgLightTime;
	u8 BgLightScale;//0-100

}DB_STRUCT;	//���ݿ⣬���ܳ���DB_PAGE_NUM*SPI_FLASH_PAGE_SIZE�ֽڣ�������ָ��

//DB �汾=ֵ����+ռ����byte+���ݿ���ʼ��ַ+���ݿ�ռ��ҳ��+�û�����ֵ
#define DB_VERSION (Setting_DBMaxNum+sizeof(DB_STRUCT)+DB_PAGE_NUM+DB_DATA_START_ADDR+0x01)
//4ϵͳĬ������
const static DB_STRUCT Def_DB_Setting={
	DB_VERSION,		//DB�汾
	
	90,  //BgLightTime lcd����ʱ��
	100,//BgLightScale lcd����ǿ��
	
};

static u8 DB_Index_Buf[SPI_FLASH_PAGE_SIZE];		//spi flash 0ҳ����,page0��������¼����
static u8 DB_Buf[DB_PAGE_NUM*SPI_FLASH_PAGE_SIZE];//���ݿ⻺��
static DB_STRUCT *gpDB_Setting;

//ϵͳ״̬�����
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

//дĬ��ֵ��flash��ϵͳ
static int DB_BurnDefaultToSpiFlash(void)
{
	int i;
	
	DB_Debug("Now burn default database to spi flash!\n\r");
	Q_SpiFlashSync(FlashSectorEarse,DB_START_SECTOR<<16,0,NULL);//��������db�������

	//��������
	memset(DB_Index_Buf,0xff,sizeof(DB_Index_Buf));
	DB_Index_Buf[0]=0;
	for(i=1;i<(1+DB_PAGE_NUM);i++)	DB_Index_Buf[i]=i;
	Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR,DB_PAGE_NUM+1,DB_Index_Buf);//д����

	//��Ĭ��ֵд��ϵͳ��flash 1ҳ
	memcpy((void *)DB_Buf,(void *)&Def_DB_Setting,sizeof(DB_STRUCT));//��Ĭ�����ݿ�����ϵͳ
	Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR+1*SPI_FLASH_PAGE_SIZE,
			sizeof(DB_STRUCT),(void *)&Def_DB_Setting);//��Ĭ�����ݿ�����flash db����1-4ҳ

#if 0 //debug 
	DB_BufDisp();
#endif

	return TRUE;
}

//��������д��flash
bool Q_DB_BurnToSpiFlash(void)
{
	int i,j;
	
	//���ҵ�ǰ������flash��λ��
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
	
	//��������
	if(i>=(SPI_FLASH_PAGE_SIZE-DB_PAGE_NUM))//д��ĩβ�ˣ�����������������ͷ��ʼд
	{
		i=1;
		Q_SpiFlashSync(FlashSectorEarse,DB_START_SECTOR<<16,0,NULL);//��������db�������
		memset(DB_Index_Buf,0xff,sizeof(DB_Index_Buf));
		DB_Index_Buf[0]=0;
		for(j=i;j<(i+DB_PAGE_NUM);j++)	DB_Index_Buf[j]=j;
		Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR,DB_PAGE_NUM+1,DB_Index_Buf);//д����
	}
	else
	{		
		for(j=i;j<(i+DB_PAGE_NUM);j++)	DB_Index_Buf[j]=j;
		Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR+i,DB_PAGE_NUM,&DB_Index_Buf[i]);//дDB_PAGE_NUM���ֽڵ�����
	}

	//��ϵͳֵд��flash ҳ
	DB_Debug("Burn database to flash sector %d page %d\n\r",DB_START_SECTOR,i);
	Q_SpiFlashSync(FlashWrite,DB_DATA_START_ADDR+i*SPI_FLASH_PAGE_SIZE,
			sizeof(DB_STRUCT),(void *)gpDB_Setting);

#if 0 //debug 
	DB_BufDisp();
#endif

	return TRUE;
}

//�Ӵ洢��������ݿ⵽�ڴ棬ͨ��ָ�뷵�ص�ǰ�洢ҳ
static bool DB_ReadFromSpiFlash(u8 *pPageIdx)
{
	int i;

	//��ʼϵͳָ��
	memset((void *)DB_Buf,0,sizeof(DB_STRUCT));

	Q_SpiFlashSync(FlashRead,DB_DATA_START_ADDR,sizeof(DB_Index_Buf),DB_Index_Buf);

	for(i=1;i<sizeof(DB_Index_Buf);i++)//spi flash��page0��������¼����
	{
		if(DB_Index_Buf[i]==0xff) break;//�ҵ���һ����ҳ����λ��
		else if(DB_Index_Buf[i]!=i)
		{
			DB_Debug("DB Index error!Rebuild database to flash!\n\r");
			DB_BurnDefaultToSpiFlash();
			return FALSE;
		}		
	}

	if(((DB_PAGE_NUM!=1)&&(i%DB_PAGE_NUM!=1))||(i-DB_PAGE_NUM<1))//���ռ��page�Ĵ�������������"1"
	{
		DB_Debug("DB Index is not right!Brun default database to flash!\n\r");
		DB_BurnDefaultToSpiFlash();
		return FALSE;
	}
	else
	{
		i-=DB_PAGE_NUM;//�ҵ����һ����¼ҳ
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

//��ʼ������ ��������
// 1.��flash��ȡ���ݿ�����
// 2.���û�ж������ݿ⣬��Ĭ��ֵ���ݿ���д��flash
// 3.��������ݿ⣬���ȡ���ݿ⵽����
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

//�����ݿ⻺���ȡֵ��ֻ��ʹ��pDB_Setting��ȡ
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

//дֵ�����ݿ⻺�棬ֻ��ʹ��pDB_Setting�洢
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

//��ϵͳ������ȡ״̬
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

//дֵ��ϵͳ����
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




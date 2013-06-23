#ifndef QSYS_DATABASE_H
#define QSYS_DATABASE_H

typedef enum {
//-------------------系统数据-----------------------
	//下面存放系统数据，系统数据会被存到flash中
	Setting_BgLightTime=0,
	Setting_BgLightScale,
	
	Setting_DBMaxNum,		//最后一个值会影响数据库版本

//-------------------系统状态-----------------------
	//下面存放系统状态，系统状态不会被存到flash中
	//并且默认值都是0
	Status_ValueStart,
	Status_FsInitFinish,//文件系统是否完成挂载

	Setting_Status_MaxNum,//设置和状态的最大数目
}SETTING_NAME;

//数据库读函数
u32 Q_DB_GetValue(SETTING_NAME Name,void *Val);

//数据库写函数,不调用DB_BurnToSpiFlash的话,掉电会失去
//成功返回TRUE
bool Q_DB_SetValue(SETTING_NAME Name,u32 IntParam,void *pParam,u8 ByteLen);

//从系统获取状态,系统状态为临时变量
u32 Q_DB_GetStatus(SETTING_NAME Name,void *Val);

//设置系统状态
bool Q_DB_SetStatus(SETTING_NAME Name,u32 IntParam,void *pParam,u8 ByteLen);

//将数据库写到spi flash里
//成功返回TRUE
bool Q_DB_BurnToSpiFlash(void);

#endif


/*
q网地址管理
1.建议地址池不要太大，64足以

*/

#include "QWeb.h"
#include "QWebAddrMgt.h"

static QW_HOST_RECORD gQwHostTable[QW_ADDR_NUM];//记录每个设备密码及状态的结构体数组
static u8 gQwHostTableMap[QW_HOST_MAP_BYTES];//每一个bit代表一个设备

void QW_HostTableInit(void)
{
	memset(gQwHostTable,0,sizeof(QW_HOST_RECORD)*QW_ADDR_NUM);
	memset(gQwHostTableMap,0,sizeof(gQwHostTableMap));
	QW_FindNextActAddr(TRUE);
}

//获取指定地址的记录
QW_HOST_RECORD *QW_GetAddrRecord(u8 Addr)
{
	return &gQwHostTable[Addr];
}

u8 *QW_GetHostTableMap(void)
{
	return gQwHostTableMap;
}

//获取下一个活动的从机地址
//SetFrist=TRUE时，从头开始，返回值为0
//SetFrist=FALSE时，轮询下一个，到末尾后，返回0，并自动返回头部
u8 QW_FindNextActAddr(bool SetFrist)
{
	static u8 Row;
	static u8 Bit;
	
	if(SetFrist) 
	{
		Row=Bit=0;
		return 0;
	}
	else
	{
		if(Bit==7)
		{
			Bit=0;
			Row++;		
		}
		Bit++;
	}

	for(;Row<sizeof(gQwHostTableMap);Row++,Bit=0)
		if(gQwHostTableMap[Row])
			for(;Bit<8;Bit++)
				if((gQwHostTableMap[Row]>>Bit)&0x01)
					return (Row*8+Bit);

	Row=Bit=0;
	return 0;
}

void QW_SetMapBit(u8 Addr)
{
	u8 Row=Addr>>3;
	u8 Bit=Addr%8;
	if(Addr>=QW_ADDR_NUM) return;
	gQwHostTableMap[Row]|=(1<<Bit);	
}

void QW_ClrMapBit(u8 Addr)
{
	u8 Row=Addr>>3;
	u8 Bit=Addr%8;
	if(Addr>=QW_ADDR_NUM) return;
	gQwHostTableMap[Row]&=~(1<<Bit);	
}

u8 QW_ReadMapBit(u8 Addr)
{
	u8 Row=Addr>>3;
	u8 Bit=Addr%8;
	if(Addr>=QW_ADDR_NUM) return 0;
	return ((gQwHostTableMap[Row]>>Bit)&0x01);
}


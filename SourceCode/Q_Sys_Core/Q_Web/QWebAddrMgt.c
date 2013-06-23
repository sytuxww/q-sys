/*
q����ַ����
1.�����ַ�ز�Ҫ̫��64����

*/

#include "QWeb.h"
#include "QWebAddrMgt.h"

static QW_HOST_RECORD gQwHostTable[QW_ADDR_NUM];//��¼ÿ���豸���뼰״̬�Ľṹ������
static u8 gQwHostTableMap[QW_HOST_MAP_BYTES];//ÿһ��bit����һ���豸

void QW_HostTableInit(void)
{
	memset(gQwHostTable,0,sizeof(QW_HOST_RECORD)*QW_ADDR_NUM);
	memset(gQwHostTableMap,0,sizeof(gQwHostTableMap));
	QW_FindNextActAddr(TRUE);
}

//��ȡָ����ַ�ļ�¼
QW_HOST_RECORD *QW_GetAddrRecord(u8 Addr)
{
	return &gQwHostTable[Addr];
}

u8 *QW_GetHostTableMap(void)
{
	return gQwHostTableMap;
}

//��ȡ��һ����Ĵӻ���ַ
//SetFrist=TRUEʱ����ͷ��ʼ������ֵΪ0
//SetFrist=FALSEʱ����ѯ��һ������ĩβ�󣬷���0�����Զ�����ͷ��
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


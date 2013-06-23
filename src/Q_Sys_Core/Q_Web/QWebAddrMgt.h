#ifndef QWEB_ADDR_MGT_H
#define QWEB_ADDR_MGT_H


typedef struct{
	u8 UseFlag;
	u8 Password;
	//u16 NameChkSum;
	u32 LastActTimeMs;
}QW_HOST_RECORD;

void QW_HostTableInit(void);
QW_HOST_RECORD *QW_GetAddrRecord(u8 Addr);
u8 *QW_GetHostTableMap(void);
u8 QW_FindNextActAddr(bool SetFrist);
void QW_SetMapBit(u8 Addr);
void QW_ClrMapBit(u8 Addr);
u8 QW_ReadMapBit(u8 Addr);


#endif


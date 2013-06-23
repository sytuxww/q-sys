/*
q网包
1.此文件用来构成q网各种包
*/

#include <string.h>
#include "QWeb.h"
#include "QWebPacket.h"
#include "QWebAddrMgt.h"
#include "Debug.h"
#include "PublicFunc.h"

u8 gQW_Send_Count=0;

void QW_ShowPacket(QW_PACKET_HEADER *pPacket)
{
	u8 *pPktData=(void *)((u32)pPacket+sizeof(QW_PACKET_HEADER));
	u8 SrcAddrStr[16];
	u8 DstAddrStr[16];
	u8 DirChar[4];

	return;

	if(pPacket->SrcAddr==gMyQwAddr)//send
	{
		sprintf((void *)DirChar,">>");	
		QW_Debug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\r");
	}
	else	//recv
	{
		sprintf((void *)DirChar,"<<");	
		QW_Debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\r");
	}
	
	QW_Debug("%s Packet Len %d\n\r",DirChar,pPacket->Len);
	QW_Debug("%s ChkSum:%04x\n\r",DirChar,pPacket->ChkSum);
	QW_Debug("%s QW_ID:%02x\n\r",DirChar,pPacket->QW_ID);

	if(pPacket->SrcAddr==QW_ADDR_BCAST) sprintf((void *)SrcAddrStr,"Boardcast");
	else if(pPacket->SrcAddr==QW_ADDR_HOST) sprintf((void *)SrcAddrStr,"Host");
	else if(pPacket->SrcAddr==QW_ADDR_DEF) sprintf((void *)SrcAddrStr,"DefaultAddr");
	else sprintf((void *)SrcAddrStr,"Addr(%x)",pPacket->SrcAddr);

	if(pPacket->DstAddr==QW_ADDR_BCAST) sprintf((void *)DstAddrStr,"Boardcast");
	else if(pPacket->DstAddr==QW_ADDR_HOST) sprintf((void *)DstAddrStr,"Host");
	else if(pPacket->DstAddr==QW_ADDR_DEF) sprintf((void *)DstAddrStr,"DefaultAddr");
	else sprintf((void *)DstAddrStr,"Addr(%x)",pPacket->DstAddr);
	
	QW_Debug("%s Dir: %s -> %s\n\r",DirChar,SrcAddrStr,DstAddrStr);
	QW_Debug("%s Type:%s[%02x]\n\r",DirChar,gPacketTypeName[pPacket->Type],pPacket->Type);
	QW_Debug("%s Count:%d\n\r",DirChar,pPacket->Count);
	QW_Debug("%s Num:%d/%d\n\r",DirChar,pPacket->Num,pPacket->Total);
	QW_Debug("%s LastChkSum:%04x\n\r",DirChar,pPacket->LastChkSum);
	QW_Debug("%s Data(%d Bytes)",DirChar,pPacket->Len-QW_PKT_MIN_LEN);

	if((pPacket->Len>QW_PKT_MIN_LEN)&&(pPacket->Len-QW_PKT_MIN_LEN<=QW_MAX_DATA_LEN)) 
	{
		int i;
		QW_Debug(":");
		for(i=0;i<(pPacket->Len-QW_PKT_MIN_LEN);i++)
		{
			if(i%8==0) QW_Debug("\n\r");
			if(i%8==0) QW_Debug("%s ",DirChar);
			QW_Debug("0x%02x ",pPktData[i]);
			
		}
		QW_Debug("\n\r");
	}
	else
		QW_Debug("\n\r");

	if(pPacket->SrcAddr==gMyQwAddr)//send
	{
		sprintf((void *)DirChar,">>");	
		QW_Debug(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\r");
	}
	else	//recv
	{
		sprintf((void *)DirChar,"<<");	
		QW_Debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n\r");
	}
}

u16 QW_GetChkSum(const u8 *pData,u16 Len)
{
	u16 Hash=0;
	u16 i=0;

	for(;i<Len;i++)
	{
		Hash+=pData[i];
		Hash+=(Hash<<5);
	}

	return Hash;
}

u16 QW_CalculatePacketTotal(u32 DataLen)
{
	return ((DataLen-1)/QW_MAX_DATA_LEN)+1;
}

QW_RESULT QW_BuildReqAddrPacket(QW_PACKET_REQ_ADDR *pPacket,u8 HopeAddr,u8 Password)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN+2);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_REQ_ADDR)-2;
	pPacket->Hdr.DstAddr=QW_ADDR_HOST;//send to host
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_REQ_ADDR;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=Rand(0xffff);
	pPacket->HopeAddr=HopeAddr;
	pPacket->Password=Password;
	MemCpy(pPacket->AttributeStr,(void *)gMyQwAttributeStr,QW_ATTRIBUTE_STR_MAX_LEN);
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildAckAddrPacket(QW_PACKET_ACK_ADDR *pPacket,u16 LastChkSum,u8 DstAddr,u8 GiveAddr)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN+1);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_ACK_ADDR)-2;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_ACK_ADDR;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=LastChkSum;
	pPacket->GiveAddr=GiveAddr;
	MemCpy(pPacket->AttributeStr,(void *)gMyQwAttributeStr,QW_ATTRIBUTE_STR_MAX_LEN);
	MemCpy(pPacket->HostTableMap,(void *)QW_GetHostTableMap(),QW_HOST_MAP_BYTES);
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildQueryPacket(QW_PACKET_QUERY *pPacket,u8 QueryAddr)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN+1);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_QUERY)-2;
	pPacket->Hdr.DstAddr=QueryAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_QUERY;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=Rand(0xffff);
	pPacket->QueryAddr=QueryAddr;
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildAckQueryPacket(QW_PACKET_ACK_QUERY *pPacket,u16 LastChkSum,u8 DstAddr)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET));
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_ACK_QUERY)-2;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_ACK_QUERY;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=LastChkSum;
	MemCpy(pPacket->AttributeStr,(void *)gMyQwAttributeStr,QW_ATTRIBUTE_STR_MAX_LEN);
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildInfoPacket(QW_PACKET_INFO *pPacket,u8 DstAddr)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET));
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_INFO)-2;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_INFO;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=Rand(0xffff);
	pPacket->HostStartTime=QW_GetNowTimeMs()-gMyQwStartTime;
	pPacket->HwID=GetHwID();
	pPacket->DeviceType=gMyQwDrviceType;// drvice type
	MemCpy(pPacket->AttributeStr,(void *)gMyQwAttributeStr,QW_ATTRIBUTE_STR_MAX_LEN);	
	MemCpy(pPacket->HostTableMap,(void *)QW_GetHostTableMap(),QW_HOST_MAP_BYTES);
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildQuitPacket(QW_PACKET_QUIT *pPacket,u8 Password)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN+1);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_QUIT)-2;
	pPacket->Hdr.DstAddr=QW_ADDR_HOST;//send to host
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_QUIT;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=Rand(0xffff);
	pPacket->Password=Password;
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildAckQuitPacket(QW_PACKET_ACK_QUIT *pPacket,u16 LastChkSum,u8 DstAddr)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_ACK_QUIT)-2;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_ACK_QUIT;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=LastChkSum;
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildFristDataPacket(QW_PACKET_DATA *pPacket,u8 DstAddr,u8 *pData,u8 DataLen,u16 Total)
{
	if(DataLen>QW_MAX_DATA_LEN) DataLen=QW_MAX_DATA_LEN;
	
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET));
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_HEADER)-2+DataLen;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_DATA;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=Total;
	pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=0;//Rand(0xffff);
	MemCpy(pPacket->Data,pData,DataLen);
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

//替换data内容，根据原包构建新包
QW_RESULT QW_BuildNextDataPacket(QW_PACKET_DATA *pOldPacket,u8 *pData,u8 DataLen)
{
	if(DataLen>QW_MAX_DATA_LEN) DataLen=QW_MAX_DATA_LEN;
	
	pOldPacket->Hdr.Len=sizeof(QW_PACKET_HEADER)-2+DataLen;
	pOldPacket->Hdr.Count=gQW_Send_Count++;
	pOldPacket->Hdr.Num++;
	pOldPacket->Hdr.LastChkSum=pOldPacket->Hdr.ChkSum;
	MemCpy((void *)pOldPacket->Data,pData,DataLen);
	pOldPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pOldPacket->Hdr.Len,pOldPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildAckPacket(QW_PACKET_ACK *pPacket,u16 LastChkSum,u8 DstAddr)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_ACK)-2;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_ACK;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=LastChkSum;
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

QW_RESULT QW_BuildEckPacket(QW_PACKET_ECK *pPacket,u16 LastChkSum,u8 DstAddr,u8 ErrorID,u8 ActionID,u16 TimeParam)
{
	//MemSet((void *)pPacket,0,sizeof(QW_PACKET)-QW_MAX_DATA_LEN+4);
	pPacket->Hdr.QW_ID=gMyQwID;
	pPacket->Hdr.Len=sizeof(QW_PACKET_ECK)-2;
	pPacket->Hdr.DstAddr=DstAddr;
	pPacket->Hdr.SrcAddr=gMyQwAddr;
	pPacket->Hdr.Type=QWPT_ECK;
	pPacket->Hdr.Count=gQW_Send_Count++;
	pPacket->Hdr.Total=pPacket->Hdr.Num=1;
	pPacket->Hdr.LastChkSum=LastChkSum;
	pPacket->ErrorID=ErrorID;
	pPacket->Action=ActionID;
	pPacket->MsParam=TimeParam;	
	pPacket->Hdr.ChkSum=QW_GetChkSum((void *)&pPacket->Hdr.Len,pPacket->Hdr.Len);

	return QWR_SUCCESS;
}

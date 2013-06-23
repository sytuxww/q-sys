/*
q网传输
1.此文件用来负责将q网数据传输到硬件层及从硬件层收取数据

*/

#include "System.h"
#include "Drivers.h"
#include "CC2500.h"
#include "QWeb.h"
#include "QWebPacket.h"

bool QW_HwSetup(void)
{
	return CC2500_Init();
}

u8 QW_SendPacket(QW_PACKET_HEADER *pPacket)
{	
	u8 Cnt;
	u8 DA;

	QW_Debug(">> Send [%s]->%d %04x @ %u\n\r",gPacketTypeName[pPacket->Type],pPacket->DstAddr,pPacket->ChkSum,QW_GetNowTimeMs());

	QW_ShowPacket((void *)pPacket);
	
	//交换字节，方便硬件处理
	DA=pPacket->DstAddr;
	pPacket->DstAddr=((u8 *)pPacket)[0];
	((u8 *)pPacket)[0]=DA;

	Cnt=CC2500_SendPacket((void *)pPacket,sizeof(QW_PACKET_DATA));
	
	//恢复交换
	DA=pPacket->DstAddr;
	pPacket->DstAddr=((u8 *)pPacket)[0];
	((u8 *)pPacket)[0]=DA;
	
	//CC2500_SetRxd();
	return Cnt;
}

u8 QW_RecvPacket(QW_PACKET_HEADER *pPacket)
{
	u8 Len,DA;
	Len=CC2500_ReceivePacket((void *)pPacket);

	if(Len==0)
	{
		return 0;
	}
	else if(Len<=PACKET_LEN)
	{	//还原字节
		DA=((u8 *)pPacket)[0];
		((u8 *)pPacket)[0]=pPacket->DstAddr;
		pPacket->DstAddr=DA;
	}
	else
	{
		QW_Debug("Error Packet Len %d!\n\r",Len);
		return 0;
	}
	
	if(pPacket->Len!=Len-2)
	{
		//QW_Debug("Error Packet Len! %d %d\n\r",pPacket->Len,Len); 
		//return 0;
	}

	// 2.从机在获取到自己的Q网ID之前，不处理除Info包之外的任何包。
	// 3.从机在获取到自己的地址之后，不处理任何非本Q网ID的包以及目的地址非本机或非广播的包。
	// 4.主机不处理除Query之外的任何非本Q网ID的包及目的地址非本机的包。
	if(gMyQwNowRole==QWDT_SLAVE)//从机
	{
		if(gMyQwID==0)//本机qwid为0时，只接收info包
		{
			if(pPacket->Type!=QWPT_INFO)
			{
				QW_Debug("Only Need Info!\n\r");
				return 0;
			}
		}
		else //有QwID时
		{
			if(pPacket->QW_ID!=gMyQwID) 
			{
				QW_Debug("Error QW ID %02x,My:%02x\n\r",pPacket->QW_ID,gMyQwID);
				return 0;
			}
			else 	if((pPacket->DstAddr!=gMyQwAddr)&&(pPacket->DstAddr!=QW_ADDR_BCAST))
			{
				QW_Debug("Error Addr %02x\n\r",pPacket->DstAddr);
				return 0;
			}
		}
	}
	else //主机
	{
		if((pPacket->QW_ID!=gMyQwID)&&(pPacket->Type!=QWPT_QUERY))
		{
			QW_Debug("Error QW ID %02x,My:%02x\n\r",pPacket->QW_ID,gMyQwID);
			return 0;
		}
		else 	if(pPacket->DstAddr!=gMyQwAddr)
		{
			QW_Debug("Error Addr %02x\n\r",pPacket->DstAddr);
			return 0;
		}
	}
	
	if(pPacket->Num>pPacket->Total)
	{
		QW_Debug("Error Packet Num! %02x/%02x\n\r",pPacket->Num,pPacket->Total);
		return 0;
	}

	//检查包ChkSum
	if(pPacket->ChkSum!=QW_GetChkSum((void *)&pPacket->Len,pPacket->Len)) 
	{
		QW_Debug("Error Packet ChkSum!%x %x\n\r",pPacket->ChkSum,QW_GetChkSum((void *)&pPacket->Len,pPacket->Len));
		//DisplayBuf((void *)pPacket,64,8);
		return 0;
	}

	QW_Debug("<< Recv [%s]<-%d %04x @ %u\n\r",gPacketTypeName[pPacket->Type],pPacket->SrcAddr,pPacket->ChkSum,QW_GetNowTimeMs());
	
	return Len;
}

void QWeb_SetAddrMode(u8 MyAddr,bool RecvBroadcost)
{
	if(RecvBroadcost) CC2500_SetRecvAddr(CRA_FIXED_00,MyAddr);
	else CC2500_SetRecvAddr(CRA_FIXED,MyAddr);

#ifdef QW_IN_DEBUG //for debug
	CC2500_SetRecvAddr(CRA_ALL,MyAddr);
#endif
}








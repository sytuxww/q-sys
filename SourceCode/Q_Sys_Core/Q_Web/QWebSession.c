/*
q网会话
1.此文件用来处理会话的相关数据
2.会话创建时，会记录首包
3.会话销毁时，会注销相关内存
4.会话匹配，按照上一包的校验码来

*/

#include "QWeb.h"
#include "QWebSession.h"
#include "QWebTransmission.h"
#include "QWebApi.h"

extern u8 gMyQwAddr;//本设备当前q网地址

#define Map(H,L) (((H)<<8)|(L))

static QW_SESSION gSession[QW_MAX_SESSION_NUM];

//获取session
QW_SESSION *QW_SessionGet(bool SetFrist)
{
	static u8 Idx=0;
	
	if(SetFrist)
	{
		Idx=0;
		return &gSession[0];
	}
	else
	{
		if(Idx<QW_MAX_SESSION_NUM)
			return &gSession[Idx++];
	};
	
	return NULL;
}

//根据id找session
QW_SESSION *QW_SessionGetById(QW_SESSION_ID ID)
{
	u8 Idx;
	for(Idx=0;Idx<QW_MAX_SESSION_NUM;Idx++)
	{
		if((gSession[Idx].Type!=QWST_UNUSE)&&(gSession[Idx].ID==ID))
			return &gSession[Idx];
	}

	return NULL;
}

//初始化数据结构
void QW_SessionsInit(void)
{
	memset(QW_SessionGet(TRUE),0,sizeof(QW_SESSION)*QW_MAX_SESSION_NUM);
}

void QW_ShowSession(QW_SESSION *pSession,bool ShowData)
{
	u8 SrcAddrStr[16];
	u8 DstAddrStr[16];
	
	QW_Debug("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\r");
	QW_Debug("@@ Session Type: %s\n\r",gSessionTypeName[pSession->Type]);
	QW_Debug("@@ LastChkSum: %x\n\r",pSession->LastChkSum);
	
	if(pSession->SrcAddr==QW_ADDR_BCAST) sprintf((void *)SrcAddrStr,"Boardcast");
	else if(pSession->SrcAddr==QW_ADDR_HOST) sprintf((void *)SrcAddrStr,"Host");
	else if(pSession->SrcAddr==QW_ADDR_DEF) sprintf((void *)SrcAddrStr,"DefaultAddr");
	else sprintf((void *)SrcAddrStr,"Addr(%x)",pSession->SrcAddr);

	if(pSession->DstAddr==QW_ADDR_BCAST) sprintf((void *)DstAddrStr,"Boardcast");
	else if(pSession->DstAddr==QW_ADDR_HOST) sprintf((void *)DstAddrStr,"Host");
	else if(pSession->DstAddr==QW_ADDR_DEF) sprintf((void *)DstAddrStr,"DefaultAddr");
	else sprintf((void *)DstAddrStr,"Addr(%x)",pSession->DstAddr);
	
	QW_Debug("@@ Dir: %s -> %s\n\r",SrcAddrStr,DstAddrStr);

	QW_Debug("@@ Flags: ");
	if(QW_SessionReadFlag(pSession,QwDestroyBit)) QW_Debug("DestroyBit ");
	if(QW_SessionReadFlag(pSession,QwDataMallcoBit)) QW_Debug("DataMallcoBit ");
	if(QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_Debug("QwNoSendFlag ");
	QW_Debug("\n\r");

	QW_Debug("@@ Packet: %d/%d , Type %s , Ptr:0x%x\n\r",pSession->PacketNum,pSession->PacketTotal,gPacketTypeName[pSession->pPacket->Hdr.Type],(u32)pSession->pPacket);
	QW_Debug("@@ ReSend Param:%d/%d , Gap:%d Ms\n\r",pSession->ReCnt,pSession->TotalReCnt,pSession->GapMs);
	QW_Debug("@@ LastTime:%d Ms , NowTime:%d Ms\n\r",pSession->LastActTimeMs,QW_GetNowTimeMs());

	QW_Debug("@@ Data(%d Bytes)",pSession->DataLen);	

	if((pSession->DataLen<=QW_MAX_MALLCO_BYTES)&&(pSession->DataLen>0)&&(ShowData))
	{
		int i;

		QW_Debug(":");
		for(i=0;i<(pSession->DataLen);i++)
		{
			if(i%8==0) QW_Debug("\n\r");
			if(i%8==0) QW_Debug("@@ ");
			QW_Debug("0x%02x ",pSession->pData[i]);
		}
		QW_Debug("\n\r");
	}
	else
		QW_Debug("\n\r");

	QW_Debug("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\r");
}

//新建一个会话，默认不重发
QW_RESULT QW_CreatSession(QW_SESSION **ppSession,QW_SESSION_ID MyID,QW_PACKET_HEADER *pFristPacket,QWEB_DATA_STRUCT *pDataStruct,QW_SessionCallBack CallBackFunc)
{
	QW_SESSION *pSession;
	
	//检查包
	if(pFristPacket==NULL) return QWR_FAILED;
	if(pFristPacket->Total>QW_MAX_SUBPACKET_NUM) return QWR_BUF_UN_ENOUGH;//检查分包数是不是太大
	
	//先找是否有空间
	QW_SessionGet(TRUE);
	while((pSession=QW_SessionGet(FALSE))!=NULL)
	{	
		if(pSession->Type==QWST_UNUSE)	//未使用
			goto SessionHandler;
	}
	return QWR_SESSION_FULL;

SessionHandler:	
	//然后构建数据			
	switch(pFristPacket->Type)
	{
		case QWPT_REQ_ADDR:
		case QWPT_QUERY:
		case QWPT_QUIT:
		case QWPT_INFO:
			if(pFristPacket->SrcAddr==gMyQwAddr) 
				pSession->Type=QWST_SEND;
			else
				pSession->Type=QWST_RECV;
			break;
		case QWPT_DATA:
			if(pFristPacket->SrcAddr==gMyQwAddr) 
			{
				if(pDataStruct==NULL) return QWR_FAILED;//发送的时候，需要此参数
				pSession->Type=QWST_SEND;
			}
			else
			{
				pSession->Type=QWST_RECV;
			}
			break;
		default:
			return QWR_FAILED;
	}
	pSession->ID=MyID;
	pSession->SrcAddr=pFristPacket->SrcAddr;
	pSession->DstAddr=pFristPacket->DstAddr;
	pSession->Flags=0;
	pSession->LastChkSum=pFristPacket->ChkSum;
	pSession->PacketTotal=pFristPacket->Total;
	pSession->PacketNum=0;
	pSession->ReCnt=0;
	pSession->TotalReCnt=1;
	pSession->GapMs=0;
	pSession->LastActTimeMs=QW_GetNowTimeMs();
	pSession->CallBackFunc=CallBackFunc;
	if(pSession->Type==QWST_SEND) pSession->pPacket=QW_Mallco(sizeof(QW_PACKET_DATA));
	//MemSet(gSession[i].pPacket,0,sizeof(QW_PACKET));
	
	if(pSession->Type==QWST_SEND)
	{
		if(pFristPacket->Type==QWPT_DATA)
		{
			pSession->DataLen=pDataStruct->DataLen;
			pSession->pData=pDataStruct->pData;
		}
		else
		{
			pSession->DataLen=0;
			pSession->pData=NULL;
		}
	}
	else //recv
	{
		u8 *pPktData=(void *)((u32)pFristPacket+sizeof(QW_PACKET_HEADER));
		pSession->DataLen=pFristPacket->Len-QW_PKT_MIN_LEN;
		pSession->pData=QW_Mallco(QW_MAX_DATA_LEN*pSession->PacketTotal);
		QW_SessionSetFlag(pSession,QwDataMallcoBit);

		MemCpy(pSession->pData,pPktData,pFristPacket->Len-QW_PKT_MIN_LEN);
		pSession->PacketNum=1;
	}

	*ppSession=pSession;

	QW_Debug("QW_CreatSession(0x%x)\n\r",(u32)pSession);
	//QW_ShowSession(&gSession[i],FALSE);

	return QWR_SUCCESS;
}

//删除session
QW_RESULT QW_DestroySession(QW_SESSION *pSession)
{
	//QW_Debug("%s\n\r",__func__);
	
	if(pSession==NULL)
	{
		Debug("QW_DestroySession Error ID!!!\n\r");
		return QWR_FAILED;
	}
	
	//验证数据
	if(pSession->Type==QWST_UNUSE) return QWR_NOHANDLE;
	
	//销毁结构
	QW_Debug("Destroy Session(0x%x):\n\r",(u32)pSession);
	QW_ShowSession(pSession,FALSE);
	if(pSession->Type==QWST_SEND) QW_Free(pSession->pPacket);
	//if(QW_SessionReadFlag(pSession,QwDataMallcoBit)) QW_Free(pSession->pData);//此处就不free了，只有recv data才会申请此处空间，改由用户自行调用QW_CopyDataFinish，提高效率
	//MemSet(pSession,0,sizeof(QW_SESSION));
	pSession->Type=QWST_UNUSE;

	return QWR_SUCCESS;
}

//根据包的内容更新session
void QW_SessionSync(QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	pSession->LastChkSum=pPacket->ChkSum;
}

//设置session的重发次数，在创建session之后调用
QW_RESULT QW_SessionSetReSend(QW_SESSION *pSession,u8 TotalReCnt,u32 ReTimeGap)
{
	if(pSession==NULL)
	{
		Debug("QW_SessionSetReSend Error ID!!!\n\r");
		return QWR_FAILED;
	}
	
	pSession->ReCnt=0;
	pSession->TotalReCnt=TotalReCnt;
	pSession->GapMs=ReTimeGap;

	return QWR_SUCCESS;
}

//设置session的接收超时参数
//如果超过TimeOutMs没有接收到新包，则session会被强行销毁
QW_RESULT QW_SessionSetRecvTimeOut(QW_SESSION *pSession,u32 TimeOutMs)
{
	if(pSession==NULL)
	{
		Debug("QW_SessionSetRecvTimeOut Error ID!!!\n\r");
		return QWR_FAILED;
	}

	pSession->ReCnt=0;
	pSession->TotalReCnt=0;
	pSession->GapMs=TimeOutMs;

	return QWR_SUCCESS;	
}

//根据包来找session
//只有返回包才有session
//匹配条件:
QW_RESULT QW_SessionFind(QW_PACKET_HEADER *pPacket,QW_SESSION **ppSession)
{
	QW_SESSION *pSession;

	if(pPacket==NULL) return QWR_FAILED;

	switch(pPacket->Type)
	{
		case QWPT_ACK_ADDR:
		case QWPT_ACK_QUERY:
		case QWPT_ACK_QUIT:
		case QWPT_ACK:
		case QWPT_ECK:
			QW_SessionGet(TRUE);
			while((pSession=QW_SessionGet(FALSE))!=NULL)
			{
				if((pPacket->LastChkSum==pSession->LastChkSum)//校验和匹配
					&&(pPacket->DstAddr==pSession->pPacket->Hdr.SrcAddr)//地址匹配
					&&(pPacket->SrcAddr==pSession->pPacket->Hdr.DstAddr)//地址匹配
					&&(pSession->Type==QWST_SEND)//类型匹配
					)
				{
					*ppSession=pSession;
					return QWR_SUCCESS;
				}
			}		
			break;
		case QWPT_DATA://recv
			QW_SessionGet(TRUE);
			while((pSession=QW_SessionGet(FALSE))!=NULL)
			{
				if((pPacket->SrcAddr==pSession->SrcAddr)//地址匹配
					&&(pPacket->DstAddr==pSession->DstAddr)//地址匹配
					&&(pPacket->Total==pSession->PacketTotal)//总数匹配
					&&(pSession->Type==QWST_RECV)//类型匹配
					)
				{
					if((pPacket->Num==(pSession->PacketNum+1))&&(pPacket->LastChkSum==pSession->LastChkSum))
					{//数目匹配
						*ppSession=pSession;
						return QWR_SUCCESS;
					}
					else if((pPacket->Num==pSession->PacketNum)&&(pPacket->ChkSum==pSession->LastChkSum))
					{//数目匹配
						*ppSession=pSession;
						return QWR_SUCCESS;
					}
				}
			}
			break;
		default:
			return QWR_FAILED;
	}

	return QWR_NOT_FIND_SESSION;
}

//对session下命令
QW_RESULT QW_SessionCmd(QW_SESSION *pSession,QW_CMD Cmd,QW_PACKET_HEADER *pPacket,u32 TimeParam)
{
	if(pSession==NULL)
	{
		Debug("QW_SessionCmd Error ID!!!\n\r");
		return QWR_FAILED;
	}
	
	switch(Cmd)
	{
		case QWC_SEND_FRIST_PKT://用来发送包，并记录到session
			if(pPacket==NULL) return QWR_FAILED;
			pSession->CallBackFunc(QWCA_BF_SEND,0,pSession,pPacket);
			MemCpy(pSession->pPacket,pPacket,sizeof(QW_PACKET_DATA));//记包
			QW_SessionSync(pSession,pPacket);
			pSession->PacketNum++;//计数
			pSession->ReCnt++;//计数
			if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket(pPacket);//发送
			pSession->LastActTimeMs=QW_GetNowTimeMs();//计时
			pSession->CallBackFunc(QWCA_AF_SEND,0,pSession,pPacket);		
			break;
		case QWC_SEND_AGAIN://重发一次包
			pSession->CallBackFunc(QWCA_BF_SEND,0,pSession,pSession->pPacket);
			QW_SessionSync(pSession,(void *)pSession->pPacket);
			if(pSession->TotalReCnt) pSession->ReCnt++;//计数
			if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket((void *)pSession->pPacket);//发送
			pSession->LastActTimeMs=QW_GetNowTimeMs();//计时
			pSession->CallBackFunc(QWCA_AF_SEND,0,pSession,pSession->pPacket);
			break;
		case QWC_SEND_NEXT_DATA://继续发数据包
			{
				if(pPacket==NULL)
				{
					u8 *p;
					u8 Len;
					pPacket=(void *)pSession->pPacket;

					p=(void *)((u32)pSession->pData+pSession->PacketNum*QW_MAX_DATA_LEN);
					if((pSession->PacketNum+1)*QW_MAX_DATA_LEN<pSession->DataLen)
						Len=QW_MAX_DATA_LEN;
					else
						Len=QW_MAX_DATA_LEN-((pSession->PacketNum+1)*QW_MAX_DATA_LEN-pSession->DataLen);
						
					QW_BuildNextDataPacket((void *)pPacket,p,Len);
				}
				else
				{
					MemCpy(pSession->pPacket,pPacket,sizeof(QW_PACKET_DATA));
				}	

				pSession->CallBackFunc(QWCA_BF_SEND,0,pSession,pSession->pPacket);
				QW_SessionSync(pSession,(void *)pSession->pPacket);
				pSession->PacketNum++;//计数
				if(pSession->TotalReCnt) pSession->ReCnt=1;//计数
				if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket((void *)pSession->pPacket);//发送		
				pSession->LastActTimeMs=QW_GetNowTimeMs();//计时
				pSession->CallBackFunc(QWCA_AF_SEND,0,pSession,pSession->pPacket);
			}
			break;
		case QWC_SEND_ACK://发送ack包
			if(pPacket==NULL) return QWR_FAILED;
			if(pPacket->Type!=QWPT_DATA) return QWR_FAILED;
			pSession->LastChkSum=pPacket->ChkSum;
			{
				QW_PACKET_ACK *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK));
				QW_BuildAckPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
				if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);//发送
				QW_Free(pPacketTemp);
			}
			pSession->LastActTimeMs=QW_GetNowTimeMs();//计时
			break;
		case QWC_RECV_DATA://收取数据包
			if(pPacket->Type!=QWPT_DATA) return QWR_REQ_ERROR;
			if(!QW_SessionReadFlag(pSession,QwDataMallcoBit)) return QWR_FAILED;

			MemCpy((void *)((u32)pSession->pData+pSession->DataLen),(void *)((u32)pPacket+sizeof(QW_PACKET_HEADER)),pPacket->Len-QW_PKT_MIN_LEN);
			pSession->LastActTimeMs=QW_GetNowTimeMs();	
			pSession->DataLen+=(pPacket->Len-QW_PKT_MIN_LEN);
			pSession->LastChkSum=pPacket->ChkSum;
			pSession->PacketNum++;
			break;
		case QWC_SET_SEND_TIME://定时发送
			if(TimeParam<=pSession->GapMs)//发送时间最长为GapMs
				pSession->LastActTimeMs=QW_GetNowTimeMs()-(pSession->GapMs-TimeParam);	
			//Debug("%d=%d-(%d-%d)\n\r",pSession->LastActTimeMs,QW_GetNowTimeMs(),pSession->GapMs,TimeParam);
			break;
	}

	return QWR_SUCCESS;
}

//设置flags指定位
//pSession为空时，通过ID寻找session
void QW_SessionSetFlag(QW_SESSION *pSession,u8 FlagsBit)
{
	if(FlagsBit>7) return;

	if(pSession!=NULL)
		pSession->Flags|=(1<<FlagsBit);
}

//清除flags指定位
//pSession为空时，通过ID寻找session
void QW_SessionClrFlag(QW_SESSION *pSession,u8 FlagsBit)
{
	if(FlagsBit>7) return;

	if(pSession!=NULL)
		pSession->Flags&=~(1<<FlagsBit);
}

//读取flags指定位
//pSession为空时，通过ID寻找session
u8 QW_SessionReadFlag(QW_SESSION *pSession,u8 FlagsBit)
{
	if(FlagsBit>7) return 0;

	if(pSession!=NULL)
		return ((pSession->Flags>>FlagsBit)&1);
	
	return 0;
}

//计算有效session的下一个发送等待时间
u32 QW_CalculateWaitTime(void)
{
	u32 NowTimeMs=QW_GetNowTimeMs();
	u32 MinWaitTimeMs=0xffffffff;
	u32 NextTimeMs;
	QW_SESSION *pSession;

	QW_SessionGet(TRUE);

	while((pSession=QW_SessionGet(FALSE))!=NULL)
	{
		if(pSession->Type==QWST_UNUSE) continue;//未使用，找下一个
		else if(pSession->Type==QWST_SEND)
		{
			if(pSession->ReCnt<1) continue;//如果还没发送过一次，就找下一个
		}
		
		if(NowTimeMs<(pSession->LastActTimeMs+pSession->GapMs))//比较时间
			NextTimeMs=(pSession->LastActTimeMs+pSession->GapMs)-NowTimeMs;//算出下次发送时间
		else 
		{
			return 0;//如果时间过期，则需要立即处理
		}
		MinWaitTimeMs=MinWaitTimeMs>NextTimeMs?NextTimeMs:MinWaitTimeMs;
	}
	
	return MinWaitTimeMs;
}

//处理发送超时
//遍历每一个session
//如果TotalReCnt=0，则计算当前是否需要重发
//如果TotalReCnt=1，不处理
//如果TotalReCnt=others，计算当期是否需要重发，如果ReCnt=0，不重发
//  如果重发
//    如果ReCnt=TotalReCnt,则不重发，调用callback
//		 重发执行，调用callback，ReCnt++，
QW_RESULT QW_SessionTimeout(void)
{
	QW_SESSION *pSession;

	QW_SessionGet(TRUE);

	while((pSession=QW_SessionGet(FALSE))!=NULL)
	{
		if(pSession->Type==QWST_UNUSE) 
		{
			continue;//无效session，跳过
		}
		else if(pSession->Type==QWST_RECV)//接收类型
		{
			if(QW_GetNowTimeMs()>=(pSession->LastActTimeMs+pSession->GapMs))//超过了时效仍然没收到包
			{
				u16 ChkSum=QW_GetChkSum((void *)pSession,sizeof(QW_SESSION));
				pSession->CallBackFunc(QWCA_TIMEOUT,0,pSession,NULL);//超时
				if(ChkSum==QW_GetChkSum((void *)pSession,sizeof(QW_SESSION)))
					QW_DestroySession(pSession);//超时销毁			
			}
		}
		else if(pSession->Type==QWST_SEND)//发送类型
		{
			if(pSession->ReCnt==0) continue;//第一次发送必须主动
				
			QWU_Debug("  Curt:%d Last:%d Gap:%d ",QW_GetNowTimeMs(),pSession->LastActTimeMs,pSession->GapMs);
			if(QW_GetNowTimeMs()>=(pSession->LastActTimeMs+pSession->GapMs))//当前时间超过了时间间隔
			{
				QWU_Debug("ReCnt:%d Total:%d\n\r",pSession->ReCnt,pSession->TotalReCnt);
				if(pSession->ReCnt==pSession->TotalReCnt)//已经重发完了
				{
					u16 ChkSum=QW_GetChkSum((void *)pSession,sizeof(QW_SESSION));
					pSession->CallBackFunc(QWCA_TIMEOUT,0,pSession,NULL);//超时
					if(ChkSum==QW_GetChkSum((void *)pSession,sizeof(QW_SESSION)))
						QW_DestroySession(pSession);//超时销毁
				}
				else//重发
				{
					QW_SessionCmd(pSession,QWC_SEND_AGAIN,NULL,0);
				}
			}
			else
				QWU_Debug("\n\r");
		}
	}

	return QWR_SUCCESS;
}




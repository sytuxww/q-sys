/*
q���Ự
1.���ļ���������Ự���������
2.�Ự����ʱ�����¼�װ�
3.�Ự����ʱ����ע������ڴ�
4.�Ựƥ�䣬������һ����У������

*/

#include "QWeb.h"
#include "QWebSession.h"
#include "QWebTransmission.h"
#include "QWebApi.h"

extern u8 gMyQwAddr;//���豸��ǰq����ַ

#define Map(H,L) (((H)<<8)|(L))

static QW_SESSION gSession[QW_MAX_SESSION_NUM];

//��ȡsession
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

//����id��session
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

//��ʼ�����ݽṹ
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

//�½�һ���Ự��Ĭ�ϲ��ط�
QW_RESULT QW_CreatSession(QW_SESSION **ppSession,QW_SESSION_ID MyID,QW_PACKET_HEADER *pFristPacket,QWEB_DATA_STRUCT *pDataStruct,QW_SessionCallBack CallBackFunc)
{
	QW_SESSION *pSession;
	
	//����
	if(pFristPacket==NULL) return QWR_FAILED;
	if(pFristPacket->Total>QW_MAX_SUBPACKET_NUM) return QWR_BUF_UN_ENOUGH;//���ְ����ǲ���̫��
	
	//�����Ƿ��пռ�
	QW_SessionGet(TRUE);
	while((pSession=QW_SessionGet(FALSE))!=NULL)
	{	
		if(pSession->Type==QWST_UNUSE)	//δʹ��
			goto SessionHandler;
	}
	return QWR_SESSION_FULL;

SessionHandler:	
	//Ȼ�󹹽�����			
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
				if(pDataStruct==NULL) return QWR_FAILED;//���͵�ʱ����Ҫ�˲���
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

//ɾ��session
QW_RESULT QW_DestroySession(QW_SESSION *pSession)
{
	//QW_Debug("%s\n\r",__func__);
	
	if(pSession==NULL)
	{
		Debug("QW_DestroySession Error ID!!!\n\r");
		return QWR_FAILED;
	}
	
	//��֤����
	if(pSession->Type==QWST_UNUSE) return QWR_NOHANDLE;
	
	//���ٽṹ
	QW_Debug("Destroy Session(0x%x):\n\r",(u32)pSession);
	QW_ShowSession(pSession,FALSE);
	if(pSession->Type==QWST_SEND) QW_Free(pSession->pPacket);
	//if(QW_SessionReadFlag(pSession,QwDataMallcoBit)) QW_Free(pSession->pData);//�˴��Ͳ�free�ˣ�ֻ��recv data�Ż�����˴��ռ䣬�����û����е���QW_CopyDataFinish�����Ч��
	//MemSet(pSession,0,sizeof(QW_SESSION));
	pSession->Type=QWST_UNUSE;

	return QWR_SUCCESS;
}

//���ݰ������ݸ���session
void QW_SessionSync(QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	pSession->LastChkSum=pPacket->ChkSum;
}

//����session���ط��������ڴ���session֮�����
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

//����session�Ľ��ճ�ʱ����
//�������TimeOutMsû�н��յ��°�����session�ᱻǿ������
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

//���ݰ�����session
//ֻ�з��ذ�����session
//ƥ������:
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
				if((pPacket->LastChkSum==pSession->LastChkSum)//У���ƥ��
					&&(pPacket->DstAddr==pSession->pPacket->Hdr.SrcAddr)//��ַƥ��
					&&(pPacket->SrcAddr==pSession->pPacket->Hdr.DstAddr)//��ַƥ��
					&&(pSession->Type==QWST_SEND)//����ƥ��
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
				if((pPacket->SrcAddr==pSession->SrcAddr)//��ַƥ��
					&&(pPacket->DstAddr==pSession->DstAddr)//��ַƥ��
					&&(pPacket->Total==pSession->PacketTotal)//����ƥ��
					&&(pSession->Type==QWST_RECV)//����ƥ��
					)
				{
					if((pPacket->Num==(pSession->PacketNum+1))&&(pPacket->LastChkSum==pSession->LastChkSum))
					{//��Ŀƥ��
						*ppSession=pSession;
						return QWR_SUCCESS;
					}
					else if((pPacket->Num==pSession->PacketNum)&&(pPacket->ChkSum==pSession->LastChkSum))
					{//��Ŀƥ��
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

//��session������
QW_RESULT QW_SessionCmd(QW_SESSION *pSession,QW_CMD Cmd,QW_PACKET_HEADER *pPacket,u32 TimeParam)
{
	if(pSession==NULL)
	{
		Debug("QW_SessionCmd Error ID!!!\n\r");
		return QWR_FAILED;
	}
	
	switch(Cmd)
	{
		case QWC_SEND_FRIST_PKT://�������Ͱ�������¼��session
			if(pPacket==NULL) return QWR_FAILED;
			pSession->CallBackFunc(QWCA_BF_SEND,0,pSession,pPacket);
			MemCpy(pSession->pPacket,pPacket,sizeof(QW_PACKET_DATA));//�ǰ�
			QW_SessionSync(pSession,pPacket);
			pSession->PacketNum++;//����
			pSession->ReCnt++;//����
			if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket(pPacket);//����
			pSession->LastActTimeMs=QW_GetNowTimeMs();//��ʱ
			pSession->CallBackFunc(QWCA_AF_SEND,0,pSession,pPacket);		
			break;
		case QWC_SEND_AGAIN://�ط�һ�ΰ�
			pSession->CallBackFunc(QWCA_BF_SEND,0,pSession,pSession->pPacket);
			QW_SessionSync(pSession,(void *)pSession->pPacket);
			if(pSession->TotalReCnt) pSession->ReCnt++;//����
			if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket((void *)pSession->pPacket);//����
			pSession->LastActTimeMs=QW_GetNowTimeMs();//��ʱ
			pSession->CallBackFunc(QWCA_AF_SEND,0,pSession,pSession->pPacket);
			break;
		case QWC_SEND_NEXT_DATA://���������ݰ�
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
				pSession->PacketNum++;//����
				if(pSession->TotalReCnt) pSession->ReCnt=1;//����
				if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket((void *)pSession->pPacket);//����		
				pSession->LastActTimeMs=QW_GetNowTimeMs();//��ʱ
				pSession->CallBackFunc(QWCA_AF_SEND,0,pSession,pSession->pPacket);
			}
			break;
		case QWC_SEND_ACK://����ack��
			if(pPacket==NULL) return QWR_FAILED;
			if(pPacket->Type!=QWPT_DATA) return QWR_FAILED;
			pSession->LastChkSum=pPacket->ChkSum;
			{
				QW_PACKET_ACK *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK));
				QW_BuildAckPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
				if(!QW_SessionReadFlag(pSession,QwNoSendFlag)) QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);//����
				QW_Free(pPacketTemp);
			}
			pSession->LastActTimeMs=QW_GetNowTimeMs();//��ʱ
			break;
		case QWC_RECV_DATA://��ȡ���ݰ�
			if(pPacket->Type!=QWPT_DATA) return QWR_REQ_ERROR;
			if(!QW_SessionReadFlag(pSession,QwDataMallcoBit)) return QWR_FAILED;

			MemCpy((void *)((u32)pSession->pData+pSession->DataLen),(void *)((u32)pPacket+sizeof(QW_PACKET_HEADER)),pPacket->Len-QW_PKT_MIN_LEN);
			pSession->LastActTimeMs=QW_GetNowTimeMs();	
			pSession->DataLen+=(pPacket->Len-QW_PKT_MIN_LEN);
			pSession->LastChkSum=pPacket->ChkSum;
			pSession->PacketNum++;
			break;
		case QWC_SET_SEND_TIME://��ʱ����
			if(TimeParam<=pSession->GapMs)//����ʱ���ΪGapMs
				pSession->LastActTimeMs=QW_GetNowTimeMs()-(pSession->GapMs-TimeParam);	
			//Debug("%d=%d-(%d-%d)\n\r",pSession->LastActTimeMs,QW_GetNowTimeMs(),pSession->GapMs,TimeParam);
			break;
	}

	return QWR_SUCCESS;
}

//����flagsָ��λ
//pSessionΪ��ʱ��ͨ��IDѰ��session
void QW_SessionSetFlag(QW_SESSION *pSession,u8 FlagsBit)
{
	if(FlagsBit>7) return;

	if(pSession!=NULL)
		pSession->Flags|=(1<<FlagsBit);
}

//���flagsָ��λ
//pSessionΪ��ʱ��ͨ��IDѰ��session
void QW_SessionClrFlag(QW_SESSION *pSession,u8 FlagsBit)
{
	if(FlagsBit>7) return;

	if(pSession!=NULL)
		pSession->Flags&=~(1<<FlagsBit);
}

//��ȡflagsָ��λ
//pSessionΪ��ʱ��ͨ��IDѰ��session
u8 QW_SessionReadFlag(QW_SESSION *pSession,u8 FlagsBit)
{
	if(FlagsBit>7) return 0;

	if(pSession!=NULL)
		return ((pSession->Flags>>FlagsBit)&1);
	
	return 0;
}

//������Чsession����һ�����͵ȴ�ʱ��
u32 QW_CalculateWaitTime(void)
{
	u32 NowTimeMs=QW_GetNowTimeMs();
	u32 MinWaitTimeMs=0xffffffff;
	u32 NextTimeMs;
	QW_SESSION *pSession;

	QW_SessionGet(TRUE);

	while((pSession=QW_SessionGet(FALSE))!=NULL)
	{
		if(pSession->Type==QWST_UNUSE) continue;//δʹ�ã�����һ��
		else if(pSession->Type==QWST_SEND)
		{
			if(pSession->ReCnt<1) continue;//�����û���͹�һ�Σ�������һ��
		}
		
		if(NowTimeMs<(pSession->LastActTimeMs+pSession->GapMs))//�Ƚ�ʱ��
			NextTimeMs=(pSession->LastActTimeMs+pSession->GapMs)-NowTimeMs;//����´η���ʱ��
		else 
		{
			return 0;//���ʱ����ڣ�����Ҫ��������
		}
		MinWaitTimeMs=MinWaitTimeMs>NextTimeMs?NextTimeMs:MinWaitTimeMs;
	}
	
	return MinWaitTimeMs;
}

//�����ͳ�ʱ
//����ÿһ��session
//���TotalReCnt=0������㵱ǰ�Ƿ���Ҫ�ط�
//���TotalReCnt=1��������
//���TotalReCnt=others�����㵱���Ƿ���Ҫ�ط������ReCnt=0�����ط�
//  ����ط�
//    ���ReCnt=TotalReCnt,���ط�������callback
//		 �ط�ִ�У�����callback��ReCnt++��
QW_RESULT QW_SessionTimeout(void)
{
	QW_SESSION *pSession;

	QW_SessionGet(TRUE);

	while((pSession=QW_SessionGet(FALSE))!=NULL)
	{
		if(pSession->Type==QWST_UNUSE) 
		{
			continue;//��Чsession������
		}
		else if(pSession->Type==QWST_RECV)//��������
		{
			if(QW_GetNowTimeMs()>=(pSession->LastActTimeMs+pSession->GapMs))//������ʱЧ��Ȼû�յ���
			{
				u16 ChkSum=QW_GetChkSum((void *)pSession,sizeof(QW_SESSION));
				pSession->CallBackFunc(QWCA_TIMEOUT,0,pSession,NULL);//��ʱ
				if(ChkSum==QW_GetChkSum((void *)pSession,sizeof(QW_SESSION)))
					QW_DestroySession(pSession);//��ʱ����			
			}
		}
		else if(pSession->Type==QWST_SEND)//��������
		{
			if(pSession->ReCnt==0) continue;//��һ�η��ͱ�������
				
			QWU_Debug("  Curt:%d Last:%d Gap:%d ",QW_GetNowTimeMs(),pSession->LastActTimeMs,pSession->GapMs);
			if(QW_GetNowTimeMs()>=(pSession->LastActTimeMs+pSession->GapMs))//��ǰʱ�䳬����ʱ����
			{
				QWU_Debug("ReCnt:%d Total:%d\n\r",pSession->ReCnt,pSession->TotalReCnt);
				if(pSession->ReCnt==pSession->TotalReCnt)//�Ѿ��ط�����
				{
					u16 ChkSum=QW_GetChkSum((void *)pSession,sizeof(QW_SESSION));
					pSession->CallBackFunc(QWCA_TIMEOUT,0,pSession,NULL);//��ʱ
					if(ChkSum==QW_GetChkSum((void *)pSession,sizeof(QW_SESSION)))
						QW_DestroySession(pSession);//��ʱ����
				}
				else//�ط�
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




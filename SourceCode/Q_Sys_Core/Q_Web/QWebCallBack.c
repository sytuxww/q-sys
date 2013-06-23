/*
q��callback��������
1.ÿ�����͵İ�������һ��callback��������
2.callback�������ж���ʱ���������յ��°�[QWCA_NEW_RECV]������ǰ[QWCA_BF_SEND]��
	���ͺ�[QWCA_AF_SEND]���յ�����[QWCA_RECV_REQ]���յ�Ӧ��[QWCA_RECV_ACK]����ʱ[QWCA_TIMEOUT]
3.����ʱ����ض��Լ��Ĺ̶���ɫ�͵�ǰ��ɫ����ǰ״̬���жϣ�����ᵼ�´������
4.�����¼�������Ҫ����Ӧ�ò�ʱ�����ʹ��QW_Sync����

*/

#include "QWeb.h"
#include "QWebSession.h"
#include "QWebCallBack.h"
#include "QWebTransmission.h"
#include "QWebAddrMgt.h"

//callbackҪ��������
// 1.QWCA_NEW_RECV���棬Ҫ���н�ɫ��飬�Լ����ļ���

QW_RESULT QW_SessionAddrHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	QW_PACKET_REQ_ADDR *pReqAddrPkt=(void *)pPacket;
	QW_PACKET_ACK_ADDR *pAckAddrPkt=(void *)pPacket;
	//QW_Debug("%s:%s\n\r",__func__,gSessionStateName[State]);

	switch(State)
	{
		case QWCA_NEW_RECV://ֱ�Ӵ���pSession=NULL,��û��Session
			QW_Debug("Get a new addr req!\n\r");
			if(gMyQwNowRole==QWDT_SLAVE)
			{
				QW_Debug("I am a Slave!\n\r");
				return QWR_NOHANDLE;//����Ǵӻ����Ͳ�����
			}
			else //�����ַ
			{
				QW_PACKET_ACK_ADDR *pAckPktTmp=QW_Mallco(sizeof(QW_PACKET_ACK_ADDR));
				u8 GiveAddr=pReqAddrPkt->HopeAddr;

				switch(GiveAddr)
				{
					case QW_ADDR_BCAST:
					case QW_ADDR_HOST:
					case QW_ADDR_DEF://�����ַ����
						GiveAddr=QW_ADDR_MIN;
						break;
				}
				
				if(QW_GetAddrRecord(GiveAddr)->UseFlag!=0)//�鿴��ַ���õ�ַ��ռ��
				{
					u8 IdleAddr;
					for(IdleAddr=QW_ADDR_MIN;IdleAddr<QW_ADDR_NUM;IdleAddr++)
						if(QW_GetAddrRecord(IdleAddr)->UseFlag==0)
						{
							GiveAddr=IdleAddr;
							break;
						}
					if(IdleAddr==QW_ADDR_NUM)
					{
						QW_Debug("!!!Address pool is full!!!\n\r");
						return QWR_ADDR_FULL;//��ַ�����ذ���������ѯsession��unfinish
					}
				}

				//�ǼǷ����ַ
				QW_GetAddrRecord(GiveAddr)->UseFlag=1;
				QW_GetAddrRecord(GiveAddr)->Password=pReqAddrPkt->Password;//��¼����
				QW_GetAddrRecord(GiveAddr)->LastActTimeMs=QW_GetNowTimeMs();
				QW_SetMapBit(GiveAddr);
				QW_Sync(QWE_NewJoin,GiveAddr,QW_ATTRIBUTE_STR_MAX_LEN,pReqAddrPkt->AttributeStr);
				QW_Debug("Give Addr is 0x%x\n\r",GiveAddr);

				//�ذ�
				QW_BuildAckAddrPacket(pAckPktTmp,pPacket->ChkSum,pPacket->SrcAddr,GiveAddr);
				QW_SendPacket((QW_PACKET_HEADER *)pAckPktTmp);
				QW_Free(pAckPktTmp);
			}	
			break;
		case QWCA_RECV_ACK://���գ����������֮����ȡ��ackӦ��
			if(gMyQwNowRole==QWDT_SLAVE)//�ӻ�
			{
				QW_Debug("Get My Addr:%x,QWeb ID:%x\n\r",pAckAddrPkt->GiveAddr,pPacket->QW_ID);
				gMyQwNowRole=QWDT_SLAVE;
				gMyQwID=pPacket->QW_ID;//�趨q��id
				gMyQwAddr=pAckAddrPkt->GiveAddr;
				QWeb_SetAddrMode(gMyQwAddr,TRUE);//�趨��ַ�����ܹ㲥
				MemCpy((void *)QW_GetHostTableMap(),pAckAddrPkt->HostTableMap,QW_HOST_MAP_BYTES);

				QW_Sync(QWE_NewJoin,QW_ADDR_HOST,QW_ATTRIBUTE_STR_MAX_LEN,pAckAddrPkt->AttributeStr);
				QW_DestroySession(pSession);
			}
			else
			{
				Debug("QW_SessionAddrHandler Host error!!!");
			}
			break;
		case QWCA_TIMEOUT://���һ�η����󣬼��GapMs�����������걾�¼�����ǰsession���١�
			QW_Debug("Req Addr Timeout!\n\r");
			//QW_Sync(QWE_ReqAddrFaile,0,0,NULL);
			break;
	}

	return QWR_SUCCESS;
}

QW_RESULT QW_SessionQueryHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	QW_RESULT Error;
	QW_PACKET_QUERY *pQueryPkt=(void *)pPacket;
	QW_PACKET_ACK_QUERY *pAckQueryPkt=(void *)pPacket;

	//QW_Debug("%s:%s\n\r",__func__,gSessionStateName[State]);

	switch(State)
	{
		case QWCA_NEW_RECV://�յ����󣬷���ack
			if(pQueryPkt->QueryAddr!=gMyQwAddr) break;
			
			if(gMyQwNowRole==QWDT_SLAVE)//����Ǵӻ�
			{
				QW_PACKET_ACK_QUERY *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK_QUERY));
				QW_BuildAckQueryPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
				QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
				QW_Free(pPacketTemp);
			}
			else  //��������������ʱ���info��
			{
				QW_SESSION *pSessionTemp=QW_SessionGetById(QWSI_HOST_INFO);
				if(pSessionTemp!=NULL)
				{
					u32 t=Rand(0xffff)%QW_T1;//Debug("Set Info time:%d\n\r",t);
					QW_SessionCmd(pSessionTemp,QWC_SET_SEND_TIME,NULL,t);//�漴ʱ�����info
				}
			}
			break;
		case QWCA_BF_SEND:
			if(gMyQwNowRole==QWDT_SLAVE)
			{

			}
			else//�������������ַά��
			{
				if(pSession->ID==QWSI_HOST_QUERY)//������ַά��
				{
					u8 Addr;
					u32 NowMs=QW_GetNowTimeMs();
					
					while((Addr=QW_FindNextActAddr(FALSE))!=0)//ѭ����ѯ�豸
					{
						if(Addr==QW_ADDR_HOST) continue;//���������Լ�
						if((QW_GetAddrRecord(Addr)->UseFlag)&&((NowMs-QW_GetAddrRecord(Addr)->LastActTimeMs)>QW_T6))//��ʱ�ľͷ���
						{
							if((NowMs-QW_GetAddrRecord(Addr)->LastActTimeMs)>(QW_T6<<1))//��ʱ1����ֱ��ɾ��
							{
								QW_SESSION *pSessionTemp;

								QW_GetAddrRecord(Addr)->UseFlag=QW_GetAddrRecord(Addr)->Password=0;
								QW_ClrMapBit(Addr);
								QW_Debug("Delete Addr[%d] Device!\n\r",Addr);

								//ɾ���ӻ���������̷���info��
								pSessionTemp=QW_SessionGetById(QWSI_HOST_INFO);
								if(pSessionTemp!=NULL)
									QW_SessionCmd(pSessionTemp,QWC_SET_SEND_TIME,NULL,100);//����info
							}
							else
							{
								//�İ�
								QW_BuildQueryPacket(pQueryPkt,Addr);
								break;//ÿ��ֻ��һ������
							}			
						}
					}
					
					if(Addr==0) 
					{
						QW_SessionSetFlag(pSession,QwNoSendFlag);
					}
				}
			}
			break;
		case QWCA_AF_SEND:
			if(gMyQwNowRole==QWDT_SLAVE)
			{

			}
			else
			{
				if(pSession->ID==QWSI_HOST_QUERY)
					QW_SessionClrFlag(pSession,QwNoSendFlag);
			}
			break;
		case QWCA_RECV_ACK:
			Debug("Get a QueryAck from Addr(%d),Str:%s\n\r",pPacket->SrcAddr,pAckQueryPkt->AttributeStr);
			QW_Sync(QWE_QueryAck,pPacket->SrcAddr,QW_ATTRIBUTE_STR_MAX_LEN,pAckQueryPkt->AttributeStr);
			break;
		case QWCA_TIMEOUT:
			if((pSession->ID==QWSI_SLAVE_QUERY)&&(gMyQwDrviceType==QWDT_AUTO_HOST)&&(gMyQwNowRole==QWDT_SLAVE)&&(pSession->DstAddr==QW_ADDR_HOST))//query��ʱҲû���յ�info��ֱ�ӱ��Ϊ����
			{
				if((Error=QW_ChangeRole(QWDT_AUTO_HOST,GetHwID()&0xff))!=QWR_SUCCESS)//��Ϊ����
				{
					QW_Debug("%s Error Change Role!%d\n\r",__func__,Error);
					while(1);
				}
			}
			break;
	}

	return QWR_SUCCESS;
}

QW_RESULT QW_SessionInfoHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	u32 OpponentStartTime,MyStartTime;
	QW_PACKET_INFO *pInfoPkt=(void *)pPacket;
	
	//QW_Debug("%s:%s\n\r",__func__,gSessionStateName[State]);

	switch(State)
	{
		case QWCA_NEW_RECV://ֱ�Ӵ���pSession=NULL,����ҪSession
			if(gMyQwNowRole==QWDT_SLAVE)//����Ǵӻ�������id��ѡ�����������id��ɾ��query��Ȼ���͵�ַ����
			{
				if(gMyQwID==0)//��û�ҵ���
				{
					QW_SESSION *pSessionTemp;

					//ѡ��q��,unfinish
					if((gMyQwHostHwId)&&(gMyQwHostHwId!=pInfoPkt->HwID))
					{
						QW_Debug("QW HW ID error!%x %x\n\r",gMyQwHostHwId,pInfoPkt->HwID);
						return QWR_SUCCESS;
					}
					
					//����q��id
					gMyQwID=pPacket->QW_ID;
					gMyQwHostHwId=pInfoPkt->HwID;

					//ɾ��slaver frist query
					pSessionTemp=QW_SessionGetById(QWSI_SLAVE_QUERY);
					if(pSessionTemp!=NULL)
						QW_DestroySession(pSessionTemp);

					{//����addr req
						QW_RESULT Result;
						QW_PACKET_REQ_ADDR *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_REQ_ADDR));
						QW_SESSION *pSessionTemp;
						
						QW_BuildReqAddrPacket(pPacketTemp,QW_MY_HOPE_ADDR,gMyQwAddrReqPw);//����
						if((Result=QW_CreatSession(&pSessionTemp,QWSI_SLAVE_REQ_ADDR,(void *)pPacketTemp,NULL,(QW_SessionCallBack)QW_SessionAddrHandler))!=QWR_SUCCESS)//���Ự
						{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
						QW_SessionSetReSend(pSessionTemp,QW_N3,QW_T7);
						QW_SessionCmd(pSessionTemp,QWC_SEND_FRIST_PKT,(void *)pPacketTemp,0);//��һ�η��ͱ����ֶ�
						QW_Free(pPacketTemp);
					}
				}
				else //�Լ��Ǵӻ��������Ѿ�������id
				{
					QWU_Debug("Get Info:%s\n\r",pInfoPkt->AttributeStr);
					
					//����table
					MemCpy((void *)QW_GetHostTableMap(),pInfoPkt->HostTableMap,QW_HOST_MAP_BYTES);
					QW_Sync(QWE_NewJoin,pPacket->SrcAddr,QW_ATTRIBUTE_STR_MAX_LEN,pInfoPkt->AttributeStr);
					
					//�жϵ�ַ������û���Լ�
					if(QW_ReadMapBit(gMyQwAddr)==0)
					{//����ǣ����ٴ������ַ
						QW_Debug("QW Host Reset!!!\n\r");
						QW_ChangeRole(QWDT_SLAVE,0);
					}
				}				
			}
			else //�Լ�������������յ�info��Ϣ������������ͻ
			{
				QW_Debug("------!!!Host Conflict!!!------\n\r");

				//�˴���������ʱ���ٲ�
				OpponentStartTime=pInfoPkt->HostStartTime;
				MyStartTime=QW_GetNowTimeMs()-gMyQwStartTime;
				if(OpponentStartTime>MyStartTime)//�Լ�������ʱ��Ƚ϶̣������Լ�
				{
					if(QW_ChangeRole(QWDT_SLAVE,0)==QWR_SUCCESS)//������ͻ������Լ�Ϊ�ӻ�
					{
						return QWR_SUCCESS;
					}
				}
				else if(OpponentStartTime==MyStartTime)
				{
					QW_Sync(QWE_HostConflict,0,0,NULL);
				}
				else
				{
					return QWR_SUCCESS;
				}
				
				return QWR_HOST_CONFLICT;//δ����ɹ�������
			}
			break;
		case QWCA_BF_SEND:
			QW_BuildInfoPacket(pInfoPkt,pPacket->DstAddr);
			break;
	}

	return QWR_SUCCESS;
}

QW_RESULT QW_SessionQuitHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	//QW_Debug("%s:%s\n\r",__func__,gSessionStateName[State]);
	QW_PACKET_QUIT *pQuitPkt=(void *)pPacket;

	switch(State)
	{
		case QWCA_NEW_RECV://�յ����󣬷���ack
			if(gMyQwNowRole==QWDT_SLAVE)
			{

			}
			else//���������������Ӧ
			{
				if((QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag)&&(pQuitPkt->Password==QW_GetAddrRecord(pPacket->SrcAddr)->Password))//������ȷ
				{
					QW_PACKET_ACK_QUIT *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK_QUIT));

					QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag=0;
					QW_GetAddrRecord(pPacket->SrcAddr)->Password=0;
								
					QW_BuildAckQuitPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
					QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
					QW_Free(pPacketTemp);
				}
			}
			break;
	}
	
	return QWR_SUCCESS;
}

QW_RESULT QW_SessionDataHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	QW_SESSION *pSessionTemp;
	QW_PACKET_DATA *pDataPkt=(void *)pPacket;
	//QW_PACKET_ACK *pAckPkt=(void *)pPacket;
	u8 Result;
	
	//QW_Debug("%s:%s\n\r",__func__,gSessionStateName[State]);

	switch(State)
	{
		case QWCA_NEW_RECV://ֱ�Ӵ���pSession=NULL,��û��Session
			//��ȷ������豸�Ƿ�Ϸ�
			if(gMyQwNowRole==QWDT_SLAVE)//����Ǵӻ������ѯ�˵�ַ�豸�Ƿ����
			{
			
			}
			else if(QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag==0) 
						return QWR_NO_SUCH_ADDR;
			
			if(pPacket->Num!=1) return QWR_REQ_ERROR;

			{//��data recv session
				QW_PACKET_ACK *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK));
				
				if((Result=QW_CreatSession(&pSessionTemp,QWSI_DATA_RECV,pPacket,NULL,(QW_SessionCallBack)QW_SessionDataHandler))!=QWR_SUCCESS)//���Ự
				{QW_Debug("%s Error CreatSession!%d\n\r",__func__,Result);while(1);}		
				QW_SessionSetRecvTimeOut(pSessionTemp,QW_N2*QW_T2);//���ó�ʱ����
				
				//�ذ�				
				QW_BuildAckPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
				QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
				QW_Free(pPacketTemp);
			}

			if(pPacket->Total==1)//data�ְ�ֻ��1����ֱ�ӷ�ͬ����Ϣ
			{
				u8 DataLen=pPacket->Len-QW_PKT_MIN_LEN;
				u8 *pData=QW_Mallco(DataLen);//������ڴ潫��Ӧ�ò����QW_CopyDataFinishʱ�ͷ�
				MemCpy(pData,pDataPkt->Data,DataLen);
				QW_Sync(QWE_Recv,pPacket->SrcAddr,DataLen,pData);
			}
			break;
		case QWCA_RECV_REQ://�ǵ�һ�ν��յ�������ƥ��session
			if(pPacket->Num==(pSession->PacketNum+1))//��ȷ��
			{
				if(QW_SessionCmd(pSession,QWC_RECV_DATA,pPacket,0)==QWR_SUCCESS)
				{
					if(QW_SessionCmd(pSession,QWC_SEND_ACK,pPacket,0)!=QWR_SUCCESS) return QWR_FAILED;

					if(pSession->PacketNum==pSession->PacketTotal)//�հ����
						QW_Sync(QWE_Recv,pPacket->SrcAddr,pSession->DataLen,pSession->pData);
				}
				else	//���մ���
				{
					Debug("Recv error!\n\r");
					QW_ShowPacket(pPacket);
					return QWR_FAILED;
				}
			}
			else if(pPacket->Num==pSession->PacketNum)//���հ�
			{
				if(QW_SessionCmd(pSession,QWC_SEND_ACK,pPacket,0)!=QWR_SUCCESS) return QWR_FAILED;
			}
			else//�ϰ���
			{
				QW_PACKET_ECK *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ECK));
				
				//��������
				Debug("Error Packet Count!\n\r");

				//�ش����
				QW_BuildEckPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr,0,0,0);
				QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
				QW_Free(pPacketTemp);
			}					
			break;
		case QWCA_RECV_ACK://���գ����������֮����ȡ��ackӦ��
			if(pPacket->Type==QWPT_ACK)//��ȷ
			{
				if(pSession->PacketNum==pSession->PacketTotal)//�������
				{
					QW_Sync(QWE_SendOk,pPacket->SrcAddr,pSession->DataLen,pSession->pData);
					QW_DestroySession(pSession);//��������ɾ��session
				}
				else
					QW_SessionCmd(pSession,QWC_SEND_NEXT_DATA,NULL,0);//������һ����
			}
			else if(pPacket->Type==QWPT_ECK)//�����ط�
			{
				QW_SessionCmd(pSession,QWC_SEND_AGAIN,NULL,0);
			}
			else//������
			{
				Debug("Recv a error packet!\n\r");
				QW_ShowPacket(pPacket);
				return QWR_FAILED;
			}
			break;
		case QWCA_TIMEOUT:
			if(pSession->Type==QWST_SEND)
			{
				QW_Sync(QWE_SendFailed,pPacket->SrcAddr,pSession->DataLen,pSession->pData);//����δ���ͬ����Ϣ
			}
			else	if(pSession->Type==QWST_RECV)
			{
				if(pSession->PacketNum!=pSession->PacketTotal) 
					QW_Debug("$$$ Recv Un Finish!\n\r");
					//QW_Sync(QWE_Recv,1,NULL);//����δ���ͬ����Ϣ
			}
			break;
	};
	
	return QWR_SUCCESS;
}


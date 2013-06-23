//---------------------------------------------------------------------
// ���������Q��Э���д�����ݴ���ϸ����ο�Q��Э��淶
// Q��Э��--�ɿ���Ƽ��ƶ��İ�˫�����豸������Э��
//---------------------------------------------------------------------
/*
q���Ĵ�������������״̬
1.�������ͣ����½�session������ذ�
2.��ʱ���ͣ����ڴ���ʱδ�ظ��İ�
3.����ذ�
*/
/*
q�������������
1.ÿ�����͵İ������Ӧ��callback������������
2.��Ӧ�д��һ���������䣬���ǳ�֮Ϊsession[�Ự]
3.Ϊ�˱���Ự��ͨ��QW_SESSION�ṹ������������
4.QW_SESSION������Ҫ�������ڴ汣���װ����յ���data
5.���������������󣬻������һ�������ʱ�䣬��ʱ�䵽�ڣ������յ��°��������յ�Ӧ�ò�����ʱ�������ж���
6.session��һ������£������ʱ���ͻᱻע��
*/

#include "System.h"
#include "CC2500.h"
#include "QWeb.h"
#include "QWebSession.h"
#include "QWebTransmission.h"
#include "QWebPacket.h"
#include "QWebCallBack.h"
#include "QWebApi.h"
#include "QWebAddrMgt.h"

//----------------------import-------------------------
extern OS_SemaphoreHandle gRfRecvHandler_Sem;//��Ƶ�������ݵ��ź���
extern bool gQWebApiFlag;
extern QWEB_DATA_STRUCT gQWebUserData;

//----------------------public--------------------------
const QW_DEVICE_TYPE gMyQwDrviceType=QWDT_AUTO_HOST;//�̶����豸����
u8 gMyQwAttributeStr[QW_ATTRIBUTE_STR_MAX_LEN]="";//q�������ַ���

QW_DEVICE_TYPE gMyQwNowRole;//��ǰ��ɫ
u8 gMyQwID=0;//q��id���ɳ�������ʱ���������
u8 gMyQwAddr=QW_ADDR_DEF;//���豸��ǰq����ַ
u8 gMyQwAddrReqPw=0;//��¼�����ַʱ������
u32 gMyQwStartTime=0;//q�������ľ���ʱ�䣬��λ����
u32 gMyQwHostHwId=0;//�ӻ�������¼q������Ӳ���룬���������ϵ�������ʶ������

//----------------------function------------------------
u32 QW_GetNowTimeMs(void)
{
	return OS_GetCurrentSysMs();
}

void *QW_Mallco(u16 Size)
{
	return Q_Mallco(Size);
}

bool QW_Free(void *Ptr)
{
	return Q_Free(Ptr);
}

//�����趨��ɫ
//�������Ϊ��������ʼ������Ϣ�����
//�������Ϊ�ӻ�����ʼ���͵�ַ�����
//��Ϊ�յ�Ӧ��������Ϊ�ӻ������ܵ��ô˺���!!!
//��Ϊ�˺����ᵼ������session���٣�������callback��������!!!
QW_RESULT QW_ChangeRole(QW_DEVICE_TYPE Role,u8 QWebID)
{
	QW_RESULT Result;
	u8 RF_Buf[QW_MAX_PACKET_LEN];
	QW_PACKET_HEADER *pPacket=(void *)RF_Buf;
	QW_SESSION *pSession;

	QW_Debug("#Change Role <%s> ID:%x\n\r",gQwDeviceName[Role],QWebID);

	switch(Role)//��ɫ���
	{
		case QWDT_HOST:
		case QWDT_AUTO_HOST:
			if(gMyQwDrviceType!=Role)
			{
				QW_Debug("Change Role ERROR!\n\r");
				return QWR_NOHANDLE;
			}
		case QWDT_SLAVE:
			if((gMyQwDrviceType!=QWDT_AUTO_HOST)&&(gMyQwDrviceType!=QWDT_SLAVE))
			{
				QW_Debug("Change Role ERROR!\n\r");
				return QWR_NOHANDLE;
			}
	}

	gMyQwNowRole=Role;
	gMyQwID=QWebID;//�趨q��id

	//�任��ɫ��ɾ������Session
	QW_SessionGet(TRUE);
	while((pSession=QW_SessionGet(FALSE))!=NULL)
		QW_DestroySession(pSession);

	QW_HostTableInit();//��յ�ַ��
	
	switch(gMyQwNowRole)//��������
	{
		case QWDT_HOST:
		case QWDT_AUTO_HOST://������������Ϣ�����
			sprintf((void *)gMyQwAttributeStr,"HOST_%08X",GetHwID());
			gMyQwAddr=QW_ADDR_HOST;
			QW_SetMapBit(QW_ADDR_HOST);//��������λ
			QWeb_SetAddrMode(gMyQwAddr,TRUE);//�趨��ַ�����ܹ㲥
			QW_BuildInfoPacket((void *)pPacket,QW_ADDR_BCAST);//��info��
			if((Result=QW_CreatSession(&pSession,QWSI_HOST_INFO,pPacket,NULL,(QW_SessionCallBack)QW_SessionInfoHandler))!=QWR_SUCCESS)//���Ự
			{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
			QW_SessionSetReSend(pSession,QW_N_UMLIMITED,QW_T5);//�����ط�����
			QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//��һ�η��ͱ����ֶ�
			gMyQwStartTime=QW_GetNowTimeMs();

			QW_BuildQueryPacket((void *)pPacket,QW_ADDR_BCAST);//��query������ַ��Ч
			if((Result=QW_CreatSession(&pSession,QWSI_HOST_QUERY,pPacket,NULL,(QW_SessionCallBack)QW_SessionQueryHandler))!=QWR_SUCCESS)//���Ự
			{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
			QW_SessionSetReSend(pSession,QW_N_UMLIMITED,QW_T3);//�����ط�����
			QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//��һ�η��ͱ����ֶ�		
			break;
		case QWDT_SLAVE://���������Ͳ�ѯ��
			sprintf((void *)gMyQwAttributeStr,"SLAVE_%08X",GetHwID());
			gMyQwAddr=QW_ADDR_DEF;
			QWeb_SetAddrMode(gMyQwAddr,TRUE);//�趨��ַ�����ܹ㲥
			QW_BuildQueryPacket((void *)pPacket,QW_ADDR_HOST);//��query��
			if((Result=QW_CreatSession(&pSession,QWSI_SLAVE_QUERY,pPacket,NULL,(QW_SessionCallBack)QW_SessionQueryHandler))!=QWR_SUCCESS)//���Ự
			{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
			QW_SessionSetReSend(pSession,(gMyQwDrviceType==QWDT_SLAVE)?QW_N_UMLIMITED:QW_N1,QW_T1);//���ݽ�ɫ�����ط�����
			QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//��һ�η��ͱ����ֶ�
			break;
	}
	
	return QWR_SUCCESS;
}

u8 QW_WaitRecvSem(u16 WaitMs)
{
	u8 Error;

	CC2500_InterruptEnable(TRUE);//�����ж�
	CC2500_SetRxd();
	Error=OS_SemaphoreTake(gRfRecvHandler_Sem,OS_Ms2Tick(WaitMs));
	CC2500_InterruptEnable(FALSE);//�ر��ж�
	return Error;
}

extern void *QWebHandler_Task_Handle;
void QWebHandler_Task(void *Task_Parameters)
{
	u8 RF_Buf[QW_MAX_PACKET_LEN],Len;
	u8 RecvCount=0;//�յ��İ�����
	u32 TimeOutMs;
	u8 Result;
	QW_PACKET_HEADER *pPacket=(void *)RF_Buf;

	OS_TaskDelayMs(500);
	if(QW_HwSetup()==FALSE)
	{
		Debug("Read Rf chip id error!\n\r");
		OS_TaskSuspend(QWebHandler_Task_Handle);//ֹͣ���߳�
	}

QWebWaitStart:	
	gMyQwAttributeStr[0]=0;//q�������ַ���
	gMyQwID=0;//q��id���ɳ�������ʱ���������
	gMyQwAddr=QW_ADDR_DEF;//���豸��ǰq����ַ
	gMyQwAddrReqPw=Rand(0xff);//��¼�����ַʱ������
	gMyQwStartTime=0;//q�������ľ���ʱ�䣬��λ����
	gMyQwHostHwId=0;//�ӻ�������¼q������Ӳ���룬���������ϵ�������ʶ������
	QW_HostTableInit();
	QW_SessionsInit();
	OS_TaskSuspend(QWebHandler_Task_Handle);//ֹͣ���߳�

#if 0//for debug
	gMyQwID=1;
	gMyQwAddr=QW_ADDR_HOST;
	QWeb_SetAddrMode(gMyQwAddr,TRUE);//�趨��ַ�����ܹ㲥
	while(1)
	{
		QW_WaitRecvSem(0);
		Len=QW_RecvPacket(pPacket);
		if(Len)//if �յ���Ч��
		{		
			Debug("\n\r");
			QW_ShowPacket((void *)pPacket);
			//MemSet((void *)pPacket,0,sizeof(QW_PACKET));
		}
	}					
#endif	
	
	//��ʼ��.Initialize.
	Debug("QWeb Start up\n\rSizeof(QW_PACKET_HEADER)=%d\n\rSizeof(QW_PACKET_DATA)=%d\n\r",
				sizeof(QW_PACKET_HEADER),sizeof(QW_PACKET_DATA));
	Debug("Sizeof(QW_SESSION)=%d\n\r",sizeof(QW_SESSION));

	if(gMyQwDrviceType==QWDT_HOST) //�̶������趨��ɫ
		QW_ChangeRole(QWDT_HOST,GetHwID()&0xff);
	else//�Զ������͹̶��ӻ�����ʱ���Ƿ��͵�ַ�������ֻ����������ͬ
		QW_ChangeRole(QWDT_SLAVE,0);

//���������շ�״̬�����������������Ҫ���ڴ����ַά�����ˡ�
//�԰��Ĵ�����2�ִ�������
// 1.��������
// 2.�հ����������յ���֮�󣬱������
	while(1) 
	{
		TimeOutMs=QW_CalculateWaitTime();//����ȴ�ʱ��;
		QWU_Debug("Now %u,Wait %u\n\n\r",QW_GetNowTimeMs(),TimeOutMs);
		
		if(TimeOutMs)
			Result=QW_WaitRecvSem(TimeOutMs+1);//�ȴ����������ݷ�������
		else 
			Result=OS_ERR_TIMEOUT;
			
		if(Result==OS_ERR_NONE)//if �յ���Ч���������ݷ�������
		{
			if(gQWebApiFlag==TRUE)//Ӧ�ò�������Ҫ����
			{
				OS_DeclareCritical();
				OS_EnterCritical();
				gQWebApiFlag=FALSE;//��ԭ��־
				OS_ExitCritical();
							
				switch(gQWebUserData.CMD)
				{
					case QWAC_Stop://ֹͣq�����������
						goto QWebWaitStart;
					case QWAC_SendData:
						{
							QW_SESSION *pSession;					
							QW_Debug("Get User Send Data Cmd!Addr:%d,%d,%d\n\r",gQWebUserData.DstAddr,gQWebUserData.DataLen,QW_CalculatePacketTotal(gQWebUserData.DataLen));
							
							//������
							if(gQWebUserData.DataLen>QW_MAX_DATA_LEN)
								QW_BuildFristDataPacket((void *)pPacket,gQWebUserData.DstAddr,gQWebUserData.pData,
									QW_MAX_DATA_LEN,QW_CalculatePacketTotal(gQWebUserData.DataLen));
							else
								QW_BuildFristDataPacket((void *)pPacket,gQWebUserData.DstAddr,gQWebUserData.pData,
									gQWebUserData.DataLen,1);
							
							//����Session
							if((Result=QW_CreatSession(&pSession,QWSI_DATA_SEND,pPacket,&gQWebUserData,(QW_SessionCallBack)QW_SessionDataHandler))!=QWR_SUCCESS)//���Ự
							{QW_Debug("%s Error CreatSession!%d\n\r",__func__,Result);while(1);}
							QW_SessionSetReSend(pSession,QW_N2,QW_T2);//�����ط�����
							QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//��һ�η��ͱ����ֶ�			
						}
						break;
					case QWAC_QueryName:
						{
							QW_PACKET_HEADER *pPacket=QW_Mallco(sizeof(QW_PACKET_QUERY));
							QW_SESSION *pSession;
							QW_RESULT Result;

							QW_BuildQueryPacket((void *)pPacket,gQWebUserData.DstAddr);//��query��
							if((Result=QW_CreatSession(&pSession,QWSI_API_QUERY,pPacket,NULL,(QW_SessionCallBack)QW_SessionQueryHandler))!=QWR_SUCCESS)//���Ự
							{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
							QW_SessionSetReSend(pSession,1,QW_T1);//�����ط�����
							QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//��һ�η��ͱ����ֶ�	
							QW_Free(pPacket);
						}
						break;
				}
			}
			else
			{
				RecvCount++;
				Len=QW_RecvPacket(pPacket);
				if(Len)//if �յ���Ч��
				{		
					QW_SESSION *pSession;
					
					QW_ShowPacket((void *)pPacket);

					if(gMyQwNowRole!=QWDT_SLAVE)
						if(QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag)//�����豸�����ʱ��
							QW_GetAddrRecord(pPacket->SrcAddr)->LastActTimeMs=QW_GetNowTimeMs();
				
					if(QW_SessionFind(pPacket,&pSession)==QWR_SUCCESS)//		if�ӵ�ǰsession����Ѱ�Ƿ����Ѿ�������session���������
					{
						switch(pPacket->Type)
						{
							case QWPT_REQ_ADDR://��ַ��������������豸����ʱ����Ψһ�豸��ַ
							case QWPT_QUERY://��ѯ�������ڲ�ѯָ����ַ���豸�Ƿ����
							case QWPT_INFO://��Ϣ��������������������豸��Ϣ
							case QWPT_QUIT://�豸�˳����������豸�˳�			
							case QWPT_DATA://���ݰ��������豸����Զ��������շ�
								pSession->CallBackFunc(QWCA_RECV_REQ,0,pSession,pPacket);
								break;
							case QWPT_ACK_ADDR://��ַ����Ӧ������������������ַ���豸
							case QWPT_ACK_QUERY://��ѯӦ���������Ӧ���ѯ��
							case QWPT_ACK_QUIT://�豸�˳�Ӧ���������������Ӧ�豸�˳�
							case QWPT_ACK://��ȷӦ���������δ����Ӧ�����������Ӧ������
							case QWPT_ECK://����Ӧ���������δ����Ӧ����������Ĵ���Ӧ������
								//if(pPacket->LastChkSum==pSession->LastChkSum)
								pSession->CallBackFunc(QWCA_RECV_ACK,0,pSession,pPacket);
								break;
						}
					}
					else//		else���û�У������	
					{
						QWU_Debug("Not Find Session For This Packet!\n\r");
						switch(pPacket->Type)
						{
	 						case QWPT_REQ_ADDR:
	 							QW_SessionAddrHandler(QWCA_NEW_RECV,0,NULL,pPacket);
	 							break;
	 						case QWPT_QUERY:
	 							QW_SessionQueryHandler(QWCA_NEW_RECV,0,NULL,pPacket);
	 							break;
	 						case QWPT_INFO:
	 							QW_SessionInfoHandler(QWCA_NEW_RECV,0,NULL,pPacket);
	 							break;
	 						case QWPT_QUIT:
	 							QW_SessionQuitHandler(QWCA_NEW_RECV,0,NULL,pPacket);
	 							break;
	 						case QWPT_DATA:
								QW_SessionDataHandler(QWCA_NEW_RECV,0,NULL,pPacket);
	 							break;
	 						default:
	 							QWU_Debug("Error Packet type!\n\r");
						}
					}				
				}
				else//������Ϊ0
				{
					//QW_Debug("Not Recv Packet!\n\r");
					continue;
				}
			}
		}
		else//else ��ʱ�������¼������ڷ��Ͳ�ѯ��������Ƿ��ж�ʱ����
		{
			QWU_Debug("QWeb Wait TimeOut\n\r");
			QW_SessionTimeout();
		}
	}
}





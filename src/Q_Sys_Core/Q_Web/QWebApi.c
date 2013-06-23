/*
q���ӿ�
1.���ļ�ΪӦ�ò��q��Э��Ľӿڣ����Դ��ļ������к�������ֱ�Ӳ���q��������ֻ�ܴ������ݼ��¼�
2.�޸�q������ʱ�����ʹ���ٽ���

*/

#include "User.h"
#include "InputHandler.h"
#include "QWeb.h"
#include "QWebSession.h"
#include "QWebTransmission.h"
#include "QWebPacket.h"
#include "QWebCallBack.h"
#include "QWebAddrMgt.h"
#include "QWebApi.h"

extern void *QWebHandler_Task_Handle;
extern OS_SemaphoreHandle gRfRecvHandler_Sem;

static bool gQWebState=FALSE;
bool gQWebApiFlag=FALSE;
QWEB_DATA_STRUCT gQWebUserData;

//ֹͣq�����������ݻᱻ��գ�q���߳�Ҳ�����
void QWA_StopQWeb(void)
{
	gQWebState=FALSE;
	while(gQWebApiFlag==TRUE);
	{
		OS_DeclareCritical();
		OS_EnterCritical();
		gQWebUserData.CMD=QWAC_Stop;
		gQWebUserData.DstAddr=0;
		gQWebUserData.DataLen=0;
		gQWebUserData.pData=NULL;
		gQWebApiFlag=TRUE;
		OS_ExitCritical();
		OS_SemaphoreGive(gRfRecvHandler_Sem);
	}
}

//��q��
void QWA_StartQWeb(void)
{
	OS_TaskResume(QWebHandler_Task_Handle);
	gQWebState=TRUE;
}

//����true��ʾ���У�����false��ʾͣ��
bool QWA_QWebState(void)
{
	return gQWebState;
}

//ֻ�л�ȡ����Ч��ַ��Ӧ�ò���ܹ���ʼ����q��
u8 QWA_GetMyAddr(void)
{
	return gMyQwAddr;
}

//���ط�0��ʾ��ǰ��ַ����������
u8 QWA_QueryOnline(u8 Addr)
{
	return QW_ReadMapBit(Addr);
}

//��ȡ��һ����Ĵӻ���ַ
//SetFrist=TRUEʱ����ͷ��ʼ������ֵΪ0
//SetFrist=FALSEʱ����ѯ��һ������ĩβ�󣬷���0�����Զ�����ͷ��
u8 QWA_QueryNextOnline(bool SetFrist)
{
	return QW_FindNextActAddr(SetFrist);
}

//�ȴ�ͬ�����ز�ѯ��
QW_RESULT QWA_QueryName(u8 Addr)
{
	if(((Addr<QW_ADDR_MIN)||(Addr>QW_ADDR_MAX))&&(Addr!=QW_ADDR_HOST))
	return QWR_NO_SUCH_ADDR;
	
	if(QW_ReadMapBit(Addr))
	{
		while(gQWebApiFlag==TRUE);
		{
			OS_DeclareCritical();
			OS_EnterCritical();
			gQWebUserData.CMD=QWAC_QueryName;
			gQWebUserData.DstAddr=Addr;
			gQWebUserData.DataLen=0;
			gQWebUserData.pData=NULL;
			gQWebApiFlag=TRUE;
			OS_ExitCritical();
			return (QW_RESULT)OS_SemaphoreGive(gRfRecvHandler_Sem);
		}
	}
	else
	{
		return QWR_NO_SUCH_ADDR;
	}
}

//NewName��Ϊ��ʱ���޸��Լ�������ΪNewName
//һֱ���ص�ǰ����
u8 *QWA_MyQWebName(u8 *NewName)
{
	u8 Len;

	if(NewName&&NewName[0])
	{
		Len=strlen((void *)NewName);

		if(Len>=QW_ATTRIBUTE_STR_MAX_LEN) Len=QW_ATTRIBUTE_STR_MAX_LEN-1;

		MemCpy(gMyQwAttributeStr,NewName,Len);//����???
		gMyQwAttributeStr[Len]=0;	
	}

	return gMyQwAttributeStr;
}

//Ӧ�ó���������
//pData�����send ok����send failed֮����ܱ��ͷ�
QW_RESULT QWA_SendData(u8 Addr,u32 DataLen,u8 *pData)
{
	if(DataLen>QW_MAX_MALLCO_BYTES) return QWR_BUF_UN_ENOUGH;
	
	while(gQWebApiFlag==TRUE);
	{
		OS_DeclareCritical();
		OS_EnterCritical();
		gQWebUserData.CMD=QWAC_SendData;
		gQWebUserData.DstAddr=Addr;
		gQWebUserData.DataLen=DataLen;
		gQWebUserData.pData=pData;
		gQWebApiFlag=TRUE;
		OS_ExitCritical();
		Debug("QW Send Data @ %d\n\r",QW_GetNowTimeMs());
		return (QW_RESULT)OS_SemaphoreGive(gRfRecvHandler_Sem);
	}
}

//���Դ�ӡ
void QWA_Show(void)
{
	QW_SESSION *pSession;
	u8 Addr;
	
	Debug("QWeb Show\n\r");
	QW_SessionGet(TRUE);
	while((pSession=QW_SessionGet(FALSE))!=NULL)
		if(pSession->Type!=QWST_UNUSE)
			QW_ShowSession(pSession,TRUE);

	//Debug("Table:");
	//DisplayBuf((void *)QW_GetHostTableMap(),QW_HOST_MAP_BYTES,16);
	Debug("My Addr[%d], Online: ",gMyQwAddr);
	QW_FindNextActAddr(TRUE);
	while((Addr=QW_FindNextActAddr(FALSE))!=0)
		Debug("[%d] ",Addr);
	Debug("\n\r");
}

//Ӧ�ò�����������¼�����Ӧ�����ִ�д˺������Է�ֹ�ڴ�й¶
//QWE_NewJoin
//QWE_Recv
//QWE_QueryAck
void QWA_CopyDataFinish(void *Ptr)
{
	QW_Free(Ptr);
}

//q�������źŸ�Ӧ�ó���
void QW_Sync(QW_EVT Evt,u8 Addr,u32 DataLen,u8 *pData)
{
	INPUT_EVENT IE;
	
	switch(Evt)
	{
		case QWE_NewJoin://���ڴ�й¶Σ�գ�������Ҫ����QW_CopyDataFinish
			{
				u8 *p=QW_Mallco(QW_ATTRIBUTE_STR_MAX_LEN);
				IE.uType=Sync_Type;
				IE.EventType=Input_QWebEvt;
				IE.Num=QWE_NewJoin;
				IE.Info.SyncInfo.IntParam=Addr;//addr
				IE.Info.SyncInfo.pParam=p;//name
				MemCpy(p,pData,QW_ATTRIBUTE_STR_MAX_LEN);
				p[QW_ATTRIBUTE_STR_MAX_LEN-1]=0;
				OS_MsgBoxSend(gInputHandler_Queue,&IE,1000,FALSE);
			}
			break;
		case QWE_Recv://���ڴ�й¶Σ�գ�������Ҫ����QW_CopyDataFinish
			Debug("$$$ QW_SYSC Recv %d @ %d:\n\r",DataLen,QW_GetNowTimeMs());
			//DisplayBuf(pData,DataLen,16);
			IE.uType=Sync_Type;
			IE.EventType=Input_QWebEvt;
			IE.Num=QWE_Recv;
			IE.Info.SyncInfo.IntParam=(Addr<<24)|DataLen;//data len��������ʱ���ᳬ��0xffffff�������ø�8λ��ʾ��ַ
			IE.Info.SyncInfo.pParam=pData;//data
			OS_MsgBoxSend(gInputHandler_Queue,&IE,1000,FALSE);
			break;
		case QWE_SendOk:
			Debug("$$$ QW_SYSC Send Finish @ %d\n\r",QW_GetNowTimeMs());
			IE.uType=Sync_Type;
			IE.EventType=Input_QWebEvt;
			IE.Num=QWE_SendOk;
			IE.Info.SyncInfo.IntParam=(Addr<<24)|DataLen;
			IE.Info.SyncInfo.pParam=pData;
			OS_MsgBoxSend(gInputHandler_Queue,&IE,1000,FALSE);
			break;
		case QWE_SendFailed:
			Debug("$$$ QW_SYSC Send Work UnFinish @ %d\n\r",QW_GetNowTimeMs());
			IE.uType=Sync_Type;
			IE.EventType=Input_QWebEvt;
			IE.Num=QWE_SendFailed;
			IE.Info.SyncInfo.IntParam=(Addr<<24)|DataLen;
			IE.Info.SyncInfo.pParam=pData;
			OS_MsgBoxSend(gInputHandler_Queue,&IE,1000,FALSE);
			break;
		case QWE_HostConflict:
			break;
		case QWE_QueryAck://���ڴ�й¶Σ�գ�������Ҫ����QW_CopyDataFinish
			{
				u8 *p=QW_Mallco(QW_ATTRIBUTE_STR_MAX_LEN);
				IE.uType=Sync_Type;
				IE.EventType=Input_QWebEvt;
				IE.Num=QWE_QueryAck;
				IE.Info.SyncInfo.IntParam=Addr;//add
				IE.Info.SyncInfo.pParam=p;//name
				MemCpy(p,pData,QW_ATTRIBUTE_STR_MAX_LEN);
				p[QW_ATTRIBUTE_STR_MAX_LEN-1]=0;
				OS_MsgBoxSend(gInputHandler_Queue,&IE,1000,FALSE);
			}
			break;
	}
}





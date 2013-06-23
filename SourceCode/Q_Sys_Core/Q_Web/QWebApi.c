/*
q网接口
1.此文件为应用层和q网协议的接口，所以此文件内所有函数不能直接操作q网动作，只能传递数据及事件
2.修改q网数据时，务必使用临界区

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

//停止q网，所有数据会被清空，q网线程也会挂起
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

//打开q网
void QWA_StartQWeb(void)
{
	OS_TaskResume(QWebHandler_Task_Handle);
	gQWebState=TRUE;
}

//返回true表示运行，返回false表示停在
bool QWA_QWebState(void)
{
	return gQWebState;
}

//只有获取到有效地址后，应用层才能够开始操作q网
u8 QWA_GetMyAddr(void)
{
	return gMyQwAddr;
}

//返回非0表示当前地址的主机存在
u8 QWA_QueryOnline(u8 Addr)
{
	return QW_ReadMapBit(Addr);
}

//获取下一个活动的从机地址
//SetFrist=TRUE时，从头开始，返回值为0
//SetFrist=FALSE时，轮询下一个，到末尾后，返回0，并自动返回头部
u8 QWA_QueryNextOnline(bool SetFrist)
{
	return QW_FindNextActAddr(SetFrist);
}

//等待同步返回查询名
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

//NewName不为空时，修改自己的网名为NewName
//一直返回当前网名
u8 *QWA_MyQWebName(u8 *NewName)
{
	u8 Len;

	if(NewName&&NewName[0])
	{
		Len=strlen((void *)NewName);

		if(Len>=QW_ATTRIBUTE_STR_MAX_LEN) Len=QW_ATTRIBUTE_STR_MAX_LEN-1;

		MemCpy(gMyQwAttributeStr,NewName,Len);//互斥???
		gMyQwAttributeStr[Len]=0;	
	}

	return gMyQwAttributeStr;
}

//应用程序发送数据
//pData必须等send ok或者send failed之后才能被释放
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

//调试打印
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

//应用层调用完如下事件的响应后必须执行此函数，以防止内存泄露
//QWE_NewJoin
//QWE_Recv
//QWE_QueryAck
void QWA_CopyDataFinish(void *Ptr)
{
	QW_Free(Ptr);
}

//q网发送信号给应用程序
void QW_Sync(QW_EVT Evt,u8 Addr,u32 DataLen,u8 *pData)
{
	INPUT_EVENT IE;
	
	switch(Evt)
	{
		case QWE_NewJoin://有内存泄露危险，用完需要调用QW_CopyDataFinish
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
		case QWE_Recv://有内存泄露危险，用完需要调用QW_CopyDataFinish
			Debug("$$$ QW_SYSC Recv %d @ %d:\n\r",DataLen,QW_GetNowTimeMs());
			//DisplayBuf(pData,DataLen,16);
			IE.uType=Sync_Type;
			IE.EventType=Input_QWebEvt;
			IE.Num=QWE_Recv;
			IE.Info.SyncInfo.IntParam=(Addr<<24)|DataLen;//data len，数据暂时不会超过0xffffff，所以用高8位表示地址
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
		case QWE_QueryAck://有内存泄露危险，用完需要调用QW_CopyDataFinish
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





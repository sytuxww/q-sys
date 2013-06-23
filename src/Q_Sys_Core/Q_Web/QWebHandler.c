//---------------------------------------------------------------------
// 本代码根据Q网协议编写，数据传输细节请参考Q网协议规范
// Q网协议--由酷享科技制定的半双工多设备简单网络协议
//---------------------------------------------------------------------
/*
q网的处理，有如下三种状态
1.主动发送，则新建session来处理回包
2.定时发送，用于处理超时未回复的包
3.处理回包
*/
/*
q网程序基本构架
1.每种类型的包有其对应的callback函数来处理它
2.有应有答的一次完整传输，我们称之为session[会话]
3.为了保存会话，通过QW_SESSION结构体来保存数据
4.QW_SESSION根据需要会申请内存保存首包及收到的data
5.程序处理完各种事务后，会计算下一个事务的时间，当时间到期，或者收到新包，或者收到应用层请求时，其会进行动作
6.session在一般情况下，如果超时，就会被注销
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
extern OS_SemaphoreHandle gRfRecvHandler_Sem;//射频接收数据的信号量
extern bool gQWebApiFlag;
extern QWEB_DATA_STRUCT gQWebUserData;

//----------------------public--------------------------
const QW_DEVICE_TYPE gMyQwDrviceType=QWDT_AUTO_HOST;//固定的设备类型
u8 gMyQwAttributeStr[QW_ATTRIBUTE_STR_MAX_LEN]="";//q网属性字符串

QW_DEVICE_TYPE gMyQwNowRole;//当前角色
u8 gMyQwID=0;//q网id，由程序启动时生成随机码
u8 gMyQwAddr=QW_ADDR_DEF;//本设备当前q网地址
u8 gMyQwAddrReqPw=0;//记录请求地址时的密码
u32 gMyQwStartTime=0;//q网启动的绝对时间，单位毫秒
u32 gMyQwHostHwId=0;//从机用来记录q网主机硬件码，借以主机断点重启后识别主机

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

//主动设定角色
//如果设置为主机，则开始发送信息宣告包
//如果设置为从机，则开始发送地址请求包
//因为收到应答包而变更为从机，不能调用此函数!!!
//因为此函数会导致所有session销毁，所以在callback里面慎用!!!
QW_RESULT QW_ChangeRole(QW_DEVICE_TYPE Role,u8 QWebID)
{
	QW_RESULT Result;
	u8 RF_Buf[QW_MAX_PACKET_LEN];
	QW_PACKET_HEADER *pPacket=(void *)RF_Buf;
	QW_SESSION *pSession;

	QW_Debug("#Change Role <%s> ID:%x\n\r",gQwDeviceName[Role],QWebID);

	switch(Role)//角色检查
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
	gMyQwID=QWebID;//设定q网id

	//变换角色后，删除所有Session
	QW_SessionGet(TRUE);
	while((pSession=QW_SessionGet(FALSE))!=NULL)
		QW_DestroySession(pSession);

	QW_HostTableInit();//清空地址表
	
	switch(gMyQwNowRole)//发包处理
	{
		case QWDT_HOST:
		case QWDT_AUTO_HOST://构建并发送信息宣告包
			sprintf((void *)gMyQwAttributeStr,"HOST_%08X",GetHwID());
			gMyQwAddr=QW_ADDR_HOST;
			QW_SetMapBit(QW_ADDR_HOST);//设置主机位
			QWeb_SetAddrMode(gMyQwAddr,TRUE);//设定地址，接受广播
			QW_BuildInfoPacket((void *)pPacket,QW_ADDR_BCAST);//建info包
			if((Result=QW_CreatSession(&pSession,QWSI_HOST_INFO,pPacket,NULL,(QW_SessionCallBack)QW_SessionInfoHandler))!=QWR_SUCCESS)//建会话
			{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
			QW_SessionSetReSend(pSession,QW_N_UMLIMITED,QW_T5);//设置重发参数
			QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//第一次发送必须手动
			gMyQwStartTime=QW_GetNowTimeMs();

			QW_BuildQueryPacket((void *)pPacket,QW_ADDR_BCAST);//建query包，地址无效
			if((Result=QW_CreatSession(&pSession,QWSI_HOST_QUERY,pPacket,NULL,(QW_SessionCallBack)QW_SessionQueryHandler))!=QWR_SUCCESS)//建会话
			{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
			QW_SessionSetReSend(pSession,QW_N_UMLIMITED,QW_T3);//设置重发参数
			QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//第一次发送必须手动		
			break;
		case QWDT_SLAVE://构建并发送查询包
			sprintf((void *)gMyQwAttributeStr,"SLAVE_%08X",GetHwID());
			gMyQwAddr=QW_ADDR_DEF;
			QWeb_SetAddrMode(gMyQwAddr,TRUE);//设定地址，接受广播
			QW_BuildQueryPacket((void *)pPacket,QW_ADDR_HOST);//建query包
			if((Result=QW_CreatSession(&pSession,QWSI_SLAVE_QUERY,pPacket,NULL,(QW_SessionCallBack)QW_SessionQueryHandler))!=QWR_SUCCESS)//建会话
			{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
			QW_SessionSetReSend(pSession,(gMyQwDrviceType==QWDT_SLAVE)?QW_N_UMLIMITED:QW_N1,QW_T1);//根据角色构建重发参数
			QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//第一次发送必须手动
			break;
	}
	
	return QWR_SUCCESS;
}

u8 QW_WaitRecvSem(u16 WaitMs)
{
	u8 Error;

	CC2500_InterruptEnable(TRUE);//开启中断
	CC2500_SetRxd();
	Error=OS_SemaphoreTake(gRfRecvHandler_Sem,OS_Ms2Tick(WaitMs));
	CC2500_InterruptEnable(FALSE);//关闭中断
	return Error;
}

extern void *QWebHandler_Task_Handle;
void QWebHandler_Task(void *Task_Parameters)
{
	u8 RF_Buf[QW_MAX_PACKET_LEN],Len;
	u8 RecvCount=0;//收到的包计数
	u32 TimeOutMs;
	u8 Result;
	QW_PACKET_HEADER *pPacket=(void *)RF_Buf;

	OS_TaskDelayMs(500);
	if(QW_HwSetup()==FALSE)
	{
		Debug("Read Rf chip id error!\n\r");
		OS_TaskSuspend(QWebHandler_Task_Handle);//停止本线程
	}

QWebWaitStart:	
	gMyQwAttributeStr[0]=0;//q网属性字符串
	gMyQwID=0;//q网id，由程序启动时生成随机码
	gMyQwAddr=QW_ADDR_DEF;//本设备当前q网地址
	gMyQwAddrReqPw=Rand(0xff);//记录请求地址时的密码
	gMyQwStartTime=0;//q网启动的绝对时间，单位毫秒
	gMyQwHostHwId=0;//从机用来记录q网主机硬件码，借以主机断点重启后识别主机
	QW_HostTableInit();
	QW_SessionsInit();
	OS_TaskSuspend(QWebHandler_Task_Handle);//停止本线程

#if 0//for debug
	gMyQwID=1;
	gMyQwAddr=QW_ADDR_HOST;
	QWeb_SetAddrMode(gMyQwAddr,TRUE);//设定地址，接受广播
	while(1)
	{
		QW_WaitRecvSem(0);
		Len=QW_RecvPacket(pPacket);
		if(Len)//if 收到有效包
		{		
			Debug("\n\r");
			QW_ShowPacket((void *)pPacket);
			//MemSet((void *)pPacket,0,sizeof(QW_PACKET));
		}
	}					
#endif	
	
	//初始化.Initialize.
	Debug("QWeb Start up\n\rSizeof(QW_PACKET_HEADER)=%d\n\rSizeof(QW_PACKET_DATA)=%d\n\r",
				sizeof(QW_PACKET_HEADER),sizeof(QW_PACKET_DATA));
	Debug("Sizeof(QW_SESSION)=%d\n\r",sizeof(QW_SESSION));

	if(gMyQwDrviceType==QWDT_HOST) //固定主机设定角色
		QW_ChangeRole(QWDT_HOST,GetHwID()&0xff);
	else//自动主机和固定从机启动时都是发送地址请求包，只不过次数不同
		QW_ChangeRole(QWDT_SLAVE,0);

//进入正常收发状态，如果是主机，则需要定期处理地址维护事宜。
//对包的处理，有2种触发条件
// 1.主动发包
// 2.收包解析，当收到包之后，必须解析
	while(1) 
	{
		TimeOutMs=QW_CalculateWaitTime();//计算等待时间;
		QWU_Debug("Now %u,Wait %u\n\n\r",QW_GetNowTimeMs(),TimeOutMs);
		
		if(TimeOutMs)
			Result=QW_WaitRecvSem(TimeOutMs+1);//等待包或者数据发送请求
		else 
			Result=OS_ERR_TIMEOUT;
			
		if(Result==OS_ERR_NONE)//if 收到有效包或者数据发送请求
		{
			if(gQWebApiFlag==TRUE)//应用层有数据要发送
			{
				OS_DeclareCritical();
				OS_EnterCritical();
				gQWebApiFlag=FALSE;//还原标志
				OS_ExitCritical();
							
				switch(gQWebUserData.CMD)
				{
					case QWAC_Stop://停止q网，清空数据
						goto QWebWaitStart;
					case QWAC_SendData:
						{
							QW_SESSION *pSession;					
							QW_Debug("Get User Send Data Cmd!Addr:%d,%d,%d\n\r",gQWebUserData.DstAddr,gQWebUserData.DataLen,QW_CalculatePacketTotal(gQWebUserData.DataLen));
							
							//构建包
							if(gQWebUserData.DataLen>QW_MAX_DATA_LEN)
								QW_BuildFristDataPacket((void *)pPacket,gQWebUserData.DstAddr,gQWebUserData.pData,
									QW_MAX_DATA_LEN,QW_CalculatePacketTotal(gQWebUserData.DataLen));
							else
								QW_BuildFristDataPacket((void *)pPacket,gQWebUserData.DstAddr,gQWebUserData.pData,
									gQWebUserData.DataLen,1);
							
							//构建Session
							if((Result=QW_CreatSession(&pSession,QWSI_DATA_SEND,pPacket,&gQWebUserData,(QW_SessionCallBack)QW_SessionDataHandler))!=QWR_SUCCESS)//建会话
							{QW_Debug("%s Error CreatSession!%d\n\r",__func__,Result);while(1);}
							QW_SessionSetReSend(pSession,QW_N2,QW_T2);//设置重发参数
							QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//第一次发送必须手动			
						}
						break;
					case QWAC_QueryName:
						{
							QW_PACKET_HEADER *pPacket=QW_Mallco(sizeof(QW_PACKET_QUERY));
							QW_SESSION *pSession;
							QW_RESULT Result;

							QW_BuildQueryPacket((void *)pPacket,gQWebUserData.DstAddr);//建query包
							if((Result=QW_CreatSession(&pSession,QWSI_API_QUERY,pPacket,NULL,(QW_SessionCallBack)QW_SessionQueryHandler))!=QWR_SUCCESS)//建会话
							{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
							QW_SessionSetReSend(pSession,1,QW_T1);//设置重发参数
							QW_SessionCmd(pSession,QWC_SEND_FRIST_PKT,pPacket,0);//第一次发送必须手动	
							QW_Free(pPacket);
						}
						break;
				}
			}
			else
			{
				RecvCount++;
				Len=QW_RecvPacket(pPacket);
				if(Len)//if 收到有效包
				{		
					QW_SESSION *pSession;
					
					QW_ShowPacket((void *)pPacket);

					if(gMyQwNowRole!=QWDT_SLAVE)
						if(QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag)//更新设备的最后活动时间
							QW_GetAddrRecord(pPacket->SrcAddr)->LastActTimeMs=QW_GetNowTimeMs();
				
					if(QW_SessionFind(pPacket,&pSession)==QWR_SUCCESS)//		if从当前session中找寻是否有已经建立的session，并处理包
					{
						switch(pPacket->Type)
						{
							case QWPT_REQ_ADDR://地址请求包，用于新设备加入时请求唯一设备地址
							case QWPT_QUERY://查询包，用于查询指定地址的设备是否存在
							case QWPT_INFO://信息宣告包，用于主动发送设备信息
							case QWPT_QUIT://设备退出包，用于设备退出			
							case QWPT_DATA://数据包，用于设备间的自定义数据收发
								pSession->CallBackFunc(QWCA_RECV_REQ,0,pSession,pPacket);
								break;
							case QWPT_ACK_ADDR://地址请求应答包，用于主机分配地址给设备
							case QWPT_ACK_QUERY://查询应答包，用于应答查询包
							case QWPT_ACK_QUIT://设备退出应答包，用于主机响应设备退出
							case QWPT_ACK://正确应答包，用于未定义应答包的主包的应答工作。
							case QWPT_ECK://错误应答包，用于未定义应答包的主包的错误应答工作。
								//if(pPacket->LastChkSum==pSession->LastChkSum)
								pSession->CallBackFunc(QWCA_RECV_ACK,0,pSession,pPacket);
								break;
						}
					}
					else//		else如果没有，则处理包	
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
				else//包长度为0
				{
					//QW_Debug("Not Recv Packet!\n\r");
					continue;
				}
			}
		}
		else//else 超时，处理事件，定期发送查询包，检查是否有定时事务
		{
			QWU_Debug("QWeb Wait TimeOut\n\r");
			QW_SessionTimeout();
		}
	}
}





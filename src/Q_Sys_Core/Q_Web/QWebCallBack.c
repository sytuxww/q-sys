/*
q网callback基本构架
1.每种类型的包，均有一个callback来处理它
2.callback被调用有多种时机，比如收到新包[QWCA_NEW_RECV]，发送前[QWCA_BF_SEND]，
	发送后[QWCA_AF_SEND]，收到请求[QWCA_RECV_REQ]，收到应答[QWCA_RECV_ACK]，超时[QWCA_TIMEOUT]
3.处理时，务必对自己的固定角色和当前角色及当前状态做判断，否则会导致处理错误
4.当有事件及数据要丢给应用层时，务必使用QW_Sync函数

*/

#include "QWeb.h"
#include "QWebSession.h"
#include "QWebCallBack.h"
#include "QWebTransmission.h"
#include "QWebAddrMgt.h"

//callback要做的事情
// 1.QWCA_NEW_RECV里面，要进行角色检查，以及包的检验

QW_RESULT QW_SessionAddrHandler(QW_CALLBACK_ACT State,int IntParam,QW_SESSION *pSession,QW_PACKET_HEADER *pPacket)
{
	QW_PACKET_REQ_ADDR *pReqAddrPkt=(void *)pPacket;
	QW_PACKET_ACK_ADDR *pAckAddrPkt=(void *)pPacket;
	//QW_Debug("%s:%s\n\r",__func__,gSessionStateName[State]);

	switch(State)
	{
		case QWCA_NEW_RECV://直接处理，pSession=NULL,还没有Session
			QW_Debug("Get a new addr req!\n\r");
			if(gMyQwNowRole==QWDT_SLAVE)
			{
				QW_Debug("I am a Slave!\n\r");
				return QWR_NOHANDLE;//如果是从机，就不处理
			}
			else //分配地址
			{
				QW_PACKET_ACK_ADDR *pAckPktTmp=QW_Mallco(sizeof(QW_PACKET_ACK_ADDR));
				u8 GiveAddr=pReqAddrPkt->HopeAddr;

				switch(GiveAddr)
				{
					case QW_ADDR_BCAST:
					case QW_ADDR_HOST:
					case QW_ADDR_DEF://请求地址不对
						GiveAddr=QW_ADDR_MIN;
						break;
				}
				
				if(QW_GetAddrRecord(GiveAddr)->UseFlag!=0)//查看地址表，该地址被占用
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
						return QWR_ADDR_FULL;//地址满，回包，建立查询session，unfinish
					}
				}

				//登记分配地址
				QW_GetAddrRecord(GiveAddr)->UseFlag=1;
				QW_GetAddrRecord(GiveAddr)->Password=pReqAddrPkt->Password;//记录密码
				QW_GetAddrRecord(GiveAddr)->LastActTimeMs=QW_GetNowTimeMs();
				QW_SetMapBit(GiveAddr);
				QW_Sync(QWE_NewJoin,GiveAddr,QW_ATTRIBUTE_STR_MAX_LEN,pReqAddrPkt->AttributeStr);
				QW_Debug("Give Addr is 0x%x\n\r",GiveAddr);

				//回包
				QW_BuildAckAddrPacket(pAckPktTmp,pPacket->ChkSum,pPacket->SrcAddr,GiveAddr);
				QW_SendPacket((QW_PACKET_HEADER *)pAckPktTmp);
				QW_Free(pAckPktTmp);
			}	
			break;
		case QWCA_RECV_ACK://接收，发送请求包之后，收取到ack应答
			if(gMyQwNowRole==QWDT_SLAVE)//从机
			{
				QW_Debug("Get My Addr:%x,QWeb ID:%x\n\r",pAckAddrPkt->GiveAddr,pPacket->QW_ID);
				gMyQwNowRole=QWDT_SLAVE;
				gMyQwID=pPacket->QW_ID;//设定q网id
				gMyQwAddr=pAckAddrPkt->GiveAddr;
				QWeb_SetAddrMode(gMyQwAddr,TRUE);//设定地址，接受广播
				MemCpy((void *)QW_GetHostTableMap(),pAckAddrPkt->HostTableMap,QW_HOST_MAP_BYTES);

				QW_Sync(QWE_NewJoin,QW_ADDR_HOST,QW_ATTRIBUTE_STR_MAX_LEN,pAckAddrPkt->AttributeStr);
				QW_DestroySession(pSession);
			}
			else
			{
				Debug("QW_SessionAddrHandler Host error!!!");
			}
			break;
		case QWCA_TIMEOUT://最后一次发包后，间隔GapMs触发。触发完本事件，则当前session销毁。
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
		case QWCA_NEW_RECV://收到请求，发送ack
			if(pQueryPkt->QueryAddr!=gMyQwAddr) break;
			
			if(gMyQwNowRole==QWDT_SLAVE)//如果是从机
			{
				QW_PACKET_ACK_QUERY *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK_QUERY));
				QW_BuildAckQueryPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
				QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
				QW_Free(pPacketTemp);
			}
			else  //如果是主机，随机时间回info包
			{
				QW_SESSION *pSessionTemp=QW_SessionGetById(QWSI_HOST_INFO);
				if(pSessionTemp!=NULL)
				{
					u32 t=Rand(0xffff)%QW_T1;//Debug("Set Info time:%d\n\r",t);
					QW_SessionCmd(pSessionTemp,QWC_SET_SEND_TIME,NULL,t);//随即时间后发送info
				}
			}
			break;
		case QWCA_BF_SEND:
			if(gMyQwNowRole==QWDT_SLAVE)
			{

			}
			else//如果是主机，地址维护
			{
				if(pSession->ID==QWSI_HOST_QUERY)//主机地址维护
				{
					u8 Addr;
					u32 NowMs=QW_GetNowTimeMs();
					
					while((Addr=QW_FindNextActAddr(FALSE))!=0)//循环查询设备
					{
						if(Addr==QW_ADDR_HOST) continue;//跳过主机自己
						if((QW_GetAddrRecord(Addr)->UseFlag)&&((NowMs-QW_GetAddrRecord(Addr)->LastActTimeMs)>QW_T6))//超时的就发包
						{
							if((NowMs-QW_GetAddrRecord(Addr)->LastActTimeMs)>(QW_T6<<1))//超时1倍，直接删除
							{
								QW_SESSION *pSessionTemp;

								QW_GetAddrRecord(Addr)->UseFlag=QW_GetAddrRecord(Addr)->Password=0;
								QW_ClrMapBit(Addr);
								QW_Debug("Delete Addr[%d] Device!\n\r",Addr);

								//删除从机后必须立刻发送info包
								pSessionTemp=QW_SessionGetById(QWSI_HOST_INFO);
								if(pSessionTemp!=NULL)
									QW_SessionCmd(pSessionTemp,QWC_SET_SEND_TIME,NULL,100);//发送info
							}
							else
							{
								//改包
								QW_BuildQueryPacket(pQueryPkt,Addr);
								break;//每次只问一个即可
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
			if((pSession->ID==QWSI_SLAVE_QUERY)&&(gMyQwDrviceType==QWDT_AUTO_HOST)&&(gMyQwNowRole==QWDT_SLAVE)&&(pSession->DstAddr==QW_ADDR_HOST))//query超时也没有收到info，直接变更为主机
			{
				if((Error=QW_ChangeRole(QWDT_AUTO_HOST,GetHwID()&0xff))!=QWR_SUCCESS)//变为主机
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
		case QWCA_NEW_RECV://直接处理，pSession=NULL,不需要Session
			if(gMyQwNowRole==QWDT_SLAVE)//如果是从机，接收id，选择后设置自身id，删除query，然后发送地址请求
			{
				if(gMyQwID==0)//还没找到网
				{
					QW_SESSION *pSessionTemp;

					//选择q网,unfinish
					if((gMyQwHostHwId)&&(gMyQwHostHwId!=pInfoPkt->HwID))
					{
						QW_Debug("QW HW ID error!%x %x\n\r",gMyQwHostHwId,pInfoPkt->HwID);
						return QWR_SUCCESS;
					}
					
					//设置q网id
					gMyQwID=pPacket->QW_ID;
					gMyQwHostHwId=pInfoPkt->HwID;

					//删除slaver frist query
					pSessionTemp=QW_SessionGetById(QWSI_SLAVE_QUERY);
					if(pSessionTemp!=NULL)
						QW_DestroySession(pSessionTemp);

					{//建立addr req
						QW_RESULT Result;
						QW_PACKET_REQ_ADDR *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_REQ_ADDR));
						QW_SESSION *pSessionTemp;
						
						QW_BuildReqAddrPacket(pPacketTemp,QW_MY_HOPE_ADDR,gMyQwAddrReqPw);//建包
						if((Result=QW_CreatSession(&pSessionTemp,QWSI_SLAVE_REQ_ADDR,(void *)pPacketTemp,NULL,(QW_SessionCallBack)QW_SessionAddrHandler))!=QWR_SUCCESS)//建会话
						{QW_Debug("%s ERROR CreatSession!%d\n\r",__func__,Result);while(1);}
						QW_SessionSetReSend(pSessionTemp,QW_N3,QW_T7);
						QW_SessionCmd(pSessionTemp,QWC_SEND_FRIST_PKT,(void *)pPacketTemp,0);//第一次发送必须手动
						QW_Free(pPacketTemp);
					}
				}
				else //自己是从机，并且已经有网络id
				{
					QWU_Debug("Get Info:%s\n\r",pInfoPkt->AttributeStr);
					
					//更新table
					MemCpy((void *)QW_GetHostTableMap(),pInfoPkt->HostTableMap,QW_HOST_MAP_BYTES);
					QW_Sync(QWE_NewJoin,pPacket->SrcAddr,QW_ATTRIBUTE_STR_MAX_LEN,pInfoPkt->AttributeStr);
					
					//判断地址表中有没有自己
					if(QW_ReadMapBit(gMyQwAddr)==0)
					{//如果是，则再次请求地址
						QW_Debug("QW Host Reset!!!\n\r");
						QW_ChangeRole(QWDT_SLAVE,0);
					}
				}				
			}
			else //自己是主机，如果收到info信息，则处理主机冲突
			{
				QW_Debug("------!!!Host Conflict!!!------\n\r");

				//此处根据启动时间仲裁
				OpponentStartTime=pInfoPkt->HostStartTime;
				MyStartTime=QW_GetNowTimeMs()-gMyQwStartTime;
				if(OpponentStartTime>MyStartTime)//自己的启动时间比较短，牺牲自己
				{
					if(QW_ChangeRole(QWDT_SLAVE,0)==QWR_SUCCESS)//主机冲突，变更自己为从机
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
				
				return QWR_HOST_CONFLICT;//未变更成功，报错
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
		case QWCA_NEW_RECV://收到请求，发送ack
			if(gMyQwNowRole==QWDT_SLAVE)
			{

			}
			else//如果是主机，则响应
			{
				if((QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag)&&(pQuitPkt->Password==QW_GetAddrRecord(pPacket->SrcAddr)->Password))//密码正确
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
		case QWCA_NEW_RECV://直接处理，pSession=NULL,还没有Session
			//先确定这个设备是否合法
			if(gMyQwNowRole==QWDT_SLAVE)//如果是从机，则查询此地址设备是否存在
			{
			
			}
			else if(QW_GetAddrRecord(pPacket->SrcAddr)->UseFlag==0) 
						return QWR_NO_SUCH_ADDR;
			
			if(pPacket->Num!=1) return QWR_REQ_ERROR;

			{//建data recv session
				QW_PACKET_ACK *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ACK));
				
				if((Result=QW_CreatSession(&pSessionTemp,QWSI_DATA_RECV,pPacket,NULL,(QW_SessionCallBack)QW_SessionDataHandler))!=QWR_SUCCESS)//建会话
				{QW_Debug("%s Error CreatSession!%d\n\r",__func__,Result);while(1);}		
				QW_SessionSetRecvTimeOut(pSessionTemp,QW_N2*QW_T2);//设置超时参数
				
				//回包				
				QW_BuildAckPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr);
				QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
				QW_Free(pPacketTemp);
			}

			if(pPacket->Total==1)//data分包只有1个，直接发同步信息
			{
				u8 DataLen=pPacket->Len-QW_PKT_MIN_LEN;
				u8 *pData=QW_Mallco(DataLen);//分配的内存将在应用层调用QW_CopyDataFinish时释放
				MemCpy(pData,pDataPkt->Data,DataLen);
				QW_Sync(QWE_Recv,pPacket->SrcAddr,DataLen,pData);
			}
			break;
		case QWCA_RECV_REQ://非第一次接收到包，有匹配session
			if(pPacket->Num==(pSession->PacketNum+1))//正确包
			{
				if(QW_SessionCmd(pSession,QWC_RECV_DATA,pPacket,0)==QWR_SUCCESS)
				{
					if(QW_SessionCmd(pSession,QWC_SEND_ACK,pPacket,0)!=QWR_SUCCESS) return QWR_FAILED;

					if(pSession->PacketNum==pSession->PacketTotal)//收包完成
						QW_Sync(QWE_Recv,pPacket->SrcAddr,pSession->DataLen,pSession->pData);
				}
				else	//接收错误
				{
					Debug("Recv error!\n\r");
					QW_ShowPacket(pPacket);
					return QWR_FAILED;
				}
			}
			else if(pPacket->Num==pSession->PacketNum)//重收包
			{
				if(QW_SessionCmd(pSession,QWC_SEND_ACK,pPacket,0)!=QWR_SUCCESS) return QWR_FAILED;
			}
			else//断包了
			{
				QW_PACKET_ECK *pPacketTemp=QW_Mallco(sizeof(QW_PACKET_ECK));
				
				//包错误处理
				Debug("Error Packet Count!\n\r");

				//回错误包
				QW_BuildEckPacket(pPacketTemp,pPacket->ChkSum,pPacket->SrcAddr,0,0,0);
				QW_SendPacket((QW_PACKET_HEADER *)pPacketTemp);
				QW_Free(pPacketTemp);
			}					
			break;
		case QWCA_RECV_ACK://接收，发送请求包之后，收取到ack应答
			if(pPacket->Type==QWPT_ACK)//正确
			{
				if(pSession->PacketNum==pSession->PacketTotal)//发包完成
				{
					QW_Sync(QWE_SendOk,pPacket->SrcAddr,pSession->DataLen,pSession->pData);
					QW_DestroySession(pSession);//可以立刻删除session
				}
				else
					QW_SessionCmd(pSession,QWC_SEND_NEXT_DATA,NULL,0);//发送下一个包
			}
			else if(pPacket->Type==QWPT_ECK)//错误重发
			{
				QW_SessionCmd(pSession,QWC_SEND_AGAIN,NULL,0);
			}
			else//包错误
			{
				Debug("Recv a error packet!\n\r");
				QW_ShowPacket(pPacket);
				return QWR_FAILED;
			}
			break;
		case QWCA_TIMEOUT:
			if(pSession->Type==QWST_SEND)
			{
				QW_Sync(QWE_SendFailed,pPacket->SrcAddr,pSession->DataLen,pSession->pData);//发送未完成同步信息
			}
			else	if(pSession->Type==QWST_RECV)
			{
				if(pSession->PacketNum!=pSession->PacketTotal) 
					QW_Debug("$$$ Recv Un Finish!\n\r");
					//QW_Sync(QWE_Recv,1,NULL);//接收未完成同步信息
			}
			break;
	};
	
	return QWR_SUCCESS;
}


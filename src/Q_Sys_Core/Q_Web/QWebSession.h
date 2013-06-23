#ifndef QWEB_SESSION_H
#define QWEB_SESSION_H

#include "QWebPacket.h"
#include "QWebApi.h"

typedef enum{
	QWCA_NOTHING=0,
	QWCA_NEW_RECV,//无session，接收到新包
	QWCA_BF_SEND,//有session，发送前
	QWCA_AF_SEND,//有session，发送后
	QWCA_RECV_REQ,//有session，接收到主包
	QWCA_RECV_ACK,//有session，接收到应答包
	QWCA_TIMEOUT,//有session，最后一次发包后，间隔GapMs触发。触发完本事件，根据DestroyFlag销毁session
}QW_CALLBACK_ACT;

typedef QW_RESULT (*QW_SessionCallBack)(QW_CALLBACK_ACT ,int ,void *,void *);
	
typedef enum{
	QWST_UNUSE=0,
	QWST_SEND,
	QWST_RECV,
}QW_SESSION_TYPE;

typedef enum{//用于id的session必须是唯一的
	QWSI_NULL=0,
	QWSI_HOST_INFO,//host的定期info session，用于循环发送
	QWSI_HOST_QUERY,//host的定期query session，用于定期查询主机是否活动
	QWSI_SLAVE_QUERY,//slave的首次query，用于查询当前有哪些主机
	QWSI_SLAVE_REQ_ADDR,//slave的地址请求session
	QWSI_DATA_RECV,//多数据包接收session
	QWSI_DATA_SEND,//数据包发送session
	QWSI_API_QUERY,//应用层要求的query
}QW_SESSION_ID;

//4	标志位定义
#define QwDestroyBit		(1<<0)	//设置此位后，表示系统将销毁此session
#define QwDataMallcoBit (1<<1)	//设置此位，表示pData成员是mallco的
#define QwNoSendFlag	(1<<2)	//设置此位，超时后不执行发送动作，数据依然+1

//4	标志位定义结束

typedef struct{
	QW_SESSION_TYPE Type;
	QW_SESSION_ID ID;//特定程序识别号
	u8 SrcAddr;//源地址
	u8 DstAddr;//目的地址
	u8 Flags;//标志位
	u16 LastChkSum;//最后一个包的PacketChkSum域
	u16 PacketTotal;//要处理的包总数
	u16 PacketNum;//已处理的包总数，从0开始
	u8 ReCnt;//发送次数，从0开始，只对主包有效
	u8 TotalReCnt;//总的发送次数，为0时，表示无限重发，只对主包有效
	u32 GapMs; //发送间隔，单位1ms
	u32 LastActTimeMs;//最后一次发送的时间
	QW_SessionCallBack CallBackFunc;//回调函数
	QW_PACKET_DATA *pPacket;//记录主包的内存
	u32 DataLen;//用户数据的长度
	u8 *pData;//用户数据
}QW_SESSION;

u32 QW_CalculateWaitTime(void);
void QW_ShowSession(QW_SESSION *pSession,bool ShowData);
void QW_SessionsInit(void);
QW_RESULT QW_CreatSession(QW_SESSION **ppSession,QW_SESSION_ID MyID,QW_PACKET_HEADER *pFristPacket,QWEB_DATA_STRUCT *pDataStruct,QW_SessionCallBack CallBackFunc);
QW_RESULT QW_SessionSetReSend(QW_SESSION *pSession,u8 TotalReCnt,u32 ReTimeGap);
QW_RESULT QW_SessionSetRecvTimeOut(QW_SESSION *pSession,u32 TimeOutMs);
QW_RESULT QW_SessionFind(QW_PACKET_HEADER *pPacket,QW_SESSION **ppSession);
QW_RESULT QW_SessionCmd(QW_SESSION *pSession,QW_CMD Cmd,QW_PACKET_HEADER *pPacket,u32 TimeParam);
void QW_SessionSetFlag(QW_SESSION *pSession,u8 FlagsBit);
void QW_SessionClrFlag(QW_SESSION *pSession,u8 FlagsBit);
u8 QW_SessionReadFlag(QW_SESSION *pSession,u8 FlagsBit);
QW_RESULT QW_DestroySession(QW_SESSION *pSession);
QW_SESSION *QW_SessionGet(bool SetFrist);
QW_SESSION *QW_SessionGetById(QW_SESSION_ID ID);
QW_RESULT QW_SessionTimeout(void);



#endif


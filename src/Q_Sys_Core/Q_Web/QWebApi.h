#ifndef QWEB_API_H
#define QWEB_API_H

//4 Q网地址定义
enum{
	QW_ADDR_BCAST=0x00,//广播地址，协议定义，不可修改
	QW_ADDR_HOST=0x01,//主机地址，协议定义，不可修改
	QW_ADDR_MIN=0x02,//最小可用地址
	QW_MY_HOPE_ADDR=0x03,//期望地址，用户只能修改此值，范围0x02-0xfe
	QW_ADDR_MAX=63,//最大可用地址，减小该值可以减小系统负担
	QW_ADDR_NUM=QW_ADDR_MAX+1,
	QW_ADDR_DEF=0xff,//设备默认地址，协议定义，不可修改
};
#define QW_HOST_MAP_BYTES  (((QW_ADDR_NUM-1)>>3)+1)
//4 地址定义结束

typedef enum{
	QWR_SUCCESS=0,//成功
	QWR_FAILED,//失败
	QWR_NOHANDLE,//没有处理

	//内部错误
	QWR_SESSION_FULL,
	QWR_BUF_UN_ENOUGH,
	QWR_NOT_FIND_SESSION,
	
	//外部错误
	QWR_ADDR_FULL,//地址满
	QWR_HOST_CONFLICT,//主机冲突
	QWR_REQ_ERROR,//请求包错误
	QWR_NO_SUCH_ADDR,//设备不存在
}QW_RESULT;

typedef enum{
	QWAC_Null=0,
	QWAC_Stop,//停止q网线程，所有数据清空
	QWAC_SendData,
	QWAC_QueryName,	
}QW_API_CMD;

typedef struct{
	u8 ID;//返回的id号，用于用户匹配
	QW_API_CMD CMD;//命令
	u8 DstAddr;//目的地址
	u32 DataLen;//数据长度
	u8 *pData;//数据指针
}QWEB_DATA_STRUCT;

typedef enum{
	QWE_NewJoin,//如果自己是主机，表示有新从机加入，如果自己是从机，表示获取到自己的地址
	QWE_Recv,//接收到数据
	QWE_SendOk,//发送数据成功
	QWE_SendFailed,//发送数据失败
	QWE_HostConflict,//主机冲突
	QWE_QueryAck,//获取到一个query应答
	QWE_Error,
}QW_EVT;

//q网api，供应用层调用
void QWA_StopQWeb(void);
void QWA_StartQWeb(void);
bool QWA_QWebState(void);
u8 QWA_GetMyAddr(void);
u8 QWA_QueryOnline(u8 Addr);
u8 QWA_QueryNextOnline(bool SetFrist);
QW_RESULT QWA_QueryName(u8 Addr);
u8 *QWA_MyQWebName(u8 *NewName);
QW_RESULT QWA_SendData(u8 Addr,u32 DataLen,u8 *pData);
void QWA_CopyDataFinish(void *Ptr);
void QWA_Show(void);

//同步函数，供内部调用
void QW_Sync(QW_EVT Evt,u8 Addr,u32 DataLen,u8 *pData);

#endif


#ifndef Q_WEB_PACKET_H
#define Q_WEB_PACKET_H

typedef enum{
	QWPT_REQ_ADDR=0,//地址请求包，用于新设备加入时请求唯一设备地址
	QWPT_ACK_ADDR,//地址请求应答包，用于主机分配地址给设备

	QWPT_QUERY,//查询包，用于查询指定地址的设备是否存在
	QWPT_ACK_QUERY,//查询应答包，用于应答查询包

	QWPT_INFO,//信息宣告包，用于主动发送设备信息
	
	QWPT_QUIT,//设备退出包，用于设备退出
	QWPT_ACK_QUIT,//设备退出应答包，用于主机响应设备退出

	QWPT_DATA,//数据包，用于设备间的自定义数据收发
	QWPT_ACK,//正确应答包，用于未定义应答包的主包的应答工作。
	QWPT_ECK,//错误应答包，用于未定义应答包的主包的错误应答工作。
}QW_PACKET_TYPE;

typedef struct{ 
	u16 ChkSum;//包校验和，后续所有有效字节的哈希33和
	u8 Len;//当前包长度(不含ChkSum长度)，不能超过允许的最大包长度
	u8 QW_ID;//Q网ID，由主机决定，目的是防止同一区域存在多个Q网（主机）的冲突
	u8 DstAddr;//目的设备地址
	u8 SrcAddr;//源设备地址
	QW_PACKET_TYPE Type;//包类型，QW_PACKET_TYPE
	u8 Count;//主包计数，设备每发一个主包，此计数应+1，应答包的计数应同主包
	u16 Total;//分包情况下，此数目表示分包总个数。单包情况下，此数为1
	u16 Num;//分包情况下，此数目表示当前分包序号。单包情况下，此数为1
	u16 LastChkSum;//前一个主包的校验和
}QW_PACKET_HEADER;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 HopeAddr;//希望得到的地址
	u8 Password;//从机自己设置的密码
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//设备属性字符串
}QW_PACKET_REQ_ADDR;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 GiveAddr;//分配的地址
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//设备属性字符串
	u8 HostTableMap[QW_HOST_MAP_BYTES];//注册从机映射表
}QW_PACKET_ACK_ADDR;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 QueryAddr;//请求地址
}QW_PACKET_QUERY;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//设备属性字符串
}QW_PACKET_ACK_QUERY;//从机用于响应query包

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u32 HostStartTime;//本机启动时间
	u32 HwID;//应该为唯一的硬件id号
	u8 DeviceType;//设备类型
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//设备属性字符串
	u8 HostTableMap[QW_HOST_MAP_BYTES];//注册从机映射表
}QW_PACKET_INFO;//注意只有主机才有权利发info包，用于响应query包

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 Password;//从机申请地址时设置的密码
}QW_PACKET_QUIT;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
}QW_PACKET_ACK_QUIT;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 Data[QW_MAX_DATA_LEN];//包内容头指针
}QW_PACKET_DATA;
typedef struct{
	QW_PACKET_HEADER Hdr;//包头
}QW_PACKET_ACK;

typedef struct{
	QW_PACKET_HEADER Hdr;//包头
	u8 ErrorID;
	u8 Action;
	u16 MsParam;
}QW_PACKET_ECK;

void QW_ShowPacket(QW_PACKET_HEADER *pPacket);
u16 QW_GetChkSum(const u8 *Buf,u16 Len);
u16 QW_CalculatePacketTotal(u32 DataLen);
QW_RESULT QW_BuildReqAddrPacket(QW_PACKET_REQ_ADDR *pPacket,u8 HopeAddr,u8 Password);
QW_RESULT QW_BuildAckAddrPacket(QW_PACKET_ACK_ADDR *pPacket,u16 LastChkSum,u8 DstAddr,u8 GiveAddr);
QW_RESULT QW_BuildQueryPacket(QW_PACKET_QUERY *pPacket,u8 QueryAddr);
QW_RESULT QW_BuildAckQueryPacket(QW_PACKET_ACK_QUERY *pPacket,u16 LastChkSum,u8 DstAddr);
QW_RESULT QW_BuildInfoPacket(QW_PACKET_INFO *pPacket,u8 DstAddr);
QW_RESULT QW_BuildQuitPacket(QW_PACKET_QUIT *pPacket,u8 Password);
QW_RESULT QW_BuildAckQuitPacket(QW_PACKET_ACK_QUIT *pPacket,u16 LastChkSum,u8 DstAddr);
QW_RESULT QW_BuildFristDataPacket(QW_PACKET_DATA *pPacket,u8 DstAddr,u8 *pData,u8 DataLen,u16 Total);
QW_RESULT QW_BuildNextDataPacket(QW_PACKET_DATA *pOldPacket,u8 *pData,u8 DataLen);
QW_RESULT QW_BuildAckPacket(QW_PACKET_ACK *pPacket,u16 LastChkSum,u8 DstAddr);
QW_RESULT QW_BuildEckPacket(QW_PACKET_ECK *pPacket,u16 LastChkSum,u8 DstAddr,u8 ErrorID,u8 ActionID,u16 TimeParam);

#endif


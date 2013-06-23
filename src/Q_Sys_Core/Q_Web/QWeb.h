//---------------------------------------------------------------------
// 本代码根据Q网协议编写，数据传输细节请参考Q网协议规范
// Q网协议--由酷享科技制定的半双工多设备简单网络协议
//---------------------------------------------------------------------
#ifndef QSYS_QWEB_H
#define QSYS_QWEB_H

#include "stm32f10x.h"
#include "PublicFunc.h"
#include "Debug.h"
#include "QWebApi.h"
#include <string.h>

//#define QW_IN_DEBUG 1
#define QW_Debug Debug//重要显示
#define QWU_Debug(x,y...) //次要显示

#ifndef NULL
#define NULL ((void *)0)
#endif

#define QW_MAX_PACKET_LEN 64 //包传输最大字节数，硬件相关
#define QW_MAX_DATA_LEN 50//包中的数据段最大字节数
#define QW_MAX_SESSION_NUM 10//允许最多会话数
#define QW_MAX_MALLCO_BYTES	(50*50)//允许一个session最多申请的堆大小
#define QW_MAX_SUBPACKET_NUM  (QW_MAX_MALLCO_BYTES/QW_MAX_DATA_LEN)//允许最多的分包长度，与内存空间有关，内存大可以分多点
#define QW_PKT_MIN_LEN (sizeof(QW_PACKET_HEADER)-2) //包的最小长度
#define QW_ATTRIBUTE_STR_MAX_LEN	16 //属性字符串最长长度


//4Q网重发次数定义
#define QW_N_UMLIMITED 0//0表示无限次
#define QW_N1	3//设备重发Query的次数
#define QW_N2	4//数据包重发次数
#define QW_N3	3//设备重发地址请求次数
//4次数定义结束

 
//4 Q网时间定义，单位均为ms
#define QW_T1	1000//设备启动时重发Query的时间
#define QW_T2	50//数据包Data重发间隔
#define QW_T3	3000//轮询设备静默情况的时间
#define QW_T4	3000//设备响应查询包的最大随机时间
#define QW_T5	10000//宣告包Info重发间隔
#define QW_T6   10000 //设备最长静默时间，超过此时间，主机会发包查询，超过此时间的2倍，直接删除
#define QW_T7 	1000//设备重发地址请求时间
//4 时间定义结束

typedef enum{
	QWDT_HOST=0,//固定主机
	QWDT_AUTO_HOST,//启动时为从机，如果没找到主机，自动升级为主机
	QWDT_SLAVE,//固定从机
}QW_DEVICE_TYPE;

typedef enum{
	QWC_NOTHING=0,

	//Dir out
	QWC_SEND_FRIST_PKT=0x01,//用来发送包，并记录到session
	QWC_SEND_AGAIN,//重发一次session记录的包
	QWC_SEND_NEXT_DATA,//继续发数据包，数据来源为session pData对应位移处
	QWC_SEND_ACK,//发送ack包

	//Dir in
	QWC_RECV_DATA=0x11,//接收数据

	//Settings
	QWC_SET_SEND_TIME=0x21,//定时发送
	
}QW_CMD;//最高位表示方向

//全局变量声明
extern const QW_DEVICE_TYPE gMyQwDrviceType;//固定的设备类型
extern u8 gMyQwAttributeStr[];//q网属性字符串
extern QW_DEVICE_TYPE gMyQwNowRole;//当前角色
extern u8   gMyQwID;//q网id，由程序启动时生成随机码
extern u8   gMyQwAddr;//本设备当前q网地址
extern u8   gMyQwAddrReqPw;
extern u32 gMyQwStartTime;//q网启动的绝对时间
extern u32 gMyQwHostHwId;//从机用来记录q网主机硬件id

extern const u8 *gPacketTypeName[];
extern const u8 *gSessionTypeName[];
extern const u8 *gQwDeviceName[];
extern const u8 *gSessionStateName[];

u32 QW_GetNowTimeMs(void);
void *QW_Mallco(u16 Size);
bool QW_Free(void *Ptr);
QW_RESULT QW_ChangeRole(QW_DEVICE_TYPE Role,u8 QWebID);

#endif


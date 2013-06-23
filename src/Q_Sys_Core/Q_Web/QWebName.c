#include "QWeb.h"

const u8 *gPacketTypeName[]={
	"QWPT_REQ_ADDR",
	"QWPT_ACK_ADDR",

	"QWPT_QUERY",
	"QWPT_ACK_QUERY",

	"QWPT_INFO",
	
	"QWPT_QUIT",
	"QWPT_ACK_QUIT",

	"QWPT_DATA",
	"QWPT_ACK",
	"QWPT_ECK",
};

const u8 *gSessionTypeName[]={
"UNUSE",
"SEND",
"RECV",
};

const u8 *gQwDeviceName[]={
	"Host",
	"AutoHost",
	"Salve",
};

const u8 *gSessionStateName[]={
	"QWCA_NOTHING",
	"QWCA_NEW_RECV",//无session，接收到新包
	"QWCA_BF_SEND",//有session，发送前
	"QWCA_AF_SEND",//有session，发送后
	"QWCA_RECV_REQ",//有session，接收到主包
	"QWCA_RECV_ACK",//有session，接收到应答包
	"QWCA_TIMEOUT",//有session，最后一次发包后，间隔GapMs触发。触发完本事件，根据DestroyFlag销毁session
};


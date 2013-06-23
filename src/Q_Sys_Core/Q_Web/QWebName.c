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
	"QWCA_NEW_RECV",//��session�����յ��°�
	"QWCA_BF_SEND",//��session������ǰ
	"QWCA_AF_SEND",//��session�����ͺ�
	"QWCA_RECV_REQ",//��session�����յ�����
	"QWCA_RECV_ACK",//��session�����յ�Ӧ���
	"QWCA_TIMEOUT",//��session�����һ�η����󣬼��GapMs�����������걾�¼�������DestroyFlag����session
};


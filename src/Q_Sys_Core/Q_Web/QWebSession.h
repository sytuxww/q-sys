#ifndef QWEB_SESSION_H
#define QWEB_SESSION_H

#include "QWebPacket.h"
#include "QWebApi.h"

typedef enum{
	QWCA_NOTHING=0,
	QWCA_NEW_RECV,//��session�����յ��°�
	QWCA_BF_SEND,//��session������ǰ
	QWCA_AF_SEND,//��session�����ͺ�
	QWCA_RECV_REQ,//��session�����յ�����
	QWCA_RECV_ACK,//��session�����յ�Ӧ���
	QWCA_TIMEOUT,//��session�����һ�η����󣬼��GapMs�����������걾�¼�������DestroyFlag����session
}QW_CALLBACK_ACT;

typedef QW_RESULT (*QW_SessionCallBack)(QW_CALLBACK_ACT ,int ,void *,void *);
	
typedef enum{
	QWST_UNUSE=0,
	QWST_SEND,
	QWST_RECV,
}QW_SESSION_TYPE;

typedef enum{//����id��session������Ψһ��
	QWSI_NULL=0,
	QWSI_HOST_INFO,//host�Ķ���info session������ѭ������
	QWSI_HOST_QUERY,//host�Ķ���query session�����ڶ��ڲ�ѯ�����Ƿ�
	QWSI_SLAVE_QUERY,//slave���״�query�����ڲ�ѯ��ǰ����Щ����
	QWSI_SLAVE_REQ_ADDR,//slave�ĵ�ַ����session
	QWSI_DATA_RECV,//�����ݰ�����session
	QWSI_DATA_SEND,//���ݰ�����session
	QWSI_API_QUERY,//Ӧ�ò�Ҫ���query
}QW_SESSION_ID;

//4	��־λ����
#define QwDestroyBit		(1<<0)	//���ô�λ�󣬱�ʾϵͳ�����ٴ�session
#define QwDataMallcoBit (1<<1)	//���ô�λ����ʾpData��Ա��mallco��
#define QwNoSendFlag	(1<<2)	//���ô�λ����ʱ��ִ�з��Ͷ�����������Ȼ+1

//4	��־λ�������

typedef struct{
	QW_SESSION_TYPE Type;
	QW_SESSION_ID ID;//�ض�����ʶ���
	u8 SrcAddr;//Դ��ַ
	u8 DstAddr;//Ŀ�ĵ�ַ
	u8 Flags;//��־λ
	u16 LastChkSum;//���һ������PacketChkSum��
	u16 PacketTotal;//Ҫ����İ�����
	u16 PacketNum;//�Ѵ���İ���������0��ʼ
	u8 ReCnt;//���ʹ�������0��ʼ��ֻ��������Ч
	u8 TotalReCnt;//�ܵķ��ʹ�����Ϊ0ʱ����ʾ�����ط���ֻ��������Ч
	u32 GapMs; //���ͼ������λ1ms
	u32 LastActTimeMs;//���һ�η��͵�ʱ��
	QW_SessionCallBack CallBackFunc;//�ص�����
	QW_PACKET_DATA *pPacket;//��¼�������ڴ�
	u32 DataLen;//�û����ݵĳ���
	u8 *pData;//�û�����
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


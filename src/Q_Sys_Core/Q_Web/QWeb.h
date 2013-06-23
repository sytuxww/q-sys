//---------------------------------------------------------------------
// ���������Q��Э���д�����ݴ���ϸ����ο�Q��Э��淶
// Q��Э��--�ɿ���Ƽ��ƶ��İ�˫�����豸������Э��
//---------------------------------------------------------------------
#ifndef QSYS_QWEB_H
#define QSYS_QWEB_H

#include "stm32f10x.h"
#include "PublicFunc.h"
#include "Debug.h"
#include "QWebApi.h"
#include <string.h>

//#define QW_IN_DEBUG 1
#define QW_Debug Debug//��Ҫ��ʾ
#define QWU_Debug(x,y...) //��Ҫ��ʾ

#ifndef NULL
#define NULL ((void *)0)
#endif

#define QW_MAX_PACKET_LEN 64 //����������ֽ�����Ӳ�����
#define QW_MAX_DATA_LEN 50//���е����ݶ�����ֽ���
#define QW_MAX_SESSION_NUM 10//�������Ự��
#define QW_MAX_MALLCO_BYTES	(50*50)//����һ��session�������ĶѴ�С
#define QW_MAX_SUBPACKET_NUM  (QW_MAX_MALLCO_BYTES/QW_MAX_DATA_LEN)//�������ķְ����ȣ����ڴ�ռ��йأ��ڴ����Էֶ��
#define QW_PKT_MIN_LEN (sizeof(QW_PACKET_HEADER)-2) //������С����
#define QW_ATTRIBUTE_STR_MAX_LEN	16 //�����ַ��������


//4Q���ط���������
#define QW_N_UMLIMITED 0//0��ʾ���޴�
#define QW_N1	3//�豸�ط�Query�Ĵ���
#define QW_N2	4//���ݰ��ط�����
#define QW_N3	3//�豸�ط���ַ�������
//4�����������

 
//4 Q��ʱ�䶨�壬��λ��Ϊms
#define QW_T1	1000//�豸����ʱ�ط�Query��ʱ��
#define QW_T2	50//���ݰ�Data�ط����
#define QW_T3	3000//��ѯ�豸��Ĭ�����ʱ��
#define QW_T4	3000//�豸��Ӧ��ѯ����������ʱ��
#define QW_T5	10000//�����Info�ط����
#define QW_T6   10000 //�豸���Ĭʱ�䣬������ʱ�䣬�����ᷢ����ѯ��������ʱ���2����ֱ��ɾ��
#define QW_T7 	1000//�豸�ط���ַ����ʱ��
//4 ʱ�䶨�����

typedef enum{
	QWDT_HOST=0,//�̶�����
	QWDT_AUTO_HOST,//����ʱΪ�ӻ������û�ҵ��������Զ�����Ϊ����
	QWDT_SLAVE,//�̶��ӻ�
}QW_DEVICE_TYPE;

typedef enum{
	QWC_NOTHING=0,

	//Dir out
	QWC_SEND_FRIST_PKT=0x01,//�������Ͱ�������¼��session
	QWC_SEND_AGAIN,//�ط�һ��session��¼�İ�
	QWC_SEND_NEXT_DATA,//���������ݰ���������ԴΪsession pData��Ӧλ�ƴ�
	QWC_SEND_ACK,//����ack��

	//Dir in
	QWC_RECV_DATA=0x11,//��������

	//Settings
	QWC_SET_SEND_TIME=0x21,//��ʱ����
	
}QW_CMD;//���λ��ʾ����

//ȫ�ֱ�������
extern const QW_DEVICE_TYPE gMyQwDrviceType;//�̶����豸����
extern u8 gMyQwAttributeStr[];//q�������ַ���
extern QW_DEVICE_TYPE gMyQwNowRole;//��ǰ��ɫ
extern u8   gMyQwID;//q��id���ɳ�������ʱ���������
extern u8   gMyQwAddr;//���豸��ǰq����ַ
extern u8   gMyQwAddrReqPw;
extern u32 gMyQwStartTime;//q�������ľ���ʱ��
extern u32 gMyQwHostHwId;//�ӻ�������¼q������Ӳ��id

extern const u8 *gPacketTypeName[];
extern const u8 *gSessionTypeName[];
extern const u8 *gQwDeviceName[];
extern const u8 *gSessionStateName[];

u32 QW_GetNowTimeMs(void);
void *QW_Mallco(u16 Size);
bool QW_Free(void *Ptr);
QW_RESULT QW_ChangeRole(QW_DEVICE_TYPE Role,u8 QWebID);

#endif


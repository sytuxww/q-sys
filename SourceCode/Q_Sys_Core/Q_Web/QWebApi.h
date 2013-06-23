#ifndef QWEB_API_H
#define QWEB_API_H

//4 Q����ַ����
enum{
	QW_ADDR_BCAST=0x00,//�㲥��ַ��Э�鶨�壬�����޸�
	QW_ADDR_HOST=0x01,//������ַ��Э�鶨�壬�����޸�
	QW_ADDR_MIN=0x02,//��С���õ�ַ
	QW_MY_HOPE_ADDR=0x03,//������ַ���û�ֻ���޸Ĵ�ֵ����Χ0x02-0xfe
	QW_ADDR_MAX=63,//�����õ�ַ����С��ֵ���Լ�Сϵͳ����
	QW_ADDR_NUM=QW_ADDR_MAX+1,
	QW_ADDR_DEF=0xff,//�豸Ĭ�ϵ�ַ��Э�鶨�壬�����޸�
};
#define QW_HOST_MAP_BYTES  (((QW_ADDR_NUM-1)>>3)+1)
//4 ��ַ�������

typedef enum{
	QWR_SUCCESS=0,//�ɹ�
	QWR_FAILED,//ʧ��
	QWR_NOHANDLE,//û�д���

	//�ڲ�����
	QWR_SESSION_FULL,
	QWR_BUF_UN_ENOUGH,
	QWR_NOT_FIND_SESSION,
	
	//�ⲿ����
	QWR_ADDR_FULL,//��ַ��
	QWR_HOST_CONFLICT,//������ͻ
	QWR_REQ_ERROR,//���������
	QWR_NO_SUCH_ADDR,//�豸������
}QW_RESULT;

typedef enum{
	QWAC_Null=0,
	QWAC_Stop,//ֹͣq���̣߳������������
	QWAC_SendData,
	QWAC_QueryName,	
}QW_API_CMD;

typedef struct{
	u8 ID;//���ص�id�ţ������û�ƥ��
	QW_API_CMD CMD;//����
	u8 DstAddr;//Ŀ�ĵ�ַ
	u32 DataLen;//���ݳ���
	u8 *pData;//����ָ��
}QWEB_DATA_STRUCT;

typedef enum{
	QWE_NewJoin,//����Լ�����������ʾ���´ӻ����룬����Լ��Ǵӻ�����ʾ��ȡ���Լ��ĵ�ַ
	QWE_Recv,//���յ�����
	QWE_SendOk,//�������ݳɹ�
	QWE_SendFailed,//��������ʧ��
	QWE_HostConflict,//������ͻ
	QWE_QueryAck,//��ȡ��һ��queryӦ��
	QWE_Error,
}QW_EVT;

//q��api����Ӧ�ò����
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

//ͬ�����������ڲ�����
void QW_Sync(QW_EVT Evt,u8 Addr,u32 DataLen,u8 *pData);

#endif


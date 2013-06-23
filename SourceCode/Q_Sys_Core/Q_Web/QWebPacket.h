#ifndef Q_WEB_PACKET_H
#define Q_WEB_PACKET_H

typedef enum{
	QWPT_REQ_ADDR=0,//��ַ��������������豸����ʱ����Ψһ�豸��ַ
	QWPT_ACK_ADDR,//��ַ����Ӧ������������������ַ���豸

	QWPT_QUERY,//��ѯ�������ڲ�ѯָ����ַ���豸�Ƿ����
	QWPT_ACK_QUERY,//��ѯӦ���������Ӧ���ѯ��

	QWPT_INFO,//��Ϣ��������������������豸��Ϣ
	
	QWPT_QUIT,//�豸�˳����������豸�˳�
	QWPT_ACK_QUIT,//�豸�˳�Ӧ���������������Ӧ�豸�˳�

	QWPT_DATA,//���ݰ��������豸����Զ��������շ�
	QWPT_ACK,//��ȷӦ���������δ����Ӧ�����������Ӧ������
	QWPT_ECK,//����Ӧ���������δ����Ӧ����������Ĵ���Ӧ������
}QW_PACKET_TYPE;

typedef struct{ 
	u16 ChkSum;//��У��ͣ�����������Ч�ֽڵĹ�ϣ33��
	u8 Len;//��ǰ������(����ChkSum����)�����ܳ����������������
	u8 QW_ID;//Q��ID��������������Ŀ���Ƿ�ֹͬһ������ڶ��Q�����������ĳ�ͻ
	u8 DstAddr;//Ŀ���豸��ַ
	u8 SrcAddr;//Դ�豸��ַ
	QW_PACKET_TYPE Type;//�����ͣ�QW_PACKET_TYPE
	u8 Count;//�����������豸ÿ��һ���������˼���Ӧ+1��Ӧ����ļ���Ӧͬ����
	u16 Total;//�ְ�����£�����Ŀ��ʾ�ְ��ܸ�������������£�����Ϊ1
	u16 Num;//�ְ�����£�����Ŀ��ʾ��ǰ�ְ���š���������£�����Ϊ1
	u16 LastChkSum;//ǰһ��������У���
}QW_PACKET_HEADER;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u8 HopeAddr;//ϣ���õ��ĵ�ַ
	u8 Password;//�ӻ��Լ����õ�����
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//�豸�����ַ���
}QW_PACKET_REQ_ADDR;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u8 GiveAddr;//����ĵ�ַ
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//�豸�����ַ���
	u8 HostTableMap[QW_HOST_MAP_BYTES];//ע��ӻ�ӳ���
}QW_PACKET_ACK_ADDR;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u8 QueryAddr;//�����ַ
}QW_PACKET_QUERY;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//�豸�����ַ���
}QW_PACKET_ACK_QUERY;//�ӻ�������Ӧquery��

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u32 HostStartTime;//��������ʱ��
	u32 HwID;//Ӧ��ΪΨһ��Ӳ��id��
	u8 DeviceType;//�豸����
	u8 AttributeStr[QW_ATTRIBUTE_STR_MAX_LEN];//�豸�����ַ���
	u8 HostTableMap[QW_HOST_MAP_BYTES];//ע��ӻ�ӳ���
}QW_PACKET_INFO;//ע��ֻ����������Ȩ����info����������Ӧquery��

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u8 Password;//�ӻ������ַʱ���õ�����
}QW_PACKET_QUIT;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
}QW_PACKET_ACK_QUIT;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
	u8 Data[QW_MAX_DATA_LEN];//������ͷָ��
}QW_PACKET_DATA;
typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
}QW_PACKET_ACK;

typedef struct{
	QW_PACKET_HEADER Hdr;//��ͷ
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


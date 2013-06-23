#ifndef QSYS_DATABASE_H
#define QSYS_DATABASE_H

typedef enum {
//-------------------ϵͳ����-----------------------
	//������ϵͳ���ݣ�ϵͳ���ݻᱻ�浽flash��
	Setting_BgLightTime=0,
	Setting_BgLightScale,
	
	Setting_DBMaxNum,		//���һ��ֵ��Ӱ�����ݿ�汾

//-------------------ϵͳ״̬-----------------------
	//������ϵͳ״̬��ϵͳ״̬���ᱻ�浽flash��
	//����Ĭ��ֵ����0
	Status_ValueStart,
	Status_FsInitFinish,//�ļ�ϵͳ�Ƿ���ɹ���

	Setting_Status_MaxNum,//���ú�״̬�������Ŀ
}SETTING_NAME;

//���ݿ������
u32 Q_DB_GetValue(SETTING_NAME Name,void *Val);

//���ݿ�д����,������DB_BurnToSpiFlash�Ļ�,�����ʧȥ
//�ɹ�����TRUE
bool Q_DB_SetValue(SETTING_NAME Name,u32 IntParam,void *pParam,u8 ByteLen);

//��ϵͳ��ȡ״̬,ϵͳ״̬Ϊ��ʱ����
u32 Q_DB_GetStatus(SETTING_NAME Name,void *Val);

//����ϵͳ״̬
bool Q_DB_SetStatus(SETTING_NAME Name,u32 IntParam,void *pParam,u8 ByteLen);

//�����ݿ�д��spi flash��
//�ɹ�����TRUE
bool Q_DB_BurnToSpiFlash(void);

#endif


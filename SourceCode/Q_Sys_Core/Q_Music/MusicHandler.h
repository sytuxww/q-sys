#ifndef QSYS_MUSIC_HANDLER_H
#define QSYS_MUSIC_HANDLER_H

//4	���ֿ�����
typedef enum{
	//״̬��
	MusicNothing=0,//�ɽ����û�״̬����
	MusicPlay,
	MusicPause,//�ɽ����û�״̬����
	MusicContinue,//�ɽ����û�״̬����
	MusicJump,
	MusicStop,

	//������
	MusicChangeVol,
	MusicGetFFT,
	MusicSetIo,
}MUSIC_CMD;

typedef struct{	
	MUSIC_CMD Cmd;
	u16 ParamU16;
	u8 *pFilePath;	
	void *pParam;
}MUSIC_EVENT;

typedef struct{
	u8 Title[48];
	u8 Performer[32];
	u8 Album[32];
	u8 Year[6];
	u8 Note[32];

	u32 AudioByte;//��Ƶ���ִ�С
	u32 Duration;//��λ ��
	u16 BitRate;//������
	u16 SampleRate;//������
	u16 AudioOffset;//��Ƶ����ƫ��ֵ
	bool HasToc;//�Ƿ���toc table
	u8 Toc[100];//toc table
}MUSIC_INFO;

//4	���ֿ��������

//ͨ���˺����������ֿ����̷߳�����
//�ɹ�����TRUE
bool Q_MusicSync(MUSIC_EVENT *pMusicEvent);

//�ȴ�Music�߳̿���
//PerWaitMsΪ��С�ȴ�����
//void WaitMusicHandlerIdle(u16 PerWaitMs);

//�������ֲ����̵߳ĵ�ǰ״̬
//����ֵPlayOne,PlayDir,PlayList,Nothing,Pause
MUSIC_CMD Q_MusicGetState(void);

//���ص�ǰ�ļ�λ��
u32 Q_MusicGetFilePtr(void);

//���ص�ǰ����·��ͷָ��
u8 * Q_MusicGetPath(void);

//���ص�ǰ�����ļ���Ϣ
MUSIC_INFO *Q_MusicGetInfo(void);

//�������ֲ���ʱ�䣬��λ����
u32 Q_MusicGetPlayMs(void);

#endif


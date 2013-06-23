#ifndef QSYS_INPUT_HANDLER_H
#define QSYS_INPUT_HANDLER_H

//------------------------�궨��----------------------------

//-------------------------��������------------------------------
//����gInputHandler_Queue���е����Ͷ��壬��ϵͳʹ�ã�ҳ�治���á�
typedef enum{
	Input_TchNormal=0,//�����壬���ڴ��ݲ���
	Input_TchPress,//��������
	Input_TchContinue,//���ִ���״̬�����ڴ��ݳ���ʱ��ʵʱ����
	Input_TchRelease,//����Ч�������ɿ�,��Ч����ָPressʱ����ע������
	Input_TchReleaseVain,//�ڷ���Ч�����ɿ�
	Input_ExtiKey,		//�ⲿ�жϰ���
	Input_RtcSec,     //����ʵʱʱ�Ӻ�Input�̵߳Ľ���
	Input_RtcMin,		//����ʵʱʱ�Ӻ�Input�̵߳Ľ���
	Input_RtcAlarm, //����ʵʱʱ�Ӻ�Input�̵߳Ľ���
	Input_Timer,	//���ڶ�ʱ������
	Input_PageSync,	//����run�̺߳�Input�̵߳Ľ���
	Input_LcdOff,//LCD��ʱϨ��
	Input_LcdOn,//LCD�ӳ�ʱϨ��֮��ָ�����ʱ�������¼�
	Input_GotoNewPage,//��Ҫ������ҳ��
	Input_GotoSubPage,//��Ҫ������ҳ��
	Input_SubPageReturn,//����ҳ�淵��
	Input_SubPageTranslate,//��һ����ҳ�������һ��ͬ����ҳ��
	Input_UartInput,//����������
	Input_MscPlay,//������Ƶ
	Input_MscPause,//������ͣ
	Input_MscContinue,//���ż���
	Input_MscStop,//����ֹͣ
	Input_QWebEvt,//�յ�Q����Ϣ
	Input_Error,//����
}INPUT_EVT_TYPE;

typedef enum{
	Touch_Type=0,//�����¼�
	Sync_Type,	//ͬ���¼�
	Rtc_Type,//rtc��ص��¼�������lcd light��rtc sec min��rtc alarm
	SingleNum_Type,//����ֻ��Ҫһ������Num���¼�
}UNION_TYPE;

typedef struct{
	UNION_TYPE uType;//����ָ�����������������,ʵ�ʾ��ǵ�һ������
	INPUT_EVT_TYPE EventType;//����ָ���¼�������,ʵ�ʾ��ǵڶ�������
	u16 Num;
	union{
		TOUCH_INFO TouchInfo;//size = 8
		SYNC_INFO SyncInfo;//size = 8
		u8 Items[8];
	}Info;
}INPUT_EVENT;//sizeof=1+1+2+8=12
//����gInputHandler_Queue���е����ͽ���

typedef struct {
	PAGE_RID RegID;//RegID������Ϊ0����ÿ��ҳ�����Ψһ��
	const PAGE_ATTRIBUTE *pPage;
}PAGE_CONGREGATION;//��¼ҳ��

typedef struct{ 
	PERIP_EVT PeripEvt;//�¼����ͣ�ֻ�Բ����¼���Ч����Ϊ0��ʾ�����
	PeripheralsHandlerFunc GobalPeripEvtHandler;//������
}GOBAL_PERIPEVT_RECORD;//��¼ȫ���¼��Ľṹ��

//------------------------------��������----------------------------------
extern const PAGE_ATTRIBUTE *gpCurrentPage;//��ǰҳ��ָ��


extern OS_MsgBoxHandle gInputHandler_Queue;

//--------------------------��������------------------------
extern void Allow_Touch_Input(void);
void InputHandler_Task( void *Task_Parameters );

#endif


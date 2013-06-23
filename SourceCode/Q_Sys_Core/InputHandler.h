#ifndef QSYS_INPUT_HANDLER_H
#define QSYS_INPUT_HANDLER_H

//------------------------宏定义----------------------------

//-------------------------类型声明------------------------------
//传入gInputHandler_Queue队列的类型定义，被系统使用，页面不可用。
typedef enum{
	Input_TchNormal=0,//无意义，用于传递参数
	Input_TchPress,//触摸按下
	Input_TchContinue,//保持触摸状态，用于传递长按时的实时坐标
	Input_TchRelease,//在有效区域触摸松开,有效区域指Press时的已注册区域
	Input_TchReleaseVain,//在非有效区域松开
	Input_ExtiKey,		//外部中断按键
	Input_RtcSec,     //用于实时时钟和Input线程的交互
	Input_RtcMin,		//用于实时时钟和Input线程的交互
	Input_RtcAlarm, //用于实时时钟和Input线程的交互
	Input_Timer,	//用于定时器到期
	Input_PageSync,	//用于run线程和Input线程的交互
	Input_LcdOff,//LCD超时熄灭
	Input_LcdOn,//LCD从超时熄灭之后恢复点亮时触发此事件
	Input_GotoNewPage,//需要进入新页面
	Input_GotoSubPage,//需要进入子页面
	Input_SubPageReturn,//从子页面返回
	Input_SubPageTranslate,//从一个子页面进入另一个同级子页面
	Input_UartInput,//串口有输入
	Input_MscPlay,//播放音频
	Input_MscPause,//播放暂停
	Input_MscContinue,//播放继续
	Input_MscStop,//播放停止
	Input_QWebEvt,//收到Q网信息
	Input_Error,//错误
}INPUT_EVT_TYPE;

typedef enum{
	Touch_Type=0,//触碰事件
	Sync_Type,	//同步事件
	Rtc_Type,//rtc相关的事件，包括lcd light、rtc sec min、rtc alarm
	SingleNum_Type,//所有只需要一个参数Num的事件
}UNION_TYPE;

typedef struct{
	UNION_TYPE uType;//用于指定后面的联合体类型,实际就是第一级分类
	INPUT_EVT_TYPE EventType;//用于指定事件的类型,实际就是第二级分类
	u16 Num;
	union{
		TOUCH_INFO TouchInfo;//size = 8
		SYNC_INFO SyncInfo;//size = 8
		u8 Items[8];
	}Info;
}INPUT_EVENT;//sizeof=1+1+2+8=12
//传入gInputHandler_Queue队列的类型结束

typedef struct {
	PAGE_RID RegID;//RegID不允许为0，且每个页面必须唯一。
	const PAGE_ATTRIBUTE *pPage;
}PAGE_CONGREGATION;//记录页面

typedef struct{ 
	PERIP_EVT PeripEvt;//事件类型，只对部分事件有效，如为0表示此项空
	PeripheralsHandlerFunc GobalPeripEvtHandler;//函数体
}GOBAL_PERIPEVT_RECORD;//记录全局事件的结构体

//------------------------------变量声明----------------------------------
extern const PAGE_ATTRIBUTE *gpCurrentPage;//当前页面指针


extern OS_MsgBoxHandle gInputHandler_Queue;

//--------------------------函数声明------------------------
extern void Allow_Touch_Input(void);
void InputHandler_Task( void *Task_Parameters );

#endif


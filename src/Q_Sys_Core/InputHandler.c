/*酷系统 说明
1.本文将是酷系统中很重要的一个文件，主体为InputHandler线程，负责新页面
跳转函数的实现和系统事件的发出。

2.本文件的所有函数都只供InputHandler_Task使用，其包含了每个页面的
SystemEventHandler函数和TouchEventHandler函数，每个页面的这两个函数都属于
InputHandler_Task的一部分。也只有这两个函数才能操作lcd资源，所以即便
SystemEventHandler函数生成了新的线程，新的线程要改变lcd显示，也必须通过
事件机制告诉InputHandler_Task，让InputHandler_Task去调用SystemEventHandler对lcd
进行处理，这点编程人员一定要清楚。

3.每个页面的SystemEventHandler函数在第一次运行的时候，都有权利新建若干
个线程，但是为了页面的同步，当当前页面退出时，此页面对应的线程
也最好暂停，这就是页面线程主循环里要放PageRunTaskLoopStart和PageRunTaskLoopStop的
原因。当然，你也可以不放这两个宏，这样的话，你的线程不会被暂停，
而是在后台运行。不过你要谨慎编程，以防系统出错。

*/

#include "System.h"
#include "AllPage.h"

#define Input_Debug Debug 

#if 1 //系统内核全局变量
const PAGE_ATTRIBUTE *gpCurrentPage=NULL;	//当前页面指针

static u8 PageTracks[MAX_PAGE_TRACK_NUM];//用于记录页面跳转记录
static u8 gCurTrackIdx;//当前Track记录索引

static u8 PageLayers[MAX_PAGE_LAYER_NUM+1];//用于记录页面层级。[0]不可用，所以要加一个元素
static u8 gCurLayerNum;//用于记录页面层数

u16 gEntriesOfPage[PAGE_TOTAL];//记录每个页面进入的次数
u32 gPagePeripEvtFlag[PAGE_TOTAL];//记录每个页面的外围事件响应标志

GOBAL_PERIPEVT_RECORD gGobalPeripEvtRecord[MAX_GOBAL_SYSEVT];//记录所有页面的全局事件表
u32 gGobalPeripEvtBitFlag;//全局事件标志
QSH_VAR_REG(gGobalPeripEvtBitFlag,"u32 gGobalPeripEvtBitFlag","u32");

SYS_MSG gCurrSysMsg=SM_State_OK;//记录页面case返回的信息
u8 gPageHeapRecord=0;//记录用户页面堆栈分配数目的变量
#endif 

extern TOUCH_REGION *GetTouchInfoByIdx(u8 Idx);

#if 1 //control obj 
extern void ControlObjInit(void);
extern void PageSwithcCtrlObjDataHandler(const PAGE_ATTRIBUTE *pNewPage);
CO_MSG ButtonCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo);
CO_MSG YesNoCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx);
CO_MSG NumCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo);
CO_MSG StrCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo);

extern void PushPageCtrlObjData(void);
extern void PopPageCtrlObjData(bool);
extern void CleanPageCtrlObjData(void);
extern SYS_MSG CurrPageCtrlObjInit(INPUT_EVT_TYPE EventType,int IntParam,void *pInfoParam);
#endif

#include "PageDataHandler.c"

bool GetCurrPageOptionsMask(u32 Option)
{
	return (gpCurrentPage->OptionsMask&Option)?TRUE:FALSE;
}

//处理全局事件响应
SYS_MSG MyGobalPeripEvtHandler(PERIP_EVT PeripEvt,int intParam, void *pParam)
{
	SYS_MSG SysMsg=0;
	u8 i;

	if(!ReadBit(gGobalPeripEvtBitFlag,PeripEvt)) return 0;//没有此类事件
	
	//找到匹配记录
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
 		if(gGobalPeripEvtRecord[i].PeripEvt==PeripEvt)//事件对上号了
 		{
			SysMsg|=gGobalPeripEvtRecord[i].GobalPeripEvtHandler(PeripEvt,intParam,pParam);
 		}
	}	

	return SysMsg;
}

//将一个页面从当前页面切换为非当前页面时调用的函数
//SubPage==TRUE表示即将进入的是子页面
static SYS_MSG OldPageClean(INPUT_EVT_TYPE EventType,u8 NewPageIndex)
{
	SYS_MSG SysMsg=SM_State_Faile;
	u8 LayerNum;
	
	Input_Debug("%s : Clean %s\n\r",__FUNCTION__,gpCurrentPage->Name);

	switch(EventType)
	{
		case Input_GotoSubPage://进入子页面
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_PreSubPage,GetRegIdByIdx(NewPageIndex),
				(void *)GetPageByIdx(NewPageIndex));
			break;
		case Input_SubPageReturn:	//从子页面返回
		case Input_SubPageTranslate://子页面切换
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_PageClean,GetRegIdByIdx(NewPageIndex),
				(void *)GetPageByIdx(NewPageIndex));
			break;
		case Input_GotoNewPage://进入新页面
			LayerNum=GetCurLayerNum();
			for(;LayerNum;LayerNum--)//对上一级页面逐个处理
			{
				SysMsg|=Q_GetPageByLayer(LayerNum)->SysEvtHandler(Sys_PageClean,GetRegIdByIdx(NewPageIndex),
					(void *)GetPageByIdx(NewPageIndex));
			}
			break;
	}

	Input_Debug("%s end: Clean %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return SysMsg;
}

//激活新页面时先进行初始化
static SYS_MSG CurrentPageInit(INPUT_EVT_TYPE EventType,int IntParam,void *pSysParam)
{
	SYS_MSG SysMsg=SM_State_Faile;
	
	Input_Debug("%s : %s\n\r",__FUNCTION__,gpCurrentPage->Name);

	switch(EventType)
	{
		case Input_GotoSubPage://进入子页面
		case Input_SubPageTranslate://进入新的同级子页面
		case Input_GotoNewPage://进入新页面
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_PageInit,IntParam,pSysParam);
			break;
		case Input_SubPageReturn://从子页面返回
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_SubPageReturn,IntParam,pSysParam);
			break;
	}
		
	Input_Debug("%s end: %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return SysMsg;
}

//KeyId - 按键序号
//KeyStaus - 状态值 0(release) or 1(press)
void ExtiKeyHandler(u8 KeyId,u8 KeyStaus)
{
	INPUT_EVENT EventParam;

	EventParam.uType=SingleNum_Type;
	EventParam.EventType=Input_ExtiKey;
	EventParam.Num=((KeyId+EXTI_KEY_VALUE_START)<<1)+KeyStaus;
	OS_MsgBoxSend(gInputHandler_Queue,&EventParam,100,FALSE);	
}

static SYS_MSG GotoPageHandler(INPUT_EVT_TYPE EventType,u16 PageIdx,int IntParam, void *pSysParam)
{
	u32 TimeMsRecord;
	u8 i;
	OS_DeclareCritical();

	//关闭触摸和外部按键输入，保证在换页面或者程序的过程中忽略所有输入
	Q_DisableInput();

	//在无输入状态下Clean页面
	if(gpCurrentPage)
	{
		if((!(gCurrSysMsg&SM_NoPageClean)) //Q_GotoPage返回值告诉系统不要执行Sys_PageClean或Sys_PreSubPage事件
			&& (GetPageByIdx(PageIdx)->Type != POP_PAGE)) //如果是pop页面，也不需要clean
			gCurrSysMsg|=OldPageClean(EventType,PageIdx);
		Debug("∧∧∧∧∧∧ Leave %s ∧∧∧∧∧∧\n\r",gpCurrentPage->Name);
	}
	else	//没有上一个页面
	{

	}	

	//4--------------------------数据处理-----------------------------
	OS_EnterCritical();//目前，只有一个线程操作这些全局变量，所以临界区并不是必须的。

	//记录页面Tarck
	InsertPageTrack(PageIdx);

	//记录页面层级
	if(EventType==Input_GotoSubPage) //如果是进入子页面
	{
		PushPageCtrlObjData();//保存前层的数据内容
		CleanPageCtrlObjData();//清除页面数据
		InsertPageLayer(PageIdx);//增加一层
	}
	else if(EventType==Input_SubPageReturn)
	{
		DeletePageLayer(1);//删除一层
		PopPageCtrlObjData(FALSE);//还原前层的数据内容
	}
	else if(EventType==Input_GotoNewPage)
	{
		DeletePageLayer(0xff);//删除所有层
		PopPageCtrlObjData(TRUE);//删除所有存储的数据内容
		CleanPageCtrlObjData();//清除页面数据
		InsertPageLayer(PageIdx);//增加一层

		//页面堆检查
		if(gPageHeapRecord)
		{
			Debug("gPageHeapRecord:%d\n\r",gPageHeapRecord);
			Q_ErrorStopScreen("!!!Warnning!!! Page momery don't be freed!");
		}
	}
	else if(EventType==Input_SubPageTranslate)
	{
		DeletePageLayer(1);//删除一层
		CleanPageCtrlObjData();//清除页面数据
		InsertPageLayer(PageIdx);//增加一层
	}
	
	//更换主指针
	gpCurrentPage=Q_GetPageByTrack(0);
	gEntriesOfPage[PageIdx]++;//进页面一次自加一次

	PageSwithcCtrlObjDataHandler(gpCurrentPage);//控件数据处理
	
	OS_ExitCritical();
	//4-------------------------数据处理完成--------------------------

	TimeMsRecord=OS_GetCurrentSysMs();
	Debug("\n\r∨∨∨∨∨∨ Goto %s ∨∨∨∨∨∨\n\r",gpCurrentPage->Name);
	Debug("##Author:%s\n\r",gpCurrentPage->Author);
	Debug("##Description:%s\n\r",gpCurrentPage->Description);

	Debug("##Page Layers:");
	for(i=1;i<GetCurLayerNum();i++) Debug("%s->",GetPageByIdx(PageLayers[i])->Name); 
	Debug("%s\n\r",GetPageByIdx(PageLayers[i])->Name); 

	if((!(gCurrSysMsg&SM_NoPageInit)) //Q_GotoPage返回值告诉系统不要执行page init
		&&(Q_GetPageByTrack(1)->Type!=POP_PAGE)) //从pop页面返回，也不需要page init
		gCurrSysMsg|=CurrentPageInit(EventType,IntParam,pSysParam);

	if((!(gCurrSysMsg&SM_NoTouchInit))//Q_GotoPage或页面SystemEventHandler函数返回值告诉系统不需要touch init
		&&(Q_GetPageByTrack(1)->Type!=POP_PAGE))	 //从pop页面返回，也不需要touch init
		gCurrSysMsg|=CurrPageCtrlObjInit(EventType,IntParam,pSysParam);	

	if(!(gCurrSysMsg&SM_NoPopReturn)//从前面的函数返回掩码
		&&(Q_GetPageByTrack(1)->Type==POP_PAGE))//前一个页面是pop页面
		gCurrSysMsg|=gpCurrentPage->SysEvtHandler(Sys_PopPageReturn,IntParam,pSysParam);

	switch(GetRegIdByIdx(GetPageIdxByTrack(1)))//判断从哪个页面返回
	{
		case PRID_NumCtrlObjPage: //从num控件页返回
			{
				NUM_CTRL_OBJ *pNumCtrlObj=pSysParam;
				if(gpCurrentPage->NumCtrlObjHander)
					gpCurrentPage->NumCtrlObjHander(pNumCtrlObj->ObjID,pNumCtrlObj->Value,pSysParam);
				else 
					Debug("!!!Not Define Handler Function In Page Struct!!!\n\r");
			}break;
		case PRID_StrCtrlObjPage: //从Str控件页返回
			{
				STR_CTRL_OBJ *pStrCtrlObj=pSysParam;
				STR_BOX_OBJ *pStrBoxObj=pSysParam;
				STR_ENUM_OBJ *pStrEnumObj=pSysParam;
				if(gpCurrentPage->StrCtrlObjHandler)
				{
					if(pStrCtrlObj->Type == SCOT_StrBox)
						gpCurrentPage->StrCtrlObjHandler(pStrBoxObj->ObjID,0,pStrBoxObj->pStrBuf,pSysParam);
					else if(pStrCtrlObj->Type == SCOT_StrEnum)
						gpCurrentPage->StrCtrlObjHandler(pStrEnumObj->ObjID,pStrEnumObj->pStrEnumBuf[pStrEnumObj->Idx],&pStrEnumObj->pStrEnumBuf[pStrEnumObj->Idx+1],pSysParam);
				}
				else 
					Debug("!!!Not Define Handler Function In Page Struct!!!\n\r");
			}break;
		defalut:
			while((OS_GetCurrentSysMs()-TimeMsRecord)<300) OS_TaskDelayMs(50);//循环延时300ms，以避免触摸响应混乱
	}
	
	Q_EnableInput();
	
	Input_Debug("%s end: %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return gCurrSysMsg;
}

//用于处理控件触摸事件分类
//Num里存储的是touch线程的触摸区域索引
//InType:	Input_TchPress,//触摸按下
//				Input_TchContinue,//保持触摸状态，用于传递长按时的实时坐标
//				Input_TchRelease,//在有效区域触摸松开,有效区域指Press时的已注册区域
//				Input_TchReleaseVain,//在非有效区域松开
//Idx:usb by GetTouchInfoByIdx(Num)
//pTouchInfo:touch info
static CO_MSG CtrlObjTypeHandler(INPUT_EVT_TYPE InType,u16 Idx,TOUCH_INFO *pTouchInfo)
{	
	CO_MSG CoMsg=CO_State_OK;

	switch(GetTouchInfoByIdx(Idx)->Type)
	{
		case COT_ImgBtn:
		case COT_CharBtn:
		case COT_DynImgBtn:
		case COT_DynCharBtn:
			CoMsg=ButtonCtrlObjTchHandler(InType,Idx,pTouchInfo);
			break;
		case COT_YesNo:
			CoMsg=YesNoCtrlObjTchHandler(InType,Idx);
			break;
		case COT_Num:
			CoMsg=NumCtrlObjTchHandler(InType,Idx,pTouchInfo);
			break;
		case COT_Str:
			CoMsg=StrCtrlObjTchHandler(InType,Idx,pTouchInfo);
			break;
	}
	return CoMsg;
}

//系统全局数据初始化
static void QSYS_DataInit(void)
{
	u16 i;
	
	MemSet(PageTracks,0,MAX_PAGE_TRACK_NUM);//清空页面跳转痕迹
	gCurTrackIdx=0;
	
	MemSet(PageLayers,0,MAX_PAGE_LAYER_NUM+1);//清空页面层级记录
	gCurLayerNum=0;

	MemSet(gEntriesOfPage,0,PAGE_TOTAL);//清空页面进入次数记录

	//复制所有页面的事件掩码到记录
	for(i=0;i<PAGE_TOTAL;i++)
	{
		gPagePeripEvtFlag[i]=AllPage[i].pPage->PeripEvtInitMask;

		//顺便检查页面合法性
		if((AllPage[i].pPage->SysEvtHandler==NULL)||(AllPage[i].pPage->ButtonHandler==NULL)
			||(AllPage[i].pPage->PeripEvtHandler==NULL))
			Q_ErrorStopScreen("Page SysEvtHandler & TchEvtHandler no allow be NULL!");
	}

	MemSet(gGobalPeripEvtRecord,0,MAX_GOBAL_SYSEVT*sizeof(GOBAL_PERIPEVT_RECORD));//清空全局事件记录
	gGobalPeripEvtBitFlag=0;

	ControlObjInit();
}

extern void QSYS_Init(void);
//本任务负责对触摸触发的事件进行处理，过程中会调用当前页面的TouchEventHandler函数。
//如下函数为本任务专属
// 1.本页下的所有函数
// 2.当前页面的SystemEventHandler 函数和TouchEventHandler函数及各种控件函数
//所以，其他线程请不要调用这些函数
extern void *KeysHandler_Task_Handle;
void InputHandler_Task( void *Task_Parameters )
{
	INPUT_EVENT InEventParam;
	EXIT_KEY_INFO ExtiKeyInfo[EXTI_KEY_MAX_NUM];//用于存储外部按键信息
	u32 TickStamp[EXTI_KEY_MAX_NUM];
	u16 ExtiKeyNum;
	SYS_MSG SysMsg;
	CO_MSG CoMsg;

	QSYS_Init();//系统初始化
	QSYS_DataInit();//系统数据初始化

	MemSet(ExtiKeyInfo,0,sizeof(EXIT_KEY_INFO)*EXTI_KEY_MAX_NUM);

	Q_GotoPage(GotoNewPage,"MainPage",0,NULL);	//进主页;
	
	while(1)
	{
		OS_TaskStkCheck(FALSE);
		//Debug("Wait Input Event!\n\r");
		OS_MsgBoxReceive(gInputHandler_Queue,&InEventParam,OS_MAX_DELAY);//等待有效的输入
		
		//if(Sync_Type==InEventParam.uType)
			//if(Input_GotoSubPage==InEventParam.EventType) Debug("@@@Gopage\n\r");
		
		SysMsg=CoMsg=0;
		switch(InEventParam.uType)//对第一级分类进行判断
		{
			case Touch_Type:
				CoMsg=CtrlObjTypeHandler(InEventParam.EventType,InEventParam.Num,&InEventParam.Info.TouchInfo);
				break; 
			case Sync_Type:
				if(Input_PageSync==InEventParam.EventType)//处理页面同步
				{
					SysMsg=gpCurrentPage->SysEvtHandler(Sys_PageSync,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
				}
				else if((Input_GotoNewPage==InEventParam.EventType)
						|(Input_GotoSubPage==InEventParam.EventType)
						|(Input_SubPageReturn==InEventParam.EventType))//处理页面跳转
				{
					SysMsg=GotoPageHandler(InEventParam.EventType,InEventParam.Num,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
				}
				else if(Input_UartInput==InEventParam.EventType)//处理串口输入
				{
					MyGobalPeripEvtHandler(Perip_UartInput,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
					if(Q_InspectPeripEvt(PRID_Current,Perip_UartInput)==HasPagePeripEvt)//检查是否允许触发
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_UartInput,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
				}
				else if(Input_QWebEvt==InEventParam.EventType)//处理q网事件
				{
					switch(InEventParam.Num)
					{
						case QWE_NewJoin:
							MyGobalPeripEvtHandler(Perip_QWebJoin,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebJoin)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebJoin,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							QWA_CopyDataFinish(InEventParam.Info.SyncInfo.pParam);//收回q网内存
							break;
						case QWE_Recv:
							MyGobalPeripEvtHandler(Perip_QWebRecv,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebRecv)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebRecv,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							QWA_CopyDataFinish(InEventParam.Info.SyncInfo.pParam);//收回q网内存
							break;
						case QWE_SendOk:
							MyGobalPeripEvtHandler(Perip_QWebSendOk,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebSendOk)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebSendOk,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
						case QWE_SendFailed:
							MyGobalPeripEvtHandler(Perip_QWebSendFailed,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebSendFailed)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebSendFailed,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
						case QWE_HostConflict:
							MyGobalPeripEvtHandler(Perip_QWebHostConflict,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebHostConflict)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebHostConflict,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
						case QWE_QueryAck:
							MyGobalPeripEvtHandler(Perip_QWebQueryName,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebQueryName)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebQueryName,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							QWA_CopyDataFinish(InEventParam.Info.SyncInfo.pParam);//收回q网内存
							break;
						case QWE_Error:
							MyGobalPeripEvtHandler(Perip_QWebError,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebError)==HasPagePeripEvt)//检查是否允许触发
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebError,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
					}
				}
				break;
			case	SingleNum_Type:
				if(Input_ExtiKey==InEventParam.EventType)//外部实体按键事件
				{	//Num最低位放状态，高位放键值
					ExtiKeyNum=(InEventParam.Num>>1)-EXTI_KEY_VALUE_START;//得到按键号，注意在发送按键值时区分
					if(InEventParam.Num&0x01)//key press
					{
						ExtiKeyInfo[ExtiKeyNum].Id++;
						TickStamp[ExtiKeyNum]=OS_GetCurrentTick();
						ExtiKeyInfo[ExtiKeyNum].TimeStamp=0;
						MyGobalPeripEvtHandler(Perip_KeyPress,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
						if(Q_InspectPeripEvt(PRID_Current,Perip_KeyPress)==HasPagePeripEvt)//检查是否允许触发
							if(gpCurrentPage->PeripEvtHandler)//这里要检查是因为按键可能在还没进入第一个页面的时候就按下了。
								CoMsg=gpCurrentPage->PeripEvtHandler(Perip_KeyPress,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
					}
					else	//key release
					{
						ExtiKeyInfo[ExtiKeyNum].TimeStamp=(OS_GetCurrentTick()-TickStamp[ExtiKeyNum])*OS_TICK_RATE_MS;
						MyGobalPeripEvtHandler(Perip_KeyRelease,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
						if(Q_InspectPeripEvt(PRID_Current,Perip_KeyRelease)==HasPagePeripEvt)//检查是否允许触发
							if(gpCurrentPage->PeripEvtHandler)//这里要检查是因为按键可能在还没进入第一个页面的时候就按下了。
								CoMsg=gpCurrentPage->PeripEvtHandler(Perip_KeyRelease,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
					}
					//OS_TaskDelayMs(20);
				}
				else if(Input_Timer==InEventParam.EventType)
				{
					MyGobalPeripEvtHandler(Perip_Timer,InEventParam.Num,NULL);
					if(Q_InspectPeripEvt(PRID_Current,Perip_Timer)==HasPagePeripEvt)
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_Timer,InEventParam.Num,NULL);
				}
				else if(Input_LcdOn==InEventParam.EventType)
				{
					MyGobalPeripEvtHandler(Perip_LcdOn,-1,NULL);
					if(Q_InspectPeripEvt(PRID_Current,Perip_LcdOn)==HasPagePeripEvt)
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_LcdOn,-1,NULL);
				}
				else if(Input_MscPlay==InEventParam.EventType)
				{
					MyGobalPeripEvtHandler(Perip_MscPlay,InEventParam.Num,(void *)Q_MusicGetPath());
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscPlay)==HasPagePeripEvt)
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_MscPlay,InEventParam.Num,(void *)Q_MusicGetPath());
				}
				else if(Input_MscPause==InEventParam.EventType)
				{
					MyGobalPeripEvtHandler(Perip_MscPause,InEventParam.Num,(void *)Q_MusicGetPath());
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscPause)==HasPagePeripEvt)
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_MscPause,InEventParam.Num,(void *)Q_MusicGetPath());
				}
				else if(Input_MscContinue==InEventParam.EventType)
				{
					MyGobalPeripEvtHandler(Perip_MscContinue,InEventParam.Num,(void *)Q_MusicGetPath());
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscContinue)==HasPagePeripEvt)
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_MscContinue,InEventParam.Num,(void *)Q_MusicGetPath());
				}
				else if(Input_MscStop==InEventParam.EventType)
				{
					MyGobalPeripEvtHandler(Perip_MscStop,InEventParam.Num,(void *)Q_MusicGetPath());
					if(Q_InspectPeripEvt(PRID_Current,Perip_MscStop)==HasPagePeripEvt)
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_MscStop,InEventParam.Num,(void *)Q_MusicGetPath());
				}
				break;
			case Rtc_Type:
				{
					u8 ItemsNum=0;
					
					while(InEventParam.Info.Items[ItemsNum]!=0)
					{
						if(Input_RtcSec==InEventParam.Info.Items[ItemsNum])
						{
							MyGobalPeripEvtHandler(Perip_RtcSec,InEventParam.Num,NULL);
							if(Q_InspectPeripEvt(PRID_Current,Perip_RtcSec)==HasPagePeripEvt)
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_RtcSec,InEventParam.Num,NULL);
						}
						else if(Input_RtcMin==InEventParam.Info.Items[ItemsNum])
						{
							MyGobalPeripEvtHandler(Perip_RtcMin,InEventParam.Num,NULL);
							if(Q_InspectPeripEvt(PRID_Current,Perip_RtcMin)==HasPagePeripEvt)
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_RtcMin,InEventParam.Num,NULL);
						}
						else if(Input_RtcAlarm==InEventParam.Info.Items[ItemsNum])
						{
							MyGobalPeripEvtHandler(Perip_RtcAlarm,InEventParam.Num,NULL);
							if(Q_InspectPeripEvt(PRID_Current,Perip_RtcAlarm)==HasPagePeripEvt)
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_RtcAlarm,InEventParam.Num,NULL);
						}	
						else if(Input_LcdOff==InEventParam.Info.Items[ItemsNum])
						{
							MyGobalPeripEvtHandler(Perip_LcdOff,0,NULL);
							if(Q_InspectPeripEvt(PRID_Current,Perip_LcdOff)==HasPagePeripEvt)
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_LcdOff,0,NULL);
						}
						ItemsNum++;
					}
				}
				break;
			default:
				Debug("Error:No such event type!!!\n\r");
		}

		//系统信息处理
		if((SysMsg&SM_TouchOff)||(CoMsg&CO_TouchOff)) Disable_Touch_Inperrupt();
		if((SysMsg&SM_TouchOn)||(CoMsg&CO_TouchOn)) Enable_Touch_Inperrupt();
		
		if((SysMsg&SM_ExtiKeyOff)||(CoMsg&CO_ExtiKeyOff)) OS_TaskSuspend(KeysHandler_Task_Handle);//挂起按键监控线程
		if((SysMsg&SM_ExtiKeyOn)||(CoMsg&CO_ExtiKeyOn)) OS_TaskResume(KeysHandler_Task_Handle);//恢复按键监控线程
	}
}


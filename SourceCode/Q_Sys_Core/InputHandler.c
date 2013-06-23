/*��ϵͳ ˵��
1.���Ľ��ǿ�ϵͳ�к���Ҫ��һ���ļ�������ΪInputHandler�̣߳�������ҳ��
��ת������ʵ�ֺ�ϵͳ�¼��ķ�����

2.���ļ������к�����ֻ��InputHandler_Taskʹ�ã��������ÿ��ҳ���
SystemEventHandler������TouchEventHandler������ÿ��ҳ�������������������
InputHandler_Task��һ���֡�Ҳֻ���������������ܲ���lcd��Դ�����Լ���
SystemEventHandler�����������µ��̣߳��µ��߳�Ҫ�ı�lcd��ʾ��Ҳ����ͨ��
�¼����Ƹ���InputHandler_Task����InputHandler_Taskȥ����SystemEventHandler��lcd
���д����������Աһ��Ҫ�����

3.ÿ��ҳ���SystemEventHandler�����ڵ�һ�����е�ʱ�򣬶���Ȩ���½�����
���̣߳�����Ϊ��ҳ���ͬ��������ǰҳ���˳�ʱ����ҳ���Ӧ���߳�
Ҳ�����ͣ�������ҳ���߳���ѭ����Ҫ��PageRunTaskLoopStart��PageRunTaskLoopStop��
ԭ�򡣵�Ȼ����Ҳ���Բ����������꣬�����Ļ�������̲߳��ᱻ��ͣ��
�����ں�̨���С�������Ҫ������̣��Է�ϵͳ����

*/

#include "System.h"
#include "AllPage.h"

#define Input_Debug Debug 

#if 1 //ϵͳ�ں�ȫ�ֱ���
const PAGE_ATTRIBUTE *gpCurrentPage=NULL;	//��ǰҳ��ָ��

static u8 PageTracks[MAX_PAGE_TRACK_NUM];//���ڼ�¼ҳ����ת��¼
static u8 gCurTrackIdx;//��ǰTrack��¼����

static u8 PageLayers[MAX_PAGE_LAYER_NUM+1];//���ڼ�¼ҳ��㼶��[0]�����ã�����Ҫ��һ��Ԫ��
static u8 gCurLayerNum;//���ڼ�¼ҳ�����

u16 gEntriesOfPage[PAGE_TOTAL];//��¼ÿ��ҳ�����Ĵ���
u32 gPagePeripEvtFlag[PAGE_TOTAL];//��¼ÿ��ҳ�����Χ�¼���Ӧ��־

GOBAL_PERIPEVT_RECORD gGobalPeripEvtRecord[MAX_GOBAL_SYSEVT];//��¼����ҳ���ȫ���¼���
u32 gGobalPeripEvtBitFlag;//ȫ���¼���־
QSH_VAR_REG(gGobalPeripEvtBitFlag,"u32 gGobalPeripEvtBitFlag","u32");

SYS_MSG gCurrSysMsg=SM_State_OK;//��¼ҳ��case���ص���Ϣ
u8 gPageHeapRecord=0;//��¼�û�ҳ���ջ������Ŀ�ı���
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

//����ȫ���¼���Ӧ
SYS_MSG MyGobalPeripEvtHandler(PERIP_EVT PeripEvt,int intParam, void *pParam)
{
	SYS_MSG SysMsg=0;
	u8 i;

	if(!ReadBit(gGobalPeripEvtBitFlag,PeripEvt)) return 0;//û�д����¼�
	
	//�ҵ�ƥ���¼
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
 		if(gGobalPeripEvtRecord[i].PeripEvt==PeripEvt)//�¼����Ϻ���
 		{
			SysMsg|=gGobalPeripEvtRecord[i].GobalPeripEvtHandler(PeripEvt,intParam,pParam);
 		}
	}	

	return SysMsg;
}

//��һ��ҳ��ӵ�ǰҳ���л�Ϊ�ǵ�ǰҳ��ʱ���õĺ���
//SubPage==TRUE��ʾ�������������ҳ��
static SYS_MSG OldPageClean(INPUT_EVT_TYPE EventType,u8 NewPageIndex)
{
	SYS_MSG SysMsg=SM_State_Faile;
	u8 LayerNum;
	
	Input_Debug("%s : Clean %s\n\r",__FUNCTION__,gpCurrentPage->Name);

	switch(EventType)
	{
		case Input_GotoSubPage://������ҳ��
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_PreSubPage,GetRegIdByIdx(NewPageIndex),
				(void *)GetPageByIdx(NewPageIndex));
			break;
		case Input_SubPageReturn:	//����ҳ�淵��
		case Input_SubPageTranslate://��ҳ���л�
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_PageClean,GetRegIdByIdx(NewPageIndex),
				(void *)GetPageByIdx(NewPageIndex));
			break;
		case Input_GotoNewPage://������ҳ��
			LayerNum=GetCurLayerNum();
			for(;LayerNum;LayerNum--)//����һ��ҳ���������
			{
				SysMsg|=Q_GetPageByLayer(LayerNum)->SysEvtHandler(Sys_PageClean,GetRegIdByIdx(NewPageIndex),
					(void *)GetPageByIdx(NewPageIndex));
			}
			break;
	}

	Input_Debug("%s end: Clean %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return SysMsg;
}

//������ҳ��ʱ�Ƚ��г�ʼ��
static SYS_MSG CurrentPageInit(INPUT_EVT_TYPE EventType,int IntParam,void *pSysParam)
{
	SYS_MSG SysMsg=SM_State_Faile;
	
	Input_Debug("%s : %s\n\r",__FUNCTION__,gpCurrentPage->Name);

	switch(EventType)
	{
		case Input_GotoSubPage://������ҳ��
		case Input_SubPageTranslate://�����µ�ͬ����ҳ��
		case Input_GotoNewPage://������ҳ��
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_PageInit,IntParam,pSysParam);
			break;
		case Input_SubPageReturn://����ҳ�淵��
			SysMsg=gpCurrentPage->SysEvtHandler(Sys_SubPageReturn,IntParam,pSysParam);
			break;
	}
		
	Input_Debug("%s end: %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return SysMsg;
}

//KeyId - �������
//KeyStaus - ״ֵ̬ 0(release) or 1(press)
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

	//�رմ������ⲿ�������룬��֤�ڻ�ҳ����߳���Ĺ����к�����������
	Q_DisableInput();

	//��������״̬��Cleanҳ��
	if(gpCurrentPage)
	{
		if((!(gCurrSysMsg&SM_NoPageClean)) //Q_GotoPage����ֵ����ϵͳ��Ҫִ��Sys_PageClean��Sys_PreSubPage�¼�
			&& (GetPageByIdx(PageIdx)->Type != POP_PAGE)) //�����popҳ�棬Ҳ����Ҫclean
			gCurrSysMsg|=OldPageClean(EventType,PageIdx);
		Debug("�ġġġġġ� Leave %s �ġġġġġ�\n\r",gpCurrentPage->Name);
	}
	else	//û����һ��ҳ��
	{

	}	

	//4--------------------------���ݴ���-----------------------------
	OS_EnterCritical();//Ŀǰ��ֻ��һ���̲߳�����Щȫ�ֱ����������ٽ��������Ǳ���ġ�

	//��¼ҳ��Tarck
	InsertPageTrack(PageIdx);

	//��¼ҳ��㼶
	if(EventType==Input_GotoSubPage) //����ǽ�����ҳ��
	{
		PushPageCtrlObjData();//����ǰ�����������
		CleanPageCtrlObjData();//���ҳ������
		InsertPageLayer(PageIdx);//����һ��
	}
	else if(EventType==Input_SubPageReturn)
	{
		DeletePageLayer(1);//ɾ��һ��
		PopPageCtrlObjData(FALSE);//��ԭǰ�����������
	}
	else if(EventType==Input_GotoNewPage)
	{
		DeletePageLayer(0xff);//ɾ�����в�
		PopPageCtrlObjData(TRUE);//ɾ�����д洢����������
		CleanPageCtrlObjData();//���ҳ������
		InsertPageLayer(PageIdx);//����һ��

		//ҳ��Ѽ��
		if(gPageHeapRecord)
		{
			Debug("gPageHeapRecord:%d\n\r",gPageHeapRecord);
			Q_ErrorStopScreen("!!!Warnning!!! Page momery don't be freed!");
		}
	}
	else if(EventType==Input_SubPageTranslate)
	{
		DeletePageLayer(1);//ɾ��һ��
		CleanPageCtrlObjData();//���ҳ������
		InsertPageLayer(PageIdx);//����һ��
	}
	
	//������ָ��
	gpCurrentPage=Q_GetPageByTrack(0);
	gEntriesOfPage[PageIdx]++;//��ҳ��һ���Լ�һ��

	PageSwithcCtrlObjDataHandler(gpCurrentPage);//�ؼ����ݴ���
	
	OS_ExitCritical();
	//4-------------------------���ݴ������--------------------------

	TimeMsRecord=OS_GetCurrentSysMs();
	Debug("\n\r�ššššš� Goto %s �ššššš�\n\r",gpCurrentPage->Name);
	Debug("##Author:%s\n\r",gpCurrentPage->Author);
	Debug("##Description:%s\n\r",gpCurrentPage->Description);

	Debug("##Page Layers:");
	for(i=1;i<GetCurLayerNum();i++) Debug("%s->",GetPageByIdx(PageLayers[i])->Name); 
	Debug("%s\n\r",GetPageByIdx(PageLayers[i])->Name); 

	if((!(gCurrSysMsg&SM_NoPageInit)) //Q_GotoPage����ֵ����ϵͳ��Ҫִ��page init
		&&(Q_GetPageByTrack(1)->Type!=POP_PAGE)) //��popҳ�淵�أ�Ҳ����Ҫpage init
		gCurrSysMsg|=CurrentPageInit(EventType,IntParam,pSysParam);

	if((!(gCurrSysMsg&SM_NoTouchInit))//Q_GotoPage��ҳ��SystemEventHandler��������ֵ����ϵͳ����Ҫtouch init
		&&(Q_GetPageByTrack(1)->Type!=POP_PAGE))	 //��popҳ�淵�أ�Ҳ����Ҫtouch init
		gCurrSysMsg|=CurrPageCtrlObjInit(EventType,IntParam,pSysParam);	

	if(!(gCurrSysMsg&SM_NoPopReturn)//��ǰ��ĺ�����������
		&&(Q_GetPageByTrack(1)->Type==POP_PAGE))//ǰһ��ҳ����popҳ��
		gCurrSysMsg|=gpCurrentPage->SysEvtHandler(Sys_PopPageReturn,IntParam,pSysParam);

	switch(GetRegIdByIdx(GetPageIdxByTrack(1)))//�жϴ��ĸ�ҳ�淵��
	{
		case PRID_NumCtrlObjPage: //��num�ؼ�ҳ����
			{
				NUM_CTRL_OBJ *pNumCtrlObj=pSysParam;
				if(gpCurrentPage->NumCtrlObjHander)
					gpCurrentPage->NumCtrlObjHander(pNumCtrlObj->ObjID,pNumCtrlObj->Value,pSysParam);
				else 
					Debug("!!!Not Define Handler Function In Page Struct!!!\n\r");
			}break;
		case PRID_StrCtrlObjPage: //��Str�ؼ�ҳ����
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
			while((OS_GetCurrentSysMs()-TimeMsRecord)<300) OS_TaskDelayMs(50);//ѭ����ʱ300ms���Ա��ⴥ����Ӧ����
	}
	
	Q_EnableInput();
	
	Input_Debug("%s end: %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return gCurrSysMsg;
}

//���ڴ���ؼ������¼�����
//Num��洢����touch�̵߳Ĵ�����������
//InType:	Input_TchPress,//��������
//				Input_TchContinue,//���ִ���״̬�����ڴ��ݳ���ʱ��ʵʱ����
//				Input_TchRelease,//����Ч�������ɿ�,��Ч����ָPressʱ����ע������
//				Input_TchReleaseVain,//�ڷ���Ч�����ɿ�
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

//ϵͳȫ�����ݳ�ʼ��
static void QSYS_DataInit(void)
{
	u16 i;
	
	MemSet(PageTracks,0,MAX_PAGE_TRACK_NUM);//���ҳ����ת�ۼ�
	gCurTrackIdx=0;
	
	MemSet(PageLayers,0,MAX_PAGE_LAYER_NUM+1);//���ҳ��㼶��¼
	gCurLayerNum=0;

	MemSet(gEntriesOfPage,0,PAGE_TOTAL);//���ҳ����������¼

	//��������ҳ����¼����뵽��¼
	for(i=0;i<PAGE_TOTAL;i++)
	{
		gPagePeripEvtFlag[i]=AllPage[i].pPage->PeripEvtInitMask;

		//˳����ҳ��Ϸ���
		if((AllPage[i].pPage->SysEvtHandler==NULL)||(AllPage[i].pPage->ButtonHandler==NULL)
			||(AllPage[i].pPage->PeripEvtHandler==NULL))
			Q_ErrorStopScreen("Page SysEvtHandler & TchEvtHandler no allow be NULL!");
	}

	MemSet(gGobalPeripEvtRecord,0,MAX_GOBAL_SYSEVT*sizeof(GOBAL_PERIPEVT_RECORD));//���ȫ���¼���¼
	gGobalPeripEvtBitFlag=0;

	ControlObjInit();
}

extern void QSYS_Init(void);
//��������Դ����������¼����д��������л���õ�ǰҳ���TouchEventHandler������
//���º���Ϊ������ר��
// 1.��ҳ�µ����к���
// 2.��ǰҳ���SystemEventHandler ������TouchEventHandler���������ֿؼ�����
//���ԣ������߳��벻Ҫ������Щ����
extern void *KeysHandler_Task_Handle;
void InputHandler_Task( void *Task_Parameters )
{
	INPUT_EVENT InEventParam;
	EXIT_KEY_INFO ExtiKeyInfo[EXTI_KEY_MAX_NUM];//���ڴ洢�ⲿ������Ϣ
	u32 TickStamp[EXTI_KEY_MAX_NUM];
	u16 ExtiKeyNum;
	SYS_MSG SysMsg;
	CO_MSG CoMsg;

	QSYS_Init();//ϵͳ��ʼ��
	QSYS_DataInit();//ϵͳ���ݳ�ʼ��

	MemSet(ExtiKeyInfo,0,sizeof(EXIT_KEY_INFO)*EXTI_KEY_MAX_NUM);

	Q_GotoPage(GotoNewPage,"MainPage",0,NULL);	//����ҳ;
	
	while(1)
	{
		OS_TaskStkCheck(FALSE);
		//Debug("Wait Input Event!\n\r");
		OS_MsgBoxReceive(gInputHandler_Queue,&InEventParam,OS_MAX_DELAY);//�ȴ���Ч������
		
		//if(Sync_Type==InEventParam.uType)
			//if(Input_GotoSubPage==InEventParam.EventType) Debug("@@@Gopage\n\r");
		
		SysMsg=CoMsg=0;
		switch(InEventParam.uType)//�Ե�һ����������ж�
		{
			case Touch_Type:
				CoMsg=CtrlObjTypeHandler(InEventParam.EventType,InEventParam.Num,&InEventParam.Info.TouchInfo);
				break; 
			case Sync_Type:
				if(Input_PageSync==InEventParam.EventType)//����ҳ��ͬ��
				{
					SysMsg=gpCurrentPage->SysEvtHandler(Sys_PageSync,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
				}
				else if((Input_GotoNewPage==InEventParam.EventType)
						|(Input_GotoSubPage==InEventParam.EventType)
						|(Input_SubPageReturn==InEventParam.EventType))//����ҳ����ת
				{
					SysMsg=GotoPageHandler(InEventParam.EventType,InEventParam.Num,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
				}
				else if(Input_UartInput==InEventParam.EventType)//����������
				{
					MyGobalPeripEvtHandler(Perip_UartInput,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
					if(Q_InspectPeripEvt(PRID_Current,Perip_UartInput)==HasPagePeripEvt)//����Ƿ�������
						SysMsg=gpCurrentPage->PeripEvtHandler(Perip_UartInput,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
				}
				else if(Input_QWebEvt==InEventParam.EventType)//����q���¼�
				{
					switch(InEventParam.Num)
					{
						case QWE_NewJoin:
							MyGobalPeripEvtHandler(Perip_QWebJoin,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebJoin)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebJoin,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							QWA_CopyDataFinish(InEventParam.Info.SyncInfo.pParam);//�ջ�q���ڴ�
							break;
						case QWE_Recv:
							MyGobalPeripEvtHandler(Perip_QWebRecv,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebRecv)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebRecv,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							QWA_CopyDataFinish(InEventParam.Info.SyncInfo.pParam);//�ջ�q���ڴ�
							break;
						case QWE_SendOk:
							MyGobalPeripEvtHandler(Perip_QWebSendOk,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebSendOk)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebSendOk,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
						case QWE_SendFailed:
							MyGobalPeripEvtHandler(Perip_QWebSendFailed,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebSendFailed)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebSendFailed,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
						case QWE_HostConflict:
							MyGobalPeripEvtHandler(Perip_QWebHostConflict,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebHostConflict)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebHostConflict,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
						case QWE_QueryAck:
							MyGobalPeripEvtHandler(Perip_QWebQueryName,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebQueryName)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebQueryName,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							QWA_CopyDataFinish(InEventParam.Info.SyncInfo.pParam);//�ջ�q���ڴ�
							break;
						case QWE_Error:
							MyGobalPeripEvtHandler(Perip_QWebError,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							if(Q_InspectPeripEvt(PRID_Current,Perip_QWebError)==HasPagePeripEvt)//����Ƿ�������
								SysMsg=gpCurrentPage->PeripEvtHandler(Perip_QWebError,InEventParam.Info.SyncInfo.IntParam,InEventParam.Info.SyncInfo.pParam);
							break;
					}
				}
				break;
			case	SingleNum_Type:
				if(Input_ExtiKey==InEventParam.EventType)//�ⲿʵ�尴���¼�
				{	//Num���λ��״̬����λ�ż�ֵ
					ExtiKeyNum=(InEventParam.Num>>1)-EXTI_KEY_VALUE_START;//�õ������ţ�ע���ڷ��Ͱ���ֵʱ����
					if(InEventParam.Num&0x01)//key press
					{
						ExtiKeyInfo[ExtiKeyNum].Id++;
						TickStamp[ExtiKeyNum]=OS_GetCurrentTick();
						ExtiKeyInfo[ExtiKeyNum].TimeStamp=0;
						MyGobalPeripEvtHandler(Perip_KeyPress,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
						if(Q_InspectPeripEvt(PRID_Current,Perip_KeyPress)==HasPagePeripEvt)//����Ƿ�������
							if(gpCurrentPage->PeripEvtHandler)//����Ҫ�������Ϊ���������ڻ�û�����һ��ҳ���ʱ��Ͱ����ˡ�
								CoMsg=gpCurrentPage->PeripEvtHandler(Perip_KeyPress,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
					}
					else	//key release
					{
						ExtiKeyInfo[ExtiKeyNum].TimeStamp=(OS_GetCurrentTick()-TickStamp[ExtiKeyNum])*OS_TICK_RATE_MS;
						MyGobalPeripEvtHandler(Perip_KeyRelease,ExtiKeyNum+EXTI_KEY_VALUE_START,&ExtiKeyInfo[ExtiKeyNum]);
						if(Q_InspectPeripEvt(PRID_Current,Perip_KeyRelease)==HasPagePeripEvt)//����Ƿ�������
							if(gpCurrentPage->PeripEvtHandler)//����Ҫ�������Ϊ���������ڻ�û�����һ��ҳ���ʱ��Ͱ����ˡ�
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

		//ϵͳ��Ϣ����
		if((SysMsg&SM_TouchOff)||(CoMsg&CO_TouchOff)) Disable_Touch_Inperrupt();
		if((SysMsg&SM_TouchOn)||(CoMsg&CO_TouchOn)) Enable_Touch_Inperrupt();
		
		if((SysMsg&SM_ExtiKeyOff)||(CoMsg&CO_ExtiKeyOff)) OS_TaskSuspend(KeysHandler_Task_Handle);//���𰴼�����߳�
		if((SysMsg&SM_ExtiKeyOn)||(CoMsg&CO_ExtiKeyOn)) OS_TaskResume(KeysHandler_Task_Handle);//�ָ���������߳�
	}
}


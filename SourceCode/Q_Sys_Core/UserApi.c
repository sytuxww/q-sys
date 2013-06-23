#include "System.h"

#define UA_Debug Debug

extern const u8 gPageTotal;

extern u16 gEntriesOfPage[];//��¼ÿ��ҳ�����Ĵ���
extern u32 gPagePeripEvtFlag[];//��¼ÿ��ҳ�����Χ�¼���Ӧ��־
extern GOBAL_PERIPEVT_RECORD gGobalPeripEvtRecord[];//��¼����ҳ���ȫ���¼���
extern u32 gGobalPeripEvtBitFlag;//ȫ���¼���־
extern SYS_MSG gCurrSysMsg;//��¼ҳ��case���ص���Ϣ
extern u8 gPageHeapRecord;//��¼�û�ҳ���ջ������Ŀ�ı���

extern const PAGE_ATTRIBUTE *GetPageByIdx(u8);
extern PAGE_RID GetRegIdByIdx(u8);
extern u8 GetPageIdxByTrack(u8);
extern u8 GetPageIdxByLayerOffset(u8);
extern u8 GetPageIdxByLayer(u8);
extern SYS_MSG FindPage(u8 *,u32,u8 *);

//�û����õ�ϵͳ������ʼ
//ҳ��ʹ�õ��ڴ���亯��
#if Q_HEAP_TRACK_DEBUG ==1
void *_Q_PageMallco(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *_Q_PageMallco(u16 Size)
#endif
{
	gPageHeapRecord++;
#if Q_HEAP_TRACK_DEBUG ==1
	return QS_Mallco(Size,pFuncName,Lines);
#else
	return QS_Mallco(Size);
#endif
}

//ҳ��ʹ�õ��ڴ��ͷź���
#if Q_HEAP_TRACK_DEBUG ==1
void _Q_PageFree(void *Ptr,u8 *pFuncName,u32 Lines)
#else
void _Q_PageFree(void *Ptr)
#endif
{
	gPageHeapRecord--;
#if Q_HEAP_TRACK_DEBUG ==1
	QS_Free(Ptr,pFuncName,Lines);
#else
	QS_Free(Ptr);
#endif
}
extern void *KeysHandler_Task_Handle;
//�������������룬�ⲿ��������
void Q_EnableInput(void)
{
	Enable_Touch_Inperrupt();
	OS_TaskResume(KeysHandler_Task_Handle);//�ָ���������߳�
}

//�رմ��������룬�ⲿ��������
void Q_DisableInput(void)
{
	Disable_Touch_Inperrupt();
	OS_TaskSuspend(KeysHandler_Task_Handle);//���𰴼�����߳�
}

//��ȡҳ������
u8 Q_GetPageTotal(void)
{
	return gPageTotal;
}

//ָ����Ե�ǰҳ��ĺۼ�ƫ��ֵ������ҳ��ָ��
//��GetPageByTrack(0)���ص�ǰҳ��ָ��
//Q_GetPageByTrack(1)����ǰһҳ��ָ��
const PAGE_ATTRIBUTE *Q_GetPageByTrack(u8 Local)
{
	return GetPageByIdx(GetPageIdxByTrack(Local));
}

//LayerOffset=0,���ص�ǰҳ��ָ��
//LayerOffset=1,������һ��ҳ��ָ��
const PAGE_ATTRIBUTE *Q_GetPageByLayerOffset(u8 LayerOffset)
{
	return GetPageByIdx(GetPageIdxByLayerOffset(LayerOffset));
}

//�õ�ָ�����ҳ��ָ��
//LayerNum=1,�õ�����
//LayerNum=2,�õ��ڶ���
const PAGE_ATTRIBUTE *Q_GetPageByLayer(u8 LayerNum)
{
	return GetPageByIdx(GetPageIdxByLayer(LayerNum));
}

//ͨ��ҳ��������ҳ���RegID
//�����ڲ���ΪNULL�򷵻ص�ǰҳ���RegID
PAGE_RID Q_FindRidByPageName(u8 *PageName)
{
	u8 PageIdx;

	if((PageName==NULL)||(PageName[0]==0))
	{
		return GetRegIdByIdx(GetPageIdxByTrack(0));
	}
	
	for(PageIdx=0;PageIdx<gPageTotal;PageIdx++)
	{
		if(strcmp((void *)PageName,(void *)GetPageByIdx(PageIdx)->Name)) continue;
		else  return GetRegIdByIdx(PageIdx);//�ҵ�ָ����ҳ����
	}

	Debug("No Such Page PageName:%s ,may be this is a RID\n\r",PageName);
	return PRID_Null;
}

//��ȡ��ǰҳ������
u8 *Q_GetCurrPageName(void)
{
	return Q_GetPageByTrack(0)->Name;
}

//��ȡ��ǰҳ��������
u16 Q_GetPageEntries(void)
{
	return gEntriesOfPage[GetPageIdxByTrack(0)];
}

//ת��ĳҳ��,���ô˺�����InputHandler���߳�����ɵ�ǰ����󣬻Ὺʼת������
//return TRUE :����ת��
//return FALSE : ������ת��
//ҳ����ҳ��֮��Ĳ������ݿ���pInfoParamָ��
SYS_MSG Q_GotoPage(PAGE_ACTION PageAction, u8 *Name, int IntParam, void *pSysParam)
{
	INPUT_EVENT InEventParam;
	u8 PageIdx;
	u8 Result;
	
	if(gpCurrentPage)
		UA_Debug("%s : %s->%s\n\r",__FUNCTION__,gpCurrentPage->Name,Name);
	else
		UA_Debug("%s : NULL->%s\n\r",__FUNCTION__,Name);
	
	//���Ҷ�Ӧҳ��
	if(PageAction==SubPageReturn) //�������ҳ�淵�أ����ҵ���һ��ҳ��
	{
		gCurrSysMsg=SM_State_OK;
		PageIdx=GetPageIdxByLayerOffset(1);
	}
	else
	{
		gCurrSysMsg=FindPage(Name,0,&PageIdx);//��ʼ��ҳ��ش���Ϣ
		if(gCurrSysMsg!=SM_State_OK)//û�ҵ���Ӧҳ��
		{
			return gCurrSysMsg;
		}
	}

	if(GetPageByIdx(PageIdx)->Type==POP_PAGE)//Ҫ�����ҳ����popҳ��
		if((PageAction!=GotoSubPage)&&(PageAction!=SubPageReturn))
		{
			Q_ErrorStopScreen("Pop Page not allow entry by \"GotoNewPage\" & \"SubPageTranslate\" param!");
			return SM_State_Faile;
		}

	//POPҳ��ֻ��������ҳ�淵�ص���ʽ�˳�
	if((Q_GetPageByTrack(0)->Type==POP_PAGE)&&(PageAction!=SubPageReturn))
	{
		Q_ErrorStopScreen("Pop Page only allow quit by \"SubPageReturn\" param!");
		return SM_State_Faile;
	}

	if(Q_GetPageByTrack(0)->Type!=POP_PAGE) //�����popҳ�淵�أ���ô����Ҫִ�����goto case
		gCurrSysMsg=GetPageByIdx(PageIdx)->SysEvtHandler(Sys_PreGotoPage, IntParam,pSysParam);

	//Debug("GotoPage Return 0x%x\n\r",SysMsg);
	
	if(gCurrSysMsg&SM_NoGoto)//ҳ���Sys_Goto_Page���ݻص���Ϣ
	{//��ҳ���Sys_PreGotoPage���ݻ�SM_NoGoto��Ϣ��ʾ����Ҫ�����ҳ���ˡ�
		return gCurrSysMsg;
	}
	else
	{
		InEventParam.uType=Sync_Type;
		switch(PageAction)
		{
			case GotoNewPage:
				InEventParam.EventType=Input_GotoNewPage;break;
			case GotoSubPage:
				InEventParam.EventType=Input_GotoSubPage;break;
			case SubPageReturn:
				InEventParam.EventType=Input_SubPageReturn;break;
		}
		InEventParam.Num=PageIdx;
		InEventParam.Info.SyncInfo.IntParam=IntParam;
		InEventParam.Info.SyncInfo.pParam=pSysParam;
		//Debug("New Page Index:%d\n\r",PageIdx);
		if((Result=OS_MsgBoxSend(gInputHandler_Queue,&InEventParam,100,FALSE))==OS_ERR_NONE)
		{
			return gCurrSysMsg;
		}
		else
		{
			Debug("GotoPage Send Msg Error!%d\n\r",Result);
			return SM_State_Faile;
		}
	}	
}

//����ϵͳ�¼���Ӧλ
void Q_SetPeripEvt(PAGE_RID RegID,u32 PeripEvtCon)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) //�˴�״̬��ʧ�
		{
			Q_ErrorStopScreen("Can't find page!\n\r");
		}
	}
	else //���RegIDΪ0�����ص�ǰҳ����¼���־
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	gPagePeripEvtFlag[PageIdx]|=PeripEvtCon;
	OS_ExitCritical();
}

//���ϵͳ�¼���Ӧλ
void Q_ClrPeripEvt(PAGE_RID RegID,u32 PeripEvtCon)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return;//�˴�״̬��ʧ��
	}
	else //���RegIDΪ0�����ص�ǰҳ����¼���־
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	gPagePeripEvtFlag[PageIdx]&=PeripEvtCon;
	OS_ExitCritical();
}

//��ϵͳ�¼���־
void Q_EnablePeripEvt(PAGE_RID RegID,PERIP_EVT PeripEvt)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return;//�˴�״̬��ʧ��
	}
	else //���RegIDΪ0�����ص�ǰҳ����¼���־
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	SetBit(gPagePeripEvtFlag[PageIdx],PeripEvt);
	OS_ExitCritical();
}

//�ر�ϵͳ�¼���־
void Q_DisablePeripEvt(PAGE_RID RegID,PERIP_EVT PeripEvt)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return;//�˴�״̬��ʧ��
	}
	else //���RegIDΪ0�����ص�ǰҳ����¼���־
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	ClrBit(gPagePeripEvtFlag[PageIdx],PeripEvt);
	OS_ExitCritical();
}

//�鿴ϵͳ�¼�
//���RegIDΪ0�����ص�ǰҳ����¼���־
INSPECT_SYSEVT_RET Q_InspectPeripEvt(PAGE_RID RegID,PERIP_EVT PeripEvt)
{
	u8 PageIdx;
	
	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return NoHasSysEvt;//�˴�״̬��ʧ��
	}
	else //���RegIDΪ0�����ص�ǰҳ����¼���־
	{
		PageIdx=GetPageIdxByTrack(0);
	}

	if(ReadBit(gPagePeripEvtFlag[PageIdx],PeripEvt)) return HasPagePeripEvt;
	if(ReadBit(gGobalPeripEvtBitFlag,PeripEvt)) return HasGobalSysEvt;
	return NoHasSysEvt;
}

//����ȫ���¼����κ�ҳ���£����ᴥ���¼��Ĵ�����SysEventHandler
//����Sys_PreGotoPage������
void Q_EnableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler)
{
	u8 i;

	if(PeripEvtHandler==NULL) return;
	
	//���Ƿ����ظ���¼
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
		if((gGobalPeripEvtRecord[i].PeripEvt==PeripEvt)&&(gGobalPeripEvtRecord[i].GobalPeripEvtHandler==PeripEvtHandler))
			return;
	}	
	
	//�ҿ�λ��
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
		if(gGobalPeripEvtRecord[i].PeripEvt==0)break;
	}
	if(i==MAX_GOBAL_SYSEVT) 
	{
		Q_ErrorStopScreen("MAX_GOBAL_SYSEVT is small!!! pls reset it!");
	}

	gGobalPeripEvtRecord[i].PeripEvt=PeripEvt;
	gGobalPeripEvtRecord[i].GobalPeripEvtHandler=PeripEvtHandler;
	SetBit(gGobalPeripEvtBitFlag,PeripEvt);
}

//ע��ȫ���¼�
void Q_DisableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler)
{
	u8 i;

	//�ҵ�ƥ���¼
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
		if((gGobalPeripEvtRecord[i].PeripEvt==PeripEvt)&&(gGobalPeripEvtRecord[i].GobalPeripEvtHandler==PeripEvtHandler))
		{
			gGobalPeripEvtRecord[i].PeripEvt=(PERIP_EVT)0;//�����¼
		}
	}	

	for(i=0;i<MAX_GOBAL_SYSEVT;i++) //����Ƿ�������ͬ��ȫ���¼�
	{
		if(gGobalPeripEvtRecord[i].PeripEvt==PeripEvt) return;
	}
	ClrBit(gGobalPeripEvtBitFlag,PeripEvt);//û�о����־
}

//���ڴ���ֹͣ
void Q_ErrorStop(const char *FileName,const char *pFuncName,const u32 Line,const char *Msg)
{
	u32 Gray;
	u16 Color;
	u16 x,y;
	u32 R,G,B;
	GUI_REGION DrawRegion;
	u8 ErrorMsg[256];

	Debug(Msg);

	for(y=0;y<LCD_HIGHT;y++)
		for(x=0;x<LCD_WIDTH;x++)
		{
			Color=Gui_ReadPixel16Bit(x,y);
			R=((Color&0x1f)<<3);
			G=(((Color>>5)&0x3f)<<2);
			B=(((Color>>11)&0x1f)<<3);	
			Gray =(R*38 + G*75 + B*15) >> 7;
			//if((x>20)&&(x<LCD_WIDTH-20)&&(y>20)&&(y<LCD_HIGHT-20))
				//Gray=(Gray*9)>>3;if(Gray>0xff) Gray=0xff;
			Gray =((Gray&0xf8)<<8)+((Gray&0xfc)<<3)+((Gray&0xf8)>>3);
			Gui_WritePixel(x,y,Gray);
		}	

	if(gpCurrentPage)
		sprintf((void *)ErrorMsg,"!!!--SYS ERROR STOP--!!!\n\rNow Page:%s\n\rFile:%s\n\rFunction:%s()\n\rLine:%d\nMsg:%s",gpCurrentPage->Name,FileName,pFuncName,Line,Msg);
	else
		sprintf((void *)ErrorMsg,"!!!--SYS ERROR STOP--!!!\n\rFile:%s\n\rFunction:%s()\n\rLine:%d\n\rMsg:%s",FileName,pFuncName,Line,Msg);

	DrawRegion.x=DrawRegion.y=19;
	DrawRegion.w=200;
	DrawRegion.h=280;
	DrawRegion.Color=FatColor(0xffffff);
	DrawRegion.Space=0x00;
	Gui_DrawFont(ASC14B_FONT,ErrorMsg,&DrawRegion);
		
	DrawRegion.x=21;
	DrawRegion.y=19;
	Gui_DrawFont(ASC14B_FONT,ErrorMsg,&DrawRegion);

	DrawRegion.x=19;
	DrawRegion.y=21;
	Gui_DrawFont(ASC14B_FONT,ErrorMsg,&DrawRegion);

	DrawRegion.x=21;
	DrawRegion.y=21;
	Gui_DrawFont(ASC14B_FONT,ErrorMsg,&DrawRegion);

	DrawRegion.x=20;
	DrawRegion.y=20;
	DrawRegion.Color=FatColor(0xff0000);
	Gui_DrawFont(ASC14B_FONT,ErrorMsg,&DrawRegion);

	while(1);
}

//�û����õ�ϵͳ��������


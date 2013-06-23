#include "System.h"

#define UA_Debug Debug

extern const u8 gPageTotal;

extern u16 gEntriesOfPage[];//¼ÇÂ¼Ã¿¸öÒ³Ãæ½øÈëµÄ´ÎÊı
extern u32 gPagePeripEvtFlag[];//¼ÇÂ¼Ã¿¸öÒ³ÃæµÄÍâÎ§ÊÂ¼şÏìÓ¦±êÖ¾
extern GOBAL_PERIPEVT_RECORD gGobalPeripEvtRecord[];//¼ÇÂ¼ËùÓĞÒ³ÃæµÄÈ«¾ÖÊÂ¼ş±í
extern u32 gGobalPeripEvtBitFlag;//È«¾ÖÊÂ¼ş±êÖ¾
extern SYS_MSG gCurrSysMsg;//¼ÇÂ¼Ò³Ãæcase·µ»ØµÄĞÅÏ¢
extern u8 gPageHeapRecord;//¼ÇÂ¼ÓÃ»§Ò³Ãæ¶ÑÕ»·ÖÅäÊıÄ¿µÄ±äÁ¿

extern const PAGE_ATTRIBUTE *GetPageByIdx(u8);
extern PAGE_RID GetRegIdByIdx(u8);
extern u8 GetPageIdxByTrack(u8);
extern u8 GetPageIdxByLayerOffset(u8);
extern u8 GetPageIdxByLayer(u8);
extern SYS_MSG FindPage(u8 *,u32,u8 *);

//ÓÃ»§¿ÉÓÃµÄÏµÍ³º¯Êı¿ªÊ¼
//Ò³ÃæÊ¹ÓÃµÄÄÚ´æ·ÖÅäº¯Êı
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

//Ò³ÃæÊ¹ÓÃµÄÄÚ´æÊÍ·Åº¯Êı
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
//¿ªÆô´¥ÃşÆÁÊäÈë£¬Íâ²¿°´¼üÊäÈë
void Q_EnableInput(void)
{
	Enable_Touch_Inperrupt();
	OS_TaskResume(KeysHandler_Task_Handle);//»Ö¸´°´¼ü¼à¿ØÏß³Ì
}

//¹Ø±Õ´¥ÃşÆÁÊäÈë£¬Íâ²¿°´¼üÊäÈë
void Q_DisableInput(void)
{
	Disable_Touch_Inperrupt();
	OS_TaskSuspend(KeysHandler_Task_Handle);//¹ÒÆğ°´¼ü¼à¿ØÏß³Ì
}

//»ñÈ¡Ò³Ãæ×ÜÊı
u8 Q_GetPageTotal(void)
{
	return gPageTotal;
}

//Ö¸¶¨Ïà¶Ôµ±Ç°Ò³ÃæµÄºÛ¼£Æ«ÒÆÖµ£¬·µ»ØÒ³ÃæÖ¸Õë
//ÈçGetPageByTrack(0)·µ»Øµ±Ç°Ò³ÃæÖ¸Õë
//Q_GetPageByTrack(1)·µ»ØÇ°Ò»Ò³ÃæÖ¸Õë
const PAGE_ATTRIBUTE *Q_GetPageByTrack(u8 Local)
{
	return GetPageByIdx(GetPageIdxByTrack(Local));
}

//LayerOffset=0,·µ»Øµ±Ç°Ò³ÃæÖ¸Õë
//LayerOffset=1,·µ»ØÉÏÒ»²ãÒ³ÃæÖ¸Õë
const PAGE_ATTRIBUTE *Q_GetPageByLayerOffset(u8 LayerOffset)
{
	return GetPageByIdx(GetPageIdxByLayerOffset(LayerOffset));
}

//µÃµ½Ö¸¶¨²ãµÄÒ³ÃæÖ¸Õë
//LayerNum=1,µÃµ½¶¥²ã
//LayerNum=2,µÃµ½µÚ¶ş²ã
const PAGE_ATTRIBUTE *Q_GetPageByLayer(u8 LayerNum)
{
	return GetPageByIdx(GetPageIdxByLayer(LayerNum));
}

//Í¨¹ıÒ³ÃæÃû³ÆÕÒÒ³ÃæµÄRegID
//Èç¹ûÈë¿Ú²ÎÊıÎªNULLÔò·µ»Øµ±Ç°Ò³ÃæµÄRegID
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
		else  return GetRegIdByIdx(PageIdx);//ÕÒµ½Ö¸¶¨µÄÒ³ÃæÁË
	}

	Debug("No Such Page PageName:%s ,may be this is a RID\n\r",PageName);
	return PRID_Null;
}

//»ñÈ¡µ±Ç°Ò³ÃæÃû³Æ
u8 *Q_GetCurrPageName(void)
{
	return Q_GetPageByTrack(0)->Name;
}

//»ñÈ¡µ±Ç°Ò³Ãæ½øÈë´ÎÊı
u16 Q_GetPageEntries(void)
{
	return gEntriesOfPage[GetPageIdxByTrack(0)];
}

//×ªÏòÄ³Ò³Ãæ,µ÷ÓÃ´Ëº¯Êıºó£¬InputHandlerÖ÷Ïß³ÌÔÚÍê³Éµ±Ç°ÈÎÎñºó£¬»á¿ªÊ¼×ªÏò¹¤×÷¡£
//return TRUE :ÔÊĞí×ªÏò
//return FALSE : ²»ÔÊĞí×ªÏò
//Ò³ÃæÓëÒ³ÃæÖ®¼äµÄ²ÎÊı´«µİ¿ÉÓÃpInfoParamÖ¸Õë
SYS_MSG Q_GotoPage(PAGE_ACTION PageAction, u8 *Name, int IntParam, void *pSysParam)
{
	INPUT_EVENT InEventParam;
	u8 PageIdx;
	u8 Result;
	
	if(gpCurrentPage)
		UA_Debug("%s : %s->%s\n\r",__FUNCTION__,gpCurrentPage->Name,Name);
	else
		UA_Debug("%s : NULL->%s\n\r",__FUNCTION__,Name);
	
	//²éÕÒ¶ÔÓ¦Ò³Ãæ
	if(PageAction==SubPageReturn) //Èç¹ûÊÇ×ÓÒ³Ãæ·µ»Ø£¬ÔòÕÒµ½ÉÏÒ»¼¶Ò³Ãæ
	{
		gCurrSysMsg=SM_State_OK;
		PageIdx=GetPageIdxByLayerOffset(1);
	}
	else
	{
		gCurrSysMsg=FindPage(Name,0,&PageIdx);//³õÊ¼»¯Ò³Ãæ»Ø´«ĞÅÏ¢
		if(gCurrSysMsg!=SM_State_OK)//Ã»ÕÒµ½¶ÔÓ¦Ò³Ãæ
		{
			return gCurrSysMsg;
		}
	}

	if(GetPageByIdx(PageIdx)->Type==POP_PAGE)//Òª½øÈëµÄÒ³ÃæÊÇpopÒ³Ãæ
		if((PageAction!=GotoSubPage)&&(PageAction!=SubPageReturn))
		{
			Q_ErrorStopScreen("Pop Page not allow entry by \"GotoNewPage\" & \"SubPageTranslate\" param!");
			return SM_State_Faile;
		}

	//POPÒ³ÃæÖ»ÔÊĞíÒÔ×ÓÒ³Ãæ·µ»ØµÄĞÎÊ½ÍË³ö
	if((Q_GetPageByTrack(0)->Type==POP_PAGE)&&(PageAction!=SubPageReturn))
	{
		Q_ErrorStopScreen("Pop Page only allow quit by \"SubPageReturn\" param!");
		return SM_State_Faile;
	}

	if(Q_GetPageByTrack(0)->Type!=POP_PAGE) //Èç¹û´ÓpopÒ³Ãæ·µ»Ø£¬ÄÇÃ´²»ĞèÒªÖ´ĞĞÕâ¸ögoto case
		gCurrSysMsg=GetPageByIdx(PageIdx)->SysEvtHandler(Sys_PreGotoPage, IntParam,pSysParam);

	//Debug("GotoPage Return 0x%x\n\r",SysMsg);
	
	if(gCurrSysMsg&SM_NoGoto)//Ò³ÃæµÄSys_Goto_Page´«µİ»ØµÄĞÅÏ¢
	{//ĞÂÒ³ÃæµÄSys_PreGotoPage´«µİ»ØSM_NoGotoĞÅÏ¢±íÊ¾²»ĞèÒª½øÈë´ËÒ³ÃæÁË¡£
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

//ÉèÖÃÏµÍ³ÊÂ¼ş¶ÔÓ¦Î»
void Q_SetPeripEvt(PAGE_RID RegID,u32 PeripEvtCon)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) //´Ë´¦×´Ì¬ÒÅÊ§¡
		{
			Q_ErrorStopScreen("Can't find page!\n\r");
		}
	}
	else //Èç¹ûRegIDÎª0£¬·µ»Øµ±Ç°Ò³ÃæµÄÊÂ¼ş±êÖ¾
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	gPagePeripEvtFlag[PageIdx]|=PeripEvtCon;
	OS_ExitCritical();
}

//Çå³şÏµÍ³ÊÂ¼ş¶ÔÓ¦Î»
void Q_ClrPeripEvt(PAGE_RID RegID,u32 PeripEvtCon)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return;//´Ë´¦×´Ì¬ÒÅÊ§¡£
	}
	else //Èç¹ûRegIDÎª0£¬·µ»Øµ±Ç°Ò³ÃæµÄÊÂ¼ş±êÖ¾
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	gPagePeripEvtFlag[PageIdx]&=PeripEvtCon;
	OS_ExitCritical();
}

//´ò¿ªÏµÍ³ÊÂ¼ş±êÖ¾
void Q_EnablePeripEvt(PAGE_RID RegID,PERIP_EVT PeripEvt)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return;//´Ë´¦×´Ì¬ÒÅÊ§¡£
	}
	else //Èç¹ûRegIDÎª0£¬·µ»Øµ±Ç°Ò³ÃæµÄÊÂ¼ş±êÖ¾
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	SetBit(gPagePeripEvtFlag[PageIdx],PeripEvt);
	OS_ExitCritical();
}

//¹Ø±ÕÏµÍ³ÊÂ¼ş±êÖ¾
void Q_DisablePeripEvt(PAGE_RID RegID,PERIP_EVT PeripEvt)
{
	u8 PageIdx;
	OS_DeclareCritical();

	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return;//´Ë´¦×´Ì¬ÒÅÊ§¡£
	}
	else //Èç¹ûRegIDÎª0£¬·µ»Øµ±Ç°Ò³ÃæµÄÊÂ¼ş±êÖ¾
	{
		PageIdx=GetPageIdxByTrack(0);
	}
	
	OS_EnterCritical();
	ClrBit(gPagePeripEvtFlag[PageIdx],PeripEvt);
	OS_ExitCritical();
}

//²é¿´ÏµÍ³ÊÂ¼ş
//Èç¹ûRegIDÎª0£¬·µ»Øµ±Ç°Ò³ÃæµÄÊÂ¼ş±êÖ¾
INSPECT_SYSEVT_RET Q_InspectPeripEvt(PAGE_RID RegID,PERIP_EVT PeripEvt)
{
	u8 PageIdx;
	
	if(RegID)
	{
		if(FindPage("",RegID,&PageIdx)!=SM_State_OK) return NoHasSysEvt;//´Ë´¦×´Ì¬ÒÅÊ§¡£
	}
	else //Èç¹ûRegIDÎª0£¬·µ»Øµ±Ç°Ò³ÃæµÄÊÂ¼ş±êÖ¾
	{
		PageIdx=GetPageIdxByTrack(0);
	}

	if(ReadBit(gPagePeripEvtFlag[PageIdx],PeripEvt)) return HasPagePeripEvt;
	if(ReadBit(gGobalPeripEvtBitFlag,PeripEvt)) return HasGobalSysEvt;
	return NoHasSysEvt;
}

//ÉèÖÃÈ«¾ÖÊÂ¼ş£¬ÈÎºÎÒ³ÃæÏÂ£¬¶¼»á´¥·¢ÊÂ¼şµÄ´¦Àíº¯ÊıSysEventHandler
//²»¶ÔSys_PreGotoPageÆğ×÷ÓÃ
void Q_EnableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler)
{
	u8 i;

	if(PeripEvtHandler==NULL) return;
	
	//ÕÒÊÇ·ñÓĞÖØ¸´¼ÇÂ¼
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
		if((gGobalPeripEvtRecord[i].PeripEvt==PeripEvt)&&(gGobalPeripEvtRecord[i].GobalPeripEvtHandler==PeripEvtHandler))
			return;
	}	
	
	//ÕÒ¿ÕÎ»×Ó
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

//×¢ÏúÈ«¾ÖÊÂ¼ş
void Q_DisableGobalPeripEvent(PERIP_EVT PeripEvt,PeripheralsHandlerFunc PeripEvtHandler)
{
	u8 i;

	//ÕÒµ½Æ¥Åä¼ÇÂ¼
	for(i=0;i<MAX_GOBAL_SYSEVT;i++)
	{
		if((gGobalPeripEvtRecord[i].PeripEvt==PeripEvt)&&(gGobalPeripEvtRecord[i].GobalPeripEvtHandler==PeripEvtHandler))
		{
			gGobalPeripEvtRecord[i].PeripEvt=(PERIP_EVT)0;//Çå³ı¼ÇÂ¼
		}
	}	

	for(i=0;i<MAX_GOBAL_SYSEVT;i++) //¼ì²éÊÇ·ñ»¹ÓĞÆäËûÍ¬ÀàÈ«¾ÖÊÂ¼ş
	{
		if(gGobalPeripEvtRecord[i].PeripEvt==PeripEvt) return;
	}
	ClrBit(gGobalPeripEvtBitFlag,PeripEvt);//Ã»ÓĞ¾ÍÇå±êÖ¾
}

//ÓÃÓÚ´íÎóÍ£Ö¹
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

//ÓÃ»§¿ÉÓÃµÄÏµÍ³º¯Êı½áÊø


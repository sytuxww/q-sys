#include "System.h"
#include "Theme.h"
#include "CtrlObjImg.h"

#define CO_Debug Debug

//***********************************±äÁ¿¶¨Òå*********************************************
static const PAGE_CONTROL_NUM *gpCtrlObjNum=NULL;//µ±Ç°Ò³Ãæ¿Ø¼þ¸öÊý¼ÇÂ¼

//ÓÃÓÚ¼ÇÂ¼Ò³ÃæÁÙÊ±²ÎÊýµÄÖ¸ÕëÊý×é¡£[0]Î´Ê¹ÓÃ£¬ËùÒÔÒª¼ÓÒ»¸öÔªËØ
static void *gPageDataPtrRecord[MAX_PAGE_LAYER_NUM+1];

//Í¼±ê°´¼üÌæ»»µÄÁÙÊ±ºó×º´æ´¢Êý×é
static REP_IMG_SUFX gRepImgSufx[MAX_IMG_KEY_NUM];

//ÎÄ×Ö°´¼üÌæ»»µÄÁÙÊ±×Ö·û´®Ö¸Õë´æ´¢Êý×é
static u8 *gRepKeyNameCon[MAX_CHAR_KEY_NUM];

static const IMG_BUTTON_OBJ *gpCurImgTchCon=NULL; //µ±Ç°°´¼üÇøÓò¼¯ºÏ
static const CHAR_BUTTON_OBJ *gpCurCharTchCon=NULL; //µ±Ç°°´¼üÇøÓò¼¯ºÏ

//¶¯Ì¬¿Ø¼þ´æ´¢ÊµÌåÖ¸Õë
static void *gCtrlObjPtrBuf[MAX_DYN_CTRL_OBJ_NUM];//ÓÃÀ´´æ´¢ÏÂÁÐ¿Ø¼þÖ¸ÕëµÄÊµÌå
//IMG_BUTTON_OBJ **gpDynImgTchCon=NULL; //¶¯Ì¬Í¼Æ¬°´¼ü¼¯ºÏ
//CHAR_BUTTON_OBJ **gpDynCharTchCon=NULL;//¶¯Ì¬ÎÄ×Ö°´¼ü¼¯ºÏ
//YES_NO_OBJ **gpYesNoCon=NULL;
//NUM_CTRL_OBJ **gpNumCtrlObjCon=NULL;
//STR_ENUM_OBJ **gpStrOptCon=NULL;
//STR_BOX_OBJ **gpStrInputCon=NULL;

//ÓÃÀ´·½±ã±à³ÌµÄºê£¬¶¨ÒåË÷ÒýÆðÊ¼Î»ÖÃ
#define CO_STATIC_TCH_NUM (gpCtrlObjNum->ImgBtnNum+gpCtrlObjNum->CharBtnNum)
#define CO_DYN_IMG_IDX_START 0
#define CO_DYN_CHAR_IDX_START (CO_DYN_IMG_IDX_START+gpCtrlObjNum->DynImgBtnNum)
#define CO_YES_NO_IDX_START (CO_DYN_CHAR_IDX_START+gpCtrlObjNum->DynCharBtnNum)
#define CO_NUM_IDX_START (CO_YES_NO_IDX_START+gpCtrlObjNum->YesNoNum)
#define CO_STR_IDX_START (CO_NUM_IDX_START+gpCtrlObjNum->NumCtrlObjNum)

//***********************************Íâ²¿ÒýÓÃ*********************************************
extern u8 GetCurLayerNum(void);
extern const PAGE_ATTRIBUTE *GetPageByIdx(u8 PageIdx);
extern TOUCH_REGION *GetTouchInfoByIdx(u8 Idx);

//***********************************º¯Êý¶¨Òå*********************************************
void ControlObjInit(void)//ÏµÍ³Æô¶¯Ê±µÄ³õÊ¼»¯
{	
	u16 i;
	
	MemSet(gPageDataPtrRecord,0,(MAX_PAGE_LAYER_NUM+1)*sizeof(void *));//Çå¿ÕÁÙÊ±Ò³Ãæ±äÁ¿±£´æÖ¸Õë

	for(i=0;i<Q_GetPageTotal();i++)
	{
		if((GetPageByIdx(i)->CtrlObjNum.ImgBtnNum>MAX_IMG_KEY_NUM)
			||(GetPageByIdx(i)->CtrlObjNum.CharBtnNum>MAX_CHAR_KEY_NUM))
		{//Ò³ÃæµÄ°´¼üÇøÓò³¬¹ýÖ§³ÖÊýÄ¿
			Debug("%s touch region num is too much!%d %d\n\r",GetPageByIdx(i)->Name,
			GetPageByIdx(i)->CtrlObjNum.ImgBtnNum,GetPageByIdx(i)->CtrlObjNum.CharBtnNum);
			Q_ErrorStopScreen("!!!New page's touch region num is over MAX_IMG_KEY_NUM or MAX_CHAR_KEY_NUM");
		}

		if((GetPageByIdx(i)->CtrlObjNum.DynImgBtnNum
			+GetPageByIdx(i)->CtrlObjNum.DynCharBtnNum
			+GetPageByIdx(i)->CtrlObjNum.YesNoNum
			+GetPageByIdx(i)->CtrlObjNum.NumCtrlObjNum
			+GetPageByIdx(i)->CtrlObjNum.StrCtrlObjNum
			)>MAX_DYN_CTRL_OBJ_NUM)
		{//Ò³ÃæµÄ¿Ø¼þ¸öÊý³¬¹ýÖ§³ÖÊýÄ¿
			Debug("%s control object num is too much!\n\r",GetPageByIdx(i)->Name);
			Q_ErrorStopScreen("!!!Control Object num is over MAX_DYN_CTRL_OBJ_NUM!!!");
		}
	}
}

//½øÈëÐÂÒ³ÃæÊ±¸´ÖÆËùÓÐ¿Ø¼þµÄÇøÓòµ½Í³Ò»µÄ´æ´¢Çø
//·µ»ØÇøÓò¸öÊý
//¿½±´Ë³ÐòÈçÏÂ:
//Í¼Æ¬°´¼ü
//ÎÄ×Ö°´¼ü
//¶¯Ì¬Í¼Æ¬°´¼ü
//¶¯Ì¬ÎÄ×Ö°´¼ü
//YesNo
//NumBox
//StrOpt
//StrInput
static void CopyCtrlObjTouchReg(TOUCH_REGION *TouchRegsBuf)
{
	u8 Idx;
	u8 Cnt=0;

	IMG_BUTTON_OBJ **pDynImgTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_IMG_IDX_START]; //¶¯Ì¬Í¼Æ¬°´¼ü¼¯ºÏ
	CHAR_BUTTON_OBJ **pDynCharTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_CHAR_IDX_START];//¶¯Ì¬ÎÄ×Ö°´¼ü¼¯ºÏ
	YES_NO_OBJ **pYesNoCon=(void *)&gCtrlObjPtrBuf[CO_YES_NO_IDX_START];//Yes No¿Ø¼þ
	NUM_CTRL_OBJ **pNumCtrlObjCon=(void *)&gCtrlObjPtrBuf[CO_NUM_IDX_START];//Num ¿Ø¼þ
	STR_CTRL_OBJ **pStrCtrlObjCon=(void *)&gCtrlObjPtrBuf[CO_STR_IDX_START];//Strings option¿Ø¼þ

	//Í¼Æ¬°´¼üÇøÓò
	for(Idx=0;Idx<gpCtrlObjNum->ImgBtnNum;Idx++,Cnt++)
	{
		TouchRegsBuf[Cnt].x=gpCurImgTchCon[Idx].x;
		TouchRegsBuf[Cnt].y=gpCurImgTchCon[Idx].y;
		TouchRegsBuf[Cnt].w=gpCurImgTchCon[Idx].w;
		TouchRegsBuf[Cnt].h=gpCurImgTchCon[Idx].h;
		TouchRegsBuf[Cnt].ObjID=gpCurImgTchCon[Idx].ObjID;
		TouchRegsBuf[Cnt].Type=COT_ImgBtn;
		TouchRegsBuf[Cnt].Index=Idx;
		TouchRegsBuf[Cnt].OptionsMask=(u8)(gpCurImgTchCon[Idx].OptionsMask&0xff);
	}
				
	//ÎÄ×Ö°´¼üÇøÓò
	for(Idx=0;Idx<gpCtrlObjNum->CharBtnNum;Idx++,Cnt++)
	{
		TouchRegsBuf[Cnt].x=gpCurCharTchCon[Idx].x;
		TouchRegsBuf[Cnt].y=gpCurCharTchCon[Idx].y;
		TouchRegsBuf[Cnt].w=gpCurCharTchCon[Idx].w;
		TouchRegsBuf[Cnt].h=gpCurCharTchCon[Idx].h;
		TouchRegsBuf[Cnt].ObjID=gpCurCharTchCon[Idx].ObjID;
		TouchRegsBuf[Cnt].Type=COT_CharBtn;
		TouchRegsBuf[Cnt].Index=Idx;
		TouchRegsBuf[Cnt].OptionsMask=(u8)(gpCurCharTchCon[Idx].OptionsMask&0xff);
	}

	//¶¯Ì¬Í¼Æ¬°´¼üÇøÓò
	for(Idx=0;Idx<gpCtrlObjNum->DynImgBtnNum;Idx++,Cnt++)
	{
		if(pDynImgTchCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pDynImgTchCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pDynImgTchCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pDynImgTchCon[Idx]->w;
			TouchRegsBuf[Cnt].h=pDynImgTchCon[Idx]->h;
			TouchRegsBuf[Cnt].ObjID=pDynImgTchCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_DynImgBtn;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(pDynImgTchCon[Idx]->OptionsMask&0xff);
		}
	}

	//¶¯Ì¬ÎÄ×Ö°´¼üÇøÓò
	for(Idx=0;Idx<gpCtrlObjNum->DynCharBtnNum;Idx++,Cnt++)
	{
		if(pDynCharTchCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pDynCharTchCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pDynCharTchCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pDynCharTchCon[Idx]->w;
			TouchRegsBuf[Cnt].h=pDynCharTchCon[Idx]->h;
			TouchRegsBuf[Cnt].ObjID=pDynCharTchCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_DynCharBtn;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(pDynCharTchCon[Idx]->OptionsMask&0xff);
		}
	}

	//yes no
	for(Idx=0;Idx<gpCtrlObjNum->YesNoNum;Idx++,Cnt++)
	{
		if(pYesNoCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pYesNoCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pYesNoCon[Idx]->y;
			TouchRegsBuf[Cnt].w=CO_YES_NO_W;
			TouchRegsBuf[Cnt].h=CO_YES_NO_H;
			TouchRegsBuf[Cnt].ObjID=pYesNoCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_YesNo;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		}
	}

	//num ctrl obj
	for(Idx=0;Idx<gpCtrlObjNum->NumCtrlObjNum;Idx++,Cnt++)
	{
		if(pNumCtrlObjCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pNumCtrlObjCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pNumCtrlObjCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pNumCtrlObjCon[Idx]->w;
			TouchRegsBuf[Cnt].h=CO_NUM_H;
			TouchRegsBuf[Cnt].ObjID=pNumCtrlObjCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_Num;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		}
	}

	//str ctrl obj
	for(Idx=0;Idx<gpCtrlObjNum->StrCtrlObjNum;Idx++,Cnt++)
	{
		if(pStrCtrlObjCon[Idx]!=NULL)
		{
			TouchRegsBuf[Cnt].x=pStrCtrlObjCon[Idx]->x;
			TouchRegsBuf[Cnt].y=pStrCtrlObjCon[Idx]->y;
			TouchRegsBuf[Cnt].w=pStrCtrlObjCon[Idx]->w;
			TouchRegsBuf[Cnt].h=CO_STR_ENUM_H;
			TouchRegsBuf[Cnt].ObjID=pStrCtrlObjCon[Idx]->ObjID;
			TouchRegsBuf[Cnt].Type=COT_Str;
			TouchRegsBuf[Cnt].Index=Idx;
			TouchRegsBuf[Cnt].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		}
	}
	
}

//ÇÐ»»Ò³ÃæÊ±Ö´ÐÐµÄ¿Ø¼þ²¿·ÖÊý¾Ý´¦Àí
void PageSwithcCtrlObjDataHandler(const PAGE_ATTRIBUTE *pNewPage)
{
	gpCtrlObjNum=&gpCurrentPage->CtrlObjNum;//µÃµ½¿Ø¼þ¸öÊý½á¹¹Ìå

	gpCurImgTchCon=gpCurrentPage->pImgButtonCon;//ÇÐ»»»ù±¾´¥ÃþÇøÓò¼¯Ö¸Õë
	gpCurCharTchCon=gpCurrentPage->pCharButtonCon;
	
	if(gTouchRegionNum) Q_Free(gpTouchRegions);//ÊÕ»ØÉÏ¸öÒ³ÃæµÄÄÚ´æ
	
	gTouchRegionNum=CO_STATIC_TCH_NUM
									+gpCtrlObjNum->DynImgBtnNum
									+gpCtrlObjNum->DynCharBtnNum
									+gpCtrlObjNum->YesNoNum//yes or no Ñ¡ÏîµÄ¸öÊý
									+gpCtrlObjNum->NumCtrlObjNum//Êý×Ö¿òµÄ¸öÊý
									+gpCtrlObjNum->StrCtrlObjNum//×Ö·û´®¿òµÄ¸öÊý
	;
	if(gTouchRegionNum)//ÉêÇëÐÂÄÚ´æÓÃÓÚ´æ·Å×ø±êÐÅÏ¢
	{
		gpTouchRegions=Q_Mallco(gTouchRegionNum*sizeof(TOUCH_REGION));//¸´ÖÆ´¥ÅöÇøÓò
		CopyCtrlObjTouchReg(gpTouchRegions);//½«ËùÓÐ¿Ø¼þµÄ×ø±êÐÅÏ¢¸´ÖÆµ½´¥ÅöÇøÓò¼ÇÂ¼½á¹¹Ìå£¬¹©´¥ÃþÏß³ÌÊ¹ÓÃ
	}
}

//Í¨¹ý¼üÖµÀ´Ñ°ÕÒË÷Òý£¬·µ»Ø0xff±íÊ¾Î´ÕÒµ½
//Ö»Ö§³ÖÕÒµ½µÄµÚÒ»¸ö!!¶ÔÓÚÒ»¸ö¼üÖµ¶à¸ö°´¼üµÄÇé¿ö²»Ö§³ÖºóÃæµÄ°´¼ü
static u8 FindImgTchIdx(u8 Key)
{
	u8 i=0;

	for(;i<gpCtrlObjNum->ImgBtnNum;i++)
		if(gpCurImgTchCon[i].ObjID==Key) return i;

	return 0xff;
}

static u8 FindCharTchIdx(u8 Key)
{
	u8 i=0;

	for(;i<gpCtrlObjNum->CharBtnNum;i++)
		if(gpCurCharTchCon[i].ObjID==Key) return i;

	return 0xff;
}

//ÓÃÓÚÏÔÊ¾Í¼Æ¬´¥ÃþÇøÓòµÄÍ¼Æ¬
static bool ImgTchDisplay(u8 Index,INPUT_EVT_TYPE InType,bool IsDyn)
{
	IMG_BUTTON_OBJ *pTouchRegion;
	GUI_REGION BmpRegion;
	GUI_REGION BgRegion;
	u8 PathBuf[MAX_BMP_PATH_LEN]="";
	u8 ImgSuffix=0,RepSuffix=0;
	bool LandScapeMode=Gui_GetLandScapeMode();//´æ´¢µ±Ç°ºáÆÁÄ£Ê½
	bool Ret=TRUE;
	COLOR_TYPE RegionColor;

	if(IsDyn)//ÊÇ·ñÊÇÒ³Ãæ¶¯Ì¬×éµÄÍ¼±ê
	{
		pTouchRegion=gCtrlObjPtrBuf[CO_DYN_IMG_IDX_START+Index];

		switch(InType)//¼ì²éÊÂ¼þÀàÐÍ
		{
			case Input_TchNormal:
				RegionColor=FatColor(NORMAL_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_NOR_DEF_SUFFIX;
				break;
			case Input_TchPress:
				RegionColor=FatColor(PRESS_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_PRS_DEF_SUFFIX;
				break;
			case Input_TchRelease:
			case Input_TchReleaseVain:
				RegionColor=FatColor(RELEASE_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_REL_DEF_SUFFIX;
				break;
			default:
				return FALSE;
		}
	}
	else //Ò³Ãæ¹Ì¶¨°´¼ü
	{
		pTouchRegion=(void *)&gpCurImgTchCon[Index];
		if(gRepImgSufx[Index])	 RepSuffix=gRepImgSufx[Index];
		
		switch(InType)//¼ì²éÊÂ¼þÀàÐÍ
		{
			case Input_TchNormal:
				RegionColor=FatColor(NORMAL_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_NOR_DEF_SUFFIX;
				break;
			case Input_TchPress:
				RegionColor=FatColor(PRESS_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_PRS_DEF_SUFFIX;
				break;
			case Input_TchRelease:
			case Input_TchReleaseVain:
				RegionColor=FatColor(RELEASE_REPLACE_COLOR);
				ImgSuffix=IMG_KEY_REL_DEF_SUFFIX;
				break;
			default:
				return FALSE;
		}
	}
	
	if(pTouchRegion->ImgPathPrefix[0])//Ö¸¶¨ÁËÂ·¾¶Ç°×º£¬ËµÃ÷ÐèÒªÏÔÊ¾°´¼üÍ¼±ê
	{
		if(pTouchRegion->OptionsMask&PathMsk)//Ê¹ÓÃ×Ô¶¨ÒåÂ·¾¶£¬Ö»Ðèµþ¼ÓÖ÷ÌâÂ·¾¶
		{
			if(strlen((void *)Q_GetNowThemePath())+strlen((void *)pTouchRegion->ImgPathPrefix)>=(MAX_BMP_PATH_LEN-7))
			{
				Debug("!!!Error:Path is too long!!!");
				return FALSE;//Â·¾¶Ì«³¤
			}
			if(RepSuffix) sprintf((void *)PathBuf,"%s%s%c%c",Q_GetNowThemePath(),pTouchRegion->ImgPathPrefix,RepSuffix,ImgSuffix);
			else sprintf((void *)PathBuf,"%s%s%c",Q_GetNowThemePath(),pTouchRegion->ImgPathPrefix,ImgSuffix);
		}
		else //²»Ê¹ÓÃ×Ô¶¨ÒåÂ·¾¶£¬Ôòµþ¼ÓÖ÷ÌâÂ·¾¶¼°Ò³ÃæÃûÎÄ¼þ¼Ð
		{
			if((strlen((void *)Q_GetNowThemePath())+strlen((void *)gpCurrentPage->Name)
				+strlen((void *)pTouchRegion->ImgPathPrefix))>=(MAX_BMP_PATH_LEN-7))
			{
				Debug("!!!Error:Path is too long!!!");
				return FALSE;//Â·¾¶Ì«³¤
			}
			if(RepSuffix) sprintf((void *)PathBuf,"%s%s/Btn/%s%c%c",Q_GetNowThemePath(),gpCurrentPage->Name,pTouchRegion->ImgPathPrefix,RepSuffix,ImgSuffix);//µÃµ½Â·¾¶
			else sprintf((void *)PathBuf,"%s%s/Btn/%s%c",Q_GetNowThemePath(),gpCurrentPage->Name,pTouchRegion->ImgPathPrefix,ImgSuffix);//µÃµ½Â·¾¶
		}
		
		if(pTouchRegion->OptionsMask&BinMsk)//ÊÇ·ñ²ÉÓÃbinÎÄ¼þ×öÍ¼±êÎÄ¼þ
			strcat((void *)PathBuf,".bin");
		else
			strcat((void *)PathBuf,".bmp");
		
		//¸ù¾ÝÏÔÊ¾Ä£Ê½¼ÆËã×ø±ê
		if(pTouchRegion->OptionsMask&LandMsk)
		{//ºáÆÁÄ£Ê½
			Gui_SetLandScapeMode(TRUE);
			BmpRegion.x=pTouchRegion->x-pTouchRegion->ImgX;
			BmpRegion.y=pTouchRegion->y+pTouchRegion->ImgY;
			BmpRegion.w=pTouchRegion->w-(pTouchRegion->ImgY<<1);
			BmpRegion.h=pTouchRegion->h-(pTouchRegion->ImgX<<1);
		}
		else		
		{//Õý³£Ä£Ê½
			Gui_SetLandScapeMode(FALSE);
			BmpRegion.x=pTouchRegion->x+pTouchRegion->ImgX;
			BmpRegion.y=pTouchRegion->y+pTouchRegion->ImgY;
			BmpRegion.w=pTouchRegion->w-(pTouchRegion->ImgX<<1);
			BmpRegion.h=pTouchRegion->h-(pTouchRegion->ImgY<<1);
		}		
		
		BmpRegion.Color=pTouchRegion->TransColor;
		if((pTouchRegion->OptionsMask&DbgMsk)||
			((pTouchRegion->OptionsMask&BinMsk)?
			(Gui_DrawImgBin(PathBuf,&BmpRegion)==Gui_No_Such_File):
			(Gui_Draw24Bmp(PathBuf,&BmpRegion)==Gui_No_Such_File)))
		{//µ÷ÊÔÄ£Ê½»òÕß´ò¿ªÍ¼±êÎÄ¼þÊ§°Ü£¬ÔòÏÔÊ¾´¥ÃþÇøÓò
			BgRegion.x=pTouchRegion->x;
			BgRegion.y=pTouchRegion->y;
			BgRegion.w=pTouchRegion->w;
			BgRegion.h=pTouchRegion->h;
			BgRegion.Color=FatColor(REGION_BG_COLOR);
			Gui_FillBlock(&BgRegion);	
			BmpRegion.Color=RegionColor;
			Gui_FillBlock(&BmpRegion);

			if(pTouchRegion->Name&&pTouchRegion->Name[0])
			{
				BmpRegion.Color=FatColor(KEY_NAME_COLOR);
				BmpRegion.Space=0;
				if(pTouchRegion->OptionsMask&F16Msk)
					Gui_DrawFont(GBK16_FONT,(void *)pTouchRegion->Name,&BmpRegion);
				else if(pTouchRegion->OptionsMask&B14Msk)
					Gui_DrawFont(ASC14B_FONT,(void *)pTouchRegion->Name,&BmpRegion);
				else
					Gui_DrawFont(GBK12_FONT,(void *)pTouchRegion->Name,&BmpRegion);
			}

			Ret=FALSE;
		}		
		
		Gui_SetLandScapeMode(LandScapeMode);
	}
	else if(pTouchRegion->OptionsMask&DbgMsk)//Ã»ÓÐÖ¸¶¨ºó×º£¬³ý·Ç¿ªÆôÁËDbgMsk,·ñÔò²»ÏÔÊ¾ÈÎºÎÄÚÈÝ
	{
		//¸ù¾ÝÏÔÊ¾Ä£Ê½¼ÆËã×ø±ê
		if(pTouchRegion->OptionsMask&LandMsk)
		{//ºáÆÁÄ£Ê½
			Gui_SetLandScapeMode(TRUE);
		}
		else		
		{//Õý³£Ä£Ê½
			Gui_SetLandScapeMode(FALSE);
		}		
		
		//ÏÔÊ¾´¥ÃþÇøÓò
		BgRegion.x=pTouchRegion->x;
		BgRegion.y=pTouchRegion->y;
		BgRegion.w=pTouchRegion->w;
		BgRegion.h=pTouchRegion->h;
		BgRegion.Color=FatColor(REGION_BG_COLOR);
		Gui_FillBlock(&BgRegion);	

		if(pTouchRegion->Name&&pTouchRegion->Name[0])//ÏÔÊ¾×Ö·û
		{
			BgRegion.x++;
			BgRegion.y++;
			BgRegion.w-=2;
			BgRegion.h-=2;
			BgRegion.Color=FatColor(KEY_NAME_COLOR);
			BgRegion.Space=0;
			if(pTouchRegion->OptionsMask&F16Msk)
				Gui_DrawFont(GBK16_FONT,(void *)pTouchRegion->Name,&BgRegion);
			else
				Gui_DrawFont(GBK12_FONT,(void *)pTouchRegion->Name,&BgRegion);
		}
	
		Gui_SetLandScapeMode(LandScapeMode);
	}

	return Ret;
}

//ÓÃÓÚÏÔÊ¾ÎÄ×Ö´¥ÃþÇøÓòµÄÎÄ×Ö
static bool CharTchDisplay(u8 Index,INPUT_EVT_TYPE InType,bool IsDyn)
{
	const CHAR_BUTTON_OBJ *pTouchRegion;
	GUI_REGION CharRegion;
	GUI_REGION BgRegion;
	bool LandScapeMode=Gui_GetLandScapeMode();//´æ´¢µ±Ç°ºáÆÁÄ£Ê½
	bool Ret=TRUE;
	u8 *pName;
	COLOR_TYPE CharColor,BgColor;

	if(IsDyn)//¼ì²éÊÇ²»ÊÇ¶¯Ì¬ÎÄ×Ö°´¼ü×é
	{
		pTouchRegion=gCtrlObjPtrBuf[CO_DYN_CHAR_IDX_START+Index];
		pName=pTouchRegion->Name;
	}
	else//ÊÇÒ³ÃæµÄ¹Ì¶¨ÎÄ×Ö°´¼ü×é
	{
		pTouchRegion=&gpCurCharTchCon[Index];

		if(gRepKeyNameCon[Index]!=NULL)	pName=gRepKeyNameCon[Index];
		else	pName=pTouchRegion->Name;
	}

	switch(InType)//¼ì²éÊÂ¼þÀàÐÍ
	{
		case Input_TchNormal:
			CharColor=pTouchRegion->NormalColor;
			BgColor=pTouchRegion->NormalBG;		
			break;
		case Input_TchPress:
			CharColor=pTouchRegion->PressColor;
			BgColor=pTouchRegion->PressBG;	
			break;
		case Input_TchRelease:
		case Input_TchReleaseVain:
			CharColor=pTouchRegion->ReleaseColor;
			BgColor=pTouchRegion->ReleaseBG;	
			break;
		default:
			return FALSE;
	}
			
	if(pName!=NULL&&pName[0])//ÓÐÎÄ×ÖÏÔÊ¾
	{		
		//4	ÏÈ¸ù¾ÝÏÔÊ¾Ä£Ê½¼ÆËã×ø±ê
		if(pTouchRegion->OptionsMask&LandMsk)
		{//ºáÆÁÄ£Ê½
			Gui_SetLandScapeMode(TRUE);
			//ÎÄ×Ö
			if(CharColor!=FatColor(TRANS_COR))//Èç¹û²»ÊÇÍ¸Ã÷É«
			{
				CharRegion.x=pTouchRegion->x-pTouchRegion->CharX;
				CharRegion.y=pTouchRegion->y+pTouchRegion->CharY;
				CharRegion.w=pTouchRegion->w-pTouchRegion->CharY;
				CharRegion.h=pTouchRegion->h-pTouchRegion->CharX;
			}

			if(pTouchRegion->OptionsMask&DbgMsk)//µ÷ÊÔÄ£Ê½£¬½«´¥ÃþÇøÓòÏÔÊ¾³öÀ´
			{
				BgRegion.x=pTouchRegion->x;
				BgRegion.y=pTouchRegion->y;
				BgRegion.w=pTouchRegion->w;
				BgRegion.h=pTouchRegion->h;
				BgRegion.Color=FatColor(REGION_BG_COLOR);
				Gui_FillBlock(&BgRegion);
			}

			//±³¾°
			if(BgColor!=FatColor(TRANS_COR))//Èç¹û²»ÊÇÍ¸Ã÷É«
			{
				if(pTouchRegion->OptionsMask&RoueMsk) //Ô²±ß
				{
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin>>4);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin&0x0f)-BgRegion.h;					
					BgRegion.x=pTouchRegion->x-(pTouchRegion->Margin&0x0f);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin>>4)+(BgRegion.h>>1);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);

					BgRegion.x-=(BgRegion.h>>1);
					BgRegion.w=(BgRegion.h>>1);
					BgRegion.Space=0xc3;
					Gui_DrawCircle(&BgRegion,TRUE);

					BgRegion.y=pTouchRegion->y-(BgRegion.h>>1)+pTouchRegion->w-(pTouchRegion->Margin>>4)-1;
					BgRegion.Space=0x3c;
					Gui_DrawCircle(&BgRegion,TRUE);
				}
				else
				{
					BgRegion.x=pTouchRegion->x-(pTouchRegion->Margin&0x0f);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin>>4);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin&0x0f);
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin>>4);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);//4	ÏÔÊ¾±³¾°
				}
			}
		}
		else		
		{//Õý³£Ä£Ê½
			Gui_SetLandScapeMode(FALSE);
			//ÎÄ×Ö
			if(CharColor!=FatColor(TRANS_COR))//Èç¹û²»ÊÇÍ¸Ã÷É«
			{
				CharRegion.x=pTouchRegion->x+pTouchRegion->CharX;
				CharRegion.y=pTouchRegion->y+pTouchRegion->CharY;
				CharRegion.w=pTouchRegion->w-pTouchRegion->CharX;
				CharRegion.h=pTouchRegion->h-pTouchRegion->CharY;
			}

			if(pTouchRegion->OptionsMask&DbgMsk)//µ÷ÊÔÄ£Ê½£¬½«´¥ÃþÇøÓòÏÔÊ¾³öÀ´
			{
				BgRegion.x=pTouchRegion->x;
				BgRegion.y=pTouchRegion->y;
				BgRegion.w=pTouchRegion->w;
				BgRegion.h=pTouchRegion->h;
				BgRegion.Color=FatColor(REGION_BG_COLOR);
				Gui_FillBlock(&BgRegion);
			}
			
			//±³¾°
			if(BgColor!=FatColor(TRANS_COR))//Èç¹û²»ÊÇÍ¸Ã÷É«
			{
				if(pTouchRegion->OptionsMask&RoueMsk) //Ô²±ß
				{
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin&0x0f);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin>>4)-BgRegion.h;
					BgRegion.x=pTouchRegion->x+(pTouchRegion->Margin>>4)+(BgRegion.h>>1);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin&0x0f);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);

					BgRegion.y+=(BgRegion.h>>1);
					BgRegion.w=(BgRegion.h>>1);
					BgRegion.Space=0xf0;
					Gui_DrawCircle(&BgRegion,TRUE);

					BgRegion.x=pTouchRegion->x-(BgRegion.h>>1)+pTouchRegion->w-(pTouchRegion->Margin>>4)-1;
					BgRegion.Space=0x0f;
					Gui_DrawCircle(&BgRegion,TRUE);
				}
				else
				{
					BgRegion.x=pTouchRegion->x+(pTouchRegion->Margin>>4);
					BgRegion.y=pTouchRegion->y+(pTouchRegion->Margin&0x0f);
					BgRegion.w=pTouchRegion->w-2*(pTouchRegion->Margin>>4);
					BgRegion.h=pTouchRegion->h-2*(pTouchRegion->Margin&0x0f);
					BgRegion.Color=BgColor;
					Gui_FillBlock(&BgRegion);//4	ÏÔÊ¾±³¾°
				}
			}
		}		

		//4	ÏÔÊ¾ÎÄ×Ö
		if(CharColor!=FatColor(TRANS_COR))//Èç¹û²»ÊÇÍ¸Ã÷É«
		{
			CharRegion.Space=pTouchRegion->Space;
			CharRegion.Color=CharColor;
			if(pTouchRegion->OptionsMask&F16Msk)
				Gui_DrawFont(GBK16_FONT,(void *)pName,&CharRegion);
			else if(pTouchRegion->OptionsMask&B14Msk)
				Gui_DrawFont(ASC14B_FONT,(void *)pName,&CharRegion);
			else
				Gui_DrawFont(GBK12_FONT,(void *)pName,&CharRegion);
		}

		Gui_SetLandScapeMode(LandScapeMode);//»¹Ô­ÆÁÄ»Ä£Ê½
	}
	else if(pTouchRegion->OptionsMask&DbgMsk)//Ã»ÓÐÖ¸¶¨ÎÄ×Ö£¬³ý·Ç¿ªÆôÁËDbgMsk,·ñÔò²»ÏÔÊ¾ÈÎºÎÄÚÈÝ
	{		
		//4	ÏÈ¸ù¾ÝÏÔÊ¾Ä£Ê½¼ÆËã×ø±ê
		if(pTouchRegion->OptionsMask&LandMsk)
		{//ºáÆÁÄ£Ê½
			Gui_SetLandScapeMode(TRUE);
		}
		else		
		{//Õý³£Ä£Ê½
			Gui_SetLandScapeMode(FALSE);
		}		

		BgRegion.x=pTouchRegion->x;
		BgRegion.y=pTouchRegion->y;
		BgRegion.w=pTouchRegion->w;
		BgRegion.h=pTouchRegion->h;
		BgRegion.Color=FatColor(REGION_BG_COLOR);
		Gui_FillBlock(&BgRegion);
		
		Gui_SetLandScapeMode(LandScapeMode);//»¹Ô­ÆÁÄ»Ä£Ê½
	}

	return Ret;
}

//½«ÏàÍ¬¼üÖµµÄ°´¼ü¶¼ÏÔÊ¾
void SameOidTchDisplay(u8 ObjID,INPUT_EVT_TYPE InType)
{
	u8 i;
	IMG_BUTTON_OBJ **pDynImgTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_IMG_IDX_START]; //¶¯Ì¬Í¼Æ¬°´¼ü¼¯ºÏ
	CHAR_BUTTON_OBJ **pDynCharTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_CHAR_IDX_START];//¶¯Ì¬ÎÄ×Ö°´¼ü¼¯ºÏ

	//ÏàÍ¬¼üÖµµÄÎÄ×Ö°´¼ü
	for(i=0;i<gpCtrlObjNum->CharBtnNum;i++)//ÕÒÏàÍ¬¼üÖµµÄ¼ü
	{
		if(gpCurCharTchCon[i].ObjID==ObjID)
			CharTchDisplay(i,InType,FALSE);
	}

	//ÏàÍ¬¼üÖµµÄ¶¯Ì¬ÎÄ×Ö°´¼ü
	for(i=0;i<gpCtrlObjNum->DynCharBtnNum;i++)
	{
		if(pDynCharTchCon[i]!=NULL)
			if(pDynCharTchCon[i]->ObjID==ObjID)
				CharTchDisplay(i,InType,TRUE);
	}
	
	//ÏàÍ¬¼üÖµµÄÍ¼Ïñ°´¼ü
	for(i=0;i<gpCtrlObjNum->ImgBtnNum;i++)//ÕÒÏàÍ¬¼üÖµµÄ¼ü
	{
		if(gpCurImgTchCon[i].ObjID==ObjID)
			ImgTchDisplay(i,InType,FALSE);
	}

	//ÏàÍ¬¼üÖµµÄ¶¯Ì¬Í¼Ïñ°´¼ü
	for(i=0;i<gpCtrlObjNum->DynImgBtnNum;i++) 
	{
		if(pDynImgTchCon[i]!=NULL)
			if(pDynImgTchCon[i]->ObjID==ObjID)
				ImgTchDisplay(i,InType,TRUE);
	}
}

//ÓÐ´¥ÃþÊäÈëÊ±£¬´¦Àí´¥ÃþÇøÓòÊÂ¼þ
CO_MSG ButtonCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	CO_MSG CoMsg=CO_State_OK;
	u8 TchEvtMsk=GetTouchInfoByIdx(Idx)->OptionsMask;//ÊÂ¼þÑÚÂë
	u8 ObjID=GetTouchInfoByIdx(Idx)->ObjID;//¼üÖµ

	switch(InType)//ÅÐ¶ÏÊÂ¼þ
	{
		case Input_TchPress:		
			//°´ÏÂÊ±Í¼±ê±ä»¯
			SameOidTchDisplay(ObjID,InType);
			if(TchEvtMsk&PrsMsk)//ÐèÒª´¦Àí
				CoMsg=gpCurrentPage->ButtonHandler(ObjID,Tch_Press,pTouchInfo);
			break;
		case Input_TchContinue:
			//²»ÐèÒª¼ì²éÊÇ·ñ´¥·¢£¬ÒòÎªÔÚTouchÏß³ÌÀïÃæÒÑ¾­¼ì²éÁË
			CoMsg=gpCurrentPage->ButtonHandler(ObjID,Tch_Continue,pTouchInfo);
			break;
		case Input_TchRelease:
			//°´¼üÊÍ·Å¸Ä±ä°´Å¥Í¼±ê²¢·¢³öÊÂ¼þ
			SameOidTchDisplay(ObjID,InType);
			if(TchEvtMsk&RelMsk)//ÐèÒª´¦Àí
				CoMsg=gpCurrentPage->ButtonHandler(ObjID,Tch_Release,pTouchInfo);
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:	
			SameOidTchDisplay(ObjID,InType);
			if(TchEvtMsk&ReVMsk)//ÐèÒª´¦Àí
				CoMsg=gpCurrentPage->ButtonHandler(ObjID,Tch_ReleaseVain,pTouchInfo);
			Allow_Touch_Input();
			break;
	}

	return CoMsg;
}

//ÓÐ´¥ÃþÊäÈëÊ±£¬´¦ÀíyesnoÑ¡¿òÊÂ¼þ
//IdxÎªËùÓÐ´¥ÃþÇøÓò¼¯ºÏµÄË÷Òý£¬°üÀ¨¾²Ì¬¿Ø¼þ²¿·Ö
CO_MSG YesNoCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx)
{
	YES_NO_OBJ *pYesNo=(void *)gCtrlObjPtrBuf[Idx-CO_STATIC_TCH_NUM];
	GUI_REGION DrawRegion;
	CO_MSG CoMsg=CO_State_OK;
	u8 ObjID=GetTouchInfoByIdx(Idx)->ObjID;//¼üÖµ

	DrawRegion.x=pYesNo->x;
	DrawRegion.y=pYesNo->y;
	DrawRegion.w=CO_YES_NO_W;
	DrawRegion.h=CO_YES_NO_H;
	DrawRegion.Color=CO_YES_NO_TRAN_COLOR;
			
	switch(InType)//ÅÐ¶ÏÊÂ¼þ
	{
		case Input_TchPress:		//°´ÏÂÊ±Í¼±ê±ä»¯
			Gui_DrawImgArray(gCtrlObj_OnOff,&DrawRegion);
			break;
		case Input_TchRelease://°´¼üÊÍ·Å¸Ä±ä°´Å¥Í¼±ê²¢·¢³öÊÂ¼þ
			pYesNo->DefVal=(bool)!pYesNo->DefVal;
			if(pYesNo->DefVal==FALSE)	Gui_DrawImgArray(gCtrlObj_Off,&DrawRegion);
			else 	Gui_DrawImgArray(gCtrlObj_On,&DrawRegion);
			
			CoMsg=gpCurrentPage->YesNoHandler(ObjID,pYesNo->DefVal);
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:	
			if(pYesNo->DefVal==FALSE)	Gui_DrawImgArray(gCtrlObj_Off,&DrawRegion);
			else 	Gui_DrawImgArray(gCtrlObj_On,&DrawRegion);
			
			Allow_Touch_Input();
			break;
	}
	
	return CoMsg;
}

//ÓÐ´¥ÃþÊäÈëÊ±£¬´¦ÀíNumBoxÊÂ¼þ
//IdxÎªËùÓÐ´¥ÃþÇøÓò¼¯ºÏµÄË÷Òý
CO_MSG NumCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	NUM_CTRL_OBJ *pNumCtrlObj=(void *)gCtrlObjPtrBuf[Idx-CO_STATIC_TCH_NUM];

	switch(InType)
	{
		case Input_TchPress:
			break;
		case Input_TchRelease:
			Q_GotoPage(GotoSubPage,"NumCtrlObjPage",PRID_NumCtrlObjPage,pNumCtrlObj);//½»¸øNumCtrlObjPage´¦Àí
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:
			Allow_Touch_Input();
			break;
	}

	
	return 0;
}

//ÓÐ´¥ÃþÊäÈëÊ±£¬´¦ÀíStrÊÂ¼þ
CO_MSG StrCtrlObjTchHandler(INPUT_EVT_TYPE InType,u8 Idx,TOUCH_INFO *pTouchInfo)
{
	STR_CTRL_OBJ *pStrCtrlObj=(void *)gCtrlObjPtrBuf[Idx-CO_STATIC_TCH_NUM];

	switch(InType)
	{
		case Input_TchPress:
			break;
		case Input_TchRelease:
			Q_GotoPage(GotoSubPage,"StrCtrlObjPage",PRID_StrCtrlObjPage,pStrCtrlObj);//½»¸øStrCtrlObjPage´¦Àí
			Allow_Touch_Input();
			break;
		case Input_TchReleaseVain:
			Allow_Touch_Input();
			break;
	}
	
	return 0;
}

//½øÈë×ÓÒ³ÃæÇ°£¬±£´æÒ³ÃæµÄÁãÊ±Êý¾Ý
//Òª±£´æµÄÊý¾ÝÓÐ:
// 1.Ò³ÃæµÄ¶¯Ì¬°´¼ü×é
// 2.
//ps Õâ¿é¿´²»¶®²»ÒªÃãÇ¿
void PushPageCtrlObjData(void)
{
	void *p;

	Debug("PushPageCtrlObjData:%d\n\r",GetCurLayerNum());

	//Debug("%d\n\r",sizeof(IMG_BUTTON_OBJ*)*MAX_DYNAMIC_IMG_KEY_NUM);
	//Debug("%d\n\r",sizeof(CHAR_BUTTON_OBJ*)*MAX_DYNAMIC_CHAR_KEY_NUM);
	//Debug("%d\n\r",sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
	//Debug("%d\n\r",sizeof(u8 *)*MAX_CHAR_KEY_NUM);
	
	p=gPageDataPtrRecord[GetCurLayerNum()]=Q_Mallco(sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM
								+sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM+sizeof(u8 *)*MAX_CHAR_KEY_NUM);
	
	memcpy(p,gCtrlObjPtrBuf,sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);//ËùÓÐ¶¯Ì¬¿Ø¼þÖ¸Õë
	p=(void *)((u32)p+sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
	memcpy(p,(void *)gRepImgSufx,sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);//ËùÓÐ¹ÌÌ¬Í¼Æ¬°´¼üµÄÌæ´úºó×º
	p=(void *)((u32)p+sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
	memcpy(p,(void *)gRepKeyNameCon,sizeof(u8 *)*MAX_CHAR_KEY_NUM);//ËùÓÐ¹ÌÌ¬ÎÄ×Ö°´¼üµÄÌæ´úÎÄ±¾Ö¸Õë
}

//µ¯³öÒ³ÃæÊý¾Ý
void PopPageCtrlObjData(bool DeleteAll)
{	
	if(DeleteAll)
	{
		u8 i;

		Debug("PopPageCtrlObjData Delete All\n\r");
		for(i=0;i<=MAX_PAGE_LAYER_NUM;i++)
			if(gPageDataPtrRecord[i]!=NULL) 
				Q_Free(gPageDataPtrRecord[i]);
	}
	else if(gPageDataPtrRecord[GetCurLayerNum()]!=NULL)
	{
		void *p=gPageDataPtrRecord[GetCurLayerNum()];

		Debug("PopPageCtrlObjData:%d\n\r",GetCurLayerNum());
		
		memcpy((void *)gCtrlObjPtrBuf,p,sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
		p=(void *)((u32)p+sizeof(void*)*MAX_DYN_CTRL_OBJ_NUM);
		memcpy((void *)gRepImgSufx,p,sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
		p=(void *)((u32)p+sizeof(REP_IMG_SUFX)*MAX_IMG_KEY_NUM);
		memcpy((void *)gRepKeyNameCon,p,sizeof(u8 *)*MAX_CHAR_KEY_NUM);

		Q_Free(gPageDataPtrRecord[GetCurLayerNum()]);
		gPageDataPtrRecord[GetCurLayerNum()]=NULL;
	}
}

//Çå³ýÒ³ÃæÊý¾Ý
void CleanPageCtrlObjData(void)
{
	u8 i;

	Debug("CleanPageCtrlObjData\n\r");
	
	//Çå¿Õ¿Ø¼þ
	for(i=0;i<MAX_DYN_CTRL_OBJ_NUM;i++) gCtrlObjPtrBuf[i]=NULL;

	//Çå¿ÕÁÙÊ±Ìæ»»Í¼±êµÄºó×º´æ´¢
	for(i=0;i<MAX_IMG_KEY_NUM;i++)
		gRepImgSufx[i]=0;
	for(i=0;i<MAX_CHAR_KEY_NUM;i++)
		gRepKeyNameCon[i]=NULL;
}

//¼¤»îµ±Ç°Ò³ÃæÏÂµÄ´¥ÃþÇøÓò¼¯
SYS_MSG CurrPageCtrlObjInit(INPUT_EVT_TYPE EventType,int IntParam,void *pInfoParam)
{
	IMG_BUTTON_OBJ **pDynImgTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_IMG_IDX_START]; //¶¯Ì¬Í¼Æ¬°´¼ü¼¯ºÏ
	CHAR_BUTTON_OBJ **pDynCharTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_CHAR_IDX_START];//¶¯Ì¬ÎÄ×Ö°´¼ü¼¯ºÏ
	SYS_MSG SysMsg=SM_State_Faile;
	u8 Index;
	
	CO_Debug("%s : %s\n\r",__FUNCTION__,gpCurrentPage->Name);

	//4 	»æÖÆÎÄ×Ö´¥ÃþÇøÓò
	for(Index=0;Index<gpCtrlObjNum->CharBtnNum;Index++)
	{
		CharTchDisplay(Index,Input_TchNormal,FALSE);
	}

	//4 	»æÖÆ¶¯Ì¬ÎÄ×Ö´¥ÃþÇøÓò
	for(Index=0;Index<gpCtrlObjNum->DynCharBtnNum;Index++)
	{//ÏÔÊ¾Í¼±ê
		if(pDynCharTchCon[Index]!=NULL)
			CharTchDisplay(Index,Input_TchNormal,TRUE);
	}

	//4 	»æÖÆÍ¼Æ¬´¥ÃþÇøÓò
	for(Index=0;Index<gpCtrlObjNum->ImgBtnNum;Index++)
	{//ÏÔÊ¾Í¼±ê
		ImgTchDisplay(Index,Input_TchNormal,FALSE);
	}

	//4 	»æÖÆ¶¯Ì¬Í¼Æ¬´¥ÃþÇøÓò
	for(Index=0;Index<gpCtrlObjNum->DynImgBtnNum;Index++)
	{//ÏÔÊ¾Í¼±ê
		if(pDynImgTchCon[Index]!=NULL)
			ImgTchDisplay(Index,Input_TchNormal,TRUE);
	}

	//4	 ´Ë´¦ÊÇ·ñÒª¼ÓÆäËû¿Ø¼þ»æÖÆ¿

	//´¥·¢ÊÂ¼þ
	switch(EventType)
	{
		case Input_GotoSubPage://½øÈë×ÓÒ³Ãæ
		case Input_SubPageTranslate://½øÈëÐÂµÄÍ¬¼¶×ÓÒ³Ãæ
		case Input_GotoNewPage://½øÈëÐÂÒ³Ãæ
				SysMsg=gpCurrentPage->SysEvtHandler(Sys_TouchSetOk,IntParam,pInfoParam);
			break;
		case Input_SubPageReturn://´Ó×ÓÒ³Ãæ·µ»Ø
				SysMsg=gpCurrentPage->SysEvtHandler(Sys_TouchSetOk_SR,IntParam,pInfoParam);
			break;
	}
	
	CO_Debug("%s end: %s\n\r",__FUNCTION__,gpCurrentPage->Name);
	return SysMsg;
}

#if 1//user api
//¸ü¸Äµ±Ç°´¥ÃþÓòÄ³¸ö°´¼üµÄÏÔÊ¾Í¼Æ¬
// 1.Ö»¶Ôµ±Ç°Ò³ÃæÓÐÐ§,×ª»»Ò³Ãæ»ò×ÓÒ³ÃæºóÊ§Ð§
// 2.Ö»ÄÜÖ¸¶¨ºó×º£¬±ÈÈçÔ­À´µÄ×ÊÔ´Í¼±êÊÇ"MusicN.bmp"
//    Ö¸¶¨Suffix='T'£¬ÔòÍ¼±ê×ÊÔ´±ä³É"MusicT.bmp"
//Èç¹ûSuffix=0£¬Ôò»Ö¸´Ô­Ê¼Í¼±ê
void Q_ChangeImgTchImg(u8 OID,u8 Suffix)
{
	OS_DeclareCritical();
	
	if((OID=FindImgTchIdx(OID))==0xff) return;
	
	if(OID>=MAX_IMG_KEY_NUM)
	{
		Debug("%s:OID is error!\n\r",__FUNCTION__);
		return;
	}
	
	OS_EnterCritical();
	gRepImgSufx[OID]=Suffix;
	OS_ExitCritical();
}

//ºÍQ_ChangeImgTchImgÏà¶Ô£¬¶ÁÈ¡µ±Ç°µÄÍ¼±êÌæ»»ºó×ºÖµ
//·µ»Ø0±íÊ¾ÊÇÄ¬ÈÏÖµ
//·ñÔò·µ»ØÌæ»»µÄºó×º
u8 Q_ReadImgTchImg(u8 OID)
{
	if((OID=FindImgTchIdx(OID))==0xff) return 0;
	
	if(OID>=MAX_IMG_KEY_NUM)
	{
		Debug("%s:OID is error!\n\r",__FUNCTION__);
		return 0;
	}

	return gRepImgSufx[OID];
}

//¸ü¸Äµ±Ç°´¥ÃþÓòÄ³¸öÎÄ×Ö°´¼üµÄÏÔÊ¾ÎÄ×Ö
// 1.Ö»¶Ôµ±Ç°Ò³ÃæÓÐÐ§,×ª»»Ò³Ãæ»ò×ÓÒ³ÃæºóÊ§Ð§
//Èç¹ûNewName=NULL£¬Ôò»Ö¸´Ô­Ê¼ÎÄ×Ö
void Q_ChangeCharTchName(u8 OID,u8 *NewName)
{
	OS_DeclareCritical();
	
	if((OID=FindCharTchIdx(OID))==0xff) return;
	
	if(OID>=MAX_CHAR_KEY_NUM)
	{
		Debug("%s:OID is error!\n\r",__FUNCTION__);
		return;
	}
	
	OS_EnterCritical();
	gRepKeyNameCon[OID]=NewName;
	OS_ExitCritical();
}

//ºÍQ_ChangeCharTchNameÏà¶Ô£¬¶ÁÈ¡µ±Ç°µÄÍ¼±êÌæ»»ºó×ºÖµ
//·µ»ØNULL±íÊ¾ÊÇÄ¬ÈÏÖµ»òÕßkey³¬³ö·¶Î§
u8 *Q_ReadCharTchName(u8 OID)
{
	if((OID=FindCharTchIdx(OID))==0xff) return NULL;
	
	if(OID>=MAX_CHAR_KEY_NUM)
	{
		Debug("%s:OID is error!\n\r",__FUNCTION__);
		return NULL;
	}
	
	return gRepKeyNameCon[OID];
}

//Á¢¿Ì³ÊÏÖÖ¸¶¨°´¼ü¼üÖµµÄÍ¼±ê»òÎÄ×Ö£¬¶ÔËùÓÐ´Ë¼üÖµµÄ°´¼ü¾ùÓÐÐ§
void Q_PresentTch(u8 OID,TCH_EVT Type)
{
	switch(Type)
	{
		case Tch_Normal:
			SameOidTchDisplay(OID,Input_TchNormal);
			break;
		case Tch_Press:		
			SameOidTchDisplay(OID,Input_TchPress);
			break;
		case Tch_Release:
			SameOidTchDisplay(OID,Input_TchRelease);
			break;
	}	
}

//ÓÃÓÚÉèÖÃÐÂµÄ¶¯Ì¬Í¼±ê°´¼ü
//IdxÖ¸¶¨Ë÷ÒýºÅ£¬´Ó1¿ªÊ¼£¬²»µÃ´óÓÚpageµÄDynImgTchNumÊôÐÔÖµ
bool Q_SetDynamicImgTch(u8 Idx,IMG_BUTTON_OBJ *pBtnObj)
{
	IMG_BUTTON_OBJ **pDynImgTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_IMG_IDX_START]; //¶¯Ì¬Í¼Æ¬°´¼ü¼¯ºÏ

	if((Idx==0)||(Idx>gpCtrlObjNum->DynImgBtnNum)) return FALSE;
	Idx--;

	pDynImgTchCon[Idx]=pBtnObj;

	if(pBtnObj!=NULL)
	{
		u8 Num=CO_STATIC_TCH_NUM+CO_DYN_IMG_IDX_START+Idx;
		gpTouchRegions[Num].x=pDynImgTchCon[Idx]->x;
		gpTouchRegions[Num].y=pDynImgTchCon[Idx]->y;
		gpTouchRegions[Num].w=pDynImgTchCon[Idx]->w;
		gpTouchRegions[Num].h=pDynImgTchCon[Idx]->h;
		gpTouchRegions[Num].ObjID=pDynImgTchCon[Idx]->ObjID;
		gpTouchRegions[Num].Type=COT_DynImgBtn;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(pDynImgTchCon[Idx]->OptionsMask&0xff);
		ImgTchDisplay(Idx,Input_TchNormal,TRUE);
	}
	else
	{
		MemSet(&gpTouchRegions[CO_STATIC_TCH_NUM+CO_DYN_IMG_IDX_START+Idx],0,sizeof(TOUCH_REGION));
	}

	return FALSE;
}

//ÓÃÓÚÉèÖÃÐÂµÄ¶¯Ì¬ÎÄ×Ö°´¼ü
//IdxÖ¸¶¨Ë÷ÒýºÅ£¬´Ó1¿ªÊ¼£¬²»µÃ´óÓÚpageµÄDynCharTchNumÊôÐÔÖµ
bool Q_SetDynamicCharTch(u8 Idx,CHAR_BUTTON_OBJ *pBtnObj)
{
	CHAR_BUTTON_OBJ **pDynCharTchCon=(void *)&gCtrlObjPtrBuf[CO_DYN_CHAR_IDX_START];//¶¯Ì¬ÎÄ×Ö°´¼ü¼¯ºÏ
		
	if((Idx==0)||(Idx>gpCtrlObjNum->DynCharBtnNum)) return FALSE;
	Idx--;

	pDynCharTchCon[Idx]=pBtnObj;

	if(pBtnObj!=NULL)
	{
		u8 Num=CO_STATIC_TCH_NUM+CO_DYN_CHAR_IDX_START+Idx;
		gpTouchRegions[Num].x=pDynCharTchCon[Idx]->x;
		gpTouchRegions[Num].y=pDynCharTchCon[Idx]->y;
		gpTouchRegions[Num].w=pDynCharTchCon[Idx]->w;
		gpTouchRegions[Num].h=pDynCharTchCon[Idx]->h;
		gpTouchRegions[Num].ObjID=pDynCharTchCon[Idx]->ObjID;
		gpTouchRegions[Num].Type=COT_DynCharBtn;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(pDynCharTchCon[Idx]->OptionsMask&0xff);
		CharTchDisplay(Idx,Input_TchNormal,TRUE);
	}
	else
	{
		MemSet(&gpTouchRegions[CO_STATIC_TCH_NUM+CO_DYN_CHAR_IDX_START+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}

//ÉèÖÃyes noÑ¡Ïî£¬pYesNoÖ¸ÏòµÄÄÚ´æÔÚµ÷ÓÃÍêº¯Êýºó²»¿É×¢Ïú
//Ò»µ©ÉèÖÃ£¬µ±½øÈëÒ³ÃæÊ±£¬»áÓÃµ½´ËÄÚ´æ
//ËùÒÔµ±Ò³Ãæ»¹´æÔÚÊ±£¬±ØÐë±£Ö¤´ËÄÚ´æ´æÔÚ
//Idx´Ó1¿ªÊ¼
bool Q_SetYesNo(u8 Idx,YES_NO_OBJ *pYesNo)
{
	YES_NO_OBJ **pYesNoCon=(void *)&gCtrlObjPtrBuf[CO_YES_NO_IDX_START];

	if(Idx>gpCtrlObjNum->YesNoNum) return FALSE;
	Idx--;

	pYesNoCon[Idx]=pYesNo;

	if(pYesNo!=NULL)
	{
		GUI_REGION DrawRegion;
		u8 Num=CO_STATIC_TCH_NUM+CO_YES_NO_IDX_START+Idx;
		gpTouchRegions[Num].x=pYesNo->x;
		gpTouchRegions[Num].y=pYesNo->y;
		gpTouchRegions[Num].w=CO_YES_NO_W;
		gpTouchRegions[Num].h=CO_YES_NO_H;
		gpTouchRegions[Num].ObjID=pYesNo->ObjID;
		gpTouchRegions[Num].Type=COT_YesNo;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);
		
		//draw
		DrawRegion.x=pYesNo->x;
		DrawRegion.y=pYesNo->y;
		DrawRegion.w=CO_YES_NO_W;
		DrawRegion.h=CO_YES_NO_H;
		DrawRegion.Color=CO_YES_NO_TRAN_COLOR;
		if(pYesNo->DefVal==FALSE)
			Gui_DrawImgArray(gCtrlObj_Off,&DrawRegion);
		else 
			Gui_DrawImgArray(gCtrlObj_On,&DrawRegion);
	}
	else
	{//Çå¿Õ´¥ÃþÇøÓò¼ÇÂ¼
		MemSet(&gpTouchRegions[CO_STATIC_TCH_NUM+CO_YES_NO_IDX_START+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}

//ÉèÖÃnum boxÑ¡Ïî£¬pNumBoxÖ¸ÏòµÄÄÚ´æÔÚµ÷ÓÃÍêº¯Êýºó²»¿É×¢Ïú
//Ò»µ©ÉèÖÃ£¬µ±½øÈëÒ³ÃæÊ±£¬»áÓÃµ½´ËÄÚ´æ
//ËùÒÔµ±Ò³Ãæ»¹´æÔÚÊ±£¬±ØÐë±£Ö¤´ËÄÚ´æ´æÔÚ
//Idx´Ó1¿ªÊ¼
bool Q_SetNumCtrlObj(u8 Idx,NUM_CTRL_OBJ *pNumCtrlObj)
{
	NUM_CTRL_OBJ **pNumCtrlObjCon=(void *)&gCtrlObjPtrBuf[CO_NUM_IDX_START];
	if(Idx>gpCtrlObjNum->NumCtrlObjNum) return FALSE;
	Idx--;
	
	pNumCtrlObjCon[Idx]=pNumCtrlObj;

	if(pNumCtrlObj!=NULL)
	{
		GUI_REGION DrawRegion;
		u8 Num=CO_STATIC_TCH_NUM+CO_NUM_IDX_START+Idx;
		gpTouchRegions[Num].x=pNumCtrlObj->x;
		gpTouchRegions[Num].y=pNumCtrlObj->y;
		gpTouchRegions[Num].w=pNumCtrlObj->w;
		gpTouchRegions[Num].h=CO_NUM_H;
		gpTouchRegions[Num].ObjID=pNumCtrlObj->ObjID;
		gpTouchRegions[Num].Type=COT_Num;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);

		{//draw
			u8 NumStr[32];
			DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.y=pNumCtrlObj->y;
			DrawRegion.w=CO_NUM_FRAME_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumLeft,&DrawRegion);//×ó±ß¿ò

			DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumCtrlObj->y;
			DrawRegion.w=CO_NUM_MIDDLE_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_FillImgArray_H(gCtrlObj_NumMiddle,pNumCtrlObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	

			DrawRegion.x=pNumCtrlObj->x+pNumCtrlObj->w-CO_NUM_ARROW_W;
			DrawRegion.y=pNumCtrlObj->y;
			DrawRegion.w=CO_NUM_FRAME_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumRight,&DrawRegion);//ÓÒ±ß¿ò
			
			sprintf((void *)NumStr,"%d",pNumCtrlObj->Value);//Êý×Ö
			if(strlen((void *)NumStr)*CO_NUM_FONT_W < (pNumCtrlObj->w-(CO_NUM_ARROW_W<<1)))
				DrawRegion.x=pNumCtrlObj->x+((pNumCtrlObj->w-strlen((void *)NumStr)*CO_NUM_FONT_W)>>1);
			else //ÏÔÊ¾³¤¶È³¬³ö·½¿ò
				DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumCtrlObj->y+3;
			DrawRegion.w=pNumCtrlObj->w-(CO_NUM_ARROW_W<<1);
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_FONT_COLOR;
			DrawRegion.Space=CO_NUM_FONT_SPACE;
			Gui_DrawFont(CO_NUM_FONT_STYLE,NumStr,&DrawRegion);
		}
	}
	else
	{//Çå¿Õ´¥ÃþÇøÓò¼ÇÂ¼
		MemSet(&gpTouchRegions[CO_STATIC_TCH_NUM+CO_NUM_IDX_START+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}

//ÉèÖÃstr¿Ø¼þÑ¡Ïî£¬pStrCtrlObjÖ¸ÏòµÄÄÚ´æÔÚµ÷ÓÃÍêº¯Êýºó²»¿É×¢Ïú
//Ò»µ©ÉèÖÃ£¬µ±½øÈëÒ³ÃæÊ±£¬»áÓÃµ½´ËÄÚ´æ
//ËùÒÔµ±Ò³Ãæ»¹´æÔÚÊ±£¬±ØÐë±£Ö¤´ËÄÚ´æ´æÔÚ
//Idx´Ó1¿ªÊ¼
bool Q_SetStrCtrlObj(u8 Idx,STR_CTRL_OBJ *pStrCtrlObj)
{
	STR_CTRL_OBJ **pStrCtrlObjCon=(void *)&gCtrlObjPtrBuf[CO_STR_IDX_START];

	if(Idx>gpCtrlObjNum->StrCtrlObjNum) return FALSE;
	Idx--;
	
	pStrCtrlObjCon[Idx]=pStrCtrlObj;

	if(pStrCtrlObj!=NULL)
	{
		GUI_REGION DrawRegion;
		u8 Num=CO_STATIC_TCH_NUM+CO_STR_IDX_START+Idx;
		gpTouchRegions[Num].x=pStrCtrlObj->x;
		gpTouchRegions[Num].y=pStrCtrlObj->y;
		gpTouchRegions[Num].w=pStrCtrlObj->w;
		if(pStrCtrlObj->Type==SCOT_StrBox)
			gpTouchRegions[Num].h=pStrCtrlObj->h;
		else if(pStrCtrlObj->Type==SCOT_StrEnum)
			gpTouchRegions[Num].h=CO_STR_ENUM_H;
		gpTouchRegions[Num].ObjID=pStrCtrlObj->ObjID;
		gpTouchRegions[Num].Type=COT_Str;
		gpTouchRegions[Num].Index=Idx;
		gpTouchRegions[Num].OptionsMask=(u8)(PrsMsk|RelMsk|ReVMsk);

		//draw
		if(pStrCtrlObj->Type==SCOT_StrBox)
		{
			STR_BOX_OBJ *pStrBoxObj=(void *)pStrCtrlObj;
		
		}
		else if(pStrCtrlObj->Type==SCOT_StrEnum)
		{
			STR_ENUM_OBJ *pStrEnumObj=(void *)pStrCtrlObj;
			u8 StrBuf[32];
			
			DrawRegion.x=pStrEnumObj->x+CO_STR_ENUM_ARROW_W-CO_STR_ENUM_FRAME_W;
			DrawRegion.y=pStrEnumObj->y;
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumLeft,&DrawRegion);//×ó±ß¿ò

			DrawRegion.x=pStrEnumObj->x+CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrEnumObj->y;
			DrawRegion.w=CO_STR_ENUM_MIDDLE_W;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
			Gui_FillImgArray_H(gCtrlObj_NumMiddle,pStrEnumObj->w-(CO_STR_ENUM_ARROW_W<<1),&DrawRegion);	//ÖÐ¿ò

			DrawRegion.x=pStrEnumObj->x+pStrEnumObj->w-CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrEnumObj->y;
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumRight,&DrawRegion);//ÓÒ±ß¿ò

			if(pStrEnumObj->Size)//ÓÐÄÚÈÝ
			{
				sprintf((void *)StrBuf,"%s",&pStrEnumObj->pStrEnumBuf[pStrEnumObj->Idx+1]);//×Ö·û´®
				if(strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W < (pStrEnumObj->w-(CO_STR_ENUM_ARROW_W<<1)))
					DrawRegion.x=pStrEnumObj->x+((pStrEnumObj->w-strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W)>>1);
				else //ÏÔÊ¾³¤¶È³¬³ö·½¿ò
					DrawRegion.x=pStrEnumObj->x+CO_STR_ENUM_ARROW_W;
				DrawRegion.y=pStrEnumObj->y+3;
				DrawRegion.w=pStrEnumObj->w-(CO_STR_ENUM_ARROW_W<<1);
				DrawRegion.h=CO_STR_ENUM_H;
				DrawRegion.Color=CO_STR_ENUM_FONT_COLOR;
				DrawRegion.Space=CO_STR_ENUM_FONT_SPACE;
				Gui_DrawFont(CO_STR_ENUM_FONT_STYLE,StrBuf,&DrawRegion);
			}
		}
	}
	else
	{//Çå¿Õ´¥ÃþÇøÓò¼ÇÂ¼
		MemSet(&gpTouchRegions[CO_STATIC_TCH_NUM+CO_STR_IDX_START+Idx],0,sizeof(TOUCH_REGION));
	}
	
	return FALSE;
}

//ÏÔÊ¾Ö¸¶¨idµÄ×Ö·û´®Ïî
bool Q_StrEnumDisplayOne(STR_ENUM_OBJ *pStrEnumObj,u8 StrID)
{
	GUI_REGION DrawRegion;
	u8 StrBuf[32];
	u8 *p=pStrEnumObj->pStrEnumBuf;
	u8 *pEnumBuf=pStrEnumObj->pStrEnumBuf;
	u16 i=0;
	
	//Í¨¹ýidÕÒidx
	while(p[0]!=StrID)//²»ÊÇÕâ¸ö×Ö·û´®
	{
		while(pEnumBuf[i++]); //ÕÒ¼ä¸ô·û
		if(i>=pStrEnumObj->Size) return FALSE;//ÕÒµ½µ×ÁË£¬Ã»µÃÕÒÁË
		p=&pEnumBuf[i];//Ö¸µ½ÏÂÒ»¸ö×Ö·û´®Ê×
	}
	pStrEnumObj->Idx=i;
	
	//ÖÐ¼ä¿ò²¿·Ö
	DrawRegion.x=pStrEnumObj->x+CO_STR_ENUM_ARROW_W;
	DrawRegion.y=pStrEnumObj->y;
	DrawRegion.w=CO_STR_ENUM_MIDDLE_W;
	DrawRegion.h=CO_STR_ENUM_H;
	DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
	Gui_FillImgArray_H(gCtrlObj_StrEnumMiddle,pStrEnumObj->w-(CO_STR_ENUM_ARROW_W<<1),&DrawRegion);	//¿ò

	sprintf((void *)StrBuf,"%s",&pStrEnumObj->pStrEnumBuf[pStrEnumObj->Idx+1]);//×Ö·û´®
	if(strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W < (pStrEnumObj->w-(CO_STR_ENUM_ARROW_W<<1)))
		DrawRegion.x=pStrEnumObj->x+((pStrEnumObj->w-strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W)>>1);
	else //ÏÔÊ¾³¤¶È³¬³ö·½¿ò
		DrawRegion.x=pStrEnumObj->x+CO_STR_ENUM_ARROW_W;
	DrawRegion.y=pStrEnumObj->y+3;
	DrawRegion.w=pStrEnumObj->w-(CO_STR_ENUM_ARROW_W<<1);
	DrawRegion.h=CO_STR_ENUM_H;
	DrawRegion.Color=CO_STR_ENUM_FONT_COLOR;
	DrawRegion.Space=CO_STR_ENUM_FONT_SPACE;
	Gui_DrawFont(CO_STR_ENUM_FONT_STYLE,StrBuf,&DrawRegion);

	return TRUE;
}

//Íùenum bufÀï¼ÓÒ»¸ö×Ö·û´®£¬ÓÃ0¼ä¸ô£¬µÚÒ»¸ö×Ö·ûÎªid£¬ºóÃæ½ô¸ú×Å×Ö·û´®ÄÚÈÝ
//StrID:¼ÓÈëµÄ×Ö·û´®Î¨Ò»µÄid
//Str:¼ÓÈëµÄ×Ö·û´®
bool Q_StrEnumAddOne(STR_ENUM_OBJ *pStrEnumObj,u8 StrID,u8 *Str)
{
	u8 StrLen=strlen((void *)Str);
	
	if((pStrEnumObj->Size+StrLen+3) > pStrEnumObj->TotalSize)//¿Õ¼ä²»¹».3 = Ç°ºó¼ä¸ô·û+id·û
		return FALSE;
	
	//¸´ÖÆ×Ö·û´®²»ÓÃMemCpy£¬·ÀÖ¹¶ÔÆëÎÊÌâ
	{
		u8 i;
		u8 *p=&pStrEnumObj->pStrEnumBuf[pStrEnumObj->Size?pStrEnumObj->Size+1:0];//ÕÒµ½Ã¶¾ÙÁÐ±íÎ²
		*p++=StrID;
		for(i=0;i<=StrLen;i++)//Í¬Ê±¸´ÖÆÁË0½áÊø·û
		{
			p[i]=Str[i];
		}
	}

	pStrEnumObj->Size+=((pStrEnumObj->Size?StrLen+1:StrLen)+1);//¼ä¸ô·û³¤¶È+×Ö·û´®³¤¶È+ID³¤¶È
	Q_StrEnumDisplayOne(pStrEnumObj,StrID);//ÐÞ¸ÄÏÔÊ¾
	return TRUE;
}

//É¾³ýÖ¸¶¨idµÄ×Ö·û´®Ã¶¾ÙÏî
bool Q_StrEnumDeleteOne(STR_ENUM_OBJ *pStrEnumObj,u8 StrID)
{
	if(pStrEnumObj->Size == 0) return FALSE;

	{
		u8 *p=pStrEnumObj->pStrEnumBuf;
		u8 *pEnumBuf=pStrEnumObj->pStrEnumBuf;
		u16 i=0;
		
		while(p[0]!=StrID)//²»ÊÇÕâ¸ö×Ö·û´®
		{
			while(pEnumBuf[i++]); //ÕÒ¼ä¸ô·û
			if(i>=pStrEnumObj->Size) return FALSE;//ÕÒµ½µ×ÁË£¬Ã»µÃÕÒÁË
			p=&pEnumBuf[i];//Ö¸µ½ÏÂÒ»¸ö×Ö·û´®Ê×
		}
		//Ìø³öÇ°ÃæµÄÑ­»·±íÊ¾ÕÒµ½ÁËÆ¥ÅäµÄ×Ö·û´®
		{
			u8 DeleteIdx=i;//iÊÇ×Ö·û´®Ê×µÄÐòºÅ
			u8 NextIdx;
			
			while(pEnumBuf[i]) pEnumBuf[i++]=0; //ÕÒ¼ä¸ô·û
			NextIdx=++i;//ÏÂÒ»¸ö×Ö·û´®Ê×
			
			if(pStrEnumObj->Idx == DeleteIdx) pStrEnumObj->Idx=0;//±¾×Ö·û´®Ë÷Òý¹éÎ»
			else if(pStrEnumObj->Idx >= NextIdx) pStrEnumObj->Idx-=(NextIdx-DeleteIdx);//ºóÐø×Ö·û´®Ë÷Òý¹éÎ»

			if(NextIdx>=pStrEnumObj->Size)//ÕÒµ½µ×ÁË£¬ÒªÉ¾³ýµÄ×Ö·û´®¾ÍÊÇ×îºóÒ»¸ö×Ö·û´®
			{
				if(DeleteIdx) pStrEnumObj->Size=DeleteIdx-1;
				else pStrEnumObj->Size=0;
				Q_StrEnumDisplayOne(pStrEnumObj,pEnumBuf[pStrEnumObj->Idx]);//ÐÞ¸ÄÏÔÊ¾
				return TRUE;
			}
			else
			{
				u8 *pLastStr=&pEnumBuf[NextIdx];//Ê£ÏÂµÄ×Ö·û´®
				u8 j,n;
				n=pStrEnumObj->Size-NextIdx+1;//Ê£ÏÂÒª¸´ÖÆµÄ²¿·Ö£¬º¬×îºóÒ»¸ö½áÊø·û
				for(j=0;j<n;j++)
				{
					p[j]=pLastStr[j];
					pLastStr[j]=0;
				}
				pStrEnumObj->Size-=(NextIdx-DeleteIdx);
				Q_StrEnumDisplayOne(pStrEnumObj,pEnumBuf[pStrEnumObj->Idx]);//ÐÞ¸ÄÏÔÊ¾
				return TRUE;
			}
		}
	}
}





#endif




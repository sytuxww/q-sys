/**************** Q-SYS *******************
 * PageName : StrCtrlObjPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "StrCtrlObjPage.h"
#include "CtrlObjImg.h"

//��������
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------��ҳϵͳ����������-----------------------
//����ҳ�水����Ҫ�õ���ö�٣�����������Ψһ�ĺ궨��
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//�����Ӳ���汾��֧�ָ����ⲿ�жϰ����������߼��̣�
	//���Դ�����ֵ��USER_KEY_VALUE_START��ʼ����ǰ��ļ�ֵ��������
	LeftArrowKV=USER_KEY_VALUE_START,
	RightArrowKV,
	StrFrameKV,
	
}StrCtrlObjPage_OID;

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE StrCtrlObjPage={
	"StrCtrlObjPage",
	"Karlno",
	"Ctrl Obj Page",
	POP_PAGE,
	0,//

	{
		0,//size of touch region array
		0,//size of touch region array,
		3,
		0,
	},
	NULL, 
	NULL,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),
	ButtonHandler,
};

//-----------------------��ҳ�Զ����������-----------------------
//����ѡ��
#define TOUCH_REGION_MARGIN 4//margin�����ӿɵ������Χ

//��̬����ģ��
static const IMG_BUTTON_OBJ ArrowTchTmp={"",0,PrsMsk|RelMsk|ReVMsk,0,0,CO_STR_ENUM_ARROW_W,CO_STR_ENUM_H,0,0,"",FatColor(NO_TRANS)};
static const IMG_BUTTON_OBJ StrTchTmp={"",0,RelMsk,0,0,0,CO_STR_ENUM_H,0,0,"",FatColor(NO_TRANS)};

typedef struct{
	STR_CTRL_OBJ_TYPE HandlerType;//�ؼ�����
	void *ObjHandler;//�ؼ�ʵ��ָ��
	u16 LeftDispBuf[CO_STR_ENUM_ARROW_W*CO_STR_ENUM_H];//�ؼ����ͷ����
	u16 RightDispBuf[CO_STR_ENUM_ARROW_W*CO_STR_ENUM_H];//�ؼ��Ҽ�ͷ����
	IMG_BUTTON_OBJ LeftArrowBtn;//���ͷ��̬����ʵ��
	IMG_BUTTON_OBJ RightArrowBtn;//�Ҽ�ͷ��̬����ʵ��
	IMG_BUTTON_OBJ StrBtn;//�м����ֿ�̬����ʵ��
}STR_CTRL_OBJ_PAGE_VARS;
static STR_CTRL_OBJ_PAGE_VARS *gpScopVars;//ֻ��Ҫ����һ��ָ�룬����ȫ�ֱ�����ʹ��

//-----------------------��ҳ�Զ��庯��-----------------------
typedef enum{
	DSA_Null=0,//ά��ԭ��
	DSA_Normal,//��ͨ
	DSA_HiLight,//����
	DSA_NormalArrow,//��ͨ��ͷ
	DSA_HiLightArrow//������ͷ
}DRAW_SE_ACT;
//����Ҫ�����StrEnum�ؼ��Ŀ�������Ҽ�ͷ������
static void DrawStrCtrlObj(STR_CTRL_OBJ *pStrCtrlObj,DRAW_SE_ACT Left,DRAW_SE_ACT Middle,DRAW_SE_ACT Right)
{
	GUI_REGION DrawRegion;
	u8 StrBuf[32];
	
	if(pStrCtrlObj->Type==SCOT_StrEnum)
	{
		if(Left != DSA_Null)//��߲���
		{
			DrawRegion.y=pStrCtrlObj->y;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
		}
		if(Left == DSA_Normal)
		{
			DrawRegion.x=pStrCtrlObj->x+CO_STR_ENUM_ARROW_W-CO_STR_ENUM_FRAME_W;
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeft,&DrawRegion);//��߿�
		}
		else if(Left == DSA_HiLight)
		{
			DrawRegion.x=pStrCtrlObj->x+CO_STR_ENUM_ARROW_W-CO_STR_ENUM_FRAME_W;
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeftH,&DrawRegion);//��߿�
		}
		else if(Left == DSA_NormalArrow)
		{
			DrawRegion.x=pStrCtrlObj->x;
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeftArrow,&DrawRegion);//���ͷ�滭
		}
		else if(Left == DSA_HiLightArrow)
		{
			DrawRegion.x=pStrCtrlObj->x;
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeftArrowH,&DrawRegion);//���ͷ�滭
		}

		if(Middle != DSA_Null)//�м�򲿷�
		{
			DrawRegion.x=pStrCtrlObj->x+CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrCtrlObj->y;
			DrawRegion.w=CO_STR_ENUM_MIDDLE_W;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
		}
		if(Middle == DSA_Normal)
			Gui_FillImgArray_H(gCtrlObj_StrEnumMiddle,pStrCtrlObj->w-(CO_STR_ENUM_ARROW_W<<1),&DrawRegion);	//��
		else if(Middle == DSA_HiLight)
			Gui_FillImgArray_H(gCtrlObj_StrEnumMiddleH,pStrCtrlObj->w-(CO_STR_ENUM_ARROW_W<<1),&DrawRegion);	//������

		if(Middle != DSA_Null)//�м����ֲ���
		{
			STR_ENUM_OBJ *pStrEnum=(STR_ENUM_OBJ *)pStrCtrlObj;
			sprintf((void *)StrBuf,"%s",&pStrEnum->pStrEnumBuf[pStrEnum->Idx+1]);//�ַ���
			if(strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W < (pStrEnum->w-(CO_STR_ENUM_ARROW_W<<1)))
				DrawRegion.x=pStrEnum->x+((pStrEnum->w-strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W)>>1);
			else //��ʾ���ȳ�������
				DrawRegion.x=pStrEnum->x+CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrEnum->y+3;
			DrawRegion.w=pStrEnum->w-(CO_STR_ENUM_ARROW_W<<1);
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Space=CO_STR_ENUM_FONT_SPACE;
		}
		if(Middle == DSA_Normal)
		{
			DrawRegion.Color=CO_STR_ENUM_FONT_COLOR;
			Gui_DrawFont(CO_STR_ENUM_FONT_STYLE,StrBuf,&DrawRegion);//����
		}
		else if(Middle == DSA_HiLight)
		{
			DrawRegion.Color=CO_STR_ENUM_FONT_COLOR_H;
			Gui_DrawFont(CO_STR_ENUM_FONT_STYLE,StrBuf,&DrawRegion);//����
		}
		
		if(Right != DSA_Null)//�ұ߲���
		{
			DrawRegion.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrCtrlObj->y;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
		}
		if(Right == DSA_Normal)
		{
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRight,&DrawRegion);//�ұ߿�
		}
		else if(Right == DSA_HiLight)
		{
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRightH,&DrawRegion);//�ұ߿�
		}
		else if(Right == DSA_NormalArrow)
		{
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRightArrow,&DrawRegion);//�Ҽ�ͷ�滭
		}
		else if(Right == DSA_HiLightArrow)
		{
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRightArrowH,&DrawRegion);//�Ҽ�ͷ�滭			
		}
	}
	else if(pStrCtrlObj->Type==SCOT_StrBox)
	{

	}
}

//-----------------------��ҳϵͳ����----------------------
//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	STR_CTRL_OBJ *pStrCtrlObj=pSysParam;
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
#if 1
			if((pStrCtrlObj->Type == SCOT_StrBox) &&(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE)) //�ļ�ϵͳû�й���
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}	
#endif
			return SM_State_OK;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�		
			gpScopVars=Q_PageMallco(sizeof(STR_CTRL_OBJ_PAGE_VARS));
			gpScopVars->HandlerType=pStrCtrlObj->Type;

			if((pStrCtrlObj->Type == SCOT_StrEnum)||(pStrCtrlObj->Type == SCOT_StrBox))
			{
				if(pStrCtrlObj->Type == SCOT_StrBox)
				{
					//CalculateKeyBoardPosition(pStrCtrlObj,&DrawRegion);//����С��������
					//PrtScreenToBin(KEY_BOARD_LCD_BUF_PATH,DrawRegion.x,DrawRegion.y,DrawRegion.w,DrawRegion.h);
					//DrawAndRegKeyBoard(&DrawRegion);
				}
				
				DrawRegion.x=pStrCtrlObj->x;
				DrawRegion.y=pStrCtrlObj->y;
				DrawRegion.w=CO_STR_ENUM_ARROW_W;
				DrawRegion.h=CO_STR_ENUM_H;
				Gui_ReadRegion16Bit(gpScopVars->LeftDispBuf,&DrawRegion);//�����ͼ

				DrawRegion.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_STR_ENUM_ARROW_W;
				DrawRegion.y=pStrCtrlObj->y;
				DrawRegion.w=CO_STR_ENUM_ARROW_W;
				DrawRegion.h=CO_STR_ENUM_H;
				Gui_ReadRegion16Bit(gpScopVars->RightDispBuf,&DrawRegion);//�����ͼ

				DrawStrCtrlObj((void *)pStrCtrlObj,DSA_NormalArrow,DSA_HiLight,DSA_NormalArrow);//����ͼ

				MemCpy(&gpScopVars->LeftArrowBtn,&ArrowTchTmp,sizeof(IMG_BUTTON_OBJ));//���ͷ
				gpScopVars->LeftArrowBtn.ObjID=LeftArrowKV;
				gpScopVars->LeftArrowBtn.x=pStrCtrlObj->x-TOUCH_REGION_MARGIN;
				gpScopVars->LeftArrowBtn.y=pStrCtrlObj->y-TOUCH_REGION_MARGIN;
				gpScopVars->LeftArrowBtn.w+=(TOUCH_REGION_MARGIN<<1);
				gpScopVars->LeftArrowBtn.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(1,&gpScopVars->LeftArrowBtn);//������̬��������

				MemCpy(&gpScopVars->RightArrowBtn,&ArrowTchTmp,sizeof(IMG_BUTTON_OBJ));//�Ҽ�ͷ
				gpScopVars->RightArrowBtn.ObjID=RightArrowKV;
				gpScopVars->RightArrowBtn.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_NUM_ARROW_W-TOUCH_REGION_MARGIN;
				gpScopVars->RightArrowBtn.y=pStrCtrlObj->y-TOUCH_REGION_MARGIN;
				gpScopVars->RightArrowBtn.w+=(TOUCH_REGION_MARGIN<<1);
				gpScopVars->RightArrowBtn.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(2,&gpScopVars->RightArrowBtn);//������̬��������

				MemCpy(&gpScopVars->StrBtn,&StrTchTmp,sizeof(IMG_BUTTON_OBJ));//���ֿ�ģ��
				gpScopVars->StrBtn.ObjID=StrFrameKV;
				gpScopVars->StrBtn.x=pStrCtrlObj->x+CO_NUM_ARROW_W+TOUCH_REGION_MARGIN;
				gpScopVars->StrBtn.y=DrawRegion.y;
				gpScopVars->StrBtn.w=pStrCtrlObj->w-(CO_NUM_ARROW_W<<1)-(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(3,&gpScopVars->StrBtn);//������̬��������

				gpScopVars->ObjHandler=pStrCtrlObj;//��¼�ؼ�ָ��			
			}
			else
			{
			}
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			if(gpScopVars!=NULL)
			{
				if((gpScopVars->HandlerType==SCOT_StrEnum)||(gpScopVars->HandlerType==SCOT_StrBox))
				{
					pStrCtrlObj=(void *)gpScopVars->ObjHandler;
					
					DrawRegion.x=pStrCtrlObj->x;
					DrawRegion.y=pStrCtrlObj->y;
					DrawRegion.w=CO_STR_ENUM_ARROW_W;
					DrawRegion.h=CO_STR_ENUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpScopVars->LeftDispBuf,&DrawRegion);//��ԭ

					DrawRegion.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_STR_ENUM_ARROW_W;
					DrawRegion.y=pStrCtrlObj->y;
					DrawRegion.w=CO_STR_ENUM_ARROW_W;
					DrawRegion.h=CO_STR_ENUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpScopVars->RightDispBuf,&DrawRegion);//��ԭ

					DrawStrCtrlObj((void *)pStrCtrlObj,DSA_Normal,DSA_Normal,DSA_Normal);//δ�ı�ԭ��ֵ
					
					if(gpScopVars->HandlerType==SCOT_StrBox)//num boxҪ�ָ�С��������
					{
						//pStrCtrlObj=(void *)gpScopVars->ObjHandler;
						//CalculateKeyBoardPosition(pStrCtrlObj,&DrawRegion);//����С��������
						//Gui_DrawImgBin(KEY_BOARD_LCD_BUF_PATH,&DrawRegion);
					}
				}
				else
				{
				}

				Q_PageFree(gpScopVars);
			}
			return SM_NoPopReturn|SM_State_OK;//��������ҳ��pop page return�¼�
		default:
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return SM_State_OK;
}

static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			if(IntParam==ExtiKeyEnter)
			{
			}
			else if(IntParam==ExtiKeyUp)
			{
			}
			else if(IntParam==ExtiKeyDown)
			{
			}
			break;
		case Perip_KeyRelease:
			if(IntParam==ExtiKeyEnter)
			{PrtScreen();
			}
			else if(IntParam==ExtiKeyUp)
			{
			}
			else if(IntParam==ExtiKeyDown)
			{
			}
			break;
		case Perip_UartInput:
			if((IntParam>>16)==1)//����1
			{
				Q_Sh_CmdHandler(IntParam&0xffff,pParam);
			}
			break;
	}

	return 0;
}

//��ʹ���߰��±�ҳTouchRegionSet�ﶨ��İ���ʱ���ᴥ�����������Ķ�Ӧ�¼�
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{		
	//GUI_REGION DrawRegion;
	
	switch(Key)
	{
		case LeftArrowKV://-
			if(InEvent == Tch_Press)
			{
				STR_CTRL_OBJ *pStrCtrlObj=(void *)gpScopVars->ObjHandler;
				DrawStrCtrlObj(pStrCtrlObj,DSA_HiLightArrow,DSA_Null,DSA_Null);//����ͼ
			}
			else
			{
				STR_BOX_OBJ *pStrBox=(void *)gpScopVars->ObjHandler;
				STR_ENUM_OBJ *pStrEnum=(void *)gpScopVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pStrBox->Type == SCOT_StrBox)
					{

					}
					else if(pStrEnum->Type == SCOT_StrEnum && pStrEnum->Size)
					{
						if(pStrEnum->Idx == 0)//�����ǰ�ǵ�һ�����������һ����ĩβ
							pStrEnum->Idx=pStrEnum->Size-1;
						else
							pStrEnum->Idx-=2;//��������ǰһ���ַ�����ĩβ
						
						while(pStrEnum->Idx>0)//���ַ�����ͷ
						{
							pStrEnum->Idx--;
							if(pStrEnum->pStrEnumBuf[pStrEnum->Idx] == 0)
							{
								pStrEnum->Idx++;
								break;
							}
						}
					}
				}
				DrawStrCtrlObj((void *)gpScopVars->ObjHandler,DSA_NormalArrow,DSA_HiLight,DSA_Null);//����ͼ
			}
			break;
		case RightArrowKV://+
			if(InEvent == Tch_Press)
			{
				STR_CTRL_OBJ *pStrCtrlObj=(void *)gpScopVars->ObjHandler;
				DrawStrCtrlObj(pStrCtrlObj,DSA_Null,DSA_Null,DSA_HiLightArrow);//����ͼ
			}
			else
			{					
				STR_CTRL_OBJ *pStrBox=(void *)gpScopVars->ObjHandler;
				STR_ENUM_OBJ *pStrEnum=(void *)gpScopVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pStrBox->Type == SCOT_StrBox)
					{

					}
					else if(pStrEnum->Type == SCOT_StrEnum && pStrEnum->Size)
					{
						while(pStrEnum->pStrEnumBuf[++pStrEnum->Idx]);//�ҵ���ǰ�ַ�����ĩβ������

						if(pStrEnum->Idx>=pStrEnum->Size)//�����ˣ���������һ��
							pStrEnum->Idx=0;
						else 
							pStrEnum->Idx++;//����һ��			
					}
				}
				DrawStrCtrlObj((void *)gpScopVars->ObjHandler,DSA_Null,DSA_HiLight,DSA_NormalArrow);//����ͼ
			}
			break;
		case StrFrameKV://������屾���ʾ����
			Q_GotoPage(SubPageReturn,"",0,gpScopVars->ObjHandler);//����ǰһ��ҳ��
			break;
			
		default:
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}




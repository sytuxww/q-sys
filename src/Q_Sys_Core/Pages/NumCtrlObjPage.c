/**************** Q-SYS *******************
 * PageName : NumCtrlObjPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "NumCtrlObjPage.h"
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
	NumFrameKV,
	
	Key0KV,
	Key1KV,
	Key2KV,
	Key3KV,
	Key4KV,
	Key5KV,
	Key6KV,
	Key7KV,
	Key8KV,
	Key9KV,
	KeyMinusKv,//+-
	KeyDotKV,//.
	KeyCancleKV,//X
	KeyOkKV,//O
}NumCtrlObjPage_OID;

static const CHAR_BUTTON_OBJ NumButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,
		//Char_x,Char_y,MarginXY,SpaceXY,NormalColor,NormalBG,PressColor,PressBG,ReleaseColor,ReleaseBG},
	{"1",Key1KV,B14Msk|RelMsk,4,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"2",Key2KV,B14Msk|RelMsk,28,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"3",Key3KV,B14Msk|RelMsk,52,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"4",Key4KV,B14Msk|RelMsk,76,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"5",Key5KV,B14Msk|RelMsk,100,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"-",KeyMinusKv,B14Msk|RelMsk,124,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"X",KeyCancleKV,B14Msk|RelMsk,148,4,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},

	{"6",Key6KV,B14Msk|RelMsk,4,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"7",Key7KV,B14Msk|RelMsk,28,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"8",Key8KV,B14Msk|RelMsk,52,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"9",Key9KV,B14Msk|RelMsk,76,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"0",Key0KV,B14Msk|RelMsk,100,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{".",KeyDotKV,B14Msk|RelMsk,124,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
	{"O",KeyOkKV,B14Msk|RelMsk,148,28,24,24,
		8,5,0x22,0x00,FatColor(0x555555),FatColor(0xcccccc),FatColor(0xffffff),FatColor(0xfc9400),FatColor(0x555555),FatColor(0xcccccc)},
};

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE NumCtrlObjPage={
	"NumCtrlObjPage",
	"Karlno",
	"Ctrl Obj Page",
	POP_PAGE,
	0,//

	{
		0,//size of touch region array
		0,//size of touch region array,
		3,
		sizeof(NumButtonCon)/sizeof(CHAR_BUTTON_OBJ),//С�����ϵĶ�̬��������
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
#define KEY_BOARD_BG_COLOR FatColor(0xffffff)
#define KEY_BOARD_LCD_BUF_PATH "/Temp/NumBoxPS.buf"

//��̬����ģ��
static const IMG_BUTTON_OBJ ArrowTchTmp={"",0,PrsMsk|RelMsk|ReVMsk,0,0,CO_NUM_ARROW_W,CO_NUM_H,0,0,"",FatColor(NO_TRANS)};
static const IMG_BUTTON_OBJ NumBtnTmp={"",0,RelMsk,0,0,0,CO_NUM_H,0,0,"",FatColor(NO_TRANS)};
static const GUI_REGION KeyBoardRegionTmp={0,0,176,56,0,FatColor(NO_TRANS)};

typedef struct{
	NUM_CTRL_OBJ_TYPE HandlerType;//�ؼ�����
	void *ObjHandler;//�ؼ�ʵ��ָ��
	s32 DefValue;//�ؼ�Ĭ��ֵ
	u16 LeftDispBuf[CO_NUM_ARROW_W*CO_NUM_H];//�ؼ����ͷ����
	u16 RightDispBuf[CO_NUM_ARROW_W*CO_NUM_H];//�ؼ��Ҽ�ͷ����
	IMG_BUTTON_OBJ LeftArrowBtn;//���ͷ��̬����ʵ��
	IMG_BUTTON_OBJ RightArrowBtn;//�Ҽ�ͷ��̬����ʵ��
	IMG_BUTTON_OBJ NumBtn;//�м����ֿ�̬����ʵ��
	CHAR_BUTTON_OBJ KeyBoard[sizeof(NumButtonCon)/sizeof(CHAR_BUTTON_OBJ)];//С����ʵ��
	GUI_REGION NumKeySaveReg;//С���̷�Χ
}NUM_CTRL_OBJ_PAGE_VARS;
static NUM_CTRL_OBJ_PAGE_VARS *gpNcopVars;//ֻ��Ҫ����һ��ָ�룬����ȫ�ֱ�����ʹ��

//-----------------------��ҳ�Զ��庯��-----------------------
typedef enum{
	DNA_Null=0,//ά��ԭ��
	DNA_Normal,//��ͨ
	DNA_HiLight,//����
	DNA_NormalArrow,//��ͨ��ͷ
	DNA_HiLightArrow//������ͷ
}DRAW_NL_ACT;
//����Ҫ�����Num�ؼ��Ŀ�������Ҽ�ͷ������
static void DrawNumCtrlObj(NUM_CTRL_OBJ *pNumCtrlObj,DRAW_NL_ACT Left,DRAW_NL_ACT Middle,DRAW_NL_ACT Right)
{
	GUI_REGION DrawRegion;
	u8 NumStr[32];
	
	if((pNumCtrlObj->Type==NCOT_NumList)||(pNumCtrlObj->Type==NCOT_NumEnum)||(pNumCtrlObj->Type==NCOT_NumBox))
	{
		if(Left != DNA_Null)//��߲���
		{
			DrawRegion.y=pNumCtrlObj->y;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Left == DNA_Normal)
		{
			DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumLeft,&DrawRegion);//��߿�
		}
		else if(Left == DNA_HiLight)
		{
			DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W-CO_NUM_FRAME_W;
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumLeftH,&DrawRegion);//��߿�
		}
		else if(Left == DNA_NormalArrow)
		{
			DrawRegion.x=pNumCtrlObj->x;
			DrawRegion.w=CO_NUM_ARROW_W;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_NumLeftArrow,&DrawRegion);//���ͷ�滭
		}
		else if(Left == DNA_HiLightArrow)
		{
			DrawRegion.x=pNumCtrlObj->x;
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumLeftArrowH,&DrawRegion);//���ͷ�滭
		}

		if(Middle != DNA_Null)//�м�򲿷�
		{
			DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumCtrlObj->y;
			DrawRegion.w=CO_NUM_MIDDLE_W;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Middle == DNA_Normal)
			Gui_FillImgArray_H(gCtrlObj_NumMiddle,pNumCtrlObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	//������
		else if(Middle == DNA_HiLight)
			Gui_FillImgArray_H(gCtrlObj_NumMiddleH,pNumCtrlObj->w-(CO_NUM_ARROW_W<<1),&DrawRegion);	//��

		if(Middle != DNA_Null)//�м����ֲ���
		{
			sprintf((void *)NumStr,"%d",pNumCtrlObj->Value);
			if(strlen((void *)NumStr)*CO_NUM_FONT_W < (pNumCtrlObj->w-(CO_NUM_ARROW_W<<1)))
				DrawRegion.x=pNumCtrlObj->x+((pNumCtrlObj->w-strlen((void *)NumStr)*CO_NUM_FONT_W)>>1);
			else //��ʾ���ȳ�������
				DrawRegion.x=pNumCtrlObj->x+CO_NUM_ARROW_W;
			DrawRegion.y=pNumCtrlObj->y+3;
			DrawRegion.w=pNumCtrlObj->w-(CO_NUM_ARROW_W<<1);
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Space=CO_NUM_FONT_SPACE;		
		}
		if(Middle == DNA_Normal)
		{
			DrawRegion.Color=CO_NUM_FONT_COLOR;
			Gui_DrawFont(CO_NUM_FONT_STYLE,NumStr,&DrawRegion);//����
		}
		else if(Middle == DNA_HiLight)
		{
			DrawRegion.Color=CO_NUM_FONT_COLOR_H;
			Gui_DrawFont(CO_NUM_FONT_STYLE,NumStr,&DrawRegion);//����
		}
		
		if(Right != DNA_Null)//�ұ߲���
		{
			DrawRegion.x=pNumCtrlObj->x+pNumCtrlObj->w-CO_NUM_ARROW_W;
			DrawRegion.y=pNumCtrlObj->y;
			DrawRegion.h=CO_NUM_H;
			DrawRegion.Color=CO_NUM_TRAN_COLOR;
		}
		if(Right == DNA_Normal)
		{
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumRight,&DrawRegion);//�ұ߿�
		}
		else if(Right == DNA_HiLight)
		{
			DrawRegion.w=CO_NUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_NumRightH,&DrawRegion);//�ұ߿�
		}
		else if(Right == DNA_NormalArrow)
		{
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumRightArrow,&DrawRegion);//�Ҽ�ͷ�滭
		}
		else if(Right == DNA_HiLightArrow)
		{
			DrawRegion.w=CO_NUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_NumRightArrowH,&DrawRegion);//�Ҽ�ͷ�滭			
		}
	}
}

//����num box�ؼ���Ϣ���С������ʾλ��
void CalculateKeyBoardPosition(NUM_CTRL_OBJ *pNumCtrlObj, GUI_REGION *pDrawReg)
{
	u16 x,y;

	x=pNumCtrlObj->x+(pNumCtrlObj->w>>1);
	y=pNumCtrlObj->y+(CO_NUM_H>>1);

	MemCpy(pDrawReg,&KeyBoardRegionTmp,sizeof(GUI_REGION));//����ģ������
	
	if(x<(LCD_WIDTH>>1))//�����
	{
		pDrawReg->x=pNumCtrlObj->x;
	}
	else//�Ҷ���
	{
		pDrawReg->x=pNumCtrlObj->x+pNumCtrlObj->w-pDrawReg->w;
	}

	if(y<(LCD_HIGHT-KeyBoardRegionTmp.h-TOUCH_REGION_MARGIN))//�����·�
	{
		pDrawReg->y=pNumCtrlObj->y+CO_NUM_H+TOUCH_REGION_MARGIN;
	}
	else //�����Ϸ�
	{
		pDrawReg->y=pNumCtrlObj->y-pDrawReg->h-TOUCH_REGION_MARGIN;
	}
}

//������Ϣ����С���̲�ע�ᶯ̬����
void DrawAndRegKeyBoard(GUI_REGION *pDrawRegion)
{
	u8 i;

	pDrawRegion->Color=KEY_BOARD_BG_COLOR;
	Gui_FillBlock(pDrawRegion);

	for(i=0;i<sizeof(NumButtonCon)/sizeof(CHAR_BUTTON_OBJ);i++)
	{
		MemCpy(&gpNcopVars->KeyBoard[i],&NumButtonCon[i],sizeof(CHAR_BUTTON_OBJ));
		gpNcopVars->KeyBoard[i].x+=pDrawRegion->x;
		gpNcopVars->KeyBoard[i].y+=pDrawRegion->y;
		Q_SetDynamicCharTch(i+1,&gpNcopVars->KeyBoard[i]);//������̬��������
	}
}
//-----------------------��ҳϵͳ����----------------------

//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	NUM_CTRL_OBJ *pNumCtrlObj=pSysParam;
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if((pNumCtrlObj->Type == NCOT_NumBox) &&(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE)) //�ļ�ϵͳû�й���
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}			
			return SM_State_OK;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�		
			gpNcopVars=Q_PageMallco(sizeof(NUM_CTRL_OBJ_PAGE_VARS));
			gpNcopVars->HandlerType=pNumCtrlObj->Type;

			if((pNumCtrlObj->Type == NCOT_NumList)||(pNumCtrlObj->Type == NCOT_NumEnum)||(pNumCtrlObj->Type == NCOT_NumBox))
			{
				if(pNumCtrlObj->Type == NCOT_NumBox)
				{
					CalculateKeyBoardPosition(pNumCtrlObj,&DrawRegion);//����С��������
					PrtScreenToBin(KEY_BOARD_LCD_BUF_PATH,DrawRegion.x,DrawRegion.y,DrawRegion.w,DrawRegion.h);
					DrawAndRegKeyBoard(&DrawRegion);
				}
				
				DrawRegion.x=pNumCtrlObj->x;
				DrawRegion.y=pNumCtrlObj->y;
				DrawRegion.w=CO_NUM_ARROW_W;
				DrawRegion.h=CO_NUM_H;
				Gui_ReadRegion16Bit(gpNcopVars->LeftDispBuf,&DrawRegion);//�����ͼ

				DrawRegion.x=pNumCtrlObj->x+pNumCtrlObj->w-CO_NUM_ARROW_W;
				DrawRegion.y=pNumCtrlObj->y;
				DrawRegion.w=CO_NUM_ARROW_W;
				DrawRegion.h=CO_NUM_H;
				Gui_ReadRegion16Bit(gpNcopVars->RightDispBuf,&DrawRegion);//�����ͼ

				DrawNumCtrlObj((void *)pNumCtrlObj,DNA_NormalArrow,DNA_HiLight,DNA_NormalArrow);//����ͼ

				MemCpy(&gpNcopVars->LeftArrowBtn,&ArrowTchTmp,sizeof(IMG_BUTTON_OBJ));//���ͷ
				gpNcopVars->LeftArrowBtn.ObjID=LeftArrowKV;
				gpNcopVars->LeftArrowBtn.x=pNumCtrlObj->x-TOUCH_REGION_MARGIN;
				gpNcopVars->LeftArrowBtn.y=pNumCtrlObj->y-TOUCH_REGION_MARGIN;
				gpNcopVars->LeftArrowBtn.w+=(TOUCH_REGION_MARGIN<<1);
				gpNcopVars->LeftArrowBtn.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(1,&gpNcopVars->LeftArrowBtn);//������̬��������

				MemCpy(&gpNcopVars->RightArrowBtn,&ArrowTchTmp,sizeof(IMG_BUTTON_OBJ));//�Ҽ�ͷ
				gpNcopVars->RightArrowBtn.ObjID=RightArrowKV;
				gpNcopVars->RightArrowBtn.x=pNumCtrlObj->x+pNumCtrlObj->w-CO_NUM_ARROW_W-TOUCH_REGION_MARGIN;
				gpNcopVars->RightArrowBtn.y=pNumCtrlObj->y-TOUCH_REGION_MARGIN;
				gpNcopVars->RightArrowBtn.w+=(TOUCH_REGION_MARGIN<<1);
				gpNcopVars->RightArrowBtn.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(2,&gpNcopVars->RightArrowBtn);//������̬��������

				MemCpy(&gpNcopVars->NumBtn,&NumBtnTmp,sizeof(IMG_BUTTON_OBJ));//���ֿ�ģ��
				gpNcopVars->NumBtn.ObjID=NumFrameKV;
				gpNcopVars->NumBtn.x=pNumCtrlObj->x+CO_NUM_ARROW_W+TOUCH_REGION_MARGIN;
				gpNcopVars->NumBtn.y=DrawRegion.y;
				gpNcopVars->NumBtn.w=pNumCtrlObj->w-(CO_NUM_ARROW_W<<1)-(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(3,&gpNcopVars->NumBtn);//������̬��������

				gpNcopVars->ObjHandler=pNumCtrlObj;//��¼�ؼ�ָ��			
				gpNcopVars->DefValue=pNumCtrlObj->Value;
			}
			else
			{
			}
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			if(gpNcopVars!=NULL)
			{
				if((gpNcopVars->HandlerType==NCOT_NumList)||(gpNcopVars->HandlerType==NCOT_NumEnum)||(gpNcopVars->HandlerType==NCOT_NumBox))
				{
					pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
					
					DrawRegion.x=pNumCtrlObj->x;
					DrawRegion.y=pNumCtrlObj->y;
					DrawRegion.w=CO_NUM_ARROW_W;
					DrawRegion.h=CO_NUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpNcopVars->LeftDispBuf,&DrawRegion);//��ԭ

					DrawRegion.x=pNumCtrlObj->x+pNumCtrlObj->w-CO_NUM_ARROW_W;
					DrawRegion.y=pNumCtrlObj->y;
					DrawRegion.w=CO_NUM_ARROW_W;
					DrawRegion.h=CO_NUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpNcopVars->RightDispBuf,&DrawRegion);//��ԭ

					if((gpNcopVars->HandlerType==NCOT_NumBox)//num boxҪ���ֵ�ķ�Χ
						&&((pNumCtrlObj->Value<pNumCtrlObj->Min)||(pNumCtrlObj->Value>pNumCtrlObj->Max)) )
						pNumCtrlObj->Value=gpNcopVars->DefValue;//�����Ϸ�Χ�򷵻�ԭֵ

					//if(gpNcopVars->DefValue == pNumCtrlObj->Value)
						DrawNumCtrlObj((void *)pNumCtrlObj,DNA_Normal,DNA_Normal,DNA_Normal);//δ�ı�ԭ��ֵ
					//else
					//	DrawNumCtrlObj((void *)pNumCtrlObj,DNA_HiLight,DNA_HiLight,DNA_HiLight);//�ı���ԭ��ֵ

					if(gpNcopVars->HandlerType==NCOT_NumBox)//num boxҪ�ָ�С��������
					{
						pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
						CalculateKeyBoardPosition(pNumCtrlObj,&DrawRegion);//����С��������
						Gui_DrawImgBin(KEY_BOARD_LCD_BUF_PATH,&DrawRegion);
					}
				}
				else
				{
				}

				Q_PageFree(gpNcopVars);
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
				NUM_CTRL_OBJ *pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
				DrawNumCtrlObj(pNumCtrlObj,DNA_HiLightArrow,DNA_Null,DNA_Null);//����ͼ
			}
			else
			{
				NUM_BOX_OBJ *pNumBox=(void *)gpNcopVars->ObjHandler;
				NUM_LIST_OBJ *pNumList=(void *)gpNcopVars->ObjHandler;
				NUM_ENUM_OBJ *pNumEnum=(void *)gpNcopVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pNumBox->Type == NCOT_NumBox)
					{
						if((pNumBox->Value-1)<pNumBox->Min)
							pNumBox->Value=pNumBox->Max;
						else
							pNumBox->Value-=1;
					}
					else if(pNumList->Type == NCOT_NumList)
					{
						if((pNumList->Value-pNumList->Step)>=pNumList->Min)
							pNumList->Value-=pNumList->Step;
						else pNumList->Value=pNumList->Max;
					}
					else if(pNumEnum->Type == NCOT_NumEnum)
					{
						if((pNumEnum->Idx-1)>=0) pNumEnum->Idx--;
						else pNumEnum->Idx=(pNumEnum->Total-1);
						pNumEnum->Value=pNumEnum->pEnumList[pNumEnum->Idx];			
					}
				}
				DrawNumCtrlObj((void *)gpNcopVars->ObjHandler,DNA_NormalArrow,DNA_HiLight,DNA_Null);//����ͼ
			}
			break;
		case RightArrowKV://+
			if(InEvent == Tch_Press)
			{
				NUM_CTRL_OBJ *pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
				DrawNumCtrlObj(pNumCtrlObj,DNA_Null,DNA_Null,DNA_HiLightArrow);//����ͼ
			}
			else
			{					
				NUM_BOX_OBJ *pNumBox=(void *)gpNcopVars->ObjHandler;
				NUM_LIST_OBJ *pNumList=(void *)gpNcopVars->ObjHandler;
				NUM_ENUM_OBJ *pNumEnum=(void *)gpNcopVars->ObjHandler;
				if(InEvent == Tch_Release)
				{
					if(pNumBox->Type == NCOT_NumBox)
					{
						if((pNumBox->Value+1)>pNumBox->Max)
							pNumBox->Value=pNumBox->Min;
						else
							pNumBox->Value+=1;
					}
					else if(pNumList->Type == NCOT_NumList)
					{
						if((pNumList->Value+pNumList->Step)<=pNumList->Max)
							pNumList->Value+=pNumList->Step;
						else pNumList->Value=pNumList->Min;
					}
					else if(pNumEnum->Type == NCOT_NumEnum)
					{
						if((pNumEnum->Idx+1)<pNumEnum->Num) pNumEnum->Idx++;
						else pNumEnum->Idx=0;
						pNumEnum->Value=pNumEnum->pEnumList[pNumEnum->Idx];		
					}
				}
				DrawNumCtrlObj((void *)gpNcopVars->ObjHandler,DNA_Null,DNA_HiLight,DNA_NormalArrow);//����ͼ
			}
			break;
		case Key0KV:case Key1KV:case Key2KV:case Key3KV:case Key4KV:
		case Key5KV:case Key6KV:case Key7KV:case Key8KV:case Key9KV:
			{
				NUM_CTRL_OBJ *pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
				Key-=Key0KV;
				pNumCtrlObj->Value=pNumCtrlObj->Value*10+Key;
				DrawNumCtrlObj(pNumCtrlObj,DNA_Null,DNA_HiLight,DNA_Null);//����ͼ
			}
			break;
		case KeyMinusKv://+-
			{
				NUM_CTRL_OBJ *pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
				pNumCtrlObj->Value=0-pNumCtrlObj->Value;
				DrawNumCtrlObj(pNumCtrlObj,DNA_Null,DNA_HiLight,DNA_Null);//����ͼ
			}
			break;
		case KeyDotKV://.
			break;
		case KeyCancleKV://X
			{
				NUM_CTRL_OBJ *pNumCtrlObj=(void *)gpNcopVars->ObjHandler;
				pNumCtrlObj->Value/=10;
				DrawNumCtrlObj(pNumCtrlObj,DNA_Null,DNA_HiLight,DNA_Null);//����ͼ
			}
			break;
		case KeyOkKV://O
		case NumFrameKV://������屾���ʾ����
			Q_GotoPage(SubPageReturn,"",0,gpNcopVars->ObjHandler);//����ǰһ��ҳ��
			break;
			
		default:
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}




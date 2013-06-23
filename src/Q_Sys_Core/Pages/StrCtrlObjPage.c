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

//函数声明
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------本页系统变量及声明-----------------------
//定义页面按键需要用到的枚举，类似于有序唯一的宏定义
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//后面的硬件版本将支持更多外部中断按键或者无线键盘，
	//所以触摸键值从USER_KEY_VALUE_START开始，将前面的键值都留下来
	LeftArrowKV=USER_KEY_VALUE_START,
	RightArrowKV,
	StrFrameKV,
	
}StrCtrlObjPage_OID;

//定义页面或者应用的属性集
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

//-----------------------本页自定义变量声明-----------------------
//功能选项
#define TOUCH_REGION_MARGIN 4//margin可增加可点击区域范围

//动态按键模板
static const IMG_BUTTON_OBJ ArrowTchTmp={"",0,PrsMsk|RelMsk|ReVMsk,0,0,CO_STR_ENUM_ARROW_W,CO_STR_ENUM_H,0,0,"",FatColor(NO_TRANS)};
static const IMG_BUTTON_OBJ StrTchTmp={"",0,RelMsk,0,0,0,CO_STR_ENUM_H,0,0,"",FatColor(NO_TRANS)};

typedef struct{
	STR_CTRL_OBJ_TYPE HandlerType;//控件类型
	void *ObjHandler;//控件实体指针
	u16 LeftDispBuf[CO_STR_ENUM_ARROW_W*CO_STR_ENUM_H];//控件左箭头缓存
	u16 RightDispBuf[CO_STR_ENUM_ARROW_W*CO_STR_ENUM_H];//控件右箭头缓存
	IMG_BUTTON_OBJ LeftArrowBtn;//左箭头动态按键实体
	IMG_BUTTON_OBJ RightArrowBtn;//右箭头动态按键实体
	IMG_BUTTON_OBJ StrBtn;//中间数字框动态按键实体
}STR_CTRL_OBJ_PAGE_VARS;
static STR_CTRL_OBJ_PAGE_VARS *gpScopVars;//只需要定义一个指针，减少全局变量的使用

//-----------------------本页自定义函数-----------------------
typedef enum{
	DSA_Null=0,//维持原样
	DSA_Normal,//普通
	DSA_HiLight,//高亮
	DSA_NormalArrow,//普通箭头
	DSA_HiLightArrow//高亮箭头
}DRAW_SE_ACT;
//根据要求绘制StrEnum控件的框体和左右箭头及文字
static void DrawStrCtrlObj(STR_CTRL_OBJ *pStrCtrlObj,DRAW_SE_ACT Left,DRAW_SE_ACT Middle,DRAW_SE_ACT Right)
{
	GUI_REGION DrawRegion;
	u8 StrBuf[32];
	
	if(pStrCtrlObj->Type==SCOT_StrEnum)
	{
		if(Left != DSA_Null)//左边部分
		{
			DrawRegion.y=pStrCtrlObj->y;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
		}
		if(Left == DSA_Normal)
		{
			DrawRegion.x=pStrCtrlObj->x+CO_STR_ENUM_ARROW_W-CO_STR_ENUM_FRAME_W;
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeft,&DrawRegion);//左边框
		}
		else if(Left == DSA_HiLight)
		{
			DrawRegion.x=pStrCtrlObj->x+CO_STR_ENUM_ARROW_W-CO_STR_ENUM_FRAME_W;
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeftH,&DrawRegion);//左边框
		}
		else if(Left == DSA_NormalArrow)
		{
			DrawRegion.x=pStrCtrlObj->x;
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeftArrow,&DrawRegion);//左箭头绘画
		}
		else if(Left == DSA_HiLightArrow)
		{
			DrawRegion.x=pStrCtrlObj->x;
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumLeftArrowH,&DrawRegion);//左箭头绘画
		}

		if(Middle != DSA_Null)//中间框部分
		{
			DrawRegion.x=pStrCtrlObj->x+CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrCtrlObj->y;
			DrawRegion.w=CO_STR_ENUM_MIDDLE_W;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
		}
		if(Middle == DSA_Normal)
			Gui_FillImgArray_H(gCtrlObj_StrEnumMiddle,pStrCtrlObj->w-(CO_STR_ENUM_ARROW_W<<1),&DrawRegion);	//框
		else if(Middle == DSA_HiLight)
			Gui_FillImgArray_H(gCtrlObj_StrEnumMiddleH,pStrCtrlObj->w-(CO_STR_ENUM_ARROW_W<<1),&DrawRegion);	//高亮框

		if(Middle != DSA_Null)//中间文字部分
		{
			STR_ENUM_OBJ *pStrEnum=(STR_ENUM_OBJ *)pStrCtrlObj;
			sprintf((void *)StrBuf,"%s",&pStrEnum->pStrEnumBuf[pStrEnum->Idx+1]);//字符串
			if(strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W < (pStrEnum->w-(CO_STR_ENUM_ARROW_W<<1)))
				DrawRegion.x=pStrEnum->x+((pStrEnum->w-strlen((void *)StrBuf)*CO_STR_ENUM_FONT_W)>>1);
			else //显示长度超出方框
				DrawRegion.x=pStrEnum->x+CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrEnum->y+3;
			DrawRegion.w=pStrEnum->w-(CO_STR_ENUM_ARROW_W<<1);
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Space=CO_STR_ENUM_FONT_SPACE;
		}
		if(Middle == DSA_Normal)
		{
			DrawRegion.Color=CO_STR_ENUM_FONT_COLOR;
			Gui_DrawFont(CO_STR_ENUM_FONT_STYLE,StrBuf,&DrawRegion);//文字
		}
		else if(Middle == DSA_HiLight)
		{
			DrawRegion.Color=CO_STR_ENUM_FONT_COLOR_H;
			Gui_DrawFont(CO_STR_ENUM_FONT_STYLE,StrBuf,&DrawRegion);//文字
		}
		
		if(Right != DSA_Null)//右边部分
		{
			DrawRegion.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_STR_ENUM_ARROW_W;
			DrawRegion.y=pStrCtrlObj->y;
			DrawRegion.h=CO_STR_ENUM_H;
			DrawRegion.Color=CO_STR_ENUM_TRAN_COLOR;
		}
		if(Right == DSA_Normal)
		{
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRight,&DrawRegion);//右边框
		}
		else if(Right == DSA_HiLight)
		{
			DrawRegion.w=CO_STR_ENUM_FRAME_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRightH,&DrawRegion);//右边框
		}
		else if(Right == DSA_NormalArrow)
		{
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRightArrow,&DrawRegion);//右箭头绘画
		}
		else if(Right == DSA_HiLightArrow)
		{
			DrawRegion.w=CO_STR_ENUM_ARROW_W;
			Gui_DrawImgArray(gCtrlObj_StrEnumRightArrowH,&DrawRegion);//右箭头绘画			
		}
	}
	else if(pStrCtrlObj->Type==SCOT_StrBox)
	{

	}
}

//-----------------------本页系统函数----------------------
//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	STR_CTRL_OBJ *pStrCtrlObj=pSysParam;
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
#if 1
			if((pStrCtrlObj->Type == SCOT_StrBox) &&(Q_DB_GetStatus(Status_FsInitFinish,NULL)==FALSE)) //文件系统没有挂载
			{
				Debug("File system not mount!!!\n\r");
				return SM_State_Faile|SM_NoGoto;
			}	
#endif
			return SM_State_OK;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件		
			gpScopVars=Q_PageMallco(sizeof(STR_CTRL_OBJ_PAGE_VARS));
			gpScopVars->HandlerType=pStrCtrlObj->Type;

			if((pStrCtrlObj->Type == SCOT_StrEnum)||(pStrCtrlObj->Type == SCOT_StrBox))
			{
				if(pStrCtrlObj->Type == SCOT_StrBox)
				{
					//CalculateKeyBoardPosition(pStrCtrlObj,&DrawRegion);//计算小键盘区域
					//PrtScreenToBin(KEY_BOARD_LCD_BUF_PATH,DrawRegion.x,DrawRegion.y,DrawRegion.w,DrawRegion.h);
					//DrawAndRegKeyBoard(&DrawRegion);
				}
				
				DrawRegion.x=pStrCtrlObj->x;
				DrawRegion.y=pStrCtrlObj->y;
				DrawRegion.w=CO_STR_ENUM_ARROW_W;
				DrawRegion.h=CO_STR_ENUM_H;
				Gui_ReadRegion16Bit(gpScopVars->LeftDispBuf,&DrawRegion);//保存旧图

				DrawRegion.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_STR_ENUM_ARROW_W;
				DrawRegion.y=pStrCtrlObj->y;
				DrawRegion.w=CO_STR_ENUM_ARROW_W;
				DrawRegion.h=CO_STR_ENUM_H;
				Gui_ReadRegion16Bit(gpScopVars->RightDispBuf,&DrawRegion);//保存旧图

				DrawStrCtrlObj((void *)pStrCtrlObj,DSA_NormalArrow,DSA_HiLight,DSA_NormalArrow);//画新图

				MemCpy(&gpScopVars->LeftArrowBtn,&ArrowTchTmp,sizeof(IMG_BUTTON_OBJ));//左箭头
				gpScopVars->LeftArrowBtn.ObjID=LeftArrowKV;
				gpScopVars->LeftArrowBtn.x=pStrCtrlObj->x-TOUCH_REGION_MARGIN;
				gpScopVars->LeftArrowBtn.y=pStrCtrlObj->y-TOUCH_REGION_MARGIN;
				gpScopVars->LeftArrowBtn.w+=(TOUCH_REGION_MARGIN<<1);
				gpScopVars->LeftArrowBtn.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(1,&gpScopVars->LeftArrowBtn);//建立动态按键区域

				MemCpy(&gpScopVars->RightArrowBtn,&ArrowTchTmp,sizeof(IMG_BUTTON_OBJ));//右箭头
				gpScopVars->RightArrowBtn.ObjID=RightArrowKV;
				gpScopVars->RightArrowBtn.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_NUM_ARROW_W-TOUCH_REGION_MARGIN;
				gpScopVars->RightArrowBtn.y=pStrCtrlObj->y-TOUCH_REGION_MARGIN;
				gpScopVars->RightArrowBtn.w+=(TOUCH_REGION_MARGIN<<1);
				gpScopVars->RightArrowBtn.h+=(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(2,&gpScopVars->RightArrowBtn);//建立动态按键区域

				MemCpy(&gpScopVars->StrBtn,&StrTchTmp,sizeof(IMG_BUTTON_OBJ));//文字框模板
				gpScopVars->StrBtn.ObjID=StrFrameKV;
				gpScopVars->StrBtn.x=pStrCtrlObj->x+CO_NUM_ARROW_W+TOUCH_REGION_MARGIN;
				gpScopVars->StrBtn.y=DrawRegion.y;
				gpScopVars->StrBtn.w=pStrCtrlObj->w-(CO_NUM_ARROW_W<<1)-(TOUCH_REGION_MARGIN<<1);
				Q_SetDynamicImgTch(3,&gpScopVars->StrBtn);//建立动态按键区域

				gpScopVars->ObjHandler=pStrCtrlObj;//记录控件指针			
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
					Gui_DrawImgArray((void *)gpScopVars->LeftDispBuf,&DrawRegion);//还原

					DrawRegion.x=pStrCtrlObj->x+pStrCtrlObj->w-CO_STR_ENUM_ARROW_W;
					DrawRegion.y=pStrCtrlObj->y;
					DrawRegion.w=CO_STR_ENUM_ARROW_W;
					DrawRegion.h=CO_STR_ENUM_H;
					DrawRegion.Color=FatColor(NO_TRANS);
					Gui_DrawImgArray((void *)gpScopVars->RightDispBuf,&DrawRegion);//还原

					DrawStrCtrlObj((void *)pStrCtrlObj,DSA_Normal,DSA_Normal,DSA_Normal);//未改变原有值
					
					if(gpScopVars->HandlerType==SCOT_StrBox)//num box要恢复小键盘区域
					{
						//pStrCtrlObj=(void *)gpScopVars->ObjHandler;
						//CalculateKeyBoardPosition(pStrCtrlObj,&DrawRegion);//计算小键盘区域
						//Gui_DrawImgBin(KEY_BOARD_LCD_BUF_PATH,&DrawRegion);
					}
				}
				else
				{
				}

				Q_PageFree(gpScopVars);
			}
			return SM_NoPopReturn|SM_State_OK;//不允许主页面pop page return事件
		default:
			//需要响应的事件未定义
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
			if((IntParam>>16)==1)//串口1
			{
				Q_Sh_CmdHandler(IntParam&0xffff,pParam);
			}
			break;
	}

	return 0;
}

//当使用者按下本页TouchRegionSet里定义的按键时，会触发这个函数里的对应事件
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{		
	//GUI_REGION DrawRegion;
	
	switch(Key)
	{
		case LeftArrowKV://-
			if(InEvent == Tch_Press)
			{
				STR_CTRL_OBJ *pStrCtrlObj=(void *)gpScopVars->ObjHandler;
				DrawStrCtrlObj(pStrCtrlObj,DSA_HiLightArrow,DSA_Null,DSA_Null);//画新图
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
						if(pStrEnum->Idx == 0)//如果当前是第一个，跳到最后一个的末尾
							pStrEnum->Idx=pStrEnum->Size-1;
						else
							pStrEnum->Idx-=2;//否则跳到前一个字符串的末尾
						
						while(pStrEnum->Idx>0)//找字符串开头
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
				DrawStrCtrlObj((void *)gpScopVars->ObjHandler,DSA_NormalArrow,DSA_HiLight,DSA_Null);//画新图
			}
			break;
		case RightArrowKV://+
			if(InEvent == Tch_Press)
			{
				STR_CTRL_OBJ *pStrCtrlObj=(void *)gpScopVars->ObjHandler;
				DrawStrCtrlObj(pStrCtrlObj,DSA_Null,DSA_Null,DSA_HiLightArrow);//画新图
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
						while(pStrEnum->pStrEnumBuf[++pStrEnum->Idx]);//找到当前字符串的末尾结束符

						if(pStrEnum->Idx>=pStrEnum->Size)//超出了，则跳到第一个
							pStrEnum->Idx=0;
						else 
							pStrEnum->Idx++;//到下一个			
					}
				}
				DrawStrCtrlObj((void *)gpScopVars->ObjHandler,DSA_Null,DSA_HiLight,DSA_NormalArrow);//画新图
			}
			break;
		case StrFrameKV://点击窗体本身表示返回
			Q_GotoPage(SubPageReturn,"",0,gpScopVars->ObjHandler);//返回前一个页面
			break;
			
		default:
			//需要响应的事件未定义
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}




/**************** Q-SYS *******************
 * PageName : TouchCheckPage
 * Author : ChenYu
 * Version : 2.0
 * Base Q-Sys Version : 2.0
 * Description : touch check
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/

/*
触摸屏校验2.0的修改如下：
1.修改了一处bug
2.将动态按键改为了静态按键并调整了位置
3.增加了强行退出的功能（点击home键）
4.增加了校验结果的显示
5.抽出了一组共八个宏方便用户定义校验范围
*/

#include "User.h"
#include "TouchCheckPage.h"
//修改以下8个宏可设置最大校验区间
#define    X_RATIO_MIN  0.9	 
#define    X_RATIO_MAX  1.1
#define    Y_RATIO_MIN  0.9
#define    Y_RATIO_MAX  1.1
#define    X_OFFSET_MIN (-7)
#define    X_OFFSET_MAX 7
#define    Y_OFFSET_MIN (-7)
#define    Y_OFFSET_MAX 7
#define    Bg_X        0
#define    Bg_Y        0
#define    Bg_W        240
#define    Bg_H        320
#define	   LeftUp_X	   20
#define	   LeftUp_Y	   20
#define	   RightUp_X   220
#define	   RightUp_Y   20
#define	   LeftDown_X  20
#define	   LeftDown_Y  300
#define	   RightDown_X 220
#define	   RightDown_Y 300
#define	   MsgBox_X		37
#define	   MsgBox_Y		100
#define	   MsgBox_W		168
#define	   MsgBox_H		16
extern float X_RATIO;
extern float Y_RATIO;
extern s32 X_OFFSET;
extern s32 Y_OFFSET;
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
enum{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,
	LU,
	LD,
	RU,
	RD,
	MsgBox,
    HomeKV,
    MailKV,
    RingKV,
    CallKV
};
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	{"",LU,RelMsk,LeftUp_X-19,LeftUp_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
    {"",RU,RelMsk,RightUp_X-19,RightUp_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
    {"",LD,RelMsk,LeftDown_X-19,LeftDown_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
    {"",RD,RelMsk,RightDown_X-19,RightDown_Y-19,38,38,0,0,"",FatColor(NO_TRANS)},
	{"",MsgBox,RelMsk,MsgBox_X,MsgBox_Y,MsgBox_W,MsgBox_H,0,0,"",FatColor(NO_TRANS)},
    {"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
    {"",MailKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
    {"",RingKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
    {"",CallKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
};
const PAGE_ATTRIBUTE TouchCheckPage={
    "TouchCheckPage",
    "ChenYu",
    "touch check",
    NORMAL_PAGE,
    0,
    {
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
    ImgButtonCon,
    NULL,
    SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyRelease),
    ButtonHandler,
};
typedef struct{
	bool lu_flag;
	bool ru_flag;
	bool ld_flag;
	bool rd_flag;
	u32 lu_x;
	u32 lu_y;
	u32 ru_x;
	u32 ru_y;
	u32 ld_x;
	u32 ld_y;
	u32 rd_x;
	u32 rd_y;
	float x_ratio;
	float y_ratio;
	s32 x_offset;
	s32 y_offset;
	GUI_REGION DrawRegion;
	bool OkFlag;
}TOUCH_CHK_PAGE_VARS;
static TOUCH_CHK_PAGE_VARS *gpTcpVars;
void Init_TouchChkPageVars(int IntParam,void *pParam){
	X_RATIO=1.0;
	Y_RATIO=1.0;
	X_OFFSET=0.0;
	Y_OFFSET=0.0;
	gpTcpVars->lu_flag=FALSE;
	gpTcpVars->ru_flag=FALSE;
	gpTcpVars->ld_flag=FALSE;
	gpTcpVars->rd_flag=FALSE;
	gpTcpVars->lu_x=LeftUp_X;
	gpTcpVars->lu_y=LeftUp_Y;
	gpTcpVars->ru_x=RightUp_X;
	gpTcpVars->ru_y=RightUp_Y;
	gpTcpVars->ld_x=LeftDown_X;
	gpTcpVars->ld_y=LeftDown_Y;
	gpTcpVars->rd_x=RightDown_X;
	gpTcpVars->rd_y=RightDown_Y;
	gpTcpVars->x_ratio =1;
	gpTcpVars->y_ratio =1;
	gpTcpVars->x_offset=0;
	gpTcpVars->y_offset=0;
	gpTcpVars->DrawRegion.x=MsgBox_X;
	gpTcpVars->DrawRegion.y=MsgBox_Y;
	gpTcpVars->DrawRegion.w=MsgBox_W;
	gpTcpVars->DrawRegion.h=MsgBox_H;
	gpTcpVars->DrawRegion.Color=FatColor(0xffffff);
	gpTcpVars->DrawRegion.Space=0x50;
	gpTcpVars->OkFlag=FALSE;
}
static void Draw_Bg(void){
    GUI_REGION DrawRegion;
    DrawRegion.x=Bg_X;
    DrawRegion.y=Bg_Y;
    DrawRegion.w=Bg_W;
    DrawRegion.h=Bg_H;
    DrawRegion.Color=FatColor(0x5a5a5a);
    Gui_FillBlock(&DrawRegion);
}
static void Draw_Focus(u32 x,u32 y,bool IfClick){
	if(IfClick==FALSE){
		Gui_DrawLine(x-8, y-8,x+8, y+8,FatColor(0x00ff00));
		Gui_DrawLine(x-8, y+8,x+8, y-8,FatColor(0x00ff00));
	}
	else{
		Gui_DrawLine(x-8, y-8,x+8, y+8,FatColor(0xff0000));
		Gui_DrawLine(x-8, y+8,x+8, y-8,FatColor(0xff0000));
	}
}

static void Draw_Result(void){
	GUI_REGION DrawRegion;
	char charbuf[32];
	DrawRegion.x=MsgBox_X;
    DrawRegion.y=MsgBox_Y+MsgBox_H+32;
    DrawRegion.w=MsgBox_W;
    DrawRegion.h=MsgBox_H;
	DrawRegion.Color=FatColor(0x0000ff);
	DrawRegion.Space=0x00;
	sprintf(charbuf,"X_RATIO =%.3f\n\r",gpTcpVars->x_ratio);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
	DrawRegion.y+=18;
	sprintf(charbuf,"Y_RATIO =%.3f\n\r",gpTcpVars->y_ratio);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
	DrawRegion.y+=18;
	sprintf(charbuf,"X_OFFSET=%d\n\r",gpTcpVars->x_offset);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
	DrawRegion.y+=18;
	sprintf(charbuf,"Y_OFFSET=%d\n\r",gpTcpVars->y_offset);
	Gui_DrawFont(GBK16_FONT,(void *)charbuf,&DrawRegion);
}

static bool Calculate(void){
	float x_ratio1;
	float x_ratio2;
	float y_ratio1;
	float y_ratio2;
	s32   x_offset1;
	s32   x_offset2;
	s32   x_offset3;
	s32   x_offset4;
	s32   y_offset1;
	s32   y_offset2;
	s32   y_offset3;
	s32   y_offset4;
	
	x_ratio1 =(float)(gpTcpVars->ru_x-gpTcpVars->lu_x)/(float)(RightUp_X-LeftUp_X);
	x_ratio2 =(float)(gpTcpVars->rd_x-gpTcpVars->ld_x)/(float)(RightDown_X-LeftDown_X);
	
	y_ratio1 =(float)(gpTcpVars->ld_y-gpTcpVars->lu_y)/(float)(LeftDown_Y-LeftUp_Y);
	y_ratio2 =(float)(gpTcpVars->rd_y-gpTcpVars->ru_y)/(float)(RightDown_Y-RightUp_Y);
	
	x_offset1=LeftUp_X-(s32)(gpTcpVars->x_ratio*(float)gpTcpVars->lu_x);
	x_offset2=LeftDown_X-(s32)(gpTcpVars->x_ratio*(float)gpTcpVars->ld_x);
	x_offset3=RightUp_X-(s32)(gpTcpVars->x_ratio*(float)gpTcpVars->ru_x);
	x_offset4=RightDown_X-(s32)(gpTcpVars->x_ratio*(float)gpTcpVars->rd_x);
	
	y_offset1=LeftUp_Y-(s32)(gpTcpVars->y_ratio*(float)gpTcpVars->lu_y);
	y_offset2=LeftDown_Y-(s32)(gpTcpVars->y_ratio*(float)gpTcpVars->ld_y);
	y_offset3=RightUp_Y-(s32)(gpTcpVars->y_ratio*(float)gpTcpVars->ru_y);
	y_offset4=RightDown_Y-(s32)(gpTcpVars->y_ratio*(float)gpTcpVars->rd_y);
	
	gpTcpVars->x_ratio=(x_ratio1+x_ratio2)/(float)2;
	gpTcpVars->y_ratio=(y_ratio1+y_ratio2)/(float)2;
	gpTcpVars->x_offset=(x_offset1+x_offset2+x_offset3+x_offset4)/4;
	gpTcpVars->y_offset=(y_offset1+y_offset2+y_offset3+y_offset4)/4;
	
	if( gpTcpVars->x_ratio>X_RATIO_MAX || gpTcpVars->x_ratio<X_RATIO_MIN || gpTcpVars->y_ratio>Y_RATIO_MAX || gpTcpVars->y_ratio<Y_RATIO_MIN )
		return FALSE;
		
	if( gpTcpVars->x_offset>X_OFFSET_MAX || gpTcpVars->x_offset<X_OFFSET_MIN || gpTcpVars->y_offset>Y_OFFSET_MAX || gpTcpVars->y_offset<Y_OFFSET_MIN )
		return FALSE;
	
	return TRUE;
}
static void JudgeAndCal(void){
	gpTcpVars->DrawRegion.Color=FatColor(0x5a5a5a);
	Gui_FillBlock(&gpTcpVars->DrawRegion);
	gpTcpVars->DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK16_FONT,"   点此重校",&gpTcpVars->DrawRegion);
	if( gpTcpVars->lu_flag==TRUE && gpTcpVars->ld_flag==TRUE && gpTcpVars->ru_flag==TRUE && gpTcpVars->rd_flag==TRUE ){
		if(Calculate()==TRUE){
			gpTcpVars->DrawRegion.Color=FatColor(0x5a5a5a);
			Gui_FillBlock(&gpTcpVars->DrawRegion);
			gpTcpVars->DrawRegion.Color=FatColor(0x00ff00);
			Gui_DrawFont(GBK16_FONT,"校验完成点此退出",&gpTcpVars->DrawRegion);
			Draw_Result();
			gpTcpVars->OkFlag=TRUE;
		}else{
			gpTcpVars->DrawRegion.Color=FatColor(0x5a5a5a);
			Gui_FillBlock(&gpTcpVars->DrawRegion);
			gpTcpVars->DrawRegion.Color=FatColor(0x00ff00);
			Gui_DrawFont(GBK16_FONT,"校验失败点此重校",&gpTcpVars->DrawRegion);
			Draw_Result();
			gpTcpVars->OkFlag=FALSE;
		}
	}
}
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent,int IntParam,void *pSysParam){
    switch(SysEvent){
        case Sys_PreGotoPage:{
        }break;
        case Sys_PageInit:{
			gpTcpVars=(TOUCH_CHK_PAGE_VARS *)Q_PageMallco(sizeof(TOUCH_CHK_PAGE_VARS));			
			 if(gpTcpVars==0){
				Q_ErrorStopScreen("gpTcpVars malloc fail !\n\r");
			 }
			Init_TouchChkPageVars(IntParam,pSysParam);
			Draw_Bg();
			Draw_Focus(LeftUp_X,LeftUp_Y,FALSE);
			Draw_Focus(LeftDown_X,LeftDown_Y,FALSE);
			Draw_Focus(RightUp_X,RightUp_Y,FALSE);
			Draw_Focus(RightDown_X,RightDown_Y,FALSE);
			Gui_DrawFont(GBK16_FONT,"请点击十字叉中心",&gpTcpVars->DrawRegion);
        }break;
        case Sys_SubPageReturn:{
        }break;
        case Sys_PageClean:{
			Q_PageFree(gpTcpVars);
        }break;
    }
    return 0;
}

static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
					PrtScreen();
					break;
				case ExtiKeyUp:
					break;
				case ExtiKeyDown:
					break; 
			}break;
	}
	
	return 0;
}

static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo){
    switch(Key){
		case LU:{
			if(gpTcpVars->lu_flag==FALSE){
				gpTcpVars->lu_flag=TRUE;
				Draw_Focus(LeftUp_X,LeftUp_Y,TRUE);
				gpTcpVars->lu_x=pTouchInfo->x;
				gpTcpVars->lu_y=pTouchInfo->y;				
				JudgeAndCal();
			}
		}break;
		case LD:{
			if(gpTcpVars->ld_flag==FALSE){
				gpTcpVars->ld_flag=TRUE;
				Draw_Focus(LeftDown_X,LeftDown_Y,TRUE);
				gpTcpVars->ld_x=pTouchInfo->x;
				gpTcpVars->ld_y=pTouchInfo->y;
				JudgeAndCal();
			}
		}break;
		case RU:{
			if(gpTcpVars->ru_flag==FALSE){
				gpTcpVars->ru_flag=TRUE;
				Draw_Focus(RightUp_X,RightUp_Y,TRUE);
				gpTcpVars->ru_x=pTouchInfo->x;
				gpTcpVars->ru_y=pTouchInfo->y;
				JudgeAndCal();
			}
		}break;
		case RD:{
			if(gpTcpVars->rd_flag==FALSE){
				gpTcpVars->rd_flag=TRUE;
				Draw_Focus(RightDown_X,RightDown_Y,TRUE);
				gpTcpVars->rd_x=pTouchInfo->x;
				gpTcpVars->rd_y=pTouchInfo->y;
				JudgeAndCal();
			}
		}break;
		case MsgBox:{
			if(gpTcpVars->OkFlag==TRUE){
				Debug("gpTcpVars->lu_x=%d\n\r",gpTcpVars->lu_x);
				Debug("gpTcpVars->lu_y=%d\n\r",gpTcpVars->lu_y);
				Debug("gpTcpVars->ld_x=%d\n\r",gpTcpVars->ld_x);
				Debug("gpTcpVars->ld_y=%d\n\r",gpTcpVars->ld_y);
				Debug("gpTcpVars->ru_x=%d\n\r",gpTcpVars->ru_x);
				Debug("gpTcpVars->ru_y=%d\n\r",gpTcpVars->ru_y);
				Debug("gpTcpVars->rd_x=%d\n\r",gpTcpVars->rd_x);
				Debug("gpTcpVars->rd_y=%d\n\r",gpTcpVars->rd_y);
				X_RATIO=gpTcpVars->x_ratio;
				Y_RATIO=gpTcpVars->y_ratio;
				X_OFFSET=gpTcpVars->x_offset;
				Y_OFFSET=gpTcpVars->y_offset;
				Debug("X_RATIO=%.3f\n\r",X_RATIO);
				Debug("Y_RATIO=%.3f\n\r",Y_RATIO);
				Debug("X_OFFSET=%d\n\r",X_OFFSET);
				Debug("Y_OFFSET=%d\n\r",Y_OFFSET);
				Q_GotoPage(GotoNewPage,"AppListPage",0,NULL);
			}
			else{
				Init_TouchChkPageVars(0,NULL);
				Draw_Bg();
				Draw_Focus(LeftUp_X,LeftUp_Y,FALSE);
				Draw_Focus(LeftDown_X,LeftDown_Y,FALSE);
				Draw_Focus(RightUp_X,RightUp_Y,FALSE);
				Draw_Focus(RightDown_X,RightDown_Y,FALSE);
				Gui_DrawFont(GBK16_FONT,"请点击十字叉中心",&gpTcpVars->DrawRegion);
			}
		}break;
        case HomeKV:{
			Q_GotoPage(GotoNewPage,"AppListPage",0,NULL);	
        }break;
        case MailKV:{
        }break;
        case RingKV:{
        }break;
        case CallKV:{
        }break;
    }
    return 0;
}

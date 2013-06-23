/**************** Q-SYS *******************
 * PageName : TestPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "TestPage.h"

//函数声明
static SYS_MSG SystemHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);//系统事件处理函数
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);//外设事件处理函数
static CO_MSG ButtonHandler(u8 OID,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);//按钮控件处理函数
static CO_MSG YesNoHandler(u8 OID,bool NowValue);//yes no控件处理函数
static CO_MSG NumCtrlObjHandler(u8 OID,s32 Value,void *pNumCtrlObj);//num控件处理函数
static CO_MSG StrCtrlObjHandler(u8 OID,u8 StrID,u8 *Str,void *pStrCtrlObj);//str控件处理函数

//-----------------------本页系统变量及声明-----------------------
//定义页面按键需要用到的枚举，类似于有序唯一的宏定义
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//后面的硬件版本将支持更多外部中断按键或者无线键盘，
	//所以触摸键值从USER_KEY_VALUE_START开始，将前面的键值都留下来
	BackKV=USER_KEY_VALUE_START,
	LeftArrowKV,
	DotKV,
	RightArrowKV,
	DoneKV,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}TestPage_OID;

//定义页面或应用的触摸区域集，相当于定义按键
//支持的最大触摸区域个数为MAX_TOUCH_REGION_NUM
//系统显示和触摸的所有坐标系均以屏幕左上角为原点(x 0,y 0)，右下角为(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Back",	BackKV,RelMsk|PathMsk,3,287,54,31,0,0,"Common/Btn/Back",FatColor(NO_TRANS)},
	{"<<",		LeftArrowKV,RelMsk|PathMsk,65,287,39,31,0,0,"Common/Btn/LeftArr",FatColor(NO_TRANS)},
	//{"Dot",		DotKV,RelMsk|PathMsk,109,292,22,22,0,0,"Common/Btn/Dot",FatColor(NO_TRANS)},
	{">>",		RightArrowKV,RelMsk|PathMsk,136,287,39,31,0,0,"Common/Btn/RightArr",FatColor(NO_TRANS)},
	{"Done",	DoneKV,RelMsk|PathMsk,183,287,54,31,0,0,"Common/Btn/Done",FatColor(NO_TRANS)},
	
	//液晶屏下面非显示区域的四个键
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

static const CHAR_BUTTON_OBJ CharButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,
		//Char_x,Char_y,MarginXY,SpaceXY,NormalColor,NormalBG,PressColor,PressBG,ReleaseColor,ReleaseBG},
	{"音乐盒",21,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,18,83,50,19,
		7,4,0x22,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
	{"电子书",22,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,69,83,50,19,
		7,4,0x22,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
	{"图片",23,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,120,83,50,19,
		14,4,0x82,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
	{"设置",24,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,171,83,50,19,
		13,4,0x82,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE TestPage={
	"TestPage",//page name
	"Author",//page author name
	"Page Description",//page description
	NORMAL_PAGE,//page type
	0,//page function mask bit.

	{//control object num records.
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //number of image touch control object
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //number of char touch control object,
		0,//number of dynamic image touch control object
		0,//number of dynamic char touch control object
		0,//number of yes no control object
		0,//number of num box control object
		0,//number of string option control object
	},
	
	ImgButtonCon, //image touch ctrl obj
	CharButtonCon,//char touch ctrl obj
	
	SystemHandler,//handler of system event
	PeripheralsHandler,//handler of Peripherals event
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),//mask bits of peripherals event
	ButtonHandler,//handler of all touch control object
	YesNoHandler,//handler of yes no control object
	NumCtrlObjHandler,//handler of num control object
	StrCtrlObjHandler,//handler of string control object
};

//-----------------------本页自定义变量声明-----------------------
typedef struct{
	bool Tip;
	u32 GlobaU32Value;
	void *pGlobaPointer;
}TestPage_VARS;//本页范围内的全局变量全部定义在这个结构体里面，并通过gpTestPageVars访问，可节省RAM占用
static TestPage_VARS *gpTestPageVars;

//-----------------------本页自定义函数-----------------------

//-----------------------本页系统函数----------------------

//发生系统事件时，会触发的函数
static SYS_MSG SystemHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)//系统事件处理函数
{
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件	
			gpTestPageVars=Q_PageMallco(sizeof(TestPage_VARS));//为本页面的全局变量申请空间
		case Sys_SubPageReturn:	//如果从子页面返回,就不会触发Sys_Page_Init事件,而是Sys_SubPage_Return
			gpTestPageVars->Tip=FALSE;

			//画标题栏
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"TestPage",(240-strlen("TestPage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("TestPage"),FatColor(0xe0e0e0));//写标题
			
			//画背景
			DrawRegion.x=0;
			DrawRegion.y=21;
			DrawRegion.w=240;
			DrawRegion.h=320-21-39;
			DrawRegion.Color=FatColor(0x8b8a8a);
			Gui_FillBlock(&DrawRegion);

			//画底栏
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);

			//画框
			DrawFrame1(25,252);	

			//画点
			{GUI_REGION DrawRegTmp={109,292,22,22,0,FatColor(NO_TRANS)};
			Gui_Draw24Bmp("Theme/F/Common/Btn/DotN.bmp",&DrawRegTmp);}
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			if(gpTestPageVars) Q_PageFree(gpTestPageVars);
		case Sys_PreSubPage:
		
			break;
		default:
			//需要响应的事件未定义
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return 0;
}

//当实体按键、串口、rtc等外设有动作时，会触发此函数
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)//外设事件处理函数
{
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			if(IntParam==ExtiKeyEnter)
			{
			
			}
			else if(IntParam==ExtiKeyUp)
			{
				Q_PresentTch(LeftArrowKV,Tch_Press);
			}
			else if(IntParam==ExtiKeyDown)
			{
				Q_PresentTch(RightArrowKV,Tch_Press);
			}
			break;
		case Perip_KeyRelease:
			if(IntParam==ExtiKeyEnter)
			{
				PrtScreen();
			}
			else if(IntParam==ExtiKeyUp)
			{
				Q_PresentTch(LeftArrowKV,Tch_Release);
			}
			else if(IntParam==ExtiKeyDown)
			{
				Q_PresentTch(RightArrowKV,Tch_Release);
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
static CO_MSG ButtonHandler(u8 OID,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)//按钮控件处理函数
{		
	//GUI_REGION DrawRegion;
	
	switch(OID)
	{
		
		case BackKV:
			Q_GotoPage(GotoNewPage,Q_GetPageByTrack(1)->Name,0,NULL);//返回前一个页面
			break;
		case DoneKV:
			Q_GotoPage(GotoNewPage,Q_GetPageByTrack(1)->Name,0,NULL);//返回前一个页面
			break;	
		case LeftArrowKV:
			break;
		case DotKV:
			if(gpTestPageVars->Tip)
			{
				GUI_REGION DrawRegTmp={109,292,22,22,0,FatColor(NO_TRANS)};
				Gui_Draw24Bmp("Theme/F/Common/Btn/DotN.bmp",&DrawRegTmp);
				gpTestPageVars->Tip=FALSE;
			}
			else
			{
				GUI_REGION DrawRegTmp={109,292,22,22,0,FatColor(NO_TRANS)};
				Gui_Draw24Bmp("Theme/F/Common/Btn/DotP.bmp",&DrawRegTmp);
				gpTestPageVars->Tip=TRUE;
			}
			break;
		case RightArrowKV:
			break;
		case HomeKV:
		case MessageKV:
		case MusicKV:
		case PepoleKV:
			break;
		default:
			//需要响应的事件未定义
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! OID:%d\n\r",Q_GetCurrPageName(),OID);
			///while(1);
	}
	
	return 0;
}

//yes no控件触发的函数
static CO_MSG YesNoHandler(u8 OID,bool NowValue)//yes no控件处理函数
{

	return 0;
}

//num控件触发的函数
static CO_MSG NumCtrlObjHandler(u8 OID,s32 Value,void *pNumCtrlObj)//num控件处理函数
{

	return 0;
}

//str控件触发的函数
static CO_MSG StrCtrlObjHandler(u8 OID,u8 StrID,u8 *Str,void *pStrCtrlObj)//str控件处理函数
{

	return 0;
}



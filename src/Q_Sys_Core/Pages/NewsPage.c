/**************** Q-SYS *******************
 * PageName : NewsPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "NewsPage.h"

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
	BackKV=USER_KEY_VALUE_START,
	LeftArrowKV,
	DotKV,
	RightArrowKV,
	DoneKV,

	Q_ShareweKv,
	InfomationKv,
	EntertainmentKv,
	FinanceKv,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}NewsPage_OID;

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
	{"酷享",Q_ShareweKv,F16Msk|RoueMsk|RelMsk,4,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
	{"资讯",InfomationKv,F16Msk|RoueMsk|RelMsk,63,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
	{"娱乐",EntertainmentKv,F16Msk|RoueMsk|RelMsk,122,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
	{"财经",FinanceKv,F16Msk|RoueMsk|RelMsk,181,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE NewsPage={
	"NewsPage",
	"Author",
	"Page Description",
	NORMAL_PAGE,
	0,//

	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
	ImgButtonCon, //touch region array
	CharButtonCon,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput)|
	Bit(Perip_QWebRecv)|Bit(Perip_QWebSendFailed)|Bit(Perip_QWebSendOk),
	ButtonHandler,
};

//-----------------------本页自定义变量声明-----------------------
#define NEWS_HOST_URL	"Q-ShareWe.idoing.org"
#define NEWS_TXT_DISP_X 8
#define NEWS_TXT_DISP_Y 60
#define NEWS_TXT_DISP_W 224
#define NEWS_TXT_DISP_H 208
#define NEWS_TXT_DISP_SPACE 0x00
#define NEWS_TXT_DISP_COLOR FatColor(0x000000)
#define NEWS_TXT_DISP_BG FatColor(0xe0e0e0)

typedef struct{
	u8 ViewAddr;//view服务器地址
	u8 HostOnlineFlag;//web服务器在线标志
	NewsPage_OID NowTab;//当前标签
	GUI_REGION TxtRegion;
}NEWS_PAGE_VARS;
static NEWS_PAGE_VARS *gNspVars=NULL;

//-----------------------本页自定义函数-----------------------
enum{
	TIP_GRAY=0,
	TIP_GREEN,
	TIP_GREEN_FLASH,
	TIP_RED,
	TIP_RED_FLASH,
};
static void TipState(u8 Act)//画状态灯
{
	GUI_REGION DrawRegTmp={109,292,22,22,0,FatColor(NO_TRANS)};
	
	switch(Act)
	{
		case TIP_GRAY:
			Gui_Draw24Bmp("Theme/F/Common/Btn/DotN.bmp",&DrawRegTmp);
			break;
		case TIP_GREEN:
			Gui_Draw24Bmp("Theme/F/Common/Btn/DotP.bmp",&DrawRegTmp);
			break;
		case TIP_GREEN_FLASH:
			break;
		case TIP_RED:
			break;
		case TIP_RED_FLASH:
			break;
	}
}

//高亮标签
static void HighLightTable(NewsPage_OID Key)
{
	switch(Key)
	{
		case Q_ShareweKv:
		case InfomationKv:
		case EntertainmentKv:
		case FinanceKv:
			Q_PresentTch(Q_ShareweKv,Tch_Normal);
			Q_PresentTch(InfomationKv,Tch_Normal);
			Q_PresentTch(EntertainmentKv,Tch_Normal);
			Q_PresentTch(FinanceKv,Tch_Normal);
			Q_PresentTch(Key,Tch_Press);
			break;
	}
}

typedef enum{
	VC_CONN='c',//连接主机
	VC_ONLINE='o',//主机在线
	VC_DOWNLINE='d',//主机不在线
	VC_HGET='g',//获取文件
	VC_HFILE='f',//回传文件
}VIEW_CMD;

typedef struct{
	u8 ChkCode;//must = v
	VIEW_CMD Cmd;
	u8 Data[1];
}VIEW_CMD_STRUCT;

//发送view命令
static void SendViewCmd(VIEW_CMD Cmd,u8 *pStr)
{
	VIEW_CMD_STRUCT *pView;

	if(gNspVars==NULL) return;
	if(Cmd != VC_CONN && gNspVars->HostOnlineFlag != TRUE) return;

	pView=Q_PageMallco(sizeof(VIEW_CMD_STRUCT)+strlen((void *)pStr));//到send ok后释放
	pView->ChkCode='v';
	pView->Cmd=Cmd;
	strcpy((void *)pView->Data,(void *)pStr);
	QWA_SendData(gNspVars->ViewAddr,sizeof(VIEW_CMD_STRUCT)+strlen((void *)pStr),(u8 *)pView);
}

//分析view命令
static void ParseViewCmd(u8 Addr,u32 Len,u8 *pData)
{
	VIEW_CMD_STRUCT *pView=(void *)pData;

	//Debug("Parse:%s\n\r",pData);
	if((pView->ChkCode!='v')&&(pView->ChkCode!='V')) return;

	switch(pView->Cmd)
	{
		case VC_ONLINE:
			gNspVars->HostOnlineFlag=TRUE;
			TipState(TIP_GREEN);
			break;
		case VC_DOWNLINE:
			gNspVars->HostOnlineFlag=FALSE;
			TipState(TIP_GRAY);
			break;
		case VC_HFILE:
			HighLightTable(gNspVars->NowTab);//高亮标签
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
			Gui_FillBlock(&gNspVars->TxtRegion);//填充背景
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_COLOR;
			Gui_DrawFont(GBK12_FONT,pView->Data,&gNspVars->TxtRegion);
			break;
	}
}

//画背景
static void DrawInitBg(void)
{
	GUI_REGION DrawRegion;

	//画标题栏
	DrawRegion.x=DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=21;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
	DrawTitle1(ASC14B_FONT,"NewsPage",(240-strlen("NewsPage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("NewsPage"),FatColor(0xe0e0e0));//写标题
	
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

	TipState(TIP_GRAY);

	gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
	Gui_FillBlock(&gNspVars->TxtRegion);
}
//-----------------------本页系统函数----------------------

//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(IntParam<QW_ADDR_HOST || IntParam>QW_ADDR_MAX) return SM_NoGoto;
			break;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件				
			if(IntParam) 
			{
				gNspVars=Q_PageMallco(sizeof(NEWS_PAGE_VARS));
				gNspVars->ViewAddr=IntParam;
				gNspVars->TxtRegion.x=NEWS_TXT_DISP_X;
				gNspVars->TxtRegion.y=NEWS_TXT_DISP_Y;
				gNspVars->TxtRegion.w=NEWS_TXT_DISP_W;
				gNspVars->TxtRegion.h=NEWS_TXT_DISP_H;
				gNspVars->TxtRegion.Space=NEWS_TXT_DISP_SPACE;
				gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
			}			
			DrawInitBg();
			SendViewCmd(VC_CONN,NEWS_HOST_URL);

			//提示信息
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
			Gui_FillBlock(&gNspVars->TxtRegion);//填充背景
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_COLOR;
			Gui_DrawFont(GBK12_FONT,"下方中部圆点为高亮表示已经正确连接到VIEW网桥及远端Web服务器，则可以点击标签按键获取新闻。若圆点为灰色，请重复进入此页面。",&gNspVars->TxtRegion);
			break;
		case Sys_SubPageReturn:	//如果从子页面返回,就不会触发Sys_Page_Init事件,而是Sys_SubPage_Return
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			if(gNspVars!=NULL) Q_PageFree(gNspVars);
		case Sys_PreSubPage:
		
			break;
		default:
			//需要响应的事件未定义
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return 0;
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

		case Perip_QWebRecv:
			ParseViewCmd(IntParam>>24,IntParam&0xffffff,pParam);
			break;
		case Perip_QWebSendOk:
			Debug("Send View Cmd OK!\n\r");
			if(IsHeapRam(pParam)) Q_PageFree(pParam);
			break;
		case Perip_QWebSendFailed:
			Debug("Send View Cmd Failed!\n\r");
			if(IsHeapRam(pParam)) Q_PageFree(pParam);
			break;
	}

	return 0;
}

//当使用者按下本页TouchRegionSet里定义的按键时，会触发这个函数里的对应事件
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{		
	switch(Key)
	{
		case BackKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//返回前一个页面
			break;
		case DoneKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//返回前一个页面
			break;	

		case Q_ShareweKv:
			gNspVars->NowTab=(NewsPage_OID)Key;
			SendViewCmd(VC_HGET,"/View/Q_ShareWe.html");
			break;
		case InfomationKv:
			gNspVars->NowTab=(NewsPage_OID)Key;
			SendViewCmd(VC_HGET,"/View/Infomation.html");
			break;
		case EntertainmentKv:
			gNspVars->NowTab=(NewsPage_OID)Key;
			SendViewCmd(VC_HGET,"/View/Entertainment.html");
			break;
		case FinanceKv:
			gNspVars->NowTab=(NewsPage_OID)Key;
			SendViewCmd(VC_HGET,"/View/Finance.html");
			break;
		
		case LeftArrowKV:
			break;
		case DotKV:
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
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}




/**************** Q-SYS *******************
 * PageName : MainPage
 * Author : YuanYin
 * Version : 2.0
 * Base Q-Sys Version : 2.x
 * Description : This the first page in Q-sys
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "MusicHandler.h"
#include "MainPage.h"
#include "FileListPage.h"

static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
static CO_MSG NumCtrlObjHandler(u8 OID,s32 Value,void *pNumCtrlObj);
static CO_MSG StrCtrlObjHandler(u8 OID,u8 StrID,u8 *Str,void *pStrCtrlObj);

//定义互斥信号量，为系统和页面或应用之间协调
typedef enum
{
	ExtiKey0=EXTI_KEY_VALUE_START,//系统默认将外部中断按键发送到第一个键值
	ExtiKey1,
	ExtiKey2,

	//后面的硬件版本将支持更多外部中断按键或者无线键盘，
	//所以触摸键值从0x40开始，将前面的键值都留下来	
	MusicKV=USER_KEY_VALUE_START,
	EBookKV,
	PictureKV,
	AppListKV,
	AdcKV,
	CalculatorKV,
	PrevKV,
	NextKV,
	
	HomeKV,
	MailKV,
	RingKV,
	CallKV,
}MainPage_OID;

//定义页面或应用的触摸区域集，相当于定义按键
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//{key,gLandScapeMode,x,y,width,hight,image x,image y,normal bmp path,release bmp path,press bmp path,transparent color,key name}
	{"<",				PrevKV,		RelMsk,0,33,17,70,0,25,"Left",FatColor(NO_TRANS)},
	{"Music",		MusicKV,	RelMsk,18,33,50,70,0,0,"Music",FatColor(0xff0000)},
	{"EBook",		EBookKV,	RelMsk,69,33,50,70,0,0,"EBook",FatColor(0xff0000)},
	{"Picture",	PictureKV,	RelMsk,120,33,50,70,0,0,"Picture",FatColor(0xff0000)},
	{"AppList",	AppListKV,	RelMsk,171,33,50,70,0,0,"AppList",FatColor(0xff0000)},
	{">",				NextKV,	RelMsk,222,33,18,70,0,25,"Right",FatColor(NO_TRANS)},
	
	//液晶屏下面非显示区域的四个键
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MailKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",RingKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",CallKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE MainPage={
	"MainPage", //name of page or app
	"YuanYin",
	"Main page",
	NORMAL_PAGE, //type,must be page or app
	0,

	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
		0,0,
		1,3,2
	},
	
	ImgButtonCon, //touch region array
	NULL,//CharButtonCon,
	
	SystemEventHandler, //init page or app function  
	PeripheralsHandler,
	Bit(Perip_RtcMin)|Bit(Perip_LcdOff)|Bit(Perip_LcdOn)|Bit(Perip_UartInput)|
	Bit(Perip_Timer)|Bit(Perip_RtcAlarm)|Bit(Perip_KeyPress)|Bit(Perip_KeyRelease),
	ButtonHandler, //touch input event handler function
	NULL,
	NumCtrlObjHandler,
	StrCtrlObjHandler,
};

static const char Week[][4]={"一","二","三","四","五","六","天"};
static void DispTime(void)
{
	RTC_TIME Time;
	char TimeMsg[64];
	GUI_REGION DrawRegion;

	if(Gui_GetBgLightVal()==0) return;//背光没亮，无需更新

	RTC_GetTime(&Time);
	sprintf(TimeMsg,"%02d月%02d日 周%s",Time.mon,Time.day,Week[Time.week]);

	DrawRegion.x=0;
	DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=18;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_Draw24Bmp("Theme/F/MainPage/Bg/A01.Bmp",&DrawRegion);
	
	DrawRegion.x=4;
	DrawRegion.y=4;
	DrawRegion.w=120;
	DrawRegion.h=16;
	DrawRegion.Space=0x11;
  	DrawRegion.Color=FatColor(0xf2f2f2);
	Gui_DrawFont(GBK12_FONT,(void *)TimeMsg,&DrawRegion);

	DrawRegion.x=200;
	DrawRegion.Space=0x00;
	sprintf(TimeMsg,"%02d:%02d",Time.hour,Time.min);
	Gui_DrawFont(GBK12_FONT,(void *)TimeMsg,&DrawRegion);
}

//在每次本页面变成系统当前页面时会调用的程序
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent,int IntParam,void *pSysParam)
{
	GUI_REGION DrawRegion;
	int Time;
	Debug("## MainPage %d\n\r",SysEvent);

	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:
		case Sys_SubPageReturn:
			if(Q_GetPageEntries()==1)  //页面首次创建时需要做的事情
			{

			}
			Time=OS_GetCurrentTick();
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=320;	
			DrawRegion.Color=FatColor(0xffffff);
      		Gui_FillBlock(&DrawRegion);	
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_DrawImgBin("Theme/F/MainPage/Bg/BG2.bin",&DrawRegion);		

			DrawRegion.x=25;
			DrawRegion.y=119+5;
			DrawRegion.w=189;
			DrawRegion.h=105;			
			DrawRegion.Space=0x19;
			Gui_DrawFont(GBK12_FONT,"酷系统 V2.0\n这是一个用于酷学玩开发板的系统框架，可以自由添加应用，代码开源，更多信息，请浏览官方网站!",&DrawRegion);
			
			Debug("Display Bmp Time:%d\n\r",(OS_GetCurrentTick()-Time)<<1);
			DispTime();
			break;
		case Sys_TouchSetOk:
			break;
		case Sys_TouchSetOk_SR:		
			DispTime();
			break;
		default:
			Debug("You should not here! %d\n\r",SysEvent);
			//while(1);
	}
	
	return 0;
}
		
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			switch(IntParam){
				case ExtiKey0:
					Debug("Key%d,%d\n\r",ExtiKey0,Perip_KeyPress);
					Q_PresentTch(NextKV,Tch_Press);
					break;
				case ExtiKey1:
					Debug("Key%d,%d\n\r",ExtiKey1,Perip_KeyPress);
					Q_PresentTch(PrevKV,Tch_Press);
					break;
				case ExtiKey2:
					Debug("Key%d,%d\n\r",ExtiKey2,Perip_KeyPress);

					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKey0:
					Debug("Key%d,%d\n\r",ExtiKey0,Perip_KeyRelease);
					Q_PresentTch(NextKV,Tch_Release);
					break;
				case ExtiKey1:
					Debug("Key%d,%d\n\r",ExtiKey1,Perip_KeyRelease);
					Q_PresentTch(PrevKV,Tch_Release);
					break;
				case ExtiKey2:
					Debug("Key%d,%d\n\r",ExtiKey2,Perip_KeyRelease);
					Q_GotoPage(GotoNewPage,"AppListPage",0,NULL);
					break; 
			}break;
		case Perip_Timer:			
			break;
		case Perip_RtcMin:		
			DispTime();
			break;
		case Perip_RtcAlarm:	
			Debug("Alarm!!!\n\r");
			break;
		case Perip_LcdOff:
			Debug("LCD bg light disable\n\r");
			break;
		case Perip_LcdOn:
			Debug("LCD bg light enable\n\r");
			DispTime();
			break;
		case Perip_UartInput:
			if((IntParam>>16)==1)//串口1
			{
				Q_Sh_CmdHandler(IntParam&0xffff,pParam);
			}
			if((IntParam>>16)==3)//串口3
			{
				IntParam&=0xffff;

				if(IntParam) Debug("###----Uart3----\n\r%s\n\r###----Uart3----\n\r",(u8 *)pParam);
			}
			break;
	}

	return 0;
}

NUM_BOX_OBJ NumBoxObj;
NUM_LIST_OBJ NumListObj;
NUM_ENUM_OBJ NumEnumObj;
STR_BOX_OBJ StrBoxObj;
STR_ENUM_OBJ StrEnumObj;
s32 gEnumList[100];
u8 gStrBoxBuf[100];
u8 gStrEnumBuf[100];

static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{
	MUSIC_EVENT MusicEvent;

	//GUI_REGION DrawRegion;
	
	//u32 cpu_sr;
	//u8 buf[]="12345\n\r";
	//FILELIST_SET FileListPageParam;

	switch(Key)
	{
		case NextKV:		
			break;
		case PrevKV:
			break;
		case MusicKV:
			Q_GotoPage(GotoNewPage,"MusicPage",0,NULL);
			break;
		case EBookKV:
			Q_GotoPage(GotoNewPage,"EBookPage",0,NULL);
			break;
		case PictureKV:
			Q_GotoPage(GotoNewPage,"PicturePage",0,NULL);
			break;
		case AppListKV:
			Debug("go to app list\n\r");
			Q_GotoPage(GotoNewPage,"AppListPage",0,NULL);
			break;
		case AdcKV:
			Q_GotoPage(GotoNewPage,"AdcPage",0,NULL);
			break;
		case CalculatorKV:
			Q_GotoPage(GotoNewPage,"CalculatorPage",0,NULL);
			break;
 		case HomeKV:
			NumListObj.ObjID=100;
			NumListObj.Type=NCOT_NumList;
			NumListObj.x=10;
			NumListObj.y=120;
			NumListObj.w=100;
			NumListObj.Value=50;
			NumListObj.Min=0;
			NumListObj.Max=100;
			NumListObj.Step=10;
			Q_SetNumList(1,&NumListObj);

			NumEnumObj.ObjID=101;
			NumEnumObj.Type=NCOT_NumEnum;
			NumEnumObj.x=120;
			NumEnumObj.y=120;
			NumEnumObj.w=100;
			NumEnumObj.Value=0;
			NumEnumObj.Idx=0;
			NumEnumObj.Num=3;
			NumEnumObj.Total=4;
			NumEnumObj.pEnumList=gEnumList;
			NumEnumObj.pEnumList[0]=0;
			NumEnumObj.pEnumList[1]=1;
			NumEnumObj.pEnumList[2]=2;
			NumEnumObj.pEnumList[3]=3;
			Q_SetNumEnum(2,&NumEnumObj);

			NumBoxObj.ObjID=102;
			NumBoxObj.Type=NCOT_NumBox;
			NumBoxObj.x=10;
			NumBoxObj.y=150;
			NumBoxObj.w=100;
			NumBoxObj.Value=50;
			NumBoxObj.Max=1000;
			NumBoxObj.Min=40;
			Q_SetNumBox(3,&NumBoxObj);
 			//Gui_SetBgLight(0);

			StrBoxObj.ObjID=103;
			StrBoxObj.Type=SCOT_StrBox;
			StrBoxObj.x=20;
			StrBoxObj.y=180;
			StrBoxObj.w=200;
			StrBoxObj.h=60;
			StrBoxObj.TotalSize=100;
			StrBoxObj.pStrBuf=gStrBoxBuf;
			Q_SetStrBox(1,&StrBoxObj);

			StrEnumObj.ObjID=104;
			StrEnumObj.Type=SCOT_StrEnum;
			StrEnumObj.x=20;
			StrEnumObj.y=260;
			StrEnumObj.w=200;
			StrEnumObj.Idx=0;
			StrEnumObj.Size=0;
			StrEnumObj.TotalSize=100;
			StrEnumObj.pStrEnumBuf=gStrEnumBuf;

			Q_SetStrEnum(2,&StrEnumObj);
			Q_StrEnumAddOne(&StrEnumObj,'a',"Test1");
			Q_StrEnumAddOne(&StrEnumObj,'b',"12345678901234567890");
			Q_StrEnumAddOne(&StrEnumObj,'c',"Test222");
			Q_StrEnumAddOne(&StrEnumObj,'d',"Test333");
			Q_StrEnumDeleteOne(&StrEnumObj,'a');

 			break;
		case MailKV:
			Gui_SetBgLight(50);
			break;
		case RingKV:
#if 1//debug by karlno
	Key=0;
	Q_SpiFlashSync(FlashRead,0,4,(void *)&Key);
	Debug("buf %d\n\r",Key);
#endif	
			break;
		case CallKV:
		#if 1//debug by karlno
	MusicEvent.Cmd=MusicPlay;
	MusicEvent.pFilePath="Music/3.mp3";
	MusicEvent.ParamU16=0xff;
	//MusicEvent.pParam=NULL;
	Q_MusicSync(&MusicEvent);
	OS_TaskDelayMs(100);
#endif	
			break;
		default:
			Debug("You should not here!Key:%d\n\r",Key);
			//while(1);
	}
	
	return 0;
}

static CO_MSG NumCtrlObjHandler(u8 OID,s32 Value,void *pNumCtrlObj)
{
	Debug("NumCtrlObj:%d,%d\n\r",OID,Value);
	return 0;
}

static CO_MSG StrCtrlObjHandler(u8 OID,u8 StrID,u8 *Str,void *pStrCtrlObj)
{
	Debug("StrCtrlObj:%d,%c,%s\n\r",OID,StrID,Str);
	return 0;
}




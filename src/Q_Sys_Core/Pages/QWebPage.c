/**************** Q-SYS *******************
 * PageName : QWebPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "QWebPage.h"

//函数声明
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
static CO_MSG YesNoHandler(u8 ObjID,bool NowValue);

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
	ListKV,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}QWebPage_OID;

//本页宏定义
#define QWEB_PAGE_BG_COLOR FatColor(0xe0e0e0)//页面背景色
#define QWEB_PAGE_FONT_COLOR FatColor(0x000000)//页面文字色
#define QWEB_PAGE_SELECT_ACT_WAITING_MS	3000 //点击列表项之后，最多等待设备回应时间
#define DEVICE_INFO_MAX_CLIENT_RECORD 7//最多纪录20个client
#define DEVICE_INFO_MAX_LIST_NUM 7//一页最多显示7个信息
#define DEVICE_NAME_MAX_LEN	16//名字长度
#define DEVICE_INFO_START_X 12
#define DEVICE_INFO_START_Y 78
#define DEVICE_INFO_W 216
#define DEVICE_INFO_H 20
#define DEVICE_INFO_LIST_SPACE 6
#define DEVICE_INFO_LIST_BG FatColor(0xcccccc)
#define DEVICE_INFO_BG_COLOR FatColor(0xf0f0f0)
#define DEVICE_INFO_FONT_COLOR FatColor(0x000000)
#define DEVICE_INFO_HL_BG_COLOR FatColor(0xf0f0f0)
#define DEVICE_INFO_HL_FONT_COLOR FatColor(0xff0000)
#define DEVICE_INFO_PRS_BG_COLOR FatColor(0xff0000)
#define DEVICE_INFO_PRS_FONT_COLOR FatColor(0xffffff)

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

	//device list region
	{"", ListKV,PrsMsk|RelMsk|ReVMsk,DEVICE_INFO_START_X,DEVICE_INFO_START_Y,DEVICE_INFO_W,DEVICE_INFO_MAX_LIST_NUM*(DEVICE_INFO_H+DEVICE_INFO_LIST_SPACE),
			0,0,"",FatColor(NO_TRANS)},
	
	//液晶屏下面非显示区域的四个键
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

//定义页面或者应用的属性集
const PAGE_ATTRIBUTE QWebPage={
	"QWebPage",
	"Author",
	"Page Description",
	NORMAL_PAGE,
	0,//

	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
		0,
		0,
		1,
	},
	ImgButtonCon, //touch region array
	NULL,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_QWebJoin)|Bit(Perip_QWebQueryName)|
	Bit(Perip_QWebRecv)|Bit(Perip_QWebSendFailed)|Bit(Perip_QWebSendOk)|Bit(Perip_Timer)|
	Bit(Perip_UartInput),
	ButtonHandler,
	YesNoHandler,
};

//-----------------------本页自定义变量声明-----------------------
typedef struct{
	u8 Addr;//记录地址
	u8 DispIdx;//记录排到列表第几个位置，从1开始
	bool IsHiLight;//设备名是否高亮
	u16 NameChk;//名字校验码
	u8 Name[DEVICE_NAME_MAX_LEN];//记录名字
}CLIENT_RECORD;

typedef struct{
	YES_NO_OBJ YesNo;//q web 开关
	u8 NowDispNum;// 当前列表显示数目，from 0 start
	u8 NowPressKey;//当前按下的列表按键号，没有时为0xff
	CLIENT_RECORD ClientRecord[DEVICE_INFO_MAX_CLIENT_RECORD];//q网设备记录
	u8 SelectedAddr;//选定的地址，没有时为0
	u32 SelectedTimeMs;//选定列表项的时间

	u8 PageTitle[32];//页面标题
	PAGE_ACTION GotoAct_List;//按下列表项后需以何种方式进入的新页面
	u8 GotoName_List[20];//按下列表项后需进入的页面
	PAGE_ACTION GotoAct_Back;//按下back后需以何种方式进入的新页面
	u8 GotoName_Back[20];//按下back后需进入的页面
	PAGE_ACTION GotoAct_Done;//按下done后需以何种方式进入的新页面
	u8 GotoName_Done[20];//按下done后需进入的页面
}QWEB_PAGE_VARS;
static QWEB_PAGE_VARS *gpQwpVar=NULL;
static QWEB_PAGE_SET *gpQwpSet=NULL;

//-----------------------本页自定义函数-----------------------
u8 *QWP_GetNameByAddr(u8 Addr)
{
	u8 i;
	
	if(gpQwpVar==NULL) return "";

	for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
	if(gpQwpVar->ClientRecord[i].Addr==Addr)
	{
		return gpQwpVar->ClientRecord[i].Name;
	}
	return "";
}

//-----------------------本页系统函数----------------------
static void DrawState(void)//更新当前q网状态显示
{
	GUI_REGION DrawRegion;
	u8 Str[32];

	if(Q_GetPageByTrack(0)!=(&QWebPage)) return;//看是否在本页面
	
	//q网开关
	DrawRegion.x=10;
	DrawRegion.y=34;
	DrawRegion.w=160;
	DrawRegion.h=16;
	DrawRegion.Space=0x00;
	DrawRegion.Color=QWEB_PAGE_BG_COLOR;
	Gui_FillBlock(&DrawRegion);
	DrawRegion.Color=QWEB_PAGE_FONT_COLOR;
	if(QWA_GetMyAddr()==QW_ADDR_DEF)//未分配
	{
		Gui_DrawFont(GBK16_FONT,"Q Web State",&DrawRegion);
	}
	else if(QWA_GetMyAddr()==QW_ADDR_HOST)//主机
	{
		sprintf((void *)Str,"[H%d]%s",QWA_GetMyAddr(),QWA_MyQWebName(NULL));
		Gui_DrawFont(GBK16_FONT,Str,&DrawRegion);
	}
	else//从机
	{
		sprintf((void *)Str,"[S%d]%s",QWA_GetMyAddr(),QWA_MyQWebName(NULL));
		Gui_DrawFont(GBK16_FONT,Str,&DrawRegion);
	}
	
	gpQwpVar->YesNo.DefVal=QWA_QWebState();
	gpQwpVar->YesNo.ObjID=1;
	gpQwpVar->YesNo.x=170;
	gpQwpVar->YesNo.y=33;
	Q_SetYesNo(1,&gpQwpVar->YesNo);
}


//显示一条记录信息
//Idx从1开始，最大DEVICE_INFO_MAX_LIST_NUM
enum{
	NormalDisp=0,//普通显示
	PressDisp,//按下显示
	HighLightDisp,//高亮显示
	CleanDisp,//清除显示
};
static bool DrawDeviceInfo(u8 Idx,u8 Addr,u8 *Name,u8 Action)
{
	GUI_REGION DrawRegion;
	u8 StrBuf[DEVICE_NAME_MAX_LEN<<1];

	if(Q_GetPageByTrack(0)!=(&QWebPage)) return TRUE;//看是否在本页面
	if((Idx<1)||(Idx>DEVICE_INFO_MAX_LIST_NUM)) return FALSE;
	Idx--;
	
	//bg
	DrawRegion.x=DEVICE_INFO_START_X;
	DrawRegion.y=DEVICE_INFO_START_Y+Idx*(DEVICE_INFO_H+DEVICE_INFO_LIST_SPACE);
	DrawRegion.w=DEVICE_INFO_W;
	DrawRegion.h=DEVICE_INFO_H;
	if(Action==NormalDisp) DrawRegion.Color=DEVICE_INFO_BG_COLOR;
	else if(Action==PressDisp) DrawRegion.Color=DEVICE_INFO_PRS_BG_COLOR;
	else if(Action==HighLightDisp) DrawRegion.Color=DEVICE_INFO_HL_BG_COLOR;
	else	DrawRegion.Color=DEVICE_INFO_LIST_BG;
	Gui_FillBlock(&DrawRegion);

	//font
	if(Action!=CleanDisp)
	{
		DrawRegion.x+=4;
		DrawRegion.y+=4;
		DrawRegion.Space=0x00;
		if(Action==PressDisp) DrawRegion.Color=DEVICE_INFO_PRS_FONT_COLOR;
		else if(Action==HighLightDisp) DrawRegion.Color=DEVICE_INFO_HL_FONT_COLOR;
		else	DrawRegion.Color=DEVICE_INFO_FONT_COLOR;
		if(Name[0]==0)
			sprintf((void *)StrBuf,"[%c%d]Waitting for get name...",(Addr==QW_ADDR_HOST)?'H':'S',Addr);
		else
			sprintf((void *)StrBuf,"[%c%d]%s",(Addr==QW_ADDR_HOST)?'H':'S',Addr,Name);
		Gui_DrawFont(GBK12_FONT,StrBuf,&DrawRegion);
	}

	return TRUE;
}

//增加一条记录，并同步显示
//Addr不能为0，Name可以为空，name为空，则不修改name
static void AddOneDevice(u8 Addr,u8 *Name)
{
	u8 i;
	u8 Len=0;
	u16 NameChk;

	if(Name!=NULL) Len=strlen((void *)Name);
	if(Len>=DEVICE_NAME_MAX_LEN) Len=DEVICE_NAME_MAX_LEN-1;
	NameChk=MakeHash33(Name,Len);

	for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
	{
		//对比地址或名字，如果地址相等，直接修改名字，如果名字相同，则修改地址。
		if((gpQwpVar->ClientRecord[i].Addr==Addr)||((gpQwpVar->ClientRecord[i].Addr!=0)&&(gpQwpVar->ClientRecord[i].NameChk==NameChk)))
		{//修改现成的
			gpQwpVar->ClientRecord[i].Addr=Addr;
			
			if((Name!=NULL)&&(gpQwpVar->ClientRecord[i].NameChk!=NameChk))//修改名字
			{
				MemCpy(gpQwpVar->ClientRecord[i].Name,Name,Len);
				gpQwpVar->ClientRecord[i].Name[Len]=0;
			}

			if(gpQwpVar->ClientRecord[i].IsHiLight==TRUE)
				DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,Addr,gpQwpVar->ClientRecord[i].Name,HighLightDisp);
			else
				DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
			break;
		}
	}

	if(i==DEVICE_INFO_MAX_CLIENT_RECORD)//not found ,add new one.
	{//建新的
		for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
			if(gpQwpVar->ClientRecord[i].Addr==0)
			{
				gpQwpVar->ClientRecord[i].Addr=Addr;
				MemCpy(gpQwpVar->ClientRecord[i].Name,Name,Len);
				gpQwpVar->ClientRecord[i].Name[Len]=0;
				gpQwpVar->ClientRecord[i].NameChk=MakeHash33(gpQwpVar->ClientRecord[i].Name,Len);
				gpQwpVar->ClientRecord[i].DispIdx=++gpQwpVar->NowDispNum;
				DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
				break;
			}
	}	

	DrawState();//更新下状态
}

//删除一个设备记录
static void DeleteOneDevice(u8 Addr)
{
	u8 i;
	u8 DispIdx=0xff;
	
	for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
	{
		if(gpQwpVar->ClientRecord[i].Addr==Addr)
		{
			DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,Addr,NULL,CleanDisp);
			DispIdx=gpQwpVar->ClientRecord[i].DispIdx;
			gpQwpVar->ClientRecord[i].Addr=0;
			gpQwpVar->ClientRecord[i].Name[0]=0;
			gpQwpVar->ClientRecord[i].DispIdx=0;
			gpQwpVar->NowDispNum--;
		}
	}

	for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
	{
		if((gpQwpVar->ClientRecord[i].Addr!=0)&&(gpQwpVar->ClientRecord[i].DispIdx>DispIdx))
		{
			DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx--,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,CleanDisp);
			DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
		}
	}
}

//高亮一个设备
static void HighLightOneDevice(u8 Addr)
{
	u8 i;
	
	for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
	{
		//对比地址，如果地址相等，高亮
		if(gpQwpVar->ClientRecord[i].Addr==Addr)
		{
			gpQwpVar->ClientRecord[i].IsHiLight=TRUE;
			DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,Addr,gpQwpVar->ClientRecord[i].Name,HighLightDisp);
			break;
		}
	}

	//没找到，说明有新的设备来了
	if(i==DEVICE_INFO_MAX_CLIENT_RECORD)//not found ,add new one.
	{//建新的
		AddOneDevice(Addr,NULL);
		HighLightOneDevice(Addr);
	}	

	DrawState();//更新下状态
}

static void DrawInitBg(void)
{
	GUI_REGION DrawRegion;
	
	//画标题栏
	DrawRegion.x=DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=21;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
	if(gpQwpVar) DrawTitle1(ASC14B_FONT,gpQwpVar->PageTitle,(240-strlen((void *)gpQwpVar->PageTitle)*GUI_ASC14B_ASCII_WIDTH)>>1,strlen((void *)gpQwpVar->PageTitle),QWEB_PAGE_BG_COLOR);//写标题
	
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

	//device info list bg
	DrawRegion.x=6;
	DrawRegion.y=56;
	DrawRegion.w=228;
	DrawRegion.h=212;
	DrawRegion.Color=DEVICE_INFO_LIST_BG;
	Gui_FillBlock(&DrawRegion);

	//device list
	DrawRegion.x=76;
	DrawRegion.y=56+4;
	DrawRegion.w=100;
	DrawRegion.h=16;
	DrawRegion.Space=0x00;
	DrawRegion.Color=DEVICE_INFO_FONT_COLOR;
	Gui_DrawFont(ASC14B_FONT,"Device List",&DrawRegion);	
}

//发生某些事件时，会触发的函数
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(pSysParam!=NULL)//如果pSysParam不为空，则表示被其他页面调用，需传递数据
			{
				gpQwpSet=Q_Mallco(sizeof(QWEB_PAGE_SET));//使用QS_Mallco是为了逃避页面的堆检查机制
				MemCpy((void *)gpQwpSet,pSysParam,sizeof(QWEB_PAGE_SET));
			}
			break;
		case Sys_PageInit:		//系统每次打开这个页面，会处理这个事件			
			gpQwpVar=Q_PageMallco(sizeof(QWEB_PAGE_VARS));//在Sys_PreGotoPage中处理，并使用OS_Mallco，是为了获取到前页数据，屏蔽页面堆检查
			MemSet(gpQwpVar,0,sizeof(QWEB_PAGE_VARS));
			gpQwpVar->NowPressKey=0xff;
			if(gpQwpSet!=NULL) 
			{
				strncpy((void *)gpQwpVar->PageTitle,(void *)gpQwpSet->PageTitle,sizeof(gpQwpVar->PageTitle));
				gpQwpVar->GotoAct_List=gpQwpSet->GotoAct_List;
				strncpy((void *)gpQwpVar->GotoName_List,(void *)gpQwpSet->GotoName_List,sizeof(gpQwpVar->GotoName_List));
				gpQwpVar->GotoAct_Back=gpQwpSet->GotoAct_Back;
				strncpy((void *)gpQwpVar->GotoName_Back,(void *)gpQwpSet->GotoName_Back,sizeof(gpQwpVar->GotoName_Back));			 	
				gpQwpVar->GotoAct_Done=gpQwpSet->GotoAct_Done;
				strncpy((void *)gpQwpVar->GotoName_Done,(void *)gpQwpSet->GotoName_Done,sizeof(gpQwpVar->GotoName_Done));
			}		
		case Sys_SubPageReturn:	//如果从子页面返回,就不会触发Sys_Page_Init事件,而是Sys_SubPage_Return
			if(SysEvent==Sys_SubPageReturn) //关全局事件
			{
				Q_DisableGobalPeripEvent(Perip_QWebJoin,PeripheralsHandler);
				Q_DisableGobalPeripEvent(Perip_QWebQueryName,PeripheralsHandler);
			}
			Q_TimSet(Q_TIM1,10000,500,TRUE);//5s poll

			DrawInitBg();
			DrawState();

			//如果q网已经开启，如果是主机，应迅速建立list
			//如果q网已经开启，如果是从机，应迅速建立list
			//如果q网没开启，不动
			if(QWA_GetMyAddr()!=QW_ADDR_DEF)//q网已经启动
			{
				u8 Addr;
				u8 MyAddr=QWA_GetMyAddr();

				if(QWA_GetMyAddr()!=QW_ADDR_HOST)//自己是从机
				{
					AddOneDevice(QW_ADDR_HOST,NULL);//添加主机
				}
				
				//轮询添加在线从机
				QWA_QueryNextOnline(TRUE);
				while((Addr=QWA_QueryNextOnline(FALSE))!=0)
				{
					if(Addr!=MyAddr) 
					{
						AddOneDevice(Addr,NULL);//添加无名设备
					}
				}
			}
			if(gpQwpSet!=NULL)//保存前页数据的内存可以销毁了
			{
				Q_Free(gpQwpSet);
				gpQwpSet=NULL;
			}
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:
			
			break;
		case Sys_PageClean:
			Q_PageFree(gpQwpVar);
			gpQwpVar=NULL;
		case Sys_PreSubPage:
			Q_TimSet(Q_TIM1,0,0,FALSE);//stop poll
			if(SysEvent==Sys_PreSubPage)//开全局事件 
			{
				Q_EnableGobalPeripEvent(Perip_QWebJoin,PeripheralsHandler);
				Q_EnableGobalPeripEvent(Perip_QWebQueryName,PeripheralsHandler);
			}
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
			switch(IntParam){
				case ExtiKeyEnter:
					break;
				case ExtiKeyUp:
					break;
				case ExtiKeyDown:
					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
					//PrtScreen();
					QWA_Show();
					break;
				case ExtiKeyUp:
					break;
				case ExtiKeyDown:
					break; 
			}break;

		case Perip_Timer://轮询检查q网
			Q_TimSet(Q_TIM1,10000,500,TRUE);//5s poll
			if(QWA_GetMyAddr()!=QW_ADDR_DEF)//已经入网
			{
				u8 i;
				
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//检查设备还在不在
				{
					if(gpQwpVar->ClientRecord[i].Addr!=0)
						if(QWA_QueryOnline(gpQwpVar->ClientRecord[i].Addr)==0)//设备不在了
						{Debug("@@ Delete %d\n\r",gpQwpVar->ClientRecord[i].Addr);
							DeleteOneDevice(gpQwpVar->ClientRecord[i].Addr);
						}
						else if(gpQwpVar->ClientRecord[i].Name[0]==0)//没名字的，查询名字 //unfinsh:如果查不到，会一直循环查
						{
							QWA_QueryName(gpQwpVar->ClientRecord[i].Addr);
							Q_TimSet(Q_TIM1,10000,200,TRUE);// 2s poll
							break;
						}
				}
			}

			//看看点击的设备，是否依然没有回应，如果超时，点击无效
			if((gpQwpVar->SelectedAddr) && (gpQwpVar->SelectedAddr!=QW_ADDR_HOST)
				&&	(OS_GetCurrentSysMs()-gpQwpVar->SelectedTimeMs > QWEB_PAGE_SELECT_ACT_WAITING_MS) )
			{
				//DeleteOneDevice(gpQwpVar->SelectedAddr);
				gpQwpVar->SelectedAddr=0;
			}
			break;
			
		case Perip_QWebJoin:
			if(QWA_GetMyAddr()==QW_ADDR_HOST)//做为主机得到从机的query ack
			{
				Debug("QWeb Join [%d]%s\n\r",IntParam,(u8 *)pParam);
				AddOneDevice(IntParam,pParam);
			}
			else if(QWA_GetMyAddr()!=QW_ADDR_DEF)//做为从机，得到主机信息
			{
				u8 Addr;
				u8 MyAddr=QWA_GetMyAddr();
				
				Debug("QWeb Get Host Info [%d]%s\n\r",IntParam,(u8 *)pParam);
				AddOneDevice(IntParam,pParam);

				//轮询在线从机
				QWA_QueryNextOnline(TRUE);
				while((Addr=QWA_QueryNextOnline(FALSE))!=0)
				{
					if(Addr!=MyAddr) AddOneDevice(Addr,NULL);//添加无名设备
				}

				if(gpQwpVar->SelectedAddr==QW_ADDR_HOST)//有选定主机作为对话地址，正在等待主机info
				{
					gpQwpVar->SelectedAddr=0;
					Q_GotoPage(gpQwpVar->GotoAct_List,gpQwpVar->GotoName_List,IntParam,pParam);
				}
			}
			break;
		case Perip_QWebQueryName:
			Debug("QWeb Query [%d]%s\n\r",IntParam,(u8 *)pParam);
			AddOneDevice(IntParam,pParam);

			if(IntParam==gpQwpVar->SelectedAddr)//有选定地址，正在等待query ack
			{
				gpQwpVar->SelectedAddr=0;
				Q_GotoPage(gpQwpVar->GotoAct_List,gpQwpVar->GotoName_List,IntParam,pParam);
			}
			break;
		case Perip_QWebRecv://收到信息
			HighLightOneDevice(IntParam>>24);
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
	u8 i;
	
	switch(Key)
	{
		case BackKV:
			Q_GotoPage(gpQwpVar->GotoAct_Back,gpQwpVar->GotoName_Back,0,NULL);
			break;
		case DoneKV:
			Q_GotoPage(gpQwpVar->GotoAct_Done,gpQwpVar->GotoName_Done,0,NULL);
			break;	
		case LeftArrowKV:

			break;
		case ListKV:
			if(InEvent==Tch_Press)
			{
				Key=((pTouchInfo->y-DEVICE_INFO_START_Y)/(DEVICE_INFO_H+DEVICE_INFO_LIST_SPACE))+1;
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
				{
					if(gpQwpVar->ClientRecord[i].DispIdx==Key)
					{
						gpQwpVar->NowPressKey=Key;//记录按下的项
						DrawDeviceInfo(Key,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,PressDisp);
						break;
					}
				}
			}
			else if(InEvent==Tch_Release)
			{
				Key=((pTouchInfo->y-DEVICE_INFO_START_Y)/(DEVICE_INFO_H+DEVICE_INFO_LIST_SPACE))+1;
				if(Key==gpQwpVar->NowPressKey)//在同一项上释放了，此按下动作有效
				{
					for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++) //还原此项，进入处理页面
					{
						if(gpQwpVar->ClientRecord[i].DispIdx==Key)
						{
							gpQwpVar->ClientRecord[i].IsHiLight=FALSE;
							DrawDeviceInfo(Key,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
							if(gpQwpVar->GotoName_List&&gpQwpVar->GotoName_List[0])//允许进入子页面
							{//进入子页面前先查询名字
								gpQwpVar->SelectedAddr=gpQwpVar->ClientRecord[i].Addr;
								QWA_QueryName(gpQwpVar->SelectedAddr);
								gpQwpVar->SelectedTimeMs=OS_GetCurrentSysMs();
								//在Perip_QWebQueryName事件里面执行goto page动作
							}
							break;
						}
					}
				}
				else//在其他项的位置释放了，此按下动作无效
				{
					for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//还原此项显示
					{
						if(gpQwpVar->ClientRecord[i].DispIdx==gpQwpVar->NowPressKey)
						{
							if(gpQwpVar->ClientRecord[i].IsHiLight==TRUE)
								DrawDeviceInfo(gpQwpVar->NowPressKey,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,HighLightDisp);
							else 
								DrawDeviceInfo(gpQwpVar->NowPressKey,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
						}
					}
				}
				gpQwpVar->NowPressKey=0xff;
			}
			else if(InEvent==Tch_ReleaseVain)//在超出list的区域释放了，无效
			{
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//还原此项显示
				{
					if(gpQwpVar->ClientRecord[i].DispIdx==gpQwpVar->NowPressKey)
					{
						if(gpQwpVar->ClientRecord[i].IsHiLight==TRUE)
							DrawDeviceInfo(gpQwpVar->NowPressKey,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,HighLightDisp);
						else 
							DrawDeviceInfo(gpQwpVar->NowPressKey,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
					}
				}
				gpQwpVar->NowPressKey=0xff;
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
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}

static CO_MSG YesNoHandler(u8 ObjID,bool NowValue)
{
	switch(ObjID)
	{
		case 1:
			if(NowValue==TRUE) 
			{
				QWA_StartQWeb();
			}
			else
			{
				
				u8 i;
				GUI_REGION DrawRegion;
				
				QWA_StopQWeb();
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//检查主机还在不在
				{
					if(gpQwpVar->ClientRecord[i].Addr!=0)
					{
						DeleteOneDevice(gpQwpVar->ClientRecord[i].Addr);
					}
				}
					
				//状态文字
				DrawRegion.x=10;
				DrawRegion.y=34;
				DrawRegion.w=160;
				DrawRegion.h=16;
				DrawRegion.Space=0x00;
				DrawRegion.Color=QWEB_PAGE_BG_COLOR;
				Gui_FillBlock(&DrawRegion);
				DrawRegion.Color=QWEB_PAGE_FONT_COLOR;
				Gui_DrawFont(GBK16_FONT,"Q Web State",&DrawRegion);
			}
			break;
	}
	
	return 0;
}


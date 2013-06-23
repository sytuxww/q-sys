/**************** Q-SYS *******************
 * PageName : AppListPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "AppListPage.h"
#include "SettingsPage.h"
#include "FileListPage.h"
#include "KeyBoardPage.h"
#include "QWebPage.h"

//��������
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

bool UseUsbOutputInfoFlag=FALSE;

//-----------------------��ҳϵͳ����������-----------------------
//����ҳ�水����Ҫ�õ���ö�٣�����������Ψһ�ĺ궨��
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//�����Ӳ���汾��֧�ָ����ⲿ�жϰ����������߼��̣�
	//���Դ�����ֵ��USER_KEY_VALUE_START��ʼ����ǰ��ļ�ֵ��������
	RetMainKV=USER_KEY_VALUE_START,
	SysSettingsKV,
	ClockSettingKV,
	AlarmSettingKV,

	FileScanKV,
	WirelessKV,
	TestPageKV,
	CommandKV,

	ContactsKV,
	DrawKV,
	GameKV,
	TouchChkKV,

	NewsKV,
	PianistKV,
	TalkKV,
	WeatherKV,
	
	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}AppListPage_OID;

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
//֧�ֵ�������������ΪMAX_TOUCH_REGION_NUM
//ϵͳ��ʾ�ʹ�������������ϵ������Ļ���Ͻ�Ϊԭ��(x 0,y 0)�����½�Ϊ(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Ret",		RetMainKV,RelMsk|BinMsk,4,35,55,50,0,0,"RetMain",FatColor(NO_TRANS)},
	{"Settings",	SysSettingsKV,RelMsk|BinMsk,63,35,55,50,0,0,"Setting",FatColor(NO_TRANS)},
	{"Clock",	ClockSettingKV,RelMsk|BinMsk,122,35,55,50,0,0,"Clock",FatColor(NO_TRANS)},
	{"Alarm",	AlarmSettingKV,RelMsk|BinMsk,181,35,55,50,0,0,"Alarm",FatColor(NO_TRANS)},

	{"Files",	FileScanKV,RelMsk|BinMsk,4,104,55,50,0,0,"FileScan",FatColor(NO_TRANS)},
	{"Wireless",	WirelessKV,RelMsk|BinMsk,63,104,55,50,0,0,"Wireless",FatColor(NO_TRANS)},
	{"Test",	TestPageKV,RelMsk|BinMsk,122,104,55,50,0,0,"Test",FatColor(NO_TRANS)},
	{"CMD",	CommandKV,RelMsk|BinMsk,181,104,55,50,0,0,"Command",FatColor(NO_TRANS)},

	{"Contacts",	ContactsKV,RelMsk|BinMsk,4,173,55,50,0,0,"Contact",FatColor(NO_TRANS)},
	{"Draw",	DrawKV,RelMsk|BinMsk,63,173,55,50,0,0,"Draw",FatColor(NO_TRANS)},
	{"Game",	GameKV,RelMsk|BinMsk,122,173,55,50,0,0,"Game",FatColor(NO_TRANS)},
	{"Touch",	TouchChkKV,RelMsk|BinMsk,181,173,55,50,0,0,"TouchChk",FatColor(NO_TRANS)},

	{"News",	NewsKV,RelMsk|BinMsk,4,242,55,50,0,0,"Map",FatColor(NO_TRANS)},
	{"Pianist",	PianistKV,RelMsk|BinMsk,63,242,55,50,0,0,"Pianist",FatColor(NO_TRANS)},
	{"Talk",	TalkKV,RelMsk|BinMsk,122,242,55,50,0,0,"Talk",FatColor(NO_TRANS)},
	{"Weather",	WeatherKV,RelMsk|BinMsk,181,242,55,50,0,0,"Weather",FatColor(NO_TRANS)},

	//Һ�����������ʾ������ĸ���
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

static const CHAR_BUTTON_OBJ CharButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,
		//Char_x,Char_y,MarginXY,SpaceXY,NormalColor,NormalBG,PressColor,PressBG,ReleaseColor,ReleaseBG},
	{"��������",RetMainKV,RoueMsk|RelMsk,4,85,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"ϵͳ����",SysSettingsKV,RoueMsk|RelMsk,63,85,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"ʱ������",ClockSettingKV,RoueMsk|RelMsk,122,85,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"��������",AlarmSettingKV,RoueMsk|RelMsk,181,85,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},

	{"�ļ����",FileScanKV,RoueMsk|RelMsk,4,154,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"����ͨ��",WirelessKV,RoueMsk|RelMsk,63,154,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"����ҳ��",TestPageKV,RoueMsk|RelMsk,122,154,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"������",CommandKV,RoueMsk|RelMsk,181+6,154,44,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},

	{"ͨ��¼",ContactsKV,RoueMsk|RelMsk,4+6,223,44,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"��ͼ",DrawKV,RoueMsk|RelMsk,63+12,223,32,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"��Ϸ",GameKV,RoueMsk|RelMsk,122+12,223,32,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"����У��",TouchChkKV,RoueMsk|RelMsk,181,223,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},

	{"������Ѷ",NewsKV,RoueMsk|RelMsk,4,292,55,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"����",PianistKV,RoueMsk|RelMsk,63+12,292,32,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"����",TalkKV,RoueMsk|RelMsk,122+12,292,32,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
	{"����",WeatherKV,RoueMsk|RelMsk,181+12,292,32,15,
		4,2,0x00,0x00,FatColor(0x000000),FatColor(0xe0e0e0),FatColor(0xffffff),FatColor(0x333333),FatColor(0x000000),FatColor(0xe0e0e0)},
};


//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE AppListPage={
	"AppListPage",
	"YuanYin",
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
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|Bit(Perip_UartInput),
	ButtonHandler,
	
};

//-----------------------��ҳ�Զ����������-----------------------
typedef enum{
	OSID_NULL=0,//ID�Ų���Ϊ0
	OSID_SYS,
	OSID_CLOCK,
	OSID_ALARM,
}OPTIONS_ID;

typedef enum{
	SYSOP_UseUsbOutput=1,//���ܴ�0��ʼ
	SYSOP_LightTime,
	SYSOP_LightScale,
	
	SYSOP_RtcYe,
	SYSOP_RtcMo,
	SYSOP_RtcDa,
	SYSOP_RtcHo,
	SYSOP_RtcMi,
	SYSOP_RtcSe,

	SYSOP_AlarmYe,
	SYSOP_AlarmMo,
	SYSOP_AlarmDa,
	SYSOP_AlarmHo,
	SYSOP_AlarmMi
}SYS_SETTINGS_ITEM;

//-----------------------��ҳ�Զ��庯��-----------------------
static void ModifySysSettings(void *OptionsBuf)
{
	u8 i,id,Total;
	s32 Val;
	bool DbFlag=FALSE;

	if(SP_GetOptionsTotal(OptionsBuf,&Total)==FALSE) return;
	for(i=1;i<=Total;i++)//�����1��ʼ����
	{
		SP_GetOptionID(OptionsBuf,i,&id);
		switch(id)
		{
			case SYSOP_UseUsbOutput:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
				SP_GetYesOrNoOption(OptionsBuf,i,&UseUsbOutputInfoFlag);
				break;
			case SYSOP_LightTime:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
				SP_GetNumOption(OptionsBuf,i,&Val);
				Q_DB_SetValue(Setting_BgLightTime,Val,NULL,0);
				DbFlag=TRUE;
				//Q_DB_BurnToSpiFlash();//�������ݿ�
				break;
			case SYSOP_LightScale:
				if(SP_IsModify(OptionsBuf,i)==FALSE) continue;//δ�޸�
				SP_GetNumOption(OptionsBuf,i,&Val);
				Gui_SetBgLight(Val);//��Ч
				Q_DB_SetValue(Setting_BgLightScale,Val,NULL,0);
				DbFlag=TRUE;
				//Q_DB_BurnToSpiFlash();//�������ݿ�
				break;
		}					
	}

	if(DbFlag) Q_DB_BurnToSpiFlash();//�������ݿ�
}

static void ModifyClock(void *OptionsBuf)
{
	u8 i,id,Total;
	s32 Val;
	RTC_TIME Time;
	bool TimeFlag=FALSE;

	if(SP_GetOptionsTotal(OptionsBuf,&Total)==FALSE) return;
	for(i=1;i<=Total;i++)//�����1��ʼ����
	{
		SP_GetOptionID(OptionsBuf,i,&id);
		switch(id)
		{
			case SYSOP_RtcYe:
				TimeFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				Time.year=Val;
				break;
			case SYSOP_RtcMo:
				TimeFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				Time.mon=Val;
				break;
			case SYSOP_RtcDa:
				TimeFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				Time.day=Val;
				break;
			case SYSOP_RtcHo:
				TimeFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				Time.hour=Val;
			case SYSOP_RtcMi:
				TimeFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				Time.min=Val;
				break;
			case SYSOP_RtcSe:
				TimeFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				Time.sec=Val;
				break;						
		}					
	}

	if(TimeFlag)
	{	
		Debug("RTC set!%d��%d��%d�� %dʱ%d��%d��,%d\n\r",Time.year,Time.mon,Time.day,Time.hour,Time.min,Time.sec,Time.week);

		if(RTC_Adjust(&Time,RtcOp_SetTime)==TRUE)
		{
			Debug("RTC set sucess!\n\r");
		}
		else
		{
			Debug("RTC set error!%d %d %d:%d-%d %d\n\r",Time.year,Time.mon,Time.day,Time.hour,Time.min,Time.sec);
		}					
	}
}

static void SetAlarm(void *OptionsBuf)
{
	u8 i,id,Total;
	s32 Val;
	RTC_TIME AlarmTime;
	bool AlarmFlag=FALSE;

	if(SP_GetOptionsTotal(OptionsBuf,&Total)==FALSE) return;
	for(i=1;i<=Total;i++)//�����1��ʼ����
	{
		SP_GetOptionID(OptionsBuf,i,&id);
		switch(id)
		{
			case SYSOP_AlarmYe:
				AlarmFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				AlarmTime.year=Val;
				break;
			case SYSOP_AlarmMo:
				AlarmFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				AlarmTime.mon=Val;
				break;
			case SYSOP_AlarmDa:
				AlarmFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				AlarmTime.day=Val;
				break;
			case SYSOP_AlarmHo:
				AlarmFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				AlarmTime.hour=Val;
			case SYSOP_AlarmMi:
				AlarmFlag|=SP_IsModify(OptionsBuf,i);
				SP_GetNumOption(OptionsBuf,i,&Val);
				AlarmTime.min=Val;
				break;				
		}					
	}

	if(AlarmFlag)
	{	
		Debug("Alarm set!%d��%d��%d�� %dʱ%d��\n\r",AlarmTime.year,AlarmTime.mon,AlarmTime.day,AlarmTime.hour,AlarmTime.min);

		if(RTC_Adjust(&AlarmTime,RtcOp_SetAlarm)==TRUE)
		{
			Debug("Alarm set sucess!\n\r");
		}
		else
		{
			Debug("Alarm set error!%d %d %d:%d-%d %d\n\r",AlarmTime.year,AlarmTime.mon,AlarmTime.day,AlarmTime.hour,AlarmTime.min,AlarmTime.sec);
		}					
	}
}

//-----------------------��ҳϵͳ����----------------------

//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	GUI_REGION DrawRegion;

	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�				
		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return

#if 1
			//��ģ�ⱳ�����������Կ�����ʾ
			DrawRegion.x=0;
			DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=320;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_DrawImgBin("Theme/F/AppListPage/Bg/Bg.bin",&DrawRegion);	
#else
			//������
			DrawRegion.x=0;
			DrawRegion.y=21;
			DrawRegion.w=240;
			DrawRegion.h=320-21;
			DrawRegion.Color=FatColor(0x8b8a8a);
			Gui_FillBlock(&DrawRegion);
			
			//��������
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"Applications",(240-strlen("Applications")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("Applications"),FatColor(0xe0e0e0));//д����
			
			//����
			DrawFrame1(25,291);	
#endif			
			break;
		
		case Sys_TouchSetOk:
			break;
		case Sys_TouchSetOk_SR:
			if(strcmp((void *)Q_GetPageByTrack(1)->Name,"SettingsPage")==0)//������ҳ�淵��
			{//������ҳ�淵��ʱ��IntParamΪ�������ѡ��ͷʱ�����ID�ţ�pSysParamΪ���������Ļ���
				switch(IntParam)
				{
					case OSID_NULL://��IntParamΪ0ʱ����ʾû���޸��κ�ֵ�����Ե��������ID����Ϊ0��������״̬��ʧ
						break;
					case OSID_SYS:
						ModifySysSettings(pSysParam);
						break;
					case OSID_CLOCK:
						ModifyClock(pSysParam);
						break;
					case OSID_ALARM:
						SetAlarm(pSysParam);
						break;
				}

				Q_PageFree(pSysParam);//������ҳ�淵�ر����ͷŵ�������ʱ������ڴ棬��������й©
			}	
			else if(strcmp((void *)Q_GetPageByTrack(1)->Name,"FileListPage")==0)//���ļ����ҳ�淵��
			{
				Debug("FileScan Return,Select %s\n\r",(u8 *)pSysParam);
				Q_PageFree(pSysParam);//���ļ��б�ҳ�淵�ر����ͷŵ�������ʱ������ڴ棬��������й©
			}
			else if(strcmp((void *)Q_GetPageByTrack(1)->Name,"KeyBoardPage")==0)//�Ӽ���ҳ�淵��
			{
				Debug("KeyBoard Return,input:%s\n\r",(u8 *)pSysParam);
				Q_PageFree(pSysParam);//�Ӽ���ҳ�淵�ر����ͷŵ�������ʱ������ڴ棬��������й©
			}
			break;
		case Sys_PageClean:
		case Sys_PreSubPage:
		
			break;
		default:
			//��Ҫ��Ӧ���¼�δ����
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
					//Q_GotoPage(GotoNewPage,"MainPage",-1,NULL);
					//PrtScreen();
					
					break;
				case ExtiKeyUp:
					break;
				case ExtiKeyDown:	
					break; 
			}break;
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
	switch(Key)
	{
		case RetMainKV:
			Q_GotoPage(GotoNewPage,"MainPage",0,NULL);
			break;
		case SysSettingsKV:
			{//���ѡ�settings page��ʾ��
				void *p;
				p=Q_PageMallco(1000);//�������ڴ�ռ䣬���ڴ�ռ����Ϊ����ҳ��ʱSystemEventHandler��ָ�����
				SP_AddOptionsHeader(p,1000,"System Settings",OSID_SYS);//���������ͷѡ�ע�����ΨһID(����Ϊ0)
				SP_AddYesOrNoOption(p,SYSOP_UseUsbOutput,UseUsbOutputInfoFlag,"USB���ʹ��","�����Ƿ�ʹ��USBģ�⴮�����������Ϣ");//���һ��yes noѡ��
				SP_AddNumListOption(p,SYSOP_LightTime,Q_DB_GetValue(Setting_BgLightTime,NULL),0,120,5,"����ʱ��","����ϵͳ�رձ����ʱ�䣬0Ϊ��Զ����");//���һ��num listѡ��
				SP_AddNumListOption(p,SYSOP_LightScale,Q_DB_GetValue(Setting_BgLightScale,NULL),5,100,5,"����ǿ��","����ϵͳ����ǿ��");//�����һ��num listѡ��
				Q_GotoPage(GotoSubPage,"SettingsPage",TRUE,p);//ok���Ӻ��ˣ����Խ���settings page�ˡ�
			}
			break;
		case ClockSettingKV:
			{
				void *p;
				RTC_TIME Time;
				p=Q_PageMallco(1000);
				SP_AddOptionsHeader(p,1000,"Set Clock",OSID_CLOCK);
				RTC_GetTime(&Time);
				SP_AddNumListOption(p,SYSOP_RtcYe,Time.year,1912,2035,1,"ϵͳʱ��-��","����ϵͳʵʱʱ��");
				SP_AddNumListOption(p,SYSOP_RtcMo,Time.mon,1,12,1,"ϵͳʱ��-��","");
				SP_AddNumListOption(p,SYSOP_RtcDa,Time.day,1,31,1,"ϵͳʱ��-��","");
				SP_AddNumListOption(p,SYSOP_RtcHo,Time.hour,0,23,1,"ϵͳʱ��-Сʱ","");
				SP_AddNumListOption(p,SYSOP_RtcMi,Time.min+1,0,59,1,"ϵͳʱ��-��","");
				SP_AddNumListOption(p,SYSOP_RtcSe,0,0,59,1,"ϵͳʱ��-��","");
				Q_GotoPage(GotoSubPage,"SettingsPage",TRUE,p);
			}
			break;
		case AlarmSettingKV:
			{
				void *p;
				RTC_TIME Time;
				p=Q_PageMallco(1000);
				SP_AddOptionsHeader(p,1000,"Set Alarm",OSID_ALARM);
				RTC_GetTime(&Time);
				SP_AddNumListOption(p,SYSOP_AlarmYe,Time.year,1912,2035,1,"��ʱʱ��-��","��������(��������)");
				SP_AddNumListOption(p,SYSOP_AlarmMo,Time.mon,1,12,1,"��ʱʱ��-��","");
				SP_AddNumListOption(p,SYSOP_AlarmDa,Time.day,1,31,1,"��ʱʱ��-��","");
				SP_AddNumListOption(p,SYSOP_AlarmHo,Time.hour,0,23,1,"��ʱʱ��-Сʱ","");
				SP_AddNumListOption(p,SYSOP_AlarmMi,Time.min,0,59,1,"��ʱʱ��-��","");
				Q_GotoPage(GotoSubPage,"SettingsPage",TRUE,p);
			}
			break;
		case FileScanKV:
			{
				void *pPath=Q_PageMallco(1024);
				FILELIST_SET FileListParam;
				FileListParam.CallBackRid=Q_FindRidByPageName(NULL);
				((u8 *)pPath)[0]=0;
				FileListParam.pPathBuf=pPath;
				FileListParam.pSuffixStr="";Debug("p:%x\n\r",(u32)pPath);
				if(Q_GotoPage(GotoSubPage,"FileListPage",FL_SelectOne|FL_NoListBuf,&FileListParam)&SM_NoGoto)
				{
					Q_PageFree(pPath);//�������ʧ�ܣ����ͷ���Դ
				}
			}
			break;
		case TestPageKV:
			{
				void *p;
				p=Q_PageMallco(1024);
				sprintf(p,"<Q-SYS Input Page>\n\r�汾: 2.1\n\r����: ChenYu\n\rQQ: 409262796");
				Q_GotoPage(GotoSubPage,"KeyBoardPage",1024,p);
			}
			break;
			//Q_GotoPage(GotoNewPage,"TestPage",0,NULL);
		case WirelessKV:
			{
				QWEB_PAGE_SET QWebPageSet;

				sprintf((void *)QWebPageSet.PageTitle,"Select One To Chat");
				QWebPageSet.GotoAct_List=GotoSubPage;
				sprintf((void *)QWebPageSet.GotoName_List,"ChatPage");
				QWebPageSet.GotoAct_Back=GotoNewPage;
				sprintf((void *)QWebPageSet.GotoName_Back,"AppListPage");
				QWebPageSet.GotoAct_Done=GotoNewPage;
				sprintf((void *)QWebPageSet.GotoName_Done,"AppListPage");
				Q_GotoPage(GotoNewPage,"QWebPage",0,&QWebPageSet);
			}
			break;
		case TouchChkKV:
			Q_GotoPage(GotoNewPage,"TouchCheckPage",0,NULL);
			break;	
		case NewsKV:
			{
				QWEB_PAGE_SET QWebPageSet;

				sprintf((void *)QWebPageSet.PageTitle,"Select View Server");
				QWebPageSet.GotoAct_List=GotoSubPage;
				sprintf((void *)QWebPageSet.GotoName_List,"NewsPage");
				QWebPageSet.GotoAct_Back=GotoNewPage;
				sprintf((void *)QWebPageSet.GotoName_Back,"AppListPage");
				QWebPageSet.GotoAct_Done=GotoNewPage;
				sprintf((void *)QWebPageSet.GotoName_Done,"AppListPage");
				Q_GotoPage(GotoNewPage,"QWebPage",0,&QWebPageSet);
			}
			break;
		case GameKV:
			Q_GotoPage(GotoNewPage,"SnakePage",0,NULL);
			break;		
		case CommandKV:
			Q_GotoPage(GotoNewPage,"CtrlObjDemoPage",0,NULL);
			break;
		case DrawKV:
			Q_GotoPage(GotoNewPage,"WaveDispPage",0,NULL);
			break;
		case HomeKV:
			break;
		case MessageKV:
		case MusicKV:
		case PepoleKV:
			break;
		default:
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}




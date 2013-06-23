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

//��������
static SYS_MSG SystemHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);//ϵͳ�¼�������
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);//�����¼�������
static CO_MSG ButtonHandler(u8 OID,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);//��ť�ؼ�������
static CO_MSG YesNoHandler(u8 OID,bool NowValue);//yes no�ؼ�������
static CO_MSG NumCtrlObjHandler(u8 OID,s32 Value,void *pNumCtrlObj);//num�ؼ�������
static CO_MSG StrCtrlObjHandler(u8 OID,u8 StrID,u8 *Str,void *pStrCtrlObj);//str�ؼ�������

//-----------------------��ҳϵͳ����������-----------------------
//����ҳ�水����Ҫ�õ���ö�٣�����������Ψһ�ĺ궨��
typedef enum
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,
	
	//�����Ӳ���汾��֧�ָ����ⲿ�жϰ����������߼��̣�
	//���Դ�����ֵ��USER_KEY_VALUE_START��ʼ����ǰ��ļ�ֵ��������
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

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
//֧�ֵ�������������ΪMAX_TOUCH_REGION_NUM
//ϵͳ��ʾ�ʹ�������������ϵ������Ļ���Ͻ�Ϊԭ��(x 0,y 0)�����½�Ϊ(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Back",	BackKV,RelMsk|PathMsk,3,287,54,31,0,0,"Common/Btn/Back",FatColor(NO_TRANS)},
	{"<<",		LeftArrowKV,RelMsk|PathMsk,65,287,39,31,0,0,"Common/Btn/LeftArr",FatColor(NO_TRANS)},
	//{"Dot",		DotKV,RelMsk|PathMsk,109,292,22,22,0,0,"Common/Btn/Dot",FatColor(NO_TRANS)},
	{">>",		RightArrowKV,RelMsk|PathMsk,136,287,39,31,0,0,"Common/Btn/RightArr",FatColor(NO_TRANS)},
	{"Done",	DoneKV,RelMsk|PathMsk,183,287,54,31,0,0,"Common/Btn/Done",FatColor(NO_TRANS)},
	
	//Һ�����������ʾ������ĸ���
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

static const CHAR_BUTTON_OBJ CharButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,
		//Char_x,Char_y,MarginXY,SpaceXY,NormalColor,NormalBG,PressColor,PressBG,ReleaseColor,ReleaseBG},
	{"���ֺ�",21,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,18,83,50,19,
		7,4,0x22,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
	{"������",22,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,69,83,50,19,
		7,4,0x22,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
	{"ͼƬ",23,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,120,83,50,19,
		14,4,0x82,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
	{"����",24,RoueMsk|PrsMsk|CotMsk|RelMsk|ReVMsk,171,83,50,19,
		13,4,0x82,0x00,FatColor(0xffffff),FatColor(0x666666),FatColor(0xffffff),FatColor(0x333333),FatColor(0xffffff),FatColor(0x666666)},
};

//����ҳ�����Ӧ�õ����Լ�
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

//-----------------------��ҳ�Զ����������-----------------------
typedef struct{
	bool Tip;
	u32 GlobaU32Value;
	void *pGlobaPointer;
}TestPage_VARS;//��ҳ��Χ�ڵ�ȫ�ֱ���ȫ������������ṹ�����棬��ͨ��gpTestPageVars���ʣ��ɽ�ʡRAMռ��
static TestPage_VARS *gpTestPageVars;

//-----------------------��ҳ�Զ��庯��-----------------------

//-----------------------��ҳϵͳ����----------------------

//����ϵͳ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)//ϵͳ�¼�������
{
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			break;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�	
			gpTestPageVars=Q_PageMallco(sizeof(TestPage_VARS));//Ϊ��ҳ���ȫ�ֱ�������ռ�
		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return
			gpTestPageVars->Tip=FALSE;

			//��������
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"TestPage",(240-strlen("TestPage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("TestPage"),FatColor(0xe0e0e0));//д����
			
			//������
			DrawRegion.x=0;
			DrawRegion.y=21;
			DrawRegion.w=240;
			DrawRegion.h=320-21-39;
			DrawRegion.Color=FatColor(0x8b8a8a);
			Gui_FillBlock(&DrawRegion);

			//������
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);

			//����
			DrawFrame1(25,252);	

			//����
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
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s SystemEventHandler:This System Event Handler case unfinish! SysEvent:%d\n\r",Q_GetCurrPageName(),SysEvent);
			//while(1);
	}
	
	return 0;
}

//��ʵ�尴�������ڡ�rtc�������ж���ʱ���ᴥ���˺���
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)//�����¼�������
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
			if((IntParam>>16)==1)//����1
			{
				Q_Sh_CmdHandler(IntParam&0xffff,pParam);
			}
			break;
	}

	return 0;
}

//��ʹ���߰��±�ҳTouchRegionSet�ﶨ��İ���ʱ���ᴥ�����������Ķ�Ӧ�¼�
static CO_MSG ButtonHandler(u8 OID,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)//��ť�ؼ�������
{		
	//GUI_REGION DrawRegion;
	
	switch(OID)
	{
		
		case BackKV:
			Q_GotoPage(GotoNewPage,Q_GetPageByTrack(1)->Name,0,NULL);//����ǰһ��ҳ��
			break;
		case DoneKV:
			Q_GotoPage(GotoNewPage,Q_GetPageByTrack(1)->Name,0,NULL);//����ǰһ��ҳ��
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
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! OID:%d\n\r",Q_GetCurrPageName(),OID);
			///while(1);
	}
	
	return 0;
}

//yes no�ؼ������ĺ���
static CO_MSG YesNoHandler(u8 OID,bool NowValue)//yes no�ؼ�������
{

	return 0;
}

//num�ؼ������ĺ���
static CO_MSG NumCtrlObjHandler(u8 OID,s32 Value,void *pNumCtrlObj)//num�ؼ�������
{

	return 0;
}

//str�ؼ������ĺ���
static CO_MSG StrCtrlObjHandler(u8 OID,u8 StrID,u8 *Str,void *pStrCtrlObj)//str�ؼ�������
{

	return 0;
}



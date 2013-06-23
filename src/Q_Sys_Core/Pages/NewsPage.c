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
	{"����",Q_ShareweKv,F16Msk|RoueMsk|RelMsk,4,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
	{"��Ѷ",InfomationKv,F16Msk|RoueMsk|RelMsk,63,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
	{"����",EntertainmentKv,F16Msk|RoueMsk|RelMsk,122,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
	{"�ƾ�",FinanceKv,F16Msk|RoueMsk|RelMsk,181,30,53,25,
		11,4,0x22,0x00,FatColor(0xffffff),FatColor(0x999999),FatColor(0xffffff),FatColor(0xff8a00),FatColor(0xffffff),FatColor(0x777777)},
};

//����ҳ�����Ӧ�õ����Լ�
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

//-----------------------��ҳ�Զ����������-----------------------
#define NEWS_HOST_URL	"Q-ShareWe.idoing.org"
#define NEWS_TXT_DISP_X 8
#define NEWS_TXT_DISP_Y 60
#define NEWS_TXT_DISP_W 224
#define NEWS_TXT_DISP_H 208
#define NEWS_TXT_DISP_SPACE 0x00
#define NEWS_TXT_DISP_COLOR FatColor(0x000000)
#define NEWS_TXT_DISP_BG FatColor(0xe0e0e0)

typedef struct{
	u8 ViewAddr;//view��������ַ
	u8 HostOnlineFlag;//web���������߱�־
	NewsPage_OID NowTab;//��ǰ��ǩ
	GUI_REGION TxtRegion;
}NEWS_PAGE_VARS;
static NEWS_PAGE_VARS *gNspVars=NULL;

//-----------------------��ҳ�Զ��庯��-----------------------
enum{
	TIP_GRAY=0,
	TIP_GREEN,
	TIP_GREEN_FLASH,
	TIP_RED,
	TIP_RED_FLASH,
};
static void TipState(u8 Act)//��״̬��
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

//������ǩ
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
	VC_CONN='c',//��������
	VC_ONLINE='o',//��������
	VC_DOWNLINE='d',//����������
	VC_HGET='g',//��ȡ�ļ�
	VC_HFILE='f',//�ش��ļ�
}VIEW_CMD;

typedef struct{
	u8 ChkCode;//must = v
	VIEW_CMD Cmd;
	u8 Data[1];
}VIEW_CMD_STRUCT;

//����view����
static void SendViewCmd(VIEW_CMD Cmd,u8 *pStr)
{
	VIEW_CMD_STRUCT *pView;

	if(gNspVars==NULL) return;
	if(Cmd != VC_CONN && gNspVars->HostOnlineFlag != TRUE) return;

	pView=Q_PageMallco(sizeof(VIEW_CMD_STRUCT)+strlen((void *)pStr));//��send ok���ͷ�
	pView->ChkCode='v';
	pView->Cmd=Cmd;
	strcpy((void *)pView->Data,(void *)pStr);
	QWA_SendData(gNspVars->ViewAddr,sizeof(VIEW_CMD_STRUCT)+strlen((void *)pStr),(u8 *)pView);
}

//����view����
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
			HighLightTable(gNspVars->NowTab);//������ǩ
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
			Gui_FillBlock(&gNspVars->TxtRegion);//��䱳��
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_COLOR;
			Gui_DrawFont(GBK12_FONT,pView->Data,&gNspVars->TxtRegion);
			break;
	}
}

//������
static void DrawInitBg(void)
{
	GUI_REGION DrawRegion;

	//��������
	DrawRegion.x=DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=21;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
	DrawTitle1(ASC14B_FONT,"NewsPage",(240-strlen("NewsPage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("NewsPage"),FatColor(0xe0e0e0));//д����
	
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

	TipState(TIP_GRAY);

	gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
	Gui_FillBlock(&gNspVars->TxtRegion);
}
//-----------------------��ҳϵͳ����----------------------

//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(IntParam<QW_ADDR_HOST || IntParam>QW_ADDR_MAX) return SM_NoGoto;
			break;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�				
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

			//��ʾ��Ϣ
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_BG;
			Gui_FillBlock(&gNspVars->TxtRegion);//��䱳��
			gNspVars->TxtRegion.Color=NEWS_TXT_DISP_COLOR;
			Gui_DrawFont(GBK12_FONT,"�·��в�Բ��Ϊ������ʾ�Ѿ���ȷ���ӵ�VIEW���ż�Զ��Web������������Ե����ǩ������ȡ���š���Բ��Ϊ��ɫ�����ظ������ҳ�档",&gNspVars->TxtRegion);
			break;
		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			if(gNspVars!=NULL) Q_PageFree(gNspVars);
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

//��ʹ���߰��±�ҳTouchRegionSet�ﶨ��İ���ʱ���ᴥ�����������Ķ�Ӧ�¼�
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{		
	switch(Key)
	{
		case BackKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//����ǰһ��ҳ��
			break;
		case DoneKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//����ǰһ��ҳ��
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
			//��Ҫ��Ӧ���¼�δ����
			Debug("%s ButtonHandler:This Touch Event Handler case unfinish! Key:%d\n\r",Q_GetCurrPageName(),Key);
			///while(1);
	}
	
	return 0;
}




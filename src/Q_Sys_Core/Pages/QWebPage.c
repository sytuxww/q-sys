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

//��������
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);
static CO_MSG YesNoHandler(u8 ObjID,bool NowValue);

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
	ListKV,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}QWebPage_OID;

//��ҳ�궨��
#define QWEB_PAGE_BG_COLOR FatColor(0xe0e0e0)//ҳ�汳��ɫ
#define QWEB_PAGE_FONT_COLOR FatColor(0x000000)//ҳ������ɫ
#define QWEB_PAGE_SELECT_ACT_WAITING_MS	3000 //����б���֮�����ȴ��豸��Ӧʱ��
#define DEVICE_INFO_MAX_CLIENT_RECORD 7//����¼20��client
#define DEVICE_INFO_MAX_LIST_NUM 7//һҳ�����ʾ7����Ϣ
#define DEVICE_NAME_MAX_LEN	16//���ֳ���
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

	//device list region
	{"", ListKV,PrsMsk|RelMsk|ReVMsk,DEVICE_INFO_START_X,DEVICE_INFO_START_Y,DEVICE_INFO_W,DEVICE_INFO_MAX_LIST_NUM*(DEVICE_INFO_H+DEVICE_INFO_LIST_SPACE),
			0,0,"",FatColor(NO_TRANS)},
	
	//Һ�����������ʾ������ĸ���
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

//����ҳ�����Ӧ�õ����Լ�
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

//-----------------------��ҳ�Զ����������-----------------------
typedef struct{
	u8 Addr;//��¼��ַ
	u8 DispIdx;//��¼�ŵ��б�ڼ���λ�ã���1��ʼ
	bool IsHiLight;//�豸���Ƿ����
	u16 NameChk;//����У����
	u8 Name[DEVICE_NAME_MAX_LEN];//��¼����
}CLIENT_RECORD;

typedef struct{
	YES_NO_OBJ YesNo;//q web ����
	u8 NowDispNum;// ��ǰ�б���ʾ��Ŀ��from 0 start
	u8 NowPressKey;//��ǰ���µ��б����ţ�û��ʱΪ0xff
	CLIENT_RECORD ClientRecord[DEVICE_INFO_MAX_CLIENT_RECORD];//q���豸��¼
	u8 SelectedAddr;//ѡ���ĵ�ַ��û��ʱΪ0
	u32 SelectedTimeMs;//ѡ���б����ʱ��

	u8 PageTitle[32];//ҳ�����
	PAGE_ACTION GotoAct_List;//�����б�������Ժ��ַ�ʽ�������ҳ��
	u8 GotoName_List[20];//�����б����������ҳ��
	PAGE_ACTION GotoAct_Back;//����back�����Ժ��ַ�ʽ�������ҳ��
	u8 GotoName_Back[20];//����back��������ҳ��
	PAGE_ACTION GotoAct_Done;//����done�����Ժ��ַ�ʽ�������ҳ��
	u8 GotoName_Done[20];//����done��������ҳ��
}QWEB_PAGE_VARS;
static QWEB_PAGE_VARS *gpQwpVar=NULL;
static QWEB_PAGE_SET *gpQwpSet=NULL;

//-----------------------��ҳ�Զ��庯��-----------------------
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

//-----------------------��ҳϵͳ����----------------------
static void DrawState(void)//���µ�ǰq��״̬��ʾ
{
	GUI_REGION DrawRegion;
	u8 Str[32];

	if(Q_GetPageByTrack(0)!=(&QWebPage)) return;//���Ƿ��ڱ�ҳ��
	
	//q������
	DrawRegion.x=10;
	DrawRegion.y=34;
	DrawRegion.w=160;
	DrawRegion.h=16;
	DrawRegion.Space=0x00;
	DrawRegion.Color=QWEB_PAGE_BG_COLOR;
	Gui_FillBlock(&DrawRegion);
	DrawRegion.Color=QWEB_PAGE_FONT_COLOR;
	if(QWA_GetMyAddr()==QW_ADDR_DEF)//δ����
	{
		Gui_DrawFont(GBK16_FONT,"Q Web State",&DrawRegion);
	}
	else if(QWA_GetMyAddr()==QW_ADDR_HOST)//����
	{
		sprintf((void *)Str,"[H%d]%s",QWA_GetMyAddr(),QWA_MyQWebName(NULL));
		Gui_DrawFont(GBK16_FONT,Str,&DrawRegion);
	}
	else//�ӻ�
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


//��ʾһ����¼��Ϣ
//Idx��1��ʼ�����DEVICE_INFO_MAX_LIST_NUM
enum{
	NormalDisp=0,//��ͨ��ʾ
	PressDisp,//������ʾ
	HighLightDisp,//������ʾ
	CleanDisp,//�����ʾ
};
static bool DrawDeviceInfo(u8 Idx,u8 Addr,u8 *Name,u8 Action)
{
	GUI_REGION DrawRegion;
	u8 StrBuf[DEVICE_NAME_MAX_LEN<<1];

	if(Q_GetPageByTrack(0)!=(&QWebPage)) return TRUE;//���Ƿ��ڱ�ҳ��
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

//����һ����¼����ͬ����ʾ
//Addr����Ϊ0��Name����Ϊ�գ�nameΪ�գ����޸�name
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
		//�Աȵ�ַ�����֣������ַ��ȣ�ֱ���޸����֣����������ͬ�����޸ĵ�ַ��
		if((gpQwpVar->ClientRecord[i].Addr==Addr)||((gpQwpVar->ClientRecord[i].Addr!=0)&&(gpQwpVar->ClientRecord[i].NameChk==NameChk)))
		{//�޸��ֳɵ�
			gpQwpVar->ClientRecord[i].Addr=Addr;
			
			if((Name!=NULL)&&(gpQwpVar->ClientRecord[i].NameChk!=NameChk))//�޸�����
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
	{//���µ�
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

	DrawState();//������״̬
}

//ɾ��һ���豸��¼
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

//����һ���豸
static void HighLightOneDevice(u8 Addr)
{
	u8 i;
	
	for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)
	{
		//�Աȵ�ַ�������ַ��ȣ�����
		if(gpQwpVar->ClientRecord[i].Addr==Addr)
		{
			gpQwpVar->ClientRecord[i].IsHiLight=TRUE;
			DrawDeviceInfo(gpQwpVar->ClientRecord[i].DispIdx,Addr,gpQwpVar->ClientRecord[i].Name,HighLightDisp);
			break;
		}
	}

	//û�ҵ���˵�����µ��豸����
	if(i==DEVICE_INFO_MAX_CLIENT_RECORD)//not found ,add new one.
	{//���µ�
		AddOneDevice(Addr,NULL);
		HighLightOneDevice(Addr);
	}	

	DrawState();//������״̬
}

static void DrawInitBg(void)
{
	GUI_REGION DrawRegion;
	
	//��������
	DrawRegion.x=DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=21;
	DrawRegion.Color=FatColor(NO_TRANS);
	Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
	if(gpQwpVar) DrawTitle1(ASC14B_FONT,gpQwpVar->PageTitle,(240-strlen((void *)gpQwpVar->PageTitle)*GUI_ASC14B_ASCII_WIDTH)>>1,strlen((void *)gpQwpVar->PageTitle),QWEB_PAGE_BG_COLOR);//д����
	
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

//����ĳЩ�¼�ʱ���ᴥ���ĺ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	switch(SysEvent)
	{
		case Sys_PreGotoPage:
			if(pSysParam!=NULL)//���pSysParam��Ϊ�գ����ʾ������ҳ����ã��贫������
			{
				gpQwpSet=Q_Mallco(sizeof(QWEB_PAGE_SET));//ʹ��QS_Mallco��Ϊ���ӱ�ҳ��ĶѼ�����
				MemCpy((void *)gpQwpSet,pSysParam,sizeof(QWEB_PAGE_SET));
			}
			break;
		case Sys_PageInit:		//ϵͳÿ�δ����ҳ�棬�ᴦ������¼�			
			gpQwpVar=Q_PageMallco(sizeof(QWEB_PAGE_VARS));//��Sys_PreGotoPage�д�����ʹ��OS_Mallco����Ϊ�˻�ȡ��ǰҳ���ݣ�����ҳ��Ѽ��
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
		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return
			if(SysEvent==Sys_SubPageReturn) //��ȫ���¼�
			{
				Q_DisableGobalPeripEvent(Perip_QWebJoin,PeripheralsHandler);
				Q_DisableGobalPeripEvent(Perip_QWebQueryName,PeripheralsHandler);
			}
			Q_TimSet(Q_TIM1,10000,500,TRUE);//5s poll

			DrawInitBg();
			DrawState();

			//���q���Ѿ������������������ӦѸ�ٽ���list
			//���q���Ѿ�����������Ǵӻ���ӦѸ�ٽ���list
			//���q��û����������
			if(QWA_GetMyAddr()!=QW_ADDR_DEF)//q���Ѿ�����
			{
				u8 Addr;
				u8 MyAddr=QWA_GetMyAddr();

				if(QWA_GetMyAddr()!=QW_ADDR_HOST)//�Լ��Ǵӻ�
				{
					AddOneDevice(QW_ADDR_HOST,NULL);//�������
				}
				
				//��ѯ������ߴӻ�
				QWA_QueryNextOnline(TRUE);
				while((Addr=QWA_QueryNextOnline(FALSE))!=0)
				{
					if(Addr!=MyAddr) 
					{
						AddOneDevice(Addr,NULL);//��������豸
					}
				}
			}
			if(gpQwpSet!=NULL)//����ǰҳ���ݵ��ڴ����������
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
			if(SysEvent==Sys_PreSubPage)//��ȫ���¼� 
			{
				Q_EnableGobalPeripEvent(Perip_QWebJoin,PeripheralsHandler);
				Q_EnableGobalPeripEvent(Perip_QWebQueryName,PeripheralsHandler);
			}
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
					//PrtScreen();
					QWA_Show();
					break;
				case ExtiKeyUp:
					break;
				case ExtiKeyDown:
					break; 
			}break;

		case Perip_Timer://��ѯ���q��
			Q_TimSet(Q_TIM1,10000,500,TRUE);//5s poll
			if(QWA_GetMyAddr()!=QW_ADDR_DEF)//�Ѿ�����
			{
				u8 i;
				
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//����豸���ڲ���
				{
					if(gpQwpVar->ClientRecord[i].Addr!=0)
						if(QWA_QueryOnline(gpQwpVar->ClientRecord[i].Addr)==0)//�豸������
						{Debug("@@ Delete %d\n\r",gpQwpVar->ClientRecord[i].Addr);
							DeleteOneDevice(gpQwpVar->ClientRecord[i].Addr);
						}
						else if(gpQwpVar->ClientRecord[i].Name[0]==0)//û���ֵģ���ѯ���� //unfinsh:����鲻������һֱѭ����
						{
							QWA_QueryName(gpQwpVar->ClientRecord[i].Addr);
							Q_TimSet(Q_TIM1,10000,200,TRUE);// 2s poll
							break;
						}
				}
			}

			//����������豸���Ƿ���Ȼû�л�Ӧ�������ʱ�������Ч
			if((gpQwpVar->SelectedAddr) && (gpQwpVar->SelectedAddr!=QW_ADDR_HOST)
				&&	(OS_GetCurrentSysMs()-gpQwpVar->SelectedTimeMs > QWEB_PAGE_SELECT_ACT_WAITING_MS) )
			{
				//DeleteOneDevice(gpQwpVar->SelectedAddr);
				gpQwpVar->SelectedAddr=0;
			}
			break;
			
		case Perip_QWebJoin:
			if(QWA_GetMyAddr()==QW_ADDR_HOST)//��Ϊ�����õ��ӻ���query ack
			{
				Debug("QWeb Join [%d]%s\n\r",IntParam,(u8 *)pParam);
				AddOneDevice(IntParam,pParam);
			}
			else if(QWA_GetMyAddr()!=QW_ADDR_DEF)//��Ϊ�ӻ����õ�������Ϣ
			{
				u8 Addr;
				u8 MyAddr=QWA_GetMyAddr();
				
				Debug("QWeb Get Host Info [%d]%s\n\r",IntParam,(u8 *)pParam);
				AddOneDevice(IntParam,pParam);

				//��ѯ���ߴӻ�
				QWA_QueryNextOnline(TRUE);
				while((Addr=QWA_QueryNextOnline(FALSE))!=0)
				{
					if(Addr!=MyAddr) AddOneDevice(Addr,NULL);//��������豸
				}

				if(gpQwpVar->SelectedAddr==QW_ADDR_HOST)//��ѡ��������Ϊ�Ի���ַ�����ڵȴ�����info
				{
					gpQwpVar->SelectedAddr=0;
					Q_GotoPage(gpQwpVar->GotoAct_List,gpQwpVar->GotoName_List,IntParam,pParam);
				}
			}
			break;
		case Perip_QWebQueryName:
			Debug("QWeb Query [%d]%s\n\r",IntParam,(u8 *)pParam);
			AddOneDevice(IntParam,pParam);

			if(IntParam==gpQwpVar->SelectedAddr)//��ѡ����ַ�����ڵȴ�query ack
			{
				gpQwpVar->SelectedAddr=0;
				Q_GotoPage(gpQwpVar->GotoAct_List,gpQwpVar->GotoName_List,IntParam,pParam);
			}
			break;
		case Perip_QWebRecv://�յ���Ϣ
			HighLightOneDevice(IntParam>>24);
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
						gpQwpVar->NowPressKey=Key;//��¼���µ���
						DrawDeviceInfo(Key,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,PressDisp);
						break;
					}
				}
			}
			else if(InEvent==Tch_Release)
			{
				Key=((pTouchInfo->y-DEVICE_INFO_START_Y)/(DEVICE_INFO_H+DEVICE_INFO_LIST_SPACE))+1;
				if(Key==gpQwpVar->NowPressKey)//��ͬһ�����ͷ��ˣ��˰��¶�����Ч
				{
					for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++) //��ԭ������봦��ҳ��
					{
						if(gpQwpVar->ClientRecord[i].DispIdx==Key)
						{
							gpQwpVar->ClientRecord[i].IsHiLight=FALSE;
							DrawDeviceInfo(Key,gpQwpVar->ClientRecord[i].Addr,gpQwpVar->ClientRecord[i].Name,NormalDisp);
							if(gpQwpVar->GotoName_List&&gpQwpVar->GotoName_List[0])//���������ҳ��
							{//������ҳ��ǰ�Ȳ�ѯ����
								gpQwpVar->SelectedAddr=gpQwpVar->ClientRecord[i].Addr;
								QWA_QueryName(gpQwpVar->SelectedAddr);
								gpQwpVar->SelectedTimeMs=OS_GetCurrentSysMs();
								//��Perip_QWebQueryName�¼�����ִ��goto page����
							}
							break;
						}
					}
				}
				else//���������λ���ͷ��ˣ��˰��¶�����Ч
				{
					for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//��ԭ������ʾ
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
			else if(InEvent==Tch_ReleaseVain)//�ڳ���list�������ͷ��ˣ���Ч
			{
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//��ԭ������ʾ
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
			//��Ҫ��Ӧ���¼�δ����
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
				for(i=0;i<DEVICE_INFO_MAX_CLIENT_RECORD;i++)//����������ڲ���
				{
					if(gpQwpVar->ClientRecord[i].Addr!=0)
					{
						DeleteOneDevice(gpQwpVar->ClientRecord[i].Addr);
					}
				}
					
				//״̬����
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


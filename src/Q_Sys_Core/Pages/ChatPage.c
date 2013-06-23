/**************** Q-SYS *******************
 * PageName : ChatPage
 * Author : YourName
 * Version : 
 * Base Q-Sys Version :
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-SYS *******************/
 
#include "User.h"
#include "Theme.h"
#include "QWebPage.h"
#include "ChatPage.h"

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
	ChatKV,
	SendKV,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}ChatPage_OID;

#define CHAT_FRAME_X 6
#define CHAT_FRAME_Y 43+6
#define CHAT_FRAME_W 228
#define CHAT_FRAME_H 219-12
#define SEND_FRAME_X 6
#define SEND_FRAME_Y 267+6
#define SEND_FRAME_W 228
#define SEND_FRAME_H 48-12
#define CHAT_TXT_COLOR FatColor(0x000000)
#define HELP_TXT_COLOR FatColor(0xaaaaaa)
#define CHAT_BG_COLOR FatColor(0xe0e0e0)
#define CHAT_BUF_MAX_BYTES 100 //������ֽ���
#define CHAT_LIST_ROW_BYTE 38//�����¼ÿ���ֽ���
#define CHAT_LIST_COL_NUM	16//�����¼����
#define CHAT_LIST_LINE_H	13//�����¼�и�
#define CHAT_MY_NAME_COLOR FatColor(0x1ba400)//�Լ���������ɫ
#define CHAT_MY_TXT_COLOR FatColor(0x000000)//�Լ���������ɫ
#define CHAT_SHE_NAME_COLOR FatColor(0x0c51b8)//���˵�������ɫ
#define CHAT_SHE_TXT_COLOR FatColor(0x000000)//���˵�������ɫ

#define MAX_RESEND_NUM 10//����ط�10��

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
//֧�ֵ�������������ΪMAX_TOUCH_REGION_NUM
//ϵͳ��ʾ�ʹ�������������ϵ������Ļ���Ͻ�Ϊԭ��(x 0,y 0)�����½�Ϊ(x 320,y 240)
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},	
	{"", ChatKV,RelMsk,CHAT_FRAME_X,CHAT_FRAME_Y,CHAT_FRAME_W,CHAT_FRAME_H,0,0,"",FatColor(NO_TRANS)},
	{"", SendKV,RelMsk,SEND_FRAME_X,SEND_FRAME_Y,SEND_FRAME_W,SEND_FRAME_H,0,0,"",FatColor(NO_TRANS)},
	
	//Һ�����������ʾ������ĸ���
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE ChatPage={
	"ChatPage",
	"Author",
	"Page Description",
	NORMAL_PAGE,
	0,//

	{
		sizeof(ImgButtonCon)/sizeof(IMG_BUTTON_OBJ), //size of touch region array
		0,//sizeof(CharButtonCon)/sizeof(CHAR_BUTTON_OBJ), //size of touch region array,
	},
	ImgButtonCon, //touch region array
	NULL,
	
	SystemEventHandler,
	PeripheralsHandler,
	Bit(Perip_KeyPress)|Bit(Perip_KeyRelease)|
	Bit(Perip_QWebRecv)|Bit(Perip_QWebSendFailed)|Bit(Perip_QWebSendOk)|
	Bit(Perip_UartInput),
	ButtonHandler,
};

//-----------------------��ҳ�Զ����������-----------------------
typedef struct{
	u8 DstAddr;
	u8 DstName[20];
	u8 SendBuf[CHAT_BUF_MAX_BYTES+2];
	u8 ChatList[CHAT_LIST_COL_NUM][CHAT_LIST_ROW_BYTE+2];
	u16 ChatListColor[CHAT_LIST_COL_NUM];
	u8 ChatListDispIdx;//start from 0 to CHAT_LIST_COL_NUM-1
	u8 ReSendCnt;//��Ҫ���͵������Ѿ����͵Ĵ���
	u8 LastRecordRole;//���һ����Ϣ�Ľ�ɫ
}CHAT_PAGE_VARS;
static CHAT_PAGE_VARS *gpCpVars;
//-----------------------��ҳ�Զ��庯��-----------------------
static u8 CalculateRowByteNums(u8 *pStr,u8 MaxByteNums)	//����һ�������������
{
	u8 i=0;

	while((i!=MaxByteNums)&&(i!=(MaxByteNums-1)))
	{
		if(pStr[i]>0x7f)
			i+=2;
		else
			i++;
	}
	return i;
}

static u16 DispOneRecord(u16 StartY,u8 DispIdx)//��ʾһ�������¼
{
	GUI_REGION DrawRegion;

	if(gpCpVars->ChatList[DispIdx][0])
	{
		DrawRegion.x=CHAT_FRAME_X;
		DrawRegion.y=StartY;
		DrawRegion.w=CHAT_FRAME_W;
		DrawRegion.h=CHAT_LIST_LINE_H;
		DrawRegion.Space=0x00;
		DrawRegion.Color=CHAT_BG_COLOR;
		Gui_FillBlock(&DrawRegion);
		DrawRegion.Color=gpCpVars->ChatListColor[DispIdx];
		Gui_DrawFont(GBK12_FONT,gpCpVars->ChatList[DispIdx],&DrawRegion);

		return StartY+CHAT_LIST_LINE_H;
	}
	return StartY;
}

static void DispChatList(void)//��ʾ�����¼
{
	u8 i;
	u16 StartY=CHAT_FRAME_Y;
	
	for(i=gpCpVars->ChatListDispIdx;i<CHAT_LIST_COL_NUM;i++)
	{
		StartY=DispOneRecord(StartY,i);
	}

	for(i=0;i<gpCpVars->ChatListDispIdx;i++)
	{
		StartY=DispOneRecord(StartY,i);
	}
}

//����һ����¼�������б�
static void InsertOneRecord(u8 *pStr,u8 SrcAddr)
{
	u16 Len,CopyLen;
	RTC_TIME NowTime;

	if(gpCpVars->LastRecordRole!=SrcAddr)//�����һ������һ����ͬһ���˷��ģ�������ʾ������Ϣ
	{
		//��������
		RTC_GetTime(&NowTime);
		if(QWA_GetMyAddr()==SrcAddr)//is my
		{
			sprintf((void *)gpCpVars->ChatList[gpCpVars->ChatListDispIdx],"%s (%02d:%02d:%02d)",QWA_MyQWebName(NULL),NowTime.hour,NowTime.min,NowTime.sec);		
			gpCpVars->ChatListColor[gpCpVars->ChatListDispIdx]=CHAT_MY_NAME_COLOR;
		}
		else
		{
			sprintf((void *)gpCpVars->ChatList[gpCpVars->ChatListDispIdx],"%s (%02d:%02d:%02d)",QWP_GetNameByAddr(SrcAddr),NowTime.hour,NowTime.min,NowTime.sec);		
			gpCpVars->ChatListColor[gpCpVars->ChatListDispIdx]=CHAT_SHE_NAME_COLOR;
		}
		gpCpVars->ChatListDispIdx++;
		if(gpCpVars->ChatListDispIdx==CHAT_LIST_COL_NUM) gpCpVars->ChatListDispIdx=0;

		gpCpVars->LastRecordRole=SrcAddr;
	}

	//��������
	Len=strlen((void *)pStr);
	while(1)
	{
		CopyLen=Len;
		
		if(CopyLen>CHAT_LIST_ROW_BYTE)
		{
			CopyLen=CalculateRowByteNums(pStr,CHAT_LIST_ROW_BYTE);
		}
		
		MemCpy(gpCpVars->ChatList[gpCpVars->ChatListDispIdx],pStr,CopyLen);
		gpCpVars->ChatList[gpCpVars->ChatListDispIdx][CopyLen]=0;
		if(QWA_GetMyAddr()==SrcAddr) gpCpVars->ChatListColor[gpCpVars->ChatListDispIdx]=CHAT_MY_TXT_COLOR;
		else gpCpVars->ChatListColor[gpCpVars->ChatListDispIdx]=CHAT_SHE_TXT_COLOR;

		//�����Լ�
		gpCpVars->ChatListDispIdx++;
		if(gpCpVars->ChatListDispIdx==CHAT_LIST_COL_NUM) gpCpVars->ChatListDispIdx=0;

		Len-=CopyLen;
		if(Len<=0) break;
		pStr=&pStr[CopyLen];
	}
}

static SYS_MSG RecvQwebData(PERIP_EVT PeripEvent,int IntParam, void *pParam)
{
	Debug("Recv from %d:%dbytes %s\n\r",IntParam>>24,IntParam&0xffffff,(u8 *)pParam);
	InsertOneRecord((void *)pParam,IntParam>>24);
	return 0;
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
			gpCpVars=Q_PageMallco(sizeof(CHAT_PAGE_VARS));
			MemSet(gpCpVars,0,sizeof(CHAT_PAGE_VARS));
			//QWA_MyQWebName("�Ҳ���");
		case Sys_SubPageReturn:	//�������ҳ�淵��,�Ͳ��ᴥ��Sys_Page_Init�¼�,����Sys_SubPage_Return
			//��������
			DrawRegion.x=DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=21;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_StatusBar1,1,21,&DrawRegion);	
			DrawTitle1(ASC14B_FONT,"ChatPage",(240-strlen("ChatPage")*GUI_ASC14B_ASCII_WIDTH)>>1,strlen("ChatPage"),FatColor(0xe0e0e0));//д����
			
			//������
			DrawRegion.x=0;
			DrawRegion.y=21;
			DrawRegion.w=240;
			DrawRegion.h=320-21;
			DrawRegion.Color=FatColor(0x8b8a8a);
			Gui_FillBlock(&DrawRegion);
			
			{//draw info
				u8 Buf[32];
				if(SysEvent==Sys_PageInit)//���Ƶ�ַ������
				{
					gpCpVars->DstAddr=IntParam;
					MemCpy(gpCpVars->DstName,pSysParam,16);
				}				
				DrawRegion.x=8;
				DrawRegion.y=26;
				DrawRegion.w=200;
				DrawRegion.h=16;
				DrawRegion.Space=0x00;
				DrawRegion.Color=FatColor(0xffffff);
				sprintf((void *)Buf,"Chat With [%d]%s\n\r",gpCpVars->DstAddr,(u8 *)gpCpVars->DstName);
				Gui_DrawFont(GBK12_FONT,Buf,&DrawRegion);
			}
			
			//����
			DrawFrame1(CHAT_FRAME_Y-6,CHAT_FRAME_H+12);
			DrawFrame1(SEND_FRAME_Y-6,SEND_FRAME_H+12);
			if(SysEvent==Sys_PageInit)
			{//help info
				DrawRegion.x=CHAT_FRAME_X;
				DrawRegion.y=CHAT_FRAME_Y;
				DrawRegion.w=CHAT_FRAME_W;
				DrawRegion.h=CHAT_FRAME_H;
				DrawRegion.Space=0x02;
				DrawRegion.Color=HELP_TXT_COLOR;
				Gui_DrawFont(GBK16_FONT,"����²������ͨ�����뷨�����ַ���������ϲ����巢�͡���С����ͼ���˳���",&DrawRegion);
			}		  

			if(strcmp((void *)Q_GetPageByTrack(1)->Name,"KeyBoardPage")==0)//�����뷨ҳ�淵��
			{
				Q_DisableGobalPeripEvent(Perip_QWebRecv,RecvQwebData);//�ص�ȫ���¼�
			
				if(IntParam)
				{//send frame
					DrawRegion.x=SEND_FRAME_X;
					DrawRegion.y=SEND_FRAME_Y;
					DrawRegion.w=SEND_FRAME_W;
					DrawRegion.h=SEND_FRAME_H;
					DrawRegion.Space=0x00;
					DrawRegion.Color=CHAT_TXT_COLOR;
					Gui_DrawFont(GBK12_FONT,(void *)pSysParam,&DrawRegion);
				}
				DispChatList();
			}
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:

			break;
		case Sys_PageClean:
			Q_PageFree(gpCpVars);
			break;
		case Sys_PreSubPage:
			{
				PAGE_ATTRIBUTE *pNewPage=pSysParam;
				if(strcmp((void *)pNewPage->Name,"KeyBoardPage")==0)	//��ȫ���¼�
					Q_EnableGobalPeripEvent(Perip_QWebRecv,RecvQwebData);
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
					Q_PresentTch(LeftArrowKV,Tch_Press);
					break;
				case ExtiKeyDown:
					Q_PresentTch(RightArrowKV,Tch_Press);
					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
					//PrtScreen();
					if(gpCpVars->SendBuf[0])
					{
						Debug("Send to Addr%d %s\n\r",strlen((void *)gpCpVars->SendBuf),gpCpVars->SendBuf);
						QWA_SendData(gpCpVars->DstAddr,strlen((void *)gpCpVars->SendBuf)+1,gpCpVars->SendBuf);
						gpCpVars->ReSendCnt=0;
					}
					break;
				case ExtiKeyUp:
					Q_PresentTch(LeftArrowKV,Tch_Release);
					break;
				case ExtiKeyDown:
					Q_PresentTch(RightArrowKV,Tch_Release);
					break; 
			}break;

		case Perip_QWebRecv:
			RecvQwebData(Perip_QWebRecv,IntParam,pParam);
			DispChatList();
			break;
		case Perip_QWebSendOk:
			InsertOneRecord(gpCpVars->SendBuf,QWA_GetMyAddr());
			DispChatList();
			{
				GUI_REGION DrawRegion;
				//�����ʾ
				DrawRegion.x=SEND_FRAME_X;
				DrawRegion.y=SEND_FRAME_Y;
				DrawRegion.w=SEND_FRAME_W;
				DrawRegion.h=SEND_FRAME_H;
				DrawRegion.Color=CHAT_BG_COLOR;
				Gui_FillBlock(&DrawRegion);
				gpCpVars->SendBuf[0]=0;
			}
			break;
		case Perip_QWebSendFailed:
			if(gpCpVars->SendBuf[0])
			{
				if(gpCpVars->ReSendCnt<MAX_RESEND_NUM)//�ط�
				{
					Debug("Send to Addr%d %s\n\r",strlen((void *)gpCpVars->SendBuf),(u8 *)gpCpVars->SendBuf);
					QWA_SendData(gpCpVars->DstAddr,strlen((void *)gpCpVars->SendBuf)+1,gpCpVars->SendBuf);
					gpCpVars->ReSendCnt++;
				}
				else
				{
					Debug("Send to Addr%d Failed:%s\n\r",strlen((void *)gpCpVars->SendBuf),(u8 *)gpCpVars->SendBuf);
				}
			}
			break;
			
		case Perip_UartInput:
			if((IntParam>>16)==1)//����1
			{
				IntParam&=0xffff;
				if(IntParam!=0)
				{//send frame
					GUI_REGION DrawRegion;
					
					DrawRegion.x=SEND_FRAME_X;
					DrawRegion.y=SEND_FRAME_Y;
					DrawRegion.w=SEND_FRAME_W;
					DrawRegion.h=SEND_FRAME_H;
					DrawRegion.Space=0x00;
					DrawRegion.Color=CHAT_BG_COLOR;
					Gui_FillBlock(&DrawRegion);
					DrawRegion.Color=CHAT_TXT_COLOR;
					Gui_DrawFont(GBK12_FONT,(void *)pParam,&DrawRegion);

					if(IntParam>CHAT_BUF_MAX_BYTES) IntParam=CHAT_BUF_MAX_BYTES;
					MemCpy(gpCpVars->SendBuf,pParam,IntParam);
					gpCpVars->SendBuf[IntParam]=0;
				}			
				else if((IntParam==0)&&((((u16 *)pParam)[0]==0x445b)||(((u16 *)pParam)[0]==0x435b))&&strlen((void *)gpCpVars->SendBuf))//ǰ�������
				{
					Debug("Send to Addr%d %s\n\r",strlen((void *)gpCpVars->SendBuf),gpCpVars->SendBuf);
					QWA_SendData(gpCpVars->DstAddr,strlen((void *)gpCpVars->SendBuf)+1,gpCpVars->SendBuf);
				}
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
		case BackKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//����ǰһ��ҳ��
			break;
		case DoneKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//����ǰһ��ҳ��
			break;	
		case LeftArrowKV:
			break;
		case DotKV:
			break;
		case RightArrowKV:
			break;

		case ChatKV:
			if(gpCpVars->SendBuf[0])
			{
				Debug("Send to Addr%d %s\n\r",strlen((void *)gpCpVars->SendBuf),gpCpVars->SendBuf);
				QWA_SendData(gpCpVars->DstAddr,strlen((void *)gpCpVars->SendBuf)+1,gpCpVars->SendBuf);
				gpCpVars->ReSendCnt=0;
			}
			break;
		case SendKV:
			Q_GotoPage(GotoSubPage,"KeyBoardPage",CHAT_BUF_MAX_BYTES,gpCpVars->SendBuf);
			break;		
		case HomeKV:
			Q_GotoPage(SubPageReturn,"",0,NULL);//����ǰһ��ҳ��
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




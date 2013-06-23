/**************** Q-OS *******************
 * PageName : 
 * Author : YuanYin
 * Version : 
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-OS *******************/
#include "User.h"
#include "EBookPage.h"
#include "FileListPage.h"

static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------��ҳϵͳ����������-----------------------
typedef enum 
{
	ExtiKeyDown=EXTI_KEY_VALUE_START,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ
	ExtiKeyUp,
	ExtiKeyEnter,

	OpenKV=USER_KEY_VALUE_START,
	BackKV,
	PrevOneKV,
	NextOneKV,
	DotKV,

	HomeKV,
	MessageKV,
	MusicKV,
	PepoleKV,
}EBook_OID;

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
static const IMG_BUTTON_OBJ ImgButtonCon[]={
	//KeyName,ObjID,OptionMask,Tch_x,Tch_y,Tch_w,Tch_h,Img_x,Img_y,BmpPathPrefix,NormalSuf,PressSuf,ReleaseSuf,TransColor},
	{"Back",	BackKV,RelMsk|PathMsk,3,287,54,31,0,0,"Common/Btn/Back",FatColor(NO_TRANS)},
	{"<<",		PrevOneKV,RelMsk|PathMsk,65,287,39,31,0,0,"Common/Btn/LeftArr",FatColor(0xff0000)},
	{"Dot",		DotKV,RelMsk|PathMsk,109,292,22,22,0,0,"Common/Btn/Dot",FatColor(0xff0000)},
	{">>",		NextOneKV,RelMsk|PathMsk,136,287,39,31,0,0,"Common/Btn/RightArr",FatColor(0xff0000)},
	{"Open",	OpenKV,RelMsk|PathMsk,183,287,54,31,0,0,"Common/Btn/Open",FatColor(NO_TRANS)},
	
	//Һ�����������ʾ������ĸ���
	{"",HomeKV,RelMsk,0,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MessageKV,RelMsk,60,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",MusicKV,RelMsk,120,320,60,30,0,0,"",FatColor(NO_TRANS)},
	{"",PepoleKV,RelMsk,180,320,60,30,0,0,"",FatColor(NO_TRANS)},
};

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE EBookPage={
	"EBookPage",
	"YuanYin",
	"Ebook page",
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
	0,
	ButtonHandler,
	
};

//-----------------------��ҳ�Զ����������-----------------------
#define EBOOK_BG_COLOR 0x8b8a8a
#define EBOOK_TXT_COLOR 0xffffff
#define EBOOK_MAX_PAGE 1024//�ļ����ҳ��
#define READ_BUF_SIZE	512 //ÿҳ�ַ����泤��
#define TXT_REGION_WIDTH 232
#define TXT_REGION_HEIGHT 281

const GUI_REGION gEBookTxtRegion={4,4,TXT_REGION_WIDTH,TXT_REGION_HEIGHT,0x12,FatColor(EBOOK_TXT_COLOR)};
typedef struct {
	u16 CurPage;//��ǰҳ��
	u16 PageTotal;//��ǰ�ļ���ҳ��
	u8 EBookPath[MAX_PATH_LEN];//��ǰ�ı�·��
	u16 EBookLocal[EBOOK_MAX_PAGE];//��ÿҳ�ַ���Ŀ

	FS_FILE *EBookObj;
	u8 ReadBuf[READ_BUF_SIZE+1];
}EBOOK_PAGE_VARS;
static EBOOK_PAGE_VARS *gpEbpVars;//ֻ��Ҫ����һ��ָ�룬����ȫ�ֱ�����ʹ��

//-----------------------��ҳ�Զ��庯��-----------------------
static u32 GetSum(u16 *Buf,u16 Len)
{
	u32 Sum=0;
	
	while(Len--)
	{
		Sum+=*Buf++;
	}

	return Sum;
}

static void GetEBookLocal(u16 BufLen,EBOOK_PAGE_VARS *pVars)
{
//	FRESULT Res;
	unsigned int ReadByte;
	
  	if ((pVars->EBookObj=FS_FOpen((void *)_TEXT(pVars->EBookPath), FA_OPEN_EXISTING | FA_READ) )== 0 ) 
	{
		Debug("Open txt flie error !:%s\n\r",pVars->EBookPath);
		return;
	}
	
	for(pVars->PageTotal=0;pVars->PageTotal<BufLen;pVars->PageTotal++) pVars->EBookLocal[pVars->PageTotal]=0;

	for(pVars->PageTotal=0;pVars->PageTotal<BufLen;pVars->PageTotal++)
	{
		if(FS_FSeek(pVars->EBookObj,GetSum(pVars->EBookLocal,pVars->PageTotal),FS_SEEK_SET)!=0)
		{
			Debug("Txt fseek err!\n");
			break;
		}	
		
		ReadByte=FS_FRead(pVars->ReadBuf, READ_BUF_SIZE,1,pVars->EBookObj);
		if( ReadByte)
	    {
	    	//dragon add
	    	pVars->ReadBuf[ReadByte]=0;
			
			pVars->EBookLocal[pVars->PageTotal]=Gui_DrawFont(GBK16_NUM,pVars->ReadBuf,(GUI_REGION *)&gEBookTxtRegion);
			Debug("%d:  %d\n\r",pVars->PageTotal,pVars->EBookLocal[pVars->PageTotal]);
		}
		else
		{
			Debug("Read txt file %s end!\n\r",pVars->EBookPath);
			break;
		}	   			
	}
	
	if(FS_FClose(pVars->EBookObj)!=0)
	{
		Debug("Close ebook file error!\n\r");
	}
}

static void DispTxt(EBOOK_PAGE_VARS *pVars)
{
	GUI_REGION DispRegion={4,4,TXT_REGION_WIDTH,TXT_REGION_HEIGHT-4,0x14,FatColor(EBOOK_BG_COLOR)};
//	FRESULT Res;
	unsigned int ReadByte;

	if((pVars->EBookPath==NULL)||(!pVars->EBookPath[0])) return;//û·��
	if(pVars->CurPage>pVars->PageTotal) return;//������ҳ��
			
  	if ((pVars->EBookObj=FS_FOpen((void *)_TEXT(pVars->EBookPath), FA_OPEN_EXISTING | FA_READ) )== 0 ) 
	{
		Debug("Open txt flie error !:%s\n\r",pVars->EBookPath);
		return;
	}

	if(FS_FSeek(pVars->EBookObj,GetSum(pVars->EBookLocal,pVars->CurPage),FS_SEEK_SET)!=0)
	{
		Debug("Txt fseek err!\n");
		return;
	}	
	
	ReadByte=FS_FRead(pVars->ReadBuf, READ_BUF_SIZE,1,pVars->EBookObj);
	if(ReadByte)
    {
    	pVars->ReadBuf[ReadByte]=0;
		Gui_FillBlock((void *)&DispRegion);
		Gui_DrawFont(GBK16_FONT,pVars->ReadBuf,(void *)&gEBookTxtRegion);
	}
	else
	{
		Debug("Read txt file end!\n\r"/*,pVars->ReadBuf*/);
	}	   			

	if(FS_FClose(pVars->EBookObj)!=0)
	{
		Debug("Close music file error!\n\r");
	}
	
	return;		
}

//-----------------------��ҳϵͳ����----------------------
//��ÿ�α�ҳ����ϵͳ��ǰҳ��ʱ����õĳ���
static SYS_MSG SystemEventHandler(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	GUI_REGION DrawRegion;

	switch(SysEvent)
	{
		case Sys_PageInit:		
			Debug("sizeof(EBOOK_PAGE_VARS)=%d\n\r",sizeof(EBOOK_PAGE_VARS));
			gpEbpVars=(EBOOK_PAGE_VARS *)Q_PageMallco(sizeof(EBOOK_PAGE_VARS));
			if(gpEbpVars==0)
			{
				Q_ErrorStopScreen("gpEbpVars malloc fail !\n\r");
			}

		case Sys_SubPageReturn:
			if(Q_GetPageEntries()==1)  //ҳ���״δ���ʱ��Ҫ��������
			{
			}
			DrawRegion.x=0;
			DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=320;
			DrawRegion.Color=FatColor(EBOOK_BG_COLOR);
			Gui_FillBlock(&DrawRegion);
			gpEbpVars->PageTotal=gpEbpVars->CurPage=0;

			//������
			DrawRegion.x=0;
			DrawRegion.y=320-39;
			DrawRegion.w=240;
			DrawRegion.h=39;
			DrawRegion.Color=FatColor(NO_TRANS);
			Gui_FillImgArray((u8 *)gImage_BottomBar1,1,39,&DrawRegion);
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:
			if(IntParam==TRUE)//����ҳ�淵��
			{
				if(CheckMultiSuffix(gpEbpVars->EBookPath,".txt"))
				{//������ļ���������ʾ
					Debug("Select %s\n\r",gpEbpVars->EBookPath);
					Gui_DrawFont(GBK16_FONT,"����Ԥ�������Ժ�...",(void *)&gEBookTxtRegion);
					GetEBookLocal(EBOOK_MAX_PAGE,gpEbpVars);
					Debug("Total Page Num:%d\n\r",gpEbpVars->PageTotal);
					DispTxt(gpEbpVars);
				}
			}
			break;
		case Sys_PageClean:
			Q_PageFree(gpEbpVars);
			break;
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
					Q_PresentTch(PrevOneKV,Tch_Press);
					break;
				case ExtiKeyDown:
					Q_PresentTch(NextOneKV,Tch_Press);
					break; 
			}break;
		case Perip_KeyRelease:
			switch(IntParam){
				case ExtiKeyEnter:
					PrtScreen();
					break;
				case ExtiKeyUp:
					Q_PresentTch(PrevOneKV,Tch_Release);
					if(gpEbpVars->CurPage>0)
					{
						gpEbpVars->CurPage--;
						DispTxt(gpEbpVars);
					}
					break;
				case ExtiKeyDown:
					Q_PresentTch(NextOneKV,Tch_Release);
					if(gpEbpVars->CurPage+1<gpEbpVars->PageTotal)
					{
						gpEbpVars->CurPage++;
						DispTxt(gpEbpVars);
					}
					break; 
			}break;
	}
	return 0;
}
static CO_MSG ButtonHandler(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{	
	FILELIST_SET FileListParam;
	
	switch(Key)
	{	
		case OpenKV:
			FileListParam.CallBackRid=Q_FindRidByPageName(NULL);
			sprintf((void *)gpEbpVars->EBookPath,"/EBook");
			FileListParam.pPathBuf=gpEbpVars->EBookPath;
			FileListParam.pSuffixStr=".txt";
			Q_GotoPage(GotoSubPage,"FileListPage",FL_SelectOne|FL_NoParent|FL_NoListBuf,&FileListParam);
			break;
		case BackKV:
			if(InEvent!=Tch_Release) return 0;
			Q_GotoPage(GotoNewPage,"MainPage",0,NULL);
			break;
		case DotKV:
			break;
		
		case PrevOneKV:
			if(gpEbpVars->CurPage>0)
			{
				gpEbpVars->CurPage--;
				DispTxt(gpEbpVars);
			}
			break;	
		case NextOneKV:
			if(gpEbpVars->CurPage+1<gpEbpVars->PageTotal)
			{
				gpEbpVars->CurPage++;
				DispTxt(gpEbpVars);
			}
			break;
		default:
			Debug("You should not here! Key:%d\n\r",Key);
			//while(1);
	}
	
	return 0;
}




/**************** Q-OS *******************
 * PageName : 
 * Author : YuanYin
 * Version : 
 * Description :
 * WebSite : Www.Q-ShareWe.Com
 ***************** Q-OS *******************/
#include "User.h"
#include "PicturePage.h"
#include "FileListPage.h"

static SYS_MSG SystemEventHandlerPicturePage(SYS_EVT SysEvent ,int IntParam, void *pSysParam);
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam);
static CO_MSG TouchEventHandlerPicturePage(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo);

//-----------------------��ҳϵͳ����������-----------------------
enum PictureOID
{
	PicturePage_ExtiKey0=0,//ϵͳĬ�Ͻ��ⲿ�жϰ������͵���һ����ֵ

	PicturePage_Open=USER_KEY_VALUE_START,
	PicturePage_Return,
	PicturePage_Pre,
	PicturePage_Next
};

//����ҳ���Ӧ�õĴ������򼯣��൱�ڶ��尴��
static const IMG_BUTTON_OBJ PicturePageTouchRegionSet[]={
	//{key,gLandScapeMode,x,y,width,hight,image x,image y,normal bmp path,release bmp path,press bmp path,transparent color,key name}
	{"Open",PicturePage_Open,RelMsk,0,0,240,60,0,0,"",0},
	{"Return",PicturePage_Return,RelMsk,0,260,240,60,0,0,"",0},
	{"<<",PicturePage_Pre,RelMsk,0,60,120,200,0,0,"",0},
	{">>",PicturePage_Next,RelMsk,120,60,120,200,0,0,"",0}
};

//����ҳ�����Ӧ�õ����Լ�
const PAGE_ATTRIBUTE PicturePage={
	"PicturePage",
	"YuanYin",
	"Picture page",
	NORMAL_PAGE,
	0,

	{
		sizeof(PicturePageTouchRegionSet)/sizeof(IMG_BUTTON_OBJ),
		0,
	},
	PicturePageTouchRegionSet,
	NULL,

	SystemEventHandlerPicturePage,
	PeripheralsHandler,
	Bit(Perip_UartInput),
	TouchEventHandlerPicturePage,
	
};

//-----------------------��ҳ�Զ����������-----------------------
#define MAX_PICTURE_NUM 256
#define PICTURE_DIR "/Picture"
#define PICTURE_SUFFIX_STR ".bmp"


static const char PictureHelp[]="ͼƬ����������:\n\r1.��������ֻ֧��320x240���ڵ�24λBmp��ʽͼƬ���뽫ͼƬ������TF����Ŀ¼Picture�ļ����¡�\n\r2.����Ϸ�60�����ص�����Ϊ��ͼƬ�ļ��б�����·�60�����ص�����Ϊ���ص������棬����������Ϊ��ʾ��һͼ������Ҳ�����Ϊ��ʾ��һͼ��\n\r3.ֱ���ڳ����ն������ַ������û��س����Զ��򿪶�ӦͼƬ��������ǰ����ɿ���ͼƬ���š�";
static u8 *gpPicPath;//��ǰͼƬ·��


//-----------------------��ҳ�Զ��庯��-----------------------
static void DispPic(void)
{
	GUI_REGION DrawRegion;
	u16 i;

	Debug("Display Picture:%s\n\r",gpPicPath);
	
	for(i=strlen((void *)gpPicPath);i;i--) if(gpPicPath[i]=='/')break;
	i++;
				
	DrawRegion.x=0;
	DrawRegion.y=0;
	DrawRegion.w=240;
	DrawRegion.h=320;
	DrawRegion.Color=FatColor(NO_TRANS);
	if(Gui_Draw24Bmp(gpPicPath,&DrawRegion)!=Gui_True) return;

	//draw title
	DrawRegion.w=234;
	DrawRegion.h=314;
	DrawRegion.Space=0x36;

	DrawRegion.x=2;
	DrawRegion.y=1;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	DrawRegion.x=2;
	DrawRegion.y=3;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	DrawRegion.x=1;
	DrawRegion.y=2;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	DrawRegion.x=3;
	DrawRegion.y=2;
	DrawRegion.Color=FatColor(0xffffff);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);
	
	DrawRegion.x=2;
	DrawRegion.y=2;
	DrawRegion.Color=FatColor(0);
	Gui_DrawFont(GBK12_FONT,&gpPicPath[i],&DrawRegion);

	return;		
}

//-----------------------��ҳϵͳ����----------------------
//��ÿ�α�ҳ����ϵͳ��ǰҳ��ʱ����õĳ���
static SYS_MSG SystemEventHandlerPicturePage(SYS_EVT SysEvent ,int IntParam, void *pSysParam)
{
	GUI_REGION DrawRegion;
	
	switch(SysEvent)
	{
		case Sys_PageInit:	
			gpPicPath=(u8 *)Q_PageMallco(MAX_PATH_LEN);

			if(gpPicPath==0)
			{
				Q_ErrorStopScreen("gpPicPath malloc fail !\n\r");
			}

			sprintf((void *)gpPicPath,PICTURE_DIR);
		case Sys_SubPageReturn:
			DrawRegion.x=0;
			DrawRegion.y=0;
			DrawRegion.w=240;
			DrawRegion.h=320;
			DrawRegion.Color=FatColor(0x8b8a8a);
			Gui_FillBlock(&DrawRegion);
			
			DrawRegion.x=4;
			DrawRegion.y=4;
			DrawRegion.w=232;
			DrawRegion.h=320;
			DrawRegion.Space=0x22;
			DrawRegion.Color=FatColor(0xffffff);
			Gui_DrawFont(GBK16_FONT,(void *)PictureHelp,&DrawRegion);
			break;
		case Sys_TouchSetOk:
		case Sys_TouchSetOk_SR:
			if(IntParam==TRUE)//����ҳ�淵��
			{
				if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
				{//������ļ���������ʾ
					DispPic();
				}
			}
			break;
		case Sys_PageClean:
			Q_PageFree(gpPicPath);
			break;
		/*

		*/
	}
	
	return 0;
}
static SYS_MSG PeripheralsHandler(PERIP_EVT PeripEvent, int IntParam, void *pParam)
{
	FILELIST_SET FLPS;
	switch(PeripEvent)
	{
		case Perip_KeyPress:
			switch(IntParam){
				case PicturePage_ExtiKey0:
					PrtScreen();
					break;
			}break;
		case Perip_UartInput:
		if((IntParam>>16)==1)//����1
		{
			IntParam&=0xffff;
			if((IntParam==0)&&(((u16 *)pParam)[0]==0x445b)) //previous one
			{
				//Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindPrevious|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						//Debug("Prev:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindLast|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							//Debug("Last:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			else if((IntParam==0)&&(((u16 *)pParam)[0]==0x435b)) //next one
			{
				//Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindNext|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						//Debug("Next:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindFrist|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							//Debug("First:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			else
			{
				sprintf((void *)gpPicPath,"Picture/%s.bmp",(u8 *)pParam);
				DispPic();
			}
		}

		break;
	}

	return 0;
}
static CO_MSG TouchEventHandlerPicturePage(u8 Key,TCH_EVT InEvent , TOUCH_INFO *pTouchInfo)
{	
	FILELIST_SET FLPS;
	
	if(InEvent!=Tch_Release) return 0;
	
	switch(Key)
	{
		
		case PicturePage_Open:
			if(InEvent!=Tch_Release) return 0;
			{
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				sprintf((void *)gpPicPath,PICTURE_DIR);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				Q_GotoPage(GotoSubPage,"FileListPage",FL_SelectOne|FL_NoParent|FL_NoListBuf,&FLPS);
			}
			break;
		case PicturePage_Return:	
			Q_GotoPage(GotoNewPage,"MainPage",0,NULL);
			break;
		case PicturePage_Pre:
			if(InEvent!=Tch_Release) return 0;
			{
				Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindPrevious|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						Debug("Prev:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindLast|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							Debug("Last:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			break;
		case PicturePage_Next:
			if(InEvent!=Tch_Release) return 0;
			{
				Debug("Now:%s\n\r",gpPicPath);
				FLPS.CallBackRid=Q_FindRidByPageName(NULL);
				FLPS.pPathBuf=gpPicPath;
				FLPS.pSuffixStr=PICTURE_SUFFIX_STR;
				if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindNext|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
				{
					if(CheckMultiSuffix(gpPicPath,PICTURE_SUFFIX_STR))
					{
						Debug("Next:%s\n\r",gpPicPath);
					}
					else
					{
						if((Q_GotoPage(GotoSubPage,"FileListPage",FL_FindFrist|FL_NoListBuf,&FLPS)&SM_STATE_MASK)==SM_State_OK)
						{
							Debug("First:%s\n\r",gpPicPath);
						}
					}
					DispPic();
				}			
			}
			break;			
	}
	
	return 0;
}



